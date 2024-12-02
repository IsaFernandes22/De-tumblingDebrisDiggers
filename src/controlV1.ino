// This is a first attempt at the code to eventually control the 
// Astroscale active debris removal detumbling project. 
// Author: Quinn 

const int xaxis = 0; // stepper motor (AC)
const int yaxis = 1; // stepper motor (AC)
const int zaxis = 2; // reaction wheel motor (DC)
const int reactionWheelAxis = zaxis; // in case we change this convention

// Here are some of the variables associated with the feedback control
float desiredAngVel[3] = {0,0,0}; // make this a const? 
float currentAngVel[3] = {0,0,0};
float prevAngVel[3] = {0,0,0};
float angVelErrorP[3] = {0,0,0}; 
float angVelErrorI[3] = {0,0,0}; // this could also be called the ang position error! Bcs it's the integral of velocity
float angVelErrorD[3] = {0,0,0}; // this could also be called the ang acceleration error! (I love math) 
float prevAngVelErrorP[3] = {0,0,0};

float astroscaleAngVel[3] = {0,0,0};

unsigned long prevTime = 0;
unsigned long currentTime = 0;
float deltaT = 0;
float minDeltaT = 10; // tune this later!

float maxVoltSteppers = 12; // 1.5 A
float maxVoltReactionWheel = 12; //30 W
float voltToMotor[3] = {0,0,0}; 
int voltageSigns[3] = {0,0,0};

// gains and parameters
float Ix = 0; 
float Iy = 0;
float Iz = 0;
float motorKp = 0;
float motorKi = 0;
float motorKd = 0;
float stepperKp = 0; //note to future self; do you need motor params? or are the steppers 'close enough' to each other?
float stepperKi = 0;
float stepperKd = 0;

// states!
int state = 0;
const int PRE_DISPATCH_STATE = 0;
const int DISPATCH_STATE = 1;
const int ATTACCHMENT_STATE = 2;
const int DETUMBLE_STATE = 3;
const int FINISH_STATE = 4;

//IO pins
int sensorInputPin[xaxis] = 0;
int sensorInputPin[yaxis] = 0;
int sensorInputPin[zaxis] = 0;
int motorSpeedPin[xaxis] = 0;
int motorSpeedPin[yaxis] = 0;
int motorSpeedPin[zaxis] = 0;
int motorDirectionPin[xaxis] = 0;
int motorDirectionPin[yaxis] = 0;
int motorDirectionPin[zaxis] = 0;

void setup() {
  //lol, set up input pins and shit

  astroscaleAngVel[xaxis] = digitalRead(sensorInputPin[xaxis]);
  astroscaleAngVel[yaxis] = digitalRead(sensorInputPin[yaxis]);
  astroscaleAngVel[zaxis] = digitalRead(sensorInputPin[zaxis]);

  currentAngVel[xaxis] = digitalRead(sensorInputPin[xaxis]) - astroscaleAngVel[xaxis];
  currentAngVel[yaxis] = digitalRead(sensorInputPin[yaxis]) - astroscaleAngVel[yaxis];
  currentAngVel[zaxis] = digitalRead(sensorInputPin[zaxis]) - astroscaleAngVel[zaxis]; 

  currentTime = millis();
}

