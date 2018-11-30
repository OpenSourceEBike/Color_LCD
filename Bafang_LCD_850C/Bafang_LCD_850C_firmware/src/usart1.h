/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _USART1_H_
#define _USART1_H_

#include "stdio.h"

// Define for the NVIC IRQChannel Preemption Priority
// lower number has higher priority
#define USART1_PRIORITY       6

void usart1_init(void);
void usart1_data_clock(void);
uint8_t usart1_received_first_package(void);

#endif
