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

#include "config.h"
#include "utils.h"
#include "pins.h"
#include "lcd.h"
#include "buttons.h"
#include "eeprom.h"
#include "ugui_driver/ugui_bafang_500c.h"
#include "ugui/ugui.h"

static struct_motor_controller_data motor_controller_data;
static struct_configuration_variables configuration_variables;

static uint32_t ui32_battery_voltage_accumulated_x10000 = 0;
static uint16_t ui16_battery_voltage_filtered_x10;

static uint16_t ui16_battery_current_accumulated_x5 = 0;
static uint16_t ui16_battery_current_filtered_x5;

static uint16_t ui16_battery_power_accumulated = 0;
static uint16_t ui16_battery_power_filtered_x50;
static uint16_t ui16_battery_power_filtered;

static uint32_t ui32_wh_sum_x5 = 0;
static uint32_t ui32_wh_sum_counter = 0;
static uint32_t ui32_wh_x10 = 0;
static uint8_t ui8_config_wh_x10_offset;

static uint32_t ui32_pedal_torque_accumulated = 0;
static uint16_t ui16_pedal_torque_filtered;
static uint32_t ui32_pedal_power_accumulated = 0;
static uint16_t ui16_pedal_power_filtered;

static uint16_t ui16_pedal_cadence_accumulated = 0;
static uint8_t ui8_pedal_cadence_filtered;

static uint8_t ui8_lights_state = 0;
static uint8_t lcd_lights_symbol = 0;

static uint16_t ui16_battery_soc_watts_hour;

uint8_t ui8_lcd_power_off_time_counter_minutes = 0;
static uint16_t ui16_lcd_power_off_time_counter = 0;

static uint16_t ui16_battery_voltage_soc_x10;

static uint8_t ui8_lcd_menu_counter_100ms = 0;
static uint8_t ui8_lcd_menu_counter_100ms_state = 0;

static uint8_t ui8_lcd_menu_counter_500ms = 0;
static uint8_t ui8_lcd_menu_counter_500ms_state = 0;

static uint8_t ui8_lcd_menu_config_submenu_state = 0;
static uint8_t ui8_lcd_menu_flash_counter = 0;
static uint16_t ui16_lcd_menu_flash_counter_temperature = 0;
static uint8_t ui8_lcd_menu_flash_state;
static uint8_t ui8_lcd_menu_flash_state_temperature;
static uint8_t ui8_lcd_menu_config_submenu_number = 0;
static uint8_t ui8_lcd_menu_config_submenu_active = 0;

void lcd_execute_main_screen (void);
void assist_level_state (void);
void power_off_management (void);
void lcd_power_off (uint8_t updateDistanceOdo);
void low_pass_filter_battery_voltage_current_power (void);
void update_menu_flashing_state (void);
void calc_battery_soc_watts_hour (void);
void calc_odometer (void);
static void automatic_power_off_management (void);
void calc_wh (void);
void brake (void);
void wheel_speed(void);
void power (void);
void power_off_management (void);
void temperature (void);
void battery_soc (void);
void calc_battery_voltage_soc (void);
void low_pass_filter_pedal_torque_and_power (void);
static void low_pass_filter_pedal_cadence (void);
void lights_state(void);
void lcd_set_backlight_intensity(uint8_t ui8_intensity);

/* Place your initialization/startup code here (e.g. MyInst_Start()) */
void lcd_init(void)
{
  bafang_500C_lcd_init();
  UG_FillScreen(0);

  // init variables with the stored value on EEPROM
  eeprom_init_variables ();
}

void lcd_clock(void)
{
  calc_battery_soc_watts_hour ();

  low_pass_filter_battery_voltage_current_power ();
  // filter using only each 500ms values
  if (ui8_lcd_menu_counter_500ms_state)
  {
    low_pass_filter_pedal_cadence ();
  }

  // filter using only each 100ms values
  if (ui8_lcd_menu_counter_100ms_state)
  {
    low_pass_filter_pedal_torque_and_power ();
  }

  calc_battery_voltage_soc ();
  calc_odometer ();
  automatic_power_off_management ();

  lcd_execute_main_screen ();

  // power off system: ONOFF long click event
  power_off_management ();
}

void lcd_draw_main_menu_mask(void)
{
  UG_DrawLine(10, 60, 310, 60, C_DIM_GRAY);

  UG_DrawLine(10, 180, 310, 180, C_DIM_GRAY);

  UG_DrawLine(10, 265, 310, 265, C_DIM_GRAY);

  // wheel speed field
  // print dot
  // dot 10 px
  UG_FillCircle(160, 145, 2, C_WHITE);

  // units
  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(C_GRAY);
  UG_FontSelect(&FONT_10X16);
  UG_PutString(200, 135 , "km/h");

  // assist level field
  //
  UG_PutString(10, 188, "Assist");

  // power field
  //
  UG_PutString(238, 188, "Power");
}

