/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018, 2019.
 *
 * Released under the GPL License, Version 3
 */

//#include <math.h>
#include "graphs.h"

#include <string.h>
#include "stm32f10x.h"
#include "stdio.h"
#include "main.h"
#include "config.h"
#include "ugui_driver/ugui_bafang_500c.h"
#include "ugui/ugui.h"
#include "lcd.h"

uint32_t ui32_array_data[255 * 4] =
{
    239 ,
    241 ,
    242 ,
    243 ,
    245 ,
    246 ,
    247 ,
    248 ,
    249 ,
    250 ,
    251 ,
    251 ,
    252 ,
    253 ,
    253 ,
    254 ,
    254 ,
    254 ,
    255 ,
    255 ,
    255 ,
    255 ,
    255 ,
    255 ,
    254 ,
    254 ,
    254 ,
    253 ,
    253 ,
    252 ,
    251 ,
    250 ,
    250 ,
    249 ,
    248 ,
    247 ,
    245 ,
    244 ,
    243 ,
    242 ,
    240 ,
    239 ,
    236 ,
    231 ,
    227 ,
    222 ,
    217 ,
    212 ,
    207 ,
    202 ,
    197 ,
    191 ,
    186 ,
    181 ,
    176 ,
    170 ,
    165 ,
    160 ,
    154 ,
    149 ,
    144 ,
    138 ,
    133 ,
    127 ,
    122 ,
    116 ,
    111 ,
    106 ,
    100 ,
    95  ,
    89  ,
    84  ,
    79  ,
    74  ,
    68  ,
    63  ,
    58  ,
    53  ,
    48  ,
    43  ,
    38  ,
    33  ,
    28  ,
    23  ,
    18  ,
    16  ,
    14  ,
    13  ,
    12  ,
    10  ,
    9 ,
    8 ,
    7 ,
    6 ,
    5 ,
    4 ,
    3 ,
    3 ,
    2 ,
    1 ,
    1 ,
    1 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    1 ,
    1 ,
    2 ,
    2 ,
    3 ,
    4 ,
    5 ,
    6 ,
    6 ,
    8 ,
    9 ,
    10  ,
    11  ,
    12  ,
    14  ,
    15  ,
    17  ,
    15  ,
    14  ,
    12  ,
    11  ,
    10  ,
    9 ,
    8 ,
    6 ,
    6 ,
    5 ,
    4 ,
    3 ,
    2 ,
    2 ,
    1 ,
    1 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    0 ,
    1 ,
    1 ,
    1 ,
    2 ,
    3 ,
    3 ,
    4 ,
    5 ,
    6 ,
    7 ,
    8 ,
    9 ,
    10  ,
    12  ,
    13  ,
    14  ,
    16  ,
    18  ,
    23  ,
    28  ,
    33  ,
    38  ,
    43  ,
    48  ,
    53  ,
    58  ,
    63  ,
    68  ,
    74  ,
    79  ,
    84  ,
    89  ,
    95  ,
    100 ,
    106 ,
    111 ,
    116 ,
    122 ,
    127 ,
    133 ,
    138 ,
    144 ,
    149 ,
    154 ,
    160 ,
    165 ,
    170 ,
    176 ,
    181 ,
    186 ,
    191 ,
    197 ,
    202 ,
    207 ,
    212 ,
    217 ,
    222 ,
    227 ,
    231 ,
    236 ,
    239 ,
    240 ,
    242 ,
    243 ,
    244 ,
    245 ,
    247 ,
    248 ,
    249 ,
    250 ,
    250 ,
    251 ,
    252 ,
    253 ,
    253 ,
    254 ,
    254 ,
    254 ,
    255 ,
    255 ,
    255 ,
    255 ,
    255 ,
    255 ,
    254 ,
    254 ,
    254 ,
    253 ,
    253 ,
    252 ,
    251 ,
    251 ,
    250 ,
    249 ,
    248 ,
    247 ,
    246 ,
    245 ,
    243 ,
    242 ,
    241 ,
    239 ,
    238 ,
};

