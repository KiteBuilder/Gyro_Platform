/*
 * TouchScreen.c
 *
 *  Created on: Dec 17, 2023
 *      Author: KiteBuilder
 */


#include "TouchScreen.h"


#define Z_THRESHOLD     300

static SPI_HandleTypeDef *p_spi;
static Touch_Port *p_cs, *p_int;
//static const uint32_t num_samples = 3;


void Touch_Set_Interface(SPI_HandleTypeDef *p_spi_handler, Touch_Port *p_port_cs, Touch_Port *p_port_int)
{
    p_spi = p_spi_handler;
    p_cs = p_port_cs;
    p_int = p_port_int;
}

void Touch_Get_Coordinates(uint16_t *x, uint16_t *y)
{
    uint8_t cmd;
    uint8_t data[2];
    uint16_t dataxy[2];

    //while ((samples_cnt < num_samples) &&  (HAL_GPIO_ReadPin(p_int->gpio, p_int->pin) == GPIO_PIN_RESET))
    if (HAL_GPIO_ReadPin(p_int->gpio, p_int->pin) == GPIO_PIN_RESET)
    {

        HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_RESET); //Select Touch

        cmd = 0xD0; //X
        HAL_SPI_Transmit(p_spi, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(p_spi, data, 2, HAL_MAX_DELAY);
        //HAL_SPI_TransmitReceive(p_spi, &cmd, data, 2, HAL_MAX_DELAY);
        dataxy[0] = ((data[0] << 8) | (data[1])) >> 3;

        cmd = 0x90; //Y
        HAL_SPI_Transmit(p_spi, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(p_spi, data, 2, HAL_MAX_DELAY);
        //HAL_SPI_TransmitReceive(p_spi, &cmd, data, 2, HAL_MAX_DELAY);
        dataxy[1] = ((data[0] << 8) | (data[1])) >> 3;

        HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_SET);   //Un-select Touch
    }

    *x = dataxy[0];
    *y = dataxy[1];
}
