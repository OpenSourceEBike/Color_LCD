/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include <eeprom_hw.h>
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
#include "adc.h"


// Battery SOC symbol:
// 10 bars, each bar: with = 7, height = 24
// symbol has contour lines of 1 pixel
#define BATTERY_SOC_START_X 8
#define BATTERY_SOC_START_Y 4
#define BATTERY_SOC_BAR_WITH 7
#define BATTERY_SOC_BAR_HEIGHT 24
#define BATTERY_SOC_CONTOUR 1



static uint8_t ui8_walk_assist_state = 0;



void lcd_main_screen(void);
uint8_t first_time_management(void);
void assist_level_state(void);
void power_off_management(void);
void lcd_power_off(uint8_t updateDistanceOdo);
void l2_low_pass_filter_battery_voltage_current_power(void);
void brake(void);
void walk_assist_state(void);
void wheel_speed(void);
void power(void);
void pedal_human_power(void);
void power_off_management(void);
void temperature(void);
void time(void);
void battery_soc(void), battery_display();
void l2_calc_battery_voltage_soc(void);
void l2_calc_wh(void);
void l2_low_pass_filter_pedal_torque_and_power(void);
//static void l2_low_pass_filter_pedal_cadence(void);
void lights_state(void);
void graphs_measurements_update(void);
void trip_distance(void);
void trip_time(void);

void lcd_set_backlight_intensity(uint8_t level) {
  // FIXME, implement
}

bool mainscreen_onpress(buttons_events_t events) {
  if((events & DOWN_LONG_CLICK) && l3_vars.ui8_walk_assist_feature_enabled)
  {
    ui8_walk_assist_state = 1;
    return true;
  }

  // long up to turn on headlights
  if(events & UP_LONG_CLICK)
  {
    if(l3_vars.ui8_lights == 0)
    {
      l3_vars.ui8_lights = 1;
      lcd_set_backlight_intensity(l3_vars.ui8_lcd_backlight_on_brightness);
    }
    else
    {
      l3_vars.ui8_lights = 0;
      lcd_set_backlight_intensity(l3_vars.ui8_lcd_backlight_off_brightness);
    }

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

void lcd_main_screen(void)
{
  lights_state();
  time();
  assist_level_state();
  wheel_speed();
  walk_assist_state();
//  offroad_mode();
  power();
  temperature();
  pedal_human_power();
  battery_soc();
  battery_display();
  brake();
  trip_time();
  trip_distance();

#if 0
  // ui32_m_draw_graphs_2 == 1 every 3.5 seconds, set on timer interrupt
  if(ui32_m_draw_graphs_2 ||
      m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    graphs_draw(&m_lcd_vars);
  }
#endif
}





//
// Fields - these might be shared my multiple screens
//
Field socField = FIELD_DRAWTEXT(&FONT_5X12);
Field batteryField = FIELD_DRAWTEXT(&MY_FONT_BATTERY);
Field timeField = FIELD_DRAWTEXT(&FONT_5X12);
Field speedField = FIELD_DRAWTEXT(&MY_FONT_NUM_24X40);
Field assistLevelField = FIELD_DRAWTEXT(&MY_FONT_NUM_24X40);
Field maxPowerField = FIELD_DRAWTEXT(&FONT_10X16);
Field humanPowerField = FIELD_DRAWTEXT(&FONT_5X12);
Field whiteFillField = { .variant = FieldFill };
Field meshFillField = { .variant = FieldMesh };
Field warnField = FIELD_DRAWTEXT(&FONT_5X12);

Field tripTimeField = FIELD_DRAWTEXT(&FONT_5X12);
Field tripDistanceField = FIELD_DRAWTEXT(&FONT_5X12);
Field odoField = FIELD_DRAWTEXT(&FONT_5X12);
Field motorTempField = FIELD_DRAWTEXT(&FONT_5X12);

/**
 * Appears at the bottom of all screens, includes status msgs or critical fault alerts
 * FIXME - get rid of this nasty define - instead add the concept of Subscreens, so that the battery bar
 * at the top and the status bar at the bottom can be shared across all screens
 */
#define STATUS_BAR \
{ \
    .x = 4, .y = 114, \
    .width = 0, .height = -1, \
    .field = &warnField \
}

#define BATTERY_BAR \
    { \
        .x = 0, .y = 0, \
        .width = -1, .height = -1, \
        .field = &batteryField \
    }, \
    { \
        .x = 32, .y = 0, \
        .width = -5, .height = -1, \
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
        .border = BorderBottom
    },
    {
        .x = 0, .y = -3,
        .width = 0, .height = -1,
        .field = &speedField,
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
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &humanPowerField,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &tripDistanceField,
        .border = BorderBottom
    },
    {
        .x = 0, .y = -1,
        .width = 0, .height = -1,
        .field = &odoField,
        .border = BorderBottom
    },
    STATUS_BAR,
    {
        .field = NULL
    } }
};



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
    copy_layer_2_layer_3_vars();
  }

  // update_menu_flashing_state();

