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

void graphs_measurements_calc_min_max_y(uint32_t graph_nr);
static void graphs_measurements_search_max_y(uint32_t graph_nr);
static void graphs_measurements_search_min_y(uint32_t graph_nr);
void graphs_clear_area(void);

void graphs_clock_1(void)
{
  uint32_t graph_nr = 0;

  // store the new value on the data array
  graphs[graph_nr].ui32_data[graphs[graph_nr].ui32_data_end_index] =
      graphs[graph_nr].ui32_data_y_last_value;
}

void graphs_clock_2(void)
{
  uint32_t graph_nr = 0;

  // find if we are now drawing over the first 256 points and see if we should increase both index
  if(graphs[graph_nr].ui32_data_array_over_255)
  {
    graphs[graph_nr].ui32_data_end_index = (graphs[graph_nr].ui32_data_end_index + 1) % 256;
    graphs[graph_nr].ui32_data_start_index = (graphs[graph_nr].ui32_data_start_index + 1) % 256;
    graphs[graph_nr].ui32_data_array_over_255 = 1;
  }
  else
  {
    graphs[graph_nr].ui32_data_end_index = (graphs[graph_nr].ui32_data_end_index + 1) % 256;

    // signal that data_array_over_255
    if(graphs[graph_nr].ui32_data_end_index >= 255)
    {
      graphs[graph_nr].ui32_data_array_over_255 = 1;
    }
  }
}

