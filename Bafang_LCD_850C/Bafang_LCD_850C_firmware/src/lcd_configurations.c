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

typedef struct _menu_data
{
  uint8_t ui8_edit_state;
  uint8_t ui8_visible_item;
  uint8_t ui8_screen_set_values;
  buttons_events_type_t menu_buttons_events;
} struct_menu_data;

static struct_motor_controller_data *p_motor_controller_data;
static struct_configuration_variables *p_configuration_variables;

static struct_lcd_configurations_vars lcd_configurations_vars =
{
  .ui32_configurations_screen_draw_static_info = 0
};

static struct_menu_data menu_data =
{
  .ui8_edit_state = 0,
  .ui8_visible_item = 0,
  .menu_buttons_events = 0,
  .ui8_screen_set_values = 0
};

struct_lcd_vars *p_lcd_vars;

static uint8_t ui8_item_number = 0;
static uint8_t ui8_previous_item_number = 0xff;
static uint8_t ui8_item_visible_start_index = 0;
static uint8_t ui8_item_visible_index = 1;
static uint16_t ui16_conf_screen_first_item_y_offset = 61;

void draw_configurations_screen_mask(void);
void clear_configurations_screen_items(void);
void draw_title_symbol(struct_menu_data *p_menu_data);
void draw_item_index_symbol(struct_menu_data *p_menu_data);
void configurations_screen_item_title_set_strings(uint8_t *ui8_p_string, struct_menu_data *p_menu_data);
void configurations_screen_item_set_strings(uint8_t *ui8_p_string1, uint8_t *ui8_p_string2, struct_menu_data *p_menu_data);
void wheel_speed_title(struct_menu_data *p_menu_data);
void wheel_max_speed(struct_menu_data *p_menu_data);
void wheel_perimeter(struct_menu_data *p_menu_data);
void wheel_speed_units(struct_menu_data *p_menu_data);
void battery_title(struct_menu_data *p_menu_data);
void battery_max_current(struct_menu_data *p_menu_data);
void battery_low_cut_off_voltage(struct_menu_data *p_menu_data);
void battery_number_cells(struct_menu_data *p_menu_data);
void battery_resistance(struct_menu_data *p_menu_data);
void battery_soc_title(struct_menu_data *p_menu_data);
void battery_soc_enable(struct_menu_data *p_menu_data);
void battery_soc_increment_decrement(struct_menu_data *p_menu_data);
void battery_soc_voltage_to_reset(struct_menu_data *p_menu_data);
void battery_soc_total_watt_hour(struct_menu_data *p_menu_data);

// call each function on the array
void (*p_items_array[])(struct_menu_data *p_menu_data) = {
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
  p_motor_controller_data = get_motor_controller_data();
  p_configuration_variables = get_configuration_variables();
}

void lcd_configurations_screen(void)
{
  static uint8_t ui8_first_time = 1;
  uint8_t ui8_i;

  // leave config menu with a button_onoff_long_click
  if (buttons_get_onoff_long_click_event ())
  {
    buttons_clear_onoff_long_click_event ();

    p_lcd_vars->ui32_main_screen_draw_static_info = 1;
    p_lcd_vars->lcd_screen_state = LCD_SCREEN_MAIN;
    ui8_first_time = 1;
    return;
  }

  // enter/leave screen set values
  if (buttons_get_onoff_click_event ())
  {
    buttons_clear_onoff_click_event ();
    buttons_clear_onoff_long_click_event ();
    buttons_clear_up_click_event ();
    buttons_clear_down_click_event ();
    menu_data.menu_buttons_events = 0;

    if(menu_data.ui8_screen_set_values) { menu_data.ui8_screen_set_values = 0; }
    else { menu_data.ui8_screen_set_values = 1; }
  }

  // now get buttons events
  //
  if(buttons_get_down_click_event ())
  {
    buttons_clear_up_click_event ();
    buttons_clear_down_click_event ();
    menu_data.menu_buttons_events = DOWN_CLICK;

    // execute net code only if weare not setting the values of variables
    if(!menu_data.ui8_screen_set_values)
    {
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
  }

  if(buttons_get_up_click_event ())
  {
    buttons_clear_up_click_event ();
    buttons_clear_down_click_event ();
    menu_data.menu_buttons_events = UP_CLICK;

    if(!menu_data.ui8_screen_set_values)
    {
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
  }

  // to draw static info
  if(lcd_configurations_vars.ui32_configurations_screen_draw_static_info)
  {
    if (ui8_first_time)
    {
      UG_FillScreen(C_BLACK);
      draw_configurations_screen_mask();
      ui8_first_time = 0;
    }
    else
    {
      clear_configurations_screen_items();
    }
  }

  for(ui8_i = 0; ui8_i < MAX_ITEMS_PER_SCREEN; ui8_i++)
  {
    // find which item we are pointing to/editing
    if((ui8_item_visible_index - 1) == ui8_i)
    {
      menu_data.ui8_edit_state = 1;
    }
    else
    {
      menu_data.ui8_edit_state = 0;
    }

    menu_data.ui8_visible_item = ui8_i;
    // call each function on the array
    (*p_items_array[ui8_item_visible_start_index + ui8_i])(&menu_data);

    draw_item_index_symbol(&menu_data);
  }

  // track state for item number change
  if(ui8_item_number != ui8_previous_item_number)
  {
    ui8_previous_item_number = ui8_item_number;
  }

  // clear this variable after 1 full cycle running
  lcd_configurations_vars.ui32_configurations_screen_draw_static_info = 0;
}

void draw_configurations_screen_mask(void)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  uint32_t ui32_counter;

  ui32_x_position = 0;
  ui32_y_position = 0;
  UG_FillFrame(ui32_x_position, ui32_y_position, ui32_x_position + DISPLAY_WIDTH - 1, ui32_y_position + 59, C_DARK_BLUE);

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
    UG_DrawLine(ui32_x_position, ui32_y_position, DISPLAY_WIDTH - 1, ui32_y_position, C_DIM_GRAY);
    ui32_y_position += 50;
  }
}

