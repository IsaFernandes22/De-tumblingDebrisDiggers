const int limitSwitchPins[3] = {0,0,0}; // order: spinning, horizontal, vertical

const int horizontalMotorPins[3] = {2,3,4}; //enable, dir, step. Any pins on the board work
const int verticalMotorPins[3] = {0,0,0};
const int spinMotorPins[3] = {0,0,0};
const int drillMotorPins[2] = {0,0}; // dir, volt. volt must be PWM enabled

const int stepperSpeed = 160; // inverted. A smaller number means faster

// likely need a seperate speed variable for the vertical movement, 
// so that we can adjust the speed that the drill goes up and down,
// but ¯\_(ツ)_/¯. Syncing that will be a task for later


void drill();
void reset();

void setup() {

  for(int i = 0; i < 3; i++){
    pinMode(limitSwitchPins[i], INPUT);
    pinMode(horizontalMotorPins[i], OUTPUT);
    pinMode(verticalMotorPins[i], OUTPUT);
    pinMode(spinMotorPins[i], OUTPUT);
  }

  pinMode(drillMotorPins[0], OUTPUT);
  pinMode(drillMotorPins[1], OUTPUT);

  digitalWrite(horizontalMotorPins[0], LOW); // this enables all steppers
  digitalWrite(verticalMotorPins[0], LOW);
  digitalWrite(verticalMotorPins[0], LOW);

}

void loop() {
  
  reset(); // positions the system such that all limit switches are triggered

  // Move to central drill position:

  int time1 = 10000; // tune this value! based on how long it takes to move drill to correct spot
  
  digitalWrite(horizontalMotorPins[1], LOW); // idk man I just guessed a direction
  
  for (int t = 0; t < time1; t++){
    digitalWrite(horizontalMotorPins[2], HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(horizontalMotorPins[2], LOW);
    delayMicroseconds(stepperSpeed);
  }

  drill(); //central hole

  int time2 = 10000; // tune this value! based on how long it takes to move drill to correct spot
  
  digitalWrite(horizontalMotorPins[1], HIGH); // idk man I just guessed a direction
  
  for (int t = 0; t < time1; t++){
    digitalWrite(horizontalMotorPins[2], HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(horizontalMotorPins[2], LOW);
    delayMicroseconds(stepperSpeed);
  }

  drill(); // first outside hole

  int time3 = 10000; // tune this value! based on how long it takes to move drill to correct spot
  
  digitalWrite(spinMotorPins[1], LOW); // idk man I just guessed a direction
  
  for (int t = 0; t < time1; t++){
    digitalWrite(spinMotorPins[2], HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(spinMotorPins[2], LOW);
    delayMicroseconds(stepperSpeed);
  }
  
  drill(); // second outside hole

  int time4 = 10000; // tune this value! based on how long it takes to move drill to correct spot
  
  digitalWrite(spinMotorPins[1], LOW); // idk man I just guessed a direction
  
  for (int t = 0; t < time1; t++){
    digitalWrite(spinMotorPins[2], HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(spinMotorPins[2], LOW);
    delayMicroseconds(stepperSpeed);
  }

  drill(); // third outside hole

  while(1); // infinite delay, in the real code this will be moving to the next state (lol)

}

void reset(){
  //run steppers to reset position, specically running until they hit the limit switches

  digitalWrite(horizontalMotorPins[1], HIGH); // idk man I just guessed a direction
  digitalWrite(verticalMotorPins[1], HIGH);
  digitalWrite(spinMotorPins[1], HIGH);

   while (digitalRead(limitSwitchPins[0]) == LOW) {
    digitalWrite(spinMotorPins[2], HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(spinMotorPins[2], LOW);
    delayMicroseconds(stepperSpeed);
  }
  
  while (digitalRead(limitSwitchPins[1]) == LOW) {
    digitalWrite(horizontalMotorPins[2], HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(horizontalMotorPins[2], LOW);
    delayMicroseconds(stepperSpeed);
  }

  while (digitalRead(limitSwitchPins[2]) == LOW) {
    digitalWrite(verticalMotorPins[2], HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(verticalMotorPins[2], LOW);
    delayMicroseconds(stepperSpeed);
  }
}

void drill(){

  //start the DC motor spinning
  digitalWrite(drillMotorPins[0], HIGH); // idk high or low
  analogWrite(drillMotorPins[1], 200); // change the PWM to adjust the speed. Will have to sync this number with how fast it moves up and down???

  //move the drill "down"
  digitalWrite(verticalMotorPins[1], HIGH);
  int drillTime = 10000;

  for (int t = 0; t < drillTime; t++){
    digitalWrite(verticalMotorPins[2], HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(verticalMotorPins[2], LOW);
    delayMicroseconds(stepperSpeed);
  }

  //stop the drill motor
  analogWrite(drillMotorPins[1], 0);

  //move the drill "up"
  digitalWrite(verticalMotorPins[1], LOW);

  for (int t = 0; t < drillTime; t++){
    digitalWrite(verticalMotorPins[2], HIGH);
    delayMicroseconds(stepperSpeed);
    digitalWrite(verticalMotorPins[2], LOW);
    delayMicroseconds(stepperSpeed);
  }

}

