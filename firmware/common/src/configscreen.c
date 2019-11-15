#include "screen.h"
#include "mainscreen.h"
#include "configscreen.h"
#include "eeprom.h"

static Field wheelMenus[] =
		{
						FIELD_EDITABLE_UINT("Max speed", &l3_vars.wheel_max_speed_x10, "kph", 1, 990, .div_digits = 1, .inc_step = 10, .hide_fraction = true),
						FIELD_EDITABLE_UINT("Circumference", &l3_vars.ui16_wheel_perimeter, "mm", 750, 3000, .inc_step = 10),
				FIELD_END };

static Field batteryMenus[] =
		{
						FIELD_EDITABLE_UINT("Max current", &l3_vars.ui8_battery_max_current, "amps", 1, 30),
						FIELD_EDITABLE_UINT("Current ramp", &l3_vars.ui8_ramp_up_amps_per_second_x10, "amps", 4, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Low cut-off", &l3_vars.ui16_battery_low_voltage_cut_off_x10, "volts", 160, 630, .div_digits = 1),
						FIELD_EDITABLE_UINT("Num cells", &l3_vars.ui8_battery_cells_number, "", 7, 14),
						FIELD_EDITABLE_UINT("Resistance", &l3_vars.ui16_battery_pack_resistance_x1000, "mohm", 0, 1000),
						FIELD_READONLY_UINT("Voltage", &l3_vars.ui16_battery_voltage_soc_x10, "volts", .div_digits = 1),
				FIELD_END };

static Field batterySOCMenus[] =
		{
						FIELD_EDITABLE_ENUM("Feature", &l3_vars.ui8_battery_soc_enable, "disable", "enable"),
						FIELD_EDITABLE_ENUM("Show", &l3_vars.ui8_battery_soc_increment_decrement, "% full", "% used"),
						FIELD_EDITABLE_UINT(_S("Reset at voltage", "Reset at"), &l3_vars.ui16_battery_voltage_reset_wh_counter_x10, "volts", 160, 630, .div_digits = 1),
						FIELD_EDITABLE_UINT(_S("Battery total Wh", "Battery total"), &l3_vars.ui32_wh_x10_100_percent, "whr", 0, 9990, .inc_step = 10),
						FIELD_EDITABLE_UINT("Used Wh", &l3_vars.ui32_wh_x10_offset, "whr", 0, 99900, .div_digits = 1, .inc_step = 100),
				FIELD_END };

static Field assistMenus[] =
		{
						FIELD_EDITABLE_UINT(_S("Num assist levels", "Num Levels"), &l3_vars.ui8_number_of_assist_levels, "", 1, 9),
						FIELD_EDITABLE_UINT("Level 1", &l3_vars.ui8_assist_level_factor[0], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 2", &l3_vars.ui8_assist_level_factor[1], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 3", &l3_vars.ui8_assist_level_factor[2], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 4", &l3_vars.ui8_assist_level_factor[3], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 5", &l3_vars.ui8_assist_level_factor[4], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 6", &l3_vars.ui8_assist_level_factor[5], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 7", &l3_vars.ui8_assist_level_factor[6], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 8", &l3_vars.ui8_assist_level_factor[7], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 9", &l3_vars.ui8_assist_level_factor[8], "", 0, 255, .div_digits = 1),
				FIELD_END };

static Field walkAssistMenus[] =
		{
						FIELD_EDITABLE_ENUM("Feature", &l3_vars.ui8_walk_assist_feature_enabled, "disable", "enable"), // FIXME, share one array of disable/enable strings
						FIELD_EDITABLE_UINT("Level 1", &l3_vars.ui8_walk_assist_level_factor[0], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 2", &l3_vars.ui8_walk_assist_level_factor[1], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 3", &l3_vars.ui8_walk_assist_level_factor[2], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 4", &l3_vars.ui8_walk_assist_level_factor[3], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 5", &l3_vars.ui8_walk_assist_level_factor[4], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 6", &l3_vars.ui8_walk_assist_level_factor[5], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 7", &l3_vars.ui8_walk_assist_level_factor[6], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 8", &l3_vars.ui8_walk_assist_level_factor[7], "", 0, 100),
						FIELD_EDITABLE_UINT("Level 9", &l3_vars.ui8_walk_assist_level_factor[8], "", 0, 100),
				FIELD_END };

static Field startupPowerMenus[] =
		{
						FIELD_EDITABLE_ENUM("Feature", &l3_vars.ui8_startup_motor_power_boost_feature_enabled, "disable", "enable"), // FIXME, share one array of disable/enable strings
						FIELD_EDITABLE_ENUM("Active on", &l3_vars.ui8_startup_motor_power_boost_always, "startup", "always"),
						FIELD_EDITABLE_ENUM("Limit to max-power", &l3_vars.ui8_startup_motor_power_boost_limit_power, "no", "yes"),
						FIELD_EDITABLE_UINT("Duration", &l3_vars.ui8_startup_motor_power_boost_time, "secs", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Fade", &l3_vars.ui8_startup_motor_power_boost_fade_time, "secs", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 1", &l3_vars.ui8_startup_motor_power_boost_factor[0], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 2", &l3_vars.ui8_startup_motor_power_boost_factor[1], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 3", &l3_vars.ui8_startup_motor_power_boost_factor[2], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 4", &l3_vars.ui8_startup_motor_power_boost_factor[3], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 5", &l3_vars.ui8_startup_motor_power_boost_factor[4], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 6", &l3_vars.ui8_startup_motor_power_boost_factor[5], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 7", &l3_vars.ui8_startup_motor_power_boost_factor[6], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 8", &l3_vars.ui8_startup_motor_power_boost_factor[7], "", 0, 255, .div_digits = 1),
						FIELD_EDITABLE_UINT("Level 9", &l3_vars.ui8_startup_motor_power_boost_factor[8], "", 0, 255, .div_digits = 1),
				FIELD_END };

static Field motorTempMenus[] =
		{
						FIELD_EDITABLE_ENUM("Feature", &l3_vars.ui8_temperature_limit_feature_enabled, "disable", "temperature", "throttle"), // FIXME, share one array of disable/enable strings
						FIELD_EDITABLE_UINT("Min limit", &l3_vars.ui8_motor_temperature_min_value_to_limit, "degC", 0, 125),
						FIELD_EDITABLE_UINT("Max limit", &l3_vars.ui8_motor_temperature_max_value_to_limit, "degC", 0, 125),
				FIELD_END };

static Field displayMenus[] =
		{
#ifndef SW102
		    FIELD_EDITABLE_UINT("Clock hours", &ui8_g_configuration_clock_hours, "", 0, 23, .onPreSetEditable = onSetConfigurationClockHours),
		    FIELD_EDITABLE_UINT("Clock minutes", &ui8_g_configuration_clock_minutes, "", 0, 59, .onPreSetEditable = onSetConfigurationClockMinutes),
        FIELD_EDITABLE_UINT("Brightness on", &l3_vars.ui8_lcd_backlight_on_brightness, "", 5, 100, .inc_step = 5, .onPreSetEditable = onSetConfigurationDisplayLcdBacklightOnBrightness),
        FIELD_EDITABLE_UINT("Brightness off", &l3_vars.ui8_lcd_backlight_off_brightness, "", 5, 100, .inc_step = 5, .onPreSetEditable = onSetConfigurationDisplayLcdBacklightOffBrightness),
        FIELD_EDITABLE_ENUM("Buttons invert", &l3_vars.ui8_buttons_up_down_invert, "default", "invert"),
#endif
		    FIELD_EDITABLE_UINT("Auto power off", &l3_vars.ui8_lcd_power_off_time_minutes, "mins", 0, 255),
        FIELD_EDITABLE_ENUM("Units", &l3_vars.ui8_units_type, "SI", "Imperial"),
		    FIELD_EDITABLE_ENUM("Reset to defaults", &ui8_g_configuration_display_reset_to_defaults, "no", "yes"),
				FIELD_END };

#if 0
static Field offroadMenus[] = {
    FIELD_EDITABLE_ENUM("Feature", &l3_vars.ui8_offroad_feature_enabled, "disable", "enable"), // FIXME, share one array of disable/enable strings
    FIELD_EDITABLE_ENUM("Active on start", &l3_vars.ui8_offroad_enabled_on_startup, "no", "yes"), // FIXME, share one array of disable/enable strings
    FIELD_EDITABLE_UINT("Speed limit", &l3_vars.ui8_offroad_speed_limit_x10, "kph", 1, 990, .div_digits = 10, .inc_step = 10, .hide_fraction = true),
    FIELD_EDITABLE_ENUM("Limit power", &l3_vars.ui8_offroad_power_limit_enabled, "no", "yes"), // FIXME, share one array of disable/enable strings
    FIELD_EDITABLE_UINT("Power limit", &l3_vars.ui8_offroad_power_limit_div25, "watt", 0, 2000), // huge FIXME - div25 can't work with this system, change it
    FIELD_END
};
#endif

static Field variousMenus[] =
		{
						FIELD_EDITABLE_ENUM("Motor voltage", &l3_vars.ui8_motor_type, "48V", "36V", "expert"),
						FIELD_EDITABLE_ENUM("Motor assist", &l3_vars.ui8_motor_assistance_startup_without_pedal_rotation, "disable", "enable"), // FIXME, share one array of disable/enable strings
				FIELD_END };

static Field technicalMenus[] =
		{
		    FIELD_READONLY_UINT("ADC throttle sensor", &l3_vars.ui8_adc_throttle, ""),
		    FIELD_READONLY_UINT("Throttle sensor", &l3_vars.ui8_throttle, ""),
		    FIELD_READONLY_UINT("ADC Torque sensor", &l3_vars.ui8_adc_pedal_torque_sensor, ""),
		    FIELD_READONLY_UINT("Torque sensor", &l3_vars.ui8_pedal_torque_sensor, ""),
		    FIELD_READONLY_UINT("Pedal cadence", &l3_vars.ui8_pedal_cadence, "rpm"),
		    FIELD_READONLY_UINT("Pedal Human power", &l3_vars.ui16_pedal_power_x10, "W", .div_digits = 1),
		    FIELD_READONLY_UINT("PWM duty-cycle", &l3_vars.ui8_duty_cycle, ""),
		    FIELD_READONLY_UINT("Motor speed", &l3_vars.ui16_motor_speed_erps, ""),
		    FIELD_READONLY_UINT("Motor FOC", &l3_vars.ui8_foc_angle, ""),
				FIELD_END };

static Field topMenus[] = {
FIELD_SCROLLABLE("Wheel", wheelMenus),
FIELD_SCROLLABLE("Battery", batteryMenus),
FIELD_SCROLLABLE(_S("Battery SOC", "Bat SOC"), batterySOCMenus),
FIELD_SCROLLABLE(_S("Assist level", "Assist"), assistMenus),
FIELD_SCROLLABLE("Walk", walkAssistMenus),
FIELD_SCROLLABLE(_S("Startup BOOST", "Start Pwr"), startupPowerMenus),
FIELD_SCROLLABLE(_S("Motor temperature", "Motor temp"), motorTempMenus),
FIELD_SCROLLABLE("Display", displayMenus),
// FIELD_SCROLLABLE("Offroad", offroadMenus),
		FIELD_SCROLLABLE("Various", variousMenus),
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


