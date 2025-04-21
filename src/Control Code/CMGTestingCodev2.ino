// CMG System with Position Feedback & Motor Control
#include <avr/wdt.h>

// Pin Definitions
const int enPin = 2;        // Stepper enable
const int stepPin = 3;      // Stepper step
const int dirPin = 4;       // Stepper direction
const int dcDirPin = 8;     // DC motor direction
const int dcPwmPin = 9;     // DC motor PWM
const int buttonPin = 52;   // Reset button
const int potPin = A1;      // Potentiometer position feedback

// Stepper Parameters
const float stepAngle = 1.8;
const int microstepping = 8;
const int gearRatio = 17;
const long stepsPerDegree = (1 / (stepAngle / gearRatio)) * microstepping;

// DC Motor Control
const int MIN_DC_SPEED = 60;
const int MAX_DC_SPEED = 180;
const int ACCEL_STEP = 3;
const unsigned long ACCEL_INTERVAL = 150;

// Position Tracking
const int POT_DEADBAND = 10;      // Filter noise
const int POSITION_READ_INTERVAL = 100; // ms

// System States
enum OperationMode {
  MODE_IDLE,
  MODE_STEPPER_CW,
  MODE_STEPPER_CCW,
  MODE_DC_FORWARD,
  MODE_DC_REVERSE,
  MODE_EMERGENCY,
  MODE_POSITION_TRACK
};

// Global Variables
OperationMode currentMode = MODE_IDLE;
unsigned long lastStepTime = 0;
unsigned long buttonPressTime = 0;
unsigned long lastAccelTime = 0;
unsigned long lastPositionRead = 0;
bool lastButtonState = HIGH;
long currentPosition = 0;
int targetDcSpeed = 0;
int currentDcSpeed = 0;
bool dcMotorForward = true;
int rawPotValue = 0;
float measuredPosition = 0;

