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
#include "ugui_driver/ugui_bafang_850c.h"
#include "ugui/ugui.h"
#include "rtc.h"

#define MAX_ITEMS                 (sizeof(items_array_is_title) - 1)
#define MAX_ITEMS_PER_SCREEN      8
#define MAX_ITEMS_VISIBLE_INDEX   ((MAX_ITEMS + 1) - MAX_ITEMS_PER_SCREEN)

typedef struct _menu_data
{
  uint8_t ui8_edit_state;
  uint8_t ui8_visible_item;
  uint8_t ui8_visible_item_edit;
  uint8_t ui8_screen_set_values;
  uint8_t ui8_item_increment;
  buttons_events_t menu_buttons_events;
} struct_menu_data;

typedef struct _var_number
{
  volatile void *p_var_number;
  uint8_t ui8_size;
  uint8_t ui8_number_digits;
  uint8_t ui8_decimal_digit;
  uint32_t ui32_max_value;
  uint32_t ui32_min_value;
  uint32_t ui32_increment_step;
  uint8_t ui8_need_update;
} var_number_t;

volatile static l3_vars_t *p_l3_vars;

lcd_configurations_menu_t lcd_configurations_menu =
{
  .ui8_item_number = 1,
  .ui8_previous_item_number = 0xff,
  .ui8_item_visible_start_index = 0,
  .ui8_item_visible_index = 1,
  .ui8_refresh_full_menu_1 = 0,
  .ui8_refresh_full_menu_2 = 0,
  .ui8_battery_soc_power_used_state = 0,
};

static struct_menu_data menu_data =
{
  .ui8_edit_state = 0,
  .ui8_visible_item = 0,
  .ui8_visible_item_edit = 0,
  .menu_buttons_events = 0,
  .ui8_screen_set_values = 0,
  .ui8_item_increment = 1,
};

lcd_vars_t *p_lcd_vars;

static uint16_t ui16_conf_screen_first_item_y_offset = 61;

void draw_configurations_screen_mask(void);
void clear_configurations_screen_items(void);
void draw_item_cursor(struct_menu_data *p_menu_data);
void configurations_screen_item_title_set_strings(uint8_t *ui8_p_string, struct_menu_data *p_menu_data);
void item_set_strings(uint8_t *ui8_p_string1, uint8_t *ui8_p_string2, struct_menu_data *p_menu_data);
void item_var_set_number(var_number_t *p_lcd_var_number, struct_menu_data *p_menu_data);
void item_var_set_strings(var_number_t *p_lcd_var_number, struct_menu_data *p_menu_data, uint8_t *p_strings);
void item_visible_manage(struct_menu_data *p_menu_data);
void wheel_speed_title(struct_menu_data *p_menu_data);
void wheel_max_speed(struct_menu_data *p_menu_data);
void wheel_perimeter(struct_menu_data *p_menu_data);
void wheel_speed_units(struct_menu_data *p_menu_data);
void battery_title(struct_menu_data *p_menu_data);
void battery_max_current(struct_menu_data *p_menu_data);
void battery_current_ramp(struct_menu_data *p_menu_data);
void battery_low_cut_off_voltage(struct_menu_data *p_menu_data);
void battery_number_cells(struct_menu_data *p_menu_data);
void battery_resistance(struct_menu_data *p_menu_data);
void battery_voltage(struct_menu_data *p_menu_data);
void battery_soc_title(struct_menu_data *p_menu_data);
void battery_soc_enable(struct_menu_data *p_menu_data);
void battery_soc_increment_decrement(struct_menu_data *p_menu_data);
void battery_soc_voltage_to_reset(struct_menu_data *p_menu_data);
void battery_soc_total_watt_hour(struct_menu_data *p_menu_data);
void battery_soc_power_used(struct_menu_data *p_menu_data);
void assist_level_title(struct_menu_data *p_menu_data);
void assist_level_number(struct_menu_data *p_menu_data);
void assist_level_number_1(struct_menu_data *p_menu_data);
void assist_level_number_2(struct_menu_data *p_menu_data);
void assist_level_number_3(struct_menu_data *p_menu_data);
void assist_level_number_4(struct_menu_data *p_menu_data);
void assist_level_number_5(struct_menu_data *p_menu_data);
void assist_level_number_6(struct_menu_data *p_menu_data);
void assist_level_number_7(struct_menu_data *p_menu_data);
void assist_level_number_8(struct_menu_data *p_menu_data);
void assist_level_number_9(struct_menu_data *p_menu_data);
void walk_assist_level_title(struct_menu_data *p_menu_data);
void walk_assist_level_enable(struct_menu_data *p_menu_data);
void walk_assist_level_number_1(struct_menu_data *p_menu_data);
void walk_assist_level_number_2(struct_menu_data *p_menu_data);
void walk_assist_level_number_3(struct_menu_data *p_menu_data);
void walk_assist_level_number_4(struct_menu_data *p_menu_data);
void walk_assist_level_number_5(struct_menu_data *p_menu_data);
void walk_assist_level_number_6(struct_menu_data *p_menu_data);
void walk_assist_level_number_7(struct_menu_data *p_menu_data);
void walk_assist_level_number_8(struct_menu_data *p_menu_data);
void walk_assist_level_number_9(struct_menu_data *p_menu_data);
void startup_power_boost_title(struct_menu_data *p_menu_data);
void startup_power_boost_enable(struct_menu_data *p_menu_data);
void startup_power_boost_startup_everytime(struct_menu_data *p_menu_data);
void startup_power_boost_limit_max_power(struct_menu_data *p_menu_data);
void startup_power_boost_duration(struct_menu_data *p_menu_data);
void startup_power_boost_fadeout_duration(struct_menu_data *p_menu_data);
void startup_power_boost_factor_1(struct_menu_data *p_menu_data);
void startup_power_boost_factor_2(struct_menu_data *p_menu_data);
void startup_power_boost_factor_3(struct_menu_data *p_menu_data);
void startup_power_boost_factor_4(struct_menu_data *p_menu_data);
void startup_power_boost_factor_5(struct_menu_data *p_menu_data);
void startup_power_boost_factor_6(struct_menu_data *p_menu_data);
void startup_power_boost_factor_7(struct_menu_data *p_menu_data);
void startup_power_boost_factor_8(struct_menu_data *p_menu_data);
void startup_power_boost_factor_9(struct_menu_data *p_menu_data);
void motor_temperature_title(struct_menu_data *p_menu_data);
void motor_temperature_enable(struct_menu_data *p_menu_data);
void motor_temperature_min_limit(struct_menu_data *p_menu_data);
void motor_temperature_max_limit(struct_menu_data *p_menu_data);
void display_title(struct_menu_data *p_menu_data);
void display_time_hours(struct_menu_data *p_menu_data);
void display_time_minutes(struct_menu_data *p_menu_data);
void display_brightness_backlight_off(struct_menu_data *p_menu_data);
void display_brightness_backlight_on(struct_menu_data *p_menu_data);
void display_auto_power_off(struct_menu_data *p_menu_data);
void display_reset_to_defaults(struct_menu_data *p_menu_data);
void offroad_title(struct_menu_data *p_menu_data);
void offroad_active_on_startup(struct_menu_data *p_menu_data);
void offroad_speed_limit(struct_menu_data *p_menu_data);
void offroad_limit_power(struct_menu_data *p_menu_data);
void offroad_power_limit(struct_menu_data *p_menu_data);
void various_title(struct_menu_data *p_menu_data);
void various_motor_voltage(struct_menu_data *p_menu_data);
void various_motor_assistance_startup_without_pedaling(struct_menu_data *p_menu_data);
void technical_data_title(struct_menu_data *p_menu_data);
void technical_data_adc_throttle(struct_menu_data *p_menu_data);
void technical_data_throttle(struct_menu_data *p_menu_data);
void technical_data_adc_torque_sensor(struct_menu_data *p_menu_data);
void technical_data_torque_sensor(struct_menu_data *p_menu_data);
void technical_data_pedal_cadence(struct_menu_data *p_menu_data);
void technical_data_pedal_human_power(struct_menu_data *p_menu_data);
void technical_data_pwm_duty_cycle(struct_menu_data *p_menu_data);
void technical_data_motor_speed_erps(struct_menu_data *p_menu_data);
void technical_data_foc_angle(struct_menu_data *p_menu_data);

