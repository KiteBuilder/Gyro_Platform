/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ILI9341_INCLUDE_FONT_6x8
#define ILI9341_INCLUDE_FONT_7x10
#define ILI9341_INCLUDE_FONT_11x18
#define ILI9341_INCLUDE_FONT_16x26
#include "ILI9341_Driver.h"
#include "Graph.h"
#include "Filter.h"
#include "TouchScreen.h"
#include "DebugProtocol.h"
#include "Time.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
timeUs_t currentTimeUs = 0, previousTimeUs = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
static void InitGraphInterface();
static void GraphsAndTextUpdate(timeDelta_t);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
ILI9341_Port cs  = {TFT_CS_GPIO_Port , TFT_CS_Pin };
ILI9341_Port dc  = {TFT_DC_GPIO_Port , TFT_DC_Pin };
ILI9341_Port rst = {TFT_RST_GPIO_Port, TFT_RST_Pin};
ILI9341_Port led = {TFT_LED_GPIO_Port, TFT_LED_Pin};

Touch_Port touch_cs  = {TOUCH_CS_GPIO_Port , TOUCH_CS_Pin };
Touch_Port touch_int  = {TOUCH_INT_GPIO_Port , TOUCH_INT_Pin };

#ifdef ILI9341_INCLUDE_FONT_6x8
ILI9341_FontDef Font_6x8 = {6, 8, Font6x8, 32, 126};
#endif

#ifdef ILI9341_INCLUDE_FONT_7x10
ILI9341_FontDef Font_7x10 = {7, 10, Font7x10, 32, 126};
#endif

#ifdef ILI9341_INCLUDE_FONT_11x18
ILI9341_FontDef Font_11x18 = {11, 18, Font11x18, 32, 126};
#endif

#ifdef ILI9341_INCLUDE_FONT_16x26
ILI9341_FontDef Font_16x26 = {16, 26, Font16x26, 32, 126};
#endif

rect_t vBat_wnd, iBat_wnd, resBat_wnd, text_wnd;
point_t vBat_hdr, iBat_hdr, resBat_hdr;

graph_t vBat_graph, iBat_graph, iFilt_graph, resBat_graph, vSag_graph;

pt1Filter_t filter;

bool f_touch;
uint16_t guard_cnt;
const uint16_t guard_threshold = 500;
uint16_t x, y;
uint8_t txt_wnd_num;

