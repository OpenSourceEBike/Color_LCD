/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include <math.h>
#include "stdio.h"
#include "main.h"
#include "utils.h"
#include "screen.h"
#include "rtc.h"
#include "fonts.h"
#include "config.h"
#include "uart.h"
#include "mainscreen.h"
#include "eeprom.h"
#include "buttons.h"
#include "lcd.h"
#include "adc.h"
#include "ugui.h"
#include "configscreen.h"

//
// Fields - these might be shared my multiple screens
//
Field socField = FIELD_DRAWTEXT();
Field batteryField = FIELD_DRAWTEXT();
Field timeField = FIELD_DRAWTEXT();
Field speedField = FIELD_READONLY_UINT("Speed", &l3_vars.ui16_wheel_speed_x10, "kph", .div_digits = 1, .hide_fraction = true);
Field assistLevelField = FIELD_READONLY_UINT("Assist", &l3_vars.ui8_assist_level, "");
Field maxPowerField = FIELD_READONLY_UINT("Motor Pwr", &l3_vars.ui16_battery_power_filtered, "W");
Field humanPowerField = FIELD_READONLY_UINT("Human Pwr", &l3_vars.ui16_pedal_power_filtered, "W");
//Field whiteFillField = { .variant = FieldFill };
//Field meshFillField = { .variant = FieldMesh };
Field warnField = FIELD_DRAWTEXT();

Field tripTimeField = FIELD_DRAWTEXT();
Field tripDistanceField = FIELD_READONLY_UINT("Trip", &l3_vars.ui32_trip_x10, "km", .div_digits = 1);
Field odoField = FIELD_READONLY_UINT("ODO", &l3_vars.ui32_odometer_x10, "km", .div_digits = 1);
Field motorTempField = FIELD_DRAWTEXT();

static uint8_t ui8_walk_assist_state = 0;


void lcd_main_screen(void);
void brake(void);
void walk_assist_state(void);
void power(void);
void temperature(void);
void time(void);
void battery_soc(void), battery_display();
void trip_time(void);

Field bootHeading = FIELD_DRAWTEXT(.msg = "OpenSource EBike");
Field bootVersion = FIELD_DRAWTEXT(.msg = VERSION_STRING);
Field bootStatus = FIELD_DRAWTEXT(.msg = "Booting...");



Screen bootScreen = {
    .fields = {
    {
        .x = 0, .y = 0,
        .field = &bootHeading,
        .font = &REGULAR_TEXT_FONT,
    },
    {
        .x = 0, .y = 32,
        .field = &bootVersion,
        .font = &REGULAR_TEXT_FONT,
    },
    {
        .x = 0, .y = 80,
        .field = &bootStatus,
        .font = &REGULAR_TEXT_FONT,
    },
    {
        .field = NULL
    }
    }};



bool mainscreen_onpress(buttons_events_t events) {
  if((events & DOWN_LONG_CLICK) && l3_vars.ui8_walk_assist_feature_enabled)
  {
    ui8_walk_assist_state = 1;
    return true;
  }

  // long up to turn on headlights
  if(events & UP_LONG_CLICK)
  {
	  l3_vars.ui8_lights = !l3_vars.ui8_lights;
	set_lcd_backlight();

    return true;
  }

  if (events & UP_CLICK /* &&
      m_lcd_vars.ui8_lcd_menu_max_power == 0 */)
  {
    l3_vars.ui8_assist_level++;

    if (l3_vars.ui8_assist_level > l3_vars.ui8_number_of_assist_levels)
      { l3_vars.ui8_assist_level = l3_vars.ui8_number_of_assist_levels; }

    return true;
  }

  if (events & DOWN_CLICK /* &&
      m_lcd_vars.ui8_lcd_menu_max_power == 0 */)
  {
    if (l3_vars.ui8_assist_level > 0)
      l3_vars.ui8_assist_level--;

    return true;
  }

  return false;
}

// Used to define  positions in terms of # of 1/8ths of screen width/height (i.e. 4 is middle, 3 is slightly to left etc)
#define XbyEighths(n) ((SCREEN_WIDTH * (n)) / 8)
#define YbyEighths(n) ((SCREEN_HEIGHT * (n)) / 8)

#ifndef SW102

/**
 * Appears at the bottom of all screens, includes status msgs or critical fault alerts
 * FIXME - get rid of this nasty define - instead add the concept of Subscreens, so that the battery bar
 * at the top and the status bar at the bottom can be shared across all screens
 */
