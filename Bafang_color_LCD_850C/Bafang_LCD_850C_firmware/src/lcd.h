/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include "ugui/ugui.h"
#include "usart1.h"

#define MAX_NUMBER_DIGITS 5 // max of 5 digits: 1234.5 or 12345

typedef struct l2_vars_struct
{
  uint16_t ui16_adc_battery_voltage;
  uint8_t ui8_battery_current_x5;
  uint8_t ui8_adc_throttle;
  uint8_t ui8_throttle;
  uint8_t ui8_adc_pedal_torque_sensor;
  uint8_t ui8_pedal_torque_sensor;
  uint8_t ui8_pedal_human_power;
  uint8_t ui8_duty_cycle;
  uint8_t ui8_error_states;
  uint16_t ui16_wheel_speed_x10;
  uint8_t ui8_pedal_cadence;
  uint16_t ui16_motor_speed_erps;
  uint8_t ui8_foc_angle;
  uint8_t ui8_temperature_current_limiting_value;
  uint8_t ui8_motor_temperature;
  uint32_t ui32_wheel_speed_sensor_tick_counter;
  uint16_t ui16_pedal_torque_x10;
  uint16_t ui16_pedal_power_x10;
  uint16_t ui16_battery_voltage_filtered_x10;
  uint16_t ui16_battery_current_filtered_x5;
  uint16_t ui16_battery_power_filtered_x50;
  uint16_t ui16_battery_power_filtered;
  uint16_t ui16_pedal_torque_filtered;
  uint16_t ui16_pedal_power_filtered;
  uint8_t ui8_pedal_cadence_filtered;
  uint16_t ui16_battery_voltage_soc_x10;
  uint32_t ui32_wh_sum_x5;
  uint32_t ui32_wh_sum_counter;
  uint32_t ui32_wh_x10;

  uint8_t ui8_assist_level;
  uint8_t ui8_number_of_assist_levels;
  uint16_t ui16_wheel_perimeter;
  uint8_t ui8_wheel_max_speed;
  uint8_t ui8_units_type;
  uint32_t ui32_wh_x10_offset;
  uint32_t ui32_wh_x10_100_percent;
  uint8_t ui8_battery_soc_enable;
  uint8_t ui8_battery_soc_increment_decrement;
  uint8_t ui8_target_max_battery_power;
  uint8_t ui8_battery_cells_number;
  uint8_t ui8_battery_max_current;
  uint8_t ui8_ramp_up_amps_per_second_x10;
  uint16_t ui16_battery_low_voltage_cut_off_x10;
  uint16_t ui16_battery_voltage_reset_wh_counter_x10;
  uint16_t ui16_battery_pack_resistance_x1000;
  uint8_t ui8_motor_type;
  uint8_t ui8_motor_assistance_startup_without_pedal_rotation;
  uint8_t ui8_assist_level_factor[10];
  uint8_t ui8_walk_assist_feature_enabled;
  uint8_t ui8_walk_assist_level_factor[10];
  uint8_t ui8_startup_motor_power_boost_feature_enabled;
  uint8_t ui8_startup_motor_power_boost_state;
  uint8_t ui8_startup_motor_power_boost_time;
  uint8_t ui8_startup_motor_power_boost_fade_time;
  uint8_t ui8_startup_motor_power_boost_factor[10];
  uint8_t ui8_temperature_limit_feature_enabled;
  uint8_t ui8_motor_temperature_min_value_to_limit;
  uint8_t ui8_motor_temperature_max_value_to_limit;
  uint8_t ui8_lcd_power_off_time_minutes;
  uint8_t ui8_lcd_backlight_on_brightness;
  uint8_t ui8_lcd_backlight_off_brightness;
  uint8_t ui8_offroad_feature_enabled;
  uint8_t ui8_offroad_enabled_on_startup;
  uint8_t ui8_offroad_speed_limit;
  uint8_t ui8_offroad_power_limit_enabled;
  uint8_t ui8_offroad_power_limit_div25;
  uint16_t ui16_odometer_distance_x10;
  uint32_t ui32_odometer_x10;

  uint8_t ui8_lights;
  uint8_t ui8_braking;
  uint8_t ui8_walk_assist;
  uint8_t ui8_offroad_mode;
} l2_vars_t;

