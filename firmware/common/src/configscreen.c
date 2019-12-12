#include "screen.h"
#include "mainscreen.h"
#include "configscreen.h"
#include "eeprom.h"

static Field wheelMenus[] =
		{
						FIELD_EDITABLE_UINT("Max speed", &ui_vars.wheel_max_speed_x10, "kph", 1, 990, .div_digits = 1, .inc_step = 10, .hide_fraction = true),
						FIELD_EDITABLE_UINT("Circumference", &ui_vars.ui16_wheel_perimeter, "mm", 750, 3000, .inc_step = 10),
						FIELD_EDITABLE_UINT("Odometer", &ui_vars.ui32_odometer_x10, "km", 0, UINT32_MAX, .div_digits = 1, .inc_step = 100, .onPreSetEditable = onSetConfigurationWheelOdometer),
				FIELD_END };

static Field batteryMenus[] =
		{
						FIELD_EDITABLE_UINT("Max current", &ui_vars.ui8_battery_max_current, "amps", 1, 30),
						FIELD_EDITABLE_UINT("Current ramp", &ui_vars.ui8_ramp_up_amps_per_second_x10, "amps", 4, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Low cut-off", &ui_vars.ui16_battery_low_voltage_cut_off_x10, "volts", 160, 630, .div_digits = 1),
						FIELD_EDITABLE_UINT("Num cells", &ui_vars.ui8_battery_cells_number, "", 7, 14),
						FIELD_EDITABLE_UINT("Resistance", &ui_vars.ui16_battery_pack_resistance_x1000, "mohm", 0, 1000),
						FIELD_READONLY_UINT("Voltage", &ui_vars.ui16_battery_voltage_soc_x10, "volts", false, .div_digits = 1),
				FIELD_END };

static Field batterySOCMenus[] =
		{
						FIELD_EDITABLE_ENUM("Feature", &ui_vars.ui8_battery_soc_enable, "disable", "enable"),
						FIELD_EDITABLE_ENUM("Show", &ui_vars.ui8_battery_soc_increment_decrement, "% full", "% used"),
						FIELD_EDITABLE_UINT(_S("Reset at voltage", "Reset at"), &ui_vars.ui16_battery_voltage_reset_wh_counter_x10, "volts", 160, 630, .div_digits = 1),
						FIELD_EDITABLE_UINT(_S("Battery total Wh", "Battery total"), &ui_vars.ui32_wh_x10_100_percent, "whr", 0, 9990, .div_digits = 1, .inc_step = 100),
						FIELD_EDITABLE_UINT("Used Wh", &ui_vars.ui32_wh_x10_offset, "whr", 0, 99900, .div_digits = 1, .inc_step = 100),
				FIELD_END };

static Field assistMenus[] =
		{
						FIELD_EDITABLE_UINT(_S("Num assist levels", "Num Levels"), &ui_vars.ui8_number_of_assist_levels, "", 1, 9),
						FIELD_EDITABLE_UINT("Level 1", &ui_vars.ui8_assist_level_factor[0], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 2", &ui_vars.ui8_assist_level_factor[1], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 3", &ui_vars.ui8_assist_level_factor[2], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 4", &ui_vars.ui8_assist_level_factor[3], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 5", &ui_vars.ui8_assist_level_factor[4], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 6", &ui_vars.ui8_assist_level_factor[5], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 7", &ui_vars.ui8_assist_level_factor[6], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 8", &ui_vars.ui8_assist_level_factor[7], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 9", &ui_vars.ui8_assist_level_factor[8], "", 0, 255, .div_digits = 1),
				FIELD_END };

static Field walkAssistMenus[] =
		{
						FIELD_EDITABLE_ENUM("Feature", &ui_vars.ui8_walk_assist_feature_enabled, "disable", "enable"), // FIXME, share one array of disable/enable strings
						FIELD_EDITABLE_UINT("Level 1", &ui_vars.ui8_walk_assist_level_factor[0], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 2", &ui_vars.ui8_walk_assist_level_factor[1], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 3", &ui_vars.ui8_walk_assist_level_factor[2], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 4", &ui_vars.ui8_walk_assist_level_factor[3], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 5", &ui_vars.ui8_walk_assist_level_factor[4], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 6", &ui_vars.ui8_walk_assist_level_factor[5], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 7", &ui_vars.ui8_walk_assist_level_factor[6], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 8", &ui_vars.ui8_walk_assist_level_factor[7], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 9", &ui_vars.ui8_walk_assist_level_factor[8], "", 0, 100),
				FIELD_END };

static Field startupPowerMenus[] =
		{
						FIELD_EDITABLE_ENUM("Feature", &ui_vars.ui8_startup_motor_power_boost_feature_enabled, "disable", "enable"), // FIXME, share one array of disable/enable strings
						FIELD_EDITABLE_ENUM("Active on", &ui_vars.ui8_startup_motor_power_boost_always, "startup", "always"),
						FIELD_EDITABLE_ENUM("Limit to max-power", &ui_vars.ui8_startup_motor_power_boost_limit_power, "no", "yes"),
						FIELD_EDITABLE_UINT("Duration", &ui_vars.ui8_startup_motor_power_boost_time, "secs", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Fade", &ui_vars.ui8_startup_motor_power_boost_fade_time, "secs", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 1", &ui_vars.ui8_startup_motor_power_boost_factor[0], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 2", &ui_vars.ui8_startup_motor_power_boost_factor[1], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 3", &ui_vars.ui8_startup_motor_power_boost_factor[2], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 4", &ui_vars.ui8_startup_motor_power_boost_factor[3], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 5", &ui_vars.ui8_startup_motor_power_boost_factor[4], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 6", &ui_vars.ui8_startup_motor_power_boost_factor[5], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 7", &ui_vars.ui8_startup_motor_power_boost_factor[6], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 8", &ui_vars.ui8_startup_motor_power_boost_factor[7], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 9", &ui_vars.ui8_startup_motor_power_boost_factor[8], "", 0, 255, .div_digits = 1),
				FIELD_END };

static Field motorTempMenus[] =
		{
						FIELD_EDITABLE_ENUM("Feature", &ui_vars.ui8_temperature_limit_feature_enabled, "disable", "temperature", "throttle"), // FIXME, share one array of disable/enable strings
						FIELD_EDITABLE_UINT("Min limit", &ui_vars.ui8_motor_temperature_min_value_to_limit, "C", 0, 255),
						FIELD_EDITABLE_UINT("Max limit", &ui_vars.ui8_motor_temperature_max_value_to_limit, "C", 0, 255),
				FIELD_END };

static Field displayMenus[] =
		{
#ifndef SW102
  FIELD_EDITABLE_UINT("Clock hours", &ui8_g_configuration_clock_hours, "", 0, 23, .onPreSetEditable = onSetConfigurationClockHours),
  FIELD_EDITABLE_UINT("Clock minutes", &ui8_g_configuration_clock_minutes, "", 0, 59, .onPreSetEditable = onSetConfigurationClockMinutes),
  FIELD_EDITABLE_UINT("Brightness on", &ui_vars.ui8_lcd_backlight_on_brightness, "", 5, 100, .inc_step = 5, .onPreSetEditable = onSetConfigurationDisplayLcdBacklightOnBrightness),
  FIELD_EDITABLE_UINT("Brightness off", &ui_vars.ui8_lcd_backlight_off_brightness, "", 5, 100, .inc_step = 5, .onPreSetEditable = onSetConfigurationDisplayLcdBacklightOffBrightness),
  FIELD_EDITABLE_ENUM("Buttons invert", &ui_vars.ui8_buttons_up_down_invert, "default", "invert"),
#endif
  FIELD_EDITABLE_UINT("Auto power off", &ui_vars.ui8_lcd_power_off_time_minutes, "mins", 0, 255),
  FIELD_EDITABLE_ENUM("Units", &ui_vars.ui8_units_type, "SI", "Imperial"),
#ifndef SW102
  FIELD_READONLY_ENUM("LCD type", &g_lcd_ic_type, "ILI9481", "ST7796", "unknown"),
#endif
  FIELD_EDITABLE_ENUM("Reset to defaults", &ui8_g_configuration_display_reset_to_defaults, "no", "yes"),
  FIELD_END };

#if 0
static Field offroadMenus[] = {
    FIELD_EDITABLE_ENUM("Feature", &ui_vars.ui8_offroad_feature_enabled, "disable", "enable"), // FIXME, share one array of disable/enable strings
    FIELD_EDITABLE_ENUM("Active on start", &ui_vars.ui8_offroad_enabled_on_startup, "no", "yes"), // FIXME, share one array of disable/enable strings
    FIELD_EDITABLE_UINT("Speed limit", &ui_vars.ui8_offroad_speed_limit_x10, "kph", 1, 990, .div_digits = 10, .inc_step = 10, .hide_fraction = true),
    FIELD_EDITABLE_ENUM("Limit power", &ui_vars.ui8_offroad_power_limit_enabled, "no", "yes"), // FIXME, share one array of disable/enable strings
    FIELD_EDITABLE_UINT("Power limit", &ui_vars.ui8_offroad_power_limit_div25, "watt", 0, 2000), // huge FIXME - div25 can't work with this system, change it
    FIELD_END
};
#endif

static Field variousMenus[] = {
  FIELD_EDITABLE_ENUM("Motor voltage", &ui_vars.ui8_motor_type, "48V", "36V", "expert"),
  FIELD_EDITABLE_ENUM("Motor assist", &ui_vars.ui8_motor_assistance_startup_without_pedal_rotation, "disable", "enable"), // FIXME, share one array of disable/enable strings
  FIELD_END };

#ifndef SW102
static Field varSpeedMenus[] = {
  FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[0].auto_max_min, "yes", "no"),
  FIELD_EDITABLE_UINT("Graph max", &g_graphVars[0].max, "km", 0, 2000, .div_digits = 1, .inc_step = 10),
  FIELD_EDITABLE_UINT("Graph min", &g_graphVars[0].min, "km", 0, 2000, .div_digits = 1, .inc_step = 10),
  FIELD_EDITABLE_ENUM("Thresholds", &wheelSpeedField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
  FIELD_EDITABLE_UINT("Max threshold", &wheelSpeedField.editable.number.config_error_threshold, "km", 0, 2000, .div_digits = 1, .inc_step = 10),
  FIELD_EDITABLE_UINT("Min threshold", &wheelSpeedField.editable.number.config_warn_threshold, "km", 0, 2000, .div_digits = 1, .inc_step = 10),
  FIELD_END };

static Field varTripDistanceMenus[] = {
  FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[1].auto_max_min, "yes", "no"),
  FIELD_EDITABLE_UINT("Graph max", &g_graphVars[1].max, "km", 0, INT32_MAX, .div_digits = 1, .inc_step = 10),
  FIELD_EDITABLE_UINT("Graph min", &g_graphVars[1].min, "km", 0, INT32_MAX, .div_digits = 1, .inc_step = 10),
  FIELD_END };

