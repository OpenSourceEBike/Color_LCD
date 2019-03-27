/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) Casainho, 2019.
 *
 * Released under the GPL License, Version 3
 */

#ifndef PINS_H_
#define PINS_H_

#include "nordic_common.h"
#include "nrf.h"

#define SYSTEM_POWER_ON_OFF__PIN              9

#define LCD_COMMAND_DATA__PIN                 30
#define LCD_CHIP_SELECT__PIN                  4
#define LCD_CLOCK                             6
#define LCD_DATA                              7

#define BUTTON_UP__PIN
#define BUTTON_ONOFF__PIN
#define BUTTON_DOWN__PIN

#define USART1_TX__PIN
#define USART1_RX__PIN

void pins_init(void);
void system_power(uint32_t ui32_state);

#endif /* PINS_H_ */
