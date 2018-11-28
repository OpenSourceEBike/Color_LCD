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
  uint8_t ui8_visible_item_edit;
  uint8_t ui8_screen_set_values;
  uint8_t ui8_item_increment;
  buttons_events_type_t menu_buttons_events;
} struct_menu_data;

typedef struct _var_number
{
  void *p_var_number;
  uint8_t ui8_size;
  uint8_t ui8_number_digits;
  uint8_t ui8_decimal_digit;
  uint32_t ui32_max_value;
  uint32_t ui32_min_value;
  uint32_t ui32_increment_step;
} struct_var_number;

static struct_motor_controller_data *p_motor_controller_data;
static struct_configuration_variables *p_configuration_variables;

static struct_lcd_configurations_vars lcd_configurations_vars =
{
  .ui8_configurations_screen_draw_static_info = 0,
  .ui8_configurations_screen_draw_static_info_first_time = 0,
  .ui8_item_number = 0,
  .ui8_previous_item_number = 0xff,
  .ui8_item_visible_start_index = 0,
  .ui8_item_visible_index = 1
};

static struct_menu_data menu_data =
{
  .ui8_edit_state = 0,
  .ui8_visible_item = 0,
  .ui8_visible_item_edit = 0,
  .menu_buttons_events = 0,
  .ui8_screen_set_values = 0,
  .ui8_item_increment = 1
};

struct_lcd_vars *p_lcd_vars;

static uint16_t ui16_conf_screen_first_item_y_offset = 61;

void draw_configurations_screen_mask(void);
void clear_configurations_screen_items(void);
void draw_item_index_symbol(struct_menu_data *p_menu_data);
void configurations_screen_item_title_set_strings(uint8_t *ui8_p_string, struct_menu_data *p_menu_data);
void item_set_strings(uint8_t *ui8_p_string1, uint8_t *ui8_p_string2, struct_menu_data *p_menu_data);
void item_var_set_number(struct_var_number *p_lcd_var_number, struct_menu_data *p_menu_data);
void item_var_set_strings(struct_var_number *p_lcd_var_number, struct_menu_data *p_menu_data, uint8_t *p_strings);
void item_visible_manage(struct_menu_data *p_menu_data);
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
void (*p_items_array[])(struct_menu_data *p_menu_data) =
{
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

uint8_t items_array_is_title[] =
{
  0, // hack: first is always a title but let's signal it is not
  0,
  0,
  0,
  1,
  0,
  0,
  0,
  0,
  1,
  0,
  0,
  0,
  0
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

    menu_data.ui8_item_increment = 1;
    item_visible_manage(&menu_data);

    // see if next item is a tittle
    if(items_array_is_title[lcd_configurations_vars.ui8_item_number] == 1)
    {
      menu_data.ui8_item_increment = 1;
      item_visible_manage(&menu_data);
    }
  }

  if(buttons_get_up_click_event ())
  {
    buttons_clear_up_click_event ();
    buttons_clear_down_click_event ();
    menu_data.menu_buttons_events = UP_CLICK;

    menu_data.ui8_item_increment = 0;
    item_visible_manage(&menu_data);

    // see if next item is a tittle
    if(items_array_is_title[lcd_configurations_vars.ui8_item_number] == 1)
    {
      menu_data.ui8_item_increment = 0;
      item_visible_manage(&menu_data);
    }
  }

  // to draw static info
  if(lcd_configurations_vars.ui8_configurations_screen_draw_static_info)
  {
    if (lcd_configurations_vars.ui8_configurations_screen_draw_static_info_first_time == 1)
    {
      UG_FillScreen(C_BLACK);
      draw_configurations_screen_mask();
      lcd_configurations_vars.ui8_configurations_screen_draw_static_info_first_time = 0;
      menu_data.ui8_screen_set_values = 0;
      menu_data.ui8_edit_state = 0;

      lcd_configurations_vars.ui8_item_number = 0;
      lcd_configurations_vars.ui8_previous_item_number = 0xff;
      lcd_configurations_vars.ui8_item_visible_start_index = 0;
      lcd_configurations_vars.ui8_item_visible_index = 0;
      menu_data.ui8_item_increment = 1;
      item_visible_manage(&menu_data);
    }
    else
    {
      clear_configurations_screen_items();
    }
  }

  for(ui8_i = 0; ui8_i < MAX_ITEMS_PER_SCREEN; ui8_i++)
  {
    // find which item we are pointing to/editing
    if(lcd_configurations_vars.ui8_item_visible_index == ui8_i)
    {
      menu_data.ui8_edit_state = 1;
      menu_data.ui8_visible_item_edit = ui8_i;
    }
    else
    {
      menu_data.ui8_edit_state = 0;
    }

    menu_data.ui8_visible_item = ui8_i;
    // call each function on the array
    (*p_items_array[lcd_configurations_vars.ui8_item_visible_start_index + ui8_i])(&menu_data);

    draw_item_index_symbol(&menu_data);
  }

  // track state for item number change
  if(lcd_configurations_vars.ui8_item_number != lcd_configurations_vars.ui8_previous_item_number)
  {
    lcd_configurations_vars.ui8_previous_item_number = lcd_configurations_vars.ui8_item_number;
  }

  // clear this variable after 1 full cycle running
  lcd_configurations_vars.ui8_configurations_screen_draw_static_info = 0;
}

