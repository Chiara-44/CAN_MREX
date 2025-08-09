/**
 * CAN MREX Transmits file 
 *
 * File:            CM_Transmits.h
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    6/08/2025
 * Last Modified:   9/08/2025
 * Version:         1.1.1
 *
 */


#ifndef CM_TRANSMIT_H
#define CM_TRANSMIT_H



void TransmitSDO(uint8_t targetNodeID, uint8_t* data, int32_t* outValue);


#endif