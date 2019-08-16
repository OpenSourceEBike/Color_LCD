/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _EEPROM_H_
#define _EEPROM_H_

#include "main.h"
#include "lcd_configurations.h"

#define ADDRESS_KEY 0
#define KEY 5

typedef struct eeprom_data
{
  uint8_t ui8_assist_level;
  uint16_t ui16_wheel_perimeter;
  uint16_t ui16_wheel_perimeter_imperial_x10;
  uint8_t ui8_wheel_max_speed;
  uint8_t ui8_wheel_max_speed_imperial;
  uint8_t ui8_units_type;
  uint32_t ui32_wh_x10_offset;
  uint32_t ui32_wh_x10_100_percent;
  uint8_t ui8_battery_soc_enable;
  uint8_t ui8_battery_max_current;
  uint8_t ui8_ramp_up_amps_per_second_x10;
  uint8_t ui8_battery_cells_number;
  uint16_t ui16_battery_low_voltage_cut_off_x10;
  uint8_t ui8_motor_type;
  uint8_t ui8_motor_assistance_startup_without_pedal_rotation;
  uint8_t ui8_assist_level_factor[10];
  uint8_t ui8_number_of_assist_levels;
  uint8_t ui8_startup_motor_power_boost_feature_enabled;
  uint8_t ui8_startup_motor_power_boost_state;
  uint8_t ui8_startup_motor_power_boost_factor[10];
  uint8_t ui8_startup_motor_power_boost_time;
  uint8_t ui8_startup_motor_power_boost_fade_time;
  uint8_t ui8_temperature_limit_feature_enabled;
  uint8_t ui8_motor_temperature_min_value_to_limit;
  uint8_t ui8_motor_temperature_min_value_to_limit_imperial;
  uint8_t ui8_motor_temperature_max_value_to_limit;
  uint8_t ui8_motor_temperature_max_value_to_limit_imperial;
  uint16_t ui16_battery_voltage_reset_wh_counter_x10;
  uint8_t ui8_lcd_power_off_time_minutes;
  uint8_t ui8_lcd_backlight_on_brightness;
  uint8_t ui8_lcd_backlight_off_brightness;
  uint16_t ui16_battery_pack_resistance_x1000;
  uint8_t ui8_offroad_feature_enabled;
  uint8_t ui8_offroad_enabled_on_startup;
  uint8_t ui8_offroad_speed_limit;
  uint8_t ui8_offroad_power_limit_enabled;
  uint8_t ui8_offroad_power_limit_div25;
  uint32_t ui32_odometer_x10;
  uint8_t ui8_walk_assist_feature_enabled;
  uint8_t ui8_walk_assist_level_factor[10];
  lcd_configurations_menu_t lcd_configurations_menu;
  graphs_id_t graph_id;
  uint8_t ui8_battery_soc_increment_decrement;
  uint8_t ui8_buttons_up_down_invert;
} eeprom_data_t;

void eeprom_init(void);
void eeprom_init_variables(void);
void eeprom_write_variables(void);
void eeprom_init_defaults(void);

#endif /* _EEPROM_H_ */
