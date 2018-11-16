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

typedef struct _motor_controller_data
{
  uint16_t ui16_adc_battery_voltage;
  uint8_t ui8_battery_current_x5;
  uint8_t ui8_motor_controller_state_1;
  uint8_t ui8_adc_throttle;
  uint8_t ui8_throttle;
  uint8_t ui8_adc_pedal_torque_sensor;
  uint8_t ui8_pedal_torque_sensor;
  uint8_t ui8_pedal_human_power;
  uint8_t ui8_duty_cycle;
  uint8_t ui8_error_states;
  uint16_t ui16_wheel_speed_x10;
  uint8_t ui8_motor_controller_state_2;
  uint8_t ui8_braking;
  uint8_t ui8_pedal_cadence;
  uint8_t ui8_lights;
  uint8_t ui8_walk_assist_level;
  uint8_t ui8_offroad_mode;
  uint16_t ui16_motor_speed_erps;
  uint8_t ui8_foc_angle;
  uint8_t ui8_temperature_current_limiting_value;
  uint8_t ui8_motor_temperature;
  uint32_t ui32_wheel_speed_sensor_tick_counter;
  uint32_t ui32_wheel_speed_sensor_tick_counter_offset;
  uint16_t ui16_pedal_torque_x10;
  uint16_t ui16_pedal_power_x10;
} struct_motor_controller_data;

typedef struct _configuration_variables
{
  uint8_t ui8_assist_level;
  uint8_t ui8_number_of_assist_levels;
  uint16_t ui16_wheel_perimeter;
  uint8_t ui8_wheel_max_speed;
  uint8_t ui8_units_type;
  uint32_t ui32_wh_x10_offset;
  uint32_t ui32_wh_x10_100_percent;
  uint8_t ui8_show_numeric_battery_soc;
  uint8_t ui8_odometer_field_state;
  uint8_t ui8_odometer_sub_field_state;
  uint8_t ui8_odometer_show_field_number;
  uint8_t ui8_target_max_battery_power;
  uint8_t ui8_battery_cells_number;
  uint8_t ui8_battery_max_current;
  uint16_t ui16_battery_low_voltage_cut_off_x10;
  uint16_t ui16_battery_voltage_reset_wh_counter_x10;
  uint16_t ui16_battery_pack_resistance_x1000;
  uint8_t ui8_motor_type;
  uint8_t ui8_motor_assistance_startup_without_pedal_rotation;
  uint8_t ui8_cruise_control;
  uint8_t ui8_assist_level_factor [9];
  uint8_t ui8_startup_motor_power_boost_feature_enabled;
  uint8_t ui8_startup_motor_power_boost_state;
  uint8_t ui8_startup_motor_power_boost_time;
  uint8_t ui8_startup_motor_power_boost_fade_time;
  uint8_t ui8_startup_motor_power_boost_factor [9];
  uint16_t ui16_adc_motor_temperature_10b;
  uint8_t ui8_temperature_limit_feature_enabled;
  uint8_t ui8_motor_temperature_min_value_to_limit;
  uint8_t ui8_motor_temperature_max_value_to_limit;
  uint8_t ui8_temperature_field_config;
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
} struct_configuration_variables;

void lcd_init(void);
void lcd_clock(void);
void lcd_draw_main_menu_mask(void);
struct_motor_controller_data* lcd_get_motor_controller_data (void);
struct_configuration_variables* get_configuration_variables (void);

#endif /* LCD_H_ */
