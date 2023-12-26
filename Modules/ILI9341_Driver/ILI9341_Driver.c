/*
 * ILI9341_Driver.c
 *
 *  Created on: Dec 5, 2023
 *      Author: KiteBuilder
 */

#include <stddef.h>
#include <stdlib.h>

#include "ILI9341_Driver.h"

static SPI_HandleTypeDef *p_spi; //SPI port for transferring data
static ILI9341_Port *p_cs, *p_dc, *p_rst, *p_led; //CS, DC, RST, LED ports
static bool f_DMA;

static uint16_t display_width, display_height; //LCD display width and height that depends on the screen orientation

static void ILI9341_Reset(void);
static void ILI9341_WriteCommand(uint8_t);
static void ILI9341_WriteData(uint8_t);
static void ILI9341_WriteBuff(uint8_t*, size_t);
static void ILI9341_WaitDMAComplete(void);
static int ILI9341_SetWindow(uint16_t, uint16_t, uint16_t, uint16_t);
static int ILI9341_SetColors(uint16_t*, uint16_t, bool);
static void ILI9341_WriteColor(uint32_t, uint16_t);

static uint16_t buff[DISPLAY_MAX_BUFFER_SIZE];

/**
  * @brief Initialize TFT display (ILI9341) interfaces
  * @param value: p_handler:  SPI interface
  *               p_port_cs:  CS port
  *               p_port_dc:  DC port
  *               p_port_rst: RTS port
  *               p_port_led: LED port
  * @retval None
  */
void ILI9341_Set_Interface(SPI_HandleTypeDef *p_spi_handler, bool f_dma, ILI9341_Port *p_port_cs, ILI9341_Port *p_port_dc, ILI9341_Port *p_port_rst, ILI9341_Port *p_port_led)
{
    p_spi = p_spi_handler;
    p_cs  = p_port_cs;
    p_dc  = p_port_dc;
    p_rst = p_port_rst;
    p_led = p_port_led;

    if (f_dma == true && p_spi->hdmatx != NULL)
    {
        f_DMA =  f_dma;
    }
}

/**
  * @brief Get display height in pixels
  * @param
  * @retval display height in pixels
  */
uint16_t ILI9341_GetHeight(void)
{
    return display_height;
}

/**
  * @brief Get display width in pixels
  * @param
  * @retval display width in pixels
  */
uint16_t ILI9341_GetWidth(void)
{
    return display_width;
}

/**
  * @brief Turn the back light LED on or off
  * @param value: flag: true - LED on, false - LED off
  * @retval None
  */
void ILI9341_BackLight(bool flag)
{
    if (flag)
    {
        HAL_GPIO_WritePin(p_led->gpio, p_led->pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(p_led->gpio, p_led->pin, GPIO_PIN_RESET);
    }
}


/**
  * @brief Write command
  * @param byte: byte sized command
  * @retval None
  */
static void ILI9341_WriteCommand(uint8_t byte)
{
    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_RESET); //Select OLED
    HAL_GPIO_WritePin(p_dc->gpio, p_dc->pin, GPIO_PIN_RESET); //Command mode
    HAL_SPI_Transmit(p_spi, (uint8_t *) &byte, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_SET);   //Un-select OLED
}

/**
  * @brief Write data byte
  * @param buffer: pointer to data buffer
  *        buff_size: amount of bytes to transfer
  * @retval None
  */
static void ILI9341_WriteData(uint8_t byte)
{
    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_RESET); //Select OLED
    HAL_GPIO_WritePin(p_dc->gpio, p_dc->pin, GPIO_PIN_SET);   //Data mode
    HAL_SPI_Transmit(p_spi, (uint8_t *) &byte, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_SET);   //Un-select OLED
}


/**
  * @brief Write data sequence
  * @param buffer: pointer to data buffer
  *        buff_size: amount of bytes to transfer
  * @retval None
  */
static void ILI9341_WriteBuff(uint8_t* buffer, size_t buff_size)
{
    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_RESET); //Select OLED
    HAL_GPIO_WritePin(p_dc->gpio, p_dc->pin, GPIO_PIN_SET);   //Data mode
    HAL_SPI_Transmit(p_spi, buffer, buff_size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_SET);   //Un-select OLED
}

/**
  * @brief Hardware reset
  * @param None
  * @retval None
  */
