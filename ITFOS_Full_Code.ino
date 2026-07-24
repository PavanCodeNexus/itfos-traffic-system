/*
 ============================================================
  ITFOS — Intelligent Traffic Flow Optimization System
  Dayananda Sagar Academy of Technology and Management
  Engineering Exploration Course — BEEXC208
  Academic Year: 2025-26

  Team:
    Rahul Seervi    (1DT25CS238)
    Prashank S C    (1DT25CS224)
    Pratwik Patil   (1DT25CS227)
    Pavan B C       (1DT25CS206)

  Guide: Dr. Srilatha Y
         Associate Professor, Dept. of Physics, DSATM
 ============================================================
*/

// ============================================================
//  PIN DEFINITIONS — ULTRASONIC SENSORS
// ============================================================
const int TRIG1 = A0;
const int ECHO1 = A1;

const int TRIG2 = A2;
const int ECHO2 = A3;

const int TRIG3 = A4;
const int ECHO3 = A5;

// ============================================================
//  PIN DEFINITIONS — RGB LED TRAFFIC SIGNALS
// ============================================================
const int R1 = 2;   // Road 1 - Red
const int G1 = 3;   // Road 1 - Green
const int BL1 = 4;  // Road 1 - Blue (unused, kept LOW)

const int R2 = 5;   // Road 2 - Red
const int G2 = 6;   // Road 2 - Green
const int BL2 = 7;  // Road 2 - Blue (unused, kept LOW)

const int R3 = 8;   // Road 3 - Red
const int G3 = 9;   // Road 3 - Green
const int BL3 = 10; // Road 3 - Blue (unused, kept LOW)

// ============================================================
//  CONSTANTS
// ============================================================
const float SOUND_SPEED     = 0.034;  // cm per microsecond
const int   GREEN_DURATION  = 5000;   // Green signal ON time (ms)
const int   YELLOW_DURATION = 1000;   // Yellow transition time (ms)
const int   MIN_DISTANCE    = 2;      // Sensor minimum valid range (cm)
const int   MAX_DISTANCE    = 400;    // Sensor maximum valid range (cm)
const int   NO_VEHICLE      = 999;    // Value returned when no echo detected

// ============================================================
//  SETUP — runs once when Arduino powers on
// ============================================================
void setup() {
  Serial.begin(9600);

  // Sensor pins
  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);

  // LED pins - FIXED B1/B2/B3 to BL1/BL2/BL3
  pinMode(R1, OUTPUT); pinMode(G1, OUTPUT); pinMode(BL1, OUTPUT);
  pinMode(R2, OUTPUT); pinMode(G2, OUTPUT); pinMode(BL2, OUTPUT);
  pinMode(R3, OUTPUT); pinMode(G3, OUTPUT); pinMode(BL3, OUTPUT);

  startupTest();

  // All roads start RED
  setRed(1); setRed(2); setRed(3);

  Serial.println("ITFOS_READY");
  Serial.println("============================================");
  Serial.println("  ITFOS - Intelligent Traffic Flow System");
  Serial.println("  DSATM Bangalore - BEEXC208 - 2025-26");
  Serial.println("============================================");
}

// ============================================================
//  MAIN LOOP — runs continuously
// ============================================================
void loop() {

  // Step 1: Read distance from all 3 sensors
  float d1 = getDistance(TRIG1, ECHO1);
  float d2 = getDistance(TRIG2, ECHO2);
  float d3 = getDistance(TRIG3, ECHO3);

  // Step 2: Print in simple "ROADx:value" format
  Serial.print("ROAD1:"); Serial.println(d1);
  Serial.print("ROAD2:"); Serial.println(d2);
  Serial.print("ROAD3:"); Serial.println(d3);

  // Step 3: Find the minimum distance (nearest vehicle = highest priority)
  float minDist = min(d1, min(d2, d3));

  // Step 4: Assign GREEN to the road with minimum distance
  if (minDist == NO_VEHICLE) {
    Serial.println("STATUS:No vehicles detected. Road 1 default GREEN.");
    giveGreen(1);

  } else if (d1 <= d2 && d1 <= d3) {
    Serial.println("PRIORITY:ROAD1");
    giveGreen(1);

  } else if (d2 <= d1 && d2 <= d3) {
    Serial.println("PRIORITY:ROAD2");
    giveGreen(2);

  } else {
    Serial.println("PRIORITY:ROAD3");
    giveGreen(3);
  }
}

