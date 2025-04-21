// @author: Isabella Fernandes
// ---------- Pin Assignments ----------
// Z axis
const int Z_EN   = 2;
const int Z_STEP = 3;
const int Z_DIR  = 4;
const int Z_LIM  = 11;

// R axis
const int R_EN   = 5;
const int R_STEP = 6;
const int R_DIR  = 7;
const int R_LIM  = 12;

// Theta axis
const int T_EN   = 8;
const int T_STEP = 9;
const int T_DIR  = 10;
const int T_LIM  = 13;

// DC motor (drill spindle)
const int DC_EN  = 44;
const int DC_IN1 = 22;
const int DC_IN2 = 23;

// Start‑button
const int BTN_PIN = A0;

// ---------- Motion parameters ----------
const int    STEPS_PER_REV = 200;     // TODO: change accordingly, steps for one full rotation
const int    STEP_DELAY_US = 1000;    // delay between steps in microseconds
const uint32_t DEBOUNCE_MS = 50;

// drill‑spin time after every Z “down” move (how long the drill spins)
const uint16_t DRILL_TIME_MS = 2000; 

// ---------- State variables ----------
bool sequenceRunning = false;
bool lastBtnState    = HIGH;
uint32_t lastBounce  = 0;

// ---------- Forward declarations ----------
void runSequence();                       // does the 13 steps
void homeAxis(int en,int st,int dir,int lim,bool towardHi);
void moveSteps(int en,int st,int dir,float rot,bool cw);
void drillZ(float rot,bool down);
void disableAll();

// ─────────── SETUP ───────────
void setup() {
  // Stepper & limit pins
  pinMode(Z_EN,OUTPUT); pinMode(Z_STEP,OUTPUT); pinMode(Z_DIR,OUTPUT);
  pinMode(R_EN,OUTPUT); pinMode(R_STEP,OUTPUT); pinMode(R_DIR,OUTPUT);
  pinMode(T_EN,OUTPUT); pinMode(T_STEP,OUTPUT); pinMode(T_DIR,OUTPUT);

  pinMode(Z_LIM,INPUT_PULLUP);
  pinMode(R_LIM,INPUT_PULLUP);
  pinMode(T_LIM,INPUT_PULLUP);

  // DC motor
  pinMode(DC_EN,OUTPUT); pinMode(DC_IN1,OUTPUT); pinMode(DC_IN2,OUTPUT);

  // Button
  pinMode(BTN_PIN,INPUT_PULLUP);

  disableAll();                 // keep everything off until commanded

  sequenceRunning = true;
  runSequence();                      // do the whole drill routine
  sequenceRunning = false;
  disableAll();
}

// ─────────── LOOP ───────────
void loop() {
 
}

// ─────────── MAIN SEQUENCE ───────────
void runSequence() {
  // 1) Home all axes (counter‑clockwise toward limits). Run till it hits limit switch.
  homeAxis(Z_EN,Z_STEP,Z_DIR,Z_LIM,LOW);
  homeAxis(R_EN,R_STEP,R_DIR,R_LIM,LOW);
  homeAxis(T_EN,T_STEP,T_DIR,T_LIM,LOW);

  // 2‑13) Operations spelled out one‑by‑one
  moveSteps(R_EN,R_STEP,R_DIR,0.709965667,true);      // step 2
  drillZ(1.352856915,true);                           // step 3
  drillZ(1.352856915,false);                          // step 4
  moveSteps(R_EN,R_STEP,R_DIR,0.709965667,false);     // step 5
  drillZ(1.352856915,true);                           // step 6
  drillZ(1.352856915,false);                          // step 7
  moveSteps(T_EN,T_STEP,T_DIR,2.4,true);      // step 8
  drillZ(1.352856915,true);                           // step 9
  drillZ(1.352856915,false);                          // step 10
  moveSteps(T_EN,T_STEP,T_DIR,0.246392157,true);      // step 11
  drillZ(1.352856915,true);                           // step 12
  drillZ(1.352856915,false);                          // step 13
}

// ─────────── HELPER FUNCTIONS ───────────
void moveSteps(int en,int st,int dir,float rotations,bool clockwise){
  uint32_t steps = rotations * STEPS_PER_REV + 0.5;
  digitalWrite(en,LOW);                    // enable driver
  digitalWrite(dir,clockwise?HIGH:LOW);
  for(uint32_t i=0;i<steps;i++){
    digitalWrite(st,HIGH);  delayMicroseconds(STEP_DELAY_US);
    digitalWrite(st,LOW);   delayMicroseconds(STEP_DELAY_US);
  }
  digitalWrite(en,HIGH);                   // disable
}

void homeAxis(int en,int st,int dir,int lim,bool towardHi){
  digitalWrite(en,LOW);
  digitalWrite(dir,towardHi?HIGH:LOW);
  while(digitalRead(lim)==HIGH){
    digitalWrite(st,HIGH); delayMicroseconds(STEP_DELAY_US);
    digitalWrite(st,LOW);  delayMicroseconds(STEP_DELAY_US);
  }
  digitalWrite(en,HIGH);
}

void drillZ(float rotations,bool down){
  // spin drill
  digitalWrite(DC_EN,HIGH);
  digitalWrite(DC_IN1,HIGH);  digitalWrite(DC_IN2,LOW);

  moveSteps(Z_EN,Z_STEP,Z_DIR,rotations,down);   // Z move
  delay(DRILL_TIME_MS);                          // keep spinning

  // stop drill
  digitalWrite(DC_EN,LOW);
  digitalWrite(DC_IN1,LOW); digitalWrite(DC_IN2,LOW);
}

void disableAll(){
  digitalWrite(Z_EN,HIGH);
  digitalWrite(R_EN,HIGH);
  digitalWrite(T_EN,HIGH);
  digitalWrite(DC_EN,LOW);
  digitalWrite(DC_IN1,LOW); digitalWrite(DC_IN2,LOW);
}