static void ILI9341_Reset(void)
{
    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(p_rst->gpio, p_rst->pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(p_rst->gpio, p_rst->pin, GPIO_PIN_SET);
    HAL_Delay(120);
}

/**
  * @brief Initialize display
  * @param None
  * @retval None
  */
void ILI9341_Init(void)
{
    ILI9341_Reset();

    ILI9341_WriteCommand(ILI9341_SWRESET);    //Software reset
    HAL_Delay(10);

    ILI9341_WriteCommand(ILI9341_DISPOFF);    //Display off

    ILI9341_WriteCommand(ILI9341_PWCTRA);     //Power control A
    ILI9341_WriteData(0x39);
    ILI9341_WriteData(0x2C);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x34);
    ILI9341_WriteData(0x02);

    ILI9341_WriteCommand(ILI9341_PWCTRB);     //Power control B
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x83);
    ILI9341_WriteData(0x30);

    ILI9341_WriteCommand(ILI9341_DRVTIMCTRA); //Driver timing control A
    ILI9341_WriteData(0x85);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x78);

    ILI9341_WriteCommand(ILI9341_DRVTIMCTRB); //Driver timing control B
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);

    ILI9341_WriteCommand(ILI9341_PWRSEQCTR);  //Power on sequence control
    ILI9341_WriteData(0x64);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0X12);
    ILI9341_WriteData(0X81);

    ILI9341_WriteCommand(ILI9341_PUMPRTCTR);  //Pump ratio control
    ILI9341_WriteData(0x20);

    ILI9341_WriteCommand(ILI9341_PWCTR1);     //Power Control 1
    ILI9341_WriteData(0x23); //VRH[5:0],  GVDD=4.20V

    ILI9341_WriteCommand(ILI9341_PWCTR2);     //Power Control 2
    ILI9341_WriteData(0x10); //BT[2:0]=00

    ILI9341_WriteCommand(ILI9341_VMCTR1);     //VCOM Control 1
    ILI9341_WriteData(0x35); //VMH=0x35, 4.025V
    ILI9341_WriteData(0x3E); //VML=0x3E, -0.950V

    ILI9341_WriteCommand(ILI9341_VMCTR2);     //VCM control2
    ILI9341_WriteData(0xEA); //0xEA = VMH + 42 VML + 42

    ILI9341_WriteCommand(ILI9341_MADCTL);     // Memory Access Control
    ILI9341_WriteData(MADCTL_MV | MADCTL_BGR); // BGR color filter panel, MV=1
    display_width = DISPLAY_PIX_WIDTH;
    display_height = DISPLAY_PIX_HEIGHT;

    ILI9341_WriteCommand(ILI9341_PIXFMT);     //Pixel format
    ILI9341_WriteData(0x55);

    ILI9341_WriteCommand(ILI9341_FRMCTR1);    //Frame ratio control
    ILI9341_WriteData(0x00); //fosc with no division
    ILI9341_WriteData(0x1B); //Frame rate 0x1B-70Hz, 0x13-100Hz

    ILI9341_WriteCommand(ILI9341_DFUNCTR);    //Display Function Control
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x82);
    ILI9341_WriteData(0x3F); //0x3F for 420x320, 0x27 for 320x280

    ILI9341_WriteCommand(ILI9341_3GFUNDIS);   //3Gamma Function Disable
    ILI9341_WriteData(0x00);

    ILI9341_WriteCommand(ILI9341_GAMMASET);   //Gamma curve selected
    ILI9341_WriteData(0x01);

    ILI9341_WriteCommand(ILI9341_GMCTRP1);    //Positive gamma correction
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x4E);
    ILI9341_WriteData(0xF1);
    ILI9341_WriteData(0x37);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x10);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x00);

    ILI9341_WriteCommand(ILI9341_GMCTRN1);    //Negative gamma correction
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x14);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x48);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x36);
    ILI9341_WriteData(0x0F);

    ILI9341_WriteCommand(ILI9341_CASET);      //Column Address set
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x01);
    ILI9341_WriteData(0x3F);

    ILI9341_WriteCommand(ILI9341_PASET);      //Page Address set
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x01);
    ILI9341_WriteData(0xDF);

    ILI9341_WriteCommand(ILI9341_SLPOUT);     //Exit Sleep
    HAL_Delay(120);

    ILI9341_WriteCommand(ILI9341_DISPON);     //Display on
}

