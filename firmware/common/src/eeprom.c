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

const eeprom_data_t m_eeprom_data_defaults = {
    .eeprom_version = EEPROM_VERSION,
		.ui8_assist_level = DEFAULT_VALUE_ASSIST_LEVEL,
		.ui16_wheel_perimeter = DEFAULT_VALUE_WHEEL_PERIMETER,
		.ui8_wheel_max_speed = DEFAULT_VALUE_WHEEL_MAX_SPEED,
		.ui8_units_type = DEFAULT_VALUE_UNITS_TYPE,
		.ui32_wh_x10_offset = DEFAULT_VALUE_WH_X10_OFFSET,
		.ui32_wh_x10_100_percent = DEFAULT_VALUE_HW_X10_100_PERCENT,
		.ui8_battery_soc_enable = DEAFULT_VALUE_SHOW_NUMERIC_BATTERY_SOC,
		.ui8_battery_max_current = DEFAULT_VALUE_BATTERY_MAX_CURRENT,
		.ui8_ramp_up_amps_per_second_x10 = DEFAULT_VALUE_RAMP_UP_AMPS_PER_SECOND_X10,
		.ui8_battery_cells_number = DEFAULT_VALUE_BATTERY_CELLS_NUMBER,
		.ui16_battery_low_voltage_cut_off_x10 = DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10,
		.ui8_motor_type = DEFAULT_VALUE_MOTOR_TYPE,
		.ui8_motor_assistance_startup_without_pedal_rotation = DEFAULT_VALUE_MOTOR_ASSISTANCE_WITHOUT_PEDAL_ROTATION,
		.ui8_assist_level_factor = {
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_1,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_2,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_3,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_4,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_5,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_6,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_7,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_8,
		DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_9 },
		.ui8_number_of_assist_levels = DEFAULT_VALUE_NUMBER_OF_ASSIST_LEVELS,
		.ui8_startup_motor_power_boost_feature_enabled = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED,
		.ui8_startup_motor_power_boost_always = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ALWAYS,
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
		DEFAULT_VALUE_LCD_POWER_OFF_TIME,
		.ui8_lcd_backlight_on_brightness =
		DEFAULT_VALUE_LCD_BACKLIGHT_ON_BRIGHTNESS,
		.ui8_lcd_backlight_off_brightness =
		DEFAULT_VALUE_LCD_BACKLIGHT_OFF_BRIGHTNESS,
		.ui16_battery_pack_resistance_x1000 =
		DEFAULT_VALUE_BATTERY_PACK_RESISTANCE,
		.ui8_offroad_feature_enabled =
		DEFAULT_VALUE_OFFROAD_FEATURE_ENABLED,
		.ui8_offroad_enabled_on_startup =
		DEFAULT_VALUE_OFFROAD_MODE_ENABLED_ON_STARTUP,
		.ui8_offroad_speed_limit = DEFAULT_VALUE_OFFROAD_SPEED_LIMIT,
		.ui8_offroad_power_limit_enabled =
		DEFAULT_VALUE_OFFROAD_POWER_LIMIT_ENABLED,
		.ui8_offroad_power_limit_div25 =
		DEFAULT_VALUE_OFFROAD_POWER_LIMIT_DIV25,
		.ui32_odometer_x10 =
		DEFAULT_VALUE_ODOMETER_X10,
		.ui8_walk_assist_feature_enabled =
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
		DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_9 },
		.field_selectors = { // we somewhat yuckily pick defaults to match the layout on the previous release
				0, // trip time
				1, // trip distance
				5, // human power
				6, // motor power
				3, // wheel speed
    },
    .x_axis_scale = DEFAULT_VALUE_X_AXIS_SCALE,
    .ui8_buttons_up_down_invert = DEFAULT_VALUE_BUTTONS_UP_DOWN_INVERT,
    .customizable_choices_selector = DEFAULT_CUSTOMIZABLE_CHOICES_SELECTOR,
    .customizableFieldIndex = DEFAULT_CUSTOMIZABLE_FIELD_INDEX,

