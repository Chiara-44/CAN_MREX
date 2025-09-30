/**
 * CAN MREX Handler file 
 *
 * File:            CM_Handler.cpp
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   12/09/2025
 * Version:         1.1.5
 *
 */



#include "CM_Handler.h"
#include "driver/twai.h"
#include "CM_ObjectDictionary.h"
#include "CM_PDO.h"
#include "CM_NMT.h"
#include "CM_EMCY.h"
#include "CM_Heartbeat.h"

void handleCAN(uint8_t nodeID, twai_message_t* pdoMsg) {
  serviceTPDOs(nodeID); // Handles all TPDOs to be sent
  sendHeartbeat(nodeID); //sends Heartbeat periodically
  
  // Receive the message
  twai_message_t rxMsg;
  if (pdoMsg == nullptr) {
    if (twai_receive(&rxMsg, pdMS_TO_TICKS(100)) != ESP_OK) return; // Timeout or error
  } else {
    rxMsg = *pdoMsg;
  }

  // Handle the message
  uint32_t canID = rxMsg.identifier;

  if (canID == 0x000) { // NMT commands (always processed)
    handleNMT(rxMsg, nodeID);
    return;
  } 
  else if (canID == 0x081) { // Emergency messages (always processed)
    handleEMCY(rxMsg, nodeID);
    return;
  } 
  else if ((canID >= 0x180 && canID <= 0x57F) && nodeOperatingMode == 0x80) { // RPDOs (only in operational state)
    processRPDO(rxMsg);
    return;
  } 
  else if (canID == 0x600 + nodeID) { // SDOs (processed in pre-op and operational)
    handleSDO(rxMsg, nodeID);
    return;
  } 
  else if (canID >= 0x700 && canID <= 0x780) { // SDOs (processed in pre-op and operational)
    receiveHeartbeat(rxMsg);
    return;
  } 
  else {
    return;
  }
}


void handleSDO(const twai_message_t& rxMsg, uint8_t nodeID) {
  uint16_t index = rxMsg.data[1] | (rxMsg.data[2] << 8);
  uint8_t subindex = rxMsg.data[3];
  uint8_t cmd = rxMsg.data[0];

  // Prepare response message
  twai_message_t txMsg;
  txMsg.identifier = 0x580 + nodeID;
  txMsg.data_length_code = 8;
  txMsg.flags = TWAI_MSG_FLAG_NONE;
  txMsg.data[1] = rxMsg.data[1];
  txMsg.data[2] = rxMsg.data[2];
  txMsg.data[3] = rxMsg.data[3];
  txMsg.data[4] = 0;
  txMsg.data[5] = 0;
  txMsg.data[6] = 0;
  txMsg.data[7] = 0;

  //lookup OD entry
  ODEntry* entry = findODEntry(index, subindex);
  if (entry == nullptr) {
    Serial.println("OD entry not found");
    return;                                               // throw error here
  }

  if (cmd == 0x40) { // --- Read request ---
    memcpy(&txMsg.data[4], entry->dataPtr, entry->size);

    // Set correct response command byte based on size
    switch (entry->size) {
      case 1: txMsg.data[0] = 0x4F; break;
      case 2: txMsg.data[0] = 0x4B; break;
      case 4: txMsg.data[0] = 0x43; break;
      default: txMsg.data[0] = 0x43; break; // fallback
    }

  } 

  else {  // --- write request ---
    // Determine expected write size from command byte
    uint8_t expectedSize = 0;
    switch(cmd) {
      case 0x2F: expectedSize = 1; break;
      case 0x2B: expectedSize = 2; break;
      case 0x23: expectedSize = 4; break;
      default: expectedSize = 4; break;                   // throw error here 
    }

    //Copy into the OD
    if (expectedSize == entry->size) {
      memcpy(entry->dataPtr, &rxMsg.data[4], expectedSize);
      txMsg.data[0] = 0x60; // Write confirmation
      // // Debug: Show updated value
      // Serial.print("Updated value: ");
      // for (int i = 0; i < entry->size; i++) {
      //   Serial.print(((uint8_t*)entry->dataPtr)[i], HEX);
      //   Serial.print(" ");
      // }
      // Serial.println();
    } else {
      Serial.println("SDO write size mismatch");              // throw error here
      return; // Abort transmission
    }
  }

  // Send the response 
  if (twai_transmit(&txMsg, pdMS_TO_TICKS(100)) == ESP_OK) {
    // Serial.println("SDO response sent");
  } else {
    Serial.println("Failed to send SDO response");
  }
}


