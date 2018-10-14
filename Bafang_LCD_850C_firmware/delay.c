/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x.h"

static unsigned int _ms;

void delay (unsigned int ms)
{
  _ms = 1;
  while (ms >= _ms) ;
}

void SysTick_Handler(void) // runs every 1ms
{
  // for delay_ms ()
  _ms++;
}

void delay_init (void)
{
  /* Setup SysTick Timer for 1 millisecond interrupts, also enables Systick and Systick-Interrupt */
  if (SysTick_Config(SystemCoreClock / 1000))
  {
    /* Capture error */
    while (1);
  }
}
