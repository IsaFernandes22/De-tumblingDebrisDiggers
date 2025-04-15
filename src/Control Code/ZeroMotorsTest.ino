// @author: Fuller Clement
//TODO: add a button to demo
// ——— Pin definitions ———
// Motor 1 (Z axis)
const int motor1_enablePin = 5;
const int motor1_stepPin   = 6;
const int motor1_dirPin    = 7;
const int limit1Pin        = 11;  // Z home switch

// Motor 2 (R axis)
const int motor2_enablePin = 2;
const int motor2_stepPin   = 3;
const int motor2_dirPin    = 4;
const int limit2Pin        = 12;  // R home switch

// Motor 3 (Θ axis)
const int motor3_enablePin = 8;
const int motor3_stepPin   = 9;
const int motor3_dirPin    = 10;
const int limit3Pin        = 13;  // Θ home switch

// State flags
bool motor1_on = false;
bool motor2_on = false;
bool motor3_on = false;

// Step timing (µs)
const unsigned long stepPulseHighTime = 500;
const unsigned long stepPulseLowTime  = 500;

// How many steps to back off when hitting or already at the switch
const unsigned int homingBackoffSteps = 10;

/**
 * Homes one axis:
 *  - axisName: human‐readable name for Serial prints
 *  - enablePin, dirPin, stepPin, limitPin: as usual
 *  - dirLevel: HIGH or LOW to move *toward* the switch
 */
void homeAxis(const char* axisName,
              int enablePin, int dirPin, int stepPin, int limitPin,
              int dirLevel)
{
  Serial.print(">> Starting homing of ");
  Serial.print(axisName);
  Serial.println(" axis");

  // Enable driver (active LOW)
  digitalWrite(enablePin, LOW);

  // --- Fail‐safe: if we're already pressing the switch, back off first ---
  if (digitalRead(limitPin) == LOW) {
    Serial.print("!! ");
    Serial.print(axisName);
    Serial.println(" already at home switch — backing off slightly");
    // reverse direction to back off
    digitalWrite(dirPin, !dirLevel);
    for (unsigned int i = 0; i < homingBackoffSteps; i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(stepPulseHighTime);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(stepPulseLowTime);
    }
    Serial.println("   Back‑off complete");
  }

  // --- Now move toward the switch ---
  digitalWrite(dirPin, dirLevel);
  unsigned long stepCount = 0;
  while (digitalRead(limitPin) == HIGH) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);

    stepCount++;
    if (stepCount % 1000 == 0) {
      Serial.print(axisName);
      Serial.print(" steps so far: ");
      Serial.println(stepCount);
    }
  }

  // Switch hit!
  Serial.print("** ");
  Serial.print(axisName);
  Serial.print(" axis home switch triggered after ");
  Serial.print(stepCount);
  Serial.println(" steps **");

  // --- Back off slightly to leave the switch ---
  Serial.print("   Backing off ");
  Serial.print(homingBackoffSteps);
  Serial.println(" steps");
  digitalWrite(dirPin, !dirLevel);
  for (unsigned int i = 0; i < homingBackoffSteps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);
  }

  // Disable driver
  digitalWrite(enablePin, HIGH);
  Serial.print(">> ");
  Serial.print(axisName);
  Serial.println(" axis homed\n");
}

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println("\n--- Gimbal Controller Starting ---");

  // —– Pin modes —–
  pinMode(motor1_enablePin, OUTPUT);
  pinMode(motor1_stepPin,   OUTPUT);
  pinMode(motor1_dirPin,    OUTPUT);

  pinMode(motor2_enablePin, OUTPUT);
  pinMode(motor2_stepPin,   OUTPUT);
  pinMode(motor2_dirPin,    OUTPUT);

  pinMode(motor3_enablePin, OUTPUT);
  pinMode(motor3_stepPin,   OUTPUT);
  pinMode(motor3_dirPin,    OUTPUT);

  // Limit switches (wired to GND when pressed)
  pinMode(limit1Pin, INPUT_PULLUP);
  pinMode(limit2Pin, INPUT_PULLUP);
  pinMode(limit3Pin, INPUT_PULLUP);

  // Disable all motors initially
  digitalWrite(motor1_enablePin, HIGH);
  digitalWrite(motor2_enablePin, HIGH);
  digitalWrite(motor3_enablePin, HIGH);

  // —– Home each axis —–
  homeAxis("Z",     motor1_enablePin, motor1_dirPin, motor1_stepPin, limit1Pin, HIGH);
  homeAxis("R",     motor2_enablePin, motor2_dirPin, motor2_stepPin, limit2Pin, HIGH);
  homeAxis("Theta", motor3_enablePin, motor3_dirPin, motor3_stepPin, limit3Pin, LOW);

  Serial.println("--- Homing complete ---");
  Serial.println("Enter commands: \"1 on\", \"1 off\", \"2 on\", \"2 off\", \"3 on\", \"3 off\"\n");
}

void loop() {
  // —– Serial control —–
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();

    if (cmd == "1 on") {
      motor1_on = true;
      digitalWrite(motor1_enablePin, LOW);
      Serial.println("[Cmd] Motor 1 (Z) ON");
    }
    else if (cmd == "1 off") {
      motor1_on = false;
      digitalWrite(motor1_enablePin, HIGH);
      Serial.println("[Cmd] Motor 1 (Z) OFF");
    }
    else if (cmd == "2 on") {
      motor2_on = true;
      digitalWrite(motor2_enablePin, LOW);
      Serial.println("[Cmd] Motor 2 (R) ON");
    }
    else if (cmd == "2 off") {
      motor2_on = false;
      digitalWrite(motor2_enablePin, HIGH);
      Serial.println("[Cmd] Motor 2 (R) OFF");
    }
    else if (cmd == "3 on") {
      motor3_on = true;
      digitalWrite(motor3_enablePin, LOW);
      Serial.println("[Cmd] Motor 3 (Θ) ON");
    }
    else if (cmd == "3 off") {
      motor3_on = false;
      digitalWrite(motor3_enablePin, HIGH);
      Serial.println("[Cmd] Motor 3 (Θ) OFF");
    }
    else {
      Serial.println("[Error] Invalid command. Use \"1 on/off\", \"2 on/off\", \"3 on/off\".");
    }
  }

  // —– Stepping loops —–
  if (motor1_on) {
    digitalWrite(motor1_stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(motor1_stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);
  }
  if (motor2_on) {
    digitalWrite(motor2_stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(motor2_stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);
  }
  if (motor3_on) {
    digitalWrite(motor3_stepPin, HIGH);
    delayMicroseconds(stepPulseHighTime);
    digitalWrite(motor3_stepPin, LOW);
    delayMicroseconds(stepPulseLowTime);
  }
}