#if 0
  // enter menu configurations: UP + DOWN click event
  if(buttons_get_up_down_click_event() &&
      m_lcd_vars.lcd_screen_state == LCD_SCREEN_MAIN)
  {
    buttons_clear_all_events();

    // reset needed variables of configurations screen
    p_lcd_configurations_vars->ui8_refresh_full_menu_1 = 1;

    // need to track start configuration
    p_lcd_configurations_vars->ui8_battery_soc_power_used_state = 1;

    m_lcd_vars.lcd_screen_state = LCD_SCREEN_CONFIGURATIONS;
  }

  // enter in menu set power: ONOFF + UP click event
  if(buttons_get_onoff_state() && buttons_get_up_state())
  {
    buttons_clear_all_events();
    m_lcd_vars.ui8_lcd_menu_max_power = 1;
  }

  // ui32_m_draw_graphs_1 == 1 every 3.5 seconds, set on timer interrupt
  // note: this piece of code must run before lcd_main_screen() -> graphs_draw()
  if(ui32_m_draw_graphs_1)
  {
    ui32_m_draw_graphs_2 = 1;
    graphs_clock_1();
  }

  switch(m_lcd_vars.lcd_screen_state)
  {
    case LCD_SCREEN_MAIN:
      lcd_main_screen();
    break;

    case LCD_SCREEN_CONFIGURATIONS:
      lcd_configurations_screen();
    break;
  }

  // ui32_m_draw_graphs_2 == 1 every 3.5 seconds, set on timer interrupt
  // note: this piece of code must run after lcd_main_screen() -> graphs_draw()
  if(ui32_m_draw_graphs_2)
  {
    graphs_clock_2();
  }
#else
  lcd_main_screen();
#endif
  screenUpdate();

#if 0
  // must be reset after a full cycle of lcd_clock()
  ui32_m_draw_graphs_1 = 0;
  ui32_m_draw_graphs_2 = 0;
#endif
}


void assist_level_state(void)
{
#if 0
  static print_number_t assist_level =
  {
    .font = &FONT_45X72,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 1,
    .ui8_left_zero_paddig = 0,
  };

  static uint8_t ui8_assist_level_previous = 0xff;

  if (m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(MAIN_SCREEN_FIELD_LABELS_COLOR);
    UG_FontSelect(&FONT_10X16);
    UG_PutString(12, 50, "ASSIST");
  }



  if ((l3_vars.ui8_assist_level != ui8_assist_level_previous) ||
      m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    ui8_assist_level_previous = l3_vars.ui8_assist_level;

    assist_level.ui32_x_position = 20;
    assist_level.ui32_y_position = 81;
    assist_level.ui32_number = (uint32_t) l3_vars.ui8_assist_level;
    assist_level.ui8_refresh_all_digits = m_lcd_vars.ui32_main_screen_draw_static_info;
    lcd_print_number(&assist_level);
  }
#endif


  fieldPrintf(&assistLevelField, "%d", l3_vars.ui8_assist_level);
}


void trip_time(void)
{
  struct_rtc_time_t *p_time;

  p_time = rtc_get_time_since_startup();

  fieldPrintf(&tripTimeField, "%02d:%02d",  p_time->ui8_hours,  p_time->ui8_minutes);
}

