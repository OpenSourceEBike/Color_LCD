/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include "main.h"
#include "ugui.h"

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

typedef struct _print_number
{
  const UG_FONT* font;
  UG_COLOR fore_color;
  UG_COLOR back_color;
  uint32_t ui32_x_position;
  uint32_t ui32_y_position;
  uint32_t ui32_x_final_position;
  uint32_t ui32_y_final_position;
  uint8_t ui8_previous_digits_array[MAX_NUMBER_DIGITS];
  uint8_t ui8_field_number_of_digits;
  uint8_t ui8_left_zero_paddig;
  uint8_t ui8_left_paddig;
  uint8_t ui8_refresh_all_digits;
  uint32_t ui32_number;
  uint8_t ui8_digit_number_start_previous;
  uint8_t ui8_clean_area_all_digits;
  uint8_t ui8_decimal_digits;
} print_number_t;

extern volatile uint32_t ui32_g_layer_2_can_execute;
extern volatile uint32_t ui32_g_graphs_data_array_over_255;
extern volatile uint32_t ui32_g_first_time;

void lcd_init(void);
void lcd_clock(void);
volatile lcd_vars_t* get_lcd_vars(void);
void lcd_print_number(print_number_t* number);
void lcd_draw_main_menu_mask(void);
void graphs_measurements_update(void);
void lcd_set_backlight_intensity(uint8_t ui8_intensity);

#endif /* LCD_H_ */
