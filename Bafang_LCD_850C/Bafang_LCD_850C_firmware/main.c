/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stdio.h"

#include "UTFT-STM8S/UTFT.h"

#include "pins.h"
#include "lcd.h"
#include "delay.h"

void system_power (uint32_t ui32_state)
{
  if (ui32_state)
  {
    GPIO_SetBits(SYSTEM_POWER_ON_OFF__PORT, SYSTEM_POWER_ON_OFF__PIN);
  }
  else
  {
    GPIO_ResetBits(SYSTEM_POWER_ON_OFF__PORT, SYSTEM_POWER_ON_OFF__PIN);
  }
}

int main(void)
{
  extern uint8_t BigFont[];

  RCC_APB1PeriphResetCmd(RCC_APB1Periph_WWDG, DISABLE);

  pins_init();
  delay_init();
  system_power(ENABLE);
  lcd_init();

  UTFT_clrScr();
  UTFT_fillScr(VGA_AQUA);

  UTFT_setColor(VGA_RED);
  UTFT_setBackColor(VGA_AQUA);
  UTFT_setFont(BigFont);
  UTFT_print("TSDZ2", 10, 140);
  UTFT_print("Flexible OpenSource firmware", 10, 180);
  UTFT_print("firmware", 10, 200);

  while (1)
  {

  }
}

