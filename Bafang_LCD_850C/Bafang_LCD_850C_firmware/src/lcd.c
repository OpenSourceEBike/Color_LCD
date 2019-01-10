/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include <math.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stdio.h"
#include "main.h"
#include "config.h"
#include "utils.h"
#include "pins.h"
#include "lcd_configurations.h"
#include "lcd.h"
#include "buttons.h"
#include "eeprom.h"
#include "usart1.h"
#include "ugui_driver/ugui_bafang_500c.h"
#include "ugui/ugui.h"
#include "rtc.h"

static struct_motor_controller_data motor_controller_data;
static struct_configuration_variables configuration_variables;

struct_lcd_vars lcd_vars =
{
  .ui32_main_screen_draw_static_info = 1,
  .lcd_screen_state = LCD_SCREEN_MAIN,
  .ui8_lcd_menu_counter_1000ms_state = 0,
  .ui8_lcd_menu_max_power = 0,
};

static struct_lcd_configurations_vars *p_lcd_configurations_vars;

static uint16_t ui16_battery_voltage_filtered_x10;
static uint16_t ui16_battery_current_filtered_x5;

static uint16_t ui16_battery_power_filtered_x50;
static uint16_t ui16_battery_power_filtered;

static uint16_t ui16_pedal_torque_filtered;
static uint16_t ui16_pedal_power_filtered;

static uint8_t ui8_pedal_cadence_filtered;

static uint16_t ui16_battery_soc_watts_hour;

static uint8_t ui8_lcd_menu_counter_100ms_state = 0;

static uint8_t ui8_lcd_menu_config_submenu_state = 0;
static uint8_t ui8_lcd_menu_flash_counter = 0;
static uint16_t ui16_lcd_menu_flash_counter_temperature = 0;
static uint8_t ui8_lcd_menu_flash_state;
static uint8_t ui8_lcd_menu_flash_state_temperature;
static uint8_t ui8_lcd_menu_config_submenu_number = 0;
static uint8_t ui8_lcd_menu_config_submenu_active = 0;

void lcd_main_screen(void);
uint8_t first_time_management(void);
void assist_level_state(void);
void power_off_management(void);
void lcd_power_off(uint8_t updateDistanceOdo);
void low_pass_filter_battery_voltage_current_power(void);
void update_menu_flashing_state(void);
void calc_battery_soc_watts_hour(void);
void calc_odometer(void);
static void automatic_power_off_management(void);
void brake(void);
void wheel_speed(void);
void power(void);
void power_off_management(void);
void temperature(void);
void time(void);
void battery_soc(void);
void calc_battery_voltage_soc(void);
void low_pass_filter_pedal_torque_and_power(void);
static void low_pass_filter_pedal_cadence(void);
void lights_state(void);
void lcd_set_backlight_intensity(uint8_t ui8_intensity);
void battery_soc_bar_set(uint32_t ui32_bar_number, uint16_t ui16_color);
void battery_soc_bar_clear(uint32_t ui32_bar_number);
void draw_configurations_screen_mask(void);

/* Place your initialization/startup code here (e.g. MyInst_Start()) */
void lcd_init(void)
{
  bafang_500C_lcd_init();
  UG_FillScreen(C_BLACK);

  lcd_configurations_screen_init();
  p_lcd_configurations_vars = get_lcd_configurations_vars();

  // init variables with the stored value on EEPROM
  eeprom_init_variables ();
}

void lcd_clock(void)
{
//  if (first_time_management())
//    return;

  update_menu_flashing_state();

  calc_battery_soc_watts_hour();

  low_pass_filter_battery_voltage_current_power();
  if (lcd_vars.ui8_lcd_menu_counter_1000ms_state)
  {
    low_pass_filter_pedal_cadence();
  }

  if (ui8_lcd_menu_counter_100ms_state)
  {
    low_pass_filter_pedal_torque_and_power();
  }

  calc_battery_voltage_soc();
  calc_odometer();
  automatic_power_off_management();

  // enter menu configurations: UP + DOWN click event
  if (buttons_get_up_down_click_event () &&
      lcd_vars.lcd_screen_state == LCD_SCREEN_MAIN)
  {
    buttons_clear_up_down_click_event ();

    // reset needed variables of configurations screen
    p_lcd_configurations_vars->ui8_refresh_full_menu_1 = 1;

    // need to track start configuration
    p_lcd_configurations_vars->ui8_battery_soc_power_used_state = 1;

    lcd_vars.lcd_screen_state = LCD_SCREEN_CONFIGURATIONS;
  }

  // enter in menu set power: ONOFF + UP click event
  if (buttons_get_onoff_state() && buttons_get_up_state())
  {
    buttons_clear_all_events();
    lcd_vars.ui8_lcd_menu_max_power = 1;
  }

  switch (lcd_vars.lcd_screen_state)
  {
    case LCD_SCREEN_MAIN:
      lcd_main_screen();
    break;

    case LCD_SCREEN_CONFIGURATIONS:
      lcd_configurations_screen();
    break;
  }

  power_off_management();
}

void lcd_draw_main_menu_mask(void)
{
  UG_DrawLine(10, 60, 310, 60, C_DIM_GRAY);
  UG_DrawLine(10, 180, 310, 180, C_DIM_GRAY);
  UG_DrawLine(10, 265, 310, 265, C_DIM_GRAY);
}

void lcd_main_screen (void)
{
  // run once only, to draw static info
  if(lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_FillScreen(C_BLACK);
    lcd_draw_main_menu_mask();
  }

  temperature();
  time();
  assist_level_state();
  wheel_speed();
//  walk_assist_state();
//  offroad_mode();
  power();
  battery_soc();
  brake();
  lights_state();

  // clear this variable after 1 full cycle running
  lcd_vars.ui32_main_screen_draw_static_info = 0;
}

uint8_t first_time_management(void)
{
  static uint8_t ui8_motor_controller_init = 1;
  uint8_t ui8_status = 0;

  // don't update LCD up to we get first communication package from the motor controller
  if(ui8_motor_controller_init &&
      (usart1_received_first_package() == 0))
  {
    ui8_status = 1;
  }
  // this will be executed only 1 time at startup
  else if (ui8_motor_controller_init)
  {
    ui8_motor_controller_init = 0;

    // reset Wh value if battery voltage is over ui16_battery_voltage_reset_wh_counter_x10 (value configured by user)
    if (((uint32_t) motor_controller_data.ui16_adc_battery_voltage *
        ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000) > ((uint32_t) configuration_variables.ui16_battery_voltage_reset_wh_counter_x10 * 1000))
    {
      configuration_variables.ui32_wh_x10_offset = 0;
    }

    if (configuration_variables.ui8_offroad_feature_enabled &&
      configuration_variables.ui8_offroad_enabled_on_startup)
    {
      motor_controller_data.ui8_offroad_mode = 1;
    }
  }

  return ui8_status;
}