void lcd_execute_main_screen (void)
{
motor_controller_data.ui16_wheel_speed_x10 = 348;
ui16_battery_power_filtered = 950;
configuration_variables.ui8_number_of_assist_levels = 5;

//  temperature ();
  assist_level_state();
//  odometer ();
  wheel_speed();
//  walk_assist_state ();
//  offroad_mode ();
  power ();
//  battery_soc ();
//  lights_state ();
//  brake ();
}

static buttons_events_type_t events = 0;
static buttons_events_type_t last_events = 0;

void assist_level_state (void)
{
  static uint8_t ui8_assist_level_last = 0xff;

  if (buttons_get_up_click_event ())
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

  if (buttons_get_down_click_event ())
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

  if (configuration_variables.ui8_assist_level != ui8_assist_level_last)
  {
    ui8_assist_level_last = configuration_variables.ui8_assist_level;

    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&FONT_32X53);
    UG_PutString(25, 210, itoa((uint32_t) configuration_variables.ui8_assist_level));
  }
}

struct_configuration_variables* get_configuration_variables (void)
{
  return &configuration_variables;
}

struct_motor_controller_data* lcd_get_motor_controller_data (void)
{
  return &motor_controller_data;
}

void power_off_management (void)
{
  // turn off
  if (buttons_get_onoff_long_click_event ()) { lcd_power_off (1); }
}

void lcd_power_off (uint8_t updateDistanceOdo)
{
//  if (updateDistanceOdo)
//  {
//    configuration_variables.ui32_wh_x10_offset = ui32_wh_x10;
//    configuration_variables.ui32_odometer_x10 += ((uint32_t) configuration_variables.ui16_odometer_distance_x10);
//  }

  // save the variables on EEPROM
  eeprom_write_variables ();

  // put screen all black and disable backlight
  UG_FillScreen(0);
  lcd_backlight(0);

  // now disable the power to all the system
  system_power(0);

  // block here
  while (1) ;
}

void low_pass_filter_battery_voltage_current_power (void)
{
  // low pass filter battery voltage
  ui32_battery_voltage_accumulated_x10000 -= ui32_battery_voltage_accumulated_x10000 >> BATTERY_VOLTAGE_FILTER_COEFFICIENT;
  ui32_battery_voltage_accumulated_x10000 += (uint32_t) motor_controller_data.ui16_adc_battery_voltage * ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000;
  ui16_battery_voltage_filtered_x10 = ((uint32_t) (ui32_battery_voltage_accumulated_x10000 >> BATTERY_VOLTAGE_FILTER_COEFFICIENT)) / 1000;

  // low pass filter batery current
  ui16_battery_current_accumulated_x5 -= ui16_battery_current_accumulated_x5 >> BATTERY_CURRENT_FILTER_COEFFICIENT;
  ui16_battery_current_accumulated_x5 += (uint16_t) motor_controller_data.ui8_battery_current_x5;
  ui16_battery_current_filtered_x5 = ui16_battery_current_accumulated_x5 >> BATTERY_CURRENT_FILTER_COEFFICIENT;

  // battery power
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
}

