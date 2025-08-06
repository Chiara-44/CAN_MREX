/**
 * CAN MREX Transmits file 
 *
 * File:            CM_Transmits.cpp
 * Oraganisation:   MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   6/08/2025
 *
 */

#include "driver/twai.h"
#include <Arduino.h>


void TransmitSDO(uint8_t targetNodeID, uint8_t* data) {
  // Construct SDO request message
  twai_message_t msg;
  msg.identifier = 0x600 + targetNodeID;  // SDO request ID
  msg.data_length_code = 8;
  msg.flags = TWAI_MSG_FLAG_NONE;

  for (int i = 0; i < 8; i++) {
    msg.data[i] = data[i];
  }

  // Transmit SDO request
  if (twai_transmit(&msg, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.print("Sent SDO request to node ");
    Serial.print(targetNodeID);
    Serial.print(" with CAN ID: 0x");
    Serial.println(msg.identifier, HEX);
  } else {
    Serial.println("Failed to send SDO request");
    return;
  }

  // Wait for confirmation response
  twai_message_t response;
  if (twai_receive(&response, pdMS_TO_TICKS(100)) == ESP_OK) {
    if (response.identifier == 0x580 + targetNodeID && response.data[0] == 0x60) {
      Serial.println("SDO write confirmation received");
    } else {
      Serial.print("Unexpected SDO response: ID 0x");
      Serial.println(response.identifier, HEX);
    }
  } else {
    Serial.println("No SDO confirmation received");
  }
}