/**
  * @brief Set screen orientation
  * @param orientation: Vertival, Horizontal, Vertical 180grad flip, Horizontal 180grad flip
  * @retval None
  */
void ILI9341_SetOrientation(ILI9341_ORIENTATION orientation)
{

    ILI9341_WriteCommand(0x36);

    switch (orientation)
    {
        case SCREEN_VERTICAL_0GRAD:
            ILI9341_WriteData(MADCTL_MX | MADCTL_BGR); //BGR=1, MY=0, MX=1, MV=0
            display_width = DISPLAY_PIX_HEIGHT;
            display_height = DISPLAY_PIX_WIDTH;
            break;

        case SCREEN_HORIZONTAL_0GRAD:
            ILI9341_WriteData(MADCTL_MV | MADCTL_BGR); //BGR=1, MY=0, MX=0, MV=1
            display_width = DISPLAY_PIX_WIDTH;
            display_height = DISPLAY_PIX_HEIGHT;
            break;

        case SCREEN_VERTICAL_180GRAD:
            ILI9341_WriteData(MADCTL_MY | MADCTL_BGR); //BGR=1, MY=1, MX=0, MV=0
            display_width = DISPLAY_PIX_HEIGHT;
            display_height = DISPLAY_PIX_WIDTH;
            break;

        case SCREEN_HORIZONTAL_180GRAD:
            ILI9341_WriteData(MADCTL_MV | MADCTL_MX | MADCTL_MY | MADCTL_BGR); //BGR=1, MY=1, MX=1, MV=1
            display_width = DISPLAY_PIX_WIDTH;
            display_height = DISPLAY_PIX_HEIGHT;
            break;

        default:
    }
}

/**
  * @brief Set the LCD display window
  * @param xStart, yStart - beginning window coordinates
  *        xEnd, yEnd - ending window coordinates
  * @retval None
  */
static int  ILI9341_SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    uint8_t buff[4];

    if ((xStart > xEnd) || (yStart > yEnd))
    {
        return -1;
    }

    if (xEnd >= display_width)
    {
        xEnd  = display_width - 1;
    }

    if (yEnd >= display_height)
    {
        yEnd  = display_height - 1;
    }

    //Address,  XData
    ILI9341_WriteCommand(ILI9341_CASET);
    buff[0] = xStart >> 8;
    buff[1] = xStart;
    buff[2] = xEnd >> 8;
    buff[3] = xEnd;

    ILI9341_WriteBuff(buff, 4);

    //Address,  YData
    ILI9341_WriteCommand(ILI9341_PASET);
    buff[0] = yStart >> 8;
    buff[1] = yStart;
    buff[2] = yEnd >> 8;
    buff[3] = yEnd;
    ILI9341_WriteBuff(buff, 4);

    return 0;
}

/**
  * @brief Set color of pixels for selected window. Maximum amount of pixels can't be greater than DISPLAY_PIX_WIDTH
  * @param color_buff: buffer with colors
  *        size: size of the buffer in bytes
  *        f_wait: wait for DMA completion if set true
  * @retval None
  */
static int ILI9341_SetColors(uint16_t *color_buff, uint16_t size, bool f_wait)
{
    if (color_buff == NULL || size == 0 || size > (DISPLAY_MAX_BUFFER_SIZE << 1))
    {
        return -1;
    }

    if (f_DMA == true)
    {
        HAL_SPI_Transmit_DMA(p_spi, (uint8_t*)color_buff, size);

        if (f_wait == true)
        {
            ILI9341_WaitDMAComplete();
        }
    }
    else
    {
        HAL_SPI_Transmit(p_spi, (uint8_t*)color_buff, size , HAL_MAX_DELAY);
    }

    return 0;
}


/**
  * @brief Wait until DMA complete transmission
  * @param color: display would be filled with
  * @retval None
  */
static void ILI9341_WaitDMAComplete(void)
{
    while (HAL_DMA_STATE_BUSY == HAL_DMA_GetState(p_spi->hdmatx))
    {
        continue;
    }
}

/**
  * @brief Write color buffer to display
  * @param pixels: amount of pixels to be written to display
  *        color: pixels color
  * @retval None
  */
