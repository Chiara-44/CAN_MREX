/**
 * CAN MREX Object Dictionary file 
 *
 * File:            CM_ObjectDictionary.cpp
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   8/09/2025
 * Version:         1.1.2
 *
 */

#include "CM_ObjectDictionary.h"

uint8_t deviceMode = 0;
uint32_t heartbeatInterval = 1000;

#define MAX_OD_ENTRIES 32
static ODEntry objectDictionary[MAX_OD_ENTRIES];
static int odCount = 0;


// od entry lookup
ODEntry* findODEntry(uint16_t index, uint8_t subindex) {
  for (int i = 0; i < odCount; i++) {
    if (objectDictionary[i].index == index && objectDictionary[i].subindex == subindex) {
      return &objectDictionary[i];
    }
  }
  return nullptr;
}

bool registerODEntry(uint16_t index, uint8_t subindex, uint8_t access, uint8_t size, void* dataPtr) {
  if (odCount >= MAX_OD_ENTRIES) return false;
  objectDictionary[odCount++] = {index, subindex, access, size, dataPtr};
  return true;
}

void initDefaultOD(){
  registerODEntry(0x1000, 0x00, 2, sizeof(uint8_t), &deviceMode);
  registerODEntry(0x1017, 0x00, 0, sizeof(uint32_t), &heartbeatInterval);
}
