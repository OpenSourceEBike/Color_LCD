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
#include "ugui_driver/ugui_bafang_850c.h"
#include "ugui/ugui.h"
#include "lcd.h"

// 255 pixels for data points
// 255 points of each graph
// 60 minutes: 255 * (3500 * 4) ms
// 30 minutes: 255 * (3500 * 2) ms
// 15 minutes: 255 * 3500 ms

volatile graphs_t graphs[NUMBER_OF_GRAPHS_ID];

l3_vars_t *p_m_l3_vars;

void graphs_measurements_calc_min_max_y(graphs_id_t graph_id);
static void graphs_measurements_search_max_y(graphs_id_t graph_id);
static void graphs_measurements_search_min_y(graphs_id_t graph_id);
void graphs_clear_area(void);

void graphs_clock_1(void)
{
  graphs_id_t graph_id = 0;

  for(graph_id = 0; graph_id < NUMBER_OF_GRAPHS_ID; graph_id++)
  {
    // store the new value on the data array
    graphs[graph_id].ui32_data[graphs[graph_id].ui32_data_end_index] =
        graphs[graph_id].ui32_data_y_last_value;
  }
}

void graphs_clock_2(void)
{
  graphs_id_t graph_id = 0;

  for(graph_id = 0; graph_id < NUMBER_OF_GRAPHS_ID; graph_id++)
  {
    // find if we are now drawing over the first 256 points and see if we should increase both index
    if(graphs[graph_id].ui32_data_array_over_255)
    {
      graphs[graph_id].ui32_data_end_index = (graphs[graph_id].ui32_data_end_index + 1) % 256;
      graphs[graph_id].ui32_data_start_index = (graphs[graph_id].ui32_data_start_index + 1) % 256;
      graphs[graph_id].ui32_data_array_over_255 = 1;
    }
    else
    {
      graphs[graph_id].ui32_data_end_index = (graphs[graph_id].ui32_data_end_index + 1) % 256;

      // signal that data_array_over_255
      if(graphs[graph_id].ui32_data_end_index >= 255)
      {
        graphs[graph_id].ui32_data_array_over_255 = 1;
      }
    }
  }
}

