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

// A special color which means "do not draw", used to let fonts have transparent backgrounds (also to save the cost of rendering when we know the area is already blank)
#define C_TRANSPARENT 0xC1C2

