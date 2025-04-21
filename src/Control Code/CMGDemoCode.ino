// CMG System with Position Feedback & Motor Control
#include <avr/wdt.h>

// --- Pin Definitions ---
const int enPin = 2;
const int stepPin = 3;
const int dirPin = 4;
const int dcDirPin = 8;
const int dcPwmPin = 9;
const int buttonPin = 52;
const int potPin = A5;

// --- Stepper Config ---
const int stepDelay = 1000;
const int pulseWidth = 100;
const float POSITION_TOLERANCE = 2.0;

// --- DC Motor Config ---
const int MIN_DC_SPEED = 60;
const int MAX_DC_SPEED = 180;
const int ACCEL_STEP = 3;
const unsigned long ACCEL_INTERVAL = 150;

// --- Runtime State ---
float currentPosition = 0;
float targetPosition = 0;
unsigned long lastStepTime = 0;
unsigned long lastPotRead = 0;
const int POT_READ_INTERVAL = 50;
const int DEADBAND = 15;

// --- Setup ---
void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(enPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(dcDirPin, OUTPUT);
  pinMode(dcPwmPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(potPin, INPUT);

  digitalWrite(enPin, HIGH);
  analogWrite(dcPwmPin, 0);
  wdt_disable();

  Serial.println("Waiting for button press...");

  // Wait for button press
  while (digitalRead(buttonPin) == HIGH);

  Serial.println("Button pressed!");

  // Emergency stop functionality
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'e') {
      emergencyStop();
      return; // Stop the rest of the program in case of emergency
    }
  }

  // Move to 90 degrees
  moveToAngle(90);
   // Start DC Motor
  setDcMotor(true, MAX_DC_SPEED);

  // Wait 1 minute
  delay(30000);  // Spin for 1 minute

  // Stop DC motor
  setDcMotor(true, 0);

  // Move to 180 degrees
  moveToAngle(180);

  setDcMotor(true, MAX_DC_SPEED);

  // Wait 1 minute
  delay(30000);  // Spin for 1 minute

  // Stop DC motor
  setDcMotor(true, 0);

  // Move to 0 degrees
  moveToAngle(0);

  // Stop DC Motor
  setDcMotor(true, 0);
  digitalWrite(enPin, HIGH);
  Serial.println("Sequence complete.");
}

// --- Move Stepper to Target ---
void moveToAngle(float target) {
  targetPosition = fmod(target, 360.0);
  updatePosition();

  float current = currentPosition;
  float cwDist = fmod(targetPosition - current + 360.0, 360.0);
  float ccwDist = fmod(current - targetPosition + 360.0, 360.0);
  bool clockwise = cwDist <= ccwDist;

  digitalWrite(enPin, LOW);
  digitalWrite(dirPin, clockwise ? HIGH : LOW);

  Serial.print("Rotating to ");
  Serial.print(targetPosition);
  Serial.println(" degrees...");

  while (!atTarget()) {
    updatePosition();
    stepMotor(clockwise);
  }

  digitalWrite(enPin, HIGH);
  Serial.print("Reached ");
  Serial.print(targetPosition);
  Serial.println(" degrees.");
}

// --- Read Potentiometer Position ---
void updatePosition() {
  if (millis() - lastPotRead > POT_READ_INTERVAL) {
    lastPotRead = millis();
    static int lastRaw = 0;
    int raw = analogRead(potPin);

    if (abs(raw - lastRaw) > DEADBAND) {
      lastRaw = raw;
      if (raw >= 175 && raw < 200)
        currentPosition = 0;
      else if (raw >= 480 && raw < 540)
        currentPosition = 90;
      else if (raw >= 857 && raw < 888)
        currentPosition = 180;
      else
        currentPosition = map(raw, 175, 888, 0, 180);
    }
  }
}

// --- Compare Target and Current ---
bool atTarget() {
  float error = fabs(targetPosition - currentPosition);
  error = min(error, 360 - error);
  return error <= POSITION_TOLERANCE;
}

// --- Step Pulse ---
void stepMotor(bool dir) {
  if (micros() - lastStepTime >= stepDelay) {
    digitalWrite(dirPin, dir);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(stepPin, LOW);
    lastStepTime = micros();
  }
}

// --- DC Motor Control ---
void setDcMotor(bool forward, int speed) {
  speed = constrain(speed, 0, 255);
  digitalWrite(dcDirPin, forward ? HIGH : LOW);
  analogWrite(dcPwmPin, speed);

  Serial.print("DC Motor running at ");
  Serial.print(speed);
  Serial.println("/255");
}

// --- Emergency Stop ---
void emergencyStop() {
  Serial.println("Emergency stop activated!");
  setDcMotor(true, 0);
  digitalWrite(enPin, HIGH);
}

void loop(){
  updatePosition();
}