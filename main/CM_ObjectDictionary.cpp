/**
 * CAN MREX Object Dictionary file 
 *
 * File:            CM_ObjectDictionary.cpp
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   9/08/2025
 * Version:         1.1.1
 *
 */

#include "CM_ObjectDictionary.h"

uint8_t deviceMode = 1;
uint32_t heartbeatInterval = 1000;
uint8_t speed = 0;

ODEntry objectDictionary[] = {
  {0x1000, 0x00, 0, sizeof(uint8_t), &deviceMode},
  {0x1017, 0x00, 2, sizeof(uint32_t), &heartbeatInterval},
  {0x0001, 0x00, 2, sizeof(uint8_t), &speed}
  // Add more entries here
};

const int OD_SIZE = sizeof(objectDictionary) / sizeof(ODEntry);


ODEntry* findODEntry(uint16_t index, uint8_t subindex) {
  for (int i = 0; i < OD_SIZE; i++) {
    if (objectDictionary[i].index == index && objectDictionary[i].subindex == subindex) {
      return &objectDictionary[i];
    }
  }
  return nullptr;
}
