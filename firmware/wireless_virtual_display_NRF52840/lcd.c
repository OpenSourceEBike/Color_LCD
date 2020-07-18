/*
 * Bafang LCD 860C/850C firmware
 *
 * Copyright (C) Casainho, 2018, 2019, 2020
 *
 * Released under the GPL License, Version 3
 */

#include <math.h>
#include "stdio.h"
#include "main.h"
#include "pins.h"
#include "lcd.h"
#include "eeprom.h"
#include "rtc.h"
#include "state.h"

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

}

void power_off_management(void)
{

}

void lcd_set_backlight_intensity(uint8_t ui8_intensity)
{

}

void lcd_power_off(uint8_t updateDistanceOdo)
{

}

volatile lcd_vars_t* get_lcd_vars(void)
{
  return &m_lcd_vars;
}

