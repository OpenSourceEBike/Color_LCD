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

// Battery SOC symbol:
// 10 bars, each bar: with = 7, height = 24
// symbol has contour lines of 1 pixel
#define BATTERY_SOC_START_X 8
#define BATTERY_SOC_START_Y 4
#define BATTERY_SOC_BAR_WITH 7
#define BATTERY_SOC_BAR_HEIGHT 24
#define BATTERY_SOC_CONTOUR 1

lcd_vars_t lcd_vars =
{
  .ui32_main_screen_draw_static_info = 1,
  .lcd_screen_state = LCD_SCREEN_MAIN,
  .ui8_lcd_menu_counter_1000ms_state = 0,
  .ui8_lcd_menu_max_power = 0,
};

static l2_vars_t l2_vars;
static l3_vars_t l3_vars;

static struct_lcd_configurations_vars *p_lcd_configurations_vars;

static uint8_t ui8_lcd_menu_counter_100ms_state = 0;

static uint8_t ui8_lcd_menu_config_submenu_state = 0;
static uint8_t ui8_lcd_menu_flash_counter = 0;
static uint16_t ui16_lcd_menu_flash_counter_temperature = 0;
static uint8_t ui8_lcd_menu_flash_state;
static uint8_t ui8_lcd_menu_flash_state_temperature;
static uint8_t ui8_lcd_menu_config_submenu_number = 0;
static uint8_t ui8_lcd_menu_config_submenu_active = 0;

volatile uint32_t ui32_g_layer_2_can_execute = 0;
volatile uint32_t ui32_g_layer_2_delayed_execute = 0;

static uint16_t ui16_m_battery_soc_watts_hour = 0;

static uint8_t ui8_m_usart1_received_first_package = 0;

void lcd_main_screen(void);
uint8_t first_time_management(void);
void assist_level_state(void);
void power_off_management(void);
void lcd_power_off(uint8_t updateDistanceOdo);
void l2_low_pass_filter_battery_voltage_current_power(void);
void update_menu_flashing_state(void);
void calc_battery_soc_watts_hour(void);
void l2_calc_odometer(void);
static void l2_automatic_power_off_management(void);
void brake(void);
void wheel_speed(void);
void power(void);
void power_off_management(void);
void temperature(void);
void time(void);
void battery_soc(void);
void l2_calc_battery_voltage_soc(void);
void l2_low_pass_filter_pedal_torque_and_power(void);
static void l2_low_pass_filter_pedal_cadence(void);
void lights_state(void);
void lcd_set_backlight_intensity(uint8_t ui8_intensity);
void battery_soc_bar_set(uint32_t ui32_bar_number, uint16_t ui16_color);
void battery_soc_bar_clear(uint32_t ui32_bar_number);
void draw_configurations_screen_mask(void);
void copy_layer_2_layer_3_vars(void);

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
  static uint8_t ui8_counter_100ms = 0;

  // every 100ms
  if(ui8_counter_100ms++ >= 3)
  {
    ui8_counter_100ms = 0;

    // receive data from layer 2 to layer 3
    // send data from layer 3 to layer 2
    ui32_g_layer_2_can_execute = 0;
    copy_layer_2_layer_3_vars();
    ui32_g_layer_2_can_execute = 1;
  }

//  if (first_time_management())
//    return;

  update_menu_flashing_state();

  calc_battery_soc_watts_hour();

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

  switch(lcd_vars.lcd_screen_state)
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
  UG_DrawLine(0, 39, 319, 39, C_DIM_GRAY);
  UG_DrawLine(0, 159, 319, 159, C_DIM_GRAY);
  UG_DrawLine(0, 239, 319, 239, C_DIM_GRAY);
  UG_DrawLine(0, 319, 319, 319, C_DIM_GRAY);

  // vertical line
  UG_DrawLine(159, 159, 159, 319, C_DIM_GRAY);
}

void lcd_main_screen (void)
{
  lights_state();

  // run once only, to draw static info
  if(lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_FillScreen(C_BLACK);
    lcd_draw_main_menu_mask();
  }

  time();
  assist_level_state();
  wheel_speed();
//  walk_assist_state();
//  offroad_mode();
  power();
  battery_soc();
  brake();

  // clear this variable after 1 full cycle running
  lcd_vars.ui32_main_screen_draw_static_info = 0;
}