static Field varOdoMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[2].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[2].max, "km", 0, INT32_MAX, .div_digits = 1, .inc_step = 10),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[2].min, "km", 0, INT32_MAX, .div_digits = 1, .inc_step = 10),
  FIELD_END };

static Field varCadenceMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[3].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[3].max, "", 0, 200, .inc_step = 1),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[3].min, "", 0, 200, .inc_step = 1),
    FIELD_EDITABLE_ENUM("Thresholds", &cadenceField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
    FIELD_EDITABLE_UINT("Max threshold", &cadenceField.editable.number.config_error_threshold, "", 0, 2000, .div_digits = 1, .inc_step = 10),
    FIELD_EDITABLE_UINT("Min threshold", &cadenceField.editable.number.config_warn_threshold, "", 0, 2000, .div_digits = 1, .inc_step = 10),
  FIELD_END };

static Field varHumanPowerMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[4].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[4].max, "", 0, 5000, .inc_step = 10),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[4].min, "", 0, 5000, .inc_step = 10),
    FIELD_EDITABLE_ENUM("Thresholds", &humanPowerField.editable.number.auto_thresholds, "disabled", "manual"),
    FIELD_EDITABLE_UINT("Max threshold", &humanPowerField.editable.number.config_error_threshold, "", 0, 20000, .div_digits = 1, .inc_step = 10),
    FIELD_EDITABLE_UINT("Min threshold", &humanPowerField.editable.number.config_warn_threshold, "", 0, 20000, .div_digits = 1, .inc_step = 10),
  FIELD_END };

