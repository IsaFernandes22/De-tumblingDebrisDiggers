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

// Define steps per revolution
const unsigned int stepsPerRevolution = 1600;

// Direction levels used during homing
const bool Z_HOME_DIR = HIGH;
const bool R_HOME_DIR = HIGH;
const bool T_HOME_DIR = HIGH;


// DC Motor pins
const int dc_enablePin = 44;
const int dc_in1Pin    = 22;
const int dc_in2Pin    = 23;


// ———————————— Helpers ————————————

// Pull all ENABLE pins LOW (active‑low) to lock all motors
inline void lockAll() {
  digitalWrite(motor1_enablePin, LOW);
  digitalWrite(motor2_enablePin, LOW);
  digitalWrite(motor3_enablePin, LOW);
}

// Perform one step pulse on stepPin, re‑locking all axes first
void singleStep(int stepPin) {
  lockAll();  // ensure all drivers are enabled before each pulse
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(stepPulseHighTime);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(stepPulseLowTime);
}

// Homing routine for one axis
void homeAxis(const char* axisName,
              int enablePin, int dirPin, int stepPin, int limitPin,
              bool dirLevel)
{
  // Ensure this axis driver is enabled
  digitalWrite(enablePin, LOW);

  // If already pressed, back off a bit
  if (digitalRead(limitPin) == LOW) {
    digitalWrite(dirPin, !dirLevel);
    for (unsigned int i = 0; i < homingBackoffSteps; i++) {
      singleStep(stepPin);
    }
  }

  // Move toward the switch
  digitalWrite(dirPin, dirLevel);
  while (digitalRead(limitPin) == HIGH) {
    singleStep(stepPin);
  }

  // Back off slightly
  digitalWrite(dirPin, !dirLevel);
  for (unsigned int i = 0; i < homingBackoffSteps; i++) {
    singleStep(stepPin);
  }

  // Leave driver enabled (locked) at the end
}

// ———————————— Setup & Loop ————————————
void setup() {
  Serial.begin(9600);

  // Configure stepper driver pins
  pinMode(motor1_enablePin, OUTPUT);
  pinMode(motor1_stepPin,   OUTPUT);
  pinMode(motor1_dirPin,    OUTPUT);

  pinMode(motor2_enablePin, OUTPUT);
  pinMode(motor2_stepPin,   OUTPUT);
  pinMode(motor2_dirPin,    OUTPUT);

  pinMode(motor3_enablePin, OUTPUT);
  pinMode(motor3_stepPin,   OUTPUT);
  pinMode(motor3_dirPin,    OUTPUT);

  // Configure limit switches
  pinMode(limit1Pin, INPUT_PULLUP);
  pinMode(limit2Pin, INPUT_PULLUP);
  pinMode(limit3Pin, INPUT_PULLUP);

  // Lock all motors from power‑up onward
  lockAll();

    pinMode(dc_enablePin, OUTPUT);
  pinMode(dc_in1Pin,    OUTPUT);
  pinMode(dc_in2Pin,    OUTPUT);

  // Ensure DC motor is off at startup
  digitalWrite(dc_enablePin, LOW);
  digitalWrite(dc_in1Pin, LOW);
  digitalWrite(dc_in2Pin, LOW);

  Serial.println("Send 'go' to begin homing sequence...");
}

