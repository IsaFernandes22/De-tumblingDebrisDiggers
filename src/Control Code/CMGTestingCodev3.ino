// CMG System with Potentiometer-Based Position Control
#include <avr/wdt.h>

// Pin Definitions
const int enPin = 2;        // Stepper enable
const int stepPin = 3;      // Stepper step
const int dirPin = 4;       // Stepper direction
const int potPin = A1;      // Potentiometer position feedback

// Stepper Parameters
const int stepDelay = 1000; // Microseconds between steps
const int pulseWidth = 100; // Microseconds step pulse width

// Position Control
const float POSITION_TOLERANCE = 2.0;   // ±2° accuracy
const int POT_READ_INTERVAL = 50;       // ms
const int DEADBAND = 15;                // ADC deadband

// System States
enum OperationMode {
  MODE_IDLE,
  MODE_CW_TO_TARGET,
  MODE_CCW_TO_TARGET,
  MODE_EMERGENCY,
  MODE_TO_180
};

// Global Variables
OperationMode currentMode = MODE_IDLE;
unsigned long lastStepTime = 0;
unsigned long lastPotRead = 0;
float currentPosition = 0;
float targetPosition = 0;
float initialPosition = 0;

void setup() {
  pinMode(enPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  digitalWrite(enPin, HIGH);
  
  Serial.begin(115200);
  Serial.println("System Ready - 90° Position Control");
  printHelp();
}

void loop() {
  updatePosition();
  runStateMachine();
  handleInput();
  wdt_reset();
}

void runStateMachine() {
  static bool direction = true;
  
  switch(currentMode) {
    case MODE_CW_TO_TARGET:
      if(checkPosition()) {
        stopMotor();
        Serial.print("Reached CW target: ");
        Serial.println(targetPosition);
      } else {
        stepMotor(HIGH); // CW direction
      }
      break;

    case MODE_CCW_TO_TARGET:
      if(checkPosition()) {
        stopMotor();
        Serial.print("Reached CCW target: ");
        Serial.println(targetPosition);
      } else {
        stepMotor(LOW); // CCW direction
      }
      break;

    case MODE_EMERGENCY:
      digitalWrite(enPin, HIGH);
      break;

    default: break;
  }
}

// Add new function for general angle movement
void moveToAngle(float target) {
  targetPosition = fmod(target, 360.0); // Keep within 0-360
  float current = currentPosition;
  
  // Calculate shortest path
  float cwDistance = fmod(targetPosition - current + 360.0, 360.0);
  float ccwDistance = fmod(current - targetPosition + 360.0, 360.0);

  digitalWrite(enPin, LOW);
  if(cwDistance <= ccwDistance) {
    currentMode = MODE_CW_TO_TARGET;
    digitalWrite(dirPin, HIGH);
  } else {
    currentMode = MODE_CCW_TO_TARGET;
    digitalWrite(dirPin, LOW);
  }
  
  Serial.print("Moving from ");
  Serial.print(current);
  Serial.print("° to ");
  Serial.print(targetPosition);
  Serial.println("°");
}

// Modify checkPosition for general targets
bool checkPosition() {
  float error = fabs(targetPosition - currentPosition);
  error = min(error, 360 - error); // Account for circular nature
  return error <= POSITION_TOLERANCE;
}

void stepMotor(bool dir) {
  if(micros() - lastStepTime >= stepDelay) {
    digitalWrite(dirPin, dir);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(stepPin, LOW);
    lastStepTime = micros();
  }
}

void updatePosition() {
  if(millis() - lastPotRead > POT_READ_INTERVAL) {
    lastPotRead = millis();
    static int lastRaw = 0;
    int raw = analogRead(potPin);
    
    // Filter noise using deadband
    if(abs(raw - lastRaw) > DEADBAND) {
      lastRaw = raw;
      //currentPosition = map(raw, 0, 1023, 0, 3600) / 10.0; // 0.1° resolution

      
      // Define thresholds for when to map to 0°, 90°, etc.
      if (lastRaw >= 175 && lastRaw < 200) {
        currentPosition = 0; // Cluster values near 0° (0 - 341)
      } 
      else if (lastRaw >= 480 && lastRaw < 540) {
        currentPosition = 90; // Cluster values near 45° (341 - 682)
      }
      else if (lastRaw >= 857 && lastRaw < 888) {
        currentPosition = 180; // Cluster values near 180° (682 - 1023)
      }
      
      // To cover 0-360°, map between 0 and 360 for the rest of the range
      else {
        currentPosition = map(lastRaw, 175, 888, 0, 180);
      }
      
      // Optional: Uncomment for debugging
      //Serial.print("Raw: ");
      //Serial.println(lastRaw);
      //Serial.println("Measured: ");
      //Serial.println(currentPosition);
    }
  }
}

void startRotation(bool clockwise) {
  initialPosition = currentPosition;
  targetPosition = clockwise ? 
    fmod(initialPosition + 90.0, 360.0) :
    (initialPosition - 90.0 < 0 ? 360.0 + (initialPosition - 90.0) : initialPosition - 90.0);
  
  digitalWrite(enPin, LOW);
  currentMode = clockwise ? MODE_CW_TO_TARGET : MODE_CCW_TO_TARGET;
  
  Serial.print("Moving from ");
  Serial.print(initialPosition);
  Serial.print("° to ");
  Serial.println(targetPosition);
}

void stopMotor() {
  digitalWrite(enPin, HIGH);
  currentMode = MODE_IDLE;
}

void handleInput() {
  if(Serial.available()) {
    char cmd = Serial.read();
    switch(cmd) {
      case 'c': 
        startRotation(true); // CW
        break;
      case 'w': 
        startRotation(false); // CCW
        break;
      case 'e': 
        currentMode = MODE_EMERGENCY;
        Serial.println("Emergency stop!");
        break;
      case 'r': 
        stopMotor();
        Serial.println("Stopped");
        break;
      case '?': 
        Serial.print("Current Position: ");
        Serial.print(currentPosition);
        Serial.println("°");
        break;
      case 'h': 
        printHelp();
        break;
      case 'g': 
        moveToAngle(180.0);
        break;
    }
  }
}

void printHelp() {
  Serial.println("\n=== Position Control Commands ===");
  Serial.println("c: Rotate 90° clockwise");
  Serial.println("w: Rotate 90° counter-clockwise");
  Serial.println("e: Emergency stop");
  Serial.println("r: Stop movement");
  Serial.println("?: Show current position");
  Serial.println("h: Show this help");
}