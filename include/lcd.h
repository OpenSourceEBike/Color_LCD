/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */

#ifndef LCD_H_
#define LCD_H_
#include "main.h"
#include "ugui.h"

void lcd_init(void);
void lcd_refresh(void);
void pset(UG_S16 x, UG_S16 y, UG_COLOR col);

#endif /* LCD_H_ */