void loop() {
  // Continuously re‑assert lock while idle
  lockAll();

  if (Serial.available() > 0) {
     String command = Serial.readStringUntil('\n');
     command.trim();

     if (command == "go") {
      Serial.println("Starting homing...");

      // Extra-safe: re-lock all axes
      lockAll();

      homeAxis("Z",     motor1_enablePin, motor1_dirPin, motor1_stepPin, limit1Pin, Z_HOME_DIR);
      Serial.println("Z homed");

      homeAxis("R",     motor2_enablePin, motor2_dirPin, motor2_stepPin, limit2Pin, R_HOME_DIR);
      Serial.println("R homed");

      homeAxis("Theta", motor3_enablePin, motor3_dirPin, motor3_stepPin, limit3Pin, T_HOME_DIR);
      Serial.println("Theta homed");

      Serial.println("Homing complete.");

            // Wait 5 seconds
      delay(5000);

      // Move Theta motor in LOW direction for 5 revolutions
      Serial.println("Moving Theta motor for 4.8 revolutions...");

      digitalWrite(motor3_dirPin, LOW);  // Set direction to LOW
      for (unsigned int i = 0; i < stepsPerRevolution * 4.8; i++) {
        singleStep(motor3_stepPin);
      }

      Serial.println("Theta movement complete.");

      //Wait 1 second
      delay(1000);

      // Move R axis in LOW direction for 1.35 revolutions
      Serial.println("Moving R axis for 1.35 revolutions...");
      digitalWrite(motor2_dirPin, LOW);  // LOW direction
      for (unsigned int i = 0; i < stepsPerRevolution * 0.43; i++) {
        singleStep(motor2_stepPin);
      }
      Serial.println("R axis movement complete.");

      // Wait 1 second
      delay(1000);

      // Turn on DC motor (forward)
      Serial.println("Turning on DC motor...");
      digitalWrite(dc_enablePin, HIGH);
      digitalWrite(dc_in1Pin, HIGH);
      digitalWrite(dc_in2Pin, LOW);

      // Wait 1 second
      delay(1000);

      // Move Z motor in LOW direction for 2 revolutions
      Serial.println("Moving Z axis for 2 revolutions...");
      digitalWrite(motor1_dirPin, LOW);  // LOW direction
      for (unsigned int i = 0; i < stepsPerRevolution * 2; i++) {
        singleStep(motor1_stepPin);
      }
      Serial.println("Z axis movement complete.");

      // Stop DC motor
      Serial.println("Stopping DC motor...");
      digitalWrite(dc_enablePin, LOW);
      digitalWrite(dc_in1Pin, LOW);
      digitalWrite(dc_in2Pin, LOW);

      Serial.println("Sequence complete.");

      // Wait 1 second
      delay(1000);
      // Move Z motor in HIGH direction for 2 revolutions
      Serial.println("Moving Z axis for 2 revolutions...");
      digitalWrite(motor1_dirPin, HIGH);  // HIGH direction
      for (unsigned int i = 0; i < stepsPerRevolution * 2; i++) {
        singleStep(motor1_stepPin);
      }
      Serial.println("Z axis movement complete.");
// Wait 1 second
      delay(1000);

      // Move R axis in HIGH direction for 1.35 revolutions
      Serial.println("Moving R axis for 1.35 revolutions...");
      digitalWrite(motor2_dirPin, HIGH);  // HIGH direction
      for (unsigned int i = 0; i < stepsPerRevolution * 0.43; i++) {
        singleStep(motor2_stepPin);
      }
      Serial.println("R axis movement complete.");

     // Wait 1 second
      delay(1000);

      // Turn on DC motor (forward)
      Serial.println("Turning on DC motor...");
      digitalWrite(dc_enablePin, HIGH);
      digitalWrite(dc_in1Pin, HIGH);
      digitalWrite(dc_in2Pin, LOW);

      // Wait 1 second
      delay(1000);

      // Move Z motor in LOW direction for 2 revolutions
      Serial.println("Moving Z axis for 2 revolutions...");
      digitalWrite(motor1_dirPin, LOW);  // LOW direction
      for (unsigned int i = 0; i < stepsPerRevolution * 2; i++) {
        singleStep(motor1_stepPin);
      }
      Serial.println("Z axis movement complete.");

      // Stop DC motor
      Serial.println("Stopping DC motor...");
      digitalWrite(dc_enablePin, LOW);
      digitalWrite(dc_in1Pin, LOW);
      digitalWrite(dc_in2Pin, LOW);

      Serial.println("Sequence complete.");

      // Wait 1 second
      delay(1000);
      // Move Z motor in HIGH direction for 2 revolutions
      Serial.println("Moving Z axis for 2 revolutions...");
      digitalWrite(motor1_dirPin, HIGH);  // HIGH direction
      for (unsigned int i = 0; i < stepsPerRevolution * 2; i++) {
        singleStep(motor1_stepPin);
      }
      Serial.println("Z axis movement complete.");



      delay(1000);

      // Move Theta motor in HIGH direction for 2.4 revolutions
      Serial.println("Moving Theta motor for 2.4 revolutions...");

      digitalWrite(motor3_dirPin, HIGH);  // Set direction to HIGH
      for (unsigned int i = 0; i < stepsPerRevolution * 2.4; i++) {
        singleStep(motor3_stepPin);
      }

      Serial.println("Theta movement complete.");



           delay(1000);

      // Turn on DC motor (forward)
      Serial.println("Turning on DC motor...");
      digitalWrite(dc_enablePin, HIGH);
      digitalWrite(dc_in1Pin, HIGH);
      digitalWrite(dc_in2Pin, LOW);

      // Wait 1 second
      delay(1000);

      // Move Z motor in LOW direction for 2 revolutions
      Serial.println("Moving Z axis for 2 revolutions...");
      digitalWrite(motor1_dirPin, LOW);  // LOW direction
      for (unsigned int i = 0; i < stepsPerRevolution * 2; i++) {
        singleStep(motor1_stepPin);
      }
      Serial.println("Z axis movement complete.");

      // Stop DC motor
      Serial.println("Stopping DC motor...");
      digitalWrite(dc_enablePin, LOW);
      digitalWrite(dc_in1Pin, LOW);
      digitalWrite(dc_in2Pin, LOW);

      Serial.println("Sequence complete.");

      // Wait 1 second
      delay(1000);
      // Move Z motor in HIGH direction for 2 revolutions
      Serial.println("Moving Z axis for 2 revolutions...");
      digitalWrite(motor1_dirPin, HIGH);  // HIGH direction
      for (unsigned int i = 0; i < stepsPerRevolution * 2; i++) {
        singleStep(motor1_stepPin);
      }
      Serial.println("Z axis movement complete.");




            delay(1000);

      // Move Theta motor in HIGH direction for 2.4 revolutions
      Serial.println("Moving Theta motor for 2.4 revolutions...");

      digitalWrite(motor3_dirPin, HIGH);  // Set direction to HIGH
      for (unsigned int i = 0; i < stepsPerRevolution * 2.4; i++) {
        singleStep(motor3_stepPin);
      }

      Serial.println("Theta movement complete.");



           delay(1000);

      // Turn on DC motor (forward)
      Serial.println("Turning on DC motor...");
      digitalWrite(dc_enablePin, HIGH);
      digitalWrite(dc_in1Pin, HIGH);
      digitalWrite(dc_in2Pin, LOW);

      // Wait 1 second
      delay(1000);

      // Move Z motor in LOW direction for 2 revolutions
      Serial.println("Moving Z axis for 2 revolutions...");
      digitalWrite(motor1_dirPin, LOW);  // LOW direction
      for (unsigned int i = 0; i < stepsPerRevolution * 2; i++) {
        singleStep(motor1_stepPin);
      }
      Serial.println("Z axis movement complete.");

      // Stop DC motor
      Serial.println("Stopping DC motor...");
      digitalWrite(dc_enablePin, LOW);
      digitalWrite(dc_in1Pin, LOW);
      digitalWrite(dc_in2Pin, LOW);

      Serial.println("Sequence complete.");

      // Wait 1 second
      delay(1000);
      // Move Z motor in HIGH direction for 2 revolutions
      Serial.println("Moving Z axis for 2 revolutions...");
      digitalWrite(motor1_dirPin, HIGH);  // HIGH direction
      for (unsigned int i = 0; i < stepsPerRevolution * 2; i++) {
        singleStep(motor1_stepPin);
      }
      Serial.println("Z axis movement complete.");
      Serial.println("All steps complete.");
    }
  }
}
