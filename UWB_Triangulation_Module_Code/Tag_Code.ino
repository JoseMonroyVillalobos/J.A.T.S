//Final Tag Code communicates with drivetrain ESP32 via ESPNow to make it follow the tag
#include "dw3000.h"
#include <math.h>
#include <esp_now.h>
#include <WiFi.h>

#define PIN_RST 27
#define PIN_IRQ 34
#define PIN_SS 4

#define RNG_DELAY_MS 1000
#define TX_ANT_DLY 16385
#define RX_ANT_DLY 16385
#define ALL_MSG_COMMON_LEN 10
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4
#define POLL_TX_TO_RESP_RX_DLY_UUS 240
#define RESP_RX_TIMEOUT_UUS 400

// Drivetrain ESP32 MAC Address
uint8_t broadcastAddress[] = {0xF8, 0xB3, 0xB7, 0x42, 0xC4, 0xC4};

/*We send the drivetrain microcontroller an integer via ESPNOW to act as commands
0 = No movement
1= Forward
2 = Backward
3 = Counterclockwise rotation, turns to left
4 = Clockwise rotation, turns to the right
*/
typedef struct struct_message {

  int movement;

} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    5,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    1,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};

//  Anchor 1
static uint8_t tx_poll_msg1[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', '1', 'V', 'E', 0xE0, 0, 0};
static uint8_t rx_resp_msg1[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', '1', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//  Anchor 2
static uint8_t tx_poll_msg2[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', '2', 'V', 'E', 0xE0, 0, 0};
static uint8_t rx_resp_msg2[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', '2', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//  Anchor 3
static uint8_t tx_poll_msg3[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', '3', 'V', 'E', 0xE0, 0, 0};
static uint8_t rx_resp_msg3[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', '3', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static uint8_t frame_seq_nb = 0;
static uint8_t rx_buffer[20];
static uint32_t status_reg = 0;

static double tof;
static double distance1;
static double distance2;
static double distance3;

#define ANCHOR1_X 0.0
#define ANCHOR1_Y 0.0
#define ANCHOR2_X 0.13  
#define ANCHOR2_Y 0.0 
#define ANCHOR3_X 0.065 
#define ANCHOR3_Y 0.1126 
static double Magnitude;
static double Angle;

extern dwt_txconfig_t txconfig_options;


// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//  Setup for round robbin scheduling
enum AnchorState {
    ANCHOR1,
    ANCHOR2,
    ANCHOR3
};

AnchorState anchorstate = ANCHOR1; // Start with the first state


void setup()
{
  UART_init();

  spiBegin(PIN_IRQ, PIN_RST);
  spiSelect(PIN_SS);

 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  delay(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

  while (!dwt_checkidlerc()) // Need to make sure DW IC is in IDLE_RC before proceeding
  {
    UART_puts("IDLE FAILED\r\n");
    while (1)
      ;
  }

  if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
  {
    UART_puts("INIT FAILED\r\n");
    while (1)
      ;
  }

  // Enabling LEDs here for debug so that for each TX the D1 LED will flash on Esp32 DW3000 module
  dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

  /* Configure DW IC. */
  if (dwt_configure(&config)) // if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device
  {
    UART_puts("CONFIG FAILED\r\n");
    while (1)
      ;
  }

  /* Configure the TX spectrum parameters (power, PG delay and PG count) */
  dwt_configuretxrf(&txconfig_options);

  /* Apply default antenna delay value. */
  dwt_setrxantennadelay(RX_ANT_DLY);
  dwt_settxantennadelay(TX_ANT_DLY);

  /* Set expected response's delay and timeout. See NOTE 1 and 5 below.
   * As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all. */
  dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
  dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);

  /* Next can enable TX/RX states output on GPIOs 5 and 6 to help debug, and also TX/RX LEDs
   * Note, in real low power applications the LEDs should not be used. */
  dwt_setlnapamode(DWT_LNA_ENABLE | DWT_PA_ENABLE);

  Serial.println("Range RX");
  Serial.println("Setup over........");
}

bool communicateWithAnchor(double &distance, uint8_t *tx_poll_msg, uint8_t *rx_resp_msg) 
{
    //  Anchor 1 Communication 
    tx_poll_msg1[ALL_MSG_SN_IDX] = frame_seq_nb;
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS_BIT_MASK);
    dwt_writetxdata(sizeof(tx_poll_msg1), tx_poll_msg, 0); /* Zero offset in TX buffer. */
    dwt_writetxfctrl(sizeof(tx_poll_msg1), 0, 1);          /* Zero offset in TX buffer, ranging. */

    /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
    * set by dwt_setrxaftertxdelay() has elapsed. */
    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

    /* We assume that the transmission is achieved correctly, poll for reception of a frame or error/timeout. */
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
    };

    /* Increment frame sequence number after transmission of the poll message (modulo 256). */
    frame_seq_nb++;

    if (status_reg & SYS_STATUS_RXFCG_BIT_MASK)
    {
      uint32_t frame_len;

      /* Clear good RX frame event in the DW IC status register. */
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG_BIT_MASK);

      /* A frame has been received, read it into the local buffer. */
      frame_len = dwt_read32bitreg(RX_FINFO_ID) & RXFLEN_MASK;
      if (frame_len <= sizeof(rx_buffer))
      {
        dwt_readrxdata(rx_buffer, frame_len, 0);

        /* Check that the frame is the expected response from the companion "SS TWR responder" example.
        * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
        rx_buffer[ALL_MSG_SN_IDX] = 0;
        if (memcmp(rx_buffer, rx_resp_msg, ALL_MSG_COMMON_LEN) == 0)
        {
          uint32_t poll_tx_ts, resp_rx_ts, poll_rx_ts, resp_tx_ts;
          int32_t rtd_init, rtd_resp;
          float clockOffsetRatio;

          /* Retrieve poll transmission and response reception timestamps.*/
          poll_tx_ts = dwt_readtxtimestamplo32();
          resp_rx_ts = dwt_readrxtimestamplo32();

          /* Read carrier integrator value and calculate clock offset ratio. See NOTE 11 below. */
          clockOffsetRatio = ((float)dwt_readclockoffset()) / (uint32_t)(1 << 26);

          /* Get timestamps embedded in response message. */
          resp_msg_get_ts(&rx_buffer[RESP_MSG_POLL_RX_TS_IDX], &poll_rx_ts);
          resp_msg_get_ts(&rx_buffer[RESP_MSG_RESP_TX_TS_IDX], &resp_tx_ts);

          /* Compute time of flight and distance, using clock offset ratio to correct for differing local and remote clock rates */
          rtd_init = resp_rx_ts - poll_tx_ts;
          rtd_resp = resp_tx_ts - poll_rx_ts;

          tof = ((rtd_init - rtd_resp * (1 - clockOffsetRatio)) / 2.0) * DWT_TIME_UNITS;
          distance = tof * SPEED_OF_LIGHT;//Meters
          distance = distance * 39.3701; //Inches
          return true; //communication was successful
        }
      }
    }
    else
    {
      /* Clear RX error/timeout events in the DW IC status register. */
      dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    }
    return false; //communication failed
 
}

void loop()
{
  if(anchorstate == ANCHOR1)
  {

    if(communicateWithAnchor(distance1, tx_poll_msg1, rx_resp_msg1))
    {

      anchorstate = ANCHOR2;//Advance to next Anchor
    }
    

    }
  if(anchorstate == ANCHOR2)
  {

    if(communicateWithAnchor(distance2, tx_poll_msg2, rx_resp_msg2))
    {

      anchorstate = ANCHOR3;//Advance to next Anchor
    }
    
  }

  if(anchorstate == ANCHOR3)
  {

    if(communicateWithAnchor(distance3, tx_poll_msg3, rx_resp_msg3))
    {

      anchorstate = ANCHOR1;//Advance to next Anchor
    }

  } 
  Serial.printf( "Distance 1: %3.3f  Distance 2: %3.3f  Distance 3: %3.3f", distance1, distance2, distance3);
  myData.movement = 0;
  double stopdistance = 30;

  // If anchor1 distance is smallest this means tag is in front of robot. If anchor1 is greater than stop distance we do foward movement following
  if ((distance1 < distance2 && distance1 < distance3) && distance1 > stopdistance) {
      //If absolute difference between distances on anchor 2 and 3 exceed 4 this means robot must reorient
      if(abs(distance2 - distance3)>4)
      {
        //If anchor3 distance is greater than anchor2 distance2 this means tag is on the right side, so we send right movement command
        if(distance3>distance2)
        {
          myData.movement = 4;
        }
        //If anchor2 distance is greater than anchor3 distance this means tag is on the left side, so we send left movement command
        else
        {
          myData.movement = 5;
        }
      }
      else
      {
        myData.movement = 1;
      }
  }

  // Default Case, Tag is behind robot or robot is within stop distance, robot can only rotate to pooint to tag like compass
  else {
      Serial.print("  No Clear Orientation abs: ");
      Serial.println(abs(distance2 - distance3));
      //If the absolute difference between anchor 2 and 3 is greater than 4 the robot needs to reorient
      if(abs(distance2 - distance3)>4)
      {
        //If anchor3 distance is greater than anchor2 distance2 this means tag is on the right side, so we send right movement command
        if(distance3>distance2)
        {
          myData.movement = 4;
        }
        else
        {
          //If anchor2 distance is greater than anchor3 distance this means tag is on the left side, so we send left movement command
          myData.movement = 3;
        }
      }
      else
      {
        myData.movement = 0;
      }
      
  }

      
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  //Sleep(RNG_DELAY_MS);
}