void trip_distance(void)
{
  fieldPrintf(&tripDistanceField, "%d.%d", l3_vars.ui32_trip_x10 / 10, l3_vars.ui32_trip_x10 % 10);
  fieldPrintf(&odoField, "%d.%d", l3_vars.ui32_odometer_x10 / 10, l3_vars.ui32_odometer_x10 % 10);
}






#if 0
void update_menu_flashing_state(void)
{
  static uint8_t ui8_lcd_menu_counter_100ms = 0;
  static uint8_t ui8_lcd_menu_counter_1000ms = 0;

  // ***************************************************************************************************
  // For flashing on menus, 0.5 seconds flash
  if (ui8_lcd_menu_flash_counter++ > 25)
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
  if (ui8_lcd_menu_counter_100ms++ > 5)
  {
    ui8_lcd_menu_counter_100ms = 0;
    ui8_lcd_menu_counter_100ms_state = 1;
  }

  // disable trigger signal
  if (m_lcd_vars.ui8_lcd_menu_counter_1000ms_trigger) { m_lcd_vars.ui8_lcd_menu_counter_1000ms_trigger = 0; }

  if(m_lcd_vars.ui8_lcd_menu_counter_1000ms_state)
  {
    if(ui8_lcd_menu_counter_1000ms++ > 40)
    {
      ui8_lcd_menu_counter_1000ms = 0;
      m_lcd_vars.ui8_lcd_menu_counter_1000ms_state = 0;
      m_lcd_vars.ui8_lcd_menu_counter_1000ms_trigger = 1;
    }
  }
  else
  {
    if(ui8_lcd_menu_counter_1000ms++ > 10)
    {
      ui8_lcd_menu_counter_1000ms = 0;
      m_lcd_vars.ui8_lcd_menu_counter_1000ms_state = 1;
      m_lcd_vars.ui8_lcd_menu_counter_1000ms_trigger = 2;
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
#endif

void brake(void)
{
#if 0
  static uint8_t ui8_braking_previous;
  uint32_t ui32_x1;
  uint32_t ui32_y1;
  uint32_t ui32_x2;
  uint32_t ui32_y2;

  // if previous state was disable, draw
  if((l3_vars.ui8_braking != ui8_braking_previous) ||
      (m_lcd_vars.ui32_main_screen_draw_static_info))
  {
    ui8_braking_previous = l3_vars.ui8_braking;

    if(l3_vars.ui8_braking)
    {
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&SMALL_TEXT_FONT);
      ui32_x1 = 190;
      ui32_y1 = 12;
      UG_PutString(ui32_x1, ui32_y1, "B");
    }
    else
    {
      // clear area
      // 1 leters
      ui32_x1 = 190;
      ui32_y1 = 12;
      ui32_x2 = ui32_x1 + ((1 * 10) + (1 * 1) + 1);
      ui32_y2 = ui32_y1 + 16;
      UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);
    }
  }
#endif
  fieldPrintf(&warnField, l3_vars.ui8_braking ? "BRAKE" : (l3_vars.ui8_walk_assist ? "WALK" : (l3_vars.ui8_lights ? "LIGH" : "")));
}

#if 0
void lcd_set_backlight_intensity(uint8_t ui8_intensity)
{
  // force to be min of 20% and max of 100%
  if(ui8_intensity < 4)
  {
    ui8_intensity = 4;
  }
  else if(ui8_intensity > 20)
  {
    ui8_intensity = 20;
  }

  TIM_SetCompare2(TIM3, ((uint16_t) ui8_intensity) * 2000);
  TIM_CtrlPWMOutputs(TIM3, ENABLE);
}
#endif

void lights_state(void)
{
#if 0
  static uint8_t ui8_lights_previous;
  uint32_t ui32_x1;
  uint32_t ui32_y1;
  uint32_t ui32_x2;
  uint32_t ui32_y2;

  if(buttons_get_up_long_click_event())
  {
    buttons_clear_up_long_click_event();

    if(l3_vars.ui8_lights == 0)
    {
      l3_vars.ui8_lights = 1;
    }
    else
    {
      l3_vars.ui8_lights = 0;
    }
  }

  if(l3_vars.ui8_lights == 0)
  {
    lcd_set_backlight_intensity(l3_vars.ui8_lcd_backlight_off_brightness);
  }
  else
  {
    lcd_set_backlight_intensity(l3_vars.ui8_lcd_backlight_on_brightness);
  }

  // if previous state was disable, draw
  if((l3_vars.ui8_lights != ui8_lights_previous) ||
      (m_lcd_vars.ui32_main_screen_draw_static_info))
  {
    ui8_lights_previous = l3_vars.ui8_lights;

    if(l3_vars.ui8_lights)
    {
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&SMALL_TEXT_FONT);
      ui32_x1 = 205;
      ui32_y1 = 12;
      UG_PutString(ui32_x1, ui32_y1, "L");
    }
    else
    {
      // clear area
      // 1 leters
      ui32_x1 = 205;
      ui32_y1 = 12;
      ui32_x2 = ui32_x1 + ((1 * 10) + (1 * 1) + 1);
      ui32_y2 = ui32_y1 + 16;
      UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);
    }
  }