void assist_level_state(void)
{
  static print_number_t assist_level =
  {
    .font = &FONT_32X53,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 1,
    .ui8_left_zero_paddig = 0,
  };

  static uint8_t ui8_assist_level_previous = 0xff;

  if (lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_GRAY);
    UG_FontSelect(&FONT_10X16);
    UG_PutString(10, 188, "Assist");
  }

  if (buttons_get_up_click_event() &&
      lcd_vars.ui8_lcd_menu_max_power == 0)
  {
    buttons_clear_up_click_event ();
    buttons_clear_up_click_long_click_event ();
    buttons_clear_up_long_click_event ();
    buttons_clear_down_click_event ();
    buttons_clear_down_click_long_click_event ();
    buttons_clear_down_long_click_event ();

    configuration_variables.ui8_assist_level++;

    if (configuration_variables.ui8_assist_level > configuration_variables.ui8_number_of_assist_levels)
      { configuration_variables.ui8_assist_level = configuration_variables.ui8_number_of_assist_levels; }
  }

  if (buttons_get_down_click_event() &&
      lcd_vars.ui8_lcd_menu_max_power == 0)
  {
    buttons_clear_up_click_event ();
    buttons_clear_up_click_long_click_event ();
    buttons_clear_up_long_click_event ();
    buttons_clear_down_click_event ();
    buttons_clear_down_click_long_click_event ();
    buttons_clear_down_long_click_event ();

    if (configuration_variables.ui8_assist_level > 0)
      configuration_variables.ui8_assist_level--;
  }

  if ((configuration_variables.ui8_assist_level != ui8_assist_level_previous) ||
      lcd_vars.ui32_main_screen_draw_static_info)
  {
    ui8_assist_level_previous = configuration_variables.ui8_assist_level;

    assist_level.ui32_x_position = 25;
    assist_level.ui32_y_position = 210;
    assist_level.ui32_number = (uint32_t) configuration_variables.ui8_assist_level;
    lcd_print_number(&assist_level);
  }
}

struct_configuration_variables* get_configuration_variables(void)
{
  return &configuration_variables;
}

struct_motor_controller_data* get_motor_controller_data(void)
{
  return &motor_controller_data;
}

void power_off_management(void)
{
  if(buttons_get_onoff_long_click_event() &&
      lcd_vars.lcd_screen_state == LCD_SCREEN_MAIN)
  {
    lcd_power_off(1);
  }
}

void lcd_power_off(uint8_t updateDistanceOdo)
{
//  if (updateDistanceOdo)
//  {
    __disable_irq();
    configuration_variables.ui32_wh_x10_offset = motor_controller_data.ui32_wh_x10;
    __enable_irq();
    configuration_variables.ui32_odometer_x10 += ((uint32_t) configuration_variables.ui16_odometer_distance_x10);
//  }

  // save the variables on EEPROM
  eeprom_write_variables ();

  // put screen all black and disable backlight
  UG_FillScreen(0);
  lcd_set_backlight_intensity(0);

  // now disable the power to all the system
  system_power(0);

  // block here
  while(1) ;
}

void low_pass_filter_battery_voltage_current_power(void)
{
  static uint32_t ui32_battery_voltage_accumulated_x10000 = 0;
  static uint16_t ui16_battery_current_accumulated_x5 = 0;

  // low pass filter battery voltage
  ui32_battery_voltage_accumulated_x10000 -= ui32_battery_voltage_accumulated_x10000 >> BATTERY_VOLTAGE_FILTER_COEFFICIENT;
  ui32_battery_voltage_accumulated_x10000 += (uint32_t) motor_controller_data.ui16_adc_battery_voltage * ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000;
  ui16_battery_voltage_filtered_x10 = ((uint32_t) (ui32_battery_voltage_accumulated_x10000 >> BATTERY_VOLTAGE_FILTER_COEFFICIENT)) / 1000;

  // low pass filter batery current
  ui16_battery_current_accumulated_x5 -= ui16_battery_current_accumulated_x5 >> BATTERY_CURRENT_FILTER_COEFFICIENT;
  ui16_battery_current_accumulated_x5 += (uint16_t) motor_controller_data.ui8_battery_current_x5;
  ui16_battery_current_filtered_x5 = ui16_battery_current_accumulated_x5 >> BATTERY_CURRENT_FILTER_COEFFICIENT;

  // battery power
  __disable_irq();
  ui16_battery_power_filtered_x50 = ui16_battery_current_filtered_x5 * ui16_battery_voltage_filtered_x10;
  ui16_battery_power_filtered = ui16_battery_power_filtered_x50 / 50;

  // loose resolution under 200W
  if (ui16_battery_power_filtered < 200)
  {
    ui16_battery_power_filtered /= 10;
    ui16_battery_power_filtered *= 10;
  }
  // loose resolution under 400W
  else if (ui16_battery_power_filtered < 400)
  {
    ui16_battery_power_filtered /= 20;
    ui16_battery_power_filtered *= 20;
  }
  // loose resolution all other values
  else
  {
    ui16_battery_power_filtered /= 25;
    ui16_battery_power_filtered *= 25;
  }
  __enable_irq();
}

