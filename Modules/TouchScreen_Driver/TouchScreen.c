/*
 * TouchScreen.c
 *
 *  Created on: Dec 17, 2023
 *      Author: KiteBuilder
 */

#include "TouchScreen.h"
#include "Time.h"

static SPI_HandleTypeDef *p_spi;
static Touch_Port *p_cs, *p_int;

static void Touch_ConvertXY(uint16_t*, uint16_t*, bool);

/**
  * @brief
  * @param None
  * @retval None
  */

void Touch_Set_Interface(SPI_HandleTypeDef *p_spi_handler, Touch_Port *p_port_cs, Touch_Port *p_port_int)
{
    p_spi = p_spi_handler;
    p_cs = p_port_cs;
    p_int = p_port_int;
}

/**
  * @brief
  * @param None
  * @retval None
  */
void Touch_Get_Coordinates(uint16_t *x, uint16_t *y, bool rotate)
{
    uint8_t cmd;
    uint16_t dataxy[2] = {0};
    uint8_t data[2] = {0};

    //while ((samples_cnt < num_samples) &&  (HAL_GPIO_ReadPin(p_int->gpio, p_int->pin) == GPIO_PIN_RESET))
    if (HAL_GPIO_ReadPin(p_int->gpio, p_int->pin) == GPIO_PIN_RESET)
    {

        HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_RESET); //Select Touch

        cmd = 0x93; //X
        HAL_SPI_Transmit(p_spi, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(p_spi, data, 2, HAL_MAX_DELAY);
        dataxy[0] = (uint16_t)((data[0] << 8) | (data[1])) >> 3;

        cmd = 0xD3; //Y
        HAL_SPI_Transmit(p_spi, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(p_spi, data, 2, HAL_MAX_DELAY);
        dataxy[1] = (uint16_t)((data[0] << 8) | (data[1])) >> 3;

        cmd = 0x00; //
        HAL_SPI_TransmitReceive(p_spi, &cmd, data, 2, HAL_MAX_DELAY);

        HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_SET);   //Un-select Touch

        *x = dataxy[0];
        *y = dataxy[1];

        Touch_ConvertXY(x, y, rotate);
    }
}

/**
  * @brief
  * @param None
  * @retval None
  */
static void Touch_ConvertXY(uint16_t *x, uint16_t *y, bool rotate)
{
    float x_fix, y_fix;

    x_fix = X_SCALE * (*x - TOUCH_LEFT) + LCD_LEFT;

    if (!rotate)
    {
        y_fix = Y_SCALE * (*y - TOUCH_TOP) + LCD_TOP;

        *x = (uint16_t)x_fix;
        *y = (uint16_t)y_fix;
    }
    else
    {
        y_fix =  LCD_BOTTOM - Y_SCALE * (*y - TOUCH_TOP);

        *y = (uint16_t)x_fix;
        *x = (uint16_t)y_fix;
    }
}