void setup() {
  // Initialize hardware
  pinMode(enPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(dcDirPin, OUTPUT);
  pinMode(dcPwmPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(potPin, INPUT);

  // Initial states
  digitalWrite(enPin, HIGH);
  analogWrite(dcPwmPin, 0);
  
  Serial.begin(115200);
  while(!Serial);
  wdt_disable();
  
  Serial.println("\nCMG System with Position Feedback");
  printHelp();
}

void loop() {
  handleInput();
  runStateMachine();
  updatePosition();
  wdt_reset();
}

// Fixed Stepper Motor Implementation
void runStateMachine() {
  static unsigned long stepsRemaining = 0;
  
  switch(currentMode) {
    case MODE_STEPPER_CW:
      if(stepsRemaining == 0) {
        stepsRemaining = 90 * stepsPerDegree;
        digitalWrite(dirPin, HIGH);
        digitalWrite(enPin, LOW);  // Enable driver
        Serial.println("Stepper CW Start");
      }
      break;

    case MODE_STEPPER_CCW:
      if(stepsRemaining == 0) {
        stepsRemaining = 90 * stepsPerDegree;
        digitalWrite(dirPin, LOW);
        digitalWrite(enPin, LOW);  // Enable driver
        Serial.println("Stepper CCW Start");
      }
      break;

    default: break;
  }

  // Stepper movement with proper pulse timing
  if((currentMode == MODE_STEPPER_CW || currentMode == MODE_STEPPER_CCW)) {
    if(micros() - lastStepTime >= 1000) {  // 1ms between steps
      if(stepsRemaining > 0) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(100);  // Minimum pulse width
        digitalWrite(stepPin, LOW);
        lastStepTime = micros();
        stepsRemaining--;
        currentPosition += (currentMode == MODE_STEPPER_CW) ? 1 : -1;
        
        // Print progress every 10% steps
        if(stepsRemaining % (stepsPerDegree * 9) == 0) {
          Serial.print("Steps remaining: ");
          Serial.println(stepsRemaining);
        }
      } else {
        digitalWrite(enPin, HIGH);  // Disable driver
        currentMode = MODE_IDLE;
        Serial.println("Stepper Movement Complete");
      }
    }
  }

  // DC motor updates
  updateDcMotorSpeed();
}

void updatePosition() {
  if(millis() - lastPositionRead > POSITION_READ_INTERVAL) {
    lastPositionRead = millis();
    
    // Read and filter potentiometer
    int newReading = analogRead(potPin);
    if(abs(newReading - rawPotValue) > POT_DEADBAND) {
      rawPotValue = newReading;
      
      // Define thresholds for when to map to 0°, 90°, etc.
      if (rawPotValue >= 0 && rawPotValue < 341) {
        measuredPosition = 0; // Cluster values near 0° (0 - 341)
      } 
      else if (rawPotValue >= 341 && rawPotValue < 682) {
        measuredPosition = 90; // Cluster values near 90° (341 - 682)
      }
      else if (rawPotValue >= 682 && rawPotValue < 1023) {
        measuredPosition = 180; // Cluster values near 180° (682 - 1023)
      }
      
      // To cover 0-360°, map between 0 and 360 for the rest of the range
      else {
        measuredPosition = map(rawPotValue, 0, 1023, 0, 360);
      }

      /* Print out for debugging
      Serial.print("Potentiometer Reading: ");
      Serial.print(rawPotValue);
      Serial.print(" -> Measured Position: ");
      Serial.println(measuredPosition);*/
    }
  }
}


void positionControl() {
  // Example: Maintain position using feedback
  // Add your control logic here
  Serial.print("Current Position: ");
  Serial.print(measuredPosition);
  Serial.println("°");
}

void handleInput() {
  bool btnState = digitalRead(buttonPin);
  
  if(btnState != lastButtonState) {
    if(btnState == LOW) handleButtonPress();
    else handleButtonRelease();
    lastButtonState = btnState;
    delay(50);
  }

  if(Serial.available()) {
    processCommand(Serial.read());
  }
}

void processCommand(char cmd) {
  switch(cmd) {
    case 's': testStepper(); break;
    case 'd': testDcMotor(); break;
    case 'p': 
      currentMode = MODE_POSITION_TRACK;
      Serial.println("Position tracking enabled");
      break;
    case 'f': setDcMotor(true, MAX_DC_SPEED); break;
    case 'r': setDcMotor(false, MAX_DC_SPEED); break;
    case 'e': emergencyStop(); break;
    case 'x': resetSystem(); break;
    case 'h': printHelp(); break;
    case '+': setDcMotor(dcMotorForward, constrain(targetDcSpeed+20, MIN_DC_SPEED, MAX_DC_SPEED)); break;
    case '-': setDcMotor(dcMotorForward, constrain(targetDcSpeed-20, MIN_DC_SPEED, MAX_DC_SPEED)); break;
    case '?': 
      Serial.print("Current Position: ");
      Serial.print(measuredPosition);
      Serial.println("°");
      break;
    default: break;
  }
}

void testStepper() {
  Serial.println("\n=== Stepper Test ===");
  
  // Clockwise rotation
  currentMode = MODE_STEPPER_CW;
  while(currentMode != MODE_IDLE) { 
    runStateMachine(); // Must be called repeatedly
    delay(10);
  }
  
  delay(1000); // Pause between movements
  
  // Counter-clockwise rotation
  currentMode = MODE_STEPPER_CCW;
  while(currentMode != MODE_IDLE) {
    runStateMachine(); // Must be called repeatedly
    delay(10);
  }
  
  Serial.println("Stepper Test Complete");
}

void testDcMotor() {
  Serial.println("\n=== DC Motor Test ===");
  setDcMotor(true, 150);
  delay(3000);
  setDcMotor(true, 0);
  delay(2000);
  setDcMotor(false, 120);
  delay(3000);
  emergencyStop();
  Serial.println("DC Motor Test Complete");
}

void setDcMotor(bool forward, int speed) {
  dcMotorForward = forward;
  targetDcSpeed = constrain(speed, 0, MAX_DC_SPEED);
  digitalWrite(dcDirPin, forward ? HIGH : LOW);
  Serial.print("DC Motor target: ");
  Serial.print(targetDcSpeed);
  Serial.println("/255");
}

void updateDcMotorSpeed() {
  if (millis() - lastAccelTime >= ACCEL_INTERVAL) {
    lastAccelTime = millis();
    
    if (currentDcSpeed < targetDcSpeed) {
      currentDcSpeed = min(currentDcSpeed + ACCEL_STEP, targetDcSpeed);
    } 
    else if (currentDcSpeed > targetDcSpeed) {
      currentDcSpeed = max(currentDcSpeed - ACCEL_STEP, targetDcSpeed);
    }
    
    analogWrite(dcPwmPin, currentDcSpeed);
  }
}

void emergencyStop() {
  currentMode = MODE_EMERGENCY;
  targetDcSpeed = 0;
  currentDcSpeed = 0;
  analogWrite(dcPwmPin, 0);
  digitalWrite(enPin, HIGH);
  Serial.println("EMERGENCY STOP!");
}

void resetSystem() {
  currentMode = MODE_IDLE;
  digitalWrite(enPin, HIGH);
  analogWrite(dcPwmPin, 0);
  currentPosition = 0;
  Serial.println("System Reset");
}

void safeRestart() {
  Serial.println("\nInitiating restart...");
  delay(100);
  wdt_enable(WDTO_15MS);
  while(1);
}

void printHelp() {
  Serial.println("\n=== System Commands ===");
  Serial.println("s: Test stepper motor");
  Serial.println("d: Test DC motor");
  Serial.println("p: Enable position tracking");
  Serial.println("f: DC motor forward");
  Serial.println("r: DC motor reverse");
  Serial.println("+: Increase speed");
  Serial.println("-: Decrease speed");
  Serial.println("?: Read current position");
  Serial.println("e: Emergency stop");
  Serial.println("x: System reset");
  Serial.println("h: Show this help");
  Serial.println("Hold button 3s for restart");
}

// Button handling functions
void handleButtonPress() {
  buttonPressTime = millis();
}

void handleButtonRelease() {
  unsigned long holdTime = millis() - buttonPressTime;
  if (holdTime > 3000) {
    safeRestart();
  } else if (holdTime > 50) {
    if (currentMode == MODE_EMERGENCY) {
      currentMode = MODE_IDLE;
      Serial.println("Emergency cleared");
    }
  }
}