void item_visible_manage(struct_menu_data *p_menu_data)
{
  if(p_menu_data->ui8_item_increment)
  {
    // execute next code only if we are not setting the values of variables
    if(!p_menu_data->ui8_screen_set_values)
    {
      // increment to next item
      if(lcd_configurations_vars.ui8_item_number < MAX_ITEMS)
      {
        lcd_configurations_vars.ui8_item_number++;
        lcd_configurations_vars.ui8_configurations_screen_draw_static_info = 1;
      }

      // increment to next visible item
      if(lcd_configurations_vars.ui8_item_visible_index < (MAX_ITEMS_PER_SCREEN - 1))
      {
        lcd_configurations_vars.ui8_item_visible_index++;
      }
      // visible item limit, so increment the start index of visible item
      else
      {
        // do not increment more over the last item
        if(lcd_configurations_vars.ui8_item_visible_start_index < MAX_ITEMS_VISIBLE_INDEX)
        {
          lcd_configurations_vars.ui8_item_visible_start_index++;
        }
      }
    }
  }
  else
  {
    if(!menu_data.ui8_screen_set_values)
    {
      // decrement to next item
      if(lcd_configurations_vars.ui8_item_number > 0)
      {
        lcd_configurations_vars.ui8_item_number--;
        lcd_configurations_vars.ui8_configurations_screen_draw_static_info = 1;
      }

      // decrement to next visible item
      if(lcd_configurations_vars.ui8_item_visible_index > 1)
      {
        lcd_configurations_vars.ui8_item_visible_index--;
      }
      // visible item limit, so decrement the start index of visible item
      else
      {
        // do not decrement more over the last item
        if(lcd_configurations_vars.ui8_item_visible_start_index > 0)
        {
          lcd_configurations_vars.ui8_item_visible_start_index--;
        }
      }
    }
  }

  // exception: force first item to never stay on title
  if(lcd_configurations_vars.ui8_item_visible_start_index == 0 &&
      lcd_configurations_vars.ui8_item_number == 0)
  {
    lcd_configurations_vars.ui8_item_number = 1;
    lcd_configurations_vars.ui8_configurations_screen_draw_static_info = 0;
  }
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
}

void wheel_max_speed(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui8_wheel_max_speed,
    .ui8_size = 8,
    .ui8_number_digits = 2,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 99,
    .ui32_min_value = 1,
    .ui32_increment_step = 1
  };

  item_set_strings("Max wheel speed", "(km/h)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void wheel_perimeter(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui16_wheel_perimeter,
    .ui8_size = 16,
    .ui8_number_digits = 4,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 3000,
    .ui32_min_value = 750,
    .ui32_increment_step = 10
  };

  item_set_strings("Wheel perimeter", "(millimeters)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void wheel_speed_units(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui8_units_type,
    .ui8_size = 0,
    .ui8_number_digits = 0,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Speed units", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "km/h\nmph");
}

