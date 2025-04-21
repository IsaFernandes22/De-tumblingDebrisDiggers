// -------- Pin definitions --------
// Motor 1 (Z axis)
const int motor1_enablePin = 2;
const int motor1_stepPin   = 3;
const int motor1_dirPin    = 4;
const int limit1Pin        = 11;  // Z home switch

// Motor 2 (R axis)
const int motor2_enablePin = 5;
const int motor2_stepPin   = 6;
const int motor2_dirPin    = 7;
const int limit2Pin        = 12;  // R home switch

// Motor 3 (Θ axis)
const int motor3_enablePin = 8;
const int motor3_stepPin   = 9;
const int motor3_dirPin    = 10;
const int limit3Pin        = 13;  // Θ home switch

// Step timing
const unsigned long stepPulseHighTime = 1000;  // microseconds
const unsigned long stepPulseLowTime  = 1000;  // microseconds
const unsigned int homingBackoffSteps = 10;
const unsigned int stepsPerRev = 3200;

// Direction levels used during homing
const bool Z_HOME_DIR = HIGH;
const bool R_HOME_DIR = HIGH;
const bool T_HOME_DIR = HIGH;

// ------------------------------------------------
// Homing routine: drive toward limit switch, back off
void homeAxis(const char* axisName,
              int enablePin, int dirPin, int stepPin, int limitPin,
              bool dirLevel)
{
  digitalWrite(enablePin, LOW);  // Enable driver (lock motor)

  // If already pressed, back off a bit
  if (digitalRead(limitPin) == LOW) {
    digitalWrite(dirPin, !dirLevel);
    for (unsigned int i = 0; i < homingBackoffSteps; i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(stepPulseHighTime);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(stepPulseLowTime);
    }
  }

  // Move toward the switch
  digitalWrite(dirPin, dirLevel);
  while (digitalRead(limitPin) == HIGH) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);
  }

  // Back off slightly
  digitalWrite(dirPin, !dirLevel);
  for (unsigned int i = 0; i < homingBackoffSteps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);
  }

  // Keep motor enabled to lock position
}

// ------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Stepper outputs
  pinMode(motor1_enablePin, OUTPUT);
  pinMode(motor1_stepPin,   OUTPUT);
  pinMode(motor1_dirPin,    OUTPUT);

  pinMode(motor2_enablePin, OUTPUT);
  pinMode(motor2_stepPin,   OUTPUT);
  pinMode(motor2_dirPin,    OUTPUT);

  pinMode(motor3_enablePin, OUTPUT);
  pinMode(motor3_stepPin,   OUTPUT);
  pinMode(motor3_dirPin,    OUTPUT);

  // Limit switches
  pinMode(limit1Pin, INPUT_PULLUP);
  pinMode(limit2Pin, INPUT_PULLUP);
  pinMode(limit3Pin, INPUT_PULLUP);

  // Disable all stepper drivers initially
  digitalWrite(motor1_enablePin, HIGH);
  digitalWrite(motor2_enablePin, HIGH);
  digitalWrite(motor3_enablePin, HIGH);

  Serial.println("Send 'go' to begin homing sequence...");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "go") {
      Serial.println("Starting homing...");

      homeAxis("Z",     motor1_enablePin, motor1_dirPin, motor1_stepPin, limit1Pin, Z_HOME_DIR); //homing command per axis
      Serial.println("Z homed");

      homeAxis("R",     motor2_enablePin, motor2_dirPin, motor2_stepPin, limit2Pin, R_HOME_DIR);
      Serial.println("R homed");

      homeAxis("Theta", motor3_enablePin, motor3_dirPin, motor3_stepPin, limit3Pin, T_HOME_DIR);
      Serial.println("Theta homed");

      Serial.println("Homing complete.");
    }
  }
}
