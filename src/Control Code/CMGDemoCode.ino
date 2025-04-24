// CMG System with Position Feedback & Motor Control
#include <avr/wdt.h>

// --- Pin Definitions ---
const int enPin = 30;
const int stepPin = 31;
const int dirPin = 32;
const int dcDirPin = 53;
const int dcPwmPin = 52;
const int buttonPin = A5;
const int potPin = A0;

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

  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'e') {
      emergencyStop();
      return;
    }
  }

  // Start-up test sequence
  moveToAngle(0);           // Always counterclockwise
  delay(2000);

  setDcMotor(true, 0);
  moveToAngle(90);          // Clockwise
  setDcMotor(true, MAX_DC_SPEED);
  delay(10000);             // Run motor briefly
  setDcMotor(true, 0);

  delay(10000);

  setDcMotor(true, 0);
  moveToAngle(180);         // Clockwise
  setDcMotor(true, MAX_DC_SPEED);
  delay(10000);
  setDcMotor(true, 0);

  delay(10000);

  moveToAngle(0);           // Counterclockwise again
  setDcMotor(true, 0);
  digitalWrite(enPin, HIGH);

  Serial.println("Sequence complete.");
}

// --- Move Stepper to Target ---
void moveToAngle(float target) {
  targetPosition = fmod(target, 360.0);
  updatePosition();

  float current = currentPosition;
  bool clockwise = (targetPosition != 0); // now: any non-zero goes CW, 0 goes CCW

  digitalWrite(enPin, LOW);
  digitalWrite(dirPin, clockwise ? HIGH : LOW);

  Serial.print("Rotating ");
  Serial.print(clockwise ? "CW" : "CCW");
  Serial.print(" to ");
  Serial.print(targetPosition);
  Serial.println(" degrees...");

  while (!atTarget()) {
    updatePosition();

    Serial.print("Current: ");
    Serial.print(currentPosition);
    Serial.print(" | Target: ");
    Serial.println(targetPosition);

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
      if (raw >= 160 && raw < 186)
        currentPosition = 0;
      else if (raw >= 480 && raw < 540)
        currentPosition = 90;
      else if (raw >= 857 && raw < 888)
        currentPosition = 180;
      else
        currentPosition = map(raw, 175, 888, 0, 180);
    }

    Serial.print("Potentiometer Raw: ");
    Serial.print(raw);
    Serial.print(" | Interpreted Degrees: ");
    Serial.println(currentPosition);
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
    digitalWrite(dirPin, !dir);  // correct direction
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

void loop() {
  updatePosition();
}