void layer_2(void)
{
  uint8_t *p_rx_buffer = 0;
  static uint32_t ui32_wheel_speed_sensor_tick_temp;
  uint8_t ui8_tx_buffer[11];
  static uint8_t ui8_i;
  uint8_t ui8_temp;
  uint16_t ui16_crc_tx;
  static uint8_t ui8_lcd_variable_id = 0;
  static uint8_t ui8_master_comm_package_id = 0;
  static uint8_t ui8_slave_comm_package_id = 0;
  static uint8_t ui8_state_machine = 0;
  static uint8_t ui8_received_package_flag = 0;

  /************************************************************************************************/
  // process rx package
  if(usart1_received_package())
  {
    p_rx_buffer = usart1_get_rx_buffer();

    // now process rx data
    // only if first byte is equal to package start byte
    if(*p_rx_buffer == 67)
    {
      p_rx_buffer++;

      // send a variable for each package sent but first verify if the last one was received otherwise, keep repeating
      // keep cycling so all variables are sent
#define VARIABLE_ID_MAX_NUMBER 9
      if ((*p_rx_buffer) == ui8_master_comm_package_id) // last package data ID was receipt, so send the next one
      {
        ui8_master_comm_package_id = (ui8_master_comm_package_id + 1) % VARIABLE_ID_MAX_NUMBER;
      }
      p_rx_buffer++;

      ui8_slave_comm_package_id = *p_rx_buffer;
      p_rx_buffer++;

      l2_vars.ui16_adc_battery_voltage = *p_rx_buffer;
      p_rx_buffer++;

      l2_vars.ui16_adc_battery_voltage |= ((uint16_t) (*p_rx_buffer & 0x30)) << 4;
      p_rx_buffer++;

      l2_vars.ui8_battery_current_x5 = *p_rx_buffer;
      p_rx_buffer++;

      l2_vars.ui16_wheel_speed_x10 = (uint16_t) *p_rx_buffer;
      p_rx_buffer++;
      l2_vars.ui16_wheel_speed_x10 += ((uint16_t) *p_rx_buffer << 8);
      p_rx_buffer++;

      ui8_temp = *p_rx_buffer;
      l2_vars.ui8_braking = ui8_temp & 1;
      p_rx_buffer++;

      if(l2_vars.ui8_temperature_limit_feature_enabled)
      {
        l2_vars.ui8_adc_throttle = *p_rx_buffer;
        p_rx_buffer++;
        l2_vars.ui8_motor_temperature = *p_rx_buffer;
        p_rx_buffer++;
      }
      else
      {
        l2_vars.ui8_adc_throttle = *p_rx_buffer;
        p_rx_buffer++;
        l2_vars.ui8_throttle = *p_rx_buffer;
        p_rx_buffer++;
      }

      l2_vars.ui8_adc_pedal_torque_sensor = *p_rx_buffer;
      p_rx_buffer++;

      l2_vars.ui8_pedal_torque_sensor = *p_rx_buffer;
      p_rx_buffer++;

      l2_vars.ui8_pedal_cadence = *p_rx_buffer;
      p_rx_buffer++;

      l2_vars.ui8_pedal_human_power = *p_rx_buffer;
      p_rx_buffer++;

      l2_vars.ui8_duty_cycle = *p_rx_buffer;
      p_rx_buffer++;

      l2_vars.ui16_motor_speed_erps = (uint16_t) *p_rx_buffer;
      p_rx_buffer++;
      l2_vars.ui16_motor_speed_erps += ((uint16_t) *p_rx_buffer << 8);
      p_rx_buffer++;

      l2_vars.ui8_foc_angle = *p_rx_buffer;
      p_rx_buffer++;

      switch (ui8_slave_comm_package_id)
      {
        case 0:
          // error states
          l2_vars.ui8_error_states = *p_rx_buffer;
        break;

        case 1:
          // temperature actual limiting value
          l2_vars.ui8_temperature_current_limiting_value = *p_rx_buffer;
        break;

        case 2:
          // wheel_speed_sensor_tick_counter
          ui32_wheel_speed_sensor_tick_temp = ((uint32_t) *p_rx_buffer);
        break;

        case 3:
          // wheel_speed_sensor_tick_counter
          ui32_wheel_speed_sensor_tick_temp |= (((uint32_t) *p_rx_buffer) << 8);
        break;

        case 4:
          // wheel_speed_sensor_tick_counter
          ui32_wheel_speed_sensor_tick_temp |= (((uint32_t) *p_rx_buffer) << 16);
          l2_vars.ui32_wheel_speed_sensor_tick_counter = ui32_wheel_speed_sensor_tick_temp;
        break;
      }
      p_rx_buffer++;

      // ui16_pedal_torque_x10
      l2_vars.ui16_pedal_torque_x10 = (uint16_t) *p_rx_buffer;
      p_rx_buffer++;
      l2_vars.ui16_pedal_torque_x10 += ((uint16_t) *p_rx_buffer << 8);
      p_rx_buffer++;

      // ui16_pedal_power_x10
      l2_vars.ui16_pedal_power_x10 = (uint16_t) *p_rx_buffer;
      p_rx_buffer++;
      l2_vars.ui16_pedal_power_x10 += ((uint16_t) *p_rx_buffer << 8);
      p_rx_buffer++;

      usart1_reset_received_package();
    }
  }
  /************************************************************************************************/

  /************************************************************************************************/
  // send tx package

  // start up byte
  ui8_tx_buffer[0] = 0x59;
  ui8_tx_buffer[1] = ui8_master_comm_package_id;
  ui8_tx_buffer[2] = ui8_slave_comm_package_id;

  // set assist level value
  if(l2_vars.ui8_assist_level)
  {
    ui8_tx_buffer[3] = l2_vars.ui8_assist_level_factor[((l2_vars.ui8_assist_level) - 1)];
  }
  else
  {
    ui8_tx_buffer[3] = 0;
  }

  // set lights state
  // walk assist level state
  // set offroad state
  ui8_tx_buffer[4] = (l2_vars.ui8_lights & 1) |
      ((l2_vars.ui8_walk_assist_level & 1) << 1) |
      ((l2_vars.ui8_offroad_mode & 1) << 2);

  // battery max current in amps
  ui8_tx_buffer[5] = l2_vars.ui8_battery_max_current;

  // battery power
  ui8_tx_buffer[6] = l2_vars.ui8_target_max_battery_power;

  switch (ui8_master_comm_package_id)
  {
    case 0:
      // battery low voltage cut-off
      ui8_tx_buffer[7] = (uint8_t) (l2_vars.ui16_battery_low_voltage_cut_off_x10 & 0xff);
      ui8_tx_buffer[8] = (uint8_t) (l2_vars.ui16_battery_low_voltage_cut_off_x10 >> 8);
    break;

    case 1:
      // wheel perimeter
      ui8_tx_buffer[7] = (uint8_t) (l2_vars.ui16_wheel_perimeter & 0xff);
      ui8_tx_buffer[8] = (uint8_t) (l2_vars.ui16_wheel_perimeter >> 8);
    break;

    case 2:
      // wheel max speed
      ui8_tx_buffer[7] = l2_vars.ui8_wheel_max_speed;
    break;

    case 3:
      // bit 0: cruise control
      // bit 1: motor voltage type: 36V or 48V
      // bit 2: MOTOR_ASSISTANCE_CAN_START_WITHOUT_PEDAL_ROTATION
      ui8_tx_buffer[7] = ((l2_vars.ui8_cruise_control & 1) |
                         ((l2_vars.ui8_motor_type & 3) << 1) |
                          ((l2_vars.ui8_motor_assistance_startup_without_pedal_rotation & 1) << 3) |
                          ((l2_vars.ui8_temperature_limit_feature_enabled & 1) << 4));
      ui8_tx_buffer[8] = l2_vars.ui8_startup_motor_power_boost_state;
    break;

    case 4:
      // startup motor power boost
      ui8_tx_buffer[7] = l2_vars.ui8_startup_motor_power_boost_factor [((l2_vars.ui8_assist_level) - 1)];
      // startup motor power boost time
      ui8_tx_buffer[8] = l2_vars.ui8_startup_motor_power_boost_time;
    break;

    case 5:
      // startup motor power boost fade time
      ui8_tx_buffer[7] = l2_vars.ui8_startup_motor_power_boost_fade_time;
      // boost feature enabled
      ui8_tx_buffer[8] = (l2_vars.ui8_startup_motor_power_boost_feature_enabled & 1) ? 1 : 0;
    break;

    case 6:
      // motor over temperature min and max values to limit
      ui8_tx_buffer[7] = l2_vars.ui8_motor_temperature_min_value_to_limit;
      ui8_tx_buffer[8] = l2_vars.ui8_motor_temperature_max_value_to_limit;
    break;

    case 7:
      // offroad mode configuration
      ui8_tx_buffer[7] = ((l2_vars.ui8_offroad_feature_enabled & 1) |
                            ((l2_vars.ui8_offroad_enabled_on_startup & 1) << 1));
      ui8_tx_buffer[8] = l2_vars.ui8_offroad_speed_limit;
    break;

    case 8:
      // offroad mode power limit configuration
      ui8_tx_buffer[7] = l2_vars.ui8_offroad_power_limit_enabled & 1;
      ui8_tx_buffer[8] = l2_vars.ui8_offroad_power_limit_div25;
    break;

    default:
      ui8_lcd_variable_id = 0;
    break;
  }

  // prepare crc of the package
  ui16_crc_tx = 0xffff;
  for (ui8_i = 0; ui8_i <= 8; ui8_i++)
  {
    crc16 (ui8_tx_buffer[ui8_i], &ui16_crc_tx);
  }
  ui8_tx_buffer[9] = (uint8_t) (ui16_crc_tx & 0xff);
  ui8_tx_buffer[10] = (uint8_t) (ui16_crc_tx >> 8) & 0xff;

  // send the full package to UART
  // this 11 bytes takes about 13 ms to be sent. Measured on 2019.03.04.
  for (ui8_i = 0; ui8_i <= 10; ui8_i++)
  {
    usart1_send_byte_and_block(ui8_tx_buffer[ui8_i]);
  }

  // let's wait for 10 packages, seems that first ADC battery voltages have incorrect values
  ui8_m_usart1_received_first_package++;
  if(ui8_m_usart1_received_first_package > 10)
    ui8_m_usart1_received_first_package = 10;

  /************************************************************************************************/
  // now do all the calculations that must be done every 100ms


  l2_low_pass_filter_battery_voltage_current_power();
//  l2_low_pass_filter_pedal_torque_and_power();
//  l2_low_pass_filter_pedal_cadence();
  l2_calc_battery_voltage_soc();
//  l2_calc_odometer();
//  l2_calc_wh();
//  automatic_power_off_management();
  /************************************************************************************************/
}

