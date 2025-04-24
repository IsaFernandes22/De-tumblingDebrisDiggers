#include <avr/wdt.h>

// -------- Drilling System Pin Definitions --------
// Motor 1 (Z axis)
const int motor1_enablePin = 2;
const int motor1_stepPin   = 3;
const int motor1_dirPin    = 4;
const int limit1Pin        = 11;

// Motor 2 (R axis)
const int motor2_enablePin = 5;
const int motor2_stepPin   = 6;
const int motor2_dirPin    = 7;
const int limit2Pin        = 12;

// Motor 3 (Θ axis)
const int motor3_enablePin = 8;
const int motor3_stepPin   = 9;
const int motor3_dirPin    = 10;
const int limit3Pin        = 13;

// DC Motor pins (Drilling)
const int dc_enablePin = 44;
const int dc_in1Pin    = 22;
const int dc_in2Pin    = 23;

// -------- CMG System Pin Definitions --------
/*
  *STEP_EN = 28;
  *STEP_STEP = 29; //Will maybe need to be double checked bc it doesnt wanna go in
  *STEP_DIR = 30;
  *potent signal = A0;
  pwm and direction for dc are 52 and 53
*/
const int cmg_enPin    = 24;
const int cmg_stepPin  = 25;
const int cmg_dirPin   = 26;
const int cmg_dcDirPin = 28;
const int cmg_dcPwmPin = 29;
const int buttonPin    = A0;  // Button on A0
const int potPin       = A5;

// -------- Shared Constants --------
const unsigned long stepPulseHighTime = 1000;
const unsigned long stepPulseLowTime  = 1000;
const unsigned int homingBackoffSteps = 10;
const unsigned int stepsPerRevolution = 1600;

// Drilling Directions
const bool Z_HOME_DIR = HIGH;
const bool R_HOME_DIR = HIGH;
const bool T_HOME_DIR = HIGH;

// CMG Constants
const int CMG_STEP_DELAY = 1000;
const int CMG_PULSE_WIDTH = 100;
const float POSITION_TOLERANCE = 2.0;
const int MIN_DC_SPEED = 60;
const int MAX_DC_SPEED = 180;

// -------- CMG Runtime Variables --------
float currentPosition = 0;
float targetPosition = 0;
unsigned long lastStepTime = 0;
unsigned long lastPotRead = 0;
const int POT_READ_INTERVAL = 50;
const int DEADBAND = 15;

// -------- Function Prototypes --------
void lockAllDrilling();
void singleStep(int stepPin);
void homeAxis(const char* axisName, int enablePin, int dirPin, int stepPin, int limitPin, bool dirLevel);
void drillingSequence();
void cmgSequence();
bool isButtonPressed();
void moveToAngle(float target);
void updatePosition();
bool atTarget();
void stepCmgMotor(bool dir);
void setCmgDcMotor(bool forward, int speed);
void emergencyStop();

// ========== Setup ==========
void setup() {
  Serial.begin(115200);

  // Drilling Pins
  pinMode(motor1_enablePin, OUTPUT);
  pinMode(motor1_stepPin, OUTPUT);
  pinMode(motor1_dirPin, OUTPUT);
  pinMode(motor2_enablePin, OUTPUT);
  pinMode(motor2_stepPin, OUTPUT);
  pinMode(motor2_dirPin, OUTPUT);
  pinMode(motor3_enablePin, OUTPUT);
  pinMode(motor3_stepPin, OUTPUT);
  pinMode(motor3_dirPin, OUTPUT);
  pinMode(limit1Pin, INPUT_PULLUP);
  pinMode(limit2Pin, INPUT_PULLUP);
  pinMode(limit3Pin, INPUT_PULLUP);
  pinMode(dc_enablePin, OUTPUT);
  pinMode(dc_in1Pin, OUTPUT);
  pinMode(dc_in2Pin, OUTPUT);

  // CMG Pins
  pinMode(cmg_enPin, OUTPUT);
  pinMode(cmg_stepPin, OUTPUT);
  pinMode(cmg_dirPin, OUTPUT);
  pinMode(cmg_dcDirPin, OUTPUT);
  pinMode(cmg_dcPwmPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(potPin, INPUT);

  // Initialize Motors
  lockAllDrilling();
  digitalWrite(dc_enablePin, LOW);
  digitalWrite(cmg_enPin, HIGH);
  analogWrite(cmg_dcPwmPin, 0);

  Serial.println("System Ready. Press Button to Start.");
}

// ========== Main Loop ==========
void loop() {
  if (isButtonPressed()) {
    drillingSequence();
    cmgSequence();
  }
  // Check for emergency stop
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'e') emergencyStop();
  }
}

