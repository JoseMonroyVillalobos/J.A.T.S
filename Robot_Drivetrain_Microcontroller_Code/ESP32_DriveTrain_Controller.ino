#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>
#include <math.h>
#define MAX_PWM_VALUE 191  // Max PWM value to limit voltage to 6V when battery is at 8.6V * (177/255)= 6V. Motors rated for 6V

// Define sound speed in cm/us
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// Motor Pins
int rightMotorPin1 = 22;
int rightMotorPin2 = 4; 
int rightMotorPWM = 26; 

int leftMotorPin1 = 19;
int leftMotorPin2 = 21;
int leftMotorPWM = 25;  

int StandByPin = 5; // High when motors are in motion, low when motors are not in use

// PWM configuration
const int freq = 1600;  // PWM frequency set to 20 kHz
const int resolution = 8; // PWM resolution (8 bits)

const int SPEED_VALUE = MAX_PWM_VALUE; // Maximum allowed PWM value




long duration;
float FdistanceCm;
float BdistanceCm;


unsigned long previousMillis = 0; // Stores the last time the sensor was checked
const long interval = 500;        // Interval between distance checks in milliseconds

//Front Ultra Sonic Sensor
const int FtrigPin = 39; // Pin for Trigger
const int FechoPin = 36; // Pin for Echo

//Back Ultra Sonic Sensor
const int BtrigPin = 35; // Pin for Trigger
const int BechoPin = 34; // Pin for Echo

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed) {

  // Right motor direction
  if (rightMotorSpeed > 0) {
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);
    digitalWrite(StandByPin, HIGH);
  } else if (rightMotorSpeed < 0) {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);
    digitalWrite(StandByPin, HIGH);
  } else {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, LOW);
    digitalWrite(StandByPin, LOW);
  }

  // Left motor direction
  if (leftMotorSpeed > 0) {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);
    digitalWrite(StandByPin, HIGH);
  } else if (leftMotorSpeed < 0) {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);
    digitalWrite(StandByPin, HIGH);
  } else {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, LOW);
    digitalWrite(StandByPin, LOW);
  }

  // Right motor speed (PWM)
  int rightSpeedValue = abs(rightMotorSpeed);
  ledcWrite(rightMotorPWM, rightSpeedValue);

  // Left motor speed (PWM)
  int leftSpeedValue = abs(leftMotorSpeed);
  ledcWrite(leftMotorPWM, leftSpeedValue);
}


//Experimental controls not yet ready
void joystickControl(int angle, int radius) {
  // Map radius to PWM speed (0 to 255)
  int speed = map(radius, 0, 7, 0, MAX_PWM_VALUE);

  // Variables for motor speeds
  int leftMotorSpeed = 0;
  int rightMotorSpeed = 0;

  // Convert angle to radians for trigonometric functions
  float radians = radians(angle);

  // Calculate motor speeds based on the angle
  if (angle == 0) { // Hard right
    leftMotorSpeed = speed;
    rightMotorSpeed = -speed;
  } else if (angle == 90) { // Forward
    leftMotorSpeed = speed;
    rightMotorSpeed = speed;
  } else if (angle == 180) { // Hard left
    leftMotorSpeed = -speed;
    rightMotorSpeed = speed;
  } else if (angle == 270) { // Backward
    leftMotorSpeed = -speed;
    rightMotorSpeed = -speed;
  } else { // For other angles, use trigonometric mapping
    leftMotorSpeed = speed * sin(radians);
    rightMotorSpeed = speed * cos(radians);
  }

  // Output values to Serial Monitor
  Serial.print("Angle: ");
  Serial.print(angle);
  Serial.print("°     Radius: ");
  Serial.print(radius);
  Serial.print("    Speed: ");
  Serial.print(speed);
  Serial.print("    Left Motor Speed: ");
  Serial.print(leftMotorSpeed);
  Serial.print("    Right Motor Speed: ");
  Serial.println(rightMotorSpeed);

  // Call motor control function
  rotateMotor(rightMotorSpeed, leftMotorSpeed);
}


