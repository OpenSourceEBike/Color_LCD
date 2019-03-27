/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) Casainho, 2019.
 *
 * Released under the GPL License, Version 3
 */

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "pins.h"

typedef enum
{
  DATA = 0,
  COMMAND = 1
} command_data_t;

void send_byte(uint8_t byte, uint8_t command_data);

void lcd_init(void)
{
  uint8_t init_array[] =
  {
    0xAE,
    0xA8,
    0x3F,
    0xD5,
    0x50,
    0xC0,
    0xD3,
    0x60,
    0xDC,
    0x00,
    0x20,
    0x81,
    0xBF,
    0xA0,
    0xA4,
    0xA6,
    0xAD,
    0x8A,
    0xD9,
    0x1F,
    0xDB,
    0x30,
    0xB0,
    0x00,
    0x10,
    0xB1,
    0x00,
    0x10,
    0xB2,
    0x00,
    0x10,
    0xB3,
    0x00,
    0x10,
    0xB4,
    0x00,
    0x10,
    0xB5,
    0x00,
    0x10,
    0xB6,
    0x00,
    0x10,
    0xB7,
    0x00,
    0x10,
    0xB8,
    0x00,
    0x10,
    0xB9,
    0x00,
    0x10,
    0xBA,
    0x00,
    0x10,
    0xBB,
    0x00,
    0x10,
    0xBC,
    0x00,
    0x10,
    0xBD,
    0x00,
    0x10,
    0xBE,
    0x00,
    0x10,
    0xBF,
    0x00,
    0x10
  };

  // init pins
  nrf_gpio_cfg_output(LCD_CHIP_SELECT__PIN);
  nrf_gpio_cfg_output(LCD_COMMAND_DATA__PIN);
  nrf_gpio_cfg_output(LCD_CLOCK);
  nrf_gpio_cfg_output(LCD_DATA);

  // send initialization sequence of commands
  for(uint8_t i = 0; i < sizeof(init_array); i++)
  {
    send_byte(COMMAND, init_array[i]);
  }
}

void send_byte(uint8_t byte, command_data_t command_data)
{
  nrf_delay_us(1);
  nrf_gpio_pin_clear(LCD_CHIP_SELECT__PIN);
  nrf_delay_us(1);

  nrf_gpio_pin_clear(LCD_COMMAND_DATA__PIN);

//  if(command_data == COMMAND)
//  {
//    nrf_gpio_pin_clear(LCD_COMMAND_DATA__PIN);
//  }
//  else
//  {
//    nrf_gpio_pin_set(LCD_COMMAND_DATA__PIN);
//  }
  nrf_delay_us(1);

  for(uint8_t i = 0; i < 8; i++)
  {
    if(byte & 128)
    {
      nrf_gpio_pin_set(LCD_DATA);
    }
    else
    {
      nrf_gpio_pin_clear(LCD_DATA);
    }
    byte <<= 1;

    nrf_gpio_pin_set(LCD_CLOCK);
    nrf_delay_us(1);
    nrf_gpio_pin_clear(LCD_CLOCK);
    nrf_delay_us(1);
  }

  nrf_gpio_pin_set(LCD_COMMAND_DATA__PIN);
  nrf_delay_us(1);
  nrf_gpio_pin_set(LCD_CHIP_SELECT__PIN);
  nrf_delay_us(1);
}