#if 0
void graphs_draw(uint32_t *p_draw_graphs, lcd_vars_t *p_lcd_vars)
{
  uint16_t i;
  uint32_t number_lines_to_draw;
  uint32_t y_amplitude;
  uint32_t graph_next_start_x;
  static uint32_t graph_x_index = 0;
  static uint32_t data_x_start_index = 0;
  uint32_t temp;

  static print_number_t graph_max_value =
  {
    .font = &SMALL_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 4,
    .ui8_left_zero_paddig = 0,
  };

  static print_number_t graph_min_value =
  {
    .font = &SMALL_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 4,
    .ui8_left_zero_paddig = 0,
  };

  static print_number_t graph_last_value =
  {
    .font = &REGULAR_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 4,
    .ui8_left_zero_paddig = 0,
  };

  // ui32_m_draw_graphs == 1 only every 3.5 seconds or higher
  if(*p_draw_graphs == 0)
  {
    return;
  }
  else
  {
    *p_draw_graphs = 0;
  }

  // draw tittle
  if(p_lcd_vars->ui32_main_screen_draw_static_info &&
      p_lcd_vars->lcd_screen_state == LCD_SCREEN_MAIN)
  {
    uint32_t ui32_x_position;
    uint32_t ui32_y_position;

    UG_SetBackcolor(C_DARK_BLUE);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&SMALL_TEXT_FONT);
    ui32_x_position = 25;
    ui32_y_position = 150;
//    UG_PutString(ui32_x_position,
//                 ui32_y_position,
//                 graphs[graph_nr].ui8_title);
    UG_PutString(ui32_x_position,
                 ui32_y_position,
                 "human power");
  }

  // store the new value on the data array
  graphs[graph_nr].ui32_data[graphs[graph_nr].ui32_data_end_index] =
      graphs[graph_nr].ui32_data_y_last_value;

  // calc new min and max values
  graphs_measurements_calc_min_max_y();

  // calc new pixel ratio
  temp = 0;
  if(graphs[graph_nr].ui32_graph_data_y_max > graphs[graph_nr].ui32_graph_data_y_min)
  {
    temp = graphs[graph_nr].ui32_graph_data_y_max - graphs[graph_nr].ui32_graph_data_y_min;
  }

  graphs[graph_nr].ui32_data_y_rate_per_pixel_x100 = 0;
  if(temp)
  {
    graphs[graph_nr].ui32_data_y_rate_per_pixel_x100 = (GRAPH_Y_LENGHT * 100) / temp;
  }

  // graphic is full, move data 1 line to left,
  // draw full lines because the full graph need to be refreshed
  if(m_graphs_data_array_over_255)
  {
    graphs_clear_area();

    number_lines_to_draw = 256;
    data_x_start_index = graphs[graph_nr].ui32_data_start_index;
    graph_x_index = 0;
  }
  // new min or max,
  // draw full lines because the full graph need to be refreshed
  else if(new_max_min)
  {
    graphs_clear_area();

    number_lines_to_draw = graphs[graph_nr].ui32_data_end_index + 1 -
        graphs[graph_nr].ui32_data_start_index;
    data_x_start_index = 0;
    graph_x_index = 0;
  }
  // draw only the new 1 line
  else
  {
    number_lines_to_draw = 1;
    graph_x_index = graphs[graph_nr].ui32_draw_x_last_index;
  }

  // draw the lines
  for(i = 0; i < number_lines_to_draw; i++)
  {
    y_amplitude = graphs[graph_nr].ui32_data[data_x_start_index] - graphs[graph_nr].ui32_graph_data_y_min;
    y_amplitude *= graphs[graph_nr].ui32_data_y_rate_per_pixel_x100;
    if(y_amplitude)
    {
      y_amplitude /= 100;
    }
    else
    {
      y_amplitude = 0;
    }

    graph_next_start_x = GRAPH_START_X + graph_x_index;

    // apply to screen only if we are drawing main screen
    if(p_lcd_vars->lcd_screen_state == LCD_SCREEN_MAIN)
    {
      uint32_t color;

      // if max, use a specific color
      if((graphs[graph_nr].ui32_data[data_x_start_index] > graphs[graph_nr].ui32_graph_data_y_min) &&
          (graphs[graph_nr].ui32_data[data_x_start_index] >= graphs[graph_nr].ui32_graph_data_y_max))
      {
        color = C_ORANGE_RED;
      }
      else
      {
        color = C_WHITE;
      }

      UG_DrawLine(graph_next_start_x,           // X1
                  GRAPH_START_Y,                // Y1
                  graph_next_start_x,           // X2
                  GRAPH_START_Y - y_amplitude,  // Y2
                  color);
    }

    data_x_start_index++;
    if(data_x_start_index >= 256)
    {
      data_x_start_index = 0;
    }

    graph_x_index++;
    if(graph_x_index >= 256)
    {
      graph_x_index = 0;
    }
  }

  // save last x index for next time
  graphs[graph_nr].ui32_draw_x_last_index = graph_x_index;

  // find if we are now drawing over the first 256 points
  if(m_graphs_data_array_over_255 == 0 &&
      graphs[graph_nr].ui32_data_end_index >= 255)
  {
    m_graphs_data_array_over_255 = 1;
  }

  // see if we should increase both index
  if(m_graphs_data_array_over_255)
  {
    graphs[graph_nr].ui32_data_end_index = (graphs[graph_nr].ui32_data_end_index + 1) % 256;
    graphs[graph_nr].ui32_data_start_index = (graphs[graph_nr].ui32_data_start_index + 1) % 256;
  }
  else
  {
    graphs[graph_nr].ui32_data_end_index = (graphs[graph_nr].ui32_data_end_index + 1) % 256;
  }

  // apply to screen only if we are drawing main screen
  if(p_lcd_vars->lcd_screen_state == LCD_SCREEN_MAIN)
  {
    // draw max and min values as also last value
    graph_max_value.ui32_x_position = 0;
    graph_max_value.ui32_y_position = GRAPH_START_Y - GRAPH_Y_LENGHT - 18;
    graph_max_value.ui32_number = graphs[graph_nr].ui32_graph_data_y_max;
    graph_max_value.ui8_refresh_all_digits = 1;
    lcd_print_number(&graph_max_value);

    graph_min_value.ui32_x_position = 0;
    graph_min_value.ui32_y_position = GRAPH_START_Y - 18;
    graph_min_value.ui32_number = graphs[graph_nr].ui32_graph_data_y_min;
    graph_min_value.ui8_refresh_all_digits = 1;
    lcd_print_number(&graph_min_value);

    graph_last_value.ui32_x_position = 225;
    graph_last_value.ui32_y_position = GRAPH_START_Y - GRAPH_Y_LENGHT - 30;
    graph_last_value.ui32_number = graphs[graph_nr].ui32_data_y_last_value;
    graph_last_value.ui8_refresh_all_digits = 1;
    lcd_print_number(&graph_last_value);
  }

  new_max_min = 0;
}
#endif

