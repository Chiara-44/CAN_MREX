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



//OPTIONAL: timing for a non blocking function occuring every two seconds
unsigned long previousMillis = 0;
const long interval = 2000; // 2 seconds
uint8_t mode = 0;
uint32_t heartbeatNode2;

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
  
 

  // User code Setup end ------------------------------------------------------


}

void loop() {
  handleCAN(nodeID); // Handles all incoming can messages
  //User Code begin loop() ----------------------------------------------------
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Write mode to node 2 (Speed in the dictionary)
    executeSDOWrite(nodeID, 2, 0x0001, 0x00, &mode, sizeof(mode));

    Serial.print("Mode transmitted: ");
    Serial.print(mode);
    Serial.println();
    mode++;
    

    // Read heartbeat interval from node 2 (index 0x1017, subindex 0x00)
    executeSDORead(nodeID, 2, 0x1017, 0x00, &heartbeatNode2);

    Serial.print("Heartbeat from node 2 Received: ");
    Serial.print(heartbeatNode2);
    Serial.println();
  }

  //User code end loop() --------------------------------------------------------
}