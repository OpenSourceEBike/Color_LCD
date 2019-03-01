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

void usart1_init(void);
uint8_t* usart1_get_rx_buffer (void);

#endif