void clear_configurations_screen_items(void)
{
  uint32_t ui32_x_position = 0;
  uint32_t ui32_y_position;
  uint8_t ui8_counter;

  for(ui8_counter = 0; ui8_counter < MAX_ITEMS_PER_SCREEN; ui8_counter++)
  {
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        (ui8_counter * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position + 1, ui32_x_position + DISPLAY_WIDTH - 1, ui32_y_position + 48, C_BLACK);
  }
}

void wheel_speed_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Wheel speed", p_menu_data);
  draw_title_symbol(p_menu_data);
}

void wheel_max_speed(struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  static uint8_t ui8_wheel_max_speed_previous = 0xff;

  configurations_screen_item_set_strings("Max wheel speed", "(km/h)", p_menu_data);

  // if we are in edit mode...
  if(p_menu_data->ui8_screen_set_values &&
      p_menu_data->ui8_edit_state)
  {
    if(p_menu_data->menu_buttons_events == UP_CLICK)
    {
      p_configuration_variables->ui8_wheel_max_speed++;
      if (p_configuration_variables->ui8_wheel_max_speed >= 99) { p_configuration_variables->ui8_wheel_max_speed = 99; }
    }

    if(p_menu_data->menu_buttons_events == DOWN_CLICK)
    {
        p_configuration_variables->ui8_wheel_max_speed--;
      if (p_configuration_variables->ui8_wheel_max_speed < 1)  { p_configuration_variables->ui8_wheel_max_speed = 1; }
    }

    p_menu_data->menu_buttons_events = 0;
  }

  // clear at every 500ms
  if((p_menu_data->ui8_screen_set_values) &&
      (p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 1) &&
      (p_menu_data->ui8_edit_state))
  {
    ui32_x_position = DISPLAY_WIDTH - 40 - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, ui32_x_position + 24, ui32_y_position + 20, C_BLACK);
  }
  // draw value at every 500ms or when it changes
  else if((p_menu_data->ui8_screen_set_values) &&
      (p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 2) &&
      (p_menu_data->ui8_edit_state))
  {
    ui32_x_position = DISPLAY_WIDTH - 40 - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, ui32_x_position + 24, ui32_y_position + 20, C_BLACK);

    // draw variable value
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&REGULAR_TEXT_FONT);
    ui32_x_position = DISPLAY_WIDTH - 40 - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_PutString(ui32_x_position, ui32_y_position, itoa(p_configuration_variables->ui8_wheel_max_speed));
  }
  else if(!(p_menu_data->ui8_screen_set_values) &&
      (p_menu_data->ui8_edit_state) &&
      (p_configuration_variables->ui8_wheel_max_speed != ui8_wheel_max_speed_previous))
  {
    ui8_wheel_max_speed_previous = p_configuration_variables->ui8_wheel_max_speed;

    ui32_x_position = DISPLAY_WIDTH - 40 - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, ui32_x_position + 24, ui32_y_position + 20, C_BLACK);

    // draw variable value
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&REGULAR_TEXT_FONT);
    ui32_x_position = DISPLAY_WIDTH - 40 - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_PutString(ui32_x_position, ui32_y_position, itoa(p_configuration_variables->ui8_wheel_max_speed));
  }
}

void wheel_perimeter(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Wheel perimeter", "(millimeters)", p_menu_data);
}

void wheel_speed_units(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Speed units", "", p_menu_data);
}

void battery_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Battery", p_menu_data);
  draw_title_symbol(p_menu_data);
}

void battery_max_current(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Max current", "(amps)", p_menu_data);
}

void battery_low_cut_off_voltage(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Low cut-off voltage", "(volts)", p_menu_data);
}

void battery_number_cells(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Number of cells", "", p_menu_data);
}

void battery_resistance(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Resistance", "(milli ohms)", p_menu_data);
}

void battery_soc_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Battery SOC", p_menu_data);
  draw_title_symbol(p_menu_data);
}

