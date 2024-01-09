/*
 * DebugProtocol.h
 *
 *  Created on: Dec 26, 2023
 *      Author: KiteBuilder
 */

#ifndef DEBUGPROTOCOL_H_
#define DEBUGPROTOCOL_H_

#include "stm32f4xx_hal.h"

#define RX_MAX_CNT 64

//*****************************************************************************
//DLE-ETX protocol
//*****************************************************************************
#define DLE 0x10
#define ETX 0x03
#define ID 0x01

typedef enum {GET_DLE = 0, GET_ID, GET_DATA, GET_ETX} RxState;

//*****************************************************************************
//Parse received data
//*****************************************************************************
#pragma pack(push, 1)
typedef union
{
    float flt;
    uint8_t bt[sizeof(float)];
} byte_float_t;
#pragma pack(pop)

void Debug_InitProtocol(UART_HandleTypeDef*, float*);
bool Debug_IsRxready();
void Debug_RxCpltCallback(UART_HandleTypeDef*);

#endif /* DEBUGPROTOCOL_H_ */
