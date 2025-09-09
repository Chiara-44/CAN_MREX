/**
 * CAN MREX Transmits file 
 *
 * File:            CM_Transmit.h
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   8/09/2025
 * Version:         1.1.3
 *
 */


#ifndef CM_TRANSMIT_H
#define CM_TRANSMIT_H



void transmitSDO(uint8_t nodeID, uint8_t targetNodeID, uint8_t* data, uint32_t* outValue);
void prepareSDOTransmit(uint8_t cmd, uint16_t index, uint8_t subindex, const void* value, size_t size, uint8_t* outBuf);
void executeSDOWrite(uint8_t nodeID, uint8_t targetNodeID, uint16_t index, uint8_t subindex,  const void* value, size_t size);
void executeSDORead(uint8_t nodeID, uint8_t targetNodeID, uint16_t index, uint8_t subindex, uint32_t* outValue);
void waitSDOResponse(uint32_t* outValue, uint8_t targetNodeID, uint8_t nodeID);

#endif