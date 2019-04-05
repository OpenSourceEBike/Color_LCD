/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef LCD_CONFIGURATIONS_H_
#define LCD_CONFIGURATIONS_H_

#include <stdint.h>
#include "lcd.h"

typedef struct _lcd_configurations
{
  uint8_t ui8_item_number;
  uint8_t ui8_previous_item_number;
  uint8_t ui8_item_visible_start_index;
  uint8_t ui8_item_visible_index;
  uint8_t ui8_refresh_full_menu_1;
  uint8_t ui8_refresh_full_menu_2;
  uint8_t ui8_battery_soc_power_used_state;
} struct_lcd_configurations_vars;

void lcd_configurations_screen_init(void);
void lcd_configurations_screen(void);
struct_lcd_configurations_vars* get_lcd_configurations_vars(void);

#endif /* LCD_CONFIGURATIONS_H_ */
