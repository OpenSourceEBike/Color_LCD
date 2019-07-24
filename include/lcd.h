/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */

#ifndef LCD_H_
#define LCD_H_

void lcd_init(void);
void lcd_refresh(void); // Call to flush framebuffer to SPI device

#endif /* LCD_H_ */