static void ILI9341_WriteColor(uint32_t pixels, uint16_t color)
{
    uint16_t n = 0;

    //Write Color
    ILI9341_WriteCommand(ILI9341_RAMWR);

    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_RESET); //Select OLED
    HAL_GPIO_WritePin(p_dc->gpio, p_dc->pin, GPIO_PIN_SET);   //Data mode

    for (uint32_t i = 0; i < pixels; i++)
    {
        buff[n++] = (color >> 8) | (color << 8);

        if (n == DISPLAY_MAX_BUFFER_SIZE)
        {
            ILI9341_SetColors(buff, n << 1, true);
            n = 0;
        }
    }

    ILI9341_SetColors(buff, n << 1, true);

    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_SET);   //Un-select OLED
}

/**
  * @brief Clear display
  * @param color: display would be filled with
  * @retval None
  */
void ILI9341_Clear(uint16_t color)
{
  uint32_t limit = display_height * display_width;

  ILI9341_SetWindow(0, 0, display_width - 1, display_height - 1);
  ILI9341_WriteColor(limit, color);
}

/**
  * @brief Draw one colored pixel
  * @param X,Y: pixel coordinates
  *        Color: pixel color
  * @retval None
  */
int ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t buff[2];

    if ((x >= display_width) || (y >= display_height))
    {
        return -1; //return if out of the screen range
    }

    ILI9341_SetWindow(x, y, x + 1, y + 1);

    //Write Color
    ILI9341_WriteCommand(ILI9341_RAMWR);

    buff[0] = color >> 8; buff[1] = color;
    ILI9341_WriteBuff(buff, 2);

    return 0;
}

/**
  * @brief Draw line
  * @param (x0,y0) - line start coordinates
  *        (x1,y1) - line end coordinates
  *        color: currently used color
  * @retval None
  */
void ILI9341_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    uint16_t limit = 0;

    if ((x0 == x1) && (y0 < y1)) //Horizontal line
    {
        if (y1 >=  display_height)
        {
            y1  = display_height - 1;
        }

        limit = y1 - y0;
    }
    else if ( (y0 == y1) && (x0 < x1)) //Vertical line
    {
        if (x1 >=  display_width)
        {
            x1  = display_width - 1;
        }

        limit = x1 - x0;
    }

    if (limit != 0) //Vertical or Horizontal lines should be draw
    {
        ILI9341_SetWindow(x0, y0, x1, y1);
        ILI9341_WriteColor(limit, color);
    }
    else //None Vertical or Horizontal line
    {
        int16_t sign = 0;
        int16_t a = y0 - y1;
        int16_t b = x1 - x0;
        int32_t c = (x0 * y1) - (y0 * x1);

        if (b != 0)
        {
            if (abs(b) > abs(a))
            {
                sign = (x0 > x1) ? -1 : 1;
            }
            else
            {
                sign = (y0 > y1) ? -1 : 1;
            }
        }
        else if (a != 0)
        {
            sign = (y0 > y1) ? -1 : 1;
        }

        do
        {
            ILI9341_DrawPixel(x0, y0, color);

            if (b != 0)
            {
                if (abs(b) > abs(a))
                {
                    x0 += sign;
                    y0 = ((-a * x0) - c) / b;
                }
                else
                {
                    y0 += sign;
                    x0 = (y0*b + c)/-a;
                }
            }
            else if (a != 0)
            {
                y0 += sign;
            }

        } while ((x0 != x1) || (y0 != y1));

        ILI9341_DrawPixel(x0, y0, color);
    }
}

/**
  * @brief Draw Rectangle
  * @param (x0,y0) - up left corner coordinates
  *        (x1,y1) - down right corner coordinates
  *        color: currently used color
  * @retval None
  */
void ILI9341_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    ILI9341_DrawLine(x0, y0, x0, y1, color);
    ILI9341_DrawLine(x1, y0, x1, y1, color);
    ILI9341_DrawLine(x0, y0, x1, y0, color);
    ILI9341_DrawLine(x0, y1, x1, y1, color);
}

/**
  * @brief Draw Filled Rectangle
  * @param (x0,y0) - up left corner coordinates
  *        (x1,y1) - down right corner coordinates
  *        color: currently used color
  * @retval None
  */
void ILI9341_DrawFillRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    uint16_t x_start = (x0 < x1) ? x0 : x1;
    uint16_t y_start = (y0 < y1) ? y0 : y1;
    uint16_t x_end   = (x0 < x1) ? x1 : x0;
    uint16_t y_end   = (y0 < y1) ? y1 : y0;

    if (x_end > display_width)
    {
        x_end =  display_width;
    }

    if (y_end > display_height)
    {
        y_end =  display_height;
    }

    uint32_t limit = (x_end - x_start) * (y_end - y_start);

    ILI9341_SetWindow(x_start, y_start, x_end - 1, y_end - 1);
    ILI9341_WriteColor(limit, color);
}

