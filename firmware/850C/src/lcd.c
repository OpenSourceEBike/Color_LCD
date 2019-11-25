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

lcd_IC_t g_lcd_ic_type;

void power_off_management(void);
void lcd_power_off(uint8_t updateDistanceOdo);
void power_off_management(void);

/* Place your initialization/startup code here (e.g. MyInst_Start()) */
void lcd_init(void)
{
  g_lcd_ic_type = bafang_500C_lcd_init();
  UG_FillScreen(C_BLACK);

  set_lcd_backlight(); // default to at least some backlight
}


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
  ui8_intensity /= 5;

  // force to be min of 5% and max of 100%
  if(ui8_intensity < 1)
  {
    ui8_intensity = 1;
  }
  else if(ui8_intensity > 20)
  {
    ui8_intensity = 20;
  }

  TIM_SetCompare2(TIM3, ((uint16_t) ui8_intensity) * 2000);
}




void lcd_power_off(uint8_t updateDistanceOdo)
{
//  if (updateDistanceOdo)
//  {
    ui_vars.ui32_wh_x10_offset = ui_vars.ui32_wh_x10;
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

