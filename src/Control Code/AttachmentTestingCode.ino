/* =========================================================
   DRILL‑RIG TEST‑BENCH
   ---------------------------------------------------------
   One sketch that lets you exercise any single subsystem:
     • Stepper Z, R, Theta (jog forward/back)
     • DC motor spin
     • Limit‑switch home routine
     • Button debounce
   ---------------------------------------------------------
   HOW TO USE:
     1. Pick the test you want in loop():
          //testZJog();
          testRJog();
          //testThetaJog();
          //testZHome();
          //testRHome();
          //testThetaHome();
          //testDCMotor();
          //testButtonLED();
     2. Make sure ONLY ONE line is uncommented.
     3. Upload and watch Serial Monitor + hardware.
   ========================================================= */

#include <Arduino.h>

// ---------- Pin Map (unchanged) ----------
// Z axis
const int Z_EN   = 2,  Z_STEP = 3,  Z_DIR  = 4,  Z_LIM = 11;
// R axis
const int R_EN   = 5,  R_STEP = 6,  R_DIR  = 7,  R_LIM = 12;
// Theta axis
const int T_EN   = 8,  T_STEP = 9,  T_DIR  = 10, T_LIM = 13;
// DC motor
const int DC_EN  = 44, DC_IN1 = 22, DC_IN2 = 23;
// Button (momentary, to GND)
const int BTN_PIN = A0;
// Simple LED for button test (use built‑in LED)
const int LED_PIN = LED_BUILTIN;

// ---------- Global parameters ----------
const int    STEPS_PER_REV = 200;
const int    STEP_DELAY_US = 800;   // faster/slower jogging
const uint16_t JOG_REV     = 1;     // default jog = 1 rev
const uint32_t DEBOUNCE_MS = 50;

// ---------- Setup ----------
void setup()
{
  Serial.begin(115200);

  pinMode(Z_EN,OUTPUT); pinMode(Z_STEP,OUTPUT); pinMode(Z_DIR,OUTPUT);
  pinMode(R_EN,OUTPUT); pinMode(R_STEP,OUTPUT); pinMode(R_DIR,OUTPUT);
  pinMode(T_EN,OUTPUT); pinMode(T_STEP,OUTPUT); pinMode(T_DIR,OUTPUT);

  pinMode(Z_LIM,INPUT_PULLUP);
  pinMode(R_LIM,INPUT_PULLUP);
  pinMode(T_LIM,INPUT_PULLUP);

  pinMode(DC_EN,OUTPUT); pinMode(DC_IN1,OUTPUT); pinMode(DC_IN2,OUTPUT);

  pinMode(BTN_PIN,INPUT_PULLUP);
  pinMode(LED_PIN,OUTPUT);

  disableAll();
  Serial.println(F("\n--- Drill‑Rig Test‑Bench Ready ---"));
}

// =========================================================
// =====================   LOOP   ==========================
// =========================================================
void loop()
{
  //-------------------  PICK ONE TEST  -------------------//
  // Comment / uncomment to choose:

  testZJog();        // jog the Z stepper 1 rev CW then CCW
  //testRJog();        // jog the R stepper
  //testThetaJog();    // jog the Theta stepper
  //testZHome();       // home Z to its limit
  //testRHome();       // home R to its limit
  //testThetaHome();   // home Theta to its limit
  //testDCMotor();     // spin the DC motor fwd, pause, rev
  //testButtonLED();   // show button debounced on built‑in LED
}

// =========================================================
// =================  TEST ROUTINES  =======================
// =========================================================

// ---- Generic helpers ----
void singleMove(int en,int st,int dir,bool cw,int steps){
  digitalWrite(en,LOW);
  digitalWrite(dir,cw?HIGH:LOW);
  for(int i=0;i<steps;i++){
    digitalWrite(st,HIGH); delayMicroseconds(STEP_DELAY_US);
    digitalWrite(st,LOW);  delayMicroseconds(STEP_DELAY_US);
  }
  digitalWrite(en,HIGH);
}

