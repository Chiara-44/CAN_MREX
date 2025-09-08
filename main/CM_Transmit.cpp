/**
 * CAN MREX Transmits file 
 *
 * File:            CM_Transmits.cpp
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   8/09/2025
 * Version:         1.1.2
 *
 */

#include "driver/twai.h"
#include <Arduino.h>
#include "CM_Handler.h"


void TransmitSDO(uint8_t nodeID, uint8_t targetNodeID, uint8_t* data, int32_t* outValue) { 
  // Prepare SDO for transmit
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

  // Wait for response
  unsigned long start = millis();
  twai_message_t response;
  while (millis() - start < 200) { // try until timeout and ensure messages received before are handled
    if (twai_receive(&response, pdMS_TO_TICKS(10)) != ESP_OK) continue;       // throw error

    if (response.identifier == 0x580 + targetNodeID) { // Received response
      uint8_t cmd = response.data[0];

      if (cmd == 0x60) { // SDO Confirmed
        //Serial.println("SDO write confirmed"); //debugging
        return;
      }

      if (cmd == 0x80) { // SDO abort
        //Serial.println("SDO abort received");//debugging
        return;
      }

      if (cmd == 0x4F || cmd == 0x4B || cmd == 0x43) { // SDO Read 1, 2, 4 bytes
        int32_t value = 0;
        switch (cmd) {
          case 0x4F: value = response.data[4]; break;
          case 0x4B: value = response.data[4] | (response.data[5] << 8); break;
          case 0x43: value = response.data[4] | (response.data[5] << 8) |
                            (response.data[6] << 16) | (response.data[7] << 24); break;
        }

        //Return value 
        if (outValue != nullptr) {
          *outValue = value;
          // Serial.print("SDO read value: ");
          // Serial.println(value);
        }
        return;
      }

      Serial.println("Unexpected SDO response");  // error handling
      return;


    } else{ // handle messages that aren't the response 
      handleCAN(nodeID, &response);   
    }
  }

  Serial.println("SDO response timeout");                                     //Throw error
}
// TODO: add a counter to ensure it tries multiple times and an error if it fails.
//ALSO: it could fail to process soemthing important if it sent between teh sdo transmit and receive.