// 255 pixels for data points
// 255 points of each graph
// 60 minutes: 255 * (3500 * 4) ms
// 30 minutes: 255 * (3500 * 2) ms
// 15 minutes: 255 * 3500 ms

graphs_t graphs[5];

// graph ID based on array index number
typedef enum
{
  BATTERY_VOLTAGE = 0,
  BATTERY_CURRENT = 1,
  PEDAL_CADENCE = 2,
  PEDAL_TORQUE = 3,
  PEDAL_HUMAN_POWER = 4
} graph_id_t;

volatile uint32_t ui32_m_graphs_data_array_over_255 = 0;
static uint32_t ui32_m_refresh_graphs = 1;
static uint32_t m_first_line = 1;

void graphs_measurements_calc_min_max_y(void);
static void graphs_measurements_search_max_y(uint32_t graph_nr);
static void graphs_measurements_search_min_y(uint32_t graph_nr);

void graphs_draw(void)
{
  uint16_t i;
  uint32_t number_lines_to_draw;
  uint32_t y_amplitude;
  uint32_t graph_next_start_x;
  uint32_t x_index;

  if(m_first_line)
  {
    graphs[0].ui32_data_y_last_value = graphs[0].ui32_data[0];
  }

  // calc new min and max values
  graphs_measurements_calc_min_max_y();

  // calc new pixel ratio
  graphs[0].ui32_data_y_rate_per_pixel_x100 = (GRAPH_Y_LENGHT * 100) /
      (graphs[0].ui32_graph_data_y_max - graphs[0].ui32_graph_data_y_min);


  // finally store the new value
//  graphs[0].ui32_data[graphs[0].ui32_data_last_index] = graphs[0].ui32_data_y_last_value;


  // now increment the x_last_index and keep at 255 at ui32_data_y_last_valuethe end of graph if is the case
//  graphs[0].ui32_draw_x_last_index++;
//  if(graphs[0].ui32_draw_x_last_index >= 255)
//  {
//    graphs[0].ui32_draw_x_last_index = 255;
//  }

//  x_index++;
//  if(x_index >= 255)
//  {
//    x_index = 255;
//  }

//  // calc number of lines to draw
//  if(graphs[0].ui32_data_last_index > graphs[0].ui32_data_start_index)
//  {
//    number_lines_to_draw = graphs[0].ui32_data_last_index - graphs[0].ui32_data_start_index;
//  }
//  else
//  {
//    number_lines_to_draw = 255 - (graphs[0].ui32_data_start_index - graphs[0].ui32_data_last_index);
//  }
//  number_lines_to_draw -= (graphs[0].ui32_x_last_index + 1);

  // draw full lines because the full graph need to be refreshed
  if(ui32_m_graphs_data_array_over_255)
  {
    number_lines_to_draw = 255;
    ui32_m_refresh_graphs = 1;
    x_index = 0;
  }
  // draw only the new 1 line
  else
  {
    number_lines_to_draw = 1;
    x_index = graphs[0].ui32_draw_x_last_index;
  }

  if(ui32_m_refresh_graphs)
  {
    ui32_m_refresh_graphs = 0;

    // calc number of lines to draw
    if(ui32_m_graphs_data_array_over_255 == 0)
    {
      if(graphs[0].ui32_data_last_index >= graphs[0].ui32_data_start_index)
      {
        number_lines_to_draw = graphs[0].ui32_data_last_index - graphs[0].ui32_data_start_index;
      }
      else
      {
        number_lines_to_draw = 255 - (graphs[0].ui32_data_start_index - graphs[0].ui32_data_last_index);
      }

      x_index = 0;
    }

    // clean all lines on the LCD
    UG_FillFrame(GRAPH_START_X, GRAPH_START_Y - 100, 315, GRAPH_START_Y, C_BLACK);
  }

  graph_next_start_x = GRAPH_START_X + x_index;

  // draw only the next line and keep the others intact
  for(i = 0; i < number_lines_to_draw; i++)
  {
    y_amplitude = graphs[0].ui32_data[x_index] - graphs[0].ui32_graph_data_y_min;
    y_amplitude *= graphs[0].ui32_data_y_rate_per_pixel_x100;
    y_amplitude /= 100;

    UG_DrawLine(graph_next_start_x,           // X1
                GRAPH_START_Y,                // Y1
                graph_next_start_x,           // X2
                GRAPH_START_Y - y_amplitude,  // Y2
                C_WHITE);

    x_index++;
    if(x_index >= 255)
    {
      x_index = 255;
    }

    graphs[0].ui32_draw_x_last_index = x_index;
  }

  // find if we are yet drawing the first 255 points
  if(ui32_m_graphs_data_array_over_255 == 0 &&
      graphs[0].ui32_data_last_index >= 255)
  {
    ui32_m_graphs_data_array_over_255 = 1;
  }

  // see if we should increase both index
  if(ui32_m_graphs_data_array_over_255)
  {
    graphs[0].ui32_data_last_index = (graphs[0].ui32_data_last_index + 1) % 256;
    graphs[0].ui32_data_start_index = (graphs[0].ui32_data_start_index + 1) % 256;
  }
  else
  {
    graphs[0].ui32_data_last_index = (graphs[0].ui32_data_last_index + 1) % 256;
  }
}