void low_pass_filter_pedal_torque_and_power(void)
{
  static uint32_t ui32_pedal_torque_accumulated = 0;
  static uint32_t ui32_pedal_power_accumulated = 0;

  // low pass filter
  ui32_pedal_torque_accumulated -= ui32_pedal_torque_accumulated >> PEDAL_TORQUE_FILTER_COEFFICIENT;
  ui32_pedal_torque_accumulated += (uint32_t) motor_controller_data.ui16_pedal_torque_x10 / 10;
  ui16_pedal_torque_filtered = ((uint32_t) (ui32_pedal_torque_accumulated >> PEDAL_TORQUE_FILTER_COEFFICIENT));

  // low pass filter
  ui32_pedal_power_accumulated -= ui32_pedal_power_accumulated >> PEDAL_POWER_FILTER_COEFFICIENT;
  ui32_pedal_power_accumulated += (uint32_t) motor_controller_data.ui16_pedal_power_x10 / 10;
  ui16_pedal_power_filtered = ((uint32_t) (ui32_pedal_power_accumulated >> PEDAL_POWER_FILTER_COEFFICIENT));

  if (ui16_pedal_torque_filtered > 200)
  {
    ui16_pedal_torque_filtered /= 20;
    ui16_pedal_torque_filtered *= 20;
  }
  else if (ui16_pedal_torque_filtered > 100)
  {
    ui16_pedal_torque_filtered /= 10;
    ui16_pedal_torque_filtered *= 10;
  }
  else
  {
    // do nothing to roginal values
  }

  if (ui16_pedal_power_filtered > 500)
  {
    ui16_pedal_power_filtered /= 25;
    ui16_pedal_power_filtered *= 25;
  }
  else if (ui16_pedal_power_filtered > 200)
  {
    ui16_pedal_power_filtered /= 20;
    ui16_pedal_power_filtered *= 20;
  }
  else if (ui16_pedal_power_filtered > 10)
  {
    ui16_pedal_power_filtered /= 10;
    ui16_pedal_power_filtered *= 10;
  }
  else
  {
    ui16_pedal_power_filtered = 0; // no point to show less than 10W
  }
}

static void low_pass_filter_pedal_cadence(void)
{
  static uint16_t ui16_pedal_cadence_accumulated = 0;

  // low pass filter
  ui16_pedal_cadence_accumulated -= (ui16_pedal_cadence_accumulated >> PEDAL_CADENCE_FILTER_COEFFICIENT);
  ui16_pedal_cadence_accumulated += (uint16_t) motor_controller_data.ui8_pedal_cadence;

  // consider the filtered value only for medium and high values of the unfiltered value
  if (motor_controller_data.ui8_pedal_cadence > 20)
  {
    ui8_pedal_cadence_filtered = (uint8_t) (ui16_pedal_cadence_accumulated >> PEDAL_CADENCE_FILTER_COEFFICIENT);
  }
  else
  {
    ui8_pedal_cadence_filtered = motor_controller_data.ui8_pedal_cadence;
  }
}

void calc_wh(void)
{
  static uint8_t ui8_1s_timmer_counter = 0;
  uint32_t ui32_temp = 0;

  if(ui16_battery_power_filtered_x50 > 0)
  {
    motor_controller_data.ui32_wh_sum_x5 += ui16_battery_power_filtered_x50 / 10;
    motor_controller_data.ui32_wh_sum_counter++;
  }

  // calc at 1s rate
  if(ui8_1s_timmer_counter < 10)
  {
    ui8_1s_timmer_counter = 0;

    // avoid  zero divisison
    if(motor_controller_data.ui32_wh_sum_counter != 0)
    {
      ui32_temp = motor_controller_data.ui32_wh_sum_counter / 36;
      ui32_temp = (ui32_temp * (motor_controller_data.ui32_wh_sum_x5 / motor_controller_data.ui32_wh_sum_counter)) / 500;
    }

    motor_controller_data.ui32_wh_x10 = configuration_variables.ui32_wh_x10_offset + ui32_temp;
  }
  ui8_1s_timmer_counter++;
}

void calc_odometer(void)
{
  uint32_t uint32_temp;
  static uint8_t ui8_1s_timmer_counter;

  // calc at 1s rate
  if (ui8_1s_timmer_counter++ >= 100)
  {
    ui8_1s_timmer_counter = 0;

    uint32_temp = (motor_controller_data.ui32_wheel_speed_sensor_tick_counter - motor_controller_data.ui32_wheel_speed_sensor_tick_counter_offset)
        * ((uint32_t) configuration_variables.ui16_wheel_perimeter);
    // avoid division by 0
    if (uint32_temp > 100000) { uint32_temp /= 100000;}  // milimmeters to 0.1kms
    else { uint32_temp = 0; }

    // now store the value on the global variable
    configuration_variables.ui16_odometer_distance_x10 = (uint16_t) uint32_temp;
  }
}

static void automatic_power_off_management(void)
{
  static uint8_t ui8_lcd_power_off_time_counter_minutes = 0;
  static uint16_t ui16_lcd_power_off_time_counter = 0;

  if(configuration_variables.ui8_lcd_power_off_time_minutes != 0)
  {
    // see if we should reset the automatic power off minutes counter
    if ((motor_controller_data.ui16_wheel_speed_x10 > 0) ||   // wheel speed > 0
        (motor_controller_data.ui8_battery_current_x5 > 0) || // battery current > 0
        (motor_controller_data.ui8_braking) ||                // braking
        buttons_get_events())                                 // any button active
    {
      ui16_lcd_power_off_time_counter = 0;
      ui8_lcd_power_off_time_counter_minutes = 0;
    }

    // increment the automatic power off minutes counter
    ui16_lcd_power_off_time_counter++;

    // check if we should power off the LCD
    if(ui16_lcd_power_off_time_counter >= (100 * 60)) // 1 minute passed
    {
      ui16_lcd_power_off_time_counter = 0;

      ui8_lcd_power_off_time_counter_minutes++;
      if(ui8_lcd_power_off_time_counter_minutes >= configuration_variables.ui8_lcd_power_off_time_minutes)
      {
        lcd_power_off(1);
      }
    }
  }
  else
  {
    ui16_lcd_power_off_time_counter = 0;
    ui8_lcd_power_off_time_counter_minutes = 0;
  }
}

