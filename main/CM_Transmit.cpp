/**
 * CAN MREX Transmits file 
 *
 * File:            CM_Transmit.cpp
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   8/09/2025
 * Version:         1.1.3
 *
 */

#include "driver/twai.h"
#include <Arduino.h>
#include "CM_Handler.h"
#include "CM_Transmit.h"


void transmitSDO(uint8_t nodeID, uint8_t targetNodeID, uint8_t* data, uint32_t* outValue) { 
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
  waitSDOResponse(outValue, targetNodeID, nodeID);
}


//used to prepare the message being sent over SDO
void prepareSDOTransmit(uint8_t cmd, uint16_t index, uint8_t subindex, const void* value, size_t size, uint8_t* outBuf) {
  outBuf[0] = cmd;
  outBuf[1] = index & 0xFF;
  outBuf[2] = (index >> 8) & 0xFF;
  outBuf[3] = subindex;

  // Copy value into data[4..7]
  memset(&outBuf[4], 0, 4); // Clear padding
  if (value != nullptr && size > 0) {
    memcpy(&outBuf[4], value, size);
  }
}

void executeSDOWrite(uint8_t nodeID, uint8_t targetNodeID, uint16_t index, uint8_t subindex,  const void* value, size_t size) {
  uint8_t sdoBuf[8];
  uint8_t cmd;

  switch (size) {
    case 1: cmd = 0x2F; break;
    case 2: cmd = 0x2B; break;
    case 4: cmd = 0x23; break;
    default:
      Serial.println("Unsupported SDO write size");
      return;
  }

  prepareSDOTransmit(cmd, index, subindex, value, size, sdoBuf);
  transmitSDO(nodeID, targetNodeID, sdoBuf, nullptr);
}

void executeSDORead(uint8_t nodeID, uint8_t targetNodeID, uint16_t index, uint8_t subindex, uint32_t* outValue) {
  uint8_t sdoBuf[8];
  uint8_t cmd = 0x40;
  

  prepareSDOTransmit(cmd, index, subindex, nullptr, 0, sdoBuf);
  transmitSDO(nodeID, targetNodeID, sdoBuf, outValue);
}

void waitSDOResponse(uint32_t* outValue, uint8_t targetNodeID, uint8_t nodeID){
  // Wait for response
  unsigned long start = millis();
  twai_message_t response;
  while (millis() - start < 200) { // try until timeout and ensure messages received before are handled
    if (twai_receive(&response, pdMS_TO_TICKS(10)) != ESP_OK) continue;       // throw error

    if (response.identifier == 0x580 + targetNodeID) { // Received response
      uint8_t cmd = response.data[0];

      if (cmd == 0x60) return; // SDO Confirmed

      if (cmd == 0x80) { // SDO abort
        Serial.println("SDO abort received");//debugging
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

  Serial.println("SDO response timeout");  
}
// TODO: add a counter to ensure it tries multiple times and an error if it fails.
//ALSO: it could fail to process soemthing important if it sent between teh sdo transmit and receive.
