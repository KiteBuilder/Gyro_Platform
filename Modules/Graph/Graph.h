/*
 * Graph.h
 *
 *  Created on: Dec 12, 2023
 *      Author: KiteBuilder
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct _rect_st_
{
    uint16_t left;
    uint16_t top;
    uint16_t right;
    uint16_t bottom;
} rect_t;

typedef struct _point_st_
{
    uint16_t x;
    uint16_t y;
} point_t;

typedef struct _graph_st_
{
    point_t p_buff[512];
    uint16_t n_sample;   //should be set to 0 during initialization
    uint16_t max_sample; //calculated as wnd.right - wnd.left
    rect_t graph_wnd;
    int16_t min_y;
    int16_t max_y;
    uint16_t color;
    uint16_t back_color;
    bool f_redraw;      //changed automatically in Graph_DynamicDraw
} graph_t;


point_t Graph_CalcXY(rect_t, uint16_t, int16_t, int16_t, int16_t);
void Graph_Draw(int16_t*, uint16_t, uint16_t, rect_t, uint16_t);
void Graph_DynamicDraw(int16_t, graph_t*, bool);
void Graph_InitDynamic(rect_t*, graph_t*, int16_t, int16_t, uint16_t, uint16_t);


#endif /* GRAPH_H_ */
