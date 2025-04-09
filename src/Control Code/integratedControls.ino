// @author: Isabella Fernandes
// This is the final controls for the Mite, to be run on the Arduino Mega

#include <TMCStepper.h>         // TMCstepper - https://github.com/teemuatlut/TMCStepper
#include <SoftwareSerial.h>     // Software serial for the UART to TMC2209 - https://www.arduino.cc/en/Reference/softwareSerial
#include <Streaming.h>          // For serial debugging output - https://www.arduino.cc/reference/en/libraries/streaming/

// Define FSM states
enum State {
    PRE_DISPATCH,
    DISPATCHED,
    DISPATCH_STATE,
    CONTACT,
    ATTACHMENT,
    FOURTH_DRILL_DONE,
    DETUMBLE,
    ROTATION_CHECK,
    DONE
};
// Initialize FSM state variable
State currentState = PRE_DISPATCH;

// Function to transition between states
void transitionTo(State newState) {
currentState = newState;
Serial.print("Transitioned to state: ");
Serial.println(newState);
}

int limitSwitchPins[3] = {0, 0, 0}; // order: spinning, horizontal, vertical

int horizontalMotorPins[6] = {2, 3, 4, 5, 6, 7}; // enable, dir, step, clk, tx, rx
int verticalMotorPins[6] = {0, 0, 0, 0, 0, 0};
int spinMotorPins[6] = {0, 0, 0, 0, 0, 0};
int drillMotorPins[3] = {0, 0, 0};

SoftwareSerial horizontalSerial(horizontalMotorPins[5], horizontalMotorPins[4]);
SoftwareSerial verticalSerial(verticalMotorPins[5], verticalMotorPins[4]);
SoftwareSerial spinSerial(spinMotorPins[5], spinMotorPins[4]);
TMC2209Stepper horizontalMotor(&horizontalSerial, 0.11f, 0b00);
TMC2209Stepper verticalMotor(&verticalSerial, 0.11f, 0b00);
TMC2209Stepper spinMotor(&spinSerial, 0.11f, 0b00);

int potPin = A0;  // Potentiometer connected to A0
int potValue = 0;
int targetAngle = 0; // Angle set by potentiometer (0-180)
int currentAngle = 0; // Current angle of the motor
int maxAngle = 180; // Maximum allowed angle
int stepDelay = 500; // Delay between steps in microseconds
int stepCount = 0; // Counter for steps

void drill();
void reset();

void setup() {
  pinMode(limitSwitchPins[0], INPUT);
  pinMode(limitSwitchPins[1], INPUT);
  pinMode(limitSwitchPins[2], INPUT);

  horizontalSerial.begin(11520); // initialize software serial for UART motor control
  verticalSerial.begin(11520);
  spinSerial.begin(11520);
  horizontalMotor.beginSerial(11520); // Initialize UART
  verticalMotor.beginSerial(11520);
  spinMotor.beginSerial(11520);

  pinMode(horizontalMotorPins[0], OUTPUT);
  pinMode(horizontalMotorPins[1], OUTPUT);
  pinMode(horizontalMotorPins[2], OUTPUT);
  pinMode(verticalMotorPins[0], OUTPUT);
  pinMode(verticalMotorPins[1], OUTPUT);
  pinMode(verticalMotorPins[2], OUTPUT);
  pinMode(spinMotorPins[0], OUTPUT);
  pinMode(spinMotorPins[1], OUTPUT);
  pinMode(spinMotorPins[2], OUTPUT);
  digitalWrite(horizontalMotorPins[0], LOW); // enable!
  digitalWrite(verticalMotorPins[0], LOW);
  digitalWrite(spinMotorPins[0], LOW);

  horizontalMotor.begin();
  horizontalMotor.toff(5);
  horizontalMotor.rms_current(500);
  horizontalMotor.microsteps(256);
  horizontalMotor.en_spreadCycle(false);
  horizontalMotor.pwm_autoscale(true);
  verticalMotor.begin();
  verticalMotor.toff(5);
  verticalMotor.rms_current(500);
  verticalMotor.microsteps(256);
  verticalMotor.en_spreadCycle(false);
  verticalMotor.pwm_autoscale(true);
  spinMotor.begin();
  spinMotor.toff(5);
  spinMotor.rms_current(500);
  spinMotor.microsteps(256);
  spinMotor.en_spreadCycle(false);
  spinMotor.pwm_autoscale(true);
}

void reset() {
  // Run steppers to reset position until they hit the limit switches
  while (digitalRead(limitSwitchPins[0]) == LOW) {
    spinMotor.VACTUAL(300000);
    spinMotor.shaft(0);
    spinSerial << spinMotor.VACTUAL() << endl;
  }
  spinMotor.VACTUAL(0);
  spinSerial << spinMotor.VACTUAL() << endl;

  while (digitalRead(limitSwitchPins[1]) == LOW) {
    horizontalMotor.VACTUAL(300000);
    horizontalMotor.shaft(0);
    horizontalSerial << horizontalMotor.VACTUAL() << endl;
  }
  horizontalMotor.VACTUAL(0);
  horizontalSerial << horizontalMotor.VACTUAL() << endl;

  while (digitalRead(limitSwitchPins[2]) == LOW) {
    verticalMotor.VACTUAL(300000);
    verticalMotor.shaft(0);
    verticalSerial << verticalMotor.VACTUAL() << endl;
  }
  verticalMotor.VACTUAL(0);
  verticalSerial << verticalMotor.VACTUAL() << endl;
}