#ifndef SW102
    // enable automatic graph max min for every variable
    .graph_eeprom[0].auto_max_min = GRAPH_AUTO_MAX_MIN_NO,
    .graph_eeprom[0].max = 350, // 35 km/h
    .graph_eeprom[0].min = 0,

    .graph_eeprom[1].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[2].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[3].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[4].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[5].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[6].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[7].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[8].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[9].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[10].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[11].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,
    .graph_eeprom[12].auto_max_min = GRAPH_AUTO_MAX_MIN_YES,

    .wheelSpeedField_auto_thresholds = FIELD_THRESHOLD_AUTO,
    .cadenceField_auto_thresholds = FIELD_THRESHOLD_AUTO,
    .batteryPowerField_auto_thresholds = FIELD_THRESHOLD_AUTO,
    .batteryVoltageField_auto_thresholds = FIELD_THRESHOLD_AUTO,
    .batteryCurrentField_auto_thresholds = FIELD_THRESHOLD_AUTO,
    .motorTempField_auto_thresholds = FIELD_THRESHOLD_AUTO,
    .motorErpsField_auto_thresholds = FIELD_THRESHOLD_AUTO,
    .pwmDutyField_auto_thresholds = FIELD_THRESHOLD_AUTO,
    .motorFOCField_auto_thresholds = FIELD_THRESHOLD_AUTO,
