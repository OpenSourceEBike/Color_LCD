/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stdio.h"
#include <string.h>
#include "eeprom.h"
#include "eeprom_hw.h"
#include "main.h"
#include "mainscreen.h"
//#include "lcd_configurations.h"

static eeprom_data_t m_eeprom_data;

// get rid of some copypasta with this little wrapper for copying arrays between structs
#define COPY_ARRAY(dest, src, field) memcpy((dest)->field, (src)->field, sizeof((dest)->field))

const eeprom_data_t m_eeprom_data_defaults = { .eeprom_version = EEPROM_VERSION,
		.ui8_assist_level =
		DEFAULT_VALUE_ASSIST_LEVEL, .ui16_wheel_perimeter =
		DEFAULT_VALUE_WHEEL_PERIMETER, .ui8_wheel_max_speed =
		DEFAULT_VALUE_WHEEL_MAX_SPEED, .ui8_units_type =
		DEFAULT_VALUE_UNITS_TYPE, .ui32_wh_x10_offset =
		DEFAULT_VALUE_WH_X10_OFFSET, .ui32_wh_x10_100_percent =
		DEFAULT_VALUE_HW_X10_100_PERCENT, .ui8_battery_soc_enable =
		DEAFULT_VALUE_SHOW_NUMERIC_BATTERY_SOC, .ui8_battery_max_current =
		DEFAULT_VALUE_BATTERY_MAX_CURRENT, .ui8_ramp_up_amps_per_second_x10 =
		DEFAULT_VALUE_RAMP_UP_AMPS_PER_SECOND_X10, .ui8_battery_cells_number =
		DEFAULT_VALUE_BATTERY_CELLS_NUMBER,
		.ui16_battery_low_voltage_cut_off_x10 =
		DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10, .ui8_motor_type =
		DEFAULT_VALUE_MOTOR_TYPE,
		.ui8_motor_assistance_startup_without_pedal_rotation =
		DEFAULT_VALUE_MOTOR_ASSISTANCE_WITHOUT_PEDAL_ROTATION,
		.ui8_assist_level_factor = {
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_1,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_2,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_3,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_4,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_5,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_6,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_7,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_8,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_9 }, .ui8_number_of_assist_levels =
		DEFAULT_VALUE_NUMBER_OF_ASSIST_LEVELS,
		.ui8_startup_motor_power_boost_feature_enabled =
		DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED,
		.ui8_startup_motor_power_boost_always =
		DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ALWAYS,
		.ui8_startup_motor_power_boost_factor = {
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_1,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_2,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_3,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_4,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_5,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_6,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_7,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_8,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_9 },
		.ui8_startup_motor_power_boost_time =
		DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_TIME,
		.ui8_startup_motor_power_boost_fade_time =
		DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FADE_TIME,
		.ui8_temperature_limit_feature_enabled =
		DEFAULT_VALUE_MOTOR_TEMPERATURE_FEATURE_ENABLE,
		.ui8_motor_temperature_min_value_to_limit =
		DEFAULT_VALUE_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT,
		.ui8_motor_temperature_max_value_to_limit =
		DEFAULT_VALUE_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT,
		.ui16_battery_voltage_reset_wh_counter_x10 =
		DEFAULT_VALUE_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10,
		.ui8_lcd_power_off_time_minutes =
		DEFAULT_VALUE_LCD_POWER_OFF_TIME, .ui8_lcd_backlight_on_brightness =
		DEFAULT_VALUE_LCD_BACKLIGHT_ON_BRIGHTNESS,
		.ui8_lcd_backlight_off_brightness =
		DEFAULT_VALUE_LCD_BACKLIGHT_OFF_BRIGHTNESS,
		.ui16_battery_pack_resistance_x1000 =
		DEFAULT_VALUE_BATTERY_PACK_RESISTANCE, .ui8_offroad_feature_enabled =
		DEFAULT_VALUE_OFFROAD_FEATURE_ENABLED, .ui8_offroad_enabled_on_startup =
		DEFAULT_VALUE_OFFROAD_MODE_ENABLED_ON_STARTUP,
		.ui8_offroad_speed_limit = DEFAULT_VALUE_OFFROAD_SPEED_LIMIT,
		.ui8_offroad_power_limit_enabled =
		DEFAULT_VALUE_OFFROAD_POWER_LIMIT_ENABLED,
		.ui8_offroad_power_limit_div25 =
		DEFAULT_VALUE_OFFROAD_POWER_LIMIT_DIV25, .ui32_odometer_x10 =
				DEFAULT_VALUE_ODOMETER_X10, .ui8_walk_assist_feature_enabled =
		DEFAULT_VALUE_WALK_ASSIST_FEATURE_ENABLED,
		.ui8_walk_assist_level_factor = {
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_1,
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_2,
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_3,
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_4,
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_5,
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_6,
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_7,
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_8,
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_9 }, .field_selectors = { // we somewhat yuckily pick defaults to match the layout on the previous release
				0, 10, 0, 2, 1 } };

