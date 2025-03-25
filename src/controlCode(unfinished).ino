// control V2! I think I should rewrite this now that I understand a little better
// Authors: Quinn, Isabella

#include <TMCStepper.h>         // TMCstepper - https://github.com/teemuatlut/TMCStepper
#include <SoftwareSerial.h>     // Software serial for the UART to TMC2209 - https://www.arduino.cc/en/Reference/softwareSerial
#include <Streaming.h>          // For serial debugging output - https://www.arduino.cc/reference/en/libraries/streaming/
#include <DFRobot_QMC5883.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

unsigned long lastDeTumbleSignalTime = 0;  // Time tracker for de-tumble signal
int deTumbleSignalCount = 0;  // Counter for sending signals
const int maxSignals = 5;  // Send signal 5 times
const unsigned long signalInterval = 240000;  // 4 minutes (240000 ms)

const int xAxis = 0; // stepper motor that CAN rotate freely
const int yAxis = 1; // stepper motor that can only go 180 degrees
const int zAxis = 2; // reaction wheel motor

int xMotorPins[3] = {2,3,4,5,6,7}; //enable, dir, step, clk, tx, rx
int yMotorPins[3] = {0,0,0,0,0,0};
int zMotorPins[3] = {0,0}; // speed, dir

SoftwareSerial xSerial(xMotorPins[5], xMotorPins[4]);
SoftwareSerial ySerial(yMotorPins[5], yMotorPins[4]);
TMC2209Stepper xMotor(&xSerial, 0.11f, 0b00);
TMC2209Stepper yMotor(&ySerial, 0.11f, 0b00);
DFRobot_QMC5883 compass(&Wire, 0X0D);

// Here are some of the variables associated with the feedback control
float prevAngPos[3] = {0,0,0};
float angPos[3] = {0,0,0};
float desiredAngVel[3] = {0,0,0}; 
float currentAngVel[3] = {0,0,0};
float prevAngVel[3] = {0,0,0};
float angVelErrorP[3] = {0,0,0}; 
float angVelErrorI[3] = {0,0,0}; // this could also be called the ang position error! Bcs it's the integral of velocity
float angVelErrorD[3] = {0,0,0}; // this could also be called the ang acceleration error! (I love math) 
float prevAngVelErrorP[3] = {0,0,0};

int voltToMotors[3] = {0,0,0}; //x, y, z

float astroscaleAngVel[3] = {0,0,0};

unsigned long prevTime = 0;
unsigned long currentTime = 0;
float deltaT = 0;
float minDeltaT = 10; // tune this later!

// states!

int state = 0;
const int PRE_DISPATCH_STATE = 0;
const int DISPATCH_STATE = 1;
const int ATTACCHMENT_STATE = 2;
const int DETUMBLE_STATE = 3;
const int FINISH_STATE = 4;


void setup() {
  xSerial.begin(11520);           // initialize software serial for UART motor control
  ySerial.begin(11520);
  xMotor.beginSerial(11520);      // Initialize UART
  yMotor.beginSerial(11520);

  pinMode(xMotorPins[0], OUTPUT);
  pinMode(xMotorPins[1], OUTPUT);
  pinMode(xMotorPins[2], OUTPUT);
  pinMode(yMotorPins[0], OUTPUT);
  pinMode(yMotorPins[1], OUTPUT);
  pinMode(yMotorPins[2], OUTPUT);
  digitalWrite(xMotorPins[0], LOW); // enable!
  digitalWrite(yMotorPins[0], LOW);

  // no idea what most of this next block is doing. Investigate later?
  xMotor.begin();
  xMotor.toff(5); 
  xMotor.rms_current(500); 
  xMotor.microsteps(256); 
  xMotor.en_spreadCycle(false);
  xMotor.pwm_autoscale(true);
  yMotor.begin();
  yMotor.toff(5); 
  yMotor.rms_current(500); 
  yMotor.microsteps(256); 
  yMotor.en_spreadCycle(false);
  yMotor.pwm_autoscale(true); 

  currentTime = millis();
  mySwitch.enableTransmit(10);  // RF transmitter pin, might need to be changed (TODO)
  // may need more stuff in set up for the gyroscope- deeply confused about the gyroscope code

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

  // giant state machine if statement
  updateMotors();

  // Check if we have reached the finished state
  if (state == FINISH_STATE) {
    if (deTumbleSignalCount < maxSignals) {
      if (millis() - lastDeTumbleSignalTime >= signalInterval || lastDeTumbleSignalTime == 0) {
        mySwitch.send(12345, 24);  // sample signal so it's unique
        Serial.println("De-tumble signal sent.");

        deTumbleSignalCount++;
        lastDeTumbleSignalTime = millis();
      }
    }
  }
}

void readGyro(){

  float declinationAngle = (11.0 - (41.0 / 60.0)) / (180 / PI); // may need to fix this number
  compass.setDeclinationAngle(declinationAngle);

  float xBuffer = 0;
  float yBuffer = 0;
  float zBuffer = 0;

  float startTime = millis();

  sVector_t mag = compass.readRaw();
  compass.getHeadingDegrees();

  float prevX = mag.XAxis();
  float prevY = mag.YAxis();
  float prevZ = mag.ZAxis();

  
  for (int sample = 0; sample < 100; sample++){ // consider tuning the number of samples
    sVector_t mag = compass.readRaw();
    compass.getHeadingDegrees();

    xBuffer += mag.XAxis() - prevX;
    yBuffer += mag.YAxis() - prevY;
    zBuffer += mag.ZAxis() - prevZ;

    prevX = mag.XAxis();
    prevY = mag.YAxis();
    prevZ = mag.ZAxis();
    
    //may need to toss a delay in here
  }
  
  currentAngVel(xAxis) = xBuffer/(millis() - startTime); // FIX: none of these lines have the correct unit conversions yet 
  currentAngVel(yAxis) = yBuffer/(millis() - startTime);
  currentAngVel(zAxis) = zBuffer/(millis() - startTime);

}

void updateMotors(){
  xMotor.VACTUAL(abs(voltToMotor[0]));
  xSerial << xMotor.VACTUAL() << endl; // deeply confused by this line of code. May need to be xSerial? But the example code is like this??
  if (voltToMotor[0] < 0){
    xMotor.shaft(0); 
  } else {
    xMotor.shaft(1);
  }

    yMotor.VACTUAL(abs(voltToMotor[1]));
  Serial << yMotor.VACTUAL() << endl;
  if (voltToMotor[1] < 0){
    yMotor.shaft(0); 
  } else {
    yMotor.shaft(1);
  }

  if (voltToMotor[2] < 0){
    digitalWrite(zMotorPins[1],LOW);
  } else{
    digitalWrite(zMotorPins[1],HIGH);
  }
  analogWrite(zMotorPins[0], abs(voltToMotor[2]));
}
