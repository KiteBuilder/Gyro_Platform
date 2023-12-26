/*
 * TouchScreen.h
 *
 *  Created on: Dec 17, 2023
 *      Author: KiteBuilder
 */

#ifndef TOUCHSCREEN_H_
#define TOUCHSCREEN_H_

#include <stm32f4xx_hal.h>
#include <stdbool.h>
#include <stdint.h>

//ILI9341 port structure
typedef struct{
    GPIO_TypeDef* gpio;
    uint16_t pin;
} Touch_Port;

void Touch_Set_Interface(SPI_HandleTypeDef*, Touch_Port*, Touch_Port*);

void Touch_Get_Coordinates(uint16_t *x, uint16_t *y);

#endif /* TOUCHSCREEN_H_ */