void graphs_measurements_calc_min_max_y(void)
{
  // we are only yet adding a new point to graph
  if(ui32_m_graphs_data_array_over_255 == 0)
  {
    // equal to min
    if(graphs[0].ui32_data_y_last_value == graphs[0].ui32_graph_data_y_min)
    {
      graphs[0].ui32_graph_data_y_min_counter++;
    }
    // less than min
    else if(graphs[0].ui32_data_y_last_value < graphs[0].ui32_graph_data_y_min)
    {
      graphs[0].ui32_graph_data_y_min = graphs[0].ui32_data_y_last_value;
      graphs[0].ui32_graph_data_y_min_counter = 1;
      ui32_m_refresh_graphs = 1;
    }

    // equal to maxUG_FillFrame(GRAPH_START_X, GRAPH_START_Y - 100, 315, GRAPH_START_Y, C_BLACK);
    if(graphs[0].ui32_data_y_last_value == graphs[0].ui32_graph_data_y_max)
    {
      graphs[0].ui32_graph_data_y_max_counter++;
    }
    // higher than max
    else if(graphs[0].ui32_data_y_last_value > graphs[0].ui32_graph_data_y_max)
    {
      graphs[0].ui32_graph_data_y_max = graphs[0].ui32_data_y_last_value;
      graphs[0].ui32_graph_data_y_max_counter = 1;
      ui32_m_refresh_graphs = 1;
    }
  }

  // force min value on very first line
  if(m_first_line)
  {
    m_first_line = 0;
    graphs[0].ui32_graph_data_y_min = 0;
  }

  // we will be adding and removing points to graph
  else
  {
    // point to add
    //

    // equal to min
    if(graphs[0].ui32_data_y_last_value == graphs[0].ui32_graph_data_y_min)
    {
      graphs[0].ui32_graph_data_y_min_counter++;
    }
    // less than min
    else if(graphs[0].ui32_data_y_last_value < graphs[0].ui32_graph_data_y_min)
    {
      graphs[0].ui32_graph_data_y_min = graphs[0].ui32_data_y_last_value;
      graphs[0].ui32_graph_data_y_min_counter = 1;
    }

    // equal to max
    if(graphs[0].ui32_data_y_last_value == graphs[0].ui32_graph_data_y_max)
    {
      graphs[0].ui32_graph_data_y_max_counter++;
    }
    // higher than max
    else if(graphs[0].ui32_data_y_last_value > graphs[0].ui32_graph_data_y_max)
    {
      graphs[0].ui32_graph_data_y_max = graphs[0].ui32_data_y_last_value;
      graphs[0].ui32_graph_data_y_max_counter = 1;
    }

    // point to remove
    //

    // equal to min
    if(graphs[0].ui32_data[graphs[0].ui32_data_start_index] == graphs[0]      .ui32_graph_data_y_min)
    {
      if(graphs[0].ui32_graph_data_y_min_counter > 1)
      {
        graphs[0].ui32_graph_data_y_min_counter--;
      }
      else
      {
        graphs_measurements_search_min_y(0);
      }
    }

    // equal to max
    if(graphs[0].ui32_data[graphs[0].ui32_data_start_index] == graphs[0].ui32_graph_data_y_max)
    {
      if(graphs[0].ui32_graph_data_y_max_counter > 1)
      {
        graphs[0].ui32_graph_data_y_max_counter--;
      }
      else
      {
        graphs_measurements_search_max_y(0);
      }
    }
  }
}

