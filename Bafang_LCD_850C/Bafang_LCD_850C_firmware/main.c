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
#include "buttons.h"

#include "ugui/ugui.h"
#include "ugui_driver/ugui_bafang_500c.h"

void SetSysClockTo128Mhz(void);

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

#define INT_DIGITS 19   /* enough for 64 bit integer */

char *itoa(i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1; /* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {      /* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

int main(void)
{
  extern uint8_t BigFont[];

  SetSysClockTo128Mhz();

  RCC_APB1PeriphResetCmd(RCC_APB1Periph_WWDG, DISABLE);

  pins_init();
  delay_init();
  system_power(ENABLE);

  UG_COLOR color[3];

  color[0] = C_RED;
  color[1] = C_GREEN;
  color[2] = C_BLUE;

  /* Place your initialization/startup code here (e.g. MyInst_Start()) */
  lcd_init();
  UG_FillScreen(0);
  UG_FontSelect(&FONT_10X16);

  static buttons_events_type_t events = 0;
  static buttons_events_type_t last_events = 0;
  while (1)
  {
    delay_ms(10);
    buttons_clock();

//    UG_FillScreen(0);
    UG_PutString(10, 10, itoa(buttons_get_onoff_state ()));
    UG_PutString(10, 50, itoa(buttons_get_up_state ()));
    UG_PutString(10, 90, itoa(buttons_get_down_state ()));

    events = buttons_get_events ();
    if (events != 0)
    {
      last_events = events;
      buttons_clear_all_events();
    }
    UG_PutString(10, 125, "   ");
    UG_PutString(10, 125, itoa((uint32_t) last_events));
  }
}

void SetSysClockTo128Mhz(void)
{
  ErrorStatus HSEStartUpStatus;

  /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if (HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);

    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1);

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_16);

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
  else
  { /* If HSE fails to start-up, the application will have wrong clock configuration.
       User can add here some code to deal with this error */

    /* Go to infinite loop */
    while (1)
    {
    }
  }
}