#endif
}



#if 0 // kevinh possibly reuse in the custom soc field renderer
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
#endif

void battery_soc(void)
{
#if 0
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
    .font = &REGULAR_TEXT_FONT,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 3,
    .ui8_left_zero_paddig = 0,
    .ui8_left_paddig = 1
  };

  if(m_lcd_vars.ui32_main_screen_draw_static_info)
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

  // update battery level value only at every 1 second and this helps to visual filter the fast changing values
  if((ui8_timmer_counter++ >= 50) ||
      (m_lcd_vars.ui32_main_screen_draw_static_info))
  {
    ui8_timmer_counter = 0;

    // to keep same scale as voltage of x10
    ui32_battery_cells_number_x10 = (uint32_t) (l3_vars.ui8_battery_cells_number * 10);

    if(ui16_battery_voltage_filtered_x10.ui16_battery_voltage_soc_x10 > ((uint16_t) ((float) ui32_battery_cells_number_x10 * LI_ION_CELL_VOLTS_90))) { ui32_battery_bar_number = 10; }
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
    if(m_lcd_vars.ui32_main_screen_draw_static_info)
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
        (m_lcd_vars.ui32_main_screen_draw_static_info))
    {
      soc.ui32_x_position = BATTERY_SOC_START_X + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 10) + (BATTERY_SOC_CONTOUR * 2) + 10;
      soc.ui32_y_position = 6;

      // clean full area because it lcd_print_number() with left padding can't deal with that
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&REGULAR_TEXT_FONT);
      UG_PutString(soc.ui32_x_position, soc.ui32_y_position, "    ");

      ui16_battery_soc_watts_hour_previous = ui16_m_battery_soc_watts_hour;
      soc.ui32_x_position = BATTERY_SOC_START_X + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 10) + (BATTERY_SOC_CONTOUR * 2) + 10;
      soc.ui32_y_position = 6;
      soc.ui32_number = ui16_m_battery_soc_watts_hour;
      soc.ui8_refresh_all_digits = 1;
      lcd_print_number(&soc);

      ui32_x1 = soc.ui32_x_final_position + 2;
      ui32_y1 = soc.ui32_y_final_position + 7;
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&SMALL_TEXT_FONT);
      UG_PutString(ui32_x1, ui32_y1, "%");
    }
  }
#endif
  if (l3_vars.ui8_battery_soc_enable)
    fieldPrintf(&socField, "%3d%%", ui16_m_battery_soc_watts_hour);
  else
    fieldPrintf(&socField, "%2u.%1uV", l3_vars.ui16_battery_voltage_soc_x10 / 10, l3_vars.ui16_battery_voltage_soc_x10 % 10);
}

// Show our battery graphic
void battery_display() {
  uint8_t ui32_battery_bar_number = l3_vars.volt_based_soc / (90 / 5); // scale SOC so anything greater than 90% is 5 bars, and zero is zero.

  UG_FontSelect(&MY_FONT_BATTERY);
  fieldPrintf(&batteryField, "%d", ui32_battery_bar_number);
}