void graphs_draw(lcd_vars_t *p_lcd_vars)
{
  uint32_t number_lines_to_draw;
  uint32_t temp;
  uint32_t y_amplitude;
  static uint32_t y_amplitude_previous = 0;
  uint32_t y_amplitude_base_color;
  uint32_t y_amplitude_contour_color = 0;
  uint32_t graph_next_start_x;
  uint32_t graph_x_index = 0;
  static uint32_t data_x_start_index = 0;
  uint32_t graph_id = 0;
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
    UG_SetForecolor(MAIN_SCREEN_FIELD_LABELS_COLOR);
    UG_FontSelect(&SMALL_TEXT_FONT);
    ui32_x_position = 100;
    ui32_y_position = 326;

    switch(p_m_l3_vars->graph_id)
    {
      case GRAPH_WHEEL_SPEED:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "wheel speed");
      break;

      case GRAPH_PEDAL_HUMAN_POWER:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "human power");
      break;

      case GRAPH_PEDAL_CADENCE:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "cadence");
      break;

      case GRAPH_PEDAL_TORQUE:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "pedal torque");
      break;

      case GRAPH_BATTERY_VOLTAGE:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "battery voltage");
      break;

      case GRAPH_BATTERY_CURRENT:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "battery current");
      break;

      case GRAPH_BATTERY_SOC:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "battery SOC");
      break;

      case GRAPH_MOTOR_POWER:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "motor power");
      break;

      case GRAPH_MOTOR_TEMPERATURE:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "motor temperature");
      break;

      case GRAPH_MOTOR_PWM_DUTY_CYCLE:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "motor duty-cycle");
      break;

      case GRAPH_MOTOR_ERPS:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "motor ers");
      break;

      case GRAPH_MOTOR_FOC_ANGLE:
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "motor foc angle");
      break;

      default:
      break;
    }

    // vertical line
    UG_DrawLine(GRAPH_START_X - 1, GRAPH_START_Y, GRAPH_START_X - 1, GRAPH_START_Y - GRAPH_Y_LENGHT, C_WHITE);
  }

  // calc new min and max values
  graphs_measurements_calc_min_max_y(graph_id);

  // calc new pixel ratio
  temp = 0;
  if(graphs[graph_id].ui32_graph_data_y_max > graphs[graph_id].ui32_graph_data_y_min)
  {
    temp = graphs[graph_id].ui32_graph_data_y_max - graphs[graph_id].ui32_graph_data_y_min;
  }

  graphs[graph_id].ui32_data_y_rate_per_pixel_x100 = 0;
  if(temp)
  {
    graphs[graph_id].ui32_data_y_rate_per_pixel_x100 = (GRAPH_Y_LENGHT * 100) / temp;
  }

  // graphic is full, move data 1 line to left,
  // draw full lines because the full graph need to be refreshed
  if(graphs[graph_id].ui32_data_array_over_255)
  {
    graphs_clear_area();

    number_lines_to_draw = 256;
    data_x_start_index = graphs[graph_id].ui32_data_start_index;
  }
  // draw only needed lines
  else
  {
    graphs_clear_area();

    number_lines_to_draw = graphs[graph_id].ui32_data_end_index + 1 -
        graphs[graph_id].ui32_data_start_index;
    data_x_start_index = 0;
  }

  // draw the lines
  for(i = 0; i < number_lines_to_draw; i++)
  {
    y_amplitude = graphs[graph_id].ui32_data[data_x_start_index] - graphs[graph_id].ui32_graph_data_y_min;
    y_amplitude *= graphs[graph_id].ui32_data_y_rate_per_pixel_x100;
    if(y_amplitude)
    {
      y_amplitude /= 100;
    }
    else
    {
      y_amplitude = 0;
    }

    // contour
    if(y_amplitude >= y_amplitude_previous)
    {
      y_amplitude_base_color =  y_amplitude_previous;
      y_amplitude_contour_color = y_amplitude - y_amplitude_base_color;
    }
    else
    {
      y_amplitude_base_color =  y_amplitude;
      y_amplitude_contour_color = y_amplitude_previous - y_amplitude;
    }

    // add the 2 units/pixels to the contour
    if(y_amplitude_base_color > 0)
    {
//      y_amplitude_base_color = y_amplitude_base_color - 1;
//      y_amplitude_contour_color = y_amplitude_contour_color + 1;
    }

    graph_next_start_x = GRAPH_START_X + graph_x_index;

    // draw lines: amplitude > 2
    if(y_amplitude_base_color)
    {
      UG_DrawLine(graph_next_start_x,           // X1
                  GRAPH_START_Y,                // Y1
                  graph_next_start_x,           // X2
                  GRAPH_START_Y - y_amplitude_base_color,// Y2
                  C_BLUE);

      temp = GRAPH_START_Y - y_amplitude_base_color;
      UG_DrawLine(graph_next_start_x,           // X1
                  temp,// Y1
                  graph_next_start_x,           // X2
                  temp - y_amplitude_contour_color,// Y2
                  C_WHITE);
    }
    // draw lines: amplitude < 2
    else
    {
      UG_DrawLine(graph_next_start_x,           // X1
                  GRAPH_START_Y,                // Y1
                  graph_next_start_x,           // X2
                  GRAPH_START_Y - y_amplitude_contour_color,// Y2
                  C_WHITE);
    }

    // increment and verify roll over
    data_x_start_index++;
    if(data_x_start_index >= 256)
    {
      data_x_start_index = 0;
    }

    // no chance to roll over so just increment
    graph_x_index++;

    y_amplitude_previous = y_amplitude;
  }

  // draw max and min values as also last value
  graph_max_value.ui32_x_position = 7;
  graph_max_value.ui32_y_position = GRAPH_START_Y - GRAPH_Y_LENGHT - 1;
  graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
  graph_max_value.ui8_refresh_all_digits = 1;
  lcd_print_number(&graph_max_value);

  graph_min_value.ui32_x_position = 7;
  graph_min_value.ui32_y_position = GRAPH_START_Y - 14;
  graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
  graph_min_value.ui8_refresh_all_digits = 1;
  lcd_print_number(&graph_min_value);

//  graph_last_value.ui32_x_position = 225;
//  graph_last_value.ui32_y_position = GRAPH_START_Y - GRAPH_Y_LENGHT - 30;
//  graph_last_value.ui32_number = graphs[graph_id].ui32_data_y_last_value;
//  graph_last_value.ui8_refresh_all_digits = 1;
//  lcd_print_number(&graph_last_value);
}