static void graphs_measurements_search_min_y(uint32_t graph_nr)
{
  uint32_t i;
  uint32_t min = 0xffff;
  uint32_t search_nr_points;
  uint32_t index;

  graphs[graph_nr].ui32_graph_data_y_min = min;

  // calc number of search points
  // -1 to consider that we will remove 1
  if(graphs[graph_nr].ui32_data_last_index > graphs[graph_nr].ui32_data_start_index)
  {
    search_nr_points = (graphs[graph_nr].ui32_data_last_index - graphs[graph_nr].ui32_data_start_index) - 1;
  }
  else
  {
    search_nr_points = (255 - (graphs[graph_nr].ui32_data_start_index - graphs[graph_nr].ui32_data_last_index)) - 1;
  }

  index = graphs[graph_nr].ui32_data_start_index + 1;
  if(index >= 256)
  {
    index = 0;
  }
  //for(i = 0; i < search_nr_points; ++i)
for(i = 0; i < 254; ++i)
  {
    // new min point
    if(graphs[graph_nr].ui32_data[index] < min)
    {
      graphs[graph_nr].ui32_graph_data_y_min = graphs[0].ui32_data[index];
      graphs[graph_nr].ui32_graph_data_y_min_counter = 1;
    }
    // equal to min point
    else if(graphs[graph_nr].ui32_data[index] == min)
    {
      graphs[graph_nr].ui32_graph_data_y_min_counter++;
    }

    // increase index
    if(index >= 255)
    {
      index = 0;
    }
    else
    {
      index++;
    }
  }
}

static void graphs_measurements_search_max_y(uint32_t graph_nr)
{
  uint32_t i;
  uint32_t max = 0;
  uint32_t search_nr_points;
  uint32_t index;

  graphs[graph_nr].ui32_graph_data_y_max = max;

  // calc number of search points
  if(graphs[graph_nr].ui32_data_last_index > graphs[graph_nr].ui32_data_start_index)
  {
    search_nr_points = graphs[graph_nr].ui32_data_last_index - graphs[graph_nr].ui32_data_start_index;
  }
  else
  {
    search_nr_points = (255 - (graphs[graph_nr].ui32_data_start_index - graphs[graph_nr].ui32_data_last_index)) -  1;
  }

  index = graphs[graph_nr].ui32_data_start_index + 1;
  if(index >= 256)
  {
    index = 0;
  }
  for(i = 0; i < search_nr_points; ++i)
  {
    // new max point
    if(graphs[graph_nr].ui32_data[index] > max)
    {
      graphs[graph_nr].ui32_graph_data_y_max = graphs[0].ui32_data[index];
      graphs[graph_nr].ui32_graph_data_y_max_counter = 1;
    }
    // equal to max point
    else if(graphs[graph_nr].ui32_data[index] == max)
    {
      graphs[graph_nr].ui32_graph_data_y_max_counter++;
    }

    // increase index
    if(index >= 255)
    {
      index = 0;
    }
    else
    {
      index++;
    }
  }
}

void graphs_init(void)
{
  memcpy(graphs[0].ui32_data, ui32_array_data, (255 * 4) * 4);

  graphs[0].ui32_data_y_last_value = 0;
  graphs[0].ui32_data_last_index = 1;
  graphs[0].ui32_data_start_index = 0;
  graphs[0].ui32_graph_data_y_max = 0;
  graphs[0].ui32_graph_data_y_max_counter = 0;
  graphs[0].ui32_graph_data_y_min = 0xffffffff;
  graphs[0].ui32_graph_data_y_min_counter = 0;
}

graphs_t *get_graphs(void)
{
  return graphs;
}