void drill() {
  // Activate the vertical motor for drilling
  verticalMotor.VACTUAL(300000);
  verticalMotor.shaft(1); // direction
  verticalSerial << verticalMotor.VACTUAL() << endl;

  delay(100); // simulate drilling duration
  verticalMotor.VACTUAL(0);
  verticalSerial << verticalMotor.VACTUAL() << endl;
}

void moveCMGToPotentiometerAngle() {
  // Read the potentiometer value and map it to an angle (0-180 degrees)
  potValue = analogRead(potPin);
  targetAngle = map(potValue, 0, 1023, 0, maxAngle); // map to 0-180 degrees

  // Ensure the target angle is within limits (0-180 degrees)
  targetAngle = constrain(targetAngle, 0, maxAngle);

  Serial.print("Target Angle: ");
  Serial.println(targetAngle);

  // Calculate steps needed to reach the target angle
  int stepsNeeded = (targetAngle - currentAngle);
  if (stepsNeeded == 0) return;

  // Rotate the motor to the desired angle
  horizontalMotor.VACTUAL(300000); // Set motor speed (adjust as needed)
  horizontalMotor.shaft(stepsNeeded > 0 ? 1 : 0); // direction
  horizontalSerial << horizontalMotor.VACTUAL() << endl;

  // Move motor in small increments to achieve the target angle
  for (int i = 0; i < abs(stepsNeeded); i++) {
    currentAngle += (stepsNeeded > 0) ? 1 : -1;
    delayMicroseconds(stepDelay);
    horizontalMotor.VACTUAL(300000); // Apply step pulse
    horizontalSerial << horizontalMotor.VACTUAL() << endl;
  }

  horizontalMotor.VACTUAL(0); // Stop motor once target angle is reached
  horizontalSerial << horizontalMotor.VACTUAL() << endl;
  Serial.println("CMG Positioning Complete");
}

void loop() {
  // FSM logic: Switch based on current state
  // TODO: make this actually work with the states
  switch (currentState) {
    case PRE_DISPATCH:
      Serial.println("In PRE_DISPATCH state.");
      // Wait for dispatch signal (this is a placeholder, adapt to your actual condition)
      // if (dispatchConditionMet) {
      transitionTo(DISPATCHED);
      break;

    case DISPATCHED:
      Serial.println("In DISPATCHED state.");
      // Wait for dispatched state to complete
      // if (dispatchedConditionMet) {
      transitionTo(DISPATCH_STATE);
      break;

    case DISPATCH_STATE:
      Serial.println("In DISPATCH_STATE.");
      // Mite in transit to the RSO
      // Add check to transition to CONTACT when the RSO is reached
      transitionTo(CONTACT); // Assuming it reaches contact immediately (adjust as needed)
      break;

    case CONTACT:
      Serial.println("In CONTACT state.");
      // Mite makes contact with the RSO
      transitionTo(ATTACHMENT);
      break;

    case ATTACHMENT:
      Serial.println("In ATTACHMENT state.");
      // Once attached, wait for the 4th drill to finish
      // if (drillCount == 4) {
      transitionTo(FOURTH_DRILL_DONE);
      break;

    case FOURTH_DRILL_DONE:
      Serial.println("4th drill done, transitioning to DETUMBLE state.");
      // Activate CMG and send log bit '1' to host satellite
      activateCMG();
      transitionTo(DETUMBLE);
      break;

    case DETUMBLE:
      Serial.println("In DETUMBLE state.");
      // CMG is activated to stabilize the RSO
      sendLogToHost("1"); // Log '1' bit to host satellite
      // Check rotation status and transition to ROTATION_CHECK
      if (checkRotationStatus()) {
        transitionTo(ROTATION_CHECK);
      }
      break;

    case ROTATION_CHECK:
      Serial.println("In ROTATION_CHECK state.");
      // Check if rotation is less than 2 degrees
      if (rotationIsLessThan2Degrees()) {
        transitionTo(DONE);
      }
      break;

    case DONE:
      Serial.println("In DONE state.");
      // Final state, turn off CMG and send log '12345' to host satellite
      deactivateCMG();
      sendLogToHost("12345");
      // Optionally reset or end the operation
      transitionTo(PRE_DISPATCH);
      break;
  }

  delay(1000); // Delay between state transitions
}

// Helper function to simulate checking rotation status
bool checkRotationStatus() {
  // TODO: Add the logic to check the rotation
  // Example: return true if rotation is less than 2 degrees
  return true; // Placeholder, adjust with actual check
}

// Helper function to simulate rotation check
bool rotationIsLessThan2Degrees() {
  // TODO: dd your actual logic here to check if rotation < 2 degrees
  return true; // Placeholder, adjust with actual check
}

// Helper function to simulate CMG activation
void activateCMG() {
  // Activate your CMG here
  Serial.println("CMG activated.");
}

// Helper function to simulate CMG deactivation
void deactivateCMG() {
  //TODO
  // Deactivate your CMG here
  Serial.println("CMG deactivated.");
}

// Helper function to send log to host satellite
void sendLogToHost(const char* log) {
  // TODO Send the log to the host satellite (e.g., via serial)
  Serial.print("Sending log to host: ");
  Serial.println(log);
}