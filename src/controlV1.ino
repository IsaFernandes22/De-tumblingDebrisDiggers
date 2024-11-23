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
float minDeltaT = 0;

float maxVolt = 0;
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
const int DISPATCH_STATE = 0;
const int ATTACHMENT_STATE = 1;
const int DETUMBLE_STATE = 2;
const int FINISH_STATE = 3;

void setup() {
  //lol, set up input pins and shit

  astroscaleAngVel[axis] = digitalRead(pin);

  currentAngVel[xaxis] = digitalRead(xpin) - astroscaleAngVel[xaxis];
  currentAngVel[yaxis] = digitalRead(ypin) - astroscaleAngVel[yaxis];
  currentAngVel[zaxis] = digitalRead(zpin) - astroscaleAngVel[zaxis]; 

  currentTime = millis();
  delay(1); // so that deltaT cannot equal 0. Probably not necesary, but ¯\_(ツ)_/¯
}

void loop() {

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
  currentAngVel[xaxis] = digitalRead(xpin) - astroscaleAngVel[xaxis];
  currentAngVel[yaxis] = digitalRead(ypin) - astroscaleAngVel[yaxis];
  currentAngVel[zaxis] = digitalRead(zpin) - astroscaleAngVel[zaxis]; 

  if (state == DISPATCH_STATE){

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

  } else if (state == ATTACCHMENT_STATE){
    // turn on drills?
    // if drills are done, state == DETUMBLE_STATE

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

  }


  // need to actually write the voltage to output pins now, but 
  // I'm not sure how to interface w/ the drivers. I know it'll
  // be analogWrite, but do I need to use PWMs? Also, iirc the 
  // drivers we used in SEED lab handled negative inputs
  // really weridly


  while (millis() < currentTime + minDeltaT) {
    // waits until minDeltaT time has passed since the start of the loop? 
    // Idk if this will matter, but if we get unstable behavior it
    // could help to establish a max sampling rate
  }
}