// ============================================================
//  FUNCTION: Get Distance from HC-SR04 Sensor
// ============================================================
float getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30 ms timeout

  if (duration == 0) {
    return NO_VEHICLE;
  }

  // Distance = (Time x Speed of Sound) / 2
  float distance = (duration * SOUND_SPEED) / 2.0;

  if (distance < MIN_DISTANCE || distance > MAX_DISTANCE) {
    return NO_VEHICLE;
  }

  return distance;
}

// ============================================================
//  FUNCTION: Give GREEN to one road, RED to the other two,
//  with a short YELLOW transition first.
// ============================================================
void giveGreen(int road) {

  // Yellow transition on all roads
  setYellow(1); setYellow(2); setYellow(3);
  delay(YELLOW_DURATION);

  // Assign GREEN to priority road, RED to the rest
  if (road == 1) {
    setGreen(1); setRed(2); setRed(3);
  } else if (road == 2) {
    setRed(1); setGreen(2); setRed(3);
  } else {
    setRed(1); setRed(2); setGreen(3);
  }

  // Hold green for the defined duration, but keep sampling
  // sensors during this time so Serial output stays live
  unsigned long startTime = millis();
  while (millis() - startTime < GREEN_DURATION) {
    delay(50);
  }
}

// ============================================================
//  SIGNAL COLOR FUNCTIONS - FIXED B1/B2/B3 to BL1/BL2/BL3
// ============================================================
void setRed(int road) {
  if      (road == 1) { digitalWrite(R1, HIGH); digitalWrite(G1, LOW);  digitalWrite(BL1, LOW); }
  else if (road == 2) { digitalWrite(R2, HIGH); digitalWrite(G2, LOW);  digitalWrite(BL2, LOW); }
  else                { digitalWrite(R3, HIGH); digitalWrite(G3, LOW);  digitalWrite(BL3, LOW); }
}

void setGreen(int road) {
  if      (road == 1) { digitalWrite(R1, LOW);  digitalWrite(G1, HIGH); digitalWrite(BL1, LOW); }
  else if (road == 2) { digitalWrite(R2, LOW);  digitalWrite(G2, HIGH); digitalWrite(BL2, LOW); }
  else                { digitalWrite(R3, LOW);  digitalWrite(G3, HIGH); digitalWrite(BL3, LOW); }
}

void setYellow(int road) {
  if      (road == 1) { digitalWrite(R1, HIGH); digitalWrite(G1, HIGH); digitalWrite(BL1, LOW); }
  else if (road == 2) { digitalWrite(R2, HIGH); digitalWrite(G2, HIGH); digitalWrite(BL2, LOW); }
  else                { digitalWrite(R3, HIGH); digitalWrite(G3, HIGH); digitalWrite(BL3, LOW); }
}

void setOff(int road) {
  if      (road == 1) { digitalWrite(R1, LOW); digitalWrite(G1, LOW); digitalWrite(BL1, LOW); }
  else if (road == 2) { digitalWrite(R2, LOW); digitalWrite(G2, LOW); digitalWrite(BL2, LOW); }
  else                { digitalWrite(R3, LOW); digitalWrite(G3, LOW); digitalWrite(BL3, LOW); }
}

// ============================================================
//  FUNCTION: Startup LED Test
//  Flashes all LEDs in sequence: RED -> GREEN -> YELLOW -> OFF
// ============================================================
void startupTest() {
  setRed(1); setRed(2); setRed(3);
  delay(400);

  setGreen(1); setGreen(2); setGreen(3);
  delay(400);

  setYellow(1); setYellow(2); setYellow(3);
  delay(400);

  setOff(1); setOff(2); setOff(3);
  delay(200);
}