// call each function on the array
void (*p_items_array[])(struct_menu_data *p_menu_data) =
{
  wheel_speed_title,
  wheel_max_speed,
  wheel_perimeter,
  wheel_speed_units,
  battery_title,
  battery_max_current,
  battery_current_ramp,
  battery_low_cut_off_voltage,
  battery_number_cells,
  battery_resistance,
  battery_voltage,
  battery_soc_title,
  battery_soc_enable,
  battery_soc_increment_decrement,
  battery_soc_voltage_to_reset,
  battery_soc_total_watt_hour,
  battery_soc_power_used,
  assist_level_title,
  assist_level_number,
  assist_level_number_1,
  assist_level_number_2,
  assist_level_number_3,
  assist_level_number_4,
  assist_level_number_5,
  assist_level_number_6,
  assist_level_number_7,
  assist_level_number_8,
  assist_level_number_9,
  walk_assist_level_title,
  walk_assist_level_enable,
  walk_assist_level_number_1,
  walk_assist_level_number_2,
  walk_assist_level_number_3,
  walk_assist_level_number_4,
  walk_assist_level_number_5,
  walk_assist_level_number_6,
  walk_assist_level_number_7,
  walk_assist_level_number_8,
  walk_assist_level_number_9,
  startup_power_boost_title,
  startup_power_boost_enable,
  startup_power_boost_startup_everytime,
  startup_power_boost_limit_max_power,
  startup_power_boost_duration,
  startup_power_boost_fadeout_duration,
  startup_power_boost_factor_1,
  startup_power_boost_factor_2,
  startup_power_boost_factor_3,
  startup_power_boost_factor_4,
  startup_power_boost_factor_5,
  startup_power_boost_factor_6,
  startup_power_boost_factor_7,
  startup_power_boost_factor_8,
  startup_power_boost_factor_9,
  motor_temperature_title,
  motor_temperature_enable,
  motor_temperature_min_limit,
  motor_temperature_max_limit,
  display_title,
  display_time_hours,
  display_time_minutes,
  display_brightness_backlight_off,
  display_brightness_backlight_on,
  display_auto_power_off,
  display_reset_to_defaults,
  offroad_title,
  offroad_active_on_startup,
  offroad_speed_limit,
  offroad_limit_power,
  offroad_power_limit,
  various_title,
  various_motor_voltage,
  various_motor_assistance_startup_without_pedaling,
  technical_data_title,
  technical_data_adc_throttle,
  technical_data_throttle,
  technical_data_adc_torque_sensor,
  technical_data_torque_sensor,
  technical_data_pedal_cadence,
  technical_data_pedal_human_power,
  technical_data_pwm_duty_cycle,
  technical_data_motor_speed_erps,
  technical_data_foc_angle
};