void update_menu_flashing_state(void)
{
  static uint8_t ui8_lcd_menu_counter_100ms = 0;
  static uint8_t ui8_lcd_menu_counter_1000ms = 0;

  // ***************************************************************************************************
  // For flashing on menus, 0.5 seconds flash
  if (ui8_lcd_menu_flash_counter++ > 50)
  {
    ui8_lcd_menu_flash_counter = 0;

    if (ui8_lcd_menu_flash_state)
      ui8_lcd_menu_flash_state = 0;
    else
      ui8_lcd_menu_flash_state = 1;
  }
  // ***************************************************************************************************

  // ***************************************************************************************************
  ui8_lcd_menu_counter_100ms_state = 0;
  if (ui8_lcd_menu_counter_100ms++ > 10)
  {
    ui8_lcd_menu_counter_100ms = 0;
    ui8_lcd_menu_counter_100ms_state = 1;
  }

  // disable trigger signal
  if (lcd_vars.ui8_lcd_menu_counter_1000ms_trigger) { lcd_vars.ui8_lcd_menu_counter_1000ms_trigger = 0; }

  if(lcd_vars.ui8_lcd_menu_counter_1000ms_state)
  {
    if(ui8_lcd_menu_counter_1000ms++ > 80)
    {
      ui8_lcd_menu_counter_1000ms = 0;
      lcd_vars.ui8_lcd_menu_counter_1000ms_state = 0;
      lcd_vars.ui8_lcd_menu_counter_1000ms_trigger = 1;
    }
  }
  else
  {
    if(ui8_lcd_menu_counter_1000ms++ > 20)
    {
      ui8_lcd_menu_counter_1000ms = 0;
      lcd_vars.ui8_lcd_menu_counter_1000ms_state = 1;
      lcd_vars.ui8_lcd_menu_counter_1000ms_trigger = 2;
    }
  }
  // ***************************************************************************************************

  // ***************************************************************************************************
  // For flashing the temperature field when the current is being limited due to motor over temperature
  // flash only if current is being limited: ui8_temperature_current_limiting_value != 255
  if (motor_controller_data.ui8_temperature_current_limiting_value != 255)
  {
    if (ui8_lcd_menu_flash_state_temperature == 0) // state 0: disabled
    {
      if (ui16_lcd_menu_flash_counter_temperature > 0)
      {
        ui16_lcd_menu_flash_counter_temperature--;
      }

      if (ui16_lcd_menu_flash_counter_temperature == 0)
      {
        // if motor_controller_data.ui8_temperature_current_limiting_value == 0, flash quicker meaning motor is shutoff
        if (motor_controller_data.ui8_temperature_current_limiting_value > 0)
        {
          ui16_lcd_menu_flash_counter_temperature = 50 + ((uint16_t) motor_controller_data.ui8_temperature_current_limiting_value);
        }
        else
        {
          ui16_lcd_menu_flash_counter_temperature = 25;
        }

        ui8_lcd_menu_flash_state_temperature = 1;
      }
    }

    if (ui8_lcd_menu_flash_state_temperature == 1) // state 1: enabled
    {
      if (ui16_lcd_menu_flash_counter_temperature > 0)
      {
        ui16_lcd_menu_flash_counter_temperature--;
      }

      if (ui16_lcd_menu_flash_counter_temperature == 0)
      {
        ui16_lcd_menu_flash_counter_temperature = 25; // 0.25 second
        ui8_lcd_menu_flash_state_temperature = 0;
      }
    }
  }
  else
  {
    ui8_lcd_menu_flash_state_temperature = 1;
  }
  // ***************************************************************************************************
}

void brake(void)
{
  static uint8_t ui8_braking_previous;
  uint32_t ui32_x1;
  uint32_t ui32_y1;
  uint32_t ui32_x2;
  uint32_t ui32_y2;

  // if previous state was disable, draw
  if((motor_controller_data.ui8_braking != ui8_braking_previous) ||
      (lcd_vars.ui32_main_screen_draw_static_info))
  {
    ui8_braking_previous = motor_controller_data.ui8_braking;

    if(motor_controller_data.ui8_braking)
    {
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&SMALL_TEXT_FONT);
      ui32_x1 = 150;
      ui32_y1 = 14;
      UG_PutString(ui32_x1, ui32_y1, "brake");
    }
    else
    {
      // clear area
      // 5 leters
      ui32_x1 = 150;
      ui32_y1 = 14;
      ui32_x2 = ui32_x1 + ((5 * 10) + (5 * 1) + 1);
      ui32_y2 = ui32_y1 + 18;
      UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);
    }
  }
}


void lcd_set_backlight_intensity(uint8_t ui8_intensity)
{
  if (ui8_intensity == 0)
  {
    TIM_CtrlPWMOutputs(TIM3, DISABLE);
  }
  else if (ui8_intensity <= 20)
  {
    TIM_SetCompare2(TIM3, ((uint16_t) ui8_intensity) * 2000);
    TIM_CtrlPWMOutputs(TIM3, ENABLE);
  }
}

void lights_state(void)
{
  static uint8_t ui8_lights_state = 0;
  static uint8_t lcd_lights_symbol = 0;

  if (buttons_get_up_long_click_event())
  {
    buttons_clear_up_long_click_event();

    if (ui8_lights_state == 0)
    {
      ui8_lights_state = 1;
      lcd_lights_symbol = 1;
      motor_controller_data.ui8_lights = 1;
    }
    else
    {
      ui8_lights_state = 0;
      lcd_lights_symbol = 0;
      motor_controller_data.ui8_lights = 0;
    }
  }

  if(ui8_lights_state == 0) { lcd_set_backlight_intensity (configuration_variables.ui8_lcd_backlight_off_brightness); }
  else { lcd_set_backlight_intensity (configuration_variables.ui8_lcd_backlight_on_brightness); }
}

void calc_battery_voltage_soc(void)
{
  uint16_t ui16_fluctuate_battery_voltage_x10;

  // update battery level value only at every 100ms / 10 times per second and this helps to visual filter the fast changing values
  if (ui8_lcd_menu_counter_100ms_state)
  {
    // calculate flutuate voltage, that depends on the current and battery pack resistance
    ui16_fluctuate_battery_voltage_x10 = (uint16_t) ((((uint32_t) configuration_variables.ui16_battery_pack_resistance_x1000) * ((uint32_t) ui16_battery_current_filtered_x5)) / ((uint32_t) 500));
    // now add fluctuate voltage value
    motor_controller_data.ui16_battery_voltage_soc_x10 = ui16_battery_voltage_filtered_x10 + ui16_fluctuate_battery_voltage_x10;
  }
}

void battery_soc_bar_clear(uint32_t ui32_bar_number)
{
  uint32_t ui32_x1, ui32_x2;
  uint32_t ui32_y1, ui32_y2;

  // the first nine bars share the same code
  if (ui32_bar_number < 10)
  {
    ui32_x1 = 11 + (7 * (ui32_bar_number - 1));
    ui32_y1 = 11;
    ui32_x2 = ui32_x1 + 5;
    ui32_y2 = ui32_y1 + 19;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

    if (ui32_bar_number != 9)
    {
      ui32_x1 = ui32_x2 + 1;
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_BLACK);
    }
    else
    {
      ui32_x1 = ui32_x2 + 1;
      ui32_y1 = 11 + 6;
      ui32_x2 = ui32_x1;
      ui32_y2 = ui32_y1 + 8;
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);
    }
  }
  else
  {
    ui32_x1 = 11 + 63;
    ui32_y1 = 11 + 5;
    ui32_x2 = ui32_x1 + 5;
    ui32_y2 = ui32_y1 + 9;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);
  }
}