uint8_t first_time_management(void)
{
  static uint8_t ui8_motor_controller_init = 1;
  uint8_t ui8_status = 0;

  // don't update LCD up to we get first communication package from the motor controller
  if(ui8_motor_controller_init &&
      (ui8_m_usart1_received_first_package < 10))
  {
    ui8_status = 1;
  }
  // this will be executed only 1 time at startup
  else if (ui8_motor_controller_init)
  {
    ui8_motor_controller_init = 0;

    // reset Wh value if battery voltage is over ui16_battery_voltage_reset_wh_counter_x10 (value configured by user)
    if (((uint32_t) l3_vars.ui16_adc_battery_voltage *
        ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000) > ((uint32_t) l3_vars.ui16_battery_voltage_reset_wh_counter_x10 * 1000))
    {
      l3_vars.ui32_wh_x10_offset = 0;
    }

    if (l3_vars.ui8_offroad_feature_enabled &&
      l3_vars.ui8_offroad_enabled_on_startup)
    {
      l3_vars.ui8_offroad_mode = 1;
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
    UG_PutString(12, 50, "ASSIST");
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

    l3_vars.ui8_assist_level++;

    if (l3_vars.ui8_assist_level > l3_vars.ui8_number_of_assist_levels)
      { l3_vars.ui8_assist_level = l3_vars.ui8_number_of_assist_levels; }
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

    if (l3_vars.ui8_assist_level > 0)
      l3_vars.ui8_assist_level--;
  }

  if ((l3_vars.ui8_assist_level != ui8_assist_level_previous) ||
      lcd_vars.ui32_main_screen_draw_static_info)
  {
    ui8_assist_level_previous = l3_vars.ui8_assist_level;

    assist_level.ui32_x_position = 25;
    assist_level.ui32_y_position = 84;
    assist_level.ui32_number = (uint32_t) l3_vars.ui8_assist_level;
    assist_level.ui8_refresh_all_digits = lcd_vars.ui32_main_screen_draw_static_info;
    lcd_print_number(&assist_level);
  }
}

l3_vars_t* get_l3_vars(void)
{
  return &l3_vars;
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
    l3_vars.ui32_wh_x10_offset = l3_vars.ui32_wh_x10;
    l3_vars.ui32_odometer_x10 += ((uint32_t) l3_vars.ui16_odometer_distance_x10);
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

void l2_low_pass_filter_battery_voltage_current_power(void)
{
  static uint32_t ui32_battery_voltage_accumulated_x10000 = 0;
  static uint16_t ui16_battery_current_accumulated_x5 = 0;

  // low pass filter battery voltage
  ui32_battery_voltage_accumulated_x10000 -= ui32_battery_voltage_accumulated_x10000 >> BATTERY_VOLTAGE_FILTER_COEFFICIENT;
  ui32_battery_voltage_accumulated_x10000 += (uint32_t) l2_vars.ui16_adc_battery_voltage * ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000;
  l2_vars.ui16_battery_voltage_filtered_x10 = ((uint32_t) (ui32_battery_voltage_accumulated_x10000 >> BATTERY_VOLTAGE_FILTER_COEFFICIENT)) / 1000;

  // low pass filter batery current
  ui16_battery_current_accumulated_x5 -= ui16_battery_current_accumulated_x5 >> BATTERY_CURRENT_FILTER_COEFFICIENT;
  ui16_battery_current_accumulated_x5 += (uint16_t) l2_vars.ui8_battery_current_x5;
  l2_vars.ui16_battery_current_filtered_x5 = ui16_battery_current_accumulated_x5 >> BATTERY_CURRENT_FILTER_COEFFICIENT;

  // battery power
  l2_vars.ui16_battery_power_filtered_x50 = l2_vars.ui16_battery_current_filtered_x5 * l2_vars.ui16_battery_voltage_filtered_x10;
  l2_vars.ui16_battery_power_filtered = l2_vars.ui16_battery_power_filtered_x50 / 50;

  // loose resolution under 200W
  if(l2_vars.ui16_battery_power_filtered < 200)
  {
    l2_vars.ui16_battery_power_filtered /= 10;
    l2_vars.ui16_battery_power_filtered *= 10;
  }
  // loose resolution under 400W
  else if(l2_vars.ui16_battery_power_filtered < 400)
  {
    l2_vars.ui16_battery_power_filtered /= 20;
    l2_vars.ui16_battery_power_filtered *= 20;
  }
  // loose resolution all other values
  else
  {
    l2_vars.ui16_battery_power_filtered /= 25;
    l2_vars.ui16_battery_power_filtered *= 25;
  }
}

void l2_low_pass_filter_pedal_torque_and_power(void)
{
//  static uint32_t ui32_pedal_torque_accumulated = 0;
//  static uint32_t ui32_pedal_power_accumulated = 0;
//
//  // low pass filter
//  ui32_pedal_torque_accumulated -= ui32_pedal_torque_accumulated >> PEDAL_TORQUE_FILTER_COEFFICIENT;
//  ui32_pedal_torque_accumulated += (uint32_t) uart_rx_vars.ui16_pedal_torque_x10 / 10;
//  processed_vars.ui16_pedal_torque_filtered = ((uint32_t) (ui32_pedal_torque_accumulated >> PEDAL_TORQUE_FILTER_COEFFICIENT));
//
//  // low pass filter
//  ui32_pedal_power_accumulated -= ui32_pedal_power_accumulated >> PEDAL_POWER_FILTER_COEFFICIENT;
//  ui32_pedal_power_accumulated += (uint32_t) uart_rx_vars.ui16_pedal_power_x10 / 10;
//  processed_vars.ui16_pedal_power_filtered = ((uint32_t) (ui32_pedal_power_accumulated >> PEDAL_POWER_FILTER_COEFFICIENT));
//
//  if(processed_vars.ui16_pedal_torque_filtered > 200)
//  {
//    processed_vars.ui16_pedal_torque_filtered /= 20;
//    processed_vars.ui16_pedal_torque_filtered *= 20;
//  }
//  else if(processed_vars.ui16_pedal_torque_filtered > 100)
//  {
//    processed_vars.ui16_pedal_torque_filtered /= 10;
//    processed_vars.ui16_pedal_torque_filtered *= 10;
//  }
//  else
//  {
//    // do nothing to original values
//  }
//
//  if(processed_vars.ui16_pedal_power_filtered > 500)
//  {
//    processed_vars.ui16_pedal_power_filtered /= 25;
//    processed_vars.ui16_pedal_power_filtered *= 25;
//  }
//  else if(processed_vars.ui16_pedal_power_filtered > 200)
//  {
//    processed_vars.ui16_pedal_power_filtered /= 20;
//    processed_vars.ui16_pedal_power_filtered *= 20;
//  }
//  else if(processed_vars.ui16_pedal_power_filtered > 10)
//  {
//    processed_vars.ui16_pedal_power_filtered /= 10;
//    processed_vars.ui16_pedal_power_filtered *= 10;
//  }
//  else
//  {
//    processed_vars.ui16_pedal_power_filtered = 0; // no point to show less than 10W
//  }
}

static void l2_low_pass_filter_pedal_cadence(void)
{
  static uint16_t ui16_pedal_cadence_accumulated = 0;

  // low pass filter
  ui16_pedal_cadence_accumulated -= (ui16_pedal_cadence_accumulated >> PEDAL_CADENCE_FILTER_COEFFICIENT);
  ui16_pedal_cadence_accumulated += (uint16_t) l2_vars.ui8_pedal_cadence;

  // consider the filtered value only for medium and high values of the unfiltered value
  if(l2_vars.ui8_pedal_cadence > 20)
  {
    l2_vars.ui8_pedal_cadence_filtered = (uint8_t) (ui16_pedal_cadence_accumulated >> PEDAL_CADENCE_FILTER_COEFFICIENT);
  }
  else
  {
    l2_vars.ui8_pedal_cadence_filtered = l2_vars.ui8_pedal_cadence;
  }
}

void l2_calc_wh(void)
{
//  static uint8_t ui8_1s_timmer_counter = 0;
//  uint32_t ui32_temp = 0;
//
//  if(ui16_battery_power_filtered_x50 > 0)
//  {
//    processed_vars.ui32_wh_sum_x5 += processed_vars.ui16_battery_power_filtered_x50 / 10;
//    processed_vars.ui32_wh_sum_counter++;
//  }
//
//  // calc at 1s rate
//  if(ui8_1s_timmer_counter < 10)
//  {
//    ui8_1s_timmer_counter = 0;
//
//    // avoid  zero divisison
//    if(processed_vars.ui32_wh_sum_counter != 0)
//    {
//      ui32_temp = processed_vars.ui32_wh_sum_counter / 36;
//      ui32_temp = (ui32_temp * (processed_vars.ui32_wh_sum_x5 / processed_vars.ui32_wh_sum_counter)) / 500;
//    }
//
//    processed_vars.ui32_wh_x10 = processed_vars.ui32_wh_x10_offset + ui32_temp;
//  }
//  ui8_1s_timmer_counter++;
}

void l2_calc_odometer(void)
{
//  uint32_t uint32_temp;
//  static uint8_t ui8_1s_timmer_counter;
//
//  // calc at 1s rate
//  if (ui8_1s_timmer_counter++ >= 10)
//  {
//    ui8_1s_timmer_counter = 0;
//
//    uint32_temp = (uart_rx_vars.ui32_wheel_speed_sensor_tick_counter - l3_vars.ui32_wheel_speed_sensor_tick_counter_offset)
//        * ((uint32_t) configuration_variables.ui16_wheel_perimeter);
//    // avoid division by 0
//    if (uint32_temp > 100000) { uint32_temp /= 100000;}  // milimmeters to 0.1kms
//    else { uint32_temp = 0; }
//
//    // now store the value on the global variable
//    configuration_variables.ui16_odometer_distance_x10 = (uint16_t) uint32_temp;
//  }
}

static void l2_automatic_power_off_management(void)
{
//  static uint8_t ui8_lcd_power_off_time_counter_minutes = 0;
//  static uint16_t ui16_lcd_power_off_time_counter = 0;
//
//  if(configuration_variables.ui8_lcd_power_off_time_minutes != 0)
//  {
//    // see if we should reset the automatic power off minutes counter
//    if ((l3_vars.ui16_wheel_speed_x10 > 0) ||   // wheel speed > 0
//        (l3_vars.ui8_battery_current_x5 > 0) || // battery current > 0
//        (l3_vars.ui8_braking) ||                // braking
//        buttons_get_events())                                 // any button active
//    {
//      ui16_lcd_power_off_time_counter = 0;
//      ui8_lcd_power_off_time_counter_minutes = 0;
//    }
//
//    // increment the automatic power off minutes counter
//    ui16_lcd_power_off_time_counter++;
//
//    // check if we should power off the LCD
//    if(ui16_lcd_power_off_time_counter >= (10 * 60)) // 1 minute passed
//    {
//      ui16_lcd_power_off_time_counter = 0;
//
//      ui8_lcd_power_off_time_counter_minutes++;
//      if(ui8_lcd_power_off_time_counter_minutes >= configuration_variables.ui8_lcd_power_off_time_minutes)
//      {
//        lcd_power_off(1);
//      }
//    }
//  }
//  else
//  {
//    ui16_lcd_power_off_time_counter = 0;
//    ui8_lcd_power_off_time_counter_minutes = 0;
//  }
}

void update_menu_flashing_state(void)
{
  static uint8_t ui8_lcd_menu_counter_100ms = 0;
  static uint8_t ui8_lcd_menu_counter_1000ms = 0;

  // ***************************************************************************************************
  // For flashing on menus, 0.5 seconds flash
  if (ui8_lcd_menu_flash_counter++ > 20)
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
  if (ui8_lcd_menu_counter_100ms++ > 4)
  {
    ui8_lcd_menu_counter_100ms = 0;
    ui8_lcd_menu_counter_100ms_state = 1;
  }

  // disable trigger signal
  if (lcd_vars.ui8_lcd_menu_counter_1000ms_trigger) { lcd_vars.ui8_lcd_menu_counter_1000ms_trigger = 0; }

  if(lcd_vars.ui8_lcd_menu_counter_1000ms_state)
  {
    if(ui8_lcd_menu_counter_1000ms++ > 32)
    {
      ui8_lcd_menu_counter_1000ms = 0;
      lcd_vars.ui8_lcd_menu_counter_1000ms_state = 0;
      lcd_vars.ui8_lcd_menu_counter_1000ms_trigger = 1;
    }
  }
  else
  {
    if(ui8_lcd_menu_counter_1000ms++ > 8)
    {
      ui8_lcd_menu_counter_1000ms = 0;
      lcd_vars.ui8_lcd_menu_counter_1000ms_state = 1;
      lcd_vars.ui8_lcd_menu_counter_1000ms_trigger = 2;
    }
  }
  // ***************************************************************************************************

//  // ***************************************************************************************************
//  // For flashing the temperature field when the current is being limited due to motor over temperature
//  // flash only if current is being limited: ui8_temperature_current_limiting_value != 255
//  if (l3_vars.ui8_temperature_current_limiting_value != 255)
//  {
//    if (ui8_lcd_menu_flash_state_temperature == 0) // state 0: disabled
//    {
//      if (ui16_lcd_menu_flash_counter_temperature > 0)
//      {
//        ui16_lcd_menu_flash_counter_temperature--;
//      }
//
//      if (ui16_lcd_menu_flash_counter_temperature == 0)
//      {
//        // if l3_vars.ui8_temperature_current_limiting_value == 0, flash quicker meaning motor is shutoff
//        if (l3_vars.ui8_temperature_current_limiting_value > 0)
//        {
//          ui16_lcd_menu_flash_counter_temperature = 50 + ((uint16_t) l3_vars.ui8_temperature_current_limiting_value);
//        }
//        else
//        {
//          ui16_lcd_menu_flash_counter_temperature = 25;
//        }
//
//        ui8_lcd_menu_flash_state_temperature = 1;
//      }
//    }
//
//    if (ui8_lcd_menu_flash_state_temperature == 1) // state 1: enabled
//    {
//      if (ui16_lcd_menu_flash_counter_temperature > 0)
//      {
//        ui16_lcd_menu_flash_counter_temperature--;
//      }
//
//      if (ui16_lcd_menu_flash_counter_temperature == 0)
//      {
//        ui16_lcd_menu_flash_counter_temperature = 25; // 0.25 second
//        ui8_lcd_menu_flash_state_temperature = 0;
//      }
//    }
//  }
//  else
//  {
//    ui8_lcd_menu_flash_state_temperature = 1;
//  }
//  // ***************************************************************************************************
}

void brake(void)
{
  static uint8_t ui8_braking_previous;
  uint32_t ui32_x1;
  uint32_t ui32_y1;
  uint32_t ui32_x2;
  uint32_t ui32_y2;

  // if previous state was disable, draw
  if((l3_vars.ui8_braking != ui8_braking_previous) ||
      (lcd_vars.ui32_main_screen_draw_static_info))
  {
    ui8_braking_previous = l3_vars.ui8_braking;

    if(l3_vars.ui8_braking)
    {
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&SMALL_TEXT_FONT);
      ui32_x1 = 178;
      ui32_y1 = 10;
      UG_PutString(ui32_x1, ui32_y1, "BRK");
    }
    else
    {
      // clear area
      // 5 leters
      ui32_x1 = 178;
      ui32_y1 = 10;
      ui32_x2 = ui32_x1 + ((3 * 10) + (3 * 1) + 1);
      ui32_y2 = ui32_y1 + 16;
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
      l3_vars.ui8_lights = 1;
    }
    else
    {
      ui8_lights_state = 0;
      lcd_lights_symbol = 0;
      l3_vars.ui8_lights = 0;
    }
  }

  if(ui8_lights_state == 0) { lcd_set_backlight_intensity (l3_vars.ui8_lcd_backlight_off_brightness); }
  else { lcd_set_backlight_intensity (l3_vars.ui8_lcd_backlight_on_brightness); }
}

void l2_calc_battery_voltage_soc(void)
{
  uint16_t ui16_fluctuate_battery_voltage_x10;

  // update battery level value only at every 100ms / 10 times per second and this helps to visual filter the fast changing values
  // calculate flutuate voltage, that depends on the current and battery pack resistance
  ui16_fluctuate_battery_voltage_x10 = (uint16_t) ((((uint32_t) l3_vars.ui16_battery_pack_resistance_x1000) * ((uint32_t) l2_vars.ui16_battery_current_filtered_x5)) / ((uint32_t) 500));
  // now add fluctuate voltage value
  l2_vars.ui16_battery_voltage_soc_x10 = l2_vars.ui16_battery_voltage_filtered_x10 + ui16_fluctuate_battery_voltage_x10;
}

void battery_soc_bar_clear(uint32_t ui32_bar_number)
{
  uint32_t ui32_x1, ui32_x2;
  uint32_t ui32_y1, ui32_y2;

  // the first nine bars share the same code
  if (ui32_bar_number < 10)
  {
    // draw the bar itself
    ui32_x1 = BATTERY_SOC_START_X + BATTERY_SOC_CONTOUR + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * (ui32_bar_number - 1));
    ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR;
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH;
    ui32_y2 = ui32_y1 + BATTERY_SOC_BAR_HEIGHT;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

    // draw bar contour
    if(ui32_bar_number < 9)
    {
      ui32_x1 = ui32_x2 + BATTERY_SOC_CONTOUR;
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_BLACK);
    }
    else
    {
      ui32_x1 = ui32_x2 + 1;
      ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR + (BATTERY_SOC_BAR_HEIGHT / 4);
      ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2);
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_BLACK);
    }
  }
  else
  {
    ui32_x1 = BATTERY_SOC_START_X + BATTERY_SOC_CONTOUR + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 9);
    ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR + (BATTERY_SOC_BAR_HEIGHT / 4);
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2);
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
    ui32_x1 = BATTERY_SOC_START_X + BATTERY_SOC_CONTOUR + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * (ui32_bar_number - 1));
    ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR;
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH;
    ui32_y2 = ui32_y1 + BATTERY_SOC_BAR_HEIGHT;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, ui16_color);

    if(ui32_bar_number < 9)
    {
      ui32_x1 = ui32_x2 + 1;
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_DIM_GRAY);
    }
    else
    {
      ui32_x1 = ui32_x2 + 1;
      ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR + (BATTERY_SOC_BAR_HEIGHT / 4);
      ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2);
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_DIM_GRAY);
    }
  }
  else
  {
    ui32_x1 = BATTERY_SOC_START_X + BATTERY_SOC_CONTOUR + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 9);
    ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR + (BATTERY_SOC_BAR_HEIGHT / 4);
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2);
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
    // first 9 bars
    ui32_x1 = BATTERY_SOC_START_X;
    ui32_y1 = BATTERY_SOC_START_Y;
    ui32_x2 = ui32_x1 + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR) * 9) + (BATTERY_SOC_CONTOUR * 2);
    ui32_y2 = ui32_y1 + BATTERY_SOC_BAR_HEIGHT + (BATTERY_SOC_CONTOUR * 2);
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

    // last small bar
    ui32_x1 = ui32_x2;
    ui32_y1 = BATTERY_SOC_START_Y + (BATTERY_SOC_BAR_HEIGHT / 4);
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH + (BATTERY_SOC_CONTOUR * 2);
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2) + (BATTERY_SOC_CONTOUR * 2);
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

    // now draw the empty battery symbol
    // first 9 bars
    ui32_x1 = BATTERY_SOC_START_X;
    ui32_y1 = BATTERY_SOC_START_Y;
    ui32_x2 = ui32_x1 + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 9) + (BATTERY_SOC_CONTOUR * 2) - 2;
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    // last bar
    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 4);
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1;
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2) + (BATTERY_SOC_CONTOUR * 2);
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1 - (BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1);
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 4);
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1 - (((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 9) + (BATTERY_SOC_CONTOUR * 2) - 2);
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 - (BATTERY_SOC_BAR_HEIGHT + BATTERY_SOC_CONTOUR);
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);
  }

  // update battery level value only at every 100ms / 10 times per second and this helps to visual filter the fast changing values
  if((ui8_timmer_counter++ >= 10) ||
      (lcd_vars.ui32_main_screen_draw_static_info))
  {
    ui8_timmer_counter = 0;

    // to keep same scale as voltage of x10
    ui32_battery_cells_number_x10 = (uint32_t) (l3_vars.ui8_battery_cells_number * 10);

    if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_90))) { ui32_battery_bar_number = 10; }
    else if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_80))) { ui32_battery_bar_number = 9; }
    else if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_70))) { ui32_battery_bar_number = 8; }
    else if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_60))) { ui32_battery_bar_number = 7; }
    else if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_50))) { ui32_battery_bar_number = 6; }
    else if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_40))) { ui32_battery_bar_number = 5; }
    else if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_30))) { ui32_battery_bar_number = 4; }
    else if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_20))) { ui32_battery_bar_number = 3; }
    else if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_10))) { ui32_battery_bar_number = 2; }
    else if(l3_vars.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_0))) { ui32_battery_bar_number = 1; }
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

    // draw SOC in percentage
    if((ui16_m_battery_soc_watts_hour != ui16_battery_soc_watts_hour_previous) ||
        (lcd_vars.ui32_main_screen_draw_static_info))
    {
      ui16_battery_soc_watts_hour_previous = ui16_m_battery_soc_watts_hour;
      soc.ui32_x_position = BATTERY_SOC_START_X + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 10) + (BATTERY_SOC_CONTOUR * 2) + 10;
      soc.ui32_y_position = 10;
      soc.ui32_number = ui16_m_battery_soc_watts_hour;
      soc.ui8_refresh_all_digits = lcd_vars.ui32_main_screen_draw_static_info;
      lcd_print_number(&soc);

      ui32_x1 = soc.ui32_x_final_position + 2;
      ui32_y1 = soc.ui32_y_final_position;
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&SMALL_TEXT_FONT);
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

  if (l3_vars.ui8_temperature_limit_feature_enabled)
  {
    if((l3_vars.ui8_motor_temperature != ui8_motor_temperature_previous) ||
        (lcd_vars.ui32_main_screen_draw_static_info))
    {
      ui8_motor_temperature_previous = l3_vars.ui8_motor_temperature;

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

      if(l3_vars.ui8_motor_temperature < 10)
      {
        ui32_x1 += 22;
        UG_PutString(ui32_x1, ui32_y1, itoa(l3_vars.ui8_motor_temperature));

        ui32_x1 += ((1 * 10) + (1 * 1) + 1);
        UG_PutString(ui32_x1, ui32_y1, &ui8_ascii_degree);
        ui32_x1 += 11;
        UG_PutString(ui32_x1, ui32_y1, "c");
      }
      else if(l3_vars.ui8_motor_temperature < 100)
      {
        ui32_x1 += 11;
        UG_PutString(ui32_x1, ui32_y1, itoa(l3_vars.ui8_motor_temperature));

        ui32_x1 += ((2 * 10) + (2 * 1) + 1);
        UG_PutString(ui32_x1, ui32_y1, &ui8_ascii_degree);
        ui32_x1 += 11;
        UG_PutString(ui32_x1, ui32_y1, "c");
      }
      else
      {
        UG_PutString(ui32_x1, ui32_y1, itoa(l3_vars.ui8_motor_temperature));

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
      (p_rtc_time->ui8_minutes != p_rtc_time_previous->ui8_minutes) ||
      lcd_vars.ui32_main_screen_draw_static_info)
  {
    p_rtc_time_previous->ui8_hours = p_rtc_time->ui8_hours;
    p_rtc_time_previous->ui8_minutes = p_rtc_time->ui8_minutes;

    // print hours number
    ui32_x_position = DISPLAY_WIDTH - 1 - hours.font->char_width - (5 * hours.font->char_width) + (5 * 1);
    ui32_y_position = 6;
    hours.ui32_x_position = ui32_x_position;
    hours.ui32_y_position = ui32_y_position;
    hours.ui32_number = p_rtc_time->ui8_hours;
    hours.ui8_refresh_all_digits = lcd_vars.ui32_main_screen_draw_static_info;
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
    minutes.ui8_refresh_all_digits = lcd_vars.ui32_main_screen_draw_static_info;
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
    .ui32_x_position = 191,
    .ui32_y_position = 191,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 4,
    .ui8_left_zero_paddig = 0,
    .ui32_number = 0,
    .ui8_refresh_all_digits = 1
  };

  if(lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(C_GRAY);
    UG_FontSelect(&FONT_10X16);
    UG_PutString(183, 164, "motor power");
  }

  if(!lcd_vars.ui8_lcd_menu_max_power)
  {
    _ui16_battery_power_filtered = l3_vars.ui16_battery_power_filtered;

    if((_ui16_battery_power_filtered != ui16_battery_power_filtered_previous) ||
        lcd_vars.ui32_main_screen_draw_static_info ||
        ui8_target_max_battery_power_state == 0)
    {
      ui16_battery_power_filtered_previous = _ui16_battery_power_filtered;
      ui8_target_max_battery_power_state = 1;

      if (_ui16_battery_power_filtered > 9999) { _ui16_battery_power_filtered = 9999; }

      power_number.ui32_number = _ui16_battery_power_filtered;
      power_number.ui8_refresh_all_digits = lcd_vars.ui32_main_screen_draw_static_info;
      lcd_print_number(&power_number);
      power_number.ui8_refresh_all_digits = 0;
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
      power_number.ui8_refresh_all_digits = 1;

      // save the updated variables on EEPROM
      eeprom_write_variables();

      buttons_clear_all_events();
      return;
    }

    if(buttons_get_up_click_event())
    {
      buttons_clear_all_events();

      if(l3_vars.ui8_target_max_battery_power < 10)
      {
        l3_vars.ui8_target_max_battery_power++;
      }
      else
      {
        l3_vars.ui8_target_max_battery_power += 2;
      }

      // limit to 100 * 25 = 2500 Watts
      if(l3_vars.ui8_target_max_battery_power > 100) { l3_vars.ui8_target_max_battery_power = 100; }
    }

    if(buttons_get_down_click_event ())
    {
      buttons_clear_all_events();

      if(l3_vars.ui8_target_max_battery_power == 0)
      {

      }
      else if(l3_vars.ui8_target_max_battery_power <= 10)
      {
        l3_vars.ui8_target_max_battery_power--;
      }
      else
      {
        l3_vars.ui8_target_max_battery_power -= 2;
      }
    }

    if(ui8_lcd_menu_flash_state)
    {
      if(ui8_target_max_battery_power_state == 1)
      {
        ui8_target_max_battery_power_state = 0;

        // clear area
        power_number.ui8_clean_area_all_digits = 1;
        lcd_print_number(&power_number);
        power_number.ui8_clean_area_all_digits = 0;
      }
    }
    else
    {
      if(ui8_target_max_battery_power_state == 0)
      {
        ui8_target_max_battery_power_state = 1;

        ui16_target_max_power = l3_vars.ui8_target_max_battery_power * 25;

        power_number.ui8_refresh_all_digits = 1;
        power_number.ui32_number = ui16_target_max_power;
        lcd_print_number(&power_number);

        l3_vars.ui8_target_max_battery_power = ui16_target_max_power / 25;
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
    UG_PutString(257, 50 , "KM/H");

    // print dot
    UG_FillCircle(196, 123, 2, C_WHITE);
  }

  if ((l3_vars.ui16_wheel_speed_x10 != ui16_wheel_x10_speed_previous) ||
      lcd_vars.ui32_main_screen_draw_static_info)
  {
    ui16_wheel_x10_speed_previous = l3_vars.ui16_wheel_speed_x10;

    ui32_x_position = 126;
    ui32_y_position = 84;

    wheel_speed_integer.ui32_x_position = ui32_x_position;
    wheel_speed_integer.ui32_y_position = ui32_y_position;
    wheel_speed_integer.ui32_number = (uint32_t) (l3_vars.ui16_wheel_speed_x10 / 10);
    wheel_speed_integer.ui8_refresh_all_digits = lcd_vars.ui32_main_screen_draw_static_info;
    lcd_print_number(&wheel_speed_integer);

    // accounting for dot: 10 px
    ui32_x_position = wheel_speed_integer.ui32_x_final_position + 10;

    ui32_y_position = ui32_y_position + 10;

    wheel_speed_decimal.ui32_x_position = ui32_x_position;
    wheel_speed_decimal.ui32_y_position = ui32_y_position;
    wheel_speed_decimal.ui32_number = (uint32_t) (l3_vars.ui16_wheel_speed_x10 % 10);
    wheel_speed_decimal.ui8_refresh_all_digits = lcd_vars.ui32_main_screen_draw_static_info;
    lcd_print_number(&wheel_speed_decimal);
  }
}

void calc_battery_soc_watts_hour(void)
{
  uint32_t ui32_temp;

  ui32_temp = l3_vars.ui32_wh_x10 * 100;

  if (l3_vars.ui32_wh_x10_100_percent > 0)
  {
    ui32_temp /= l3_vars.ui32_wh_x10_100_percent;
  }
  else
  {
    ui32_temp = 0;
  }

  // 100% - current SOC or just current SOC
  if (l3_vars.ui8_battery_soc_increment_decrement)
  {
    if (ui32_temp > 100)
      ui32_temp = 100;

    ui16_m_battery_soc_watts_hour = 100 - ui32_temp;
  }
  else
  {
    ui16_m_battery_soc_watts_hour = ui32_temp;
  }
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
    if(((ui8_digits_array[ui8_digit_inverse_counter] != number->ui8_previous_digits_array[ui8_digit_inverse_counter]) ||
        (number->ui8_refresh_all_digits)) &&
        (!number->ui8_clean_area_all_digits))
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
    else if(((ui8_digits_array[ui8_digit_inverse_counter] == 0) &&  // if is a 0
        (ui8_digit_inverse_counter > ui8_digit_number_start) && // if is a digit at left from the first digit
        (!number->ui8_left_zero_paddig)) || // if we NOT want to print a 0 at left
        (number->ui8_clean_area_all_digits)) // we want to clean, so print a " "
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

void copy_layer_2_layer_3_vars(void)
{
  l3_vars.ui16_adc_battery_voltage = l2_vars.ui16_adc_battery_voltage;
  l3_vars.ui8_battery_current_x5 = l2_vars.ui8_battery_current_x5;
  l3_vars.ui8_throttle = l2_vars.ui8_throttle;
  l3_vars.ui8_adc_pedal_torque_sensor = l2_vars.ui8_adc_pedal_torque_sensor;
  l3_vars.ui8_pedal_torque_sensor = l2_vars.ui8_pedal_torque_sensor;
  l3_vars.ui8_pedal_human_power = l2_vars.ui8_pedal_human_power;
  l3_vars.ui8_duty_cycle = l2_vars.ui8_duty_cycle;
  l3_vars.ui8_error_states = l2_vars.ui8_error_states;
  l3_vars.ui16_wheel_speed_x10 = l2_vars.ui16_wheel_speed_x10;
  l3_vars.ui8_pedal_cadence = l2_vars.ui8_pedal_cadence;
  l3_vars.ui16_motor_speed_erps = l2_vars.ui16_motor_speed_erps;
  l3_vars.ui8_temperature_current_limiting_value = l2_vars.ui8_temperature_current_limiting_value;
  l3_vars.ui8_motor_temperature = l2_vars.ui8_motor_temperature;
  l3_vars.ui32_wheel_speed_sensor_tick_counter = l2_vars.ui32_wheel_speed_sensor_tick_counter;
  l3_vars.ui16_pedal_power_x10 = l2_vars.ui16_pedal_power_x10;
  l3_vars.ui16_battery_voltage_filtered_x10 = l2_vars.ui16_battery_voltage_filtered_x10;
  l3_vars.ui16_battery_current_filtered_x5 = l2_vars.ui16_battery_current_filtered_x5;
  l3_vars.ui16_battery_power_filtered_x50 = l2_vars.ui16_battery_power_filtered_x50;
  l3_vars.ui16_battery_power_filtered = l2_vars.ui16_battery_power_filtered;
  l3_vars.ui16_pedal_torque_filtered = l2_vars.ui16_pedal_torque_filtered;
  l3_vars.ui16_pedal_power_filtered = l2_vars.ui16_pedal_power_filtered;
  l3_vars.ui8_pedal_cadence_filtered = l2_vars.ui8_pedal_cadence_filtered;
  l3_vars.ui16_battery_voltage_soc_x10 = l2_vars.ui16_battery_voltage_soc_x10;
  l3_vars.ui32_wh_sum_x5 = l2_vars.ui32_wh_sum_x5;
  l3_vars.ui32_wh_sum_counter = l2_vars.ui32_wh_sum_counter;
  l3_vars.ui32_wh_x10 = l2_vars.ui32_wh_x10;
  l3_vars.ui8_braking = l2_vars.ui8_braking;

  l2_vars.ui8_assist_level = l3_vars.ui8_assist_level;
  l2_vars.ui8_assist_level_factor[0] = l3_vars.ui8_assist_level_factor[0];
  l2_vars.ui8_assist_level_factor[1] = l3_vars.ui8_assist_level_factor[1];
  l2_vars.ui8_assist_level_factor[2] = l3_vars.ui8_assist_level_factor[2];
  l2_vars.ui8_assist_level_factor[3] = l3_vars.ui8_assist_level_factor[3];
  l2_vars.ui8_assist_level_factor[4] = l3_vars.ui8_assist_level_factor[4];
  l2_vars.ui8_assist_level_factor[5] = l3_vars.ui8_assist_level_factor[5];
  l2_vars.ui8_assist_level_factor[6] = l3_vars.ui8_assist_level_factor[6];
  l2_vars.ui8_assist_level_factor[7] = l3_vars.ui8_assist_level_factor[7];
  l2_vars.ui8_assist_level_factor[8] = l3_vars.ui8_assist_level_factor[8];
  l2_vars.ui8_lights = l3_vars.ui8_lights;
  l2_vars.ui8_walk_assist_level = l3_vars.ui8_walk_assist_level;
  l2_vars.ui8_offroad_mode = l3_vars.ui8_offroad_mode;
  l2_vars.ui8_battery_max_current = l3_vars.ui8_battery_max_current;
  l2_vars.ui8_target_max_battery_power = l3_vars.ui8_target_max_battery_power;
  l2_vars.ui16_battery_low_voltage_cut_off_x10 = l3_vars.ui16_battery_low_voltage_cut_off_x10;
  l2_vars.ui16_wheel_perimeter = l3_vars.ui16_wheel_perimeter;
  l2_vars.ui8_wheel_max_speed = l3_vars.ui8_wheel_max_speed;
  l2_vars.ui8_cruise_control = l3_vars.ui8_cruise_control;
  l2_vars.ui8_motor_type = l3_vars.ui8_motor_type;
  l2_vars.ui8_motor_assistance_startup_without_pedal_rotation = l3_vars.ui8_motor_assistance_startup_without_pedal_rotation;
  l2_vars.ui8_temperature_limit_feature_enabled = l3_vars.ui8_temperature_limit_feature_enabled;
  l2_vars.ui8_startup_motor_power_boost_state = l3_vars.ui8_startup_motor_power_boost_state;
  l2_vars.ui8_startup_motor_power_boost_time = l3_vars.ui8_startup_motor_power_boost_time;
  l2_vars.ui8_startup_motor_power_boost_factor[1] = l3_vars.ui8_startup_motor_power_boost_factor[0];
  l2_vars.ui8_startup_motor_power_boost_factor[2] = l3_vars.ui8_startup_motor_power_boost_factor[1];
  l2_vars.ui8_startup_motor_power_boost_factor[3] = l3_vars.ui8_startup_motor_power_boost_factor[2];
  l2_vars.ui8_startup_motor_power_boost_factor[4] = l3_vars.ui8_startup_motor_power_boost_factor[3];
  l2_vars.ui8_startup_motor_power_boost_factor[5] = l3_vars.ui8_startup_motor_power_boost_factor[4];
  l2_vars.ui8_startup_motor_power_boost_factor[6] = l3_vars.ui8_startup_motor_power_boost_factor[5];
  l2_vars.ui8_startup_motor_power_boost_factor[7] = l3_vars.ui8_startup_motor_power_boost_factor[7];
  l2_vars.ui8_startup_motor_power_boost_factor[8] = l3_vars.ui8_startup_motor_power_boost_factor[8];
  l2_vars.ui8_startup_motor_power_boost_factor[9] = l3_vars.ui8_startup_motor_power_boost_factor[9];
  l2_vars.ui8_startup_motor_power_boost_fade_time = l3_vars.ui8_startup_motor_power_boost_fade_time;
  l2_vars.ui8_startup_motor_power_boost_feature_enabled = l3_vars.ui8_startup_motor_power_boost_feature_enabled;
  l2_vars.ui8_motor_temperature_min_value_to_limit = l3_vars.ui8_motor_temperature_min_value_to_limit;
  l2_vars.ui8_motor_temperature_max_value_to_limit = l3_vars.ui8_motor_temperature_max_value_to_limit;
  l2_vars.ui8_offroad_feature_enabled = l3_vars.ui8_offroad_feature_enabled;
  l2_vars.ui8_offroad_enabled_on_startup = l3_vars.ui8_offroad_enabled_on_startup;
  l2_vars.ui8_offroad_speed_limit = l3_vars.ui8_offroad_speed_limit;
  l2_vars.ui8_offroad_power_limit_enabled = l3_vars.ui8_offroad_power_limit_enabled;
  l2_vars.ui8_offroad_power_limit_div25 = l3_vars.ui8_offroad_power_limit_div25;
}

lcd_vars_t* get_lcd_vars(void)
{
  return &lcd_vars;
}