/**
  * @brief Draw circle - Bresenham’s circle drawing algorithm
  * @param (x0,y0) - circle center coordinates
  *         radius - circle radius
  *         color: currently used color
  * @retval None
  */
void ILI9341_DrawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int16_t x = -r, y = 0;
    int16_t err = 2 - 2*r;
    int16_t e2;

    do
    {
        ILI9341_DrawPixel(x0 - x, y0 + y, color);
        ILI9341_DrawPixel(x0 + x, y0 + y, color);
        ILI9341_DrawPixel(x0 + x, y0 - y, color);
        ILI9341_DrawPixel(x0 - x, y0 - y, color);

        e2 = err;
        if (e2 <= y)
        {
            err += ++y*2 + 1;
            if (-x == y && e2 <= x)
            {
                e2 = 0;
            }
        }

        if (e2 > x)
        {
            err += ++x*2+1;
        }
    } while (x <= 0);

}

/**
  * @brief Draw Filled circle - Bresenham’s circle drawing algorithm
  * @param (x0,y0) - circle center coordinates
  *         radius - circle radius
  *         color: currently used color
  * @retval None
  */
void ILI9341_DrawFillCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
    int16_t x = -r, y = 0;
    int16_t err = 2 - 2 * r;
    int16_t e2;

    do
    {
        ILI9341_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y, color);
        ILI9341_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y, color);

        e2 = err;
        if (e2 <= y)
        {
            err += ++y*2 + 1;
            if (-x == y && e2 <= x)
            {
                e2 = 0;
            }
        }

        if (e2 > x)
        {
            err += ++x * 2 + 1;
        }
    } while (x <= 0);
}

/**
  * @brief Draw one char to the screen buffer
  * @param ch: ASCII char symbol
  *        font: currently used font
  *        color: currently used color
  * @retval Error = -1, Success = 0
  */
int ILI9341_WriteChar(char ch, ILI9341_FontDef font, uint16_t x, uint16_t y, uint16_t color, uint16_t background_color)
{
    uint16_t pix_row;

    // Check if character declared in the array
    if (ch < font.ascii_start_num || ch > font.ascii_end_num)
    {
        return -1;
    }

    // Check remaining space on current line
    if (display_width < (x + font.width) ||
        display_height < (y + font.height) )
    {
        // Not enough space on current line
        return -1;
    }

    ILI9341_SetWindow(x, y, x + font.width - 1, y + font.height - 1);

    //Write Color
    ILI9341_WriteCommand(ILI9341_RAMWR);

    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_RESET); //Select OLED
    HAL_GPIO_WritePin(p_dc->gpio, p_dc->pin, GPIO_PIN_SET);   //Data mode

    uint16_t n = 0;

    for (uint16_t i = 0; i < font.height; i++)
    {
        pix_row = font.data[(ch - font.ascii_start_num) * font.height + i];

        for (uint16_t j = 0; j < font.width; j++)
        {
            if ((pix_row << j) & 0x8000)
            {
                buff[n++] = color << 8 | color >> 8;
            }
            else
            {
                buff[n++] = background_color << 8 | background_color >> 8;
            }

            if (n == DISPLAY_MAX_BUFFER_SIZE)
            {
                ILI9341_SetColors(buff, n << 1, true);
                n = 0;
            }
        }
    }

    ILI9341_SetColors(buff, n << 1, true);

    HAL_GPIO_WritePin(p_cs->gpio, p_cs->pin, GPIO_PIN_SET);   //Un-select OLED

    return 0;
}

/**
  * @brief Write string to the screen buffer
  * @param str: ASCII char string buffer
  *        font: currently used font
  *        color: currently used color
  * @retval last not transferred character in the case of the error
  */
char ILI9341_WriteString(char* str, ILI9341_FontDef font, uint16_t x, uint16_t y, uint16_t color, uint16_t background_color)
{
    while (*str != '\0')
    {
        if (ILI9341_WriteChar(*str, font, x, y, color, background_color) != 0)
        {
            // Char that wasn't written
            return *str;
        }
        ++str;
        x += font.width;
    }

    // Everything ok
    return *str;
}