void temperature(void)
{
#if 0
  static uint8_t ui8_motor_temperature_previous;
  uint32_t ui32_x1;
  uint32_t ui32_y1;
  uint32_t ui32_x2;
  uint32_t ui32_y2;
  uint8_t ui8_ascii_degree = 176;

  if(l3_vars.ui8_temperature_limit_feature_enabled)
  {
    if((l3_vars.ui8_motor_temperature != ui8_motor_temperature_previous) ||
        (m_lcd_vars.ui32_main_screen_draw_static_info))
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
#endif
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
#if 0
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
      m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    p_rtc_time_previous->ui8_hours = p_rtc_time->ui8_hours;
    p_rtc_time_previous->ui8_minutes = p_rtc_time->ui8_minutes;

    // print hours number
    ui32_x_position = DISPLAY_WIDTH - 1 - hours.font->char_width - (5 * hours.font->char_width) + (5 * 1);
    ui32_y_position = 6;
    hours.ui32_x_position = ui32_x_position;
    hours.ui32_y_position = ui32_y_position;
    hours.ui32_number = p_rtc_time->ui8_hours;
    hours.ui8_refresh_all_digits = m_lcd_vars.ui32_main_screen_draw_static_info;
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
    minutes.ui8_refresh_all_digits = m_lcd_vars.ui32_main_screen_draw_static_info;
    lcd_print_number(&minutes);
  }
#endif
  fieldPrintf(&timeField, "%02d:%02d",  p_rtc_time->ui8_hours,  p_rtc_time->ui8_minutes);

}

void power(void)
{
#if 0
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

  if(m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(MAIN_SCREEN_FIELD_LABELS_COLOR);
    UG_FontSelect(&FONT_10X16);
    UG_PutString(183, 164, "motor power");
  }

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
  fieldPrintf(&maxPowerField, "%dW", l3_vars.ui16_battery_power_filtered);

}

void pedal_human_power(void)
{
#if 0
  static uint16_t ui16_pedal_power_previous = 0;
  uint32_t ui32_x1;
  uint32_t ui32_y1;
  uint32_t ui32_x2;
  uint32_t ui32_y2;
  uint16_t ui16_pedal_power;

  static print_number_t power_number =
  {
    .font = &FONT_24X40,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui32_x_position = 191,
    .ui32_y_position = 268,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 4,
    .ui8_left_zero_paddig = 0,
    .ui32_number = 0,
    .ui8_refresh_all_digits = 1
  };

  if(m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(MAIN_SCREEN_FIELD_LABELS_COLOR);
    UG_FontSelect(&FONT_10X16);
    UG_PutString(178, 244, "human power");
  }

  ui16_pedal_power = l3_vars.ui16_pedal_power_filtered;

  if((ui16_pedal_power != ui16_pedal_power_previous) ||
      m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    ui16_pedal_power_previous = ui16_pedal_power;

    power_number.ui32_number = ui16_pedal_power;
    power_number.ui8_refresh_all_digits = m_lcd_vars.ui32_main_screen_draw_static_info;
    lcd_print_number(&power_number);
    power_number.ui8_refresh_all_digits = 0;
  }
  else
  {

  }
#endif
  fieldPrintf(&humanPowerField, "%4d W", l3_vars.ui16_pedal_power_filtered);
}

