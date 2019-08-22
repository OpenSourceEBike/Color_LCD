/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include <math.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stdio.h"
#include "main.h"
#include "config.h"
#include "utils.h"
#include "pins.h"
#include "lcd.h"
#include "buttons.h"
#include "eeprom.h"
#include "usart1.h"
#include "ugui_driver/ugui_bafang_850c.h"
#include "ugui.h"
#include "rtc.h"
#include "graphs.h"
#include "fonts.h"
#include "state.h"

// Battery SOC symbol:
// 10 bars, each bar: with = 7, height = 24
// symbol has contour lines of 1 pixel
#define BATTERY_SOC_START_X 8
#define BATTERY_SOC_START_Y 4
#define BATTERY_SOC_BAR_WITH 7
#define BATTERY_SOC_BAR_HEIGHT 24
#define BATTERY_SOC_CONTOUR 1

volatile lcd_vars_t m_lcd_vars =
{
  .ui32_main_screen_draw_static_info = 1,
  .lcd_screen_state = LCD_SCREEN_MAIN,
  .ui8_lcd_menu_counter_1000ms_state = 0,
  .main_screen_state = MAIN_SCREEN_STATE_MAIN,
};


static uint8_t ui8_m_usart1_received_first_package = 0;

static volatile graphs_t *m_p_graphs;
static volatile uint32_t ui32_m_draw_graphs_1 = 0;
static volatile uint32_t ui32_m_draw_graphs_2 = 0;

volatile uint32_t ui32_g_first_time = 1;

void power_off_management(void);
void lcd_power_off(uint8_t updateDistanceOdo);
void power_off_management(void);

void change_graph(void);

/* Place your initialization/startup code here (e.g. MyInst_Start()) */
void lcd_init(void)
{
  bafang_500C_lcd_init();
  UG_FillScreen(C_BLACK);

  m_p_graphs = get_graphs();
  set_lcd_backlight(); // default to at least some backlight
}

#if 0
void lcd_clock(void)
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

  if(first_time_management())
  {
    return;
  }

  update_menu_flashing_state();

  // merged in layer_2 calc_battery_soc_watts_hour();

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
  if(m_lcd_vars.lcd_screen_state == LCD_SCREEN_MAIN &&
      buttons_get_onoff_click_event() &&
      buttons_get_up_click_event())
  {
    buttons_clear_all_events();
    m_lcd_vars.main_screen_state = MAIN_SCREEN_STATE_POWER;
  }

  // ui32_m_draw_graphs_1 == 1 every 3.5 seconds, set on timer interrupt
  // note: this piece of code must run before lcd_main_screen() -> graphs_draw()
  if(ui32_m_draw_graphs_1 &&
      ui32_g_first_time == 0) // start update graphs only after a startup delay to avoid wrong values of the variables
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
  if(ui32_m_draw_graphs_1 &&
      ui32_m_draw_graphs_2 &&
      ui32_g_first_time == 0) // start update graphs only after a startup delay to avoid wrong values of the variables
  {
    graphs_clock_2();
  }

  power_off_management();

  // must be reset after a full cycle of lcd_clock()
  ui32_m_draw_graphs_1 = 0;
  ui32_m_draw_graphs_2 = 0;
}

void lcd_draw_main_menu_mask(void)
{
  UG_DrawLine(0, 39, 319, 39, MAIN_SCREEN_FIELD_LABELS_COLOR);
  UG_DrawLine(0, 159, 319, 159, MAIN_SCREEN_FIELD_LABELS_COLOR);
  UG_DrawLine(0, 239, 319, 239, MAIN_SCREEN_FIELD_LABELS_COLOR);
  UG_DrawLine(0, 319, 319, 319, MAIN_SCREEN_FIELD_LABELS_COLOR);

  // vertical line
  UG_DrawLine(159, 159, 159, 319, MAIN_SCREEN_FIELD_LABELS_COLOR);
}
#endif



#if 0
void lcd_main_screen(void)
{
  // run once only, to draw static info
  if(m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    UG_FillScreen(C_BLACK);
    lcd_draw_main_menu_mask();
  }

  lights_state();
  time();
  assist_level_state();
  wheel_speed();
  walk_assist_state();
//  offroad_mode();
  power();
  pedal_human_power();
  battery_soc();
  brake();
  trip_time();
  trip_distance();

  change_graph();

  // ui32_m_draw_graphs_2 == 1 every 3.5 seconds, set on timer interrupt
  if(ui32_m_draw_graphs_2 ||
      m_lcd_vars.ui32_main_screen_draw_static_info)
  {
    graphs_draw(&m_lcd_vars);
  }

  // this event is not used so we must clear it
  buttons_get_onoff_click_long_click_event();

  m_lcd_vars.ui32_main_screen_draw_static_info = 0;
}
#endif



void power_off_management(void)
{
  if(buttons_get_onoff_long_click_event() &&
    m_lcd_vars.lcd_screen_state == LCD_SCREEN_MAIN &&
    buttons_get_up_state() == 0 &&
    buttons_get_down_state() == 0)
  {
    lcd_power_off(1);
  }
}



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




void lcd_power_off(uint8_t updateDistanceOdo)
{
//  if (updateDistanceOdo)
//  {
    l3_vars.ui32_wh_x10_offset = l3_vars.ui32_wh_x10;
//    l3_vars.ui32_odometer_x10 += ((uint32_t) l3_vars.ui16_odometer_distance_x10);
//  }

  // save the variables on EEPROM
  eeprom_write_variables ();

  // put screen all black and disable backlight
  UG_FillScreen(0);
  lcd_set_backlight_intensity(0);

  // now disable the power to all the system
  system_power(0);

  // block here
  while(1) ;
}





volatile lcd_vars_t* get_lcd_vars(void)
{
  return &m_lcd_vars;
}

