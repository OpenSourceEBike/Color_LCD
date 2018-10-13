/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef PINS_H_
#define PINS_H_

#include "stm32f10x_gpio.h"

#define SYSTEM_POWER_ON_OFF__PORT                   GPIOC
#define SYSTEM_POWER_ON_OFF__PIN                    GPIO_Pin_1

#define LCD_BACKLIGHT__PORT                         GPIOA
#define LCD_BACKLIGHT__PIN                          GPIO_Pin_7

#define LCD_PIN_1__PORT                             GPIOC
#define LCD_PIN_1__PIN                              GPIO_Pin_6

#define LCD_PIN_2__PORT                             GPIOC
#define LCD_PIN_2__PIN                              GPIO_Pin_7

void pins_init (void);

#endif /* PINS_H_ */