void homeAxis(int en,int st,int dir,int lim,bool towardHi){
  Serial.println(F("Homing ..."));
  digitalWrite(en,LOW);
  digitalWrite(dir,towardHi?HIGH:LOW);
  while(digitalRead(lim)==HIGH){
    digitalWrite(st,HIGH); delayMicroseconds(STEP_DELAY_US);
    digitalWrite(st,LOW);  delayMicroseconds(STEP_DELAY_US);
  }
  digitalWrite(en,HIGH);
  Serial.println(F("Hit limit switch."));
}

void disableAll(){
  digitalWrite(Z_EN,HIGH); digitalWrite(R_EN,HIGH); digitalWrite(T_EN,HIGH);
  digitalWrite(DC_EN,LOW); digitalWrite(DC_IN1,LOW); digitalWrite(DC_IN2,LOW);
}

// ---- Stepper JOG tests ----
void testZJog(){
  Serial.println(F("Z jog +1 rev CW ..."));
  singleMove(Z_EN,Z_STEP,Z_DIR,true, STEPS_PER_REV*JOG_REV);
  delay(500);
  Serial.println(F("Z jog -1 rev CCW ..."));
  singleMove(Z_EN,Z_STEP,Z_DIR,false,STEPS_PER_REV*JOG_REV);
  delay(1000);
}

void testRJog(){
  Serial.println(F("R jog +1 rev CW ..."));
  singleMove(R_EN,R_STEP,R_DIR,true, STEPS_PER_REV*JOG_REV);
  delay(500);
  Serial.println(F("R jog -1 rev CCW ..."));
  singleMove(R_EN,R_STEP,R_DIR,false,STEPS_PER_REV*JOG_REV);
  delay(1000);
}

void testThetaJog(){
  Serial.println(F("Theta jog +1 rev CW ..."));
  singleMove(T_EN,T_STEP,T_DIR,true, STEPS_PER_REV*JOG_REV);
  delay(500);
  Serial.println(F("Theta jog -1 rev CCW ..."));
  singleMove(T_EN,T_STEP,T_DIR,false,STEPS_PER_REV*JOG_REV);
  delay(1000);
}

// ---- Limit‑switch home tests ----
void testZHome(){ homeAxis(Z_EN,Z_STEP,Z_DIR,Z_LIM,LOW); delay(1500);}
void testRHome(){ homeAxis(R_EN,R_STEP,R_DIR,R_LIM,LOW); delay(1500);}
void testThetaHome(){ homeAxis(T_EN,T_STEP,T_DIR,T_LIM,LOW); delay(1500);}

// ---- DC Motor test ----
void testDCMotor(){
  Serial.println(F("DC motor forward ..."));
  digitalWrite(DC_EN,HIGH); digitalWrite(DC_IN1,HIGH); digitalWrite(DC_IN2,LOW);
  delay(2000);

  Serial.println(F("Stop ..."));          // brake
  digitalWrite(DC_EN,LOW);  digitalWrite(DC_IN1,LOW); digitalWrite(DC_IN2,LOW);
  delay(1000);

  Serial.println(F("DC motor reverse ..."));
  digitalWrite(DC_EN,HIGH); digitalWrite(DC_IN1,LOW);  digitalWrite(DC_IN2,HIGH);
  delay(2000);

  Serial.println(F("Stop."));
  digitalWrite(DC_EN,LOW);  digitalWrite(DC_IN1,LOW);  digitalWrite(DC_IN2,LOW);
  delay(2000);
}

// ---- Button debounce + LED feedback ----
void testButtonLED(){
  static bool last = HIGH;
  static uint32_t tLast = 0;
  bool now = digitalRead(BTN_PIN);

  if(now!=last){ tLast = millis(); }

  if(millis()-tLast > DEBOUNCE_MS){
    digitalWrite(LED_PIN, now==LOW ? HIGH : LOW);  // LED ON when pressed
  }
  last = now;
}
