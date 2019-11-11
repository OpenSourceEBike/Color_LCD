/*
 * LCD3 firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _MAIN_H_
#define _MAIN_H_

// INTERRUPTS PRIORITIES
// Define for the NVIC IRQChannel Preemption Priority
// lower number has higher priority
#define USART1_INTERRUPT_PRIORITY       3
#define USART1_DMA_INTERRUPT_PRIORITY   4
#define TIM4_INTERRUPT_PRIORITY         5
#define RTC_INTERRUT_PRIORITY           6



#define  MAIN_SCREEN_FIELD_LABELS_COLOR C_DIM_GRAY

#endif // _MAIN_H_
