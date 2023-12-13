/*
 * ILI9341_Driver.h
 *
 *  Created on: Dec 5, 2023
 *      Author: KiteBuilder
 */

#ifndef ILI9341_DRIVER_H_
#define ILI9341_DRIVER_H_

#include <stm32f4xx_hal.h>
#include <stdbool.h>
#include <stdint.h>
#include "ILI9341_Fonts.h"

#define DISPLAY_PIX_WIDTH       480 //display width in pixels
#define DISPLAY_PIX_HEIGHT      320 //display height in pixels

#define DISPLAY_MAX_BUFFER_SIZE 1024 //Size of buffer for direct write

//ILI9341 Registers
#define ILI9341_NOP         0x00 //No-op register
#define ILI9341_SWRESET     0x01 // Software reset register
#define ILI9341_RDDID       0x04 // Read display identification information
#define ILI9341_RDDST       0x09 // Read Display Status

#define ILI9341_SLPIN       0x10 // Enter Sleep Mode
#define ILI9341_SLPOUT      0x11 // Sleep Out
#define ILI9341_PTLON       0x12 // Partial Mode ON
#define ILI9341_NORON       0x13 // Normal Display Mode ON

#define ILI9341_RDMODE      0x0A // Read Display Power Mode
#define ILI9341_RDMADCTL    0x0B // Read Display MADCTL
#define ILI9341_RDPIXFMT    0x0C // Read Display Pixel Format
#define ILI9341_RDIMGFMT    0x0D // Read Display Image Format
#define ILI9341_RDSELFDIAG  0x0F // Read Display Self-Diagnostic Result

#define ILI9341_INVOFF      0x20 // Display Inversion OFF
#define ILI9341_INVON       0x21 // Display Inversion ON
#define ILI9341_GAMMASET    0x26 // Gamma Set
#define ILI9341_DISPOFF     0x28 // Display OFF
#define ILI9341_DISPON      0x29 // Display ON

#define ILI9341_CASET       0x2A // Column Address Set
#define ILI9341_PASET       0x2B // Page Address Set
#define ILI9341_RAMWR       0x2C // Memory Write
#define ILI9341_RAMRD       0x2E // Memory Read

#define ILI9341_PTLAR       0x30 // Partial Area
#define ILI9341_VSCRDEF     0x33 // Vertical Scrolling Definition
#define ILI9341_MADCTL      0x36 // Memory Access Control
#define MADCTL_MY   0x80 // Bottom to top
#define MADCTL_MX   0x40 // Right to left
#define MADCTL_MV   0x20 // Reverse Mode
#define MADCTL_ML   0x10 // LCD refresh Bottom to top
#define MADCTL_RGB  0x00 // Red-Green-Blue pixel order
#define MADCTL_BGR  0x08 // Blue-Green-Red pixel order
#define MADCTL_MH   0x04 // LCD refresh right to left
#define ILI9341_VSCRSADD    0x37 // Vertical Scrolling Start Address
#define ILI9341_PIXFMT      0x3A // COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1     0xB1 // Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2     0xB2 // Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3     0xB3 // Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR      0xB4 // Display Inversion Control
#define ILI9341_DFUNCTR     0xB6 // Display Function Control

#define ILI9341_PWCTR1      0xC0 // Power Control 1
#define ILI9341_PWCTR2      0xC1 // Power Control 2
#define ILI9341_PWCTR3      0xC2 // Power Control 3
#define ILI9341_PWCTR4      0xC3 // Power Control 4
#define ILI9341_PWCTR5      0xC4 // Power Control 5
#define ILI9341_VMCTR1      0xC5 // VCOM Control 1
#define ILI9341_VMCTR2      0xC7 // VCOM Control 2
#define ILI9341_PWCTRA      0xCB // Power Control A
#define ILI9341_PWCTRB      0xCF // Power Control B

#define ILI9341_RDID1       0xDA // Read ID 1
#define ILI9341_RDID2       0xDB // Read ID 2
#define ILI9341_RDID3       0xDC // Read ID 3
#define ILI9341_RDID4       0xDD // Read ID 4

#define ILI9341_GMCTRP1     0xE0 // Positive Gamma Correction
#define ILI9341_GMCTRN1     0xE1 // Negative Gamma Correction
#define ILI9341_DRVTIMCTRA  0xE8 //Driver timing control A
#define ILI9341_DRVTIMCTRB  0xEA //Driver timing control B
#define ILI9341_PWRSEQCTR   0xED //Power on sequence control

#define ILI9341_3GFUNDIS    0xF2 //3Gamma Function Disable
#define ILI9341_PUMPRTCTR   0xF7 //Pump ratio control

//Some RGB color definitions

#define RGB(r,g,b)  ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3) ) //5 red | 6 green | 5 blue

#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define White           0xFFFF      /* 255, 255, 255 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */

//Screen orientation
typedef enum {SCREEN_VERTICAL_0GRAD = 0, SCREEN_HORIZONTAL_0GRAD = 1, SCREEN_VERTICAL_180GRAD = 2, SCREEN_HORIZONTAL_180GRAD = 3} ILI9341_ORIENTATION;

//ILI9341 port structure
typedef struct{
    GPIO_TypeDef* gpio;
    uint16_t pin;
} ILI9341_Port;

//ILI9341 API set
uint16_t ILI9341_GetHeight(void);
uint16_t ILI9341_GetWidth(void);
void ILI9341_Set_Interface(SPI_HandleTypeDef*, bool, ILI9341_Port*, ILI9341_Port*, ILI9341_Port*, ILI9341_Port*);
void ILI9341_BackLight(bool);
void ILI9341_Init(void);
void ILI9341_SetOrientation(ILI9341_ORIENTATION);
void ILI9341_Clear(uint16_t);
int ILI9341_DrawPixel(uint16_t, uint16_t, uint16_t);
void ILI9341_DrawLine(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
void ILI9341_DrawRectangle(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
void ILI9341_DrawFillRectangle(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
void ILI9341_DrawCircle(uint16_t, uint16_t, uint16_t, uint16_t);
void ILI9341_DrawFillCircle(uint16_t, uint16_t, uint16_t, uint16_t);
int ILI9341_WriteChar(char, ILI9341_FontDef, uint16_t, uint16_t, uint16_t, uint16_t);
char ILI9341_WriteString(char*, ILI9341_FontDef, uint16_t, uint16_t, uint16_t, uint16_t);

#endif /* ILI9341_DRIVER_H_ */
