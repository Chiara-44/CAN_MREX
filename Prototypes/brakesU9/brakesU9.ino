/**
 * CAN MREX Brake Prototype v1 CMU9
 * Author:          Chiara Gillam
 * Date Created:    10/09/2025
 * Last Modified:   10/09/2025
 * Version:         1.1.3
 *
 */

#include "driver/twai.h"
#include "CM_Handler.h"
#include "CM_Transmit.h"
#include "CM_ObjectDictionary.h"
#include "CM_PDO.h"
#include "CM_Config.h"


// User code begin: ------------------------------------------------------

uint8_t nodeID = 2;  // Change this to set your device's node ID 

// --- Pin Definitions ---
//CAN bus
#define TX_GPIO_NUM GPIO_NUM_5 // Set GPIO pin for CAN Transmit
#define RX_GPIO_NUM GPIO_NUM_4 // Set GPIO pins for CAN Receive

// Brake control
const int brakeControlPin = 18;  // Relay

// Brake wear/release switch **REMEMBER TO CHANGE code to work with switches instead of pushbuttons once brakes arrive
const int brakeOKPin = 23;       // Brown wire (1–4)
const int brakeWornPin = 19;     // Blue wire (1–2)

// Hand-release switch (simulated with push buttons)
const int handReleaseNC = 21;    // Grey wire (Normally Closed)
const int handReleaseNO = 22;    // Blue wire (Normally Open)

// State variables
volatile bool brakeEngaged = true; //I changed this so that it starts as engaged (safety)
volatile bool brakeOK = true;
volatile bool brakeWorn = false;
volatile bool handNormal = true;
volatile bool handReleased = false;

// --- OD definitions ---
uint8_t brakeState = 1; //This is the variable that will be updated by the can bus, 0 OFF, 1 ON


//OPTIONAL: timing for a non blocking function occuring every two seconds
unsigned long brakeTimPrev = 0;
const long brakeInterval = 100; // update brake every 100ms (can change if not responsive enough) (!!!Use esp_timer_get_time() for more precision if changing)

// --- Interrupt Service Routines ---

void IRAM_ATTR updateBrakeOK() {
  brakeOK = digitalRead(brakeOKPin) == LOW;
}

void IRAM_ATTR updateBrakeWorn() {
  brakeWorn = digitalRead(brakeWornPin) == LOW; 
}

void IRAM_ATTR updateHandNormal() {
  handNormal = digitalRead(handReleaseNC) == LOW;
}

void IRAM_ATTR updateHandReleased() {
  handReleased = digitalRead(handReleaseNO) == LOW;
}

// User code end ---------------------------------------------------------


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("CAN MREX intialising over (TWAI)");

  //Initialize CANMREX protocol
  initCANMREX(TX_GPIO_NUM, RX_GPIO_NUM);

  //Initializes all TPDOs and RPDOs as disabled and clears runtime state
  initDefaultPDOs(nodeID);

  //Setup OD with default entries
  initDefaultOD();

  // User code Setup Begin: -------------------------------------------------
  // --- Register OD entries ---
  registerODEntry(0x3012, 0x01, 2, sizeof(uint8_t), &brakeState);

  // --- Configure and map RPDOs ---
  configureRPDO(0, 0x180 + 3, 255, 0);  // Recieve from node 3 (main controller)

  PdoMapEntry tpdoEntries[] = {
      {0x3012, 0x01, 8}    // Example: index 0x3012, subindex 1, 8 bits
      };
  mapRPDO(0, tpdoEntries, 1);

  // Outputs
  pinMode(brakeControlPin, OUTPUT);

  // Inputs with pull-ups
  // pinMode(brakeOKPin, INPUT_PULLUP);
  // pinMode(brakeWornPin, INPUT_PULLUP);
  // pinMode(handReleaseNC, INPUT_PULLUP);
  // pinMode(handReleaseNO, INPUT_PULLUP);

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(brakeOKPin), updateBrakeOK, CHANGE);
  attachInterrupt(digitalPinToInterrupt(brakeWornPin), updateBrakeWorn, CHANGE);
  attachInterrupt(digitalPinToInterrupt(handReleaseNC), updateHandNormal, CHANGE);
  attachInterrupt(digitalPinToInterrupt(handReleaseNO), updateHandReleased, CHANGE);
 

  // User code Setup end ------------------------------------------------------


}

void loop() {
  handleCAN(nodeID); // Handles all incoming can messages
  //User Code begin loop() ----------------------------------------------------

  unsigned long currentMillis = millis();

  //Brake logic (runs every 100ms to ensure not running too often)
  if (currentMillis - brakeTimPrev >= brakeInterval) { 
    brakeTimPrev = currentMillis;

    Serial.print("Brake: ");
    Serial.println(brakeState);  //Technically blocking so we won't have Serial prints in our code (don't need it in final product anyway) 

    if (brakeState >= 1 && handNormal && !brakeWorn) {
      digitalWrite(brakeControlPin, HIGH);  // Engage brake
      Serial.println("Brake is on.");
    } else {
      digitalWrite(brakeControlPin, LOW);   // Release brake
      Serial.println("Brake is off.");
    }
  }

  //User code end loop() --------------------------------------------------------
}