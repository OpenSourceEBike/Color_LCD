/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include <math.h>
#include <string.h>
#include "stdio.h"
#include "main.h"
#include "utils.h"
#include "screen.h"
#include "rtc.h"
#include "fonts.h"
#include "uart.h"
#include "mainscreen.h"
#include "eeprom.h"
#include "buttons.h"
#include "lcd.h"
#include "adc.h"
#include "ugui.h"
#include "configscreen.h"

uint8_t ui8_m_wheel_speed_decimal;

static uint8_t ui8_walk_assist_state = 0;

uint16_t ui16_m_battery_current_filtered_x10;

void lcd_main_screen(void);
void warnings(void);
void walk_assist_state(void);
void power(void);
void time(void);
void wheel_speed(void);
void battery_soc(void);
void trip_time(void);
void wheel_speed(void);
void showNextScreen();
static bool renderWarning(FieldLayout *layout);
void DisplayResetToDefaults(void);
void onSetConfigurationBatteryTotalWh(uint32_t v);
void batteryTotalWh(void);
void batteryCurrent(void);

/// set to true if this boot was caused because we had a watchdog failure, used to show user the problem in the fault line
bool wd_failure_detected;

//
// Fields - these might be shared my multiple screens
//
Field socField = FIELD_DRAWTEXT();
Field timeField = FIELD_DRAWTEXT();
Field assistLevelField = FIELD_READONLY_UINT("assist", &l3_vars.ui8_assist_level, "");
#ifdef SW102
Field wheelSpeedIntegerField = FIELD_READONLY_UINT("speed", &l3_vars.ui16_wheel_speed_x10, "kph", .div_digits = 1, .hide_fraction = true);
#else
Field wheelSpeedIntegerField = FIELD_READONLY_UINT("speed", &l3_vars.ui16_wheel_speed_x10, "", .div_digits = 1, .hide_fraction = true);
#endif
Field wheelSpeedDecimalField = FIELD_READONLY_UINT("", &ui8_m_wheel_speed_decimal, "");
Field wheelSpeedField = FIELD_READONLY_UINT("speed", &l3_vars.ui16_wheel_speed_x10, "", .div_digits = 1);
Field tripTimeField = FIELD_READONLY_STRING("trip time", "unset");
Field tripDistanceField = FIELD_READONLY_UINT("trip distance", &l3_vars.ui32_trip_x10, "km", .div_digits = 1);
Field odoField = FIELD_READONLY_UINT("odometer", &l3_vars.ui32_odometer_x10, "km", .div_digits = 1);
Field cadenceField = FIELD_READONLY_UINT("cadence", &l3_vars.ui8_pedal_cadence_filtered, "rpm");
Field humanPowerField = FIELD_READONLY_UINT("human power", &l3_vars.ui16_pedal_power_filtered, "W");
Field batteryPowerField = FIELD_READONLY_UINT(_S("motor power", "motor pwr"), &l3_vars.ui16_battery_power_filtered, "W");
Field batteryVoltageField = FIELD_READONLY_UINT("battery voltage", &l3_vars.ui16_battery_voltage_filtered_x10, "", .div_digits = 1);
Field batteryCurrentField = FIELD_READONLY_UINT("battery current", &ui16_m_battery_current_filtered_x10, "", .div_digits = 1);
Field batterySOCField = FIELD_READONLY_UINT("battery SOC", &ui16_g_battery_soc_watts_hour, "%");
Field motorTempField = FIELD_READONLY_UINT("motor temperature", &l3_vars.ui8_motor_temperature, "C");
Field motorErpsField = FIELD_READONLY_UINT("motor speed", &l3_vars.ui16_motor_speed_erps, "");
Field pwmDutyField = FIELD_READONLY_UINT("pwm duty-cycle", &l3_vars.ui8_duty_cycle, "");
Field motorFOCField = FIELD_READONLY_UINT("motor foc", &l3_vars.ui8_foc_angle, "");
Field warnField = FIELD_CUSTOM(renderWarning);