#define STATUS_BAR \
{ \
    .x = 4, .y = 114, \
    .width = 0, .height = -1, \
    .field = &warnField, \
    .font = &REGULAR_TEXT_FONT, \
}

#define BATTERY_BAR \
    { \
        .x = 0, .y = 0, \
        .width = -1, .height = -1, \
        .field = &batteryField, \
        .font = &MY_FONT_BATTERY, \
    }, \
    { \
        .x = XbyEighths(2), .y = 0, \
        .width = -5, .height = -1, \
        .font = &REGULAR_TEXT_FONT, \
        .field = &socField \
    }, \
	{ \
		.x = XbyEighths(5), .y = 0, \
		.width = -5, .height = -1, \
		.font = &REGULAR_TEXT_FONT, \
		.field = &timeField \
	}


//
// Screens
//
Screen mainScreen = {
    .onPress = mainscreen_onpress,

    .fields = {
    BATTERY_BAR,
    {
        .x = 0, .y = 32,
        .width = -1, .height = -1,
        .field = &assistLevelField,
        .font = &BIG_NUMBERS_TEXT_FONT,
        .modifier = ModLabelTop,
        .border = BorderNone
    },
    {
        .x = XbyEighths(3), .y = 32,
        .width = 0, .height = -1,
        .field = &speedField,
        .font = &HUGE_NUMBERS_TEXT_FONT,
        .modifier = ModLabelTop,
        .border = BorderNone
    },
    {
        .x = 0, .y = YbyEighths(3),
        .width = XbyEighths(4), .height = YbyEighths(1),
        .field = &tripDistanceField,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .modifier = ModLabelTop,
        .border = BorderBottom | BorderRight | BorderTop
    },
    {
        .x = XbyEighths(4), .y = YbyEighths(3),
        .width = XbyEighths(4), .height = YbyEighths(1),
        .field = &maxPowerField,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .modifier = ModLabelTop,
        .border = BorderBottom | BorderTop
    },
    {
        .x = 0, .y = YbyEighths(4),
        .width = XbyEighths(4), .height = YbyEighths(1),
        .field = &tripTimeField,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .modifier = ModLabelTop,
        .border = BorderBottom | BorderRight
    },
    {
        .x = XbyEighths(4), .y = YbyEighths(4),
        .width = XbyEighths(4), .height = YbyEighths(1),
        .field = &humanPowerField,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .modifier = ModLabelTop,
        .border = BorderBottom
    },
    STATUS_BAR,
    {
        .field = NULL
    } }
};

Screen infoScreen = {
    // .onPress = mainscreen_onpress,

    .fields = {
    BATTERY_BAR,
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &motorTempField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &humanPowerField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &tripDistanceField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &odoField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    STATUS_BAR,
    {
        .field = NULL
    } }
};

#else


/**
 * Appears at the bottom of all screens, includes status msgs or critical fault alerts
 * FIXME - get rid of this nasty define - instead add the concept of Subscreens, so that the battery bar
 * at the top and the status bar at the bottom can be shared across all screens
 */
#define STATUS_BAR \
{ \
    .x = 4, .y = 114, \
    .width = 0, .height = -1, \
    .field = &warnField, \
    .font = &REGULAR_TEXT_FONT, \
}

#define BATTERY_BAR \
    { \
        .x = 0, .y = 0, \
        .width = -1, .height = -1, \
        .field = &batteryField, \
        .font = &MY_FONT_BATTERY, \
    }, \
    { \
        .x = 32, .y = 0, \
        .width = -5, .height = -1, \
        .font = &REGULAR_TEXT_FONT, \
        .field = &socField \
    }
/*
{
    .x = 32, .y = 0,
    .width = -5, .height = -1,
    .field = &tripTimeField
},
*/

//
// Screens
//
Screen mainScreen = {
    .onPress = mainscreen_onpress,

    .fields = {
    BATTERY_BAR,
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &assistLevelField,
        .font = &BIG_NUMBERS_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    /*
    {
        .x = 19, .y = 16,
        .width = -2, .height = -1,
        // .color = ColorInvert,
        .field = &speedField,
        .border = BorderNone
    }, */
    {
        .x = 0, .y = -3,
        .width = 0, .height = 19,
        .field = &maxPowerField,
        .font = &MEDIUM_NUMBERS_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -3,
        .width = 0, .height = -1,
        .field = &speedField,
        .font = &BIG_NUMBERS_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderNone
    },
    STATUS_BAR,
    {
        .field = NULL
    } }
};