uint8_t items_array_is_title[] =
{
  0, // exception: first must always be a title but let's signal it is not
  0,
  0,
  0,
  1, // battery_title
  0,
  0,
  0,
  0,
  0,
  0,
  1, // battery_soc_title
  0,
  0,
  0,
  0,
  0,
  1, // assist_level_title
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1, // walk assist_level_title
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1, // startup_power_boost_title
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  1,
  0,
  0,
  0,
  1,
  0,
  0,
  0,
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
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

void lcd_configurations_screen_init(void)
{
  p_lcd_vars = get_lcd_vars();
  p_l3_vars = get_l3_vars();
}

void lcd_configurations_screen(void)
{
  static uint8_t ui8_first_time = 1;
  uint8_t ui8_i;

  // leave config menu with a button_onoff_long_click
  if(buttons_get_onoff_long_click_event())
  {
    buttons_clear_all_events();

    p_lcd_vars->ui32_main_screen_draw_static_info = 1;
    p_lcd_vars->lcd_screen_state = LCD_SCREEN_MAIN;
    ui8_first_time = 1;
    return;
  }

  // enter/leave screen set values
  if(buttons_get_onoff_click_event())
  {
    buttons_clear_all_events();
    menu_data.menu_buttons_events = 0;

    if(menu_data.ui8_screen_set_values) { menu_data.ui8_screen_set_values = 0; }
    else { menu_data.ui8_screen_set_values = 1; }
  }

  // now get buttons events
  //
  if(buttons_get_down_click_event() ||
      buttons_get_down_long_click_event() ||
      buttons_get_down_click_long_click_event())
  {
    if(buttons_get_down_click_event()) { menu_data.menu_buttons_events = DOWN_CLICK; }
    else if(buttons_get_down_long_click_event()) { menu_data.menu_buttons_events = DOWN_LONG_CLICK; }

    buttons_clear_all_events();

    menu_data.ui8_item_increment = 1;
    item_visible_manage(&menu_data);

    // see if next item is a tittle
    if(items_array_is_title[lcd_configurations_menu.ui8_item_number] == 1)
    {
      menu_data.ui8_item_increment = 1;
      item_visible_manage(&menu_data);
    }
  }

  if(buttons_get_up_click_event () ||
      buttons_get_up_long_click_event() ||
      buttons_get_up_click_long_click_event())
  {
    if(buttons_get_up_click_event()) { menu_data.menu_buttons_events = UP_CLICK; }
    else if(buttons_get_up_long_click_event()) { menu_data.menu_buttons_events = UP_LONG_CLICK; }

    buttons_clear_all_events();

    menu_data.ui8_item_increment = 0;
    item_visible_manage(&menu_data);

    // see if next item is a tittle
    if(items_array_is_title[lcd_configurations_menu.ui8_item_number] == 1)
    {
      menu_data.ui8_item_increment = 0;
      item_visible_manage(&menu_data);
    }
  }

  // to draw static info
  if(lcd_configurations_menu.ui8_refresh_full_menu_1 == 1 ||
      lcd_configurations_menu.ui8_refresh_full_menu_2 == 1 )
  {
    UG_FillScreen(C_BLACK);
    draw_configurations_screen_mask();
    clear_configurations_screen_items();
    menu_data.ui8_screen_set_values = 0;
    menu_data.ui8_edit_state = 0;

    // let's start at ui8_item_number higher than 0 at first time
    lcd_configurations_menu.ui8_previous_item_number = 0xff;
    if(lcd_configurations_menu.ui8_item_number == 0)
    {
      lcd_configurations_menu.ui8_item_number = 0;
      lcd_configurations_menu.ui8_item_visible_start_index = 0;
      lcd_configurations_menu.ui8_item_visible_index = 0;
      menu_data.ui8_item_increment = 1;
      item_visible_manage(&menu_data);
    }

    // for the ase we want to reset to defaults
    if(lcd_configurations_menu.ui8_refresh_full_menu_2 == 1)
    {
      lcd_configurations_menu.ui8_refresh_full_menu_2 = 0;
      lcd_configurations_menu.ui8_refresh_full_menu_1 = 1;
    }
  }

  for(ui8_i = 0; ui8_i < MAX_ITEMS_PER_SCREEN; ui8_i++)
  {
    // find which item we are pointing to/editing
    if(lcd_configurations_menu.ui8_item_visible_index == ui8_i)
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
    (*p_items_array[lcd_configurations_menu.ui8_item_visible_start_index + ui8_i])(&menu_data);

    draw_item_cursor(&menu_data);
  }

  // track state for item number change
  if(lcd_configurations_menu.ui8_item_number != lcd_configurations_menu.ui8_previous_item_number)
  {
    lcd_configurations_menu.ui8_previous_item_number = lcd_configurations_menu.ui8_item_number;
  }

  lcd_configurations_menu.ui8_refresh_full_menu_1 = 0;
}

void item_visible_manage(struct_menu_data *p_menu_data)
{
  if(p_menu_data->ui8_item_increment)
  {
    // execute next code only if we are not setting the values of variables
    if(!p_menu_data->ui8_screen_set_values)
    {
      // increment to next item
      if(lcd_configurations_menu.ui8_item_number < MAX_ITEMS)
      {
        lcd_configurations_menu.ui8_item_number++;
      }

      // increment to next visible item
      if(lcd_configurations_menu.ui8_item_visible_index < (MAX_ITEMS_PER_SCREEN - 1))
      {
        lcd_configurations_menu.ui8_item_visible_index++;
      }
      // visible item limit, so increment the start index of visible item
      else
      {
        // do not increment more over the last item
        if(lcd_configurations_menu.ui8_item_visible_start_index < MAX_ITEMS_VISIBLE_INDEX)
        {
          lcd_configurations_menu.ui8_item_visible_start_index++;
          lcd_configurations_menu.ui8_refresh_full_menu_1 = 1;
        }
      }
    }
  }
  else
  {
    if(!menu_data.ui8_screen_set_values)
    {
      // decrement to next item
      if(lcd_configurations_menu.ui8_item_number > 0)
      {
        lcd_configurations_menu.ui8_item_number--;
      }

      // decrement to next visible item
      if(lcd_configurations_menu.ui8_item_visible_index > 1)
      {
        lcd_configurations_menu.ui8_item_visible_index--;
      }
      // visible item limit, so decrement the start index of visible item
      else
      {
        // do not decrement more over the last item
        if(lcd_configurations_menu.ui8_item_visible_start_index > 0)
        {
          lcd_configurations_menu.ui8_item_visible_start_index--;
          lcd_configurations_menu.ui8_refresh_full_menu_1 = 1;
        }
      }
    }
  }

  // exception: force first item to never stay on title
  if(lcd_configurations_menu.ui8_item_visible_start_index == 0 &&
      lcd_configurations_menu.ui8_item_number == 0)
  {
    lcd_configurations_menu.ui8_item_number = 1;
    lcd_configurations_menu.ui8_refresh_full_menu_1 = 0;
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
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_wheel_max_speed,
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
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui16_wheel_perimeter,
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
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_units_type,
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
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_battery_max_current,
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

void battery_current_ramp(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_ramp_up_amps_per_second_x10,
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 4,
    .ui32_increment_step = 1
  };

  item_set_strings("Current ramp", "(amps)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void battery_low_cut_off_voltage(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui16_battery_low_voltage_cut_off_x10,
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
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_battery_cells_number,
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
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui16_battery_pack_resistance_x1000,
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

void battery_voltage(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui16_battery_voltage_soc_x10,
    .ui8_size = 16,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 999,
    .ui32_min_value = 0,
    .ui32_increment_step = 0 // 0 so user can't change the value
  };

  item_set_strings("Voltage(read only)", "Considering resistance", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void battery_soc_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Battery SOC", p_menu_data);
}

void battery_soc_enable(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_battery_soc_enable,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Feature", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "disable\nenable");
}

void battery_soc_increment_decrement(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_battery_soc_increment_decrement,
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
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui16_battery_voltage_reset_wh_counter_x10,
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

  ui32_value = p_l3_vars->ui32_wh_x10_100_percent / 10;
  var_number_t lcd_var_number =
  {
    .p_var_number = &ui32_value,
    .ui8_size = 32,
    .ui8_number_digits = 4,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 9990,
    .ui32_min_value = 0,
    .ui32_increment_step = 10
  };

  item_set_strings("Battery total", "watts/hour (watts)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
  p_l3_vars->ui32_wh_x10_100_percent = ui32_value * 10;
}

void battery_soc_power_used(struct_menu_data *p_menu_data)
{
  // run only once when start configurations
  if(lcd_configurations_menu.ui8_battery_soc_power_used_state)
  {
    lcd_configurations_menu.ui8_battery_soc_power_used_state = 0;

    // update offset with the total value
    p_l3_vars->ui32_wh_x10_offset = p_l3_vars->ui32_wh_x10;
  }
  // keep reseting this values, as it is not suposed to run the bicycle on configurations menu
  // we need to reset them because we are here to manage (increment/decrement ui32_wh_x10_offset)
  p_l3_vars->ui32_wh_sum_x5 = 0;
  p_l3_vars->ui32_wh_sum_counter = 0;
  p_l3_vars->ui32_wh_x10 = 0;

  // change offset value as it is the one that is saved on EEPROM, etc
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui32_wh_x10_offset,
    .ui8_size = 32,
    .ui8_number_digits = 5,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 99900,
    .ui32_min_value = 0,
    .ui32_increment_step = 100
  };

  item_set_strings("Watts/hour used", "(watts)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Assist level", p_menu_data);
}

void assist_level_number(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_number_of_assist_levels,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 9,
    .ui32_min_value = 1,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist levels", "number", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_number_1(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_assist_level_factor[0],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 1", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_number_2(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_assist_level_factor[1],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 2", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_number_3(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_assist_level_factor[2],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 3", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_number_4(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_assist_level_factor[3],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 4", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_number_5(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_assist_level_factor[4],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 5", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_number_6(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_assist_level_factor[5],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 6", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_number_7(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_assist_level_factor[6],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 7", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_number_8(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_assist_level_factor[7],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 8", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void assist_level_number_9(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_assist_level_factor[8],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 9", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void walk_assist_level_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Walk assist level", p_menu_data);
}

void walk_assist_level_enable(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_feature_enabled,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Feature", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "disable\nenable");
}

void walk_assist_level_number_1(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_level_factor[0],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Walk assist level 1", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void walk_assist_level_number_2(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_level_factor[1],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Walk assist level 2", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void walk_assist_level_number_3(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_level_factor[2],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Walk assist level 3", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void walk_assist_level_number_4(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_level_factor[3],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Walk assist level 4", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void walk_assist_level_number_5(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_level_factor[4],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Walk assist level 5", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void walk_assist_level_number_6(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_level_factor[5],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Walk assist level 6", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void walk_assist_level_number_7(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_level_factor[6],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Walk assist level 7", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void walk_assist_level_number_8(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_level_factor[7],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Walk assist level 8", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void walk_assist_level_number_9(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_walk_assist_level_factor[8],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Walk assist level 9", "", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Start. power boost", p_menu_data);
}

void startup_power_boost_enable(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_feature_enabled,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Feature", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "disable\nenable");
}

void startup_power_boost_startup_everytime(struct_menu_data *p_menu_data)
{
  uint8_t ui8_temp = (p_l3_vars->ui8_startup_motor_power_boost_state & 1);

  var_number_t lcd_var_number =
  {
    .p_var_number = &ui8_temp,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Active on", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "startup\nalways");

  if(ui8_temp) { p_l3_vars->ui8_startup_motor_power_boost_state |= 1; }
  else { p_l3_vars->ui8_startup_motor_power_boost_state &= ~1; }
}

void startup_power_boost_limit_max_power(struct_menu_data *p_menu_data)
{
  uint8_t ui8_temp = (p_l3_vars->ui8_startup_motor_power_boost_state & 2) >> 1;

  var_number_t lcd_var_number =
  {
    .p_var_number = &ui8_temp,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Limit to max", "power", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "no\nyes");

  if(ui8_temp) { p_l3_vars->ui8_startup_motor_power_boost_state |= 2; }
  else { p_l3_vars->ui8_startup_motor_power_boost_state &= ~2; }
}

void startup_power_boost_duration(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_time,
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Duration", "(seconds)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_fadeout_duration(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_fade_time,
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Fadeout duration", "(seconds)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_factor_1(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_factor[0],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 1", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_factor_2(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_factor[1],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 2", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_factor_3(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_factor[2],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 3", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_factor_4(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_factor[3],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 4", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_factor_5(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_factor[4],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 5", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_factor_6(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_factor[5],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 6", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_factor_7(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_factor[6],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 7", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_factor_8(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_factor[7],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 8", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void startup_power_boost_factor_9(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_startup_motor_power_boost_factor[8],
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 1,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Assist level 9", "factor", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void motor_temperature_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Motor temperature", p_menu_data);
}

void motor_temperature_enable(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_temperature_limit_feature_enabled,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Feature", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "disable\nenable");
}

void motor_temperature_min_limit(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_motor_temperature_min_value_to_limit,
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 125,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Min limit", "(celsius degrees)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void motor_temperature_max_limit(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_motor_temperature_max_value_to_limit,
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 125,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Max limit", "(celsius degrees)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void display_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Display", p_menu_data);
}

void display_time_hours(struct_menu_data *p_menu_data)
{
  struct_rtc_time_t *p_rtc_time;
  struct_rtc_time_t rtc_time_edited;
  struct_rtc_time_t *p_rtc_time_edited;
  p_rtc_time_edited = &rtc_time_edited;

  p_rtc_time = rtc_get_time();
  p_rtc_time_edited->ui8_hours = p_rtc_time->ui8_hours;

  var_number_t lcd_var_number =
  {
    .p_var_number = &p_rtc_time_edited->ui8_hours,
    .ui8_size = 8,
    .ui8_number_digits = 2,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 59,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Clock", "hours", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);

  // change time only when user did change on LCD
  if(p_rtc_time_edited->ui8_hours != p_rtc_time->ui8_hours)
  {
    p_rtc_time->ui8_hours = p_rtc_time_edited->ui8_hours;
    rtc_set_time(p_rtc_time);
  }
}

void display_time_minutes(struct_menu_data *p_menu_data)
{
  struct_rtc_time_t *p_rtc_time;
  struct_rtc_time_t rtc_time_edited;
  struct_rtc_time_t *p_rtc_time_edited;
  p_rtc_time_edited = &rtc_time_edited;

  p_rtc_time = rtc_get_time();
  p_rtc_time_edited->ui8_minutes = p_rtc_time->ui8_minutes;

  var_number_t lcd_var_number =
  {
    .p_var_number = &p_rtc_time_edited->ui8_minutes,
    .ui8_size = 8,
    .ui8_number_digits = 2,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 59,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Clock", "minutes", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);

  // change time only when user did change on LCD
  if(p_rtc_time_edited->ui8_minutes != p_rtc_time->ui8_minutes)
  {
    p_rtc_time->ui8_minutes = p_rtc_time_edited->ui8_minutes;
    rtc_set_time(p_rtc_time);
  }
}

void display_brightness_backlight_off(struct_menu_data *p_menu_data)
{
  uint8_t ui8_value = p_l3_vars->ui8_lcd_backlight_off_brightness * 5;

  var_number_t lcd_var_number =
  {
    .p_var_number = &ui8_value,
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 5
  };

  item_set_strings("Brightness", "backlight off state (%)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);

  p_l3_vars->ui8_lcd_backlight_off_brightness = ui8_value / 5;
}

void display_brightness_backlight_on(struct_menu_data *p_menu_data)
{
  uint8_t ui8_value = p_l3_vars->ui8_lcd_backlight_on_brightness * 5;

  var_number_t lcd_var_number =
  {
    .p_var_number = &ui8_value,
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 100,
    .ui32_min_value = 0,
    .ui32_increment_step = 5
  };

  item_set_strings("Brightness", "backlight on state (%)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);

  p_l3_vars->ui8_lcd_backlight_on_brightness = ui8_value / 5;
}

void display_auto_power_off(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_lcd_power_off_time_minutes,
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Auto power off", "time (minutes)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void display_reset_to_defaults(struct_menu_data *p_menu_data)
{
  static uint8_t ui8_reset_to_defaults_counter = 0;

  var_number_t lcd_var_number =
  {
    .p_var_number = &ui8_reset_to_defaults_counter,
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Reset to defaults", "inc. to 10 to reset", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);

  if (ui8_reset_to_defaults_counter > 9)
  {
    ui8_reset_to_defaults_counter = 0;

    eeprom_init_defaults();

    // this will force refresh of all LCD configuration fields
    lcd_configurations_menu.ui8_refresh_full_menu_2 = 1;
  }
}

void offroad_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Offroad mode", p_menu_data);
}

void offroad_enable(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_offroad_feature_enabled,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Feature", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "disable\nenable");
}

void offroad_active_on_startup(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_offroad_enabled_on_startup,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Active on startup", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "no\nyes");
}

void offroad_speed_limit(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_offroad_speed_limit,
    .ui8_size = 8,
    .ui8_number_digits = 2,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 99,
    .ui32_min_value = 1,
    .ui32_increment_step = 1
  };

  item_set_strings("Speed limit", "(km/h)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void offroad_limit_power(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_offroad_power_limit_enabled,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Limit power", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "no\nyes");
}

void offroad_power_limit(struct_menu_data *p_menu_data)
{
  uint16_t ui16_offroad_power_limit = ((uint16_t) p_l3_vars->ui8_offroad_power_limit_div25) * 25;

  var_number_t lcd_var_number =
  {
    .p_var_number = &ui16_offroad_power_limit,
    .ui8_size = 16,
    .ui8_number_digits = 4,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 2000,
    .ui32_min_value = 0,
    .ui32_increment_step = 25
  };

  item_set_strings("Power limit", "(watts)", p_menu_data);
  item_var_set_number(&lcd_var_number, p_menu_data);

  p_l3_vars->ui8_offroad_power_limit_div25 = ((uint8_t) ui16_offroad_power_limit / 25);
}

void various_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Various", p_menu_data);
}

void various_motor_voltage(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_motor_type,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 2,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Motor voltage", "", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "48V\n36V\nexper");
}

void various_motor_assistance_startup_without_pedaling(struct_menu_data *p_menu_data)
{
  var_number_t lcd_var_number =
  {
    .p_var_number = &p_l3_vars->ui8_motor_assistance_startup_without_pedal_rotation,
    .ui8_size = 8,
    .ui8_number_digits = 1,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 1,
    .ui32_min_value = 0,
    .ui32_increment_step = 1
  };

  item_set_strings("Motor assist", "start w/o pedaling", p_menu_data);
  item_var_set_strings(&lcd_var_number, p_menu_data, "disable\nenable");
}

void technical_data_title(struct_menu_data *p_menu_data)
{
  configurations_screen_item_title_set_strings("Technical data", p_menu_data);
}

void technical_data_adc_throttle(struct_menu_data *p_menu_data)
{
  static uint32_t ui32_var = 0;
  var_number_t lcd_var_number =
  {
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 0 // 0 so user can't change the value
  };
  lcd_var_number.p_var_number = &ui32_var;

  item_set_strings("ADC throttle", "(read only)", p_menu_data);

  // see if value is different from previous one and if it is, force draw
  if(p_l3_vars->ui8_adc_throttle != ui32_var)
  {
    lcd_var_number.ui8_need_update = 1;
  }
  else
  {
    lcd_var_number.ui8_need_update = 0;
  }

  // set the new value
  ui32_var = p_l3_vars->ui8_adc_throttle;
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void technical_data_throttle(struct_menu_data *p_menu_data)
{
  static uint32_t ui32_var = 0;
  var_number_t lcd_var_number =
  {
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 0 // 0 so user can't change the value
  };
  lcd_var_number.p_var_number = &ui32_var;

  item_set_strings("Throttle", "(read only)", p_menu_data);

  // see if value is different from previous one and if it is, force draw
  if(p_l3_vars->ui8_throttle != ui32_var)
  {
    lcd_var_number.ui8_need_update = 1;
  }
  else
  {
    lcd_var_number.ui8_need_update = 0;
  }

  // set the new value
  ui32_var = p_l3_vars->ui8_throttle;
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void technical_data_adc_torque_sensor(struct_menu_data *p_menu_data)
{
  static uint32_t ui32_var = 0;
  static var_number_t lcd_var_number =
  {
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 0, // 0 so user can't change the value
    .ui8_need_update = 1
  };
  lcd_var_number.p_var_number = &ui32_var;

  item_set_strings("ADC torque sensor", "(read only)", p_menu_data);

  // see if value is different from previous one and if it is, force draw
  if(p_l3_vars->ui8_adc_pedal_torque_sensor != ui32_var)
  {
    lcd_var_number.ui8_need_update = 1;
  }
  else
  {
    lcd_var_number.ui8_need_update = 0;
  }

  // set the new value
  ui32_var = p_l3_vars->ui8_adc_pedal_torque_sensor;
  // draw the value
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void technical_data_torque_sensor(struct_menu_data *p_menu_data)
{
  static uint32_t ui32_var = 0;
  var_number_t lcd_var_number =
  {
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 0 // 0 so user can't change the value
  };
  lcd_var_number.p_var_number = &ui32_var;

  item_set_strings("Torque sensor", "(read only)", p_menu_data);

  // see if value is different from previous one and if it is, force draw
  if(p_l3_vars->ui8_pedal_torque_sensor != ui32_var)
  {
    lcd_var_number.ui8_need_update = 1;
  }
  else
  {
    lcd_var_number.ui8_need_update = 0;
  }

  // set the new value
  ui32_var = p_l3_vars->ui8_pedal_torque_sensor;
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void technical_data_pedal_cadence(struct_menu_data *p_menu_data)
{
  static uint32_t ui32_var = 0;
  var_number_t lcd_var_number =
  {
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 0 // 0 so user can't change the value
  };
  lcd_var_number.p_var_number = &ui32_var;

  item_set_strings("Pedal cadence", "(RPM) (read only)", p_menu_data);

  // see if value is different from previous one and if it is, force draw
  if(p_l3_vars->ui8_pedal_cadence != ui32_var)
  {
    lcd_var_number.ui8_need_update = 1;
  }
  else
  {
    lcd_var_number.ui8_need_update = 0;
  }

  // set the new value
  ui32_var = p_l3_vars->ui8_pedal_cadence;
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void technical_data_pedal_human_power(struct_menu_data *p_menu_data)
{
  static uint32_t ui32_var = 0;
  var_number_t lcd_var_number =
  {
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 999,
    .ui32_min_value = 0,
    .ui32_increment_step = 0 // 0 so user can't change the value
  };
  lcd_var_number.p_var_number = &ui32_var;

  item_set_strings("Pedal human", "power (read only)", p_menu_data);

  // see if value is different from previous one and if it is, force draw
  if(p_l3_vars->ui16_pedal_power_x10 != ui32_var)
  {
    lcd_var_number.ui8_need_update = 1;
  }
  else
  {
    lcd_var_number.ui8_need_update = 0;
  }

  // set the new value
  ui32_var = p_l3_vars->ui16_pedal_power_x10 / 10;
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void technical_data_pwm_duty_cycle(struct_menu_data *p_menu_data)
{
  static uint32_t ui32_var = 0;
  var_number_t lcd_var_number =
  {
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 0 // 0 so user can't change the value
  };
  lcd_var_number.p_var_number = &ui32_var;

  item_set_strings("PWM duty-cycle", "0 - 255 (read only)", p_menu_data);

  // see if value is different from previous one and if it is, force draw
  if(p_l3_vars->ui8_duty_cycle != ui32_var)
  {
    lcd_var_number.ui8_need_update = 1;
  }
  else
  {
    lcd_var_number.ui8_need_update = 0;
  }

  // set the new value
  ui32_var = p_l3_vars->ui8_duty_cycle;
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void technical_data_motor_speed_erps(struct_menu_data *p_menu_data)
{
  static uint32_t ui32_var = 0;
  var_number_t lcd_var_number =
  {
    .ui8_size = 16,
    .ui8_number_digits = 4,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 9999,
    .ui32_min_value = 0,
    .ui32_increment_step = 0 // 0 so user can't change the value
  };
  lcd_var_number.p_var_number = &ui32_var;

  item_set_strings("Motor speed", "(ERPs) (read only)", p_menu_data);

  // see if value is different from previous one and if it is, force draw
  if(p_l3_vars->ui16_motor_speed_erps != ui32_var)
  {
    lcd_var_number.ui8_need_update = 1;
  }
  else
  {
    lcd_var_number.ui8_need_update = 0;
  }

  // set the new value
  ui32_var = p_l3_vars->ui16_motor_speed_erps;
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void technical_data_foc_angle(struct_menu_data *p_menu_data)
{
  static uint32_t ui32_var = 0;
  var_number_t lcd_var_number =
  {
    .ui8_size = 8,
    .ui8_number_digits = 3,
    .ui8_decimal_digit = 0,
    .ui32_max_value = 255,
    .ui32_min_value = 0,
    .ui32_increment_step = 0 // 0 so user can't change the value
  };
  lcd_var_number.p_var_number = &ui32_var;

  item_set_strings("Motor FOC angle", "(multiply by 1.4)", p_menu_data);

  // see if value is different from previous one and if it is, force draw
  if(p_l3_vars->ui8_foc_angle != ui32_var)
  {
    lcd_var_number.ui8_need_update = 1;
  }
  else
  {
    lcd_var_number.ui8_need_update = 0;
  }

  // set the new value
  ui32_var = p_l3_vars->ui8_foc_angle;
  item_var_set_number(&lcd_var_number, p_menu_data);
}

void configurations_screen_item_title_set_strings(uint8_t *ui8_p_string, struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;

  // update only when item number changes
  if(lcd_configurations_menu.ui8_refresh_full_menu_1)
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
  if(lcd_configurations_menu.ui8_refresh_full_menu_1)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&CONFIGURATIONS_TEXT_FONT);
    ui32_x_position = 6;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        4 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    if (*ui8_p_string2 == 0) { ui32_y_position += 10; }
    UG_PutString(ui32_x_position, ui32_y_position, ui8_p_string1);

    UG_FontSelect(&SMALL_TEXT_FONT);
    ui32_y_position += 23;
    UG_PutString(ui32_x_position, ui32_y_position, ui8_p_string2);
  }
}

//TODO BUG can't show numbers higher than 99
void item_var_set_number(var_number_t *p_lcd_var_number, struct_menu_data *p_menu_data)
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
  static uint8_t ui8_long_click_started = 0;
  static uint8_t ui8_long_click_counter = 0;
  static uint8_t ui8_long_click_trigger = 0;

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

  // if LONG CLICK, keep track of long click so variable is increased automatically 10x every second
  //
  if((p_menu_data->menu_buttons_events == UP_LONG_CLICK) ||
      (p_menu_data->menu_buttons_events == DOWN_LONG_CLICK))
  {
    ui8_long_click_started = 1;
    p_menu_data->menu_buttons_events = 0;
  }

  // trigger at ever 100ms if UP/DOWN LONG CLICK
  if((ui8_long_click_started == 1) &&
      (buttons_get_up_state() ||
          buttons_get_down_state()))
  {
    ui8_long_click_counter++;

    if(ui8_long_click_counter >= 10)
    {
      ui8_long_click_counter = 0;
      ui8_long_click_trigger = 1;
    }
  }
  else
  {
    ui8_long_click_started = 0;
    ui8_long_click_counter = 0;
  }

  // if we are in edit mode...
  if(p_menu_data->ui8_screen_set_values &&
      p_menu_data->ui8_edit_state)
  {
    if((p_menu_data->menu_buttons_events == UP_CLICK) ||
        (buttons_get_up_state() &&
            ui8_long_click_trigger))
    {
      ui8_long_click_trigger = 0;

      if(p_lcd_var_number->ui8_size == 8)
      {
        if((*ui8_p_var) <= (p_lcd_var_number->ui32_max_value - p_lcd_var_number->ui32_increment_step)) { (*ui8_p_var) += p_lcd_var_number->ui32_increment_step; }
        else { (*ui8_p_var) = (uint8_t) p_lcd_var_number->ui32_max_value; }
      }
      else if(p_lcd_var_number->ui8_size == 16)
      {
        if((*ui16_p_var) <= (p_lcd_var_number->ui32_max_value - p_lcd_var_number->ui32_increment_step)) { (*ui16_p_var) += p_lcd_var_number->ui32_increment_step; }
        else { (*ui16_p_var) = (uint16_t) p_lcd_var_number->ui32_max_value; }
      }
      else if(p_lcd_var_number->ui8_size == 32)
      {
        if((*ui32_p_var) <= (p_lcd_var_number->ui32_max_value - p_lcd_var_number->ui32_increment_step)) { (*ui32_p_var) += p_lcd_var_number->ui32_increment_step; }
        else { (*ui32_p_var) = p_lcd_var_number->ui32_max_value; }
      }

      ui8_draw_var_value = 1;
    }

    if((p_menu_data->menu_buttons_events == DOWN_CLICK) ||
        (buttons_get_down_state() &&
                 ui8_long_click_trigger))
    {
      ui8_long_click_trigger = 0;

      if(p_lcd_var_number->ui8_size == 8)
      {
        if((*ui8_p_var) >= (p_lcd_var_number->ui32_min_value + p_lcd_var_number->ui32_increment_step)) { (*ui8_p_var) -= p_lcd_var_number->ui32_increment_step; }
        else { (*ui8_p_var) = (uint8_t) p_lcd_var_number->ui32_min_value; }
      }
      else if(p_lcd_var_number->ui8_size == 16)
      {
        if((*ui16_p_var) >= (p_lcd_var_number->ui32_min_value + p_lcd_var_number->ui32_increment_step)) { (*ui16_p_var) -= p_lcd_var_number->ui32_increment_step; }
        else { (*ui16_p_var) = (uint16_t) p_lcd_var_number->ui32_min_value; }
      }
      else if(p_lcd_var_number->ui8_size == 32)
      {
        if((*ui32_p_var) >= (p_lcd_var_number->ui32_min_value + p_lcd_var_number->ui32_increment_step)) { (*ui32_p_var) -= p_lcd_var_number->ui32_increment_step; }
        else { (*ui32_p_var) = p_lcd_var_number->ui32_min_value; }
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

  if((lcd_configurations_menu.ui8_item_number != lcd_configurations_menu.ui8_previous_item_number) ||
     (lcd_configurations_menu.ui8_refresh_full_menu_1) ||
     (p_lcd_var_number->ui8_need_update))
  {
    ui8_draw_var_value = 1;
  }

  if(lcd_configurations_menu.ui8_refresh_full_menu_1)
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
    UG_FontSelect(&CONFIGURATIONS_TEXT_FONT);
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

void item_var_set_strings(var_number_t *p_lcd_var_number, struct_menu_data *p_menu_data, uint8_t *p_strings)
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

//  // force min and max values
//  if(((uint8_t *) p_lcd_var_number->p_var_number) > ((uint8_t *) p_lcd_var_number->ui32_max_value))
//  {
//    p_lcd_var_number->p_var_number = ((uint8_t *) p_lcd_var_number->ui32_max_value);
//  }
//  else if (((uint8_t *) p_lcd_var_number->p_var_number) < ((uint8_t *) p_lcd_var_number->ui32_min_value))
//  {
//    p_lcd_var_number->p_var_number = ((uint8_t *) p_lcd_var_number->ui32_min_value);
//  }

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

  if(lcd_configurations_menu.ui8_item_number != lcd_configurations_menu.ui8_previous_item_number)
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
    UG_FontSelect(&CONFIGURATIONS_TEXT_FONT);
    ui32_x_position = DISPLAY_WIDTH - 16 - 1 - (ui8_number_of_chars * 12) - (ui8_number_of_chars * 1);
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        14 + // padding from top line
        (p_menu_data->ui8_visible_item * 50);
    UG_PutString_with_length(ui32_x_position, ui32_y_position, p_first_char, ui8_number_of_chars);

    ui8_draw_var_value_state = 1;
  }
}

void draw_item_cursor(struct_menu_data *p_menu_data)
{
  uint32_t ui32_x_position ;
  uint32_t ui32_y_position;
  uint8_t ui8_counter;
  uint32_t ui32_line_lenght;
  uint8_t ui8_cursor_index;
  static uint8_t ui8_cursor_index_previous = 1;

  ui8_cursor_index = lcd_configurations_menu.ui8_item_number - lcd_configurations_menu.ui8_item_visible_start_index;

  // cursor changed, so delete from the current location before draw on the new one
  if(ui8_cursor_index != ui8_cursor_index_previous)
  {
    ui32_x_position = DISPLAY_WIDTH - 1 - 10;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (ui8_cursor_index_previous * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, DISPLAY_WIDTH - 1, ui32_y_position + 20, C_BLACK);

    ui8_cursor_index_previous = ui8_cursor_index;
  }

  // clear at every 1000ms
  if((p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 1) &&
      (p_menu_data->ui8_edit_state) &&
      (!p_menu_data->ui8_screen_set_values))
  {
    ui32_x_position = DISPLAY_WIDTH - 1 - 10;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (ui8_cursor_index_previous * 50);
    UG_FillFrame(ui32_x_position, ui32_y_position, DISPLAY_WIDTH - 1, ui32_y_position + 20, C_BLACK);
  }
  // draw value at every 1000ms or when it changes
  else if((p_lcd_vars->ui8_lcd_menu_counter_1000ms_trigger == 2) &&
      (menu_data.ui8_edit_state))
  {
    ui32_x_position = DISPLAY_WIDTH - 1;
    ui32_y_position = ui16_conf_screen_first_item_y_offset +
        12 + // padding from top line
        (ui8_cursor_index * 50);
    ui32_line_lenght = 0;

    for(ui8_counter = 0; ui8_counter < 10; ui8_counter++)
    {
      UG_DrawLine(ui32_x_position, ui32_y_position, ui32_x_position + ui32_line_lenght, ui32_y_position, C_ORANGE_RED);
      ui32_x_position--;
      ui32_line_lenght++;
      ui32_y_position++;
    }

    for(ui8_counter = 0; ui8_counter < 10; ui8_counter++)
    {
      UG_DrawLine(ui32_x_position, ui32_y_position, ui32_x_position + ui32_line_lenght, ui32_y_position, C_ORANGE_RED);
      ui32_x_position++;
      ui32_line_lenght--;
      ui32_y_position++;
    }
  }
}

lcd_configurations_menu_t* get_lcd_configurations_menu(void)
{
  return &lcd_configurations_menu;
}