/**
 * NOTE: The indexes into this array are stored in EEPROM, to prevent user confusion add new options only at the end.
 * If you remove old values, either warn users or bump up eeprom version to force eeprom contents to be discarded.
 */
Field *customizables[] = {
    &tripTimeField, // 0
    &tripDistanceField, // 1
    &odoField, // 2
    &wheelSpeedField, // 3
    &cadenceField, // 4
		&humanPowerField, // 5
		&batteryPowerField, // 6
    &batteryVoltageField, // 7
    &batteryCurrentField, // 8
    &batterySOCField, // 9
		&motorTempField, // 10
    &motorErpsField, // 11
		&pwmDutyField, // 12
		&motorFOCField, // 13
		NULL
};

// We currently don't have any graphs in the SW102, so leave them here until then
// kevinh: I think the following could be probably shared with the defs above (no need to copy and compute twice).  Also high chance of introducing bugs
// only in one place.
// Though I'm not sure why you need l2 vs l3 vars in this case.
Field wheelSpeedFieldGraph = FIELD_READONLY_UINT("speed", &l2_vars.ui16_wheel_speed_x10, "", .div_digits = 1);
Field tripDistanceFieldGraph = FIELD_READONLY_UINT("trip distance", &l2_vars.ui32_trip_x10, "", .div_digits = 1);
Field odoFieldGraph = FIELD_READONLY_UINT("odometer", &l2_vars.ui32_odometer_x10, "", .div_digits = 1);
Field cadenceFieldGraph = FIELD_READONLY_UINT("cadence", &l2_vars.ui8_pedal_cadence_filtered, "");
Field humanPowerFieldGraph = FIELD_READONLY_UINT("human power", &l2_vars.ui16_pedal_power_filtered, "");
Field batteryPowerFieldGraph = FIELD_READONLY_UINT("motor power", &l2_vars.ui16_battery_power_filtered, "");
Field batteryVoltageFieldGraph = FIELD_READONLY_UINT("battery voltage", &l2_vars.ui16_battery_voltage_filtered_x10, "", .div_digits = 1);
Field batteryCurrentFieldGraph = FIELD_READONLY_UINT("battery current", &l2_vars.ui16_battery_current_filtered_x5, "", .div_digits = 1); // FIXME, change this to x10 so div_digits will work
Field batterySOCFieldGraph = FIELD_READONLY_UINT("battery SOC", &ui16_g_battery_soc_watts_hour, "");
Field motorTempFieldGraph = FIELD_READONLY_UINT("motor temperature", &l2_vars.ui8_motor_temperature, "");
Field motorErpsFieldGraph = FIELD_READONLY_UINT("motor speed", &l2_vars.ui16_motor_speed_erps, "");
Field pwmDutyFieldGraph = FIELD_READONLY_UINT("pwm duty-cycle", &l2_vars.ui8_duty_cycle, "");
Field motorFOCFieldGraph = FIELD_READONLY_UINT("motor foc", &l2_vars.ui8_foc_angle, "");

Field wheelSpeedGraph = FIELD_GRAPH(&wheelSpeedFieldGraph);
Field tripDistanceGraph = FIELD_GRAPH(&tripDistanceFieldGraph);
Field odoGraph = FIELD_GRAPH(&odoFieldGraph);
Field cadenceGraph = FIELD_GRAPH(&cadenceFieldGraph);
Field humanPowerGraph = FIELD_GRAPH(&humanPowerFieldGraph);
Field batteryPowerGraph = FIELD_GRAPH(&batteryPowerFieldGraph);
Field batteryVoltageGraph = FIELD_GRAPH(&batteryVoltageFieldGraph, .min_threshold = -1, .warn_threshold = -1, .error_threshold = -1);
Field batteryCurrentGraph = FIELD_GRAPH(&batteryCurrentFieldGraph, .filter = FilterSquare);
Field batterySOCGraph = FIELD_GRAPH(&batterySOCFieldGraph);
Field motorTempGraph = FIELD_GRAPH(&motorTempFieldGraph);
Field motorErpsGraph = FIELD_GRAPH(&motorErpsFieldGraph);
Field pwmDutyGraph = FIELD_GRAPH(&pwmDutyFieldGraph);
Field motorFOCGraph = FIELD_GRAPH(&motorFOCFieldGraph);

