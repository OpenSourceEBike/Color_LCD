/*
 * Bafang LCD 860C/850C firmware
 *
 * Copyright (C) Casainho, 2018, 2019, 2020
 *
 * Released under the GPL License, Version 3
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

#define MAX_NUMBER_DIGITS 5 // max of 5 digits: 1234.5 or 12345

typedef enum
{
  LCD_SCREEN_MAIN = 1,
  LCD_SCREEN_CONFIGURATIONS = 2
} lcd_screen_states_t;

typedef enum
{
  MAIN_SCREEN_STATE_MAIN = 0,
  MAIN_SCREEN_STATE_POWER,
  MAIN_SCREEN_STATE_CHANGE_GRAPH
} lcd_main_screen_states_t;

typedef struct lcd_vars_struct
{
  uint32_t ui32_main_screen_draw_static_info;
  lcd_screen_states_t lcd_screen_state;
  uint8_t ui8_lcd_menu_counter_1000ms_state;
  uint8_t ui8_lcd_menu_counter_1000ms_trigger;
  lcd_main_screen_states_t main_screen_state;
} lcd_vars_t;

void lcd_init(void);
void lcd_clock(void);
volatile lcd_vars_t* get_lcd_vars(void);
void lcd_draw_main_menu_mask(void);
void graphs_measurements_update(void);
void lcd_set_backlight_intensity(uint8_t ui8_intensity);

#endif /* LCD_H_ */