void eeprom_init() {
	eeprom_hw_init();

	// read the values from EEPROM to array
	memset(&m_eeprom_data, 0, sizeof(m_eeprom_data));
	if (!flash_read_words(&m_eeprom_data,
			sizeof(m_eeprom_data)
					/ sizeof(uint32_t)) || m_eeprom_data.eeprom_version < EEPROM_MIN_COMPAT_VERSION)
		// If we are using default data it doesn't get written to flash until someone calls write
		memcpy(&m_eeprom_data, &m_eeprom_data_defaults,
				sizeof(m_eeprom_data_defaults));

	// Perform whatever migrations we need to update old eeprom formats
	if (m_eeprom_data.eeprom_version < EEPROM_VERSION) {

		m_eeprom_data.ui8_lcd_backlight_on_brightness =
				m_eeprom_data_defaults.ui8_lcd_backlight_on_brightness;
		m_eeprom_data.ui8_lcd_backlight_off_brightness =
				m_eeprom_data_defaults.ui8_lcd_backlight_off_brightness;

		m_eeprom_data.eeprom_version = EEPROM_VERSION;
	}

	eeprom_init_variables();

	set_conversions();
}

void eeprom_init_variables(void) {
	l3_vars_t *p_l3_output_vars = get_l3_vars();
	// copy data final variables
	p_l3_output_vars->ui8_assist_level = m_eeprom_data.ui8_assist_level;
	p_l3_output_vars->ui16_wheel_perimeter = m_eeprom_data.ui16_wheel_perimeter;
	p_l3_output_vars->ui8_wheel_max_speed_x10 =
			m_eeprom_data.ui8_wheel_max_speed * 10;
	p_l3_output_vars->ui8_units_type = m_eeprom_data.ui8_units_type;
	p_l3_output_vars->ui32_wh_x10_offset = m_eeprom_data.ui32_wh_x10_offset;
	p_l3_output_vars->ui32_wh_x10_100_percent =
			m_eeprom_data.ui32_wh_x10_100_percent;
	p_l3_output_vars->ui8_battery_soc_enable =
			m_eeprom_data.ui8_battery_soc_enable;
	p_l3_output_vars->ui8_battery_max_current =
			m_eeprom_data.ui8_battery_max_current;
	p_l3_output_vars->ui8_ramp_up_amps_per_second_x10 =
			m_eeprom_data.ui8_ramp_up_amps_per_second_x10;
	p_l3_output_vars->ui8_battery_cells_number =
			m_eeprom_data.ui8_battery_cells_number;
	p_l3_output_vars->ui16_battery_low_voltage_cut_off_x10 =
			m_eeprom_data.ui16_battery_low_voltage_cut_off_x10;
	p_l3_output_vars->ui8_motor_type = m_eeprom_data.ui8_motor_type;
	p_l3_output_vars->ui8_motor_assistance_startup_without_pedal_rotation =
			m_eeprom_data.ui8_motor_assistance_startup_without_pedal_rotation;
	p_l3_output_vars->ui8_temperature_limit_feature_enabled =
			m_eeprom_data.ui8_temperature_limit_feature_enabled;
	COPY_ARRAY(p_l3_output_vars, &m_eeprom_data, ui8_assist_level_factor);
	p_l3_output_vars->ui8_number_of_assist_levels =
			m_eeprom_data.ui8_number_of_assist_levels;
	p_l3_output_vars->ui8_startup_motor_power_boost_feature_enabled =
			m_eeprom_data.ui8_startup_motor_power_boost_feature_enabled;
	p_l3_output_vars->ui8_startup_motor_power_boost_limit_power =
			m_eeprom_data.ui8_startup_motor_power_boost_limit_power;
	p_l3_output_vars->ui8_startup_motor_power_boost_always =
			m_eeprom_data.ui8_startup_motor_power_boost_always;
	COPY_ARRAY(p_l3_output_vars, &m_eeprom_data,
			ui8_startup_motor_power_boost_factor);
	p_l3_output_vars->ui8_startup_motor_power_boost_time =
			m_eeprom_data.ui8_startup_motor_power_boost_time;
	p_l3_output_vars->ui8_startup_motor_power_boost_fade_time =
			m_eeprom_data.ui8_startup_motor_power_boost_fade_time;
	p_l3_output_vars->ui8_motor_temperature_min_value_to_limit =
			m_eeprom_data.ui8_motor_temperature_min_value_to_limit;
	p_l3_output_vars->ui8_motor_temperature_max_value_to_limit =
			m_eeprom_data.ui8_motor_temperature_max_value_to_limit;
	p_l3_output_vars->ui16_battery_voltage_reset_wh_counter_x10 =
			m_eeprom_data.ui16_battery_voltage_reset_wh_counter_x10;
	p_l3_output_vars->ui8_lcd_power_off_time_minutes =
			m_eeprom_data.ui8_lcd_power_off_time_minutes;
	p_l3_output_vars->ui8_lcd_backlight_on_brightness =
			m_eeprom_data.ui8_lcd_backlight_on_brightness;
	p_l3_output_vars->ui8_lcd_backlight_off_brightness =
			m_eeprom_data.ui8_lcd_backlight_off_brightness;
	p_l3_output_vars->ui16_battery_pack_resistance_x1000 =
			m_eeprom_data.ui16_battery_pack_resistance_x1000;
	p_l3_output_vars->ui8_offroad_feature_enabled =
			m_eeprom_data.ui8_offroad_feature_enabled;
	p_l3_output_vars->ui8_offroad_enabled_on_startup =
			m_eeprom_data.ui8_offroad_enabled_on_startup;
	p_l3_output_vars->ui8_offroad_speed_limit =
			m_eeprom_data.ui8_offroad_speed_limit;
	p_l3_output_vars->ui8_offroad_power_limit_enabled =
			m_eeprom_data.ui8_offroad_power_limit_enabled;
	p_l3_output_vars->ui8_offroad_power_limit_div25 =
			m_eeprom_data.ui8_offroad_power_limit_div25;
	p_l3_output_vars->ui32_odometer_x10 = m_eeprom_data.ui32_odometer_x10;
	p_l3_output_vars->ui8_walk_assist_feature_enabled =
			m_eeprom_data.ui8_walk_assist_feature_enabled;
	COPY_ARRAY(p_l3_output_vars, &m_eeprom_data, ui8_walk_assist_level_factor);
	COPY_ARRAY(p_l3_output_vars, &m_eeprom_data, field_selectors);
}