static Field varBatteryPowerMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[5].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[5].max, "", 0, 5000, .inc_step = 10),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[5].min, "", 0, 5000, .inc_step = 10),
    FIELD_EDITABLE_ENUM("Thresholds", &batteryPowerField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
    FIELD_EDITABLE_UINT("Max threshold", &batteryPowerField.editable.number.config_error_threshold, "", 0, 2000, .div_digits = 0, .inc_step = 10),
    FIELD_EDITABLE_UINT("Min threshold", &batteryPowerField.editable.number.config_warn_threshold, "", 0, 2000, .div_digits = 0, .inc_step = 10),
  FIELD_END };

static Field varBatteryVoltageMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[6].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[6].max, "", 0, 1000, .div_digits = 1, .inc_step = 1),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[6].min, "", 0, 1000, .div_digits = 1, .inc_step = 1),
    FIELD_EDITABLE_ENUM("Thresholds", &batteryVoltageField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
    FIELD_EDITABLE_UINT("Max threshold", &batteryVoltageField.editable.number.config_error_threshold, "", 0, 2000, .div_digits = 1, .inc_step = 10),
    FIELD_EDITABLE_UINT("Min threshold", &batteryVoltageField.editable.number.config_warn_threshold, "", 0, 2000, .div_digits = 1, .inc_step = 10),
  FIELD_END };