void wheel_speed(void)
{
#if 0
  static uint16_t ui16_wheel_x10_speed_previous = 0xffff;

  static print_number_t wheel_speed_integer =
  {
    .font = &FONT_61X99,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 2,
    .ui8_left_zero_paddig = 0,
  };

  static print_number_t wheel_speed_decimal =
  {
    .font = &FONT_45X72,
    .fore_color = C_WHITE,
    .back_color = C_BLACK,
    .ui8_previous_digits_array = {255, 255, 255, 255, 255},
    .ui8_field_number_of_digits = 1,
    .ui8_left_zero_paddig = 0,
  };

  const uint32_t ui32_x_position_integer = 110;
  const uint32_t ui32_x_position_dot = 238;
  const uint32_t ui32_x_position_decimal = 246;
  const uint32_t ui32_y_position_integer = 60;
  const uint32_t ui32_y_position_dot = 134;
  const uint32_t ui32_y_position_decimal = 81;

  if (m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_SetBackcolor(C_BLACK);
    UG_SetForecolor(MAIN_SCREEN_FIELD_LABELS_COLOR);
    UG_FontSelect(&FONT_10X16);
    UG_PutString(257, 50 , "KM/H");

    // print dot
    UG_FillCircle(ui32_x_position_dot, ui32_y_position_dot, 3, C_WHITE);
  }

  if ((l3_vars.ui16_wheel_speed_x10 != ui16_wheel_x10_speed_previous) ||
      m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    ui16_wheel_x10_speed_previous = l3_vars.ui16_wheel_speed_x10;

    wheel_speed_integer.ui32_x_position = ui32_x_position_integer;
    wheel_speed_integer.ui32_y_position = ui32_y_position_integer;
    wheel_speed_integer.ui32_number = (uint32_t) (l3_vars.ui16_wheel_speed_x10 / 10);
    wheel_speed_integer.ui8_refresh_all_digits = m_lcd_vars.ui32_main_screen_draw_static_info;
    lcd_print_number(&wheel_speed_integer);

    wheel_speed_decimal.ui32_x_position = ui32_x_position_decimal;
    wheel_speed_decimal.ui32_y_position = ui32_y_position_decimal;
    wheel_speed_decimal.ui32_number = (uint32_t) (l3_vars.ui16_wheel_speed_x10 % 10);
    wheel_speed_decimal.ui8_refresh_all_digits = m_lcd_vars.ui32_main_screen_draw_static_info;
    lcd_print_number(&wheel_speed_decimal);
  }
#endif
  fieldPrintf(&speedField, "%2d", l3_vars.ui16_wheel_speed_x10 / 10 /*, l3_vars.ui16_wheel_speed_x10 % 10 */);
}