void battery_soc_bar_set(uint32_t ui32_bar_number, uint16_t ui16_color)
{
  uint32_t ui32_x1, ui32_x2;
  uint32_t ui32_y1, ui32_y2;

  // the first nine bars share the same code
  if(ui32_bar_number < 10)
  {
    ui32_x1 = 11 + (7 * (ui32_bar_number - 1));
    ui32_y1 = 11;
    ui32_x2 = ui32_x1 + 5;
    ui32_y2 = ui32_y1 + 19;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, ui16_color);

    if (ui32_bar_number != 9)
    {
      ui32_x1 = ui32_x2 + 1;
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_DIM_GRAY);
    }
    else
    {
      ui32_x1 = ui32_x2 + 1;
      ui32_y1 = 11 + 6;
      ui32_x2 = ui32_x1;
      ui32_y2 = ui32_y1 + 8;
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_DIM_GRAY);
    }
  }
  else
  {
    ui32_x1 = 11 + 63;
    ui32_y1 = 11 + 5;
    ui32_x2 = ui32_x1 + 5;
    ui32_y2 = ui32_y1 + 9;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, ui16_color);
  }
}

void battery_soc(void)
{
  uint32_t ui32_x1, ui32_x2;
  uint32_t ui32_y1, ui32_y2;
  static uint8_t ui8_timmer_counter;
  uint32_t ui32_battery_bar_number;
  static uint32_t ui32_battery_bar_number_previous = 0;
  uint32_t ui32_battery_bar_number_offset;
  uint32_t ui32_battery_cells_number_x10;
  uint16_t ui16_color;
  static uint16_t ui16_color_previous;
  uint32_t ui32_temp;
  uint32_t ui32_i;
  static uint16_t ui16_battery_voltage_filtered_x10_previous = 0xffff;
  uint32_t ui32_value_temp;
  uint32_t ui32_value_integer;
  uint32_t ui32_value_decimal;
  uint32_t ui32_value_integer_number_digits;
  uint8_t ui8_counter;
  static uint16_t ui16_battery_soc_watts_hour_previous = 0xffff;

  static print_number_t soc =
  {
    .font = &SMALL_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 3,
    .ui8_left_zero_paddig = 0,
  };

  if(lcd_vars.ui32_main_screen_draw_static_info)
  {
    // first, clear the full symbol area
    ui32_x1 = 10;
    ui32_y1 = 10;
    ui32_x2 = ui32_x1 + 56;
    ui32_y2 = ui32_y1 + 21;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

    ui32_x1 = 10 + 56;
    ui32_y1 = 10 + 5;
    ui32_x2 = ui32_x1 + 6;
    ui32_y2 = ui32_y1 + 11;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

    // now draw the empty battery symbol
    ui32_x1 = 10;
    ui32_y1 = 10;
    ui32_x2 = ui32_x1 + 63;
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 + 5;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1 + 7;
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 + 11;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1 - 7;
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 + 5;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1 - 63;
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 - 21;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);
  }

  // update battery level value only at every 100ms / 10 times per second and this helps to visual filter the fast changing values
  if((ui8_timmer_counter++ >= 10) ||
      (lcd_vars.ui32_main_screen_draw_static_info))
  {
    ui8_timmer_counter = 0;

    // to keep same scale as voltage of x10
    ui32_battery_cells_number_x10 = (uint32_t) (configuration_variables.ui8_battery_cells_number * 10);

    if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_90))) { ui32_battery_bar_number = 10; }
    else if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_80))) { ui32_battery_bar_number = 9; }
    else if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_70))) { ui32_battery_bar_number = 8; }
    else if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_60))) { ui32_battery_bar_number = 7; }
    else if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_50))) { ui32_battery_bar_number = 6; }
    else if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_40))) { ui32_battery_bar_number = 5; }
    else if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_30))) { ui32_battery_bar_number = 4; }
    else if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_20))) { ui32_battery_bar_number = 3; }
    else if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_10))) { ui32_battery_bar_number = 2; }
    else if(motor_controller_data.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_0))) { ui32_battery_bar_number = 1; }
    else { ui32_battery_bar_number = 0; }

    // find the color to draw the bars
    if(ui32_battery_bar_number > 3) { ui16_color = C_GREEN; }
    else if(ui32_battery_bar_number == 3) { ui16_color = C_YELLOW; }
    else if(ui32_battery_bar_number == 2) { ui16_color = C_ORANGE; }
    else if(ui32_battery_bar_number == 1) { ui16_color = C_RED; }

    // force draw of the bars if needed
    if(lcd_vars.ui32_main_screen_draw_static_info)
    {
      ui32_battery_bar_number_previous = 0;
    }

    // number of vars are equal as before, nothing new to draw so return
    if(ui32_battery_bar_number == ui32_battery_bar_number_previous)
    {
      // do nothing
    }
    // draw new bars
    else if(ui32_battery_bar_number > ui32_battery_bar_number_previous)
    {
      // we need to redraw the total number of bars
      if(ui16_color != ui16_color_previous)
      {
        for(ui32_i = 1; ui32_i <= ui32_battery_bar_number; ui32_i++)
        {
          battery_soc_bar_set(ui32_i, ui16_color);
        }
      }
      else
      {
        ui32_temp = (ui32_battery_bar_number - ui32_battery_bar_number_previous) + 1;
        for(ui32_i = 1; ui32_i < ui32_temp; ui32_i++)
        {
          battery_soc_bar_set(ui32_battery_bar_number_previous + ui32_i, ui16_color);
        }
      }
    }
    // delete bars
    else if(ui32_battery_bar_number < ui32_battery_bar_number_previous)
    {
      // we need to redraw the total number of bars
      if(ui16_color != ui16_color_previous)
      {
        // first deleted the needed number of vars
        ui32_temp = ui32_battery_bar_number_previous - ui32_battery_bar_number;
        for(ui32_i = 0; ui32_i <= (ui32_temp - 1); ui32_i++)
        {
          battery_soc_bar_clear(ui32_battery_bar_number_previous - ui32_i);
        }

        // now draw the new ones with the new color
        for(ui32_i = 1; ui32_i <= ui32_battery_bar_number; ui32_i++)
        {
          battery_soc_bar_set(ui32_i, ui16_color);
        }
      }
      else
      {
        ui32_temp = ui32_battery_bar_number_previous - ui32_battery_bar_number;
        for(ui32_i = 0; ui32_i <= (ui32_temp - 1); ui32_i++)
        {
          battery_soc_bar_clear(ui32_battery_bar_number_previous - ui32_i);
        }
      }
    }

    ui32_battery_bar_number_previous = ui32_battery_bar_number;
    ui16_color_previous = ui16_color;

    // draw volts
    //
    if((ui16_battery_voltage_filtered_x10 != ui16_battery_voltage_filtered_x10_previous) ||
        (lcd_vars.ui32_main_screen_draw_static_info))
    {
      ui16_battery_voltage_filtered_x10_previous = ui16_battery_voltage_filtered_x10;

      // first clear the area
      // 3 digits + 1 point
      ui32_x1 = 35;
      ui32_y1 = 35;
      ui32_x2 = ui32_x1 + ((3 * 10) + (3 * 1) + 10);
      ui32_y2 = ui32_y1 + 18;
      UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

      ui32_value_integer = ((uint32_t) ui16_battery_voltage_filtered_x10) / 10;
      ui32_value_decimal = ((uint32_t) ui16_battery_voltage_filtered_x10) % 10;

      // find how many digits is the ui32_value_integer
      ui32_value_temp = ui32_value_integer;
      ui32_value_integer_number_digits = 0;
      for(ui8_counter = 0; ui8_counter < 3; ui8_counter++)
      {
        ui32_value_temp /= 10;
        ui32_value_integer_number_digits++;

        // finish for loop
        if (ui32_value_temp == 0)
        {
          ui32_value_integer_number_digits++;
          break;
        }
      }

      // draw variable value
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&SMALL_TEXT_FONT);
      ui32_x1 = 35;
      ui32_y1 = 37;
      UG_PutString(ui32_x1, ui32_y1, itoa(ui32_value_integer));

      // draw small point
      ui32_x1 += 4 + ((ui32_value_integer_number_digits - 1) * 10) - ((ui32_value_integer_number_digits - 1) * 1);
      ui32_y1 = 48;
      lcd_pixel_set(ui32_x1, ui32_y1, C_GRAY);

      // draw decimal digit
      ui32_x1 += 3;
      ui32_y1 = 37;
      UG_PutString(ui32_x1, ui32_y1, itoa(ui32_value_decimal));

      ui32_x1 += (2 + 10);
      ui32_y1 = 36;
      UG_PutString(ui32_x1, ui32_y1, "v");
    }

    // draw SOC in percentage
    if((ui16_battery_soc_watts_hour != ui16_battery_soc_watts_hour_previous) ||
        (lcd_vars.ui32_main_screen_draw_static_info))
    {
      ui16_battery_soc_watts_hour_previous = ui16_battery_soc_watts_hour;

      soc.ui32_x_position = 90;
      soc.ui32_y_position = 14;
      soc.ui32_number = ui16_battery_soc_watts_hour;
      lcd_print_number(&soc);

      ui32_x1 = soc.ui32_x_final_position + 2;
      ui32_y1 = soc.ui32_y_final_position;
      UG_PutString(ui32_x1, ui32_y1, "%");
    }
  }
}

