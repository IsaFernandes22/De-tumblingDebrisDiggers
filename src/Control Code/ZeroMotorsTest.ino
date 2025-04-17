// @author: Fuller Clement 

// -------- Pin definitions --------
// Motor 1 (Z axis)
const int motor1_enablePin = 5;
const int motor1_stepPin   = 6;
const int motor1_dirPin    = 7;
const int limit1Pin        = 11;  // Z home switch

// Motor 2 (R axis)
const int motor2_enablePin = 2;
const int motor2_stepPin   = 3;
const int motor2_dirPin    = 4;
const int limit2Pin        = 12;  // R home switch

// Motor 3 (Θ axis)
const int motor3_enablePin = 8;
const int motor3_stepPin   = 9;
const int motor3_dirPin    = 10;
const int limit3Pin        = 13;  // Θ home switch

// Button pin
const int runButtonPin = A0;

// State flags
bool motors_on = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// Step timing
const unsigned long stepPulseHighTime = 500;
const unsigned long stepPulseLowTime  = 500;
const unsigned int homingBackoffSteps = 10;

// -------- Homing Function --------
void homeAxis(const char* axisName,
              int enablePin, int dirPin, int stepPin, int limitPin,
              int dirLevel)
{
  digitalWrite(enablePin, LOW);

  if (digitalRead(limitPin) == LOW) {
    digitalWrite(dirPin, !dirLevel);
    for (unsigned int i = 0; i < homingBackoffSteps; i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(stepPulseHighTime);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(stepPulseLowTime);
    }
  }

  digitalWrite(dirPin, dirLevel);
  while (digitalRead(limitPin) == HIGH) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);`
  }

  digitalWrite(dirPin, !dirLevel);
  for (unsigned int i = 0; i < homingBackoffSteps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);
  }

  digitalWrite(enablePin, HIGH);
}

// -------- Setup --------
void setup() {
  pinMode(motor1_enablePin, OUTPUT);
  pinMode(motor1_stepPin,   OUTPUT);
  pinMode(motor1_dirPin,    OUTPUT);

  pinMode(motor2_enablePin, OUTPUT);
  pinMode(motor2_stepPin,   OUTPUT);
  pinMode(motor2_dirPin,    OUTPUT);

  pinMode(motor3_enablePin, OUTPUT);
  pinMode(motor3_stepPin,   OUTPUT);
  pinMode(motor3_dirPin,    OUTPUT);

  pinMode(limit1Pin, INPUT_PULLUP);
  pinMode(limit2Pin, INPUT_PULLUP);
  pinMode(limit3Pin, INPUT_PULLUP);

  pinMode(runButtonPin, INPUT_PULLUP);

  digitalWrite(motor1_enablePin, HIGH);
  digitalWrite(motor2_enablePin, HIGH);
  digitalWrite(motor3_enablePin, HIGH);

  // Home all axes
  homeAxis("Z",     motor1_enablePin, motor1_dirPin, motor1_stepPin, limit1Pin, HIGH);
  homeAxis("R",     motor2_enablePin, motor2_dirPin, motor2_stepPin, limit2Pin, HIGH);
  homeAxis("Theta", motor3_enablePin, motor3_dirPin, motor3_stepPin, limit3Pin, LOW);
}

// -------- Loop --------
void loop() {
  // Debounced button read
  int buttonState = digitalRead(runButtonPin);
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW && lastButtonState == HIGH) {
      motors_on = !motors_on;

      digitalWrite(motor1_enablePin, motors_on ? LOW : HIGH);
      digitalWrite(motor2_enablePin, motors_on ? LOW : HIGH);
      digitalWrite(motor3_enablePin, motors_on ? LOW : HIGH);
    }
  }

  lastButtonState = buttonState;

  // If motors are on, step each axis
  if (motors_on) {
    digitalWrite(motor1_stepPin, HIGH);
    digitalWrite(motor2_stepPin, HIGH);
    digitalWrite(motor3_stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(motor1_stepPin, LOW);
    digitalWrite(motor2_stepPin, LOW);
    digitalWrite(motor3_stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);
  }
}
