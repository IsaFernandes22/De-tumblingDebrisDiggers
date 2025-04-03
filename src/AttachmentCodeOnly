#include <TMCStepper.h>         // TMCstepper - https://github.com/teemuatlut/TMCStepper
#include <SoftwareSerial.h>     // Software serial for the UART to TMC2209 - https://www.arduino.cc/en/Reference/softwareSerial
#include <Streaming.h>          // For serial debugging output - https://www.arduino.cc/reference/en/libraries/streaming/

int limitSwitchPins[3] = [0,0,0]; // order: spinning, horizontal, vertical

int horizontalMotorPins[6] = {2,3,4,5,6,7}; //enable, dir, step, clk, tx, rx
int verticalMotorPins[6] = {0,0,0,0,0,0};
int spinMotorPins[6] = {0,0,0,0,0,0};
int drillMotorPins[3] = {0,0,0}

SoftwareSerial horizontalSerial(horizontalMotorPins[5], horizontalMotorPins[4]);
SoftwareSerial verticalSerial(verticalMotorPins[5], verticalMotorPins[4]);
SoftwareSerial spinSerial(spinMotorPins[5], spinMotorPins[4]);
TMC2209Stepper horizontalMotor(&horizontalSerial, 0.11f, 0b00);
TMC2209Stepper verticalMotor(&verticalSerial, 0.11f, 0b00);
TMC2209Stepper spinMotor(&spinSerial, 0.11f, 0b00);

void drill();
void reset();

void setup() {

  pinMode(limitSwitchPins[0], INPUT);
  pinMode(limitSwitchPins[1], INPUT);
  pinMode(limitSwitchPins[2], INPUT);


  horizontalSerial.begin(11520);           // initialize software serial for UART motor control
  verticalSerial.begin(11520);
  spinSerial.begin(11520);
  horizontalMotor.beginSerial(11520);      // Initialize UART
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

  // no idea what most of this next block is doing. Investigate later?
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

void loop() {
  
  reset(); // positions the system such that all limit switches are triggered

  //move to central drill position
  horizontalMotor.VACTUAL(300000);
  horizontalMotor.shaft(1); // direction; may need to be switched depending on which side the limit switch is on
  horizontalSerial << horizontalMotor.VACTUAL() << endl;
  delay(300); // tune amount of time based on how long it takes to get to the correct position (yes, this is the reccomended way to do this. I know it's janky)
  horizontalMotor.VACTUAL(0);
  horizontalSerial << horizontalMotor.VACTUAL() << endl;

  drill(); //central hole

  horizontalMotor.VACTUAL(300000);
  horizontalMotor.shaft(1); // direction; may need to be switched depending on which side the limit switch is on
  horizontalSerial << horizontalMotor.VACTUAL() << endl;
  delay(100); // tune amount of time based on how long it takes to get to the correct position
  horizontalMotor.VACTUAL(0);
  horizontalSerial << horizontalMotor.VACTUAL() << endl;

  drill(); // first outside hole

  spinMotor.VACTUAL(300000);
  spinMotor.shaft(1); // direction; may need to be switched depending on which side the limit switch is on
  spinSerial << spinMotor.VACTUAL() << endl;
  delay(100); // tune amount of time based on how long it takes to get to the correct position
  spinMotor.VACTUAL(0);
  spinSerial << spinMotor.VACTUAL() << endl;
  
  drill(); // second outside hole

  spinMotor.VACTUAL(300000);
  spinMotor.shaft(1); // direction; may need to be switched depending on which side the limit switch is on
  spinSerial << spinMotor.VACTUAL() << endl;
  delay(100); // tune amount of time based on how long it takes to get to the correct position
  spinMotor.VACTUAL(0);
  spinSerial << spinMotor.VACTUAL() << endl;

  drill(); // third outside hole

  while(1); // infinite delay, in the real code this will be moving to the next state (lol)

}

void reset(){
  //run steppers to reset position, specically running until they hit the limit switches
  while (digitalRead(limitSwitchPins[0]) == LOW){ // may have to switch specifics here, the idea is "while the limit switch isn't triggered"
    spinMotor.VACTUAL(300000);
    spinMotor.shaft(0); // direction; may need to be switched depending on which side the limit switch is on
    spinSerial << spinMotor.VACTUAL() << endl;
  }
  spinMotor.VACTUAL(0);
  spinSerial << spinMotor.VACTUAL() << endl;

  while (digitalRead(limitSwitchPins[1]) == LOW){ // may have to switch specifics here, the idea is "while the limit switch isn't triggered"
    horizontalMotor.VACTUAL(300000);
    horizontalMotor.shaft(0); // direction; may need to be switched depending on which side the limit switch is on
    horizontalSerial << horizontalMotor.VACTUAL() << endl;
  }
  horizontalMotor.VACTUAL(0);
  horizontalSerial << horizontalMotor.VACTUAL() << endl;

  while (digitalRead(limitSwitchPins[2]) == LOW){ // may have to switch specifics here, the idea is "while the limit switch isn't triggered"
    verticalMotor.VACTUAL(300000);
    verticalMotor.shaft(0); // direction; may need to be switched depending on which side the limit switch is on
    verticalSerial << verticalMotor.VACTUAL() << endl;
  }
  verticalMotor.VACTUAL(0);
  verticalSerial << verticalMotor.VACTUAL() << endl;
}

void drill(){

  verticalMotor.VACTUAL(300000);
  verticalMotor.shaft(1); // direction; may need to be switched depending on which side the limit switch is on
  verticalSerial << verticalMotor.VACTUAL() << endl;

  //set DC motor PWM. Will have to figure out the correct PWM to keep DC and stepper synced

  delay(100); // how long does the drilling take?
  verticalMotor.VACTUAL(0);
  spinSerial << spinMotor.VACTUAL() << endl;

}