void temperature(void)
{
  static uint8_t ui8_motor_temperature_previous;
  uint32_t ui32_x1;
  uint32_t ui32_y1;
  uint32_t ui32_x2;
  uint32_t ui32_y2;
  uint8_t ui8_ascii_degree = 176;

  if (configuration_variables.ui8_temperature_limit_feature_enabled)
  {
    if((motor_controller_data.ui8_motor_temperature != ui8_motor_temperature_previous) ||
        (lcd_vars.ui32_main_screen_draw_static_info))
    {
      ui8_motor_temperature_previous = motor_controller_data.ui8_motor_temperature;

      // first clear the area
      // 5 digits + some space
      ui32_x1 = DISPLAY_WIDTH - 1 - 18 - (7 * 10) + (7 * 1) + 10;
      ui32_y1 = 32;
      ui32_x2 = ui32_x1 + (7 * 10) + (7 * 1) + 10;
      ui32_y2 = ui32_y1 + 18;
      UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

      // draw
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&SMALL_TEXT_FONT);
      ui32_x1 = DISPLAY_WIDTH - 1 - 18 - (7 * 10) + (7 * 1) + 10;
      ui32_y1 = 32;

      if(motor_controller_data.ui8_motor_temperature < 10)
      {
        ui32_x1 += 22;
        UG_PutString(ui32_x1, ui32_y1, itoa(motor_controller_data.ui8_motor_temperature));

        ui32_x1 += ((1 * 10) + (1 * 1) + 1);
        UG_PutString(ui32_x1, ui32_y1, &ui8_ascii_degree);
        ui32_x1 += 11;
        UG_PutString(ui32_x1, ui32_y1, "c");
      }
      else if(motor_controller_data.ui8_motor_temperature < 100)
      {
        ui32_x1 += 11;
        UG_PutString(ui32_x1, ui32_y1, itoa(motor_controller_data.ui8_motor_temperature));

        ui32_x1 += ((2 * 10) + (2 * 1) + 1);
        UG_PutString(ui32_x1, ui32_y1, &ui8_ascii_degree);
        ui32_x1 += 11;
        UG_PutString(ui32_x1, ui32_y1, "c");
      }
      else
      {
        UG_PutString(ui32_x1, ui32_y1, itoa(motor_controller_data.ui8_motor_temperature));

        ui32_x1 += ((3 * 10) + (3 * 1) + 1);
        UG_PutString(ui32_x1, ui32_y1, &ui8_ascii_degree);
        ui32_x1 += 11;
        UG_PutString(ui32_x1, ui32_y1, "c");
      }
    }
  }
}