// Note: the number of graphs in this collection must equal GRAPH_VARIANT_SIZE (for now)
Field graphs = FIELD_CUSTOMIZABLE(&l3_vars.field_selectors[0],
                                  &wheelSpeedGraph,
                                  &tripDistanceGraph,
                                  &odoGraph,
                                  &cadenceGraph,
                                  &humanPowerGraph,
                                  &batteryPowerGraph,
                                  &batteryVoltageGraph,
                                  &batteryCurrentGraph,
                                  &batterySOCGraph,
                                  &motorTempGraph,
                                  &motorErpsGraph,
                                  &pwmDutyGraph,
                                  &motorFOCGraph);
Field *activeGraphs = NULL; // set only once graph data is safe to read

// Note: field_selectors[0] is used on the 850C for the graphs selector
Field custom1 = FIELD_CUSTOMIZABLE_PTR(&l3_vars.field_selectors[1], customizables);
Field custom2 = FIELD_CUSTOMIZABLE_PTR(&l3_vars.field_selectors[2], customizables);
Field custom3 = FIELD_CUSTOMIZABLE_PTR(&l3_vars.field_selectors[3], customizables);
Field custom4 = FIELD_CUSTOMIZABLE_PTR(&l3_vars.field_selectors[4], customizables);


Field bootHeading = FIELD_DRAWTEXTPTR(_S("OpenSource EBike", "OS-EBike"));
Field bootURL_1 = FIELD_DRAWTEXTPTR(_S("www.github.com/", "see github.com"));
Field bootURL_2 = FIELD_DRAWTEXTPTR(_S("OpenSource-EBike-Firmware", "see github.com"));
Field bootFirmwareVersion = FIELD_DRAWTEXTPTR("850C firmware version:");
Field bootVersion = FIELD_DRAWTEXTPTR(VERSION_STRING);
Field bootStatus = FIELD_DRAWTEXT(.msg = "Booting...");

#define MIN_VOLTAGE_10X 140 // If our measured bat voltage (using ADC in the display) is lower than this, we assume we are running on a developers desk

static void bootScreenOnPreUpdate() {
	uint16_t bvolt = battery_voltage_10x_get();

	is_sim_motor = (bvolt < MIN_VOLTAGE_10X);
  if(is_sim_motor) {
    fieldPrintf(&bootStatus, "SIMULATING TSDZ2!");
    g_motorVariablesStabilized = true;
  }
  else if(has_seen_motor)
    fieldPrintf(&bootStatus, "Found TSDZ2");
  else
    fieldPrintf(&bootStatus, "Waiting TSDZ2 - (%u.%uV)", bvolt / 10, bvolt % 10);

  // Stop showing only after we release on/off button and we are commutication with motor
  if(buttons_get_onoff_state() == 0 && (has_seen_motor || is_sim_motor))
    showNextScreen();
}

Screen bootScreen = {
  .onPreUpdate = bootScreenOnPreUpdate,

  .fields = {
    {
      .x = 0, .y = YbyEighths(1), .height = -1,
      .field = &bootHeading,
      .font = &REGULAR_TEXT_FONT,
    },

    {
      .x = 0, .y = -20, .height = -1,
      .field = &bootURL_1,
      .font = &SMALL_TEXT_FONT,
    },

    {
      .x = 0, .y = -6, .height = -1,
      .field = &bootURL_2,
      .font = &SMALL_TEXT_FONT,
    },
#ifndef SW102
    {
      .x = 0, .y = YbyEighths(4), .height = -1,
      .field = &bootFirmwareVersion,
      .font = &SMALL_TEXT_FONT,
    },
#endif
    {
      .x = 0, .y = -8, .height = -1,
      .field = &bootVersion,
      .font = &SMALL_TEXT_FONT,
    },
    {
      .x = 0, .y = YbyEighths(6), .height = -1,
      .field = &bootStatus,
      .font = &SMALL_TEXT_FONT,
    },
    {
      .field = NULL
    }
  }
};