void setUpPinModes() {
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);

  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);

  pinMode(StandByPin, OUTPUT);

  pinMode(FtrigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(FechoPin, INPUT); // Sets the echoPin as an Input
  pinMode(BtrigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(BechoPin, INPUT); // Sets the echoPin as an Input

  rotateMotor(0, 0); // Stop motors at the start
}

void setup() {
  Serial.begin(9600);
  setUpPinModes();
  Dabble.begin("MyBluetoothCarrrrrrrrrrr"); // Initialize Dabble Bluetooth, made name more distinct to make it more identifiable on list of bluetooth devices

  // Configure PWM pins 
  if (!ledcAttach(rightMotorPWM, freq, resolution)) {
    Serial.println("Error: Failed to configure right motor PWM!");
  }

  if (!ledcAttach(leftMotorPWM, freq, resolution)) {
    Serial.println("Error: Failed to configure left motor PWM!");
  }
}



bool forward = false;
bool backward = false;




void loop() {
  //unsigned long currentMillis = millis();
  int rightMotorSpeed = 0;
  int leftMotorSpeed = 0;
/*
  // Check distance at defined intervals
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Trigger the ultrasonic sensor
    digitalWrite(FtrigPin, LOW);
    
    delayMicroseconds(2);
    digitalWrite(FtrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(FtrigPin, LOW);

    // Read the echoPin, returning the sound wave travel time in microseconds
    duration = pulseIn(FechoPin, HIGH, 3000); // Timeout of 30ms for max distance ~5m

    // Calculate the distance
    FdistanceCm = duration * SOUND_SPEED / 2;
    if (BdistanceCm!=0) { // Example threshold: 20 cm
          Serial.println("Obstacle detected in the Front! Stopping and reversing...");
          
        }
    // Trigger the ultrasonic sensor
    digitalWrite(BtrigPin, LOW);
    
    delayMicroseconds(2);
    digitalWrite(BtrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(BtrigPin, LOW);

    // Read the echoPin, returning the sound wave travel time in microseconds
    duration = pulseIn(BechoPin, HIGH, 3000); // Timeout of 30ms for max distance ~5m

    // Calculate the distance
    BdistanceCm = duration * SOUND_SPEED / 2;


    if (BdistanceCm!=0) { // Example threshold: 20 cm
      Serial.println("Obstacle detected in the back! Stopping and moving forward...");
      
    }
  }
*/
  
  Dabble.processInput(); // Process Bluetooth gamepad input

  // Determine motor speeds based on gamepad input
  if (GamePad.isUpPressed()) {
    rightMotorSpeed = SPEED_VALUE;
    leftMotorSpeed = SPEED_VALUE;
  }
  else if (GamePad.isDownPressed()) {
    rightMotorSpeed = -SPEED_VALUE;
    leftMotorSpeed = -SPEED_VALUE;
  }
  else if (GamePad.isLeftPressed()) {
    rightMotorSpeed = SPEED_VALUE;
    leftMotorSpeed = -SPEED_VALUE;
  }
  else if (GamePad.isRightPressed()) {
    rightMotorSpeed = -SPEED_VALUE;
    leftMotorSpeed = SPEED_VALUE;
  }
    else if (GamePad.isTrianglePressed())
  {
    rightMotorSpeed = SPEED_VALUE;
    leftMotorSpeed = SPEED_VALUE;
    forward = true;
    backward = false;
  }
  else if (GamePad.isCrossPressed())
  {
    rightMotorSpeed = -SPEED_VALUE;
    leftMotorSpeed = -SPEED_VALUE;
    forward = false;
    backward = true;
  }
  else if (GamePad.isCirclePressed())
  {
    if(forward){
      rightMotorSpeed = SPEED_VALUE/4;
      leftMotorSpeed = SPEED_VALUE;
    }
    else if(backward){
      rightMotorSpeed = -SPEED_VALUE/4;
      leftMotorSpeed = -SPEED_VALUE;
    }
  }
  else if (GamePad.isSquarePressed())
  {
    if(forward){
      rightMotorSpeed = SPEED_VALUE;
      leftMotorSpeed = SPEED_VALUE/4;;
    }
    else if(backward){
      rightMotorSpeed = -SPEED_VALUE;
      leftMotorSpeed = -SPEED_VALUE/4;;
    }
  }
  else if (GamePad.isSelectPressed())
  {
    
  }
  /*
  int a = GamePad.getAngle();
  Serial.print("Angle: ");
  Serial.print(a);
  Serial.print('\t');
  int b = GamePad.getRadius();
  Serial.print("Radius: ");
  Serial.print(b);
  Serial.print('\t');
  float c = GamePad.getXaxisData();
  Serial.print("x_axis: ");
  Serial.print(c);
  Serial.print('\t');
  float d = GamePad.getYaxisData();
  Serial.print("y_axis: ");
  Serial.println(d);
  Serial.println();
  */
  /*
  int angle = GamePad.getAngle();
  int radius = GamePad.getRadius();
  joystickControl( angle, radius);
  */
  
  rotateMotor(rightMotorSpeed, leftMotorSpeed); // Update motor speed and direction



}
