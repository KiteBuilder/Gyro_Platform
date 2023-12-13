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
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;

TIM_HandleTypeDef htim10;

/* USER CODE BEGIN PV */
static volatile timeMs_t sysTickUptime = 0;
static volatile uint32_t sysTickValStamp = 0;

timeUs_t currentTimeUs = 0, previousTimeUs = 0;
uint32_t usTicks = 0;

timeUs_t micros(void);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM10_Init(void);
static void MX_SPI2_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
static void Init_GraphInterface();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
ILI9341_Port cs  = {TFT_CS_GPIO_Port , TFT_CS_Pin };
ILI9341_Port dc  = {TFT_DC_GPIO_Port , TFT_DC_Pin };
ILI9341_Port rst = {TFT_RST_GPIO_Port, TFT_RST_Pin};
ILI9341_Port led = {TFT_LED_GPIO_Port, TFT_LED_Pin};

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

rect_t raw_wnd, filt_wnd;
point_t raw_hdr, filt_hdr;

graph_t raw_graph, filt_graph;

bool  f_measure;
uint16_t n_samples;
uint16_t max_samples;

pt1Filter_t filter;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  f_measure =  false;
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
  MX_TIM10_Init();
  MX_SPI2_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  ILI9341_Set_Interface(&hspi2, true, &cs, &dc, &rst, &led);
  ILI9341_BackLight(true);
  ILI9341_Init();
  ILI9341_SetOrientation(SCREEN_HORIZONTAL_180GRAD);
  ILI9341_Clear(Black);

  Init_GraphInterface();

  HAL_TIM_Base_Start_IT(&htim10);

  srand(1984);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint16_t n = 0;

  pt1FilterInit(&filter, 10, US2S(TASK_PERIOD_HZ(1000)) );

  while (1)
  {

      if (f_measure == true)
      {
          f_measure = false;

          float noise = (float)rand() / (float)(1000000000);
          noise /= 4;
          float raw_val, filt_val;
          int16_t raw_int, filt_int;

          currentTimeUs = micros();

          raw_val =  (sin( (n * 1.0) / 180 * M_PI) + noise);
          raw_int = (int16_t)(raw_val * 100);
          Graph_DynamicDraw(raw_int, &raw_graph);

          float dT = (currentTimeUs - previousTimeUs) * 1e-6;
          previousTimeUs = currentTimeUs;

          filt_val = pt1FilterApply3(&filter, raw_val, dT); //pt1FilterApply4(&filter, raw_val, 10, dT);  //pt1FilterApply(&filter, raw_val);
          filt_int = (int16_t)(filt_val * 100);
          Graph_DynamicDraw(filt_int, &filt_graph);

          if (n_samples < max_samples-1)
          {
              ++n_samples;
              ++n;
          }
          else
          {
              n_samples = 0;
              //n = 0;
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
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* TIM1_UP_TIM10_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
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
  * @brief TIM10 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 24;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 999;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TFT_DC_Pin|TFT_RST_Pin|TFT_CS_Pin|TFT_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : TFT_DC_Pin TFT_RST_Pin TFT_CS_Pin TFT_LED_Pin */
  GPIO_InitStruct.Pin = TFT_DC_Pin|TFT_RST_Pin|TFT_CS_Pin|TFT_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief
  * @retval None
  */
void HAL_SYSTICK_Callback()
{
    ATOMIC_BLOCK(NVIC_PRIO_MAX)
    {
        sysTickUptime++;
        sysTickValStamp = SysTick->VAL;
        (void)(SysTick->CTRL);
    }

}

/**
  * @brief
  * @retval None
  */
timeUs_t micros(void)
{
    register uint32_t ms, cycle_cnt;

    do
    {
        ms = sysTickUptime;
        cycle_cnt = SysTick->VAL;
    } while (ms != sysTickUptime || cycle_cnt > sysTickValStamp);

    // XXX: Be careful to not trigger 64 bit division
    const uint32_t partial = (usTicks * 1000U - cycle_cnt) / usTicks;
    return ((timeUs_t)ms * 1000LL) + ((timeUs_t)partial);
}

/**
  * @brief Timer tim10 elapsed period callback
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM10)
    {
        f_measure = true;
    }
}

/**
  * @brief  To draw two windows for the Raw and Filtered data
  * @retval None
  */
static void Init_GraphInterface()
{
    char str[32];

    raw_hdr.x = 10; raw_hdr.y = 0;
    sprintf(str, "Raw data");
    ILI9341_WriteString(str, Font_11x18, raw_hdr.x, raw_hdr.y, Yellow, Red);

    filt_hdr.x = 10; filt_hdr.y = 160;
    sprintf(str, "Filtered data");
    ILI9341_WriteString(str, Font_11x18, filt_hdr.x, filt_hdr.y, Blue, Green);

    raw_wnd.left   = 1;
    raw_wnd.top    = 20;
    raw_wnd.right  = 470;
    raw_wnd.bottom = 150;
    Graph_InitDynamic(&raw_wnd, &raw_graph, -110, 160, Red, Black);

    filt_wnd.left   = 1;
    filt_wnd.top    = 180;
    filt_wnd.right  = 470;
    filt_wnd.bottom = 310;
    Graph_InitDynamic(&filt_wnd, &filt_graph, -110, 160, Green, Black);

    max_samples = raw_wnd.right - raw_wnd.left;
    n_samples = 0;
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