void low_pass_filter_pedal_torque_and_power (void)
{
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

static void low_pass_filter_pedal_cadence (void)
{
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

void calc_wh (void)
{
  static uint8_t ui8_1s_timmer_counter;
  uint32_t ui32_temp = 0;

  if (ui16_battery_power_filtered_x50 > 0)
  {
    ui32_wh_sum_x5 += ui16_battery_power_filtered_x50 / 10;
    ui32_wh_sum_counter++;
  }

  // calc at 1s rate
  if (ui8_1s_timmer_counter++ >= 10)
  {
    ui8_1s_timmer_counter = 0;

    // avoid  zero divisison
    if (ui32_wh_sum_counter != 0)
    {
      ui32_temp = ui32_wh_sum_counter / 36;
      ui32_temp = (ui32_temp * (ui32_wh_sum_x5 / ui32_wh_sum_counter)) / 500;
    }

    ui32_wh_x10 = configuration_variables.ui32_wh_x10_offset + ui32_temp;
  }
}

void calc_odometer (void)
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

static void automatic_power_off_management (void)
{
  if (configuration_variables.ui8_lcd_power_off_time_minutes != 0)
  {
    // see if we should reset the automatic power off minutes counter
    if ((motor_controller_data.ui16_wheel_speed_x10 > 0) ||   // wheel speed > 0
        (motor_controller_data.ui8_battery_current_x5 > 0) || // battery current > 0
        (motor_controller_data.ui8_braking) ||                // braking
        buttons_get_events ())                                 // any button active
    {
      ui16_lcd_power_off_time_counter = 0;
      ui8_lcd_power_off_time_counter_minutes = 0;
    }

    // increment the automatic power off minutes counter
    ui16_lcd_power_off_time_counter++;

    // check if we should power off the LCD
    if (ui16_lcd_power_off_time_counter >= (100 * 60)) // 1 minute passed
    {
      ui16_lcd_power_off_time_counter = 0;

      ui8_lcd_power_off_time_counter_minutes++;
      if (ui8_lcd_power_off_time_counter_minutes >= configuration_variables.ui8_lcd_power_off_time_minutes)
      {
        lcd_power_off (1);
      }
    }
  }
  else
  {
    ui16_lcd_power_off_time_counter = 0;
    ui8_lcd_power_off_time_counter_minutes = 0;
  }
}

void update_menu_flashing_state (void)
{
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

  ui8_lcd_menu_counter_500ms_state = 0;
  if (ui8_lcd_menu_counter_500ms++ > 50)
  {
    ui8_lcd_menu_counter_500ms = 0;
    ui8_lcd_menu_counter_500ms_state = 1;
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

void brake (void)
{
//  if (motor_controller_data.ui8_braking) { lcd_enable_brake_symbol (1); }
//  else { lcd_enable_brake_symbol (0); }
}


void lcd_set_backlight_intensity (uint8_t ui8_intensity)
{
//  if (ui8_intensity == 0)
//  {
//    TIM1_CCxCmd (TIM1_CHANNEL_4, DISABLE);
//  }
//  else if (ui8_intensity <= 20)
//  {
//    TIM1_SetCompare4 ((uint16_t) ui8_intensity);
//    TIM1_CCxCmd (TIM1_CHANNEL_4, ENABLE);
//  }
}

void lights_state (void)
{
  if (buttons_get_up_long_click_event ())
  {
    buttons_clear_up_long_click_event ();

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

  if (ui8_lights_state == 0) { lcd_set_backlight_intensity (configuration_variables.ui8_lcd_backlight_off_brightness); }
  else { lcd_set_backlight_intensity (configuration_variables.ui8_lcd_backlight_on_brightness); }

//  lcd_enable_lights_symbol (lcd_lights_symbol);
}

void calc_battery_voltage_soc (void)
{
  uint16_t ui16_fluctuate_battery_voltage_x10;

  // update battery level value only at every 100ms / 10 times per second and this helps to visual filter the fast changing values
  if (ui8_lcd_menu_counter_100ms_state)
  {
    // calculate flutuate voltage, that depends on the current and battery pack resistance
    ui16_fluctuate_battery_voltage_x10 = (uint16_t) ((((uint32_t) configuration_variables.ui16_battery_pack_resistance_x1000) * ((uint32_t) ui16_battery_current_filtered_x5)) / ((uint32_t) 500));
    // now add fluctuate voltage value
    ui16_battery_voltage_soc_x10 = ui16_battery_voltage_filtered_x10 + ui16_fluctuate_battery_voltage_x10;
  }
}

void battery_soc (void)
{
  static uint8_t ui8_timmer_counter;
  static uint8_t ui8_battery_state_of_charge;
  uint8_t ui8_battery_cells_number_x10;

  // update battery level value only at every 100ms / 10 times per second and this helps to visual filter the fast changing values
  if (ui8_timmer_counter++ >= 10)
  {
    ui8_timmer_counter = 0;

    // to keep same scale as voltage of x10
    ui8_battery_cells_number_x10 = configuration_variables.ui8_battery_cells_number * 10;

    if (ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui8_battery_cells_number_x10 * LI_ION_CELL_VOLTS_83))) { ui8_battery_state_of_charge = 4; } // 4 bars | full
    else if (ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui8_battery_cells_number_x10 * LI_ION_CELL_VOLTS_50))) { ui8_battery_state_of_charge = 3; } // 3 bars
    else if (ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui8_battery_cells_number_x10 * LI_ION_CELL_VOLTS_17))) { ui8_battery_state_of_charge = 2; } // 2 bars
    else if (ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui8_battery_cells_number_x10 * LI_ION_CELL_VOLTS_0))) { ui8_battery_state_of_charge = 1; } // 1 bar
    else { ui8_battery_state_of_charge = 0; } // flashing
  }