void time(void)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  static struct_rtc_time_t rtc_time_previous;
  static struct_rtc_time_t *p_rtc_time_previous;
  struct_rtc_time_t *p_rtc_time;

  static print_number_t hours =
  {
    .font = &REGULAR_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 2,
    .ui8_left_zero_paddig = 0,
  };

  static print_number_t minutes =
  {
    .font = &REGULAR_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 2,
    .ui8_left_zero_paddig = 1,
  };

  p_rtc_time_previous = &rtc_time_previous;
  p_rtc_time = rtc_get_time();

  if ((p_rtc_time->ui8_hours != p_rtc_time_previous->ui8_hours) ||
      (p_rtc_time->ui8_minutes != p_rtc_time_previous->ui8_minutes))
  {
    p_rtc_time_previous->ui8_hours = p_rtc_time->ui8_hours;
    p_rtc_time_previous->ui8_minutes = p_rtc_time->ui8_minutes;

    // print hours number
    ui32_x_position = DISPLAY_WIDTH - 1 - 12 - (7 * 12) + (7 * 1) + 12;
    ui32_y_position = 4;
    hours.ui32_x_position = ui32_x_position;
    hours.ui32_y_position = ui32_y_position;
    hours.ui32_number = p_rtc_time->ui8_hours;
    lcd_print_number(&hours);

    // print ":"
    ui32_x_position = hours.ui32_x_final_position;
    ui32_y_position = hours.ui32_y_final_position;
    UG_PutChar(58, ui32_x_position, ui32_y_position, C_WHITE, C_BLACK);
    ui32_x_position += minutes.font->char_width; // x width from ":"

    // print minutes number
    minutes.ui32_x_position = ui32_x_position;
    minutes.ui32_y_position = ui32_y_position;
    minutes.ui32_number = p_rtc_time->ui8_minutes;
    lcd_print_number(&minutes);
  }
}

void power(void)
{
  static uint16_t ui16_battery_power_filtered_previous;
  uint32_t ui32_x1;
  uint32_t ui32_y1;
  uint32_t ui32_x2;
  uint32_t ui32_y2;
  static uint8_t ui8_target_max_battery_power_state = 0;
  uint16_t _ui16_battery_power_filtered;
  uint16_t ui16_target_max_power;

  static print_number_t power_number =
  {
    .font = &FONT_24X40,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui32_x_position = 200,
    .ui32_y_position = 219,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 4,
    .ui8_left_zero_paddig = 0,
    .ui32_number = 0
  };

  if(lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_GRAY);
    UG_FontSelect(&FONT_10X16);
    UG_PutString(238, 188, "Power");
  }

  if(!lcd_vars.ui8_lcd_menu_max_power)
  {
    _ui16_battery_power_filtered = ui16_battery_power_filtered;

    if((_ui16_battery_power_filtered != ui16_battery_power_filtered_previous) ||
        lcd_vars.ui32_main_screen_draw_static_info ||
        ui8_target_max_battery_power_state == 0)
    {
      ui16_battery_power_filtered_previous = _ui16_battery_power_filtered;
      ui8_target_max_battery_power_state = 1;

      if (_ui16_battery_power_filtered > 9999) { _ui16_battery_power_filtered = 9999; }

      power_number.ui32_number = _ui16_battery_power_filtered;
      lcd_print_number(&power_number);
    }
    else
    {

    }
  }
  else
  {
    // because this click envent can happens and will block the detection of button_onoff_long_click_event
    buttons_clear_onoff_click_event();

    // leave this menu with a button_onoff_long_click
    if(buttons_get_onoff_long_click_event())
    {
      buttons_clear_all_events();
      lcd_vars.ui8_lcd_menu_max_power = 0;
      ui8_target_max_battery_power_state = 0;

      // save the updated variables on EEPROM
      eeprom_write_variables();

      buttons_clear_all_events();
      return;
    }

    if(buttons_get_up_click_event())
    {
      buttons_clear_all_events();

      if(configuration_variables.ui8_target_max_battery_power < 10)
      {
        configuration_variables.ui8_target_max_battery_power++;
      }
      else
      {
        configuration_variables.ui8_target_max_battery_power += 2;
      }

      // limit to 100 * 25 = 2500 Watts
      if(configuration_variables.ui8_target_max_battery_power > 100) { configuration_variables.ui8_target_max_battery_power = 100; }
    }

    if(buttons_get_down_click_event ())
    {
      buttons_clear_all_events();

      if(configuration_variables.ui8_target_max_battery_power == 0)
      {

      }
      else if(configuration_variables.ui8_target_max_battery_power <= 10)
      {
        configuration_variables.ui8_target_max_battery_power--;
      }
      else
      {
        configuration_variables.ui8_target_max_battery_power -= 2;
      }
    }

    if(ui8_lcd_menu_flash_state)
    {
      if(ui8_target_max_battery_power_state == 1)
      {
        ui8_target_max_battery_power_state = 0;

        // clear area
        ui32_x1 = 200;
        ui32_y1 = 219;
        ui32_x2 = ui32_x1 + (5 * 24) + (5 * 24) + 1;
        ui32_y2 = ui32_y1 + 41;
        UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);
      }
    }
    else
    {
      if(ui8_target_max_battery_power_state == 0)
      {
        ui8_target_max_battery_power_state = 1;

        ui16_target_max_power = configuration_variables.ui8_target_max_battery_power * 25;

        // clear area
        ui32_x1 = 200;
        ui32_y1 = 219;
        ui32_x2 = ui32_x1 + (4 * 24) + (4 * 24) + 1;
        ui32_y2 = ui32_y1 + 41;
        UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

        // draw number
        ui32_x1 = 200;
        ui32_y1 = 219;
        if(ui16_target_max_power >= 1000) {  }
        else if(ui16_target_max_power >= 100) { ui32_x1 += 24; }
        else if(ui16_target_max_power >= 10) { ui32_x1 += 48; }
        else { ui32_x1 += 72; }

        UG_SetForecolor(C_WHITE);
        UG_FontSelect(&FONT_24X40);
        UG_PutString(ui32_x1, ui32_y1, itoa((uint32_t) ui16_target_max_power));

        configuration_variables.ui8_target_max_battery_power = ui16_target_max_power / 25;
      }
    }
  }
}

