/**
 * CAN MREX Handler file 
 *
 * File:            CM_Handler.cpp
 * Oraganisation:   MREX
 * Author:          Chiara Gillam
 * Date Created:    5/08/2025
 * Last Modified:   6/08/2025
 *
 */

#include "CM_Handler.h"
#include "driver/twai.h"

void handleCAN(uint8_t nodeID) {
  twai_message_t rxMsg;

  if (twai_receive(&rxMsg, pdMS_TO_TICKS(10)) != ESP_OK) return;

  uint32_t canID = rxMsg.identifier;

  if (canID == 0x600 + nodeID) {
    handleSDO(rxMsg, nodeID);
  } else if (canID == 0x200 + nodeID) {
    handlePDO(rxMsg);
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
  twai_message_t txMsg;
  txMsg.identifier = 0x580 + nodeID;
  txMsg.data_length_code = 8;
  txMsg.flags = TWAI_MSG_FLAG_NONE;

  txMsg.data[0] = 0x60;
  txMsg.data[1] = rxMsg.data[1];
  txMsg.data[2] = rxMsg.data[2];
  txMsg.data[3] = rxMsg.data[3];
  txMsg.data[4] = 0;
  txMsg.data[5] = 0;
  txMsg.data[6] = 0;
  txMsg.data[7] = 0;

  if (twai_transmit(&txMsg, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.println("SDO confirmation sent");
  } else {
    Serial.println("Failed to send SDO confirmation");
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