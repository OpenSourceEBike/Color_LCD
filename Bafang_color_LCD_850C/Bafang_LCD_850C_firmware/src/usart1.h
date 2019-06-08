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
uint8_t* usart1_get_rx_buffer(void);
uint8_t usart1_received_package(void);
void usart1_reset_received_package(void);
void usart1_send_byte_and_block(uint8_t ui8_byte);
void usart1_start_dma_transfer(void);
void usart1_simulation_clock(void);

#endif
