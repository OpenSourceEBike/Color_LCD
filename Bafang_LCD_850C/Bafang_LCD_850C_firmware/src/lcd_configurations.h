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
  uint32_t ui32_configurations_screen_draw_static_info;
} struct_lcd_configurations_vars;

void lcd_configurations_screen_init(void);
void lcd_configurations_screen(void);
struct_lcd_configurations_vars* get_lcd_configurations_vars(void);

#endif /* LCD_CONFIGURATIONS_H_ */
