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
  extern uint8_t SmallFont[];

  RCC_APB1PeriphResetCmd(RCC_APB1Periph_WWDG, DISABLE);

  pins_init ();
  delay_init ();
  system_power (1);
  lcd_init ();

//  UTFT_setFont(SmallFont);

  while (1)
  {
//    UTFT_setColor_rgb(255, 0, 0);
//    UTFT_drawCircle (0, 0, 5);
//    UTFT_drawCircle (10, 10, 10);
//    UTFT_setColor_rgb(255, 255, 0);
//    UTFT_drawCircle (50, 50, 15);
//    delay(1000);

//    UTFT_clrScr();
    UTFT_fillScr(VGA_YELLOW);
//    delay(1000);
//    UTFT_setColor (VGA_RED);
//    UTFT_drawCircle (50, 50, 15);
//    UTFT_fillCircle (50, 50, 15);
//    UTFT_setColor_rgb(255, 0, 0);
//    UTFT_fillRect(0, 0, 100, 50);
//    UTFT_setColor_rgb(255, 255, 0);
//    UTFT_drawPixel(0,0);
//    UTFT_drawPixel(0,10);
//    UTFT_drawPixel(10,0);
//    UTFT_drawPixel(10,10);
////    UTFT_print("Hello world!",10 ,60 );
//    UTFT_setColor(VGA_FUCHSIA);
//    UTFT_drawLine(5,80,154,80);
//    UTFT_drawLine(5,81,154,81);
//    delay(1000);
  }
}