#if 0
void lcd_print_number(print_number_t* number)
{
  uint32_t ui32_number_temp;
  uint8_t ui8_digit_inverse_counter_1;
  uint8_t ui8_digit_inverse_counter_2;
  uint8_t ui8_digits_array[MAX_NUMBER_DIGITS];
  static uint32_t ui32_power_array[MAX_NUMBER_DIGITS] = {1, 10, 100, 1000, 10000};
  uint32_t ui32_number = number->ui32_number;
  uint8_t ui8_i;
  uint32_t ui32_x_position_1 = number->ui32_x_position;
  uint32_t ui32_x_position_2 = ui32_x_position_1;
  uint32_t ui32_x_position_3;
  uint32_t ui32_y_position = number->ui32_y_position;
  uint8_t ui8_decimal_digits = number->ui8_decimal_digits;
  uint8_t ui8_decimal_digits_inverse_1;
  uint8_t ui8_decimal_digits_inverse_2;
  uint8_t ui8_decimal_digits_printed_1;
  uint8_t ui8_decimal_digits_printed_2;
  uint8_t ui8_digit_number_start = 0;
  uint8_t ui8_left_padding_digit = 0;
  uint8_t ui8_left_padding_first_digit = 0;
  uint8_t ui8_left_padding_digit_total = 0;

  // can't process over MAX_NUMBER_DIGITS
  if(number->ui8_field_number_of_digits > MAX_NUMBER_DIGITS)
  {
    return;
  }

  // set the font that will be used
  UG_FontSelect(number->font);

  ui8_decimal_digits_printed_1 = number->ui8_decimal_digits ? 0: 1;
  ui8_decimal_digits_printed_2 = ui8_decimal_digits_printed_1;

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

  ui8_digit_inverse_counter_1 = number->ui8_field_number_of_digits - 1;
  ui8_digit_inverse_counter_2 = ui8_digit_inverse_counter_1;

  // print first the "."
  // invert the decimal digits
  if(ui8_decimal_digits)
  {
    ui8_decimal_digits_inverse_1 = ui8_digit_inverse_counter_2 - ui8_decimal_digits;
    ui8_decimal_digits_inverse_2 = ui8_decimal_digits_inverse_1;

    // print first the "."
    // loop over all digits
    for(ui8_i = 0; ui8_i < number->ui8_field_number_of_digits; ui8_i++)
    {
      // increase X position for next char
      ui32_x_position_2 += number->font->char_width + 1;

      ui8_digit_inverse_counter_2--;

      // print only 1 time the "."
      if(ui8_decimal_digits_printed_2 == 0 &&
          ui8_decimal_digits_inverse_2 == 0)
      {
        ui8_decimal_digits_printed_2 = 1;

        // print a "."
        UG_PutChar(46, ui32_x_position_2 - (number->font->char_width / 4), ui32_y_position, number->fore_color, number->back_color);

        break;
      }

      // decrement only if positive
      if(ui8_decimal_digits_inverse_2)
      {
        ui8_decimal_digits_inverse_2--;
      }
    }
  }

  // loop over all digits
  for(ui8_i = 0; ui8_i < number->ui8_field_number_of_digits; ui8_i++)
  {
    // only digits that changed
    if(((ui8_digits_array[ui8_digit_inverse_counter_1] != number->ui8_previous_digits_array[ui8_digit_inverse_counter_1]) ||
        (number->ui8_refresh_all_digits)) &&
        (!number->ui8_clean_area_all_digits))
    {
      if((ui8_digits_array[ui8_digit_inverse_counter_1] == 0) && // if is a 0
          (number->ui8_left_paddig) && // left padding
          (ui8_digit_inverse_counter_1 > 0) && // digits that not be printed
          (ui8_left_padding_first_digit == 0)) // if first digit was printed, do not skip next zeros
      {
        // print nothing
        ui8_left_padding_digit = 1;
      }
      else if((ui8_digits_array[ui8_digit_inverse_counter_1] == 0) && // if is a 0
          (ui8_decimal_digits_printed_1 == 0) && // decimal digit not printed yet
          (ui8_decimal_digits_inverse_1 == 0))
      {
        // print a "0"
        UG_PutChar(48, ui32_x_position_1, ui32_y_position, number->fore_color, number->back_color);
      }
      else if((ui8_digits_array[ui8_digit_inverse_counter_1] == 0) && // if is a 0
          (ui8_digit_inverse_counter_1 > ui8_digit_number_start) && // if is a digit at left from the first digit
          (number->ui8_left_zero_paddig)) // if we want to print a 0 at left
      {
        // print a "0"
        UG_PutChar(48, ui32_x_position_1, ui32_y_position, number->fore_color, number->back_color);
      }
      else if((ui8_digits_array[ui8_digit_inverse_counter_1] == 0) &&  // if is a 0
          (ui8_digit_inverse_counter_1 > ui8_digit_number_start) && // if is a digit at left from the first digit
          (!number->ui8_left_zero_paddig)) // if we NOT want to print a 0 at left
      {
        // print a " "
        UG_PutChar(32, ui32_x_position_1, ui32_y_position, number->fore_color, number->back_color);
      }
      else
      {
        // print the digit
        UG_PutChar((ui8_digits_array[ui8_digit_inverse_counter_1] + 48), ui32_x_position_1, ui32_y_position, number->fore_color, number->back_color);

        if(number->ui8_left_paddig)
        {
          ui8_left_padding_first_digit = 1;
        }
      }
    }
    else if((ui8_digits_array[ui8_digit_inverse_counter_1] == 0) && // if is a 0
        (number->ui8_left_paddig) && // left padding
        (ui8_digit_inverse_counter_1 > 0) && // digits that not be printed
        (ui8_left_padding_first_digit == 0)) // if first digit was printed, do not skip next zeros
    {
      // print nothing
    }
    else if((ui8_digits_array[ui8_digit_inverse_counter_1] == 0) && // if is a 0
        (ui8_decimal_digits_printed_1 == 0) && // decimal digit not printed yet
        (ui8_decimal_digits_inverse_1 == 0))
    {
      // print a "0"
      UG_PutChar(48, ui32_x_position_1, ui32_y_position, number->fore_color, number->back_color);
    }
    // the case where there was a 0 but we want to remove it
    else if(((ui8_digits_array[ui8_digit_inverse_counter_1] == 0) &&  // if is a 0
        (ui8_digit_inverse_counter_1 > ui8_digit_number_start) && // if is a digit at left from the first digit
        (!number->ui8_left_zero_paddig)) || // if we NOT want to print a 0 at left
        (number->ui8_clean_area_all_digits)) // we want to clean, so print a " "
    {
      // print a " "
      UG_PutChar(32, ui32_x_position_1, ui32_y_position, number->fore_color, number->back_color);
    }
    // the case where there was a " " but we need to write a 0
    else if((ui8_digits_array[ui8_digit_inverse_counter_1] == 0) &&  // if is a 0
        (ui8_digit_number_start > number->ui8_digit_number_start_previous)) // if is a digit at left from the first digit
    {
      // print a "0"
      UG_PutChar(48, ui32_x_position_1, ui32_y_position, number->fore_color, number->back_color);
    }
    // the case where there was a " " (decimal number) but we need to write a 0
    else if((ui8_digits_array[ui8_digit_inverse_counter_1] == 0) &&  // if is a 0
        (ui8_decimal_digits_printed_1 == 0) && // decimal digits were not printed yet
        (ui8_decimal_digits_inverse_1 == 1)) // first unit digit
    {
      // print a "0"
      UG_PutChar(48, ui32_x_position_1, ui32_y_position, number->fore_color, number->back_color);
    }
    else
    {
      // do not change the field, keep with previous value
    }

    if(ui8_left_padding_digit == 0)
    {
      // increase X position for next char
      ui32_x_position_1 += number->font->char_width + 1;
    }
    // do not increment for next char position
    else
    {
      ui8_left_padding_digit = 0;
      ui8_left_padding_digit_total++;
    }

    ui8_digit_inverse_counter_1--;

    // print only 1 time the "."
    if(ui8_decimal_digits_printed_1 == 0 &&
        ui8_decimal_digits_inverse_1 == 0)
    {
      ui8_decimal_digits_printed_1 = 1;

      // increase X position for next char
      ui32_x_position_1 += ((number->font->char_width / 2) + 1);
    }

    // decrement only if positive
    if(ui8_decimal_digits_inverse_1)
    {
      ui8_decimal_digits_inverse_1--;
    }
  }

  // clean empty
  ui32_x_position_3 = ui32_x_position_1;
  while(ui8_left_padding_digit_total > 0)
  {
    ui8_left_padding_digit_total--;

    // print a " "
    UG_PutChar(32, ui32_x_position_3, ui32_y_position, number->fore_color, number->back_color);

    // increase X position for next char
    ui32_x_position_3 += number->font->char_width + 1;
  }

  // save the digits
  for(ui8_i = 0; ui8_i < number->ui8_field_number_of_digits; ui8_i++)
  {
    number->ui8_previous_digits_array[ui8_i] = ui8_digits_array[ui8_i];
  }

  number->ui8_digit_number_start_previous = ui8_digit_number_start;

  // store final position
  number->ui32_x_final_position = ui32_x_position_1;
  number->ui32_y_final_position = ui32_y_position;
}
#endif



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

