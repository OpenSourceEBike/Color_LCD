#pragma once

#include <stdint.h>

void uart_init(void);
uint8_t uart_get_stream_version(void);
void uart_set_stream_version(uint8_t version);
const uint8_t* uart_get_rx_buffer_rdy(void);
uint8_t* uart_get_tx_buffer(void);
void uart_send_tx_buffer(uint8_t *tx_buffer);

#define UART_NUMBER_DATA_BYTES_TO_RECEIVE       28
#define UART_NUMBER_DATA_BYTES_TO_SEND          84

#define UART_NUMBER_CRC_BYTES                   2
#define UART_NUMBER_START_BYTES                 1
