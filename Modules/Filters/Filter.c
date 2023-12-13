/*
 * Filter.c
 *
 *  Created on: Dec 13, 2023
 *      Author: KiteBuilder
 */

#include <math.h>
#include "Filter.h"

/**
  * @brief
  * @param None
  * @retval None
  */
static float pt1ComputeRC(const float f_cut) // f_cut = cutoff frequency
{
    return 1.0f / (2.0f * M_PI * f_cut);
}

/**
  * @brief
  * @param None
  * @retval None
  */
void pt1FilterInitRC(pt1Filter_t *filter, float tau, float dT)
{
    filter->state = 0.0f;
    filter->RC = tau;
    filter->dT = dT;
    filter->alpha = filter->dT / (filter->RC + filter->dT);
}

/**
  * @brief
  * @param None
  * @retval None
  */
void pt1FilterInit(pt1Filter_t *filter, float f_cut, float dT)
{
    pt1FilterInitRC(filter, pt1ComputeRC(f_cut), dT);
}

/**
  * @brief
  * @param None
  * @retval None
  */
float  pt1FilterApply(pt1Filter_t *filter, float input)
{
    filter->state = filter->state + filter->alpha * (input - filter->state);
    return filter->state;
}

/**
  * @brief
  * @param None
  * @retval None
  */
float pt1FilterApply3(pt1Filter_t *filter, float input, float dT)
{
    filter->dT = dT;
    filter->state = filter->state + dT / (filter->RC + dT) * (input - filter->state);
    return filter->state;
}

/**
  * @brief
  * @param None
  * @retval None
  */
float pt1FilterApply4(pt1Filter_t *filter, float input, float f_cut, float dT)
{
    // Pre calculate and store RC
    if (!filter->RC) {
        filter->RC = pt1ComputeRC(f_cut);
    }

    filter->dT = dT;    // cache latest dT for possible use in pt1FilterApply
    filter->alpha = filter->dT / (filter->RC + filter->dT);
    filter->state = filter->state + filter->alpha * (input - filter->state);
    return filter->state;
}