void battery_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Battery", p_menu_data);
}

void battery_max_current(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui8_battery_max_current,
    .ui8_size = 8,
    .ui8_number_digits = 2,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 30,
    .ui32_min_value = 1,
    .ui32_increment_step = 1
  };

  item_set_strings("Max current", "(amps)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void battery_low_cut_off_voltage(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui16_battery_low_voltage_cut_off_x10,
    .ui8_size = 16,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 630,
    .ui32_min_value = 160,
    .ui32_increment_step = 1
  };

  item_set_strings("Low cut-off", "voltage (volts)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void battery_number_cells(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui8_battery_cells_number,
    .ui8_size = 8,
    .ui8_number_digits = 2,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 14,
    .ui32_min_value = 7,
    .ui32_increment_step = 1
  };

  item_set_strings("Number of cells", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void battery_resistance(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui16_battery_pack_resistance_x1000,
    .ui8_size = 16,
    .ui8_number_digits = 4,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1000,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Resistance", "(milli ohms)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void battery_soc_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Battery SOC", p_menu_data);
}

void battery_soc_enable(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui8_battery_soc_enable,
    .ui8_size = 0,
    .ui8_number_digits = 0,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Feature", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "enable\ndisable");
}

void battery_soc_increment_decrement(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui8_battery_soc_increment_decrement,
    .ui8_size = 0,
    .ui8_number_digits = 0,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Decrement", "or increment", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "inc\ndec");
}

void battery_soc_voltage_to_reset(struct_menu_data *p_menu_data)
{
  struct_var_number lcd_var_number =
  {
    .p_var_number = &p_configuration_variables->ui16_battery_voltage_reset_wh_counter_x10,
    .ui8_size = 16,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 630,
    .ui32_min_value = 160,
    .ui32_increment_step = 1
  };

  item_set_strings("Voltage to reset", "(volts)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void battery_soc_total_watt_hour(struct_menu_data *p_menu_data)
{
  uint32_t ui32_value;

  ui32_value = p_configuration_variables->ui32_wh_x10_100_percent / 10;
  struct_var_number lcd_var_number =
  {
    .p_var_number = &ui32_value,
    .ui8_size = 32,
    .ui8_number_digits = 4,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 9990,
    .ui32_min_value = 0,
    .ui32_increment_step = 10
  };

  item_set_strings("Battery total", "watts/hour", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
  p_configuration_variables->ui32_wh_x10_100_percent = ui32_value * 10;
}

void configurations_screen_item_title_set_strings(uint8_t *ui8_p_string, struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;

  // update only when item number changes
  if(lcd_configurations_vars.ui8_item_number != lcd_configurations_vars.ui8_previous_item_number)
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

void item_set_strings(uint8_t *ui8_p_string1, uint8_t *ui8_p_string2, struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;

  // update only when item number changes
  if(lcd_configurations_vars.ui8_item_number != lcd_configurations_vars.ui8_previous_item_number)
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

void item_var_set_number(struct_var_number *p_lcd_var_number, struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  uint8_t ui8_draw_var_value = 0;
  static uint8_t ui8_draw_var_value_state = 0;
  uint32_t ui32_value;
  uint32_t ui32_value_temp;
  uint32_t ui32_value_integer;
  uint32_t ui32_value_decimal;
  uint32_t ui32_value_integer_number_digits;
  uint8_t ui8_counter;
  uint8_t *ui8_p_var;
  uint16_t *ui16_p_var;
  uint32_t *ui32_p_var;

  if(p_lcd_var_number->ui8_size == 8)
  {
    ui8_p_var = ((uint8_t *) p_lcd_var_number->p_var_number);
  }
  else if(p_lcd_var_number->ui8_size == 16)
  {
    ui16_p_var = ((uint16_t *) p_lcd_var_number->p_var_number);
  }
  else if(p_lcd_var_number->ui8_size == 32)
  {
    ui32_p_var = ((uint32_t *) p_lcd_var_number->p_var_number);
  }

  // if we are in edit mode...
  if(p_menu_data->ui8_screen_set_values &&
      p_menu_data->ui8_edit_state)
  {
    if(p_menu_data->menu_buttons_events == UP_CLICK)
    {
      if(p_lcd_var_number->ui8_size == 8)
      {
        if((*ui8_p_var) <= (p_lcd_var_number->ui32_max_value - p_lcd_var_number->ui32_increment_step)) { (*ui8_p_var) += p_lcd_var_number->ui32_increment_step; }
      }
      else if(p_lcd_var_number->ui8_size == 16)
      {
        if((*ui16_p_var) <= (p_lcd_var_number->ui32_max_value - p_lcd_var_number->ui32_increment_step)) { (*ui16_p_var) += p_lcd_var_number->ui32_increment_step; }
      }
      else if(p_lcd_var_number->ui8_size == 32)
      {
        if((*ui32_p_var) <= (p_lcd_var_number->ui32_max_value - p_lcd_var_number->ui32_increment_step)) { (*ui32_p_var) += p_lcd_var_number->ui32_increment_step; }
      }

      ui8_draw_var_value = 1;
    }

    if(p_menu_data->menu_buttons_events == DOWN_CLICK)
    {
      if(p_lcd_var_number->ui8_size == 8)
      {
        if((*ui8_p_var) >= (p_lcd_var_number->ui32_min_value + p_lcd_var_number->ui32_increment_step)) { (*ui8_p_var) -= p_lcd_var_number->ui32_increment_step; }
      }
      else if(p_lcd_var_number->ui8_size == 16)
      {
        if((*ui16_p_var) >= (p_lcd_var_number->ui32_min_value + p_lcd_var_number->ui32_increment_step)) { (*ui16_p_var) -= p_lcd_var_number->ui32_increment_step; }
      }
      else if(p_lcd_var_number->ui8_size == 32)
      {
        if((*ui32_p_var) >= (p_lcd_var_number->ui32_min_value + p_lcd_var_number->ui32_increment_step)) { (*ui32_p_var) -= p_lcd_var_number->ui32_increment_step; }
      }

      ui8_draw_var_value = 1;
    }

    p_menu_data->menu_buttons_events = 0;
  }

  // clear at every 1000ms
  if((p_menu_data->ui8_screen_set_values) &&
      (p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 1) &&
      (p_menu_data->ui8_edit_state))
  {
    ui32_x_position = DISPLAY_WIDTH - 16 - 1 - (p_lcd_var_number->ui8_number_digits * 12) - (p_lcd_var_number->ui8_number_digits * 1);
    if(p_lcd_var_number->ui8_decimal_digit) { ui32_x_position -= 6; } // offset value for the decimal point
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, DISPLAY_WIDTH - 16 - 1, ui32_y_position + 20, C_BLACK);

    ui8_draw_var_value_state = 0;
  }
  // draw value at every 1000ms
  else if((p_menu_data->ui8_screen_set_values) &&
      (p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 2) &&
      (p_menu_data->ui8_edit_state))
  {
    ui8_draw_var_value = 1;
  }
  // force draw value at every 1000ms
  else if((p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 2) &&
      !(ui8_draw_var_value_state))
  {
    ui8_draw_var_value = 1;
  }

  if(p_lcd_var_number->ui8_size == 8)
  {
    ui8_p_var = ((uint8_t *) p_lcd_var_number->p_var_number);
  }
  else if(p_lcd_var_number->ui8_size == 16)
  {
    ui16_p_var = ((uint16_t *) p_lcd_var_number->p_var_number);
  }
  else if(p_lcd_var_number->ui8_size == 32)
  {
    ui32_p_var = ((uint32_t *) p_lcd_var_number->p_var_number);
  }

  if(lcd_configurations_vars.ui8_item_number != lcd_configurations_vars.ui8_previous_item_number)
  {
    ui8_draw_var_value = 1;
  }

  if(ui8_draw_var_value)
  {
    if(p_lcd_var_number->ui8_size == 8)
    {
      ui32_value = (uint32_t) (*ui8_p_var);
    }
    else if(p_lcd_var_number->ui8_size == 16)
    {
      ui32_value = (uint32_t) (*ui16_p_var);
    }
    else if(p_lcd_var_number->ui8_size == 32)
    {
      ui32_value = (*ui32_p_var);
    }

    if(p_lcd_var_number->ui8_decimal_digit)
    {
      ui32_value_integer = ui32_value / 10;
      ui32_value_decimal = ui32_value % 10;
    }
    else
    {
      ui32_value_integer = ui32_value;
    }

    // find how many digits is the ui32_value_integer
    ui32_value_temp = ui32_value_integer;
    ui32_value_integer_number_digits = 0;
    for(ui8_counter = 0; ui8_counter < p_lcd_var_number->ui8_number_digits; ui8_counter++)
    {
      ui32_value_temp /= 10;
      ui32_value_integer_number_digits++;

      // finish for loop
      if (ui32_value_temp == 0)
      {
        if(p_lcd_var_number->ui8_decimal_digit) { ui32_value_integer_number_digits++; }

        break;
      }
    }

    ui32_x_position = DISPLAY_WIDTH - 16 - 1 - (ui32_value_integer_number_digits * 12) - (ui32_value_integer_number_digits * 1) - 10;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, ui32_x_position + (ui32_value_integer_number_digits * 12) + (ui32_value_integer_number_digits * 1), ui32_y_position + 20, C_BLACK);

    // draw variable value
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&REGULAR_TEXT_FONT);
    ui32_x_position = DISPLAY_WIDTH - 16 - 1 - (ui32_value_integer_number_digits * 12) - (ui32_value_integer_number_digits * 1);
    if(p_lcd_var_number->ui8_decimal_digit) { ui32_x_position -= 6; } // offset value for the decimal point
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);

    UG_PutString(ui32_x_position, ui32_y_position, itoa(ui32_value_integer));

    if(p_lcd_var_number->ui8_decimal_digit)
    {
      // draw small point
      ui32_x_position += 5 + ((ui32_value_integer_number_digits - 1) * 12) - ((ui32_value_integer_number_digits - 1) * 1);
      ui32_y_position += 14;
      UG_FillCircle(ui32_x_position, ui32_y_position, 1, C_WHITE);

      // draw decimal digit
      ui32_x_position += 4;
      ui32_y_position = ui16_conf_screen_first_item_y_offset +
          14 + // padding from top line
          (p_menu_data->ui8_visible_item * 50);
      UG_PutString(ui32_x_position, ui32_y_position, itoa(ui32_value_decimal));
    }

    ui8_draw_var_value_state = 1;
  }
}

void item_var_set_strings(struct_var_number *p_lcd_var_number, struct_menu_data *p_menu_data, uint8_t *p_strings)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  uint8_t ui8_draw_var_value = 0;
  static uint8_t ui8_draw_var_value_state = 0;
  uint8_t *p_strings_pointer;
  uint8_t *p_first_char;
  uint8_t ui8_number_of_chars;
  uint8_t *p_temp;
  uint8_t ui8_counter;
  uint8_t *ui8_p_value;

  ui8_p_value = ((uint8_t *) p_lcd_var_number->p_var_number);

  // if we are in edit mode...
  if(p_menu_data->ui8_screen_set_values &&
     p_menu_data->ui8_edit_state)
  {
    if(p_menu_data->menu_buttons_events == UP_CLICK)
    {
      if(*ui8_p_value <= (p_lcd_var_number->ui32_max_value - p_lcd_var_number->ui32_increment_step)) { *ui8_p_value += p_lcd_var_number->ui32_increment_step; }
      ui8_draw_var_value = 1;
    }

    if(p_menu_data->menu_buttons_events == DOWN_CLICK)
    {
      if(*ui8_p_value >= (p_lcd_var_number->ui32_min_value + p_lcd_var_number->ui32_increment_step)) { *ui8_p_value -= p_lcd_var_number->ui32_increment_step; }
      ui8_draw_var_value = 1;
    }

    p_menu_data->menu_buttons_events = 0;
  }

  // clear at every 1000ms
  if((p_menu_data->ui8_screen_set_values) &&
     (p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 1) &&
     (p_menu_data->ui8_edit_state))
  {
   ui32_x_position = DISPLAY_WIDTH - 16 - 1 - (7 * 12) - (7 * 1);
   ui32_y_position = ui16_conf_screen_first_item_y_offset +
       14 + // padding from top line
       (p_menu_data->ui8_visible_item * 50);
   UG_FillFrame(ui32_x_position, ui32_y_position, DISPLAY_WIDTH - 16 - 1, ui32_y_position + 20, C_BLACK);

   ui8_draw_var_value_state = 0;
  }
  // draw value at every 1000ms
  else if((p_menu_data->ui8_screen_set_values) &&
     (p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 2) &&
     (p_menu_data->ui8_edit_state))
  {
    ui8_draw_var_value = 1;
  }
  // force draw value at every 1000ms
  else if((p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 2) &&
     !(ui8_draw_var_value_state))
  {
    ui8_draw_var_value = 1;
  }

  if(lcd_configurations_vars.ui8_item_number != lcd_configurations_vars.ui8_previous_item_number)
  {
    ui8_draw_var_value = 1;
  }

  if(ui8_draw_var_value)
  {
    // find index of the first char of the string
    p_strings_pointer = p_strings;
    for(ui8_counter = 0; ui8_counter < *ui8_p_value; ui8_counter++)
    {
      while((*p_strings_pointer != '\n') && (*p_strings_pointer != 0))
      {
        p_strings_pointer++;
      }
      // account for '\n' and 0
      p_strings_pointer++;
    }
    p_first_char = p_strings_pointer;

    // find number of chars of the string
    p_temp = p_first_char;
    ui8_number_of_chars = 0;
    while((*p_temp != '\n') && (*p_temp != 0))
    {
      p_temp++;
      ui8_number_of_chars++;
    }

    ui32_x_position = DISPLAY_WIDTH - 16 - 1 - (ui8_number_of_chars * 12) - (ui8_number_of_chars * 1) - 12;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, ui32_x_position + (ui8_number_of_chars * 12) + (ui8_number_of_chars * 1), ui32_y_position + 20, C_BLACK);

    // draw string
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&REGULAR_TEXT_FONT);
    ui32_x_position = DISPLAY_WIDTH - 16 - 1 - (ui8_number_of_chars * 12) - (ui8_number_of_chars * 1);
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_PutString_with_length(ui32_x_position, ui32_y_position, p_first_char, ui8_number_of_chars);

    ui8_draw_var_value_state = 1;
  }
}

void draw_item_index_symbol(struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position ;
  uint32_t ui32_y_position;
  uint8_t ui8_counter;
  uint32_t ui32_line_lenght;
  uint16_t ui16_color;
  uint8_t ui8_visible_item;

  ui8_visible_item = lcd_configurations_vars.ui8_item_number - lcd_configurations_vars.ui8_item_visible_start_index;

  // clear at every 1000ms
  if((p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 1) &&
      (p_menu_data->ui8_edit_state) &&
      (!p_menu_data->ui8_screen_set_values))
  {
    ui16_color = C_BLACK;

    ui32_x_position = DISPLAY_WIDTH - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (ui8_visible_item * 50);
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
  // draw value at every 1000ms or when it changes
  else if((p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 2) &&
      (menu_data.ui8_edit_state))
  {
    ui16_color = C_RED;

    ui32_x_position = DISPLAY_WIDTH - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (ui8_visible_item * 50);
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
