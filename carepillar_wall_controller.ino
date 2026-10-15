#include <Wire.h>
#include <MFRC522v2.h>
#include <MFRC522DriverI2C.h>

// =====================================================
// CAREPILLAR INTERACTIVE WALL PROTOTYPE
//
// Functions:
// - RFID identity authentication
// - RGB status feedback
// - Dual motor control
// - Alternating forward / backward wall movement
// =====================================================


// =====================================================
// SYSTEM PARAMETERS
// =====================================================

// Single movement duration
// 10000 ms = 10 seconds
const unsigned long RUN_TIME_MS = 10000;

// Normal motor speed
const int RUN_SPEED = 180;

// Short full-speed boost at startup
const unsigned long START_BOOST_MS = 300;


// =====================================================
// RFID
// =====================================================

const uint8_t RFID_ADDRESS = 0x28;

TwoWire &customI2C = Wire;
MFRC522DriverI2C driver{RFID_ADDRESS, customI2C};
MFRC522 mfrc522{driver};

// Authorized RFID card
// UID: 5B 52 89 A9
const byte allowedUID[4] = {
  0x5B, 0x52, 0x89, 0xA9
};


// =====================================================
// MOTOR C
// =====================================================

const int PWMC = 6;
const int CIN1 = 8;
const int CIN2 = 9;


// =====================================================
// MOTOR D
// =====================================================

const int PWMD = 3;
const int DIN1 = 4;
const int DIN2 = 5;


// =====================================================
// RGB LED
//
// Common-anode RGB:
// LOW  = ON
// HIGH = OFF
// =====================================================

const int RED_PIN   = A0;
const int GREEN_PIN = A1;
const int BLUE_PIN  = A2;


// =====================================================
// MOVEMENT STATE
//
// true  = next movement is forward
// false = next movement is backward
// =====================================================

bool nextMoveForward = true;


// =====================================================
// RGB FUNCTIONS
// =====================================================

void lightOff() {

  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);
}


void greenLight() {

  lightOff();

  digitalWrite(GREEN_PIN, LOW);
}


void redLight() {

  lightOff();

  digitalWrite(RED_PIN, LOW);
}


// =====================================================
// MOTOR FUNCTIONS
// =====================================================

// Stop both motors
void stopMotors() {

  analogWrite(PWMC, 0);
  analogWrite(PWMD, 0);

  digitalWrite(CIN1, LOW);
  digitalWrite(CIN2, LOW);

  digitalWrite(DIN1, LOW);
  digitalWrite(DIN2, LOW);
}


// Forward movement
void moveForward(int speedValue) {

  // Motor C
  digitalWrite(CIN1, HIGH);
  digitalWrite(CIN2, LOW);

  // Motor D is installed in mirrored orientation
  digitalWrite(DIN1, LOW);
  digitalWrite(DIN2, HIGH);

  analogWrite(PWMC, speedValue);
  analogWrite(PWMD, speedValue);
}


// Backward movement
void moveBackward(int speedValue) {

  // Motor C
  digitalWrite(CIN1, LOW);
  digitalWrite(CIN2, HIGH);

  // Motor D
  digitalWrite(DIN1, HIGH);
  digitalWrite(DIN2, LOW);

  analogWrite(PWMC, speedValue);
  analogWrite(PWMD, speedValue);
}


// =====================================================
// RFID AUTHENTICATION
// =====================================================

bool isAuthorizedCard() {

  // Current authorized card uses a 4-byte UID
  if (mfrc522.uid.size != 4) {
    return false;
  }

  for (byte i = 0; i < 4; i++) {

    if (mfrc522.uid.uidByte[i] != allowedUID[i]) {
      return false;
    }
  }

  return true;
}


// =====================================================
// PRINT RFID UID
// =====================================================

void printUID() {

  Serial.print("Card UID: ");

  for (byte i = 0; i < mfrc522.uid.size; i++) {

    if (mfrc522.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(mfrc522.uid.uidByte[i], HEX);

    if (i < mfrc522.uid.size - 1) {
      Serial.print(" ");
    }
  }

  Serial.println();
}


// =====================================================
// WALL MOVEMENT SEQUENCE
// =====================================================

void runWallMovement(bool forward) {

  if (forward) {

    Serial.println("Direction: FORWARD");

    // Full-speed startup boost
    moveForward(255);
    delay(START_BOOST_MS);

    // Normal movement
    moveForward(RUN_SPEED);
    delay(RUN_TIME_MS);
  }

  else {

    Serial.println("Direction: BACKWARD");

    // Full-speed startup boost
    moveBackward(255);
    delay(START_BOOST_MS);

    // Normal movement
    moveBackward(RUN_SPEED);
    delay(RUN_TIME_MS);
  }

  stopMotors();
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);


  // ---------------------------------------------------
  // Motor initialization
  // ---------------------------------------------------

  pinMode(PWMC, OUTPUT);
  pinMode(CIN1, OUTPUT);
  pinMode(CIN2, OUTPUT);

  pinMode(PWMD, OUTPUT);
  pinMode(DIN1, OUTPUT);
  pinMode(DIN2, OUTPUT);

  stopMotors();


  // ---------------------------------------------------
  // RGB initialization
  // ---------------------------------------------------

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Standby = LED off
  lightOff();


  // ---------------------------------------------------
  // RFID initialization
  // ---------------------------------------------------

  mfrc522.PCD_Init();

  delay(100);


  Serial.println();
  Serial.println("================================");
  Serial.println("Carepillar Wall Control System");
  Serial.println("================================");
  Serial.println("System ready.");
  Serial.println("Next movement: FORWARD");
  Serial.println("Waiting for RFID card...");
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  // Wait for a new RFID card
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Read RFID card
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }


  printUID();


  // ===================================================
  // AUTHORIZED CARD
  // ===================================================

  if (isAuthorizedCard()) {

    Serial.println("ACCESS GRANTED");

    // Green feedback
    greenLight();

    // Short pause so the feedback is visible
    delay(700);


    Serial.println("Wall moving...");


    // Run in the direction stored in the current state
    runWallMovement(nextMoveForward);


    Serial.println("Wall stopped.");


    // Toggle direction for the next authorized scan
    nextMoveForward = !nextMoveForward;


    if (nextMoveForward) {
      Serial.println("Next movement: FORWARD");
    }

    else {
      Serial.println("Next movement: BACKWARD");
    }


    // Keep green light briefly after movement
    delay(1000);


    // Return to standby
    lightOff();

    Serial.println("Remove card and scan again.");
    Serial.println();
  }


  // ===================================================
  // UNAUTHORIZED CARD
  // ===================================================

  else {

    Serial.println("ACCESS DENIED");

    // Ensure wall never moves
    stopMotors();


    // Red status feedback
    redLight();

    delay(2000);


    // Return to standby
    lightOff();

    Serial.println("Waiting for RFID card...");
    Serial.println();
  }


  // End communication with current RFID card
  mfrc522.PICC_HaltA();

  delay(800);
}
