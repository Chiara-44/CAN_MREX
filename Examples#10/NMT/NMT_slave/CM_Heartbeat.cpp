/**
 * CAN MREX Heartbeat file
 *
 * File:            CM_Heartbeat.cpp
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    12/09/2025
 * Last Modified:   13/09/2025
 * Version:         1.1.1
 *
 */

#include "driver/twai.h"
#include "CM_ObjectDictionary.h"
#include <Arduino.h>

uint32_t lastMS = 0;

uint32_t node1HeartbeatLast;
uint8_t node1Opertatingstate;

void sendHeartbeat(uint8_t nodeID){
  uint64_t currentMs = millis();
  if (currentMs - lastMS > heartbeatInterval){
    twai_message_t txMsg;
    txMsg.identifier = 0x700 + nodeID;     
    txMsg.data_length_code = 1;
    txMsg.data[0] = nodeOperatingMode;
    lastMS = currentMs;
    twai_transmit(&txMsg, pdMS_TO_TICKS(100));
  }
}

void receiveHeartbeat(const twai_message_t rxMsg){
  node1Opertatingstate = rxMsg.data[0];
}

void printOperatingMode(){
  Serial.println(node1Opertatingstate);
}


void setupHeartbeatConsumer(){
  registerODEntry(0x1000, 0x00, 2, sizeof(node1HeartbeatLast), &node1HeartbeatLast);
}