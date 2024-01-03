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

//Calibration values that were gotten by two calibration bars
#define LCD_LEFT      10
#define LCD_TOP       10
#define LCD_RIGHT     470
#define LCD_BOTTOM    310

#define TOUCH_LEFT    320
#define TOUCH_TOP     240
#define TOUCH_RIGHT   3960
#define TOUCH_BOTTOM  3840

#define Y_SCALE (((LCD_BOTTOM - LCD_TOP) * 1.0) / (TOUCH_BOTTOM - TOUCH_TOP))
#define X_SCALE (((LCD_RIGHT - LCD_LEFT) * 1.0) / (TOUCH_RIGHT - TOUCH_LEFT))

//ILI9341 port structure
typedef struct{
    GPIO_TypeDef* gpio;
    uint16_t pin;
} Touch_Port;

void Touch_Set_Interface(SPI_HandleTypeDef*, Touch_Port*, Touch_Port*);

void Touch_Get_Coordinates(uint16_t *x, uint16_t *y,  bool);

#endif /* TOUCHSCREEN_H_ */