static Field varBatteryCurrentMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[7].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[7].max, "", 0, 50, .inc_step = 1),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[7].min, "", 0, 50, .inc_step = 1),
    FIELD_EDITABLE_ENUM("Thresholds", &batteryCurrentField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
    FIELD_EDITABLE_UINT("Max threshold", &batteryCurrentField.editable.number.config_error_threshold, "", 0, 2000, .div_digits = 1, .inc_step = 10),
    FIELD_EDITABLE_UINT("Min threshold", &batteryCurrentField.editable.number.config_warn_threshold, "", 0, 2000, .div_digits = 1, .inc_step = 10),
  FIELD_END };

static Field varBatterySOCMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[8].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[8].max, "", 0, 100, .inc_step = 1),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[8].min, "", 0, 100, .inc_step = 1),
    FIELD_EDITABLE_ENUM("Thresholds", &batterySOCField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
    FIELD_EDITABLE_UINT("Max threshold", &batterySOCField.editable.number.config_error_threshold, "", 0, 200, .div_digits = 1, .inc_step = 1),
    FIELD_EDITABLE_UINT("Min threshold", &batterySOCField.editable.number.config_warn_threshold, "", 0, 200, .div_digits = 1, .inc_step = 1),
  FIELD_END };

static Field varMotorTempMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[9].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[9].max, "C", 0, 200, .inc_step = 1),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[9].min, "C", 0, 200, .inc_step = 1),
    FIELD_EDITABLE_ENUM("Thresholds", &motorTempField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
    FIELD_EDITABLE_UINT("Max threshold", &motorTempField.editable.number.config_error_threshold, "C", 0, 200, .div_digits = 1, .inc_step = 1),
    FIELD_EDITABLE_UINT("Min threshold", &motorTempField.editable.number.config_warn_threshold, "C", 0, 200, .div_digits = 1, .inc_step = 1),
  FIELD_END };

static Field varMotorERPSMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[10].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[10].max, "", 0, 2000, .inc_step = 1),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[10].min, "", 0, 2000, .inc_step = 1),
    FIELD_EDITABLE_ENUM("Thresholds", &motorErpsField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
    FIELD_EDITABLE_UINT("Max threshold", &motorErpsField.editable.number.config_error_threshold, "", 0, 2000, .div_digits = 1, .inc_step = 1),
    FIELD_EDITABLE_UINT("Min threshold", &motorErpsField.editable.number.config_warn_threshold, "", 0, 2000, .div_digits = 1, .inc_step = 1),
  FIELD_END };

static Field varMotorPWMMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[11].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[11].max, "", 0, 255, .inc_step = 1),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[11].min, "", 0, 255, .inc_step = 1),
    FIELD_EDITABLE_ENUM("Thresholds", &pwmDutyField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
    FIELD_EDITABLE_UINT("Max threshold", &pwmDutyField.editable.number.config_error_threshold, "", 0, 500, .div_digits = 1, .inc_step = 1),
    FIELD_EDITABLE_UINT("Min threshold", &pwmDutyField.editable.number.config_warn_threshold, "", 0, 500, .div_digits = 1, .inc_step = 1),
  FIELD_END };

static Field varMotorFOCMenus[] = {
    FIELD_EDITABLE_ENUM("Graph auto max min", &g_graphVars[12].auto_max_min, "yes", "no"),
    FIELD_EDITABLE_UINT("Graph max", &g_graphVars[12].max, "", 0, 60, .inc_step = 1),
    FIELD_EDITABLE_UINT("Graph min", &g_graphVars[12].min, "", 0, 60, .inc_step = 1),
    FIELD_EDITABLE_ENUM("Thresholds", &motorFOCField.editable.number.auto_thresholds, "disabled", "manual", "auto"),
    FIELD_EDITABLE_UINT("Max threshold", &motorFOCField.editable.number.config_error_threshold, "", 0, 120, .div_digits = 1, .inc_step = 1),
    FIELD_EDITABLE_UINT("Min threshold", &motorFOCField.editable.number.config_warn_threshold, "", 0, 120, .div_digits = 1, .inc_step = 1),
  FIELD_END };

