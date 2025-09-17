/**
 * CAN MREX main (Template) file 
 *
 * File:            main.ino
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    5/08/2025
 * Last Modified:   13/09/2025
 * Version:         1.1.4
 *
 */

#include "CM.h" // inlcudes all CAN MREX files

// User code begin: ------------------------------------------------------

const uint8_t nodeID = 1;  // Change this to set your device's node ID
const bool nmtMaster = false;
const bool heartbeatConsumer = false;

// --- Pin Definitions ---
#define TX_GPIO_NUM GPIO_NUM_5 // Set GPIO pin for CAN Transmit
#define RX_GPIO_NUM GPIO_NUM_4 // Set GPIO pins for CAN Receive

// --- OD definitions ---


//OPTIONAL: timing for a non blocking function occuring every two seconds
// unsigned long previousMillis = 0;
// const long interval = 2000; // 2 seconds

// User code end ---------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Serial Coms started at 115200 baud");
  
  //Initialize CANMREX protocol
  initCANMREX(TX_GPIO_NUM, RX_GPIO_NUM, nodeID);

  // User code Setup Begin: -------------------------------------------------
  // --- Register OD entries ---


  // --- Register TPDOs ---
  

  // --- Register RPDOs ---
 

  // User code Setup end ------------------------------------------------------


}

void loop() {
  //User Code begin loop() ----------------------------------------------------
  // --- Stopped mode (This is default starting point) ---
  if (nodeOperatingMode == 0x02){ 
    handleCAN(nodeID);
    Serial.println("Emergency stop in 5 seconds");
    delay(5000);
    Serial.println("Emergency stop");
    sendEMCY(0x00, nodeID, 0x0000);
    delay(20000);
  }

  // --- Pre operational state (This is where you can do checks and make sure that everything is okay) ---
  if (nodeOperatingMode == 0x80){ 
    handleCAN(nodeID);
  }

  // --- Operational state (Normal operating mode) ---
  if (nodeOperatingMode == 0x01){ 
    handleCAN(nodeID);
  }

  //User code end loop() --------------------------------------------------------
}