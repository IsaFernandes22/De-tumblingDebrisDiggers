// @author: Isabella Fernandes
// TODO: this is currently code just for attachment demo, CMG code still needs to be added
// // ---------------------------------------------------------

const int BTN_PIN = A0; // Start button

// // ---------- Pin Assignments ----------
// Motor 1 (Z axis)
//TODO: is there 2 motors here? one for up and down and one for the actual drill?
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

// ---------- Motion parameters ----------
const int    STEPS_PER_REV = 200;   // adjust for your motor/driver
const int    STEP_DELAY_US = 1000;  // speed control (µs per step)
const uint16_t DRILL_TIME_MS = 2000; // spin time after each Z‑down move

// ---------- Forward declarations ----------
void runSequence();
void homeAxis(int en,int st,int dir,int lim,bool towardHi);
void moveSteps(int enablePin, int dirPin, int stepPin, int limitPin, bool dirLevel); //TODO: figure out how this needs to move different axes
void drillZ(float rot,bool down); //this moves drill (not up or down, cw or ccw)
void disableAllAttach();


//-------------------------------------------------
void setup(){
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP); //TODO: see if button restarts system on press without using the loop function

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

  runSequence();
  disableAllAttach(); // power down motors when done

  //TODO: CMG code will run after sequence is done

}

void loop(){
    // should be empty (button press should reset system)
}

// ------------------------------------------------
// ---------- Helper functions -------------------

void runSequence(){
  //home all axes
  

  //move theta slightly to clear the way for R axis

  //locate to center

  //drill down and back up

  //locate center edge

  //drill down and back up

  //locate 3rd point (theta drill)

  //drill down and back up

  //locate 4th point (theta drill)

  //drill down and back up

}

// tested and works
void homeAxis(int enablePin, int dirPin, int stepPin, int limitPin,
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
  //TODO: check that this is done
}

void drillZ(float rot,bool down){
  //TODO: check that this is correct
  // Start drill spindle
  digitalWrite(DC_IN1, HIGH);
  digitalWrite(DC_IN2, LOW);
  digitalWrite(DC_EN,  HIGH);

  // Move Z axis
  moveSteps(Z_EN, Z_STEP, Z_DIR, rotations, down);
  delay(DRILL_TIME_MS);  // drill continues spinning

  // Stop drill spindle
  digitalWrite(DC_EN, LOW);
}

void disableAllAttach(){

}