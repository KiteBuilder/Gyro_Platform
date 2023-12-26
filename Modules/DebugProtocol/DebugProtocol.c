/*
 * DebugProtocol.c
 *
 *  Created on: Dec 26, 2023
 *      Author: KiteBuilder
 */

#include <stdbool.h>
#include "DebugProtocol.h"

UART_HandleTypeDef *huart_dbg;

uint8_t rxID = 0;
RxState rxState = GET_DLE;
uint8_t rxData[RX_MAX_CNT];
uint8_t rxCnt = 0;

uint8_t rxByte;
float *p_fltData;
bool  f_ready;

static void ParseDLEETXProtocol(uint8_t);
static void HandleRxPacket_Float(uint8_t*, uint16_t, float*);

/**
  * @brief Parse the protocol that consists of next fields |DLE|ID|DATA|DLE|ETX|
  *        DLE value byte in data area should be dabbled
  * @param rxByte: received byte
  * @retval None
  */
static void ParseDLEETXProtocol(uint8_t rxByte)
{
    switch (rxState)
    {
        case GET_DLE:
            if (rxByte == DLE)
            {
                rxState = GET_ID;
                rxCnt = 0;
            }
            break;

        case GET_ID:
            if (rxByte != ETX || rxByte != DLE)
            {
                rxID = rxByte;
                if (rxByte == ID)
                {
                    rxState = GET_DATA;
                }
                else
                {
                    rxState = GET_DLE;
                }
            }
            else
            {
                rxState = GET_DLE;
            }
            break;

        case GET_DATA:
            if (rxByte == DLE)
            {
                rxState = GET_ETX;
            }
            else
            {
                if (rxCnt < RX_MAX_CNT)
                {
                    rxData[rxCnt++] = rxByte;
                }
                else
                {
                    rxState = GET_DLE;
                }
            }
            break;

        case GET_ETX:
            if (rxByte == DLE)
            {
                if (rxCnt < RX_MAX_CNT)
                {
                    rxData[rxCnt++] = rxByte;
                    rxState = GET_DATA;
                }
                else
                {
                    rxState = GET_DLE;
                }
            }
            else
            {
                rxState = GET_DLE;
                //Handle received packet
                HandleRxPacket_Float(rxData, rxCnt, p_fltData);
            }
            break;
    }
}

/**
  * @brief Handle the RX packet as a packet with a raw of float types
  * @param buf: source, received data buffer
  *        size: size of buffer in bytes
  *        f_buf: destination, float buffer
  * @retval None
  */
static void HandleRxPacket_Float(uint8_t *buf, uint16_t size, float *f_buf)
{
    byte_float_t btft;
    uint16_t i = 0, j = 0;

    do
    {
        btft.bt[0] = buf[i];
        btft.bt[1] = buf[i + 1];
        btft.bt[2] = buf[i + 2];
        btft.bt[3] = buf[i + 3];

        f_buf[j++] = btft.flt;

        i += 4;

    }while (i < size);

    f_ready = true;
}

/**
  * @brief UART RX complete callback
  * @param huart: UART descriptor
  * @retval None
  */
void Debug_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == huart_dbg)
    {
        ParseDLEETXProtocol(rxByte);
        HAL_UART_Receive_IT(huart, &rxByte, 1);
    }
}

/**
  * @brief Initialize debug protocol
  * @param huart: UART descriptor
  *        data_buff: pointer to the float data buffer
  * @retval None
  */
void Debug_InitProtocol(UART_HandleTypeDef *huart, float *data_buff)
{
    huart_dbg = huart;
    p_fltData = data_buff;
    f_ready = false;

    HAL_UART_Receive_IT(huart_dbg, &rxByte, 1);
}

/**
  * @brief Check for the received packet
  * @param None
  * @retval bool
  */
bool Debug_IsRxready()
{
    if (f_ready == true)
    {
        f_ready = false;
        return true;
    }
    return false;
}