// ========== Drilling Functions ==========
void lockAllDrilling() {
  digitalWrite(motor1_enablePin, LOW);
  digitalWrite(motor2_enablePin, LOW);
  digitalWrite(motor3_enablePin, LOW);
}

void homeAxis(const char* axisName, int enablePin, int dirPin, int stepPin, int limitPin, bool dirLevel) {
  digitalWrite(enablePin, LOW);
  if (digitalRead(limitPin) == LOW) {
    digitalWrite(dirPin, !dirLevel);
    for (unsigned int i = 0; i < homingBackoffSteps; i++) singleStep(stepPin);
  }
  digitalWrite(dirPin, dirLevel);
  while (digitalRead(limitPin) == HIGH) singleStep(stepPin);
  digitalWrite(dirPin, !dirLevel);
  for (unsigned int i = 0; i < homingBackoffSteps; i++) singleStep(stepPin);
}

void drillingSequence() {
  Serial.println("Starting Drilling Sequence...");
  lockAllDrilling();

  homeAxis("Z", motor1_enablePin, motor1_dirPin, motor1_stepPin, limit1Pin, Z_HOME_DIR);
  Serial.println("Z Homed");
  homeAxis("R", motor2_enablePin, motor2_dirPin, motor2_stepPin, limit2Pin, R_HOME_DIR);
  Serial.println("R Homed");
  homeAxis("Theta", motor3_enablePin, motor3_dirPin, motor3_stepPin, limit3Pin, T_HOME_DIR);
  Serial.println("Theta Homed");

  // ... [Include all drilling movements and operations from original code here] ...

  Serial.println("Drilling Sequence Complete.");
}

// ========== CMG Functions ==========
void updatePosition() {
  if (millis() - lastPotRead > POT_READ_INTERVAL) {
    lastPotRead = millis();
    int raw = analogRead(potPin);
    if (raw >= 175 && raw < 200) currentPosition = 0;
    else if (raw >= 480 && raw < 540) currentPosition = 90;
    else if (raw >= 857 && raw < 888) currentPosition = 180;
    else currentPosition = map(raw, 175, 888, 0, 180);
  }
}

bool atTarget() {
  float error = fabs(targetPosition - currentPosition);
  error = min(error, 360 - error);
  return error <= POSITION_TOLERANCE;
}

void stepCmgMotor(bool dir) {
  if (micros() - lastStepTime >= CMG_STEP_DELAY) {
    digitalWrite(cmg_dirPin, dir);
    digitalWrite(cmg_stepPin, HIGH);
    delayMicroseconds(CMG_PULSE_WIDTH);
    digitalWrite(cmg_stepPin, LOW);
    lastStepTime = micros();
  }
}

void moveToAngle(float target) {
  targetPosition = fmod(target, 360.0);
  updatePosition();
  float current = currentPosition;
  float cwDist = fmod(targetPosition - current + 360.0, 360.0);
  bool clockwise = cwDist <= (360.0 - cwDist);

  digitalWrite(cmg_enPin, LOW);
  digitalWrite(cmg_dirPin, clockwise ? HIGH : LOW);

  while (!atTarget()) {
    updatePosition();
    stepCmgMotor(clockwise);
  }
  digitalWrite(cmg_enPin, HIGH);
}

void setCmgDcMotor(bool forward, int speed) {
  speed = constrain(speed, 0, 255);
  digitalWrite(cmg_dcDirPin, forward ? HIGH : LOW);
  analogWrite(cmg_dcPwmPin, speed);
}

void cmgSequence() {
  Serial.println("Starting CMG Sequence...");
  moveToAngle(90);
  setCmgDcMotor(true, MAX_DC_SPEED);
  delay(30000);
  setCmgDcMotor(true, 0);
  moveToAngle(180);
  setCmgDcMotor(true, MAX_DC_SPEED);
  delay(30000);
  setCmgDcMotor(true, 0);
  moveToAngle(0);
  digitalWrite(cmg_enPin, HIGH);
  Serial.println("CMG Sequence Complete.");
}

// ========== Utilities ==========
bool isButtonPressed() {
  static bool lastState = HIGH;
  bool currentState = digitalRead(buttonPin);
  if (lastState == HIGH && currentState == LOW) {
    delay(50);
    currentState = digitalRead(buttonPin);
    if (currentState == LOW) {
      lastState = currentState;
      return true;
    }
  }
  lastState = currentState;
  return false;
}

void emergencyStop() {
  Serial.println("EMERGENCY STOP!");
  setCmgDcMotor(true, 0);
  digitalWrite(cmg_enPin, HIGH);
  lockAllDrilling();
  digitalWrite(dc_enablePin, LOW);
  while(1); // Halt
}