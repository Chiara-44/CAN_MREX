/**
 * CAN MREX Handler file 
 *
 * File:            CM_Handler.h
 * Oraganisation:   MREX
 * Author:          Chiara Gillam
 * Date Created:    5/08/2025
 * Last Modified:   6/08/2025
 *
 */

#ifndef CM_HANDLER_H
#define CM_HANDLER_H

#include <Arduino.h>
#include "driver/twai.h"

void handleCAN(uint8_t nodeID);
void handleSDO(const twai_message_t& rxMsg, uint8_t nodeID);
void handlePDO(const twai_message_t& rxMsg);
void handleNMT(const twai_message_t& rxMsg);
void handleHeartbeat(const twai_message_t& rxMsg);

#endif