void graphs_measurements_calc_min_max_y(graphs_id_t graph_id)
{
  uint32_t start_index;

  // we are only yet adding a new point to graph
  if(graphs[graph_id].ui32_data_array_over_255 == 0)
  {
    // equal to min
    if(graphs[graph_id].ui32_data_y_last_value == graphs[graph_id].ui32_graph_data_y_min)
    {
      graphs[graph_id].ui32_graph_data_y_min_counter++;
    }
    // less than min
    else if(graphs[graph_id].ui32_data_y_last_value < graphs[graph_id].ui32_graph_data_y_min)
    {
      graphs[graph_id].ui32_graph_data_y_min = graphs[graph_id].ui32_data_y_last_value;
      graphs[graph_id].ui32_graph_data_y_min_counter = 1;
    }

    // equal to max
    if(graphs[graph_id].ui32_data_y_last_value == graphs[graph_id].ui32_graph_data_y_max)
    {
      graphs[graph_id].ui32_graph_data_y_max_counter++;
    }
    // higher than max
    else if(graphs[graph_id].ui32_data_y_last_value > graphs[graph_id].ui32_graph_data_y_max)
    {
      graphs[graph_id].ui32_graph_data_y_max = graphs[graph_id].ui32_data_y_last_value;
      graphs[graph_id].ui32_graph_data_y_max_counter = 1;
    }
  }

  // we will be adding and removing points to graph
  else
  {
    graphs_measurements_search_min_y(graph_id);
    graphs_measurements_search_max_y(graph_id);
  }
}

static void graphs_measurements_search_min_y(graphs_id_t graph_id)
{
  uint32_t i;
  uint32_t search_nr_points;
  uint32_t index;

  graphs[graph_id].ui32_graph_data_y_min = 0xffff;

  // calc number of search points
  if(graphs[graph_id].ui32_data_end_index > graphs[graph_id].ui32_data_start_index)
  {
    search_nr_points = (graphs[graph_id].ui32_data_end_index - graphs[graph_id].ui32_data_start_index) + 1;
  }
  else
  {
    search_nr_points = (256 - (graphs[graph_id].ui32_data_start_index - graphs[graph_id].ui32_data_end_index)) + 1;
  }

  index = graphs[graph_id].ui32_data_start_index;

  for(i = 0; i < search_nr_points; ++i)
  {
    // new min point
    if(graphs[graph_id].ui32_data[index] < graphs[graph_id].ui32_graph_data_y_min)
    {
      graphs[graph_id].ui32_graph_data_y_min = graphs[graph_id].ui32_data[index];
      graphs[graph_id].ui32_graph_data_y_min_counter = 1;
    }
    // equal to min point
    else if(graphs[graph_id].ui32_data[index] == graphs[graph_id].ui32_graph_data_y_min)
    {
      graphs[graph_id].ui32_graph_data_y_min_counter++;
    }

    index++;
    if(index >= 256)
    {
      index = 0;
    }
  }
}

static void graphs_measurements_search_max_y(graphs_id_t graph_id)
{
  uint32_t i;
  uint32_t search_nr_points;
  uint32_t index;

  graphs[graph_id].ui32_graph_data_y_max = 0;

  // calc number of search points
  if(graphs[graph_id].ui32_data_end_index > graphs[graph_id].ui32_data_start_index)
  {
    search_nr_points = (graphs[graph_id].ui32_data_end_index - graphs[graph_id].ui32_data_start_index) + 1;
  }
  else
  {
    search_nr_points = (256 - (graphs[graph_id].ui32_data_start_index - graphs[graph_id].ui32_data_end_index)) + 1;
  }

  index = graphs[graph_id].ui32_data_start_index;

  for(i = 0; i < search_nr_points; ++i)
  {
    // new max point
    if(graphs[graph_id].ui32_data[index] > graphs[graph_id].ui32_graph_data_y_max)
    {
      graphs[graph_id].ui32_graph_data_y_max = graphs[graph_id].ui32_data[index];
      graphs[graph_id].ui32_graph_data_y_max_counter = 1;
    }
    // equal to max point
    else if(graphs[graph_id].ui32_data[index] == graphs[graph_id].ui32_graph_data_y_max)
    {
      graphs[graph_id].ui32_graph_data_y_max_counter++;
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
  graphs_id_t graph_id = 0;

  p_m_l3_vars = get_l3_vars();

  for(graph_id = 0; graph_id < NUMBER_OF_GRAPHS_ID; graph_id++)
  {
  //  snprintf(graphs[graph_id].ui8_title, 12, "human power");
    graphs[graph_id].ui32_data_y_last_value = 0;
    graphs[graph_id].ui32_data_end_index = 0;
    graphs[graph_id].ui32_data_start_index = 0;
    graphs[graph_id].ui32_graph_data_y_max = 0;
    graphs[graph_id].ui32_graph_data_y_max_counter = 0;
    graphs[graph_id].ui32_graph_data_y_min = 0;
    graphs[graph_id].ui32_graph_data_y_min_counter = 1;
    graphs[graph_id].ui32_data_array_over_255 = 0;
  }
}

volatile graphs_t *get_graphs(void)
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