void eeprom_write_variables(void) {
	l3_vars_t *p_l3_output_vars = get_l3_vars();
	m_eeprom_data.ui8_assist_level = p_l3_output_vars->ui8_assist_level;
	m_eeprom_data.ui16_wheel_perimeter = p_l3_output_vars->ui16_wheel_perimeter;
	m_eeprom_data.ui8_wheel_max_speed =
			p_l3_output_vars->ui8_wheel_max_speed_x10 / 10;
	m_eeprom_data.ui8_units_type = p_l3_output_vars->ui8_units_type;
	m_eeprom_data.ui32_wh_x10_offset = p_l3_output_vars->ui32_wh_x10_offset;
	m_eeprom_data.ui32_wh_x10_100_percent =
			p_l3_output_vars->ui32_wh_x10_100_percent;
	m_eeprom_data.ui8_battery_soc_enable =
			p_l3_output_vars->ui8_battery_soc_enable;
	m_eeprom_data.ui8_battery_max_current =
			p_l3_output_vars->ui8_battery_max_current;
	m_eeprom_data.ui8_ramp_up_amps_per_second_x10 =
			p_l3_output_vars->ui8_ramp_up_amps_per_second_x10;
	m_eeprom_data.ui8_battery_cells_number =
			p_l3_output_vars->ui8_battery_cells_number;
	m_eeprom_data.ui16_battery_low_voltage_cut_off_x10 =
			p_l3_output_vars->ui16_battery_low_voltage_cut_off_x10;
	m_eeprom_data.ui8_motor_type = p_l3_output_vars->ui8_motor_type;
	m_eeprom_data.ui8_motor_assistance_startup_without_pedal_rotation =
			p_l3_output_vars->ui8_motor_assistance_startup_without_pedal_rotation;
	m_eeprom_data.ui8_temperature_limit_feature_enabled =
			p_l3_output_vars->ui8_temperature_limit_feature_enabled;
	COPY_ARRAY(&m_eeprom_data, p_l3_output_vars, ui8_assist_level_factor);
	m_eeprom_data.ui8_number_of_assist_levels =
			p_l3_output_vars->ui8_number_of_assist_levels;
	m_eeprom_data.ui8_startup_motor_power_boost_feature_enabled =
			p_l3_output_vars->ui8_startup_motor_power_boost_feature_enabled;
	m_eeprom_data.ui8_startup_motor_power_boost_always =
			p_l3_output_vars->ui8_startup_motor_power_boost_always;
	m_eeprom_data.ui8_startup_motor_power_boost_limit_power =
			p_l3_output_vars->ui8_startup_motor_power_boost_limit_power;
	COPY_ARRAY(&m_eeprom_data, p_l3_output_vars,
			ui8_startup_motor_power_boost_factor);
	m_eeprom_data.ui8_startup_motor_power_boost_time =
			p_l3_output_vars->ui8_startup_motor_power_boost_time;
	m_eeprom_data.ui8_startup_motor_power_boost_fade_time =
			p_l3_output_vars->ui8_startup_motor_power_boost_fade_time;
	m_eeprom_data.ui8_motor_temperature_min_value_to_limit =
			p_l3_output_vars->ui8_motor_temperature_min_value_to_limit;
	m_eeprom_data.ui8_motor_temperature_max_value_to_limit =
			p_l3_output_vars->ui8_motor_temperature_max_value_to_limit;
	m_eeprom_data.ui16_battery_voltage_reset_wh_counter_x10 =
			p_l3_output_vars->ui16_battery_voltage_reset_wh_counter_x10;
	m_eeprom_data.ui8_lcd_power_off_time_minutes =
			p_l3_output_vars->ui8_lcd_power_off_time_minutes;
	m_eeprom_data.ui8_lcd_backlight_on_brightness =
			p_l3_output_vars->ui8_lcd_backlight_on_brightness;
	m_eeprom_data.ui8_lcd_backlight_off_brightness =
			p_l3_output_vars->ui8_lcd_backlight_off_brightness;
	m_eeprom_data.ui16_battery_pack_resistance_x1000 =
			p_l3_output_vars->ui16_battery_pack_resistance_x1000;
	m_eeprom_data.ui8_offroad_feature_enabled =
			p_l3_output_vars->ui8_offroad_feature_enabled;
	m_eeprom_data.ui8_offroad_enabled_on_startup =
			p_l3_output_vars->ui8_offroad_enabled_on_startup;
	m_eeprom_data.ui8_offroad_speed_limit =
			p_l3_output_vars->ui8_offroad_speed_limit;
	m_eeprom_data.ui8_offroad_power_limit_enabled =
			p_l3_output_vars->ui8_offroad_power_limit_enabled;
	m_eeprom_data.ui8_offroad_power_limit_div25 =
			p_l3_output_vars->ui8_offroad_power_limit_div25;
	m_eeprom_data.ui32_odometer_x10 = p_l3_output_vars->ui32_odometer_x10;
	m_eeprom_data.ui8_walk_assist_feature_enabled =
			p_l3_output_vars->ui8_walk_assist_feature_enabled;
	COPY_ARRAY(&m_eeprom_data, p_l3_output_vars, ui8_walk_assist_level_factor);
	COPY_ARRAY(&m_eeprom_data, p_l3_output_vars, field_selectors);

	flash_write_words(&m_eeprom_data, sizeof(m_eeprom_data) / sizeof(uint32_t));
}