// Allow common operations (like walk assist and headlights) button presses to work on any page
bool anyscreen_onpress(buttons_events_t events) {
  if ((events & DOWN_LONG_CLICK) && l3_vars.ui8_walk_assist_feature_enabled) {
    ui8_walk_assist_state = 1;
    return true;
  }

  // long up to turn on headlights
  if (events & UP_LONG_CLICK) {
    l3_vars.ui8_lights = !l3_vars.ui8_lights;
    set_lcd_backlight();

    return true;
  }

  return false;
}

bool mainscreen_onpress(buttons_events_t events) {
	if(anyscreen_onpress(events))
	  return true;

	if (events & UP_CLICK /* &&
	 m_lcd_vars.ui8_lcd_menu_max_power == 0 */) {
		l3_vars.ui8_assist_level++;

		if (l3_vars.ui8_assist_level > l3_vars.ui8_number_of_assist_levels) {
			l3_vars.ui8_assist_level = l3_vars.ui8_number_of_assist_levels;
		}

		return true;
	}

	if (events & DOWN_CLICK /* &&
	 m_lcd_vars.ui8_lcd_menu_max_power == 0 */) {
		if (l3_vars.ui8_assist_level > 0)
			l3_vars.ui8_assist_level--;

		return true;
	}

	return false;
}


void set_conversions() {
  screenConvertMiles = l3_vars.ui8_units_type != 0; // Set initial value on unit conversions (FIXME, move this someplace better)
  screenConvertFarenheit = screenConvertMiles; // FIXME, should be based on a different eeprom config value
}

void lcd_main_screen(void) {
	time();
	walk_assist_state();
//  offroad_mode();
	power();
	battery_soc();
	battery_display();
	warnings();
	trip_time();
	wheel_speed();
}

void wheel_speed(void)
{
  // limit otherwise at startup this value goes crazy
  if(l3_vars.ui16_wheel_speed_x10 > 999) {
    l3_vars.ui16_wheel_speed_x10 = 999;
  }

  // Note: no need to check for 'wheel speed previous' because this math is so cheap
  ui8_m_wheel_speed_decimal = (uint8_t) (l3_vars.ui16_wheel_speed_x10 % 10);
}

