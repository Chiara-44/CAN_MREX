/**
 * CAN MREX main (Template) file 
 *
 * File:            main.ino
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    5/08/2025
 * Last Modified:   9/09/2025
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

uint8_t nodeID = 1;  // Change this to set your device's node ID 

// --- Pin Definitions ---
#define TX_GPIO_NUM GPIO_NUM_5 // Set GPIO pin for CAN Transmit
#define RX_GPIO_NUM GPIO_NUM_4 // Set GPIO pins for CAN Receive

// --- OD definitions ---
uint16_t speed = 0;
uint8_t brake = 0;


//OPTIONAL: timing for a non blocking function occuring every two seconds
unsigned long previousMillis = 0;
const long interval = 2000; // 2 seconds

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
  registerODEntry(0x2000, 0x01, 2, sizeof(uint16_t), &speed);
  registerODEntry(0x2001, 0x00, 2, sizeof(uint8_t), &brake);


  configureTPDO(0, 0x180 + nodeID, 255, 100, 1000);  // COB-ID, transType, inhibit, event
  
  PdoMapEntry tpdoEntries[] = {
      {0x2000, 0x01, 16},  // Example: index 0x2000, subindex 1, 16 bits
      {0x2001, 0x00, 8}    // Example: index 0x2001, subindex 0, 8 bits
    };
  mapTPDO(0, tpdoEntries, 2); //TPDO 1, entries, num entries
 

  // User code Setup end ------------------------------------------------------


}

void loop() {
  handleCAN(nodeID); // Handles all incoming can messages
  //User Code begin loop() ----------------------------------------------------
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial.print("Speed: ");
    Serial.println(speed);
    Serial.print("Brake: ");
    Serial.println(brake);
    speed += 2;
    brake += 1;
    
  }

  //User code end loop() --------------------------------------------------------
}