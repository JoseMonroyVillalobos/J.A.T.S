#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

#define MAX_PWM_VALUE 177  // Max PWM value to limit voltage to 6V when battery is at 8.6V * (177/255)= 6V. Motors rated for 6V

// Motor Pins
int rightMotorPin1 = 22;
int rightMotorPin2 = 4; 
int rightMotorPWM = 26; 

int leftMotorPin1 = 19;
int leftMotorPin2 = 21;
int leftMotorPWM = 25;  



int StandByPin = 5; // High when motors are in motion, low when motors are not in use

// PWM configuration
const int freq = 1000;  // PWM frequency set to 20 kHz
const int resolution = 8; // PWM resolution (8 bits)

const int SPEED_VALUE = MAX_PWM_VALUE; // Maximum allowed PWM value

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed) {
  //Currently set on high, will implement correctly later
  digitalWrite(StandByPin, HIGH);

  // Right motor direction
  if (rightMotorSpeed > 0) {
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);
  } else if (rightMotorSpeed < 0) {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);
  } else {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, LOW);
  }

  // Left motor direction
  if (leftMotorSpeed > 0) {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);
  } else if (leftMotorSpeed < 0) {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);
  } else {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, LOW);
  }

  // Right motor speed (PWM)
  int rightSpeedValue = abs(rightMotorSpeed);
  ledcWrite(rightMotorPWM, rightSpeedValue);

  // Left motor speed (PWM)
  int leftSpeedValue = abs(leftMotorSpeed);
  ledcWrite(leftMotorPWM, leftSpeedValue);
}

void setUpPinModes() {
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);

  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);

  pinMode(StandByPin, OUTPUT);

  rotateMotor(0, 0); // Stop motors at the start
}

void setup() {
  Serial.begin(9600);
  setUpPinModes();
  Dabble.begin("*******************MyBluetoothCar*******************"); // Initialize Dabble Bluetooth, made name more distinct to make it more identifiable on list of bluetooth devices

  // Configure PWM pins 
  if (!ledcAttach(rightMotorPWM, freq, resolution)) {
    Serial.println("Error: Failed to configure right motor PWM!");
  }

  if (!ledcAttach(leftMotorPWM, freq, resolution)) {
    Serial.println("Error: Failed to configure left motor PWM!");
  }
}

void loop() {
  int rightMotorSpeed = 0;
  int leftMotorSpeed = 0;

  Dabble.processInput(); // Process Bluetooth gamepad input

  // Determine motor speeds based on gamepad input
  if (GamePad.isUpPressed()) {
    rightMotorSpeed = SPEED_VALUE;
    leftMotorSpeed = SPEED_VALUE;
  }
  if (GamePad.isDownPressed()) {
    rightMotorSpeed = -SPEED_VALUE;
    leftMotorSpeed = -SPEED_VALUE;
  }
  if (GamePad.isLeftPressed()) {
    rightMotorSpeed = SPEED_VALUE;
    leftMotorSpeed = -SPEED_VALUE;
  }
  if (GamePad.isRightPressed()) {
    rightMotorSpeed = -SPEED_VALUE;
    leftMotorSpeed = SPEED_VALUE;
  }

  rotateMotor(rightMotorSpeed, leftMotorSpeed); // Update motor speed and direction
}
