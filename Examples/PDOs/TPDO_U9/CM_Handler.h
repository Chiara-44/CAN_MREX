/**
 * CAN MREX Handler file 
 *
 * File:            CM_Handler.h
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   9/09/2025
 * Version:         1.1.3
 *
 */


#ifndef CM_HANDLER_H
#define CM_HANDLER_H

#include <Arduino.h>
#include "driver/twai.h"

void handleCAN(uint8_t nodeID, twai_message_t* pdoMsg = nullptr);
void handleSDO(const twai_message_t& rxMsg, uint8_t nodeID);
void handleNMT(const twai_message_t& rxMsg);
void handleHeartbeat(const twai_message_t& rxMsg);

#endif