#endif

    .ui16_torque_sensor_calibration_table_left[0][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_WEIGHT_1,
    .ui16_torque_sensor_calibration_table_left[0][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_ADC_1,
    .ui16_torque_sensor_calibration_table_left[1][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_WEIGHT_2,
    .ui16_torque_sensor_calibration_table_left[1][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_ADC_2,
    .ui16_torque_sensor_calibration_table_left[2][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_WEIGHT_3,
    .ui16_torque_sensor_calibration_table_left[2][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_ADC_3,
    .ui16_torque_sensor_calibration_table_left[3][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_WEIGHT_4,
    .ui16_torque_sensor_calibration_table_left[3][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_ADC_4,
    .ui16_torque_sensor_calibration_table_left[4][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_WEIGHT_5,
    .ui16_torque_sensor_calibration_table_left[4][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_ADC_5,
    .ui16_torque_sensor_calibration_table_left[5][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_WEIGHT_6,
    .ui16_torque_sensor_calibration_table_left[5][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_ADC_6,
    .ui16_torque_sensor_calibration_table_left[6][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_WEIGHT_7,
    .ui16_torque_sensor_calibration_table_left[6][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_ADC_7,
    .ui16_torque_sensor_calibration_table_left[7][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_WEIGHT_8,
    .ui16_torque_sensor_calibration_table_left[7][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_LEFT_ADC_8,

    .ui16_torque_sensor_calibration_table_right[0][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_WEIGHT_1,
    .ui16_torque_sensor_calibration_table_right[0][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_ADC_1,
    .ui16_torque_sensor_calibration_table_right[1][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_WEIGHT_2,
    .ui16_torque_sensor_calibration_table_right[1][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_ADC_2,
    .ui16_torque_sensor_calibration_table_right[2][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_WEIGHT_3,
    .ui16_torque_sensor_calibration_table_right[2][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_ADC_3,
    .ui16_torque_sensor_calibration_table_right[3][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_WEIGHT_4,
    .ui16_torque_sensor_calibration_table_right[3][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_ADC_4,
    .ui16_torque_sensor_calibration_table_right[4][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_WEIGHT_5,
    .ui16_torque_sensor_calibration_table_right[4][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_ADC_5,
    .ui16_torque_sensor_calibration_table_right[5][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_WEIGHT_6,
    .ui16_torque_sensor_calibration_table_right[5][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_ADC_6,
    .ui16_torque_sensor_calibration_table_right[6][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_WEIGHT_7,
    .ui16_torque_sensor_calibration_table_right[6][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_ADC_7,
    .ui16_torque_sensor_calibration_table_right[7][0] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_WEIGHT_8,
    .ui16_torque_sensor_calibration_table_right[7][1] = DEFAULT_TORQUE_SENSOR_CALIBRATION_RIGHT_ADC_8,
};

void eeprom_init() {
	eeprom_hw_init();

	// read the values from EEPROM to array
	memset(&m_eeprom_data, 0, sizeof(m_eeprom_data));

	// if eeprom is blank use defaults
	// if eeprom version is less than the min required version, wipe and use defaults
	// if eeprom version is greater than the current app version, user must have downgraded - wipe and use defaults
	if (!flash_read_words(&m_eeprom_data,
			sizeof(m_eeprom_data)
					/ sizeof(uint32_t))
	    || m_eeprom_data.eeprom_version < EEPROM_MIN_COMPAT_VERSION
	    || m_eeprom_data.eeprom_version > EEPROM_VERSION
	    )
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
	ui_vars_t *ui_vars = get_ui_vars();
	// copy data final variables
	ui_vars->ui8_assist_level = m_eeprom_data.ui8_assist_level;
	ui_vars->ui16_wheel_perimeter = m_eeprom_data.ui16_wheel_perimeter;
	ui_vars->wheel_max_speed_x10 =
			m_eeprom_data.ui8_wheel_max_speed * 10;
	ui_vars->ui8_units_type = m_eeprom_data.ui8_units_type;
	ui_vars->ui32_wh_x10_offset = m_eeprom_data.ui32_wh_x10_offset;
	ui_vars->ui32_wh_x10_100_percent =
			m_eeprom_data.ui32_wh_x10_100_percent;
	ui_vars->ui8_battery_soc_enable =
			m_eeprom_data.ui8_battery_soc_enable;
	ui_vars->ui8_battery_max_current =
			m_eeprom_data.ui8_battery_max_current;
	ui_vars->ui8_ramp_up_amps_per_second_x10 =
			m_eeprom_data.ui8_ramp_up_amps_per_second_x10;
	ui_vars->ui8_battery_cells_number =
			m_eeprom_data.ui8_battery_cells_number;
	ui_vars->ui16_battery_low_voltage_cut_off_x10 =
			m_eeprom_data.ui16_battery_low_voltage_cut_off_x10;
	ui_vars->ui8_motor_type = m_eeprom_data.ui8_motor_type;
	ui_vars->ui8_motor_assistance_startup_without_pedal_rotation =
			m_eeprom_data.ui8_motor_assistance_startup_without_pedal_rotation;
	ui_vars->ui8_temperature_limit_feature_enabled =
			m_eeprom_data.ui8_temperature_limit_feature_enabled;
	COPY_ARRAY(ui_vars, &m_eeprom_data, ui8_assist_level_factor);
	ui_vars->ui8_number_of_assist_levels =
			m_eeprom_data.ui8_number_of_assist_levels;
	ui_vars->ui8_startup_motor_power_boost_feature_enabled =
			m_eeprom_data.ui8_startup_motor_power_boost_feature_enabled;
	ui_vars->ui8_startup_motor_power_boost_limit_power =
			m_eeprom_data.ui8_startup_motor_power_boost_limit_power;
	ui_vars->ui8_startup_motor_power_boost_always =
			m_eeprom_data.ui8_startup_motor_power_boost_always;
	COPY_ARRAY(ui_vars, &m_eeprom_data,
			ui8_startup_motor_power_boost_factor);
	ui_vars->ui8_startup_motor_power_boost_time =
			m_eeprom_data.ui8_startup_motor_power_boost_time;
	ui_vars->ui8_startup_motor_power_boost_fade_time =
			m_eeprom_data.ui8_startup_motor_power_boost_fade_time;
	ui_vars->ui8_motor_temperature_min_value_to_limit =
			m_eeprom_data.ui8_motor_temperature_min_value_to_limit;
	ui_vars->ui8_motor_temperature_max_value_to_limit =
			m_eeprom_data.ui8_motor_temperature_max_value_to_limit;
	ui_vars->ui16_battery_voltage_reset_wh_counter_x10 =
			m_eeprom_data.ui16_battery_voltage_reset_wh_counter_x10;
	ui_vars->ui8_lcd_power_off_time_minutes =
			m_eeprom_data.ui8_lcd_power_off_time_minutes;
	ui_vars->ui8_lcd_backlight_on_brightness =
			m_eeprom_data.ui8_lcd_backlight_on_brightness;
	ui_vars->ui8_lcd_backlight_off_brightness =
			m_eeprom_data.ui8_lcd_backlight_off_brightness;
	ui_vars->ui16_battery_pack_resistance_x1000 =
			m_eeprom_data.ui16_battery_pack_resistance_x1000;
	ui_vars->ui8_offroad_feature_enabled =
			m_eeprom_data.ui8_offroad_feature_enabled;
	ui_vars->ui8_offroad_enabled_on_startup =
			m_eeprom_data.ui8_offroad_enabled_on_startup;
	ui_vars->ui8_offroad_speed_limit =
			m_eeprom_data.ui8_offroad_speed_limit;
	ui_vars->ui8_offroad_power_limit_enabled =
			m_eeprom_data.ui8_offroad_power_limit_enabled;
	ui_vars->ui8_offroad_power_limit_div25 =
			m_eeprom_data.ui8_offroad_power_limit_div25;
	ui_vars->ui32_odometer_x10 = m_eeprom_data.ui32_odometer_x10;
	ui_vars->ui8_walk_assist_feature_enabled =
			m_eeprom_data.ui8_walk_assist_feature_enabled;
	COPY_ARRAY(ui_vars, &m_eeprom_data, ui8_walk_assist_level_factor);
	COPY_ARRAY(ui_vars, &m_eeprom_data, field_selectors);
  ui_vars->x_axis_scale = m_eeprom_data.x_axis_scale;
  ui_vars->ui8_buttons_up_down_invert = m_eeprom_data.ui8_buttons_up_down_invert;
  ui_vars->ui8_torque_sensor_calibration_pedal_ground = m_eeprom_data.ui8_torque_sensor_calibration_pedal_ground;
  graphs.customizable.selector = &m_eeprom_data.customizable_choices_selector;
  g_customizableFieldIndex = m_eeprom_data.customizableFieldIndex;

#ifndef SW102
  for (uint8_t i = 0; i < GRAPH_VARIANT_SIZE; i++) {
    g_graphs[i].auto_max_min = m_eeprom_data.graph_eeprom[i].auto_max_min;
    g_graphs[i].max = m_eeprom_data.graph_eeprom[i].max;
    g_graphs[i].min = m_eeprom_data.graph_eeprom[i].min;
  }
  wheelSpeedField.editable.number.auto_thresholds = m_eeprom_data.wheelSpeedField_auto_thresholds;
  wheelSpeedField.editable.number.config_error_threshold = m_eeprom_data.wheelSpeedField_config_error_threshold;
  wheelSpeedField.editable.number.config_warn_threshold = m_eeprom_data.wheelSpeedField_config_warn_threshold;
  cadenceField.editable.number.auto_thresholds = m_eeprom_data.cadenceField_auto_thresholds;
  cadenceField.editable.number.config_error_threshold = m_eeprom_data.cadenceField_config_error_threshold;
  cadenceField.editable.number.config_warn_threshold = m_eeprom_data.cadenceField_config_warn_threshold;
  humanPowerField.editable.number.auto_thresholds = m_eeprom_data.humanPowerField_auto_thresholds;
  humanPowerField.editable.number.config_error_threshold = m_eeprom_data.humanPowerField_config_error_threshold;
  humanPowerField.editable.number.config_warn_threshold = m_eeprom_data.humanPowerField_config_warn_threshold;
  batteryPowerField.editable.number.auto_thresholds = m_eeprom_data.batteryPowerField_auto_thresholds;
  batteryPowerField.editable.number.config_error_threshold = m_eeprom_data.batteryPowerField_config_error_threshold;
  batteryPowerField.editable.number.config_warn_threshold = m_eeprom_data.batteryPowerField_config_warn_threshold;
  batteryVoltageField.editable.number.auto_thresholds = m_eeprom_data.batteryVoltageField_auto_thresholds;
  batteryVoltageField.editable.number.config_error_threshold = m_eeprom_data.batteryVoltageField_config_error_threshold;
  batteryVoltageField.editable.number.config_warn_threshold = m_eeprom_data.batteryVoltageField_config_warn_threshold;
  batteryCurrentField.editable.number.auto_thresholds = m_eeprom_data.batteryCurrentField_auto_thresholds;
  batteryCurrentField.editable.number.config_error_threshold = m_eeprom_data.batteryCurrentField_config_error_threshold;
  batteryCurrentField.editable.number.config_warn_threshold = m_eeprom_data.batteryCurrentField_config_warn_threshold;
  motorTempField.editable.number.auto_thresholds = m_eeprom_data.motorTempField_auto_thresholds;
  motorTempField.editable.number.config_error_threshold = m_eeprom_data.motorTempField_config_error_threshold;
  motorTempField.editable.number.config_warn_threshold = m_eeprom_data.motorTempField_config_warn_threshold;
  motorErpsField.editable.number.auto_thresholds = m_eeprom_data.motorErpsField_auto_thresholds;
  motorErpsField.editable.number.config_error_threshold = m_eeprom_data.motorErpsField_config_error_threshold;
  motorErpsField.editable.number.config_warn_threshold = m_eeprom_data.motorErpsField_config_warn_threshold;
  pwmDutyField.editable.number.auto_thresholds = m_eeprom_data.pwmDutyField_auto_thresholds;
  pwmDutyField.editable.number.config_error_threshold = m_eeprom_data.pwmDutyField_config_error_threshold;
  pwmDutyField.editable.number.config_warn_threshold = m_eeprom_data.pwmDutyField_config_warn_threshold;
  motorFOCField.editable.number.auto_thresholds = m_eeprom_data.motorFOCField_auto_thresholds;
  motorFOCField.editable.number.config_error_threshold = m_eeprom_data.motorFOCField_config_error_threshold;
  motorFOCField.editable.number.config_warn_threshold = m_eeprom_data.motorFOCField_config_warn_threshold;
#endif

  ui_vars->ui8_torque_sensor_calibration_feature_enabled = m_eeprom_data.ui8_torque_sensor_calibration_feature_enabled;
  ui_vars->ui8_torque_sensor_calibration_pedal_ground = m_eeprom_data.ui8_torque_sensor_calibration_pedal_ground;
  for (uint8_t i = 0; i < 8; i++) {
    ui_vars->ui16_torque_sensor_calibration_table_left[i][0] = m_eeprom_data.ui16_torque_sensor_calibration_table_left[i][0];
    ui_vars->ui16_torque_sensor_calibration_table_left[i][1] = m_eeprom_data.ui16_torque_sensor_calibration_table_left[i][1];
    ui_vars->ui16_torque_sensor_calibration_table_right[i][0] = m_eeprom_data.ui16_torque_sensor_calibration_table_right[i][0];
    ui_vars->ui16_torque_sensor_calibration_table_right[i][1] = m_eeprom_data.ui16_torque_sensor_calibration_table_right[i][1];
  }
}

void eeprom_write_variables(void) {
	ui_vars_t *ui_vars = get_ui_vars();
	m_eeprom_data.ui8_assist_level = ui_vars->ui8_assist_level;
	m_eeprom_data.ui16_wheel_perimeter = ui_vars->ui16_wheel_perimeter;
	m_eeprom_data.ui8_wheel_max_speed =
			ui_vars->wheel_max_speed_x10 / 10;
	m_eeprom_data.ui8_units_type = ui_vars->ui8_units_type;
	m_eeprom_data.ui32_wh_x10_offset = ui_vars->ui32_wh_x10_offset;
	m_eeprom_data.ui32_wh_x10_100_percent =
			ui_vars->ui32_wh_x10_100_percent;
	m_eeprom_data.ui8_battery_soc_enable =
			ui_vars->ui8_battery_soc_enable;
	m_eeprom_data.ui8_battery_max_current =
			ui_vars->ui8_battery_max_current;
	m_eeprom_data.ui8_ramp_up_amps_per_second_x10 =
			ui_vars->ui8_ramp_up_amps_per_second_x10;
	m_eeprom_data.ui8_battery_cells_number =
			ui_vars->ui8_battery_cells_number;
	m_eeprom_data.ui16_battery_low_voltage_cut_off_x10 =
			ui_vars->ui16_battery_low_voltage_cut_off_x10;
	m_eeprom_data.ui8_motor_type = ui_vars->ui8_motor_type;
	m_eeprom_data.ui8_motor_assistance_startup_without_pedal_rotation =
			ui_vars->ui8_motor_assistance_startup_without_pedal_rotation;
	m_eeprom_data.ui8_temperature_limit_feature_enabled =
			ui_vars->ui8_temperature_limit_feature_enabled;
	COPY_ARRAY(&m_eeprom_data, ui_vars, ui8_assist_level_factor);
	m_eeprom_data.ui8_number_of_assist_levels =
			ui_vars->ui8_number_of_assist_levels;
	m_eeprom_data.ui8_startup_motor_power_boost_feature_enabled =
			ui_vars->ui8_startup_motor_power_boost_feature_enabled;
	m_eeprom_data.ui8_startup_motor_power_boost_always =
			ui_vars->ui8_startup_motor_power_boost_always;
	m_eeprom_data.ui8_startup_motor_power_boost_limit_power =
			ui_vars->ui8_startup_motor_power_boost_limit_power;
	COPY_ARRAY(&m_eeprom_data, ui_vars,
			ui8_startup_motor_power_boost_factor);
	m_eeprom_data.ui8_startup_motor_power_boost_time =
			ui_vars->ui8_startup_motor_power_boost_time;
	m_eeprom_data.ui8_startup_motor_power_boost_fade_time =
			ui_vars->ui8_startup_motor_power_boost_fade_time;
	m_eeprom_data.ui8_motor_temperature_min_value_to_limit =
			ui_vars->ui8_motor_temperature_min_value_to_limit;
	m_eeprom_data.ui8_motor_temperature_max_value_to_limit =
			ui_vars->ui8_motor_temperature_max_value_to_limit;
	m_eeprom_data.ui16_battery_voltage_reset_wh_counter_x10 =
			ui_vars->ui16_battery_voltage_reset_wh_counter_x10;
	m_eeprom_data.ui8_lcd_power_off_time_minutes =
			ui_vars->ui8_lcd_power_off_time_minutes;
	m_eeprom_data.ui8_lcd_backlight_on_brightness =
			ui_vars->ui8_lcd_backlight_on_brightness;
	m_eeprom_data.ui8_lcd_backlight_off_brightness =
			ui_vars->ui8_lcd_backlight_off_brightness;
	m_eeprom_data.ui16_battery_pack_resistance_x1000 =
			ui_vars->ui16_battery_pack_resistance_x1000;
	m_eeprom_data.ui8_offroad_feature_enabled =
			ui_vars->ui8_offroad_feature_enabled;
	m_eeprom_data.ui8_offroad_enabled_on_startup =
			ui_vars->ui8_offroad_enabled_on_startup;
	m_eeprom_data.ui8_offroad_speed_limit =
			ui_vars->ui8_offroad_speed_limit;
	m_eeprom_data.ui8_offroad_power_limit_enabled =
			ui_vars->ui8_offroad_power_limit_enabled;
	m_eeprom_data.ui8_offroad_power_limit_div25 =
			ui_vars->ui8_offroad_power_limit_div25;
	m_eeprom_data.ui32_odometer_x10 = ui_vars->ui32_odometer_x10;
	m_eeprom_data.ui8_walk_assist_feature_enabled =
			ui_vars->ui8_walk_assist_feature_enabled;
	COPY_ARRAY(&m_eeprom_data, ui_vars, ui8_walk_assist_level_factor);
	COPY_ARRAY(&m_eeprom_data, ui_vars, field_selectors);
  m_eeprom_data.x_axis_scale = ui_vars->x_axis_scale;
  m_eeprom_data.ui8_buttons_up_down_invert = ui_vars->ui8_buttons_up_down_invert;

  m_eeprom_data.ui8_torque_sensor_calibration_feature_enabled = ui_vars->ui8_torque_sensor_calibration_feature_enabled;
  m_eeprom_data.ui8_torque_sensor_calibration_pedal_ground = ui_vars->ui8_torque_sensor_calibration_pedal_ground;
  for (uint8_t i = 0; i < 8; i++) {
    m_eeprom_data.ui16_torque_sensor_calibration_table_left[i][0] = ui_vars->ui16_torque_sensor_calibration_table_left[i][0];
    m_eeprom_data.ui16_torque_sensor_calibration_table_left[i][1] = ui_vars->ui16_torque_sensor_calibration_table_left[i][1];
    m_eeprom_data.ui16_torque_sensor_calibration_table_right[i][0] = ui_vars->ui16_torque_sensor_calibration_table_right[i][0];
    m_eeprom_data.ui16_torque_sensor_calibration_table_right[i][1] = ui_vars->ui16_torque_sensor_calibration_table_right[i][1];
  }

  m_eeprom_data.customizable_choices_selector = *graphs.customizable.selector;
  m_eeprom_data.customizableFieldIndex = g_customizableFieldIndex;

#ifndef SW102
  for (uint8_t i = 0; i < GRAPH_VARIANT_SIZE; i++) {
    m_eeprom_data.graph_eeprom[i].auto_max_min = g_graphs[i].auto_max_min;
    m_eeprom_data.graph_eeprom[i].max = g_graphs[i].max;
    m_eeprom_data.graph_eeprom[i].min = g_graphs[i].min;
  }
  m_eeprom_data.wheelSpeedField_auto_thresholds = wheelSpeedField.editable.number.auto_thresholds;
  m_eeprom_data.wheelSpeedField_config_error_threshold = wheelSpeedField.editable.number.config_error_threshold;
  m_eeprom_data.wheelSpeedField_config_warn_threshold = wheelSpeedField.editable.number.config_warn_threshold;
  m_eeprom_data.cadenceField_auto_thresholds = cadenceField.editable.number.auto_thresholds;
  m_eeprom_data.cadenceField_config_error_threshold = cadenceField.editable.number.config_error_threshold;
  m_eeprom_data.cadenceField_config_warn_threshold = cadenceField.editable.number.config_warn_threshold;
  m_eeprom_data.humanPowerField_auto_thresholds = humanPowerField.editable.number.auto_thresholds;
  m_eeprom_data.humanPowerField_config_error_threshold = humanPowerField.editable.number.config_error_threshold;
  m_eeprom_data.humanPowerField_config_warn_threshold = humanPowerField.editable.number.config_warn_threshold;
  m_eeprom_data.batteryPowerField_auto_thresholds = batteryPowerField.editable.number.auto_thresholds;
  m_eeprom_data.batteryPowerField_config_error_threshold = batteryPowerField.editable.number.config_error_threshold;
  m_eeprom_data.batteryPowerField_config_warn_threshold = batteryPowerField.editable.number.config_warn_threshold;
  m_eeprom_data.batteryVoltageField_auto_thresholds = batteryVoltageField.editable.number.auto_thresholds;
  m_eeprom_data.batteryVoltageField_config_error_threshold = batteryVoltageField.editable.number.config_error_threshold;
  m_eeprom_data.batteryVoltageField_config_warn_threshold = batteryVoltageField.editable.number.config_warn_threshold;
  m_eeprom_data.batteryCurrentField_auto_thresholds = batteryCurrentField.editable.number.auto_thresholds;
  m_eeprom_data.batteryCurrentField_config_error_threshold = batteryCurrentField.editable.number.config_error_threshold;
  m_eeprom_data.batteryCurrentField_config_warn_threshold = batteryCurrentField.editable.number.config_warn_threshold;
  m_eeprom_data.motorTempField_auto_thresholds = motorTempField.editable.number.auto_thresholds;
  m_eeprom_data.motorTempField_config_error_threshold = motorTempField.editable.number.config_error_threshold;
  m_eeprom_data.motorTempField_config_warn_threshold = motorTempField.editable.number.config_warn_threshold;
  m_eeprom_data.motorErpsField_auto_thresholds = motorErpsField.editable.number.auto_thresholds;
  m_eeprom_data.motorErpsField_config_error_threshold = motorErpsField.editable.number.config_error_threshold;
  m_eeprom_data.motorErpsField_config_warn_threshold = motorErpsField.editable.number.config_warn_threshold;
  m_eeprom_data.pwmDutyField_auto_thresholds = pwmDutyField.editable.number.auto_thresholds;
  m_eeprom_data.pwmDutyField_config_error_threshold = pwmDutyField.editable.number.config_error_threshold;
  m_eeprom_data.pwmDutyField_config_warn_threshold = pwmDutyField.editable.number.config_warn_threshold;
  m_eeprom_data.motorFOCField_auto_thresholds = motorFOCField.editable.number.auto_thresholds;
  m_eeprom_data.motorFOCField_config_error_threshold = motorFOCField.editable.number.config_error_threshold;
  m_eeprom_data.motorFOCField_config_warn_threshold = motorFOCField.editable.number.config_warn_threshold;
#endif

	flash_write_words(&m_eeprom_data, sizeof(m_eeprom_data) / sizeof(uint32_t));
}

void eeprom_init_defaults(void)
{
  // FIXME: SW102 implementation!
#ifndef SW102
  // first force KEY value to 0
  eeprom_write(ADDRESS_KEY, 0);

  // eeprom_init() will read the default values now
  eeprom_init();
#endif
}
