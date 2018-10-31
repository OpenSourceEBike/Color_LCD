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

#include "pins.h"
#include "lcd.h"
#include "delay.h"
#include "UTFT-STM8S/UTFT.h"

#include "ugui/ugui.h"
#include "ugui_driver/ugui_bafang_500c.h"

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

  UG_COLOR color[3];

  color[0] = C_RED;
  color[1] = C_GREEN;
  color[2] = C_BLUE;

//  lcd_init ();

  /* Place your initialization/startup code here (e.g. MyInst_Start()) */
  Display_Init();

  while (1) {
  /* Clear screen. */
//  UTFT_clrScr();
  UG_DrawFrame(100, 100, 10, 10, C_PINK);     // Draw a Frame
//  UG_FillScreen(VGA_YELLOW);

//      UG_FontSelect(&FONT_8X14);
//      UG_PutString(10, 10, "Hello World!");


//        UG_DrawLine(10, 10, 10, 100, C_PINK);     // Draw a Frame


//  delay_ms(2000);
//
//  UTFT_clrScr();
//  UTFT_fillScr(VGA_YELLOW);
//
//  delay_ms(2000);

  }

//  lcd_init();
//
//  UTFT_clrScr();
//  UTFT_fillScr(VGA_AQUA);
//
//  UTFT_setColor(VGA_RED);
//  UTFT_setBackColor(VGA_AQUA);
//  UTFT_setFont(BigFont);
//  UTFT_print("TSDZ2", 10, 140);
//  UTFT_print("Flexible OpenSource firmware", 10, 180);
//  UTFT_print("firmware", 10, 200);

  while (1)
  {

  }
}