void graphs_draw(lcd_vars_t *p_lcd_vars)
{
  uint32_t number_lines_to_draw;
  uint32_t y_amplitude;
  uint32_t graph_next_start_x;
  uint32_t graph_x_index = 0;
  static uint32_t data_x_start_index = 0;
  uint32_t color;
  uint32_t graph_nr = 0;
  uint32_t temp;
  uint32_t i;

  static print_number_t graph_max_value =
  {
    .font = &SMALL_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 4,
    .ui8_left_zero_paddig = 0,
  };

  static print_number_t graph_min_value =
  {
    .font = &SMALL_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 4,
    .ui8_left_zero_paddig = 0,
  };

  static print_number_t graph_last_value =
  {
    .font = &REGULAR_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 4,
    .ui8_left_zero_paddig = 0,
  };

  // draw tittle
  if(p_lcd_vars->ui32_main_screen_draw_static_info)
  {
    uint32_t ui32_x_position;
    uint32_t ui32_y_position;

    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&SMALL_TEXT_FONT);
    ui32_x_position = 100;
    ui32_y_position = 326;
    UG_PutString(ui32_x_position,
                 ui32_y_position,
                 "human power");
  }

  // calc new min and max values
  graphs_measurements_calc_min_max_y(graph_nr);

  // calc new pixel ratio
  temp = 0;
  if(graphs[graph_nr].ui32_graph_data_y_max > graphs[graph_nr].ui32_graph_data_y_min)
  {
    temp = graphs[graph_nr].ui32_graph_data_y_max - graphs[graph_nr].ui32_graph_data_y_min;
  }

  graphs[graph_nr].ui32_data_y_rate_per_pixel_x100 = 0;
  if(temp)
  {
    graphs[graph_nr].ui32_data_y_rate_per_pixel_x100 = (GRAPH_Y_LENGHT * 100) / temp;
  }

  // graphic is full, move data 1 line to left,
  // draw full lines because the full graph need to be refreshed
  if(graphs[graph_nr].ui32_data_array_over_255)
  {
    graphs_clear_area();

    number_lines_to_draw = 256;
    data_x_start_index = graphs[graph_nr].ui32_data_start_index;
  }
  // draw only needed lines
  else
  {
    graphs_clear_area();

    number_lines_to_draw = graphs[graph_nr].ui32_data_end_index + 1 -
        graphs[graph_nr].ui32_data_start_index;
    data_x_start_index = 0;
  }

  // draw the lines
  for(i = 0; i < number_lines_to_draw; i++)
  {
    y_amplitude = graphs[graph_nr].ui32_data[data_x_start_index] - graphs[graph_nr].ui32_graph_data_y_min;
    y_amplitude *= graphs[graph_nr].ui32_data_y_rate_per_pixel_x100;
    if(y_amplitude)
    {
      y_amplitude /= 100;
    }
    else
    {
      y_amplitude = 0;
    }

    graph_next_start_x = GRAPH_START_X + graph_x_index;

    // if max, use a specific color
    if((graphs[graph_nr].ui32_data[data_x_start_index] > graphs[graph_nr].ui32_graph_data_y_min) &&
        (graphs[graph_nr].ui32_data[data_x_start_index] >= graphs[graph_nr].ui32_graph_data_y_max))
    {
      color = C_GREEN;
    }
    else
    {
      color = C_WHITE;
    }

    UG_DrawLine(graph_next_start_x,           // X1
                GRAPH_START_Y,                // Y1
                graph_next_start_x,           // X2
                GRAPH_START_Y - y_amplitude,  // Y2
                color);

    // increment and verify roll over
    data_x_start_index++;
    if(data_x_start_index >= 256)
    {
      data_x_start_index = 0;
    }

    // no chance to roll over so just increment
    graph_x_index++;
  }

  // draw max and min values as also last value
  graph_max_value.ui32_x_position = 2;
  graph_max_value.ui32_y_position = GRAPH_START_Y - GRAPH_Y_LENGHT - 18;
  graph_max_value.ui32_number = graphs[graph_nr].ui32_graph_data_y_max;
  graph_max_value.ui8_refresh_all_digits = 1;
  lcd_print_number(&graph_max_value);

  graph_min_value.ui32_x_position = 2;
  graph_min_value.ui32_y_position = GRAPH_START_Y - 18;
  graph_min_value.ui32_number = graphs[graph_nr].ui32_graph_data_y_min;
  graph_min_value.ui8_refresh_all_digits = 1;
  lcd_print_number(&graph_min_value);

//  graph_last_value.ui32_x_position = 225;
//  graph_last_value.ui32_y_position = GRAPH_START_Y - GRAPH_Y_LENGHT - 30;
//  graph_last_value.ui32_number = graphs[graph_nr].ui32_data_y_last_value;
//  graph_last_value.ui8_refresh_all_digits = 1;
//  lcd_print_number(&graph_last_value);
}