typedef struct l3_vars_struct
{
  uint16_t ui16_adc_battery_voltage;
  uint8_t ui8_battery_current_x5;
  uint8_t ui8_adc_throttle;
  uint8_t ui8_throttle;
  uint8_t ui8_adc_pedal_torque_sensor;
  uint8_t ui8_pedal_torque_sensor;
  uint8_t ui8_pedal_human_power;
  uint8_t ui8_duty_cycle;
  uint8_t ui8_error_states;
  uint16_t ui16_wheel_speed_x10;
  uint8_t ui8_pedal_cadence;
  uint16_t ui16_motor_speed_erps;
  uint8_t ui8_foc_angle;
  uint8_t ui8_temperature_current_limiting_value;
  uint8_t ui8_motor_temperature;
  uint32_t ui32_wheel_speed_sensor_tick_counter;
  uint32_t ui32_wheel_speed_sensor_tick_counter_offset;
  uint16_t ui16_pedal_torque_x10;
  uint16_t ui16_pedal_power_x10;
  uint16_t ui16_battery_voltage_filtered_x10;
  uint16_t ui16_battery_current_filtered_x5;
  uint16_t ui16_battery_power_filtered_x50;
  uint16_t ui16_battery_power_filtered;
  uint16_t ui16_pedal_torque_filtered;
  uint16_t ui16_pedal_power_filtered;
  uint8_t ui8_pedal_cadence_filtered;
  uint16_t ui16_battery_voltage_soc_x10;
  uint32_t ui32_wh_sum_x5;
  uint32_t ui32_wh_sum_counter;
  uint32_t ui32_wh_x10;

  uint8_t ui8_assist_level;
  uint8_t ui8_number_of_assist_levels;
  uint16_t ui16_wheel_perimeter;
  uint8_t ui8_wheel_max_speed;
  uint8_t ui8_units_type;
  uint32_t ui32_wh_x10_offset;
  uint32_t ui32_wh_x10_100_percent;
  uint8_t ui8_battery_soc_enable;
  uint8_t ui8_battery_soc_increment_decrement;
  uint8_t ui8_target_max_battery_power;
  uint8_t ui8_battery_cells_number;
  uint8_t ui8_battery_max_current;
  uint8_t ui8_ramp_up_amps_per_second_x10;
  uint16_t ui16_battery_low_voltage_cut_off_x10;
  uint16_t ui16_battery_voltage_reset_wh_counter_x10;
  uint16_t ui16_battery_pack_resistance_x1000;
  uint8_t ui8_motor_type;
  uint8_t ui8_motor_assistance_startup_without_pedal_rotation;
  uint8_t ui8_assist_level_factor[10];
  uint8_t ui8_walk_assist_feature_enabled;
  uint8_t ui8_walk_assist_level_factor[10];
  uint8_t ui8_startup_motor_power_boost_feature_enabled;
  uint8_t ui8_startup_motor_power_boost_state;
  uint8_t ui8_startup_motor_power_boost_time;
  uint8_t ui8_startup_motor_power_boost_fade_time;
  uint8_t ui8_startup_motor_power_boost_factor[10];
  uint8_t ui8_temperature_limit_feature_enabled;
  uint8_t ui8_motor_temperature_min_value_to_limit;
  uint8_t ui8_motor_temperature_max_value_to_limit;
  uint8_t ui8_lcd_power_off_time_minutes;
  uint8_t ui8_lcd_backlight_on_brightness;
  uint8_t ui8_lcd_backlight_off_brightness;
  uint8_t ui8_offroad_feature_enabled;
  uint8_t ui8_offroad_enabled_on_startup;
  uint8_t ui8_offroad_speed_limit;
  uint8_t ui8_offroad_power_limit_enabled;
  uint8_t ui8_offroad_power_limit_div25;
  uint16_t ui16_odometer_distance_x10;
  uint32_t ui32_odometer_x10;
  uint16_t ui16_distance_since_power_on_x10;
  uint32_t ui32_trip_x10;

  uint8_t ui8_lights;
  uint8_t ui8_braking;
  uint8_t ui8_walk_assist;
  uint8_t ui8_offroad_mode;
} l3_vars_t;

typedef enum
{
  LCD_SCREEN_MAIN = 1,
  LCD_SCREEN_CONFIGURATIONS = 2
} lcd_screen_states_t;

typedef struct lcd_vars_struct
{
  uint32_t ui32_main_screen_draw_static_info;
  lcd_screen_states_t lcd_screen_state;
  uint8_t ui8_lcd_menu_counter_1000ms_state;
  uint8_t ui8_lcd_menu_counter_1000ms_trigger;
  uint8_t ui8_lcd_menu_max_power;
} lcd_vars_t;

typedef struct _print_number
{
  const UG_FONT* font;
  UG_COLOR fore_color;
  UG_COLOR back_color;
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  uint32_t ui32_x_final_position;
  uint32_t ui32_y_final_position;
  uint8_t ui8_previous_digits_array[MAX_NUMBER_DIGITS];
  uint8_t ui8_field_number_of_digits;
  uint8_t ui8_left_zero_paddig;
  uint8_t ui8_left_paddig;
  uint8_t ui8_refresh_all_digits;
  uint32_t ui32_number;
  uint8_t ui8_digit_number_start_previous;
  uint8_t ui8_clean_area_all_digits;
  uint8_t ui8_decimal_digits;
} print_number_t;

extern volatile uint32_t ui32_g_layer_2_can_execute;
extern volatile uint8_t ui8_g_usart1_tx_buffer[UART_NUMBER_DATA_BYTES_TO_SEND + 3];
extern volatile uint32_t ui32_g_graphs_data_array_over_255;

void lcd_init(void);
void lcd_clock(void);
void layer_2(void);
l3_vars_t* get_l3_vars(void);
lcd_vars_t* get_lcd_vars(void);
void lcd_print_number(print_number_t* number);
void lcd_draw_main_menu_mask(void);

#endif /* LCD_H_ */