void walk_assist_state(void)
{
  // kevinh - note on the sw102 we show WALK in the box normally used for BRAKE display - the display code is handled there now
#if 0
  static uint8_t ui8_walk_assist_previous;
  uint32_t ui32_x1;
  uint32_t ui32_y1;
  uint32_t ui32_x2;
  uint32_t ui32_y2;



  // if previous state was disable, draw
  if((l3_vars.ui8_walk_assist != ui8_walk_assist_previous) ||
      (m_lcd_vars.ui32_main_screen_draw_static_info))
  {
    ui8_walk_assist_previous = l3_vars.ui8_walk_assist;

    if(l3_vars.ui8_walk_assist)
    {
      UG_SetBackcolor(C_BLACK);
      UG_SetForecolor(C_WHITE);
      UG_FontSelect(&SMALL_TEXT_FONT);
      ui32_x1 = 190;
      ui32_y1 = 10;
      UG_PutString(ui32_x1, ui32_y1, "W");
    }
    else
    {
      // clear area
      // 1 leters
      ui32_x1 = 190;
      ui32_y1 = 10;
      ui32_x2 = ui32_x1 + ((1 * 10) + (1 * 1) + 1);
      ui32_y2 = ui32_y1 + 16;
      UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);
    }
  }
#endif
  if(/* m_lcd_vars.lcd_screen_state == LCD_SCREEN_MAIN && */
      l3_vars.ui8_walk_assist_feature_enabled)
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

