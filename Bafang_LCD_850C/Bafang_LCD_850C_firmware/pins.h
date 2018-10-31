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

#define LCD_RESET__PORT                             GPIOC
#define LCD_RESET__PIN                              GPIO_Pin_7

#define LCD_COMMAND_DATA__PORT                      GPIOC
#define LCD_COMMAND_DATA__PIN                       GPIO_Pin_3

#define LCD_CHIP_SELECT__PORT                       GPIOC
#define LCD_CHIP_SELECT__PIN                        GPIO_Pin_4

#define LCD_WRITE__PORT                             GPIOC
#define LCD_WRITE__PIN                              GPIO_Pin_5

#define LCD_READ__PORT                              GPIOC
#define LCD_READ__PIN                               GPIO_Pin_6

void pins_init (void);

#endif /* PINS_H_ */