Screen infoScreen = {
    // .onPress = mainscreen_onpress,

    .fields = {
    BATTERY_BAR,
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &motorTempField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &humanPowerField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &tripDistanceField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &odoField,
        .font = &REGULAR_TEXT_FONT,
        .modifier = ModNoLabel,
        .border = BorderBottom
    },
    STATUS_BAR,
    {
        .field = NULL
    } }
};

#endif


void lcd_main_screen(void)
{
  time();
  walk_assist_state();
//  offroad_mode();
  power();
  temperature();
  battery_soc();
  battery_display();
  brake();
  trip_time();
}



void power(void)
{
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

void screen_clock(void)
{
  static uint8_t ui8_counter_100ms = 0;

  // every 100ms
  if(ui8_counter_100ms++ >= 4)
  {
    ui8_counter_100ms = 0;

    // receive data from layer 2 to layer 3
    // send data from layer 3 to layer 2
    ui32_g_layer_2_can_execute = 0;
    copy_layer_2_layer_3_vars();
    ui32_g_layer_2_can_execute = 1;
  }

  lcd_main_screen();
  screenUpdate();

#if 0
  // must be reset after a full cycle of lcd_clock()
  ui32_m_draw_graphs_1 = 0;
  ui32_m_draw_graphs_2 = 0;
#endif
}



void trip_time(void)
{
  struct_rtc_time_t *p_time;

  p_time = rtc_get_time_since_startup();

  fieldPrintf(&tripTimeField, "%02d:%02d",  p_time->ui8_hours,  p_time->ui8_minutes);
}




void brake(void)
{
  fieldPrintf(&warnField, l3_vars.ui8_braking ? "BRAKE" : (l3_vars.ui8_walk_assist ? "WALK" : (l3_vars.ui8_lights ? "LIGH" : "")));
}




void battery_soc(void)
{
  if (l3_vars.ui8_battery_soc_enable)
    fieldPrintf(&socField, "%3d%%", ui16_m_battery_soc_watts_hour);
  else
    fieldPrintf(&socField, "%2u.%1uV", l3_vars.ui16_battery_voltage_soc_x10 / 10, l3_vars.ui16_battery_voltage_soc_x10 % 10);
}

// Show our battery graphic
void battery_display() {
  uint8_t ui32_battery_bar_number = l3_vars.volt_based_soc / (90 / 5); // scale SOC so anything greater than 90% is 5 bars, and zero is zero.

  fieldPrintf(&batteryField, "%d", ui32_battery_bar_number);
}

void temperature(void)
{
  if(l3_vars.ui8_temperature_limit_feature_enabled)
  {
    fieldPrintf(&motorTempField, "%dC", l3_vars.ui8_motor_temperature);
  }
  else {
    fieldPrintf(&motorTempField, "no temp");
  }
}

void time(void)
{
  struct_rtc_time_t *p_rtc_time = rtc_get_time();

  // force to be [0 - 12]
  if(l3_vars.ui8_units_type)
  {
    if(p_rtc_time->ui8_hours > 12)
    {
      p_rtc_time->ui8_hours -= 12;
    }
  }

  fieldPrintf(&timeField, "%02d:%02d",  p_rtc_time->ui8_hours,  p_rtc_time->ui8_minutes);
}



void walk_assist_state(void)
{
  // kevinh - note on the sw102 we show WALK in the box normally used for BRAKE display - the display code is handled there now
  if(l3_vars.ui8_walk_assist_feature_enabled)
  {
    // if down button is still pressed
    if(ui8_walk_assist_state &&
        buttons_get_down_state())
    {
      l3_vars.ui8_walk_assist = 1;
    }
    else if(buttons_get_down_state() == 0)
    {
      ui8_walk_assist_state = 0;
      l3_vars.ui8_walk_assist = 0;
    }
  }
  else
  {
    ui8_walk_assist_state = 0;
    l3_vars.ui8_walk_assist = 0;
  }
}




// Screens in a loop, shown when the user short presses the power button
static Screen *screens[] = {
	&mainScreen,
	&configScreen,
    &infoScreen,
    NULL
};

static int nextScreen = 0;

void showNextScreen() {
  Screen *next = screens[nextScreen++];

  if(!next) {
    nextScreen = 0;
    next = screens[nextScreen++];
  }

  screenShow(next);
}


static bool appwide_onpress(buttons_events_t events)
{
  if (events & ONOFF_LONG_CLICK)
  {
    lcd_power_off(1);
    return true;
  }

  if(events & SCREEMCLICK_NEXT_SCREEN) {
    showNextScreen();
    return true;
  }

  return false;
}

/// Called every 20ms to check for button events and dispatch to our handlers
static void handle_buttons() {
  if (buttons_events)
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

#define MIN_VOLTAGE_10X 140 // If our measured bat voltage (using ADC in the display) is lower than this, we assume we are running on a developers desk


/// Call every 20ms from the main thread.
void main_idle() {
	static uint32_t start_time;

	if(start_time == 0)
		start_time = ui32_seconds_since_startup;

    screen_clock();
    handle_buttons();
    automatic_power_off_management(); // Note: this was moved from layer_2() because it does eeprom operations which should not be used from ISR

    if(getCurrentScreen() == &bootScreen) { // FIXME move this into an onIdle callback on the screen
      uint16_t bvolt = battery_voltage_10x_get();

      is_sim_motor = (bvolt < MIN_VOLTAGE_10X);

      if(is_sim_motor)
        fieldPrintf(&bootStatus, "SIMULATING motor!");
      else if(has_seen_motor)
        fieldPrintf(&bootStatus, "Found motor");
      else
        fieldPrintf(&bootStatus, "No motor? (%u.%uV)", bvolt / 10, bvolt % 10);

      // Stop showing the boot screen after a few seconds (once we've found a motor)
      if(ui32_seconds_since_startup - start_time >= 5 && (has_seen_motor || is_sim_motor))
        showNextScreen();
    }
}


#if 0 // kevinh possibly repurpose in graph render widget
void graphs_measurements_update(void)
{
  static uint32_t counter = 0;
//  static uint8_t ui8_first_time = 1;
  static uint8_t ui8_first_time = 0;
  static uint32_t ui32_pedal_power_accumulated = 0;

#ifndef SIMULATION
  if(ui8_first_time &&
      l2_vars.ui8_motor_temperature != 0)
  {
    ui8_first_time = 0;
  }

  if(ui8_first_time == 0)
  {
    // apply the same low pass filter as for the value show to user
    ui32_pedal_power_accumulated -= ui32_pedal_power_accumulated >> PEDAL_POWER_FILTER_COEFFICIENT;
    ui32_pedal_power_accumulated += (uint32_t) l2_vars.ui16_pedal_power_x10 / 10;

    // sum the value
    m_p_graphs[0].measurement.ui32_sum_value += ((uint32_t) (ui32_pedal_power_accumulated >> PEDAL_POWER_FILTER_COEFFICIENT));

    // every 3.5 seconds, update the graph array values
    if(++counter >= 35)
    {
      if(m_p_graphs[0].measurement.ui32_sum_value)
      {
        /*store the average value on the 3.5 seconds*/
        m_p_graphs[0].ui32_data_y_last_value = m_p_graphs[0].measurement.ui32_sum_value / counter;
        m_p_graphs[0].measurement.ui32_sum_value = 0;
      }
      else
      {
        /*store the average value on the 3.5 seconds*/
        m_p_graphs[0].ui32_data_y_last_value = 0;
        m_p_graphs[0].measurement.ui32_sum_value = 0;
      }

      m_p_graphs[0].ui32_data_y_last_value_previous = m_p_graphs[0].ui32_data_y_last_value;

      counter = 0;

      // signal to draw graphs on main loop
      ui32_m_draw_graphs_1 = 1;
    }
  }
#else
  // every 0.5 second
  counter++;
  if(counter >= 2)
  {
    m_p_graphs[0].ui32_data_y_last_value = l2_vars.ui16_pedal_power_filtered;

    if(l2_vars.ui16_pedal_power_filtered == 0)
    {
      m_p_graphs[0].ui32_data_y_last_value++;
    }

    // signal to draw graphs on main loop
    ui32_m_draw_graphs_1 = 1;

    counter = 0;
  }
#endif
}
#endif



#if 0
void update_menu_flashing_state(void)
{

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
#endif

