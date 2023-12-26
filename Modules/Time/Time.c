/*
 * Time.c
 *
 *  Created on: Dec 26, 2023
 *      Author: KiteBuilder
 */
#include "stm32f4xx_hal.h"
#include "time.h"

static volatile timeMs_t sysTickUptime = 0;
static volatile uint32_t sysTickValStamp = 0;

static volatile int sysTickPending = 0;

uint32_t usTicks = 0;

/**
  * @brief
  * @retval None
  */
void SYSTICK_Callback()
{
    ATOMIC_BLOCK(NVIC_PRIO_MAX)
    {
        sysTickUptime++;
        sysTickValStamp = SysTick->VAL;
        sysTickPending = 0;
        (void)(SysTick->CTRL);
    }
}

/**
  * @brief Return system uptime in microseconds
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

    const uint32_t partial = (usTicks * 1000U - cycle_cnt) / usTicks;
    return ((timeUs_t)ms * 1000LL) + ((timeUs_t)partial);
}

/**
  * @brief Delay in microseconds
  * @param us: time in microseconds
  * @retval None
  */
void delayUs(timeUs_t us)
{
    timeUs_t now = micros();
    while (micros() - now < us);
}

/**
  * @brief Delay in milliseconds
  * @param ms: time in mlliseconds
  * @retval None
  */
void delayMs(timeMs_t ms)
{
    while (ms--)
        delayUs(1000);
}



