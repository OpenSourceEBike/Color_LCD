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
#define LCD_RESET__PIN                              GPIO_Pin_6

#define LCD_COMMAND_DATA__PORT                      GPIOC
#define LCD_COMMAND_DATA__PIN                       GPIO_Pin_3

#define LCD_CHIP_SELECT__PORT                       GPIOC
#define LCD_CHIP_SELECT__PIN                        GPIO_Pin_4

#define LCD_WRITE__PORT                             GPIOC
#define LCD_WRITE__PIN                              GPIO_Pin_5

#define LCD_READ__PORT                              GPIOC
#define LCD_READ__PIN                               GPIO_Pin_7

#define LCD_BUS__PORT                               GPIOB

#define BUTTON_UP__PORT                             GPIOC
#define BUTTON_UP__PIN                              GPIO_Pin_11
//#define BUTTON_ONOFF__PORT                          GPIOC
//#define BUTTON_ONOFF__PIN                           GPIO_Pin_12
#define BUTTON_ONOFF__PORT                          GPIOA
#define BUTTON_ONOFF__PIN                           GPIO_Pin_11

#define BUTTON_DOWN__PORT                           GPIOA
#define BUTTON_DOWN__PIN                            GPIO_Pin_15

#define USART1__PORT                                GPIOA
#define USART1_TX__PIN                              GPIO_Pin_9
#define USART1_RX__PIN                              GPIO_Pin_10

#define EEPROM_SPI__PORT                            GPIOC
#define EEPROM_SPI_SDA_PIN                          GPIO_Pin_8
#define EEPROM_SPI_SCL_PIN                          GPIO_Pin_9

#define USB_CHARGE__PORT                            GPIOA
#define USB_CHARGE__PIN                             GPIO_Pin_3

void pins_init(void);
void system_power(uint32_t ui32_state);

#endif /* PINS_H_ */