void power(void) {
#if 0

  if(!m_lcd_vars.ui8_lcd_menu_max_power)
  {
    _ui16_battery_power_filtered = l3_vars.ui16_battery_power_filtered;

    if((_ui16_battery_power_filtered != ui16_battery_power_filtered_previous) ||
        m_lcd_vars.ui32_main_screen_draw_static_info ||
        ui8_target_max_battery_power_state == 0)
    {
      ui16_battery_power_filtered_previous = _ui16_battery_power_filtered;
      ui8_target_max_battery_power_state = 1;

      if (_ui16_battery_power_filtered > 9999) { _ui16_battery_power_filtered = 9999; }

      power_number.ui32_number = _ui16_battery_power_filtered;
      power_number.ui8_refresh_all_digits = m_lcd_vars.ui32_main_screen_draw_static_info;
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
      m_lcd_vars.ui8_lcd_menu_max_power = 0;
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
#endif
}

void mainscreen_show(void) {
	screenShow(&mainScreen);
}

void screen_clock(void) {
	static uint8_t ui8_counter_100ms = 0;

	// every 100ms
	if (ui8_counter_100ms++ >= 4) {
		ui8_counter_100ms = 0;

		// receive data from layer 2 to layer 3
		// send data from layer 3 to layer 2
		ui32_g_layer_2_can_execute = 0;
		copy_layer_2_layer_3_vars();
		ui32_g_layer_2_can_execute = 1;
	}

	lcd_main_screen();

#ifndef SW102
  clock_time();
#endif
  DisplayResetToDefaults();
  batteryTotalWh();
  batteryCurrent();

	screenUpdate();
}

void trip_time(void) {
	rtc_time_t *p_time = rtc_get_time_since_startup();
	static int oldmin = -1; // used to prevent unneeded updates
	static char timestr[8]; // 12:13

	if(p_time->ui8_minutes != oldmin) {
		oldmin = p_time->ui8_minutes;
		sprintf(timestr, "%d:%02d", p_time->ui8_hours, p_time->ui8_minutes);
		updateReadOnlyStr(&tripTimeField, timestr);
	}
}

static ColorOp warnColor = ColorNormal;
static char warningStr[MAX_FIELD_LEN];

// We use a custom callback so we can reuse the standard drawtext code, but with a dynamically changing color
static bool renderWarning(FieldLayout *layout) {
	layout->color = warnColor;
	return renderDrawTextCommon(layout, warningStr);
}

static void setWarning(ColorOp color, const char *str) {
	warnColor = color;
	warnField.blink = (color == ColorError);
	warnField.dirty = (strcmp(str, warningStr) != 0);
	if(warnField.dirty)
		strncpy(warningStr, str, sizeof(warningStr));
}


#define NO_ERROR                                0
#define ERROR_MOTOR_BLOCKED                     1
#define ERROR_TORQUE_APPLIED_DURING_POWER_ON    2
#define ERROR_BRAKE_APPLIED_DURING_POWER_ON     3
#define ERROR_THROTTLE_APPLIED_DURING_POWER_ON  4
#define ERROR_NO_SPEED_SENSOR_DETECTED          5
#define ERROR_LOW_CONTROLLER_VOLTAGE            6 // controller works with no less than 15 V so give error code if voltage is too low

static const char *motorErrors[] = { "None", "Motor Blocked", "Torque Fault", "Brake Fault", "Throttle Fault", "Speed Fault", "Low Volt" };

void warnings(void) {
	// High priorty faults in red

	if(l3_vars.ui8_error_states) {
		const char *str = (l3_vars.ui8_error_states > ERROR_MAX) ? "Unknown Motor" : motorErrors[l3_vars.ui8_error_states];
		setWarning(ColorError, str);
		return;
	}

	if(l3_vars.ui8_motor_temperature >= l3_vars.ui8_motor_temperature_max_value_to_limit) {
		setWarning(ColorError, "Temp Shutdown");
		return;
	}

	// If we had a watchdog failure, show it forever - so user will report a bug
	if(wd_failure_detected) {
    setWarning(ColorError, "Report Bug!");
    return;
	}

	// warn faults in yellow
	if(l3_vars.ui8_motor_temperature >= l3_vars.ui8_motor_temperature_min_value_to_limit) {
		setWarning(ColorWarning, "Temp Warning");
		return;
	}

	// All of the following possible 'faults' are low priority

	if(l3_vars.ui8_braking) {
		setWarning(ColorNormal, "BRAKE");
		return;
	}

	if(l3_vars.ui8_walk_assist) {
		setWarning(ColorNormal, "WALK");
		return;
	}

	if(l3_vars.ui8_lights) {
		setWarning(ColorNormal, "LIGHT");
		return;
	}

	setWarning(ColorNormal, "");
}

void battery_soc(void) {
	if (l3_vars.ui8_battery_soc_enable)
		fieldPrintf(&socField, "%3d%%", ui16_g_battery_soc_watts_hour);
	else
		fieldPrintf(&socField, "%u.%1uV",
				l3_vars.ui16_battery_voltage_soc_x10 / 10,
				l3_vars.ui16_battery_voltage_soc_x10 % 10);
}


void time(void) {
	rtc_time_t *p_rtc_time = rtc_get_time();

	// force to be [0 - 12]
	if (l3_vars.ui8_units_type) { // FIXME, should be based on a different eeprom config value, just because someone is using mph doesn't mean they want 12 hr time
		if (p_rtc_time->ui8_hours > 12) {
			p_rtc_time->ui8_hours -= 12;
		}
	}

	fieldPrintf(&timeField, "%d:%02d", p_rtc_time->ui8_hours,
			p_rtc_time->ui8_minutes);
}

void walk_assist_state(void) {
	// kevinh - note on the sw102 we show WALK in the box normally used for BRAKE display - the display code is handled there now
	if (l3_vars.ui8_walk_assist_feature_enabled) {
		// if down button is still pressed
		if (ui8_walk_assist_state && buttons_get_down_state()) {
			l3_vars.ui8_walk_assist = 1;
		} else if (buttons_get_down_state() == 0) {
			ui8_walk_assist_state = 0;
			l3_vars.ui8_walk_assist = 0;
		}
	} else {
		ui8_walk_assist_state = 0;
		l3_vars.ui8_walk_assist = 0;
	}
}

// Screens in a loop, shown when the user short presses the power button
extern Screen *screens[];

void showNextScreen() {
	static int nextScreen;

	Screen *next = screens[nextScreen++];

	if (!next) {
		nextScreen = 0;
		next = screens[nextScreen++];
	}

	screenShow(next);
}

static bool appwide_onpress(buttons_events_t events)
{
  // power off only after we release first time the onoff button
  if (events & ONOFF_LONG_CLICK)
  {
    lcd_power_off(1);
    return true;
  }

  if(events & SCREENCLICK_NEXT_SCREEN) {
    showNextScreen();
    return true;
  }
	return false;
}

/// Called every 20ms to check for button events and dispatch to our handlers
static void handle_buttons() {

  static uint8_t firstTime = 1;

  // keep tracking of first time release of onoff button
  if(firstTime && buttons_get_onoff_state() == 0) {
    firstTime = 0;
    buttons_clear_onoff_click_event();
    buttons_clear_onoff_long_click_event();
    buttons_clear_onoff_click_long_click_event();
  }

  if (buttons_events && firstTime == 0)
  {
    bool handled = false;

		if (!handled)
			handled |= screenOnPress(buttons_events);

		// Note: this must be after the screen/menu handlers have had their shot
		if (!handled)
			handled |= appwide_onpress(buttons_events);

		if (handled)
			buttons_clear_all_events();
	}

	buttons_clock(); // Note: this is done _after_ button events is checked to provide a 20ms debounce
}

/// Call every 20ms from the main thread.
void main_idle() {
	static uint8_t ui8_100ms_timer_counter = 0;
	
	handle_buttons();
	screen_clock(); // This is _after_ handle_buttons so if a button was pressed this tick, we immediately update the GUI

	if (++ui8_100ms_timer_counter >= 5) {
		ui8_100ms_timer_counter = 0;
		automatic_power_off_management(); // Note: this was moved from layer_2() because it does eeprom operations which should not be used from ISR
	}
}

void batteryTotalWh(void) {

  ui32_g_configuration_wh_100_percent = l3_vars.ui32_wh_x10_100_percent / 10;
}

void onSetConfigurationBatteryTotalWh(uint32_t v) {

  l3_vars.ui32_wh_x10_100_percent = v * 10;
}

void DisplayResetToDefaults(void) {

  if (ui8_g_configuration_display_reset_to_defaults) {
    ui8_g_configuration_display_reset_to_defaults = 0;
    eeprom_init_defaults();
  }
}

void batteryCurrent(void) {

  ui16_m_battery_current_filtered_x10 = l3_vars.ui16_battery_current_filtered_x5 * 2;
}
