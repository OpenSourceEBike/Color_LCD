/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018, 2019.
 *
 * Released under the GPL License, Version 3
 */

//#include <math.h>
#include <string.h>
#include "stm32f10x.h"
#include "stdio.h"
#include "main.h"
#include "config.h"
#include "ugui_driver/ugui_bafang_850c.h"
#include "ugui.h"
#include "lcd.h"
#include "graphs.h"

// 255 pixels for data points
// 255 points of each graph
// 60 minutes: 255 * (3500 * 4) ms
// 30 minutes: 255 * (3500 * 2) ms
// 15 minutes: 255 * 3500 ms

volatile graphs_t graphs[NUMBER_OF_GRAPHS_ID];

volatile l3_vars_t *p_m_l3_vars;

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

void graphs_draw(volatile lcd_vars_t *p_lcd_vars)
{
  uint32_t ui32_number_lines_to_draw;
  uint32_t ui32_temp;
  uint32_t ui32_temp_1;
  uint32_t ui32_temp_2;
  uint32_t ui32_y_amplitude;
  uint32_t ui32_y_amplitude_previous = 0;
  uint32_t ui32_y_amplitude_base_color;
  uint32_t ui32_y_amplitude_contour_color = 0;
  uint32_t ui32_graph_next_start_x;
  uint32_t ui32_graph_x_index = 0;
  uint32_t ui32_data_x_start_index = 0;
  graphs_id_t graph_id = p_m_l3_vars->graph_id;
  uint32_t ui32_i;
  uint32_t ui32_force_amplitude_max = 0;

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

  // calc new min and max values
  graphs_measurements_calc_min_max_y(graph_id);

  // calc new pixel ratio
  ui32_temp = 0;
  if(graphs[graph_id].ui32_graph_data_y_max > graphs[graph_id].ui32_graph_data_y_min)
  {
    ui32_temp = graphs[graph_id].ui32_graph_data_y_max - graphs[graph_id].ui32_graph_data_y_min;
  }

  graphs[graph_id].ui32_data_y_rate_per_pixel_x100 = 0;
  if(ui32_temp)
  {
    graphs[graph_id].ui32_data_y_rate_per_pixel_x100 = (GRAPH_Y_LENGHT * 100) / ui32_temp;
  }

  // force full amplitude when
  if(graphs[graph_id].ui32_graph_data_y_max == graphs[graph_id].ui32_graph_data_y_min &&
      graphs[graph_id].ui32_graph_data_y_min > 0)
  {
    ui32_force_amplitude_max = 1;
  }

  // graphic is full, move data 1 line to left,
  // draw full lines because the full graph need to be refreshed
  if(graphs[graph_id].ui32_data_array_over_255)
  {
    graphs_clear_area();

    ui32_number_lines_to_draw = 256;
    ui32_data_x_start_index = graphs[graph_id].ui32_data_start_index;
  }
  // draw only needed lines
  else
  {
    graphs_clear_area();

    ui32_number_lines_to_draw = graphs[graph_id].ui32_data_end_index + 1 -
        graphs[graph_id].ui32_data_start_index;
    ui32_data_x_start_index = 0;
  }

  // draw the lines
  for(ui32_i = 0; ui32_i < ui32_number_lines_to_draw; ui32_i++)
  {
    ui32_y_amplitude = graphs[graph_id].ui32_data[ui32_data_x_start_index] - graphs[graph_id].ui32_graph_data_y_min;
    ui32_y_amplitude *= graphs[graph_id].ui32_data_y_rate_per_pixel_x100;
    if(ui32_y_amplitude)
    {
      ui32_y_amplitude /= 100;
    }
    else
    {
      ui32_y_amplitude = 0;
    }

    if(ui32_force_amplitude_max)
    {
      ui32_y_amplitude = GRAPH_Y_LENGHT;
    }

    // force first line to not be full white
    if(ui32_i == 0)
    {
      ui32_y_amplitude_previous = ui32_y_amplitude;
    }

    // contour
    if(ui32_y_amplitude >= ui32_y_amplitude_previous)
    {
      ui32_y_amplitude_base_color =  ui32_y_amplitude_previous;
      ui32_y_amplitude_contour_color = ui32_y_amplitude - ui32_y_amplitude_base_color;
    }
    else
    {
      ui32_y_amplitude_base_color =  ui32_y_amplitude;
      ui32_y_amplitude_contour_color = ui32_y_amplitude_previous - ui32_y_amplitude;
    }

    // add the 2 units/pixels to the contour
    if(ui32_y_amplitude_base_color > 0)
    {
//      y_amplitude_base_color = y_amplitude_base_color - 1;
//      y_amplitude_contour_color = y_amplitude_contour_color + 1;
    }

    ui32_graph_next_start_x = GRAPH_START_X + ui32_graph_x_index;

    // draw lines: amplitude > 2
    if(ui32_y_amplitude_base_color)
    {
      UG_DrawLine(ui32_graph_next_start_x,           // X1
                  GRAPH_START_Y,                // Y1
                  ui32_graph_next_start_x,           // X2
                  GRAPH_START_Y - ui32_y_amplitude_base_color,// Y2
                  C_BLUE);

      ui32_temp = GRAPH_START_Y - ui32_y_amplitude_base_color;
      UG_DrawLine(ui32_graph_next_start_x,           // X1
                  ui32_temp,// Y1
                  ui32_graph_next_start_x,           // X2
                  ui32_temp - ui32_y_amplitude_contour_color,// Y2
                  C_WHITE);
    }
    // draw lines: amplitude < 2
    else
    {
      UG_DrawLine(ui32_graph_next_start_x,           // X1
                  GRAPH_START_Y,                // Y1
                  ui32_graph_next_start_x,           // X2
                  GRAPH_START_Y - ui32_y_amplitude_contour_color,// Y2
                  C_WHITE);
    }

    // increment and verify roll over
    ui32_data_x_start_index++;
    if(ui32_data_x_start_index >= 256)
    {
      ui32_data_x_start_index = 0;
    }

    // no chance to roll over so just increment
    ui32_graph_x_index++;

    ui32_y_amplitude_previous = ui32_y_amplitude;
  }

  // draw max and min values
  graph_max_value.ui32_y_position = GRAPH_START_Y - GRAPH_Y_LENGHT - 1;
  graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
  graph_max_value.ui8_refresh_all_digits = 1;
  graph_min_value.ui32_y_position = GRAPH_START_Y - 14;
  graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
  graph_min_value.ui8_refresh_all_digits = 1;

  switch(graph_id)
  {
    case GRAPH_WHEEL_SPEED:
      if(p_m_l3_vars->ui8_units_type == 0)
      {
        ui32_temp_1 = graphs[graph_id].ui32_graph_data_y_max;
        ui32_temp_2 = graphs[graph_id].ui32_graph_data_y_min;
      }
      else
      {
        ui32_temp_1 = (graphs[graph_id].ui32_graph_data_y_max * 10) / 16;
        ui32_temp_2 = (graphs[graph_id].ui32_graph_data_y_min * 10) / 16;
      }

      graph_max_value.ui32_x_position = 1;
      graph_max_value.ui32_number = ui32_temp_1;
      graph_max_value.ui8_decimal_digits = 1;
      graph_min_value.ui32_x_position = 1;
      graph_min_value.ui32_number = ui32_temp_2;
      graph_min_value.ui8_decimal_digits = 1;
    break;

    case GRAPH_PEDAL_HUMAN_POWER:
      graph_max_value.ui32_x_position = 7;
      graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
      graph_max_value.ui8_decimal_digits = 0;
      graph_min_value.ui32_x_position = 7;
      graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
      graph_min_value.ui8_decimal_digits = 0;
    break;

    case GRAPH_PEDAL_CADENCE:
      graph_max_value.ui32_x_position = 7;
      graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
      graph_max_value.ui8_decimal_digits = 0;
      graph_min_value.ui32_x_position = 7;
      graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
      graph_min_value.ui8_decimal_digits = 0;
    break;

    case GRAPH_BATTERY_VOLTAGE:
      graph_max_value.ui32_x_position = 1;
      graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
      graph_max_value.ui8_decimal_digits = 1;
      graph_min_value.ui32_x_position = 1;
      graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
      graph_min_value.ui8_decimal_digits = 1;
    break;

    case GRAPH_BATTERY_CURRENT:
      graph_max_value.ui32_x_position = 1;
      graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
      graph_max_value.ui8_decimal_digits = 1;
      graph_min_value.ui32_x_position = 1;
      graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
      graph_min_value.ui8_decimal_digits = 1;
    break;

    case GRAPH_BATTERY_SOC:
      graph_max_value.ui32_x_position = 7;
      graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
      graph_max_value.ui8_decimal_digits = 0;
      graph_min_value.ui32_x_position = 7;
      graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
      graph_min_value.ui8_decimal_digits = 0;
    break;

    case GRAPH_MOTOR_POWER:
      graph_max_value.ui32_x_position = 7;
      graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
      graph_max_value.ui8_decimal_digits = 0;
      lcd_print_number(&graph_max_value);
      graph_min_value.ui32_x_position = 7;
      graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
      graph_min_value.ui8_decimal_digits = 0;
      lcd_print_number(&graph_min_value);
    break;

    case GRAPH_MOTOR_TEMPERATURE:
      if(p_m_l3_vars->ui8_units_type == 0)
      {
        ui32_temp_1 = graphs[graph_id].ui32_graph_data_y_max;
        ui32_temp_2 = graphs[graph_id].ui32_graph_data_y_min;
      }
      else
      {
        ui32_temp_1 = (((uint32_t) graphs[graph_id].ui32_graph_data_y_max * 18) + 320) / 10;
        ui32_temp_2 = (((uint32_t) graphs[graph_id].ui32_graph_data_y_min * 18) + 320) / 10;
      }

      graph_max_value.ui32_x_position = 7;
      graph_max_value.ui32_number = ui32_temp_1;
      graph_max_value.ui8_decimal_digits = 0;
      graph_min_value.ui32_x_position = 7;
      graph_min_value.ui32_number = ui32_temp_2;
      graph_min_value.ui8_decimal_digits = 0;
    break;

    case GRAPH_MOTOR_PWM_DUTY_CYCLE:
      graph_max_value.ui32_x_position = 7;
      graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
      graph_max_value.ui8_decimal_digits = 0;
      graph_min_value.ui32_x_position = 7;
      graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
      graph_min_value.ui8_decimal_digits = 0;
    break;

    case GRAPH_MOTOR_ERPS:
      graph_max_value.ui32_x_position = 7;
      graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max;
      graph_max_value.ui8_decimal_digits = 0;
      graph_min_value.ui32_x_position = 7;
      graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min;
      graph_min_value.ui8_decimal_digits = 0;
    break;

    case GRAPH_MOTOR_FOC_ANGLE:
      graph_max_value.ui32_x_position = 1;
      graph_max_value.ui32_number = graphs[graph_id].ui32_graph_data_y_max / 10;
      graph_max_value.ui8_decimal_digits = 1;
      graph_min_value.ui32_x_position = 1;
      graph_min_value.ui32_number = graphs[graph_id].ui32_graph_data_y_min / 10;
      graph_min_value.ui8_decimal_digits = 1;

    break;

    default:
    break;
  }

  // first erase the area
  UG_FillFrame(0,
              GRAPH_START_Y - GRAPH_Y_LENGHT - 1,
              GRAPH_START_X - 2,
              GRAPH_START_Y - GRAPH_Y_LENGHT - 1 + graph_max_value.font->char_height,
              C_BLACK);

  UG_FillFrame(0,
              GRAPH_START_Y - 14,
              GRAPH_START_X - 2,
              GRAPH_START_Y - 14 + graph_max_value.font->char_height,
              C_BLACK);

  // print the max only on specific conditions
  if(graphs[graph_id].ui32_graph_data_y_max != 0)
  {
    lcd_print_number(&graph_max_value);
  }

  // print the min only on specific conditions
  if(graphs[graph_id].ui32_graph_data_y_max != graphs[graph_id].ui32_graph_data_y_min ||
      graphs[graph_id].ui32_graph_data_y_min == 0)
  {
    lcd_print_number(&graph_min_value);
  }

//  graph_last_value.ui32_x_position = 225;
//  graph_last_value.ui32_y_position = GRAPH_START_Y - GRAPH_Y_LENGHT - 30;
//  graph_last_value.ui32_number = graphs[graph_id].ui32_data_y_last_value;
//  graph_last_value.ui8_refresh_all_digits = 1;
//  lcd_print_number(&graph_last_value);
}

