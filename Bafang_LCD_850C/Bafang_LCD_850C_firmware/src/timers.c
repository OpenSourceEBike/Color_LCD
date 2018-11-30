/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x.h"
#include "lcd.h"

static volatile uint32_t _ms;
volatile uint32_t timer_base_counter_1ms = 0;

void delay_ms (uint32_t ms)
{
  _ms = 1;
  while (ms >= _ms) ;
}

void SysTick_Handler(void) // runs every 1ms
{
  static uint8_t ui8_100ms_timmer_counter = 0;

  _ms++; // for delay_ms ()

  timer_base_counter_1ms++;

  // calc wh every 100ms
  if(ui8_100ms_timmer_counter > 100)
  {
    ui8_100ms_timmer_counter = 0;

    // must be called every 100ms
    calc_wh();
  }
  ui8_100ms_timmer_counter++;
}

void systick_init (void)
{
  /* Setup SysTick Timer for 1 millisecond interrupts, also enables Systick and Systick-Interrupt */
  if (SysTick_Config(SystemCoreClock / 1000))
  {
    /* Capture error */
    while (1);
  }
}

uint32_t get_timer_base_counter_1ms (void)
{
  return timer_base_counter_1ms;
}