static Field variablesMenus[] = {
  FIELD_SCROLLABLE("Speed", varSpeedMenus),
  FIELD_SCROLLABLE("Trip distance", varTripDistanceMenus),
  FIELD_SCROLLABLE("Odometer", varOdoMenus),
  FIELD_SCROLLABLE("Cadence", varCadenceMenus),
  FIELD_SCROLLABLE("Human power", varHumanPowerMenus),
  FIELD_SCROLLABLE("Battery power", varBatteryPowerMenus),
  FIELD_SCROLLABLE("Battery voltage", varBatteryVoltageMenus),
  FIELD_SCROLLABLE("Battery current", varBatteryCurrentMenus),
  FIELD_SCROLLABLE("Battery SOC", varBatterySOCMenus),
  FIELD_SCROLLABLE("Motor temperature", varMotorTempMenus),
  FIELD_SCROLLABLE("Motor speed", varMotorERPSMenus),
  FIELD_SCROLLABLE("Motor pwm dut-cycle", varMotorPWMMenus),
  FIELD_SCROLLABLE("Motor FOC", varMotorFOCMenus),
  FIELD_END };
#endif

static Field technicalMenus[] = {
  FIELD_READONLY_UINT("ADC throttle sensor", &ui_vars.ui8_adc_throttle, ""),
  FIELD_READONLY_UINT("Throttle sensor", &ui_vars.ui8_throttle, ""),
  FIELD_READONLY_UINT("ADC Torque sensor", &ui_vars.ui8_adc_pedal_torque_sensor, ""),
  FIELD_READONLY_UINT("Torque sensor", &ui_vars.ui8_pedal_torque_sensor, ""),
  FIELD_READONLY_UINT("Pedal cadence", &ui_vars.ui8_pedal_cadence, "rpm"),
  FIELD_READONLY_UINT("Pedal right", &ui_vars.ui8_pas_pedal_right, ""),
  FIELD_READONLY_UINT("Pedal Human power", &ui_vars.ui16_pedal_power_x10, "W", .div_digits = 1),
  FIELD_READONLY_UINT("PWM duty-cycle", &ui_vars.ui8_duty_cycle, ""),
  FIELD_READONLY_UINT("Motor speed", &ui_vars.ui16_motor_speed_erps, ""),
  FIELD_READONLY_UINT("Motor FOC", &ui_vars.ui8_foc_angle, ""),
  FIELD_READONLY_UINT("Hall sensors", &ui_vars.ui8_motor_hall_sensors, ""),
  FIELD_END };

static Field topMenus[] = {
  FIELD_SCROLLABLE("Wheel", wheelMenus),
  FIELD_SCROLLABLE("Battery", batteryMenus),
  FIELD_SCROLLABLE(_S("Battery SOC", "Bat SOC"), batterySOCMenus),
  FIELD_SCROLLABLE(_S("Assist level", "Assist"), assistMenus),
  FIELD_SCROLLABLE(_S("Walk assist", "Walk"), walkAssistMenus),
  FIELD_SCROLLABLE(_S("Startup BOOST", "Start Pwr"), startupPowerMenus),
  FIELD_SCROLLABLE(_S("Motor temperature", "Motor temp"), motorTempMenus),
#ifndef SW102
  FIELD_SCROLLABLE("Variables", variablesMenus),
#endif
  // FIELD_SCROLLABLE("Offroad", offroadMenus),
  FIELD_SCROLLABLE("Various", variousMenus),
  FIELD_SCROLLABLE("Display", displayMenus),
  FIELD_SCROLLABLE("Technical", technicalMenus),
  FIELD_END };

static Field configRoot = FIELD_SCROLLABLE(_S("Configurations", "Config"), topMenus);

uint8_t ui8_g_configuration_display_reset_to_defaults = 0;
uint32_t ui32_g_configuration_wh_100_percent = 0;

static void configScreenOnEnter() {
	// Set the font preference for this screen
	editable_label_font = &CONFIGURATIONS_TEXT_FONT;
	editable_value_font = &CONFIGURATIONS_TEXT_FONT;
	editable_units_font = &CONFIGURATIONS_TEXT_FONT;
}

static void configExit() {
	// save the variables on EEPROM
	eeprom_write_variables();
	set_conversions(); // we just changed units
}

static void configPreUpdate() {
  set_conversions(); // while in the config menu we might change units at any time - keep the display looking correct
}

//
// Screens
//
Screen configScreen = {
    .onExit = configExit,
    .onEnter = configScreenOnEnter,
    .onPreUpdate = configPreUpdate,

.fields = {
		{ .color = ColorNormal, .field = &configRoot },
		{ .field = NULL } } };