//  // first clean battery symbols
//  ui8_lcd_frame_buffer[23] &= ~241;
//
//  switch (ui8_battery_state_of_charge)
//  {
//    case 0:
//    // empty, so flash the empty battery symbol
//    if (ui8_lcd_menu_flash_state)
//    {
//      ui8_lcd_frame_buffer[23] |= 16;
//    }
//    break;
//
//    case 1:
//      ui8_lcd_frame_buffer[23] |= 144;
//    break;
//
//    case 2:
//      ui8_lcd_frame_buffer[23] |= 145;
//    break;
//
//    case 3:
//      ui8_lcd_frame_buffer[23] |= 209;
//    break;
//
//    case 4:
//      ui8_lcd_frame_buffer[23] |= 241;
//    break;
//  }
}

void temperature (void)
{
  // if motor current is being limited due to temperature, force showing temperature!!
  if (motor_controller_data.ui8_temperature_current_limiting_value != 255)
  {
    if (ui8_lcd_menu_flash_state_temperature)
    {
//      lcd_print (motor_controller_data.ui8_motor_temperature, TEMPERATURE_FIELD, 0);
//      lcd_enable_temperature_degrees_symbol (1);
    }
  }
  else
  {
    switch (configuration_variables.ui8_temperature_field_config)
    {
      // show nothing
      case 0:
      break;

      // show battery_soc_watts_hour
      case 1:
//        lcd_print (ui16_battery_soc_watts_hour, TEMPERATURE_FIELD, 0);
      break;

      // show motor temperature
      case 2:
//        lcd_print (motor_controller_data.ui8_motor_temperature, TEMPERATURE_FIELD, 0);
//        lcd_enable_temperature_degrees_symbol (1);
      break;
    }
  }
}

void power (void)
{
  static uint16_t ui16_battery_power_filtered_last;
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;

  if (ui16_battery_power_filtered != ui16_battery_power_filtered_last)
  {
    ui16_battery_power_filtered_last = ui16_battery_power_filtered;

    ui32_x_position = 200;
    ui32_y_position = 219;
    if (ui16_battery_power_filtered > 1000) {  }
    else if (ui16_battery_power_filtered > 100) { ui32_x_position += 24; }
    else if (ui16_battery_power_filtered > 10) { ui32_x_position += 48; }
    else if (ui16_battery_power_filtered > 1) { ui32_x_position += 72; }

    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&FONT_24X40);
    UG_PutString(ui32_x_position, ui32_y_position, itoa((uint32_t) ui16_battery_power_filtered));
  }
}

void wheel_speed(void)
{
//  // show wheel speed only when we should not start show odometer field number
//  if (ui8_start_odometer_show_field_number == 0)
//  {
//    if (configuration_variables.ui8_units_type)
//    {
//      lcd_print (((float) motor_controller_data.ui16_wheel_speed_x10 / 1.6), WHEEL_SPEED_FIELD, 0);
//      lcd_enable_mph_symbol (1);
//    }
//    else
//    {
//      lcd_print (motor_controller_data.ui16_wheel_speed_x10, WHEEL_SPEED_FIELD, 0);
//      lcd_enable_kmh_symbol (1);
//    }
//  }

  uint32_t ui32_wheel_speed;
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  static uint16_t ui16_wheel_speed_x10_last = 0xffff;

  if (motor_controller_data.ui16_wheel_speed_x10 != ui16_wheel_speed_x10_last)
  {
    ui16_wheel_speed_x10_last = motor_controller_data.ui16_wheel_speed_x10;

    UG_SetForecolor(C_WHITE);
    UG_FontSelect(&FONT_32X53);

    ui32_wheel_speed = (uint32_t) (motor_controller_data.ui16_wheel_speed_x10 / 10);
    ui32_x_position = 124;
    ui32_y_position = 106;
    if (ui32_wheel_speed > 10)
    {
      ui32_x_position -= 32;
      UG_PutString(ui32_x_position, ui32_y_position, itoa((uint32_t) ui32_wheel_speed));
      ui32_x_position += 32;
    }
    else
    {
      UG_PutString(ui32_x_position, ui32_y_position, itoa((uint32_t) ui32_wheel_speed));
    }

    // print dot
    // dot 10 px
    ui32_x_position += 10;

    // decimal digit
    UG_FontSelect(&FONT_24X40);
    ui32_x_position += (24 + 10);
    UG_PutString(ui32_x_position, ui32_y_position + 10, itoa((uint32_t) motor_controller_data.ui16_wheel_speed_x10 % 10));
  }
}

void calc_battery_soc_watts_hour (void)
{
  uint32_t ui32_temp;

  ui32_temp = ui32_wh_x10 * 100;
  if (configuration_variables.ui32_wh_x10_100_percent > 0)
  {
    ui32_temp /= configuration_variables.ui32_wh_x10_100_percent;
  }
  else
  {
    ui32_temp = 0;
  }

  // 100% - current SOC or just current SOC
  if (configuration_variables.ui8_show_numeric_battery_soc & 2)
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
