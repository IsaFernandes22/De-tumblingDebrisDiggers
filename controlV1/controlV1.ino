// This is a first attempt at the code to eventually control the 
// Astroscale active debris removal detumbling project. 
// Author: Quinn 

// This could probably be hard coded, this is just for readablity and
// in case we do change this for some reason. These are going to be used
// for the indices of various arrays
const int xaxis = 0; // stepper motor (AC)
const int yaxis = 1; // stepper motor (AC)
const int zaxis = 2; // reaction wheel motor (DC)
const int reactionWheelAxes = zaxis; // in case we change this convention

// Here are some of the variables associated with the feedback control
float desiredAngVel[3] = {0,0,0};
float currentAngVel[3] = {0,0,0};
float angVelError[3] = {0,0,0};
float maxVolt = 0;
float voltToMotor[3] = {0,0,0}; 

// gains and parameters
float Ix = 0; 
float Iy = 0;
float Iz = 0;
float motorKp = 0;
float motorKi = 0;
float motorKd = 0;
float stepperKp = 0; //note to future self; do you need motor params? or are the steppers 'close enough'?
float stepperKi = 0;
float stepperKd = 0;


void setup() {
  //lol, set up input pins and shit

}

void loop() {
  // read sensor data
  currentAngVel[0] = digitalRead(xpin);
  currentAngVel[1] = digitalRead(ypin); 
  currentAngVel[1] = digitalRead(zpin); 

  //calculate voltage to send to stepper motors to line up the reaction wheel
  for (int i = 0; i < 3; i++) {
    angVelError[i] = desiredAngVel[i] - currentAngVel[i];
    if (i == reactionWheelAxis){
      voltToMotor[i] = 0; //math here using motorKs
    } else {
      voltToMotor[i] = 0; //math here using stepperKs
    }


  }



}