void battery_soc_enable(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Feature", "(enable/disable)", p_menu_data);
}

void battery_soc_increment_decrement(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Decrement", "or increment", p_menu_data);
}

void battery_soc_voltage_to_reset(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Voltage to reset", "", p_menu_data);
}

void battery_soc_total_watt_hour(struct_menu_data *p_menu_data)
{
  configurations_screen_item_set_strings("Total watts/hour", "", p_menu_data);
}

void configurations_screen_item_title_set_strings(uint8_t *ui8_p_string, struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;

  // update only when item number changes
  if(ui8_item_number != ui8_previous_item_number)
  {
    ui32_x_position = 0;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        (p_menu_data->ui8_visible_item * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, ui32_x_position + DISPLAY_WIDTH - 1, ui32_y_position + 48, C_DIM_GRAY);

    UG_SetBackcolor(C_DIM_GRAY);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&TITLE_TEXT_FONT);
    ui32_x_position = 6;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);

    UG_PutString(ui32_x_position, ui32_y_position, ui8_p_string);
  }
}

void configurations_screen_item_set_strings(uint8_t *ui8_p_string1, uint8_t *ui8_p_string2, struct_menu_data *p_menu_data)
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
        (p_menu_data->ui8_visible_item * 50);
    UG_PutString(ui32_x_position, ui32_y_position, ui8_p_string1);

    UG_FontSelect(&SMALL_TEXT_FONT);
    ui32_y_position += 23;
    UG_PutString(ui32_x_position, ui32_y_position, ui8_p_string2);
  }
}

void draw_title_symbol(struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position ;
  uint32_t ui32_y_position;
  uint8_t ui8_counter;
  uint32_t ui32_line_center;
  uint32_t ui32_line_lenght;

  // update only when item number changes
  if(ui8_item_number != ui8_previous_item_number)
  {
    ui32_x_position = DISPLAY_WIDTH - 14 - 10;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    ui32_line_lenght = 1;

    for(ui8_counter = 0; ui8_counter < 10; ui8_counter++)
    {
      UG_DrawLine(ui32_x_position, ui32_y_position, ui32_x_position + ui32_line_lenght, ui32_y_position, C_WHITE);
      ui32_x_position--;
      if (ui32_line_lenght == 1) { ui32_line_lenght++; }
      else { ui32_line_lenght += 2; }
      ui32_y_position++;
    }

    for(ui8_counter = 0; ui8_counter < 11; ui8_counter++)
    {
      UG_DrawLine(ui32_x_position, ui32_y_position, ui32_x_position + ui32_line_lenght, ui32_y_position, C_WHITE);
      ui32_x_position++;
      if (ui32_line_lenght == 1) { ui32_line_lenght++; }
      else { ui32_line_lenght -= 2; }
      ui32_y_position++;
    }
  }
}

void draw_item_index_symbol(struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position ;
  uint32_t ui32_y_position;
  uint8_t ui8_counter;
  uint32_t ui32_line_lenght;
  uint16_t ui16_color;

  // clear at every 500ms
  if((p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 1) &&
      (p_menu_data->ui8_edit_state) &&
      (!p_menu_data->ui8_screen_set_values))
  {
    ui16_color = C_WHITE;

    ui32_x_position = DISPLAY_WIDTH - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    ui32_line_lenght = 0;

    for(ui8_counter = 0; ui8_counter < 10; ui8_counter++)
    {
      UG_DrawLine(ui32_x_position, ui32_y_position, ui32_x_position + ui32_line_lenght, ui32_y_position, ui16_color);
      ui32_x_position--;
      ui32_line_lenght++;
      ui32_y_position++;
    }

    for(ui8_counter = 0; ui8_counter < 10; ui8_counter++)
    {
      UG_DrawLine(ui32_x_position, ui32_y_position, ui32_x_position + ui32_line_lenght, ui32_y_position, ui16_color);
      ui32_x_position++;
      ui32_line_lenght--;
      ui32_y_position++;
    }
  }
  // draw value at every 500ms or when it changes
  else if((p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 2) &&
      (menu_data.ui8_edit_state))
  {
    ui16_color = C_RED;

    ui32_x_position = DISPLAY_WIDTH - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    ui32_line_lenght = 0;

    for(ui8_counter = 0; ui8_counter < 10; ui8_counter++)
    {
      UG_DrawLine(ui32_x_position, ui32_y_position, ui32_x_position + ui32_line_lenght, ui32_y_position, ui16_color);
      ui32_x_position--;
      ui32_line_lenght++;
      ui32_y_position++;
    }

    for(ui8_counter = 0; ui8_counter < 10; ui8_counter++)
    {
      UG_DrawLine(ui32_x_position, ui32_y_position, ui32_x_position + ui32_line_lenght, ui32_y_position, ui16_color);
      ui32_x_position++;
      ui32_line_lenght--;
      ui32_y_position++;
    }
  }
}

struct_lcd_configurations_vars* get_lcd_configurations_vars(void)
{
  return &lcd_configurations_vars;
}
