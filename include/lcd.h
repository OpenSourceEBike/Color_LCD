/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */

#pragma once

#include "stdint.h"

void lcd_init(void);
void lcd_refresh(void); // Call to flush framebuffer to SPI device
void lcd_set_backlight_intensity(uint8_t level);