void loop() {

  while (millis() < currentTime + minDeltaT) {
    // waits until minDeltaT time has passed since the start of the loop? 
    // Idk if this will matter, but if we get unstable behavior, establishing
    // a max sampling rate may help?
  }

  prevTime = currentTime;
  currentTime = millis();
  deltaT = currentTime - prevTime; 
  
  if (prevTime > currentTime){ // in case millis() overflows (shouldn't happen, takes ~50 days)
    int negativeTime = 4294967295 - prevTime; //4294967295 is the max value of millis()
    deltaT = currentTime + negativeTime;  
  } 

  // store past values
  prevAngVel[xaxis] = currentAngVel[xaxis];
  prevAngVel[yaxis] = currentAngVel[yaxis]; 
  prevAngVel[zaxis] = currentAngVel[zaxis]; 

  // read sensor data
  currentAngVel[xaxis] = digitalRead(sensorInputPin[xaxis]) - astroscaleAngVel[xaxis];
  currentAngVel[yaxis] = digitalRead(sensorInputPin[yaxis]) - astroscaleAngVel[yaxis];
  currentAngVel[zaxis] = digitalRead(sensorInputPin[zaxis]) - astroscaleAngVel[zaxis];

  if (state == PRE_DISPATCH_STATE){

    astroscaleAngVel[xaxis] = digitalRead(sensorInputPin[xaxis]);
    astroscaleAngVel[yaxis] = digitalRead(sensorInputPin[yaxis]);
    astroscaleAngVel[zaxis] = digitalRead(sensorInputPin[zaxis]);
    // if recieve flag from pi that it's been launched, 
    // delay a certain amount of time? and then enter 
    // dispatch state

  } else if (state == DISPATCH_STATE){

    // if there is angular acceleration, state == ATTACHMENT_STATE
    // OR; the sensor we are using DOES also read linear acceleration
    // and using that instead may be easier to do without interferring 
    // with anything else

    // I'm actually super not sure how to handle this state, bcs
    // the mite will be idle, then launched (having accl), then
    // in free space (const vel wrt astroscale satelitte but possible
    // angvel wrt ground, therefore vel vec changing (but vel mag not))
    // then hit RSO (collision!). May have to talk to someone about how
    // to handle this on Tues

    for (int i = 0; i < 3; i++) {
      voltToMotor[i] = 0; 
    }

  } else if (state == ATTACCHMENT_STATE){
    // turn on drills?
    // if drills are done, state == DETUMBLE_STATE
    for (int i = 0; i < 3; i++) {
      voltToMotor[i] = 0; 
    }

  } else if (state == DETUMBLE_STATE){
      //calculate voltage to send to stepper motors to line up and spin the reaction wheel
    for (int i = 0; i < 3; i++) {
      prevAngVelErrorP[i] = angVelErrorP[i];
      angVelErrorP[i] = desiredAngVel[i] - currentAngVel[i];
      angVelErrorI[i] = angVelErrorI[i] + angVelErrorP[i] * deltaT;
      angVelErrorD[i] = (angVelErrorP[i] - prevAngVelErrorP[i])/deltaT;
      if (i == reactionWheelAxis){
        voltToMotor[i] = motorKp * angVelErrorP[i] + motorKi * angVelErrorI[i] + motorKd * angVelErrorD[i]; 
      } else {
        voltToMotor[i] = stepperKp * angVelErrorP[i] + stepperKi * angVelErrorI[i] + stepperKd * angVelErrorD[i];
      }
    }
    // if angVels are all 0, state = FINISH_STATE

  } else if (state == FINISH_STATE){
    // send done flag to Pi
    for (int i = 0; i < 3; i++) {
      voltToMotor[i] = 0; 
    }

  }

    // Sets speed and direction on motors.
  for (int i = 0; i < 3; i++) {
    if (voltToMotor[i] > 0) {
      digitalWrite(motorDirectionPin[i], HIGH);  
    } else {
      digitalWrite(motorDirectionPin[i], LOW);  
    }

    voltToMotor[i] = abs(voltToMotor[i]);
    
    if (i == reactionWheelAxis){
      voltToMotor[i] = min(voltToMotor[i], maxVoltReactionWheel);
    } else {
      voltToMotor[i] = min(voltToMotor[i], maxVoltSteppers);
    }

    voltToMotor[i] *= (255 / maxVolt); // converts to PWM input (duty cycle)! It's a value 0-255, and if it's maxVolt it should be 255
    analogWrite(motorSpeedPin[i], voltToMotor[i]);
  }
}