void graphs_measurements_calc_min_max_y(uint32_t graph_nr)
{
  uint32_t start_index;

  // we are only yet adding a new point to graph
  if(graphs[graph_nr].ui32_data_array_over_255 == 0)
  {
    // equal to min
    if(graphs[graph_nr].ui32_data_y_last_value == graphs[graph_nr].ui32_graph_data_y_min)
    {
      graphs[graph_nr].ui32_graph_data_y_min_counter++;
    }
    // less than min
    else if(graphs[graph_nr].ui32_data_y_last_value < graphs[graph_nr].ui32_graph_data_y_min)
    {
      graphs[graph_nr].ui32_graph_data_y_min = graphs[graph_nr].ui32_data_y_last_value;
      graphs[graph_nr].ui32_graph_data_y_min_counter = 1;
    }

    // equal to max
    if(graphs[graph_nr].ui32_data_y_last_value == graphs[graph_nr].ui32_graph_data_y_max)
    {
      graphs[graph_nr].ui32_graph_data_y_max_counter++;
    }
    // higher than max
    else if(graphs[graph_nr].ui32_data_y_last_value > graphs[graph_nr].ui32_graph_data_y_max)
    {
      graphs[graph_nr].ui32_graph_data_y_max = graphs[graph_nr].ui32_data_y_last_value;
      graphs[graph_nr].ui32_graph_data_y_max_counter = 1;
    }
  }

  // we will be adding and removing points to graph
  else
  {
    graphs_measurements_search_min_y(graph_nr);
    graphs_measurements_search_max_y(graph_nr);
  }
}

static void graphs_measurements_search_min_y(uint32_t graph_nr)
{
  uint32_t i;
  uint32_t search_nr_points;
  uint32_t index;

  graphs[graph_nr].ui32_graph_data_y_min = 0xffff;

  // calc number of search points
  if(graphs[graph_nr].ui32_data_end_index > graphs[graph_nr].ui32_data_start_index)
  {
    search_nr_points = (graphs[graph_nr].ui32_data_end_index - graphs[graph_nr].ui32_data_start_index) + 1;
  }
  else
  {
    search_nr_points = (256 - (graphs[graph_nr].ui32_data_start_index - graphs[graph_nr].ui32_data_end_index)) + 1;
  }

  index = graphs[graph_nr].ui32_data_start_index;

  for(i = 0; i < search_nr_points; ++i)
  {
    // new min point
    if(graphs[graph_nr].ui32_data[index] < graphs[graph_nr].ui32_graph_data_y_min)
    {
      graphs[graph_nr].ui32_graph_data_y_min = graphs[graph_nr].ui32_data[index];
      graphs[graph_nr].ui32_graph_data_y_min_counter = 1;
    }
    // equal to min point
    else if(graphs[graph_nr].ui32_data[index] == graphs[graph_nr].ui32_graph_data_y_min)
    {
      graphs[graph_nr].ui32_graph_data_y_min_counter++;
    }

    index++;
    if(index >= 256)
    {
      index = 0;
    }
  }
}

static void graphs_measurements_search_max_y(uint32_t graph_nr)
{
  uint32_t i;
  uint32_t search_nr_points;
  uint32_t index;

  graphs[graph_nr].ui32_graph_data_y_max = 0;

  // calc number of search points
  if(graphs[graph_nr].ui32_data_end_index > graphs[graph_nr].ui32_data_start_index)
  {
    search_nr_points = (graphs[graph_nr].ui32_data_end_index - graphs[graph_nr].ui32_data_start_index) + 1;
  }
  else
  {
    search_nr_points = (256 - (graphs[graph_nr].ui32_data_start_index - graphs[graph_nr].ui32_data_end_index)) + 1;
  }

  index = graphs[graph_nr].ui32_data_start_index;

  for(i = 0; i < search_nr_points; ++i)
  {
    // new max point
    if(graphs[graph_nr].ui32_data[index] > graphs[graph_nr].ui32_graph_data_y_max)
    {
      graphs[graph_nr].ui32_graph_data_y_max = graphs[graph_nr].ui32_data[index];
      graphs[graph_nr].ui32_graph_data_y_max_counter = 1;
    }
    // equal to max point
    else if(graphs[graph_nr].ui32_data[index] == graphs[graph_nr].ui32_graph_data_y_max)
    {
      graphs[graph_nr].ui32_graph_data_y_max_counter++;
    }

    index++;
    if(index >= 256)
    {
      index = 0;
    }
  }
}

void graphs_init(void)
{
  uint32_t graph_nr = 0;

  snprintf(graphs[graph_nr].ui8_title, 12, "human power");
  graphs[graph_nr].ui32_data_y_last_value = 0;
  graphs[graph_nr].ui32_data_end_index = 0;
  graphs[graph_nr].ui32_data_start_index = 0;
  graphs[graph_nr].ui32_graph_data_y_max = 0;
  graphs[graph_nr].ui32_graph_data_y_max_counter = 0;
  graphs[graph_nr].ui32_graph_data_y_min = 0;
  graphs[graph_nr].ui32_graph_data_y_min_counter = 1;
  graphs[graph_nr].ui32_data_array_over_255 = 0;
}

graphs_t *get_graphs(void)
{
  return graphs;
}

void graphs_clear_area(void)
{
  // clean all lines on the LCD
  UG_FillFrame(GRAPH_START_X,
               GRAPH_START_Y - 100,
               315,
               GRAPH_START_Y,
               C_BLACK);
}
