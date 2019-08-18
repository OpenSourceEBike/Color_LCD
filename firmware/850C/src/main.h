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

#define  SMALL_TEXT_FONT              FONT_10X16
#define  REGULAR_TEXT_FONT            FONT_16X26

#define  TITLE_TEXT_FONT              FONT_16X26
#define  MEDIUM_NUMBERS_TEXT_FONT     FONT_24X40
#define  BIG_NUMBERS_TEXT_FONT        FONT_32X53

#define  CONFIGURATIONS_TEXT_FONT     FONT_12X20


#define  MAIN_SCREEN_FIELD_LABELS_COLOR C_DARK_GRAY

#endif // _MAIN_H_
