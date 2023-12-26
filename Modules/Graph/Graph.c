/*
 * Graph.c
 *
 *  Created on: Dec 12, 2023
 *      Author: KiteBuilder
 */

#include "Graph.h"
#include "ILI9341_Driver.h"


point_t dyn_buff[DISPLAY_PIX_WIDTH];


/**
  * @brief
  * @retval None
  */
point_t Graph_CalcXY(rect_t rect, uint16_t x, int16_t y, int16_t ymin, int16_t ymax)
{
    point_t pt;

    pt.x = rect.left + x;
    pt.y = rect.top + (int)(((rect.bottom - rect.top) * (ymax - y)) / (ymax - ymin));

    return pt;
}

/**
  * @brief
  * @param None
  * @retval None
  */
void Graph_Draw(int16_t *data_buff, uint16_t min_y, uint16_t max_y, rect_t graph_wnd, uint16_t color)
{
    point_t graph_buff[2];
    uint16_t samples = graph_wnd.right - graph_wnd.left;

    for (uint16_t i = 0; i < samples - 1; i++)
    {
        graph_buff[0] = Graph_CalcXY(graph_wnd, i, data_buff[i], min_y, max_y);
        graph_buff[1] = Graph_CalcXY(graph_wnd, i + 1, data_buff[i + 1], min_y, max_y);

        ILI9341_DrawLine(graph_buff[0].x, graph_buff[0].y, graph_buff[1].x, graph_buff[1].y, color);
    }
}

/**
  * @brief
  * @param None
  * @retval None
  */
void Graph_InitDynamic(rect_t *wnd, graph_t *graph, int16_t min_y, int16_t max_y, uint16_t color, uint16_t back_color)
{
    graph->graph_wnd = *wnd;
    graph->n_sample = 0;
    graph->max_sample = wnd->right - wnd->left;
    graph->min_y = min_y;
    graph->max_y = max_y;
    graph->color = color;
    graph->back_color = back_color;
    graph->f_redraw = false;

    ILI9341_DrawRectangle(wnd->left - 1, wnd->top - 1, wnd->right + 1, wnd->bottom + 1, color);
}

/**
  * @brief
  * @param None
  * @retval None
  */
void Graph_DynamicDraw(int16_t data, graph_t *graph)
{
    if (graph->n_sample == graph->max_sample)
    {
        graph->f_redraw = true;
        graph->n_sample = 0;
    }

    if (graph->f_redraw == true && graph->n_sample < graph->max_sample - 1)
    {
        ILI9341_DrawLine(graph->p_buff[graph->n_sample].x, graph->p_buff[graph->n_sample].y, graph->p_buff[graph->n_sample + 1].x, graph->p_buff[graph->n_sample + 1].y, graph->back_color);
    }

    if (data > graph->max_y)
    {
        data = graph->max_y;
    }
    else if (data < graph->min_y)
    {
        data = graph->min_y;
    }

    graph->p_buff[graph->n_sample] = Graph_CalcXY(graph->graph_wnd, graph->n_sample, data, graph->min_y, graph->max_y);

    if (graph->n_sample != 0)
    {
        ILI9341_DrawLine(graph->p_buff[graph->n_sample - 1].x, graph->p_buff[graph->n_sample - 1].y, graph->p_buff[graph->n_sample].x, graph->p_buff[graph->n_sample].y, graph->color);

        //To draw the update marker line
        ILI9341_DrawLine(graph->p_buff[graph->n_sample].x + 1, graph->graph_wnd.top, graph->p_buff[graph->n_sample].x + 1, graph->graph_wnd.bottom, graph->back_color);
        ILI9341_DrawLine(graph->p_buff[graph->n_sample].x + 2, graph->graph_wnd.top, graph->p_buff[graph->n_sample].x + 2, graph->graph_wnd.bottom, graph->color);
    }

    ++graph->n_sample;
}