void graphs_measurements_calc_min_max_y(graphs_id_t graph_id)
{
//  // we are only yet adding a new point to graph
//  if(graphs[graph_id].ui32_data_array_over_255 == 0)
//  {
//    // new min point
//    if(graphs[graph_id].ui32_data[graphs[graph_id].ui32_data_end_index] < graphs[graph_id].ui32_graph_data_y_min)
//    {
//      graphs[graph_id].ui32_graph_data_y_min = graphs[graph_id].ui32_data[graphs[graph_id].ui32_data_end_index];
//      graphs[graph_id].ui32_graph_data_y_min_counter = 1;
//    }
//    // equal to min point
//    else if(graphs[graph_id].ui32_data[graphs[graph_id].ui32_data_end_index] == graphs[graph_id].ui32_graph_data_y_min)
//    {
//      graphs[graph_id].ui32_graph_data_y_min_counter++;
//    }
//
//    // new max point
//    if(graphs[graph_id].ui32_data[graphs[graph_id].ui32_data_end_index] > graphs[graph_id].ui32_graph_data_y_max)
//    {
//      graphs[graph_id].ui32_graph_data_y_max = graphs[graph_id].ui32_data[graphs[graph_id].ui32_data_end_index];
//      graphs[graph_id].ui32_graph_data_y_max_counter = 1;
//    }
//    // equal to max point
//    else if(graphs[graph_id].ui32_data[graphs[graph_id].ui32_data_end_index] == graphs[graph_id].ui32_graph_data_y_max)
//    {
//      graphs[graph_id].ui32_graph_data_y_max_counter++;
//    }
//  }
//
//  // we will be adding and removing points to graph
//  else
//  {
    graphs_measurements_search_min_y(graph_id);
    graphs_measurements_search_max_y(graph_id);
//  }
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

void graphs_draw_title(volatile lcd_vars_t *p_lcd_vars, uint32_t ui32_state)
{
  graphs_id_t graph_id = p_m_l3_vars->graph_id;
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;

  // first clean the area
  if(ui32_state == 1)
  {
    UG_FillFrame(0,
                 326,
                 DISPLAY_WIDTH - 1,
                 326 + SMALL_TEXT_FONT.char_height,
                 C_BLACK);
  }

  // draw tittle
  if(p_lcd_vars->ui32_main_screen_draw_static_info ||
      ui32_state == 2)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(MAIN_SCREEN_FIELD_LABELS_COLOR);
    UG_FontSelect(&SMALL_TEXT_FONT);
    ui32_y_position = 326;

    switch(graph_id)
    {
      case GRAPH_WHEEL_SPEED:
        ui32_x_position = 100;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "wheel speed");
      break;

      case GRAPH_PEDAL_HUMAN_POWER:
        ui32_x_position = 100;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "human power");
      break;

      case GRAPH_PEDAL_CADENCE:
        ui32_x_position = 120;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "cadence");
      break;

      case GRAPH_BATTERY_VOLTAGE:
        ui32_x_position = 80;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "battery voltage");
      break;

      case GRAPH_BATTERY_CURRENT:
        ui32_x_position = 80;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "battery current");
      break;

      case GRAPH_BATTERY_SOC:
        ui32_x_position = 100;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "battery SOC");
      break;

      case GRAPH_MOTOR_POWER:
        ui32_x_position = 100;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "motor power");
      break;

      case GRAPH_MOTOR_TEMPERATURE:
        ui32_x_position = 70;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "motor temperature");
      break;

      case GRAPH_MOTOR_PWM_DUTY_CYCLE:
        ui32_x_position = 70;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "motor duty-cycle");
      break;

      case GRAPH_MOTOR_ERPS:
        ui32_x_position = 110;
        UG_PutString(ui32_x_position,
                     ui32_y_position,
                     "motor erps");
      break;

      case GRAPH_MOTOR_FOC_ANGLE:
        ui32_x_position = 80;
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
}
