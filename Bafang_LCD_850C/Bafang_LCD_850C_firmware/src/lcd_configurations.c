/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stdio.h"
#include <math.h>

#include "main.h"
#include "config.h"
#include "utils.h"
#include "pins.h"
#include "lcd.h"
#include "lcd_configurations.h"
#include "buttons.h"
#include "eeprom.h"
#include "ugui_driver/ugui_bafang_500c.h"
#include "ugui/ugui.h"

#define MAX_ITEMS                 (14 - 1)
#define MAX_ITEMS_PER_SCREEN      8
#define MAX_ITEMS_VISIBLE_INDEX   ((MAX_ITEMS + 1) - MAX_ITEMS_PER_SCREEN)

static struct_motor_controller_data motor_controller_data;
static struct_configuration_variables configuration_variables;

static struct_lcd_configurations_vars lcd_configurations_vars =
{
  .ui32_configurations_screen_draw_static_info = 0
};

struct_lcd_vars *p_lcd_vars;

static uint8_t ui8_item_number = 0;
static uint8_t ui8_previous_item_number = 0xff;
static uint8_t ui8_item_visible_start_index = 0;
static uint8_t ui8_item_visible_index = 1;
static uint16_t ui16_conf_screen_first_item_y_offset = 61;

void lcd_draw_configurations_screen_mask(void);
void write_edit_mark(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void configurations_screen_item_title_set_strings(uint8_t *ui8_p_string, uint8_t ui8_visible_item);
void configurations_screen_item_set_strings(uint8_t *ui8_p_string1, uint8_t *ui8_p_string2, uint8_t ui8_visible_item);
void wheel_speed_title(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void wheel_max_speed(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void wheel_perimeter(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void wheel_speed_units(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_title(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_max_current(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_low_cut_off_voltage(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_number_cells(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_resistance(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_soc_title(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_soc_enable(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_soc_increment_decrement(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_soc_voltage_to_reset(uint8_t ui8_edit_state, uint8_t ui8_visible_item);
void battery_soc_total_watt_hour(uint8_t ui8_edit_state, uint8_t ui8_visible_item);

// call each function on the array
void (*p_items_array[])(uint8_t ui8_edit_state, uint8_t ui8_visible_item) = {
  wheel_speed_title,
  wheel_max_speed,
  wheel_perimeter,
  wheel_speed_units,
  battery_title,
  battery_max_current,
  battery_low_cut_off_voltage,
  battery_number_cells,
  battery_resistance,
  battery_soc_title,
  battery_soc_enable,
  battery_soc_increment_decrement,
  battery_soc_voltage_to_reset,
  battery_soc_total_watt_hour
};

void lcd_configurations_screen_init(void)
{
  p_lcd_vars = get_lcd_vars();
}

void lcd_configurations_screen(void)
{
  uint8_t ui8_i;
  uint8_t ui8_edit_state;

  // leave config menu with a button_onoff_long_click
  if (buttons_get_onoff_long_click_event ())
  {
    buttons_clear_onoff_long_click_event ();

    p_lcd_vars->ui32_main_screen_draw_static_info = 1;
    p_lcd_vars->lcd_screen_state = LCD_SCREEN_MAIN;
    return;
  }

  // now get buttons events
  //
  if(buttons_get_down_click_event ())
  {
    buttons_clear_down_click_event ();

    // increment to next item
    if(ui8_item_number < MAX_ITEMS)
    {
      ui8_item_number++;
      lcd_configurations_vars.ui32_configurations_screen_draw_static_info = 1;
    }

    // increment to next visible item
    if(ui8_item_visible_index < MAX_ITEMS_PER_SCREEN)
    {
      ui8_item_visible_index++;
    }
    // visible item limit, so increment the start index of visible item
    else
    {
      // do not increment more over the last item
      if(ui8_item_visible_start_index < MAX_ITEMS_VISIBLE_INDEX)
      {
        ui8_item_visible_start_index++;
      }
    }
  }

  if(buttons_get_up_click_event ())
  {
    buttons_clear_up_click_event ();

    // decrement to next item
    if(ui8_item_number > 0)
    {
      ui8_item_number--;
      lcd_configurations_vars.ui32_configurations_screen_draw_static_info = 1;
    }

    // decrement to next visible item
    if(ui8_item_visible_index > 1)
    {
      ui8_item_visible_index--;
    }
    // visible item limit, so decrement the start index of visible item
    else
    {
      // do not decrement more over the last item
      if(ui8_item_visible_start_index > 0)
      {
        ui8_item_visible_start_index--;
      }
    }
  }

  // to draw static info
  if(lcd_configurations_vars.ui32_configurations_screen_draw_static_info)
  {
    UG_FillScreen(C_BLACK);
    lcd_draw_configurations_screen_mask();
  }

  for(ui8_i = 0; ui8_i < MAX_ITEMS_PER_SCREEN; ui8_i++)
  {
    // find which item we are pointing to/editing
    if((ui8_item_visible_index - 1) == ui8_i)
    {
      ui8_edit_state = 1;
    }
    else
    {
      ui8_edit_state = 0;
    }

    // call each function on the array
    (*p_items_array[ui8_item_visible_start_index + ui8_i])(ui8_edit_state, ui8_i);
  }

  // track state for item number change
  if(ui8_item_number != ui8_previous_item_number)
  {
    ui8_previous_item_number = ui8_item_number;
  }

  // clear this variable after 1 full cycle running
  lcd_configurations_vars.ui32_configurations_screen_draw_static_info = 0;
}

void lcd_draw_configurations_screen_mask(void)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  uint32_t ui32_counter;

  ui32_x_position = 0;
  ui32_y_position = 0;
  UG_FillFrame(ui32_x_position, ui32_y_position, ui32_x_position + DISPLAY_WIDTH, ui32_y_position + 59, C_DARK_BLUE);

  UG_SetBackcolor(C_DARK_BLUE);
  UG_SetForecolor(C_WHITE);
  UG_FontSelect(&TITLE_TEXT_FONT);
  ui32_x_position = 42;
  ui32_y_position = 16;
  UG_PutString(ui32_x_position, ui32_y_position, "CONFIGURATIONS");

  ui32_x_position = 0;
  ui32_y_position = 60;
  for (ui32_counter = 0; ui32_counter < 9; ui32_counter++)
  {
    UG_DrawLine(ui32_x_position, ui32_y_position, DISPLAY_WIDTH, ui32_y_position, C_DIM_GRAY);
    ui32_y_position += 50;
  }
}

void wheel_speed_title(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_title_set_strings("Wheel speed", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void wheel_max_speed(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Max wheel speed", "(km/h)", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void wheel_perimeter(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Wheel perimeter", "(millimeters)", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void wheel_speed_units(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Speed units", "", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_title(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_title_set_strings("Battery", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_max_current(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Max current", "(amps)", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_low_cut_off_voltage(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Low cut-off voltage", "(volts)", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_number_cells(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Number of cells", "(ex: 13 for 48V battery)", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_resistance(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Resistance", "(milli ohms)", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_soc_title(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_title_set_strings("Battery SOC", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_soc_enable(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Feature", "(enable/disable)", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_soc_increment_decrement(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Decrement", "or increment", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_soc_voltage_to_reset(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Voltage to reset", "", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void battery_soc_total_watt_hour(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  configurations_screen_item_set_strings("Total watts/hour", "", ui8_visible_item);
  write_edit_mark(ui8_edit_state, ui8_visible_item);
}

void configurations_screen_item_title_set_strings(uint8_t *ui8_p_string, uint8_t ui8_visible_item)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;

  // update only when item number changes
  if(ui8_item_number != ui8_previous_item_number)
  {
    ui32_x_position = 0;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        (ui8_visible_item * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, ui32_x_position + DISPLAY_WIDTH, ui32_y_position + 49, C_DIM_GRAY);

    UG_SetBackcolor(C_DIM_GRAY);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&TITLE_TEXT_FONT);
    ui32_x_position = 6;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (ui8_visible_item * 50);

    UG_PutString(ui32_x_position, ui32_y_position, ui8_p_string);
  }
}

void configurations_screen_item_set_strings(uint8_t *ui8_p_string1, uint8_t *ui8_p_string2, uint8_t ui8_visible_item)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;

  // update only when item number changes
  if(ui8_item_number != ui8_previous_item_number)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&REGULAR_TEXT_FONT);
    ui32_x_position = 6;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        4 + // padding from top line
        (ui8_visible_item * 50);
    UG_PutString(ui32_x_position, ui32_y_position, ui8_p_string1);

    UG_FontSelect(&SMALL_TEXT_FONT);
    ui32_y_position += 23;
    UG_PutString(ui32_x_position, ui32_y_position, ui8_p_string2);
  }
}

void write_edit_mark(uint8_t ui8_edit_state, uint8_t ui8_visible_item)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;

  // update only when item number changes
  if((ui8_item_number != ui8_previous_item_number) &&
      (ui8_edit_state))
  {
    ui32_x_position = 310;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        24 + // middle space
        (ui8_visible_item * 50);
    UG_FillCircle(ui32_x_position, ui32_y_position, 4, C_RED);
  }
}

struct_lcd_configurations_vars* get_lcd_configurations_vars(void)
{
  return &lcd_configurations_vars;
}

