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

#define UART_NUMBER_DATA_BYTES_TO_RECEIVE   25  // change this value depending on how many data bytes there is to receive ( Package = one start byte + data bytes + two bytes 16 bit CRC )
#define UART_NUMBER_DATA_BYTES_TO_SEND      6   // change this value depending on how many data bytes there is to send ( Package = one start byte + data bytes + two bytes 16 bit CRC )
#define UART_MAX_NUMBER_MESSAGE_ID          8   // change this value depending on how many different packages there is to send

void usart1_init(void);
uint8_t* usart1_get_rx_buffer(void);
uint8_t usart1_received_package(void);
void usart1_reset_received_package(void);
void usart1_send_byte_and_block(uint8_t ui8_byte);
void usart1_start_dma_transfer(void);

#endif
