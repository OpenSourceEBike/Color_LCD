/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stdio.h"

#include "pins.h"
#include "UTFT-STM8S/UTFT.h"

void lcd_backlight (uint32_t ui32_state);

void lcd_init (void)
{
  // next step is needed to have PB3 and PB4 working as GPIO
  /* Disable the Serial Wire Jtag Debug Port SWJ-DP */
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = LCD_BACKLIGHT__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_BACKLIGHT__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_READ__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_READ__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_RESET__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_RESET__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_COMMAND_DATA__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_COMMAND_DATA__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_CHIP_SELECT__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_CHIP_SELECT__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_WRITE__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_WRITE__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = 0xffff;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // enable backlight
  lcd_backlight (ENABLE);

  // keep RESET and READ pins always at 1
  GPIO_SetBits(LCD_RESET__PORT, LCD_RESET__PIN);
  GPIO_SetBits(LCD_READ__PORT, LCD_READ__PIN);

  UTFT ();
  UTFT_InitLCD ();
}

void lcd_backlight (uint32_t ui32_state)
{
  if (ui32_state)
  {
    GPIO_SetBits(LCD_BACKLIGHT__PORT, LCD_BACKLIGHT__PIN);
  }
  else
  {
    GPIO_ResetBits(LCD_BACKLIGHT__PORT, LCD_BACKLIGHT__PIN);
  }
}
