#if defined(ESP32)
#include "esp32BLEUtilities.h"
bool BLE_status = 0;

bool isDeviceConnected = false;
bool prevDeviceConnected = false;
uint8_t tx_Value = 0;
uint8_t rxdatalength = 0;
uint8_t bytesremaining = 0;
uint8_t* rxdataBuffer = NULL;
bool newDataReceived = 0;

BLEServer *bleServer = NULL;
BLECharacteristic *bleTxCharacteristic;

class BleServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* bleServer) {
        isDeviceConnected = true;
    }

    void onDisconnect(BLEServer* bleServer) {
        isDeviceConnected = false;
    }
};

class BleCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *bleCharacteristic) {
        String rx_Value = bleCharacteristic->getValue(); // Changed std::string to String
        if (newDataReceived == 1) delete[] rxdataBuffer;

        newDataReceived = 1;
        if (rx_Value.length() > 0) {
            rxdataBuffer = new uint8_t[rx_Value.length()];
            for (int i = 0; i < rx_Value.length(); i++) {
                rxdataBuffer[i] = rx_Value[i];
                #ifdef DEBUG
                Serial.print(rxdataBuffer[i]);
                Serial.print(" ");
                #endif
            }
            #ifdef DEBUG
            Serial.println();
            #endif
            rxdatalength = rx_Value.length();
            bytesremaining = rx_Value.length();
        }
    }
};

void Esp32ble::begin(String a)  // Changed std::string to String
{
    BLEDevice::init(a);

    // Create the BLE Server
    bleServer = BLEDevice::createServer();
    bleServer->setCallbacks(new BleServerCallbacks());

    // Create the BLE Service
    BLEService *bleService = bleServer->createService(UUID_Service);

    // Create a BLE Characteristic
    bleTxCharacteristic = bleService->createCharacteristic(
        UUID_Transmit,
        BLECharacteristic::PROPERTY_NOTIFY
    );

    bleTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *bleRxCharacteristic = bleService->createCharacteristic(
        UUID_Receive,
        BLECharacteristic::PROPERTY_WRITE
    );

    bleRxCharacteristic->setCallbacks(new BleCallbacks());

    // Start the service
    bleService->start();

    // Start advertising
    bleServer->getAdvertising()->start();
    #ifdef DEBUG
    Serial.println("Waiting for a client connection to notify...");
    #endif
}

void Esp32ble::write(uint8_t a)
{
    if (isDeviceConnected) {
        bleTxCharacteristic->setValue(&a, 1);
        bleTxCharacteristic->notify();
        delay(10); // Avoid congestion in the Bluetooth stack
    }

    // Handle disconnection
    if (!isDeviceConnected && prevDeviceConnected) {
        delay(500);
        bleServer->startAdvertising();
        #ifdef DEBUG
        Serial.println("Start advertising");
        #endif
        prevDeviceConnected = isDeviceConnected;
    }
}

void Esp32ble::write(String x)  // Changed std::string to String
{
    if (isDeviceConnected) {
        bleTxCharacteristic->setValue(x.c_str());  // Convert String to const char* for setValue
        bleTxCharacteristic->notify();
        delay(10); // Avoid congestion in the Bluetooth stack
    }

    if (!isDeviceConnected && prevDeviceConnected) {
        delay(500);
        bleServer->startAdvertising();
        #ifdef DEBUG
        Serial.println("Start advertising");
        #endif
        prevDeviceConnected = isDeviceConnected;
    }
}

void Esp32ble::write(int a)
{
    if (isDeviceConnected) {
        bleTxCharacteristic->setValue(a);
        bleTxCharacteristic->notify();
        delay(10); // Avoid congestion in the Bluetooth stack
    }

    if (!isDeviceConnected && prevDeviceConnected) {
        delay(500);
        bleServer->startAdvertising();
        #ifdef DEBUG
        Serial.println("Start advertising");
        #endif
        prevDeviceConnected = isDeviceConnected;
    }
}

void Esp32ble::write(float a)
{
    if (isDeviceConnected) {
        bleTxCharacteristic->setValue(a);
        bleTxCharacteristic->notify();
        delay(10); // Avoid congestion in the Bluetooth stack
    }

    if (!isDeviceConnected && prevDeviceConnected) {
        delay(500);
        bleServer->startAdvertising();
        #ifdef DEBUG
        Serial.println("Start advertising");
        #endif
        prevDeviceConnected = isDeviceConnected;
    }
}

uint8_t Esp32ble::available()
{
    return bytesremaining;
}

uint8_t Esp32ble::read()
{
    if (bytesremaining > 0) {
        uint8_t a = rxdataBuffer[rxdatalength - bytesremaining];
        bytesremaining--;
        return a;
    } else {
        return 0;
    }
}

void Esp32ble::stop()
{
    btStop();
}

Esp32ble esp32ble;
#endif
