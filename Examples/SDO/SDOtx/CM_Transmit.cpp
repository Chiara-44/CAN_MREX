/**
 * CAN MREX Transmits file 
 *
 * File:            CM_Transmits.cpp
 * Oraganisation:   MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   9/08/2025
 * Version:         1.1.1
 *
 */

#include "driver/twai.h"
#include <Arduino.h>


void TransmitSDO(uint8_t targetNodeID, uint8_t* data, int32_t* outValue) {
  twai_message_t msg;
  msg.identifier = 0x600 + targetNodeID;
  msg.data_length_code = 8;
  msg.flags = TWAI_MSG_FLAG_NONE;
  memcpy(msg.data, data, 8);

  // Transmit SDO request
  if (twai_transmit(&msg, pdMS_TO_TICKS(100)) != ESP_OK) {
    Serial.println("Failed to send SDO request");
    return;
  }

  Serial.print("Sent SDO to node ");
  Serial.println(targetNodeID);

  // Wait for response
  twai_message_t response;
  unsigned long start = millis();
  while (millis() - start < 100) {
    if (twai_receive(&response, pdMS_TO_TICKS(10)) == ESP_OK) {
      if (response.identifier == 0x580 + targetNodeID) {
        uint8_t cmd = response.data[0];

        if (cmd == 0x60) {
          Serial.println("SDO write confirmed");
          return;
        }

        if (cmd == 0x80) {
          Serial.println("SDO abort received");
          return;
        }

        if (cmd == 0x4F || cmd == 0x4B || cmd == 0x43) {
          int32_t value = 0;
          switch (cmd) {
            case 0x4F: value = response.data[4]; break;
            case 0x4B: value = response.data[4] | (response.data[5] << 8); break;
            case 0x43: value = response.data[4] | (response.data[5] << 8) |
                              (response.data[6] << 16) | (response.data[7] << 24); break;
          }

          if (outValue != nullptr) {
            *outValue = value;
            Serial.print("SDO read value: ");
            Serial.println(value);
          }
          return;
        }

        Serial.println("Unexpected SDO response");
        return;
      }
    }
  }

  Serial.println("SDO response timeout");
}


