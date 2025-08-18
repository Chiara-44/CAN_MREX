/**
 * CAN MREX Handler file 
 *
 * File:            CM_Handler.cpp
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   9/08/2025
 * Version:         1.1.1
 *
 */


#include "CM_Handler.h"
#include "driver/twai.h"
#include "CM_ObjectDictionary.h"

void handleCAN(uint8_t nodeID) {
  twai_message_t rxMsg;

  if (twai_receive(&rxMsg, pdMS_TO_TICKS(10)) != ESP_OK) return;

  uint32_t canID = rxMsg.identifier;

  if (canID == 0x600 + nodeID) {
    handleSDO(rxMsg, nodeID);
  } elseif ((id >= 0x200 + nodeID) && (id <= 0x500 + nodeID)) {
    processRPDO(rxMsg);     // Handles RPDO1–4
  } else if (canID == 0x000) {
    handleNMT(rxMsg);
  } else if (canID == 0x700 + nodeID) {
    handleHeartbeat(rxMsg);
  } else {
    Serial.print("Unhandled CAN ID: 0x");
    Serial.println(canID, HEX);
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

  // Debug: Incoming SDO
  Serial.print("SDO received: ");
  for (int i = 0; i < 8; i++) {
    if (rxMsg.data[i] < 0x10) Serial.print("0");
    Serial.print(rxMsg.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  ODEntry* entry = findODEntry(index, subindex);
  if (entry == nullptr) {
    Serial.println("OD entry not found");
    return;
  }

  if (cmd == 0x40) { // Read request
    memcpy(&txMsg.data[4], entry->dataPtr, entry->size);

    // Set correct response command byte based on size
    switch (entry->size) {
      case 1: txMsg.data[0] = 0x4F; break;
      case 2: txMsg.data[0] = 0x4B; break;
      case 4: txMsg.data[0] = 0x43; break;
      default: txMsg.data[0] = 0x43; break; // fallback
    }

  } else {
    // Determine expected write size from command byte
    uint8_t expectedSize = 0;
    if (cmd == 0x2F) expectedSize = 1;
    else if (cmd == 0x2B) expectedSize = 2;
    else if (cmd == 0x23) expectedSize = 4;

    if (expectedSize == entry->size) {
      memcpy(entry->dataPtr, &rxMsg.data[4], expectedSize);
      txMsg.data[0] = 0x60; // Write confirmation

      // Debug: Show updated value
      Serial.print("Updated value: ");
      for (int i = 0; i < entry->size; i++) {
        Serial.print(((uint8_t*)entry->dataPtr)[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

    } else {
      Serial.println("SDO write size mismatch");
      return; // Abort transmission
    }
  }

  // Debug: Outgoing SDO response
  Serial.print("SDO Response: ");
  for (int i = 0; i < 8; i++) {
    if (txMsg.data[i] < 0x10) Serial.print("0");
    Serial.print(txMsg.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  if (twai_transmit(&txMsg, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.println("SDO response sent");
  } else {
    Serial.println("Failed to send SDO response");
  }
}


void handlePDO(const twai_message_t& rxMsg) {
  int16_t sensorVal = (rxMsg.data[1] << 8) | rxMsg.data[0];
  Serial.print("Received PDO value: ");
  Serial.println(sensorVal);
}

void handleNMT(const twai_message_t& rxMsg) {
  Serial.print("Received NMT command: 0x");
  Serial.println(rxMsg.data[0], HEX);
}

void handleHeartbeat(const twai_message_t& rxMsg) {
  Serial.println("Heartbeat received");
}