float fltData[RX_MAX_CNT/sizeof(float)];

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  f_touch = false;
  guard_cnt = 0;
  txt_wnd_num = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  usTicks = SystemCoreClock / 1000000;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  ILI9341_Set_Interface(&hspi2, true, &cs, &dc, &rst, &led);
  ILI9341_BackLight(true);
  ILI9341_Init();
  ILI9341_SetOrientation(SCREEN_VERTICAL_180GRAD);//SCREEN_HORIZONTAL_180GRAD);
  ILI9341_Clear(Black);

  Touch_Set_Interface(&hspi1, &touch_cs, &touch_int);

  InitGraphInterface();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  Debug_InitProtocol(&huart1, fltData);

  while (1)
  {
      if (Debug_IsRxready())
      {
          currentTimeUs = micros();
          timeDelta_t dT = currentTimeUs - previousTimeUs;
          previousTimeUs = currentTimeUs;
          GraphsAndTextUpdate(dT);
      }

      if (f_touch == true)
      {
          f_touch = false;

          if (y < text_wnd.bottom)
          {
              ILI9341_DrawFillRectangle(text_wnd.left, text_wnd.top, text_wnd.right, text_wnd.bottom, Black);
             if (++txt_wnd_num > 1)
             {
                 txt_wnd_num = 0;
             }
          }
      }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TOUCH_CS_GPIO_Port, TOUCH_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TFT_DC_Pin|TFT_RST_Pin|TFT_CS_Pin|TFT_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : TOUCH_CS_Pin */
  GPIO_InitStruct.Pin = TOUCH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(TOUCH_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TOUCH_INT_Pin */
  GPIO_InitStruct.Pin = TOUCH_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(TOUCH_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TFT_DC_Pin TFT_RST_Pin TFT_CS_Pin TFT_LED_Pin */
  GPIO_InitStruct.Pin = TFT_DC_Pin|TFT_RST_Pin|TFT_CS_Pin|TFT_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief UART RX complete callback
  * @param None
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    Debug_RxCpltCallback(huart);
}

/**
  * @brief
  * @retval None
  */
void HAL_SYSTICK_Callback()
{
    if(guard_cnt != 0)
    {
        --guard_cnt;
    }
}

/**
  * @brief
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == touch_int.pin)
    {
        if (guard_cnt == 0 && f_touch == false)
        {
            if (HAL_GPIO_ReadPin(touch_int.gpio, touch_int.pin) == GPIO_PIN_RESET)
            {
                Touch_Get_Coordinates(&x, &y, true);
                f_touch = true;
            }

            guard_cnt =  guard_threshold;
        }
    }
}

/**
  * @brief  To draw two windows for the Graphs
  * @retval None
  */
static void InitGraphInterface()
{
    char str[32];
    const uint16_t wnd_height = 130;
    const uint16_t y_offset= 70;

    vBat_hdr.x = 10; vBat_hdr.y = y_offset - Font_7x10.height;
    sprintf(str, "iBat(0-30)A");
    ILI9341_WriteString(str, Font_7x10, vBat_hdr.x, vBat_hdr.y, Yellow, Red);

    iBat_hdr.x = 100; iBat_hdr.y = y_offset - Font_7x10.height;
    sprintf(str, "vBat(15-30)V");
    ILI9341_WriteString(str, Font_7x10, iBat_hdr.x, iBat_hdr.y, Blue, Green);

    resBat_hdr.x = 200; resBat_hdr.y = y_offset - Font_7x10.height;
    sprintf(str, "resBat(0-150)mOmh");
    ILI9341_WriteString(str, Font_7x10, resBat_hdr.x, resBat_hdr.y, Green, Blue);

    iBat_wnd.left   = 1;
    iBat_wnd.top    = y_offset;
    iBat_wnd.right  = 318;
    iBat_wnd.bottom = iBat_wnd.top + wnd_height;
    Graph_InitDynamic(&iBat_wnd, &iFilt_graph, 0, 3000, Yellow, Black);
    Graph_InitDynamic(&iBat_wnd, &iBat_graph, 0, 3000, Red, Black);

    vBat_wnd.left   = 1;
    vBat_wnd.top    = iBat_wnd.bottom + 5;
    vBat_wnd.right  = 318;
    vBat_wnd.bottom = vBat_wnd.top + wnd_height;
    Graph_InitDynamic(&vBat_wnd, &vSag_graph, 150, 300, Cyan, Black);
    Graph_InitDynamic(&vBat_wnd, &vBat_graph, 150, 300, Green, Black);

    resBat_wnd.left   = 1;
    resBat_wnd.top    = vBat_wnd.bottom + 5;
    resBat_wnd.right  = 318;
    resBat_wnd.bottom = resBat_wnd.top + wnd_height;
    Graph_InitDynamic(&resBat_wnd, &resBat_graph, 0, 150, Blue, Black);

    text_wnd.left = 0; text_wnd.right = 320; text_wnd.top = 0; text_wnd.bottom = Font_16x26.height * 2;
}

/**
  * @brief  To plot graphs and update text information
  * @retval None
  */
static void GraphsAndTextUpdate(timeDelta_t dT)
{
    char str[32];
    point_t point;
    uint16_t data;
    char sign;

    //IBat
    pt1FilterApply4(&filter, fltData[0], 0.5,US2S(dT));

    point.x = 0; point.y = Font_16x26.height + 5;
    data = (uint16_t)(fabs(filter.state) * 100);
    sign  = (filter.state < 0) ? '-': '+';
    sprintf(str, "I%c%2d.%02d", sign, data/100, data % 100);
    ILI9341_WriteString(str, Font_16x26, point.x, point.y, Red, Black);

    if (txt_wnd_num == 0)
    {
        //vBat
        point.x = 0; point.y = 0;
        data = (uint16_t)(fltData[1] * 10);
        sprintf(str, "V%2d.%1d", data/10, data % 10);
        ILI9341_WriteString(str, Font_16x26, point.x, point.y, Green, Black);

        //Battery Impedance
        point.x = Font_16x26.width * 6; point.y = 0;
        data = (uint16_t)(fltData[4] * 10000);
        sprintf(str, "R%3d.%1d", data/10, data % 10);
        ILI9341_WriteString(str, Font_16x26, point.x, point.y, Blue, Black);

        //Capacity mAh
        point.x = Font_16x26.width * 8; point.y = Font_16x26.height + 5;
        sign  = (fltData[2] < 0) ? '-': '+';
        data = (uint16_t)(fabs(fltData[2]) * 10);
        sprintf(str, "%c%4d.%1dmAh", sign, data/10, data % 10);
        ILI9341_WriteString(str, Font_16x26, point.x, point.y, Orange, Black);

        //Capacity Wh
        point.x = Font_16x26.width * 12; point.y = 0;
        sign  = (fltData[3] < 0) ? '-': '+';
        data = (uint16_t)(fabs(fltData[3]) * 100);
        sprintf(str, "%c%2d.%02dWh", sign, data/100, data % 100);
        ILI9341_WriteString(str, Font_16x26, point.x, point.y, Magenta, Black);
    }
    else
    {
        //vRest
        point.x = 0; point.y = 0;
        data = (uint16_t)(fltData[7] * 10);
        sprintf(str, "V%2d.%1d", data/10, data % 10);
        ILI9341_WriteString(str, Font_16x26, point.x, point.y, Yellow, Black);

        //Battery temperature
        point.x = Font_16x26.width * 6; point.y = 0;
        data = (uint16_t)(fltData[8] * 10);
        sprintf(str, "t%2d.%1d", data/10, data % 10);
        ILI9341_WriteString(str, Font_16x26, point.x, point.y, Olive, Black);

        //Capacity module
        point.x = Font_16x26.width * 13; point.y = 0;
        data = (uint16_t)(fabs(fltData[6]) * 10);
        sprintf(str, "M%4d.%1d", data/10, data % 10);
        ILI9341_WriteString(str, Font_16x26, point.x, point.y, Purple, Black);

        //Remaining capacity mAh
        point.x = Font_16x26.width * 8; point.y = Font_16x26.height + 5;
        sign  = (fltData[5] < 0) ? '-': '+';
        data = (uint16_t)(fabs(fltData[5]) * 10);
        sprintf(str, "%c%4d.%1dmAh", sign, data/10, data % 10);
        ILI9341_WriteString(str, Font_16x26, point.x, point.y, Cyan, Black);
    }

    int16_t iFilt_int = (int16_t)(filter.state * 100);
    Graph_DynamicDraw(iFilt_int, &iFilt_graph, false);

    int16_t iBat_int = (int16_t)(fltData[0] * 100);
    Graph_DynamicDraw(iBat_int, &iBat_graph, true);

    int16_t vSag_int = (int16_t)(fltData[7] * 10);
    Graph_DynamicDraw(vSag_int, &vSag_graph, false);

    int16_t vBat_int = (int16_t)(fltData[1] * 10);
    Graph_DynamicDraw(vBat_int, &vBat_graph, true);

    int16_t resBat_int = (int16_t)(fltData[4] * 1000);
    Graph_DynamicDraw(resBat_int, &resBat_graph, true);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
