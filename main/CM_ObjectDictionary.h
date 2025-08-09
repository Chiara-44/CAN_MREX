/**
 * CAN MREX Object Dictionary file 
 *
 * File:            CM_ObjectDictionary.h
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   9/08/2025
 * Version:         1.1.1
 *
 */

 

#ifndef CM_OBJECT_DICTIONARY_H
#define CM_OBJECT_DICTIONARY_H

#include <stdint.h>

extern uint8_t deviceMode;
extern uint32_t heartbeatInterval;
extern uint8_t speed;

typedef struct {
  uint16_t index;
  uint8_t subindex;
  uint8_t access; // 0 = RO, 1 = WO, 2 = RW
  uint8_t size;   // in bytes
  void* dataPtr;
} ODEntry;

extern ODEntry objectDictionary[];
extern const int OD_SIZE;

ODEntry* findODEntry(uint16_t index, uint8_t subindex);

#endif