/*
 * Filter.h
 *
 *  Created on: Dec 13, 2023
 *      Author: KiteBuilder
 */

#ifndef FILTER_H_
#define FILTER_H_

#define US2S(us)    ((us) * 1e-6f)
#define TASK_PERIOD_HZ(hz) (1000000 / (hz))

typedef struct pt1Filter_s {
    float state;
    float RC;
    float dT;
    float alpha;
} pt1Filter_t;

void pt1FilterInit(pt1Filter_t *filter, float f_cut, float dT);
float pt1FilterApply(pt1Filter_t *filter, float input);
float pt1FilterApply3(pt1Filter_t *filter, float input, float dT);
float pt1FilterApply4(pt1Filter_t *filter, float input, float f_cut, float dT);

#endif /* FILTER_H_ */
