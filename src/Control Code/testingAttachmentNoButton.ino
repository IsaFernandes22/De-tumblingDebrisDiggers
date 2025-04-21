// ---------------------------------------------------------
// Drill‑rig sequence — runs ONCE automatically at start‑up
// @author: Isabella Fernandes
// ---------------------------------------------------------

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

// ---------- Motion parameters ----------
const int    STEPS_PER_REV = 200;   // adjust for your motor/driver
const int    STEP_DELAY_US = 1000;  // speed control (µs per step)
const uint16_t DRILL_TIME_MS = 2000; // spin time after each Z‑down move

// ---------- Forward declarations ----------
void runSequence();
void homeAxis(int en,int st,int dir,int lim,bool towardHi);
void moveSteps(int en,int st,int dir,float rot,bool cw);
void drillZ(float rot,bool down);
void disableAll();

// -------------------- SETUP --------------------
void setup() {
  // Configure I/O
  pinMode(Z_EN,OUTPUT); pinMode(Z_STEP,OUTPUT); pinMode(Z_DIR,OUTPUT);
  pinMode(R_EN,OUTPUT); pinMode(R_STEP,OUTPUT); pinMode(R_DIR,OUTPUT);
  pinMode(T_EN,OUTPUT); pinMode(T_STEP,OUTPUT); pinMode(T_DIR,OUTPUT);

  pinMode(Z_LIM,INPUT_PULLUP);
  pinMode(R_LIM,INPUT_PULLUP);
  pinMode(T_LIM,INPUT_PULLUP);

  pinMode(DC_EN,OUTPUT); pinMode(DC_IN1,OUTPUT); pinMode(DC_IN2,OUTPUT);

  disableAll();      // ensure everything starts off

  runSequence();     // <‑‑ run the full drill routine once
  disableAll();      // power down motors when done
}

// -------------------- LOOP --------------------
void loop() {
  // nothing here — sequence already ran once in setup()
}

// ===============   MAIN SEQUENCE   =================
void runSequence() {
  // 1) Home all axes (CCW toward limits)
  homeAxis(Z_EN,Z_STEP,Z_DIR,Z_LIM,LOW);
  homeAxis(R_EN,R_STEP,R_DIR,R_LIM,LOW);
  homeAxis(T_EN,T_STEP,T_DIR,T_LIM,LOW);

  // 2‑13) Drill routine
  moveSteps(R_EN,R_STEP,R_DIR,0.709965667,true);      // 2
  drillZ(1.352856915,true);                           // 3  (down)
  drillZ(1.352856915,false);                          // 4  (up)
  moveSteps(R_EN,R_STEP,R_DIR,0.709965667,false);     // 5
  drillZ(1.352856915,true);                           // 6
  drillZ(1.352856915,false);                          // 7
  moveSteps(T_EN,T_STEP,T_DIR,2.4,true);              // 8  (Theta CW)
  drillZ(1.352856915,true);                           // 9
  drillZ(1.352856915,false);                          // 10
  moveSteps(T_EN,T_STEP,T_DIR,2.4,true);              // 11
  drillZ(1.352856915,true);                           // 12
  drillZ(1.352856915,false);                          // 13
}

// ===============   HELPER FUNCTIONS   ===============
void moveSteps(int en,int st,int dir,float rotations,bool clockwise){
  uint32_t steps = rotations * STEPS_PER_REV + 0.5;
  digitalWrite(en,LOW);                    // enable driver
  digitalWrite(dir,clockwise ? HIGH : LOW);
  for(uint32_t i = 0; i < steps; i++){
    digitalWrite(st,HIGH); delayMicroseconds(STEP_DELAY_US);
    digitalWrite(st,LOW);  delayMicroseconds(STEP_DELAY_US);
  }
  digitalWrite(en,HIGH);                   // disable driver
}

void homeAxis(int en,int st,int dir,int lim,bool towardHi){
  digitalWrite(en,LOW);
  digitalWrite(dir,towardHi ? HIGH : LOW);
  while(digitalRead(lim) == HIGH){
    digitalWrite(st,HIGH); delayMicroseconds(STEP_DELAY_US);
    digitalWrite(st,LOW);  delayMicroseconds(STEP_DELAY_US);
  }
  digitalWrite(en,HIGH);
}

void drillZ(float rotations,bool down){
  // Start drill spindle (DC motor)
  //digitalWrite(DC_EN,HIGH);
  //digitalWrite(DC_IN1,HIGH);  // forward
  //digitalWrite(DC_IN2,LOW);

  // ----- start spindle (always CW) -----
  digitalWrite(DC_IN1, HIGH);   // keep direction pins fixed
  digitalWrite(DC_IN2, LOW);
  digitalWrite(DC_EN,  HIGH);   // enable driver -> motor spins CW


  // Move Z axis
  moveSteps(Z_EN,Z_STEP,Z_DIR,rotations,down);
  delay(DRILL_TIME_MS);       // keep spinning while drilling

  // Stop drill spindle
  //digitalWrite(DC_EN,LOW);
  //digitalWrite(DC_IN1,LOW);
  //digitalWrite(DC_IN2,LOW);
   // ----- stop spindle -----
  digitalWrite(DC_EN,  LOW);    // disable driver, motor coasts/brakes
  // leave IN1/IN2 as‑is so direction never changes
}

void disableAll(){
  digitalWrite(Z_EN,HIGH);
  digitalWrite(R_EN,HIGH);
  digitalWrite(T_EN,HIGH);
  digitalWrite(DC_EN,LOW);
  digitalWrite(DC_IN1,LOW);
  digitalWrite(DC_IN2,LOW);
}
