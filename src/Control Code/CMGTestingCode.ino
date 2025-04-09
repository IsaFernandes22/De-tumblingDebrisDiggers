// Stepper and DC Motor Sequence Control
const int enaPin = 2;     // Stepper enable pin on D2
const int stepPin = 3;    // Stepper step pin on D3
const int dirPin = 4;     // Stepper direction pin on D4
const int dcDirPin = 8;   // DC motor direction pin on D8
const int dcPwmPin = 9;   // DC motor power pin on D9

// Stepper motor parameters
const float stepAngle = 1.8;     // Most common step angle for NEMA 17 (degrees per full step)
const int microstepping = 8;     // TMC2209 microstepping setting
const int gearRatio = 17;        // 17:1 worm gear

// Calculate steps needed for 90 degrees
// Formula: (90 degrees / (step angle / gear ratio)) * microstepping
const long stepsFor90Degrees = long((90.0 / (stepAngle / gearRatio)) * microstepping);

// Control parameters
unsigned long stepCount = 0;
bool clockwise = true;               // Start in clockwise direction
const int pauseBetweenRotations = 1000;  // 1 second pause between actions
const unsigned long dcRunTime = 20000;   // DC motor run time (20 seconds)

// State machine states
enum State {
  STEPPER_CW,    // Rotate stepper clockwise
  DC_FORWARD,    // Run DC motor
  STEPPER_CCW,   // Rotate stepper counter-clockwise
  PAUSE_FINAL    // Final pause before restarting
};

State currentState = STEPPER_CW;
unsigned long stateStartTime = 0;

void setup() {
  // Initialize pins
  pinMode(dirPin, OUTPUT);
  pinMode(enaPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dcDirPin, OUTPUT);
  pinMode(dcPwmPin, OUTPUT);
  
  // Initialize serial for debugging
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("===========================================");
  Serial.println("Stepper and DC Motor Sequence");
  Serial.println("===========================================");
  Serial.print("Steps calculated for 90 degrees: ");
  Serial.println(stepsFor90Degrees);
  
  // Set initial states
  digitalWrite(dirPin, HIGH);    // Start clockwise
  digitalWrite(enaPin, LOW);     // Enable the stepper motor
  digitalWrite(dcDirPin, HIGH);  // Set DC motor direction forward
  digitalWrite(dcPwmPin, LOW);   // DC motor initially off
  
  Serial.println("Starting with clockwise stepper rotation");
  stateStartTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long stateElapsedTime = currentTime - stateStartTime;
  
  switch (currentState) {
    case STEPPER_CW:
      // Rotate stepper 90 degrees clockwise
      if (stepCount < stepsFor90Degrees) {
        // Send step pulse
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(800);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(800);
        
        // Count this step
        stepCount++;
        
        // Print progress periodically
        if (stepCount % (stepsFor90Degrees / 10) == 0) {
          int percentComplete = (stepCount * 100) / stepsFor90Degrees;
          Serial.print("CW Progress: ");
          Serial.print(percentComplete);
          Serial.println("% complete");
        }
      } else {
        // Stepper rotation complete, move to next state
        Serial.println("Clockwise rotation complete");
        Serial.println("Starting DC motor at max speed for 20 seconds");
        
        // Disable stepper to save power
        digitalWrite(enaPin, HIGH);
        
        // Turn on DC motor at max speed
        digitalWrite(dcDirPin, HIGH);  // Forward direction
        digitalWrite(dcPwmPin, HIGH);  // Full speed
        
        // Reset step counter and update state
        stepCount = 0;
        currentState = DC_FORWARD;
        stateStartTime = currentTime;
      }
      break;
      
    case DC_FORWARD:
      // Run DC motor for 20 seconds
      if (stateElapsedTime % 5000 == 0 && stateElapsedTime > 0) {
        // Print status every 5 seconds
        Serial.print("DC motor running for ");
        Serial.print(stateElapsedTime / 1000);
        Serial.println(" seconds");
      }
      
      // Check if 20 seconds have elapsed
      if (stateElapsedTime >= dcRunTime) {
        // Turn off DC motor
        Serial.println("DC motor run complete");
        digitalWrite(dcPwmPin, LOW);
        
        Serial.println("Preparing for counter-clockwise rotation");
        
        // Re-enable stepper and set direction to counter-clockwise
        digitalWrite(enaPin, LOW);
        digitalWrite(dirPin, LOW);  // Set to counter-clockwise
        
        // Small pause for direction change to register
        delay(100);
        
        // Update state
        currentState = STEPPER_CCW;
        stateStartTime = currentTime;
      }
      break;
      
    case STEPPER_CCW:
      // Rotate stepper 90 degrees counter-clockwise
      if (stepCount < stepsFor90Degrees) {
        // Send step pulse
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(800);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(800);
        
        // Count this step
        stepCount++;
        
        // Print progress periodically
        if (stepCount % (stepsFor90Degrees / 10) == 0) {
          int percentComplete = (stepCount * 100) / stepsFor90Degrees;
          Serial.print("CCW Progress: ");
          Serial.print(percentComplete);
          Serial.println("% complete");
        }
      } else {
        // Stepper rotation complete, move to next state
        Serial.println("Counter-clockwise rotation complete");
        
        // Disable stepper to save power
        digitalWrite(enaPin, HIGH);
        
        // Update state to final pause
        currentState = PAUSE_FINAL;
        stateStartTime = currentTime;
        
        Serial.print("Pausing for ");
        Serial.print(pauseBetweenRotations / 1000);
        Serial.println(" seconds before restarting sequence");
      }
      break;
      
    case PAUSE_FINAL:
      // Final pause before restarting the sequence
      if (stateElapsedTime >= pauseBetweenRotations) {
        Serial.println("===========================================");
        Serial.println("Restarting the sequence");
        
        // Re-enable stepper and set direction to clockwise
        digitalWrite(enaPin, LOW);
        digitalWrite(dirPin, HIGH);  // Set to clockwise
        
        // Reset step counter and update state
        stepCount = 0;
        currentState = STEPPER_CW;
        stateStartTime = currentTime;
        
        Serial.println("Starting with clockwise stepper rotation");
      }
      break;
  }
}