/**
 * CAN MREX Network managment tool file
 *
 * File:            CM_NMT.cpp
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    12/09/2025
 * Last Modified:   12/09/2025
 * Version:         1.1.1
 *
 */

#include "CM_ObjectDictionary.h"
#include "driver/twai.h"
#include "CM_NMT.h"

void handleNMT(const twai_message_t& rxMsg, uint8_t nodeID){
  if (rxMsg.data[1] != nodeID); return;
  nodeOperatingMode = rxMsg.data[0];
}


void sendNMT(uint8_t sendOperatingMode, uint8_t targetNodeID){
  twai_message_t txMsg;
  txMsg.identifier = 0x000;
  txMsg.data_length_code = 2;
  txMsg.data[0] = sendOperatingMode;
  txMsg.data[1] = targetNodeID;
}