void wheel_speed(void)
{
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  static uint16_t ui16_wheel_x10_speed_previous = 0xffff;

  static print_number_t wheel_speed_integer =
  {
    .font = &BIG_NUMBERS_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 2,
    .ui8_left_zero_paddig = 0,
  };

  static print_number_t wheel_speed_decimal =
  {
    .font = &MEDIUM_NUMBERS_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 1,
    .ui8_left_zero_paddig = 0,
  };

  if (lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_GRAY);
    UG_FontSelect(&FONT_10X16);
    UG_PutString(200, 135 , "km/h");

    // print dot
    UG_FillCircle(160, 145, 2, C_WHITE);
  }

  if ((motor_controller_data.ui16_wheel_speed_x10 != ui16_wheel_x10_speed_previous) ||
      lcd_vars.ui32_main_screen_draw_static_info)
  {
    ui16_wheel_x10_speed_previous = motor_controller_data.ui16_wheel_speed_x10;

    ui32_x_position = 92;
    ui32_y_position = 106;

    wheel_speed_integer.ui32_x_position = ui32_x_position;
    wheel_speed_integer.ui32_y_position = ui32_y_position;
    wheel_speed_integer.ui32_number = (uint32_t) (motor_controller_data.ui16_wheel_speed_x10 / 10);
    lcd_print_number(&wheel_speed_integer);

    // accounting for dot: 10 px
    ui32_x_position = wheel_speed_integer.ui32_x_final_position + 10;

    ui32_y_position = ui32_y_position + 10;

    wheel_speed_decimal.ui32_x_position = ui32_x_position;
    wheel_speed_decimal.ui32_y_position = ui32_y_position;
    wheel_speed_decimal.ui32_number = (uint32_t) (motor_controller_data.ui16_wheel_speed_x10 % 10);
    lcd_print_number(&wheel_speed_decimal);
  }
}

void calc_battery_soc_watts_hour(void)
{
  uint32_t ui32_temp;

  __disable_irq();
  ui32_temp = motor_controller_data.ui32_wh_x10 * 100;
  __enable_irq();
  if (configuration_variables.ui32_wh_x10_100_percent > 0)
  {
    ui32_temp /= configuration_variables.ui32_wh_x10_100_percent;
  }
  else
  {
    ui32_temp = 0;
  }

  // 100% - current SOC or just current SOC
  if (configuration_variables.ui8_battery_soc_increment_decrement)
  {
    if (ui32_temp > 100)
      ui32_temp = 100;

    ui16_battery_soc_watts_hour = 100 - ui32_temp;
  }
  else
  {
    ui16_battery_soc_watts_hour = ui32_temp;
  }
}

struct_lcd_vars* get_lcd_vars(void)
{
  return &lcd_vars;
}

void lcd_print_number(print_number_t* number)
{
  uint32_t ui32_number_temp;
  uint8_t ui8_digit_inverse_counter;
  uint8_t ui8_digits_array[MAX_NUMBER_DIGITS];
  static uint32_t ui32_power_array[MAX_NUMBER_DIGITS] = {1, 10, 100, 1000, 10000};
  uint32_t ui32_number = number->ui32_number;
  uint8_t ui8_i;
  uint32_t ui32_x_position = number->ui32_x_position;
  uint32_t ui32_y_position = number->ui32_y_position;
  // save digit number where number start
  uint8_t ui8_digit_number_start = 0;

  // can't process over MAX_NUMBER_DIGITS
  if(number->ui8_field_number_of_digits > MAX_NUMBER_DIGITS)
  {
    return;
  }

  // set the font that will be used
  UG_FontSelect(number->font);

  // get all digits from the number
  ui32_number_temp = ui32_number;
  for(ui8_i = 0; ui8_i < number->ui8_field_number_of_digits; ui8_i++)
  {
    ui8_digits_array[ui8_i] = ui32_number_temp % 10;
    ui32_number_temp /= 10;

    // find the digit number start
    if(ui8_digits_array[ui8_i] != 0)
    {
      ui8_digit_number_start = ui8_i;
    }
  }

  // loop over all digits
  ui8_digit_inverse_counter = number->ui8_field_number_of_digits - 1;
  for(ui8_i = 0; ui8_i < number->ui8_field_number_of_digits; ui8_i++)
  {
    // only digits that changed
    if(ui8_digits_array[ui8_digit_inverse_counter] != number->ui8_previous_digits_array[ui8_digit_inverse_counter])
    {
      if((ui8_digits_array[ui8_digit_inverse_counter] == 0) && // if is a 0
          (ui8_digit_inverse_counter > ui8_digit_number_start) && // if is a digit at left from the first digit
          (number->ui8_left_zero_paddig)) // if we want to print a 0 at left
      {
        // print a "0"
        UG_PutChar(48, ui32_x_position, ui32_y_position, number->fore_color, number->back_color);
      }
      else if((ui8_digits_array[ui8_digit_inverse_counter] == 0) &&  // if is a 0
          (ui8_digit_inverse_counter > ui8_digit_number_start) && // if is a digit at left from the first digit
          (!number->ui8_left_zero_paddig)) // if we NOT want to print a 0 at left
      {
        // print a " "
        UG_PutChar(32, ui32_x_position, ui32_y_position, number->fore_color, number->back_color);
      }
      else
      {
        // print the digit
        UG_PutChar((ui8_digits_array[ui8_digit_inverse_counter] + 48), ui32_x_position, ui32_y_position, number->fore_color, number->back_color);
      }
    }
    // the case where there was a 0 but we want to remove it
    else if((ui8_digits_array[ui8_digit_inverse_counter] == 0) &&  // if is a 0
        (ui8_digit_inverse_counter > ui8_digit_number_start) && // if is a digit at left from the first digit
        (!number->ui8_left_zero_paddig)) // if we NOT want to print a 0 at left
    {
      // print a " "
      UG_PutChar(32, ui32_x_position, ui32_y_position, number->fore_color, number->back_color);
    }
    // the case where there was a " " but we need to write a 0
    else if((ui8_digits_array[ui8_digit_inverse_counter] == 0) &&  // if is a 0
        (ui8_digit_number_start > number->ui8_digit_number_start_previous)) // if is a digit at left from the first digit
    {
      // print a "0"
      UG_PutChar(48, ui32_x_position, ui32_y_position, number->fore_color, number->back_color);
    }
    else
    {
      // do not change the field, keep with previous value
    }

    ui8_digit_inverse_counter--;

    // increase X position for next char
    ui32_x_position += number->font->char_width + 1;
  }

  // save the digits
  for(ui8_i = 0; ui8_i < number->ui8_field_number_of_digits; ui8_i++)
  {
    number->ui8_previous_digits_array[ui8_i] = ui8_digits_array[ui8_i];
  }

  number->ui8_digit_number_start_previous = ui8_digit_number_start;

  // store final position
  number->ui32_x_final_position = ui32_x_position;
  number->ui32_y_final_position = ui32_y_position;
}

