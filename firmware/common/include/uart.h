#pragma once

#include <stdint.h>

void uart_init(void);
uint8_t uart_get_stream_version(void);
void uart_set_stream_version(uint8_t version);
const uint8_t* uart_get_rx_buffer_rdy(void);
uint8_t* uart_get_tx_buffer(void);
void uart_send_tx_buffer(uint8_t *tx_buffer);

#define UART_NUMBER_DATA_BYTES_TO_RECEIVE_V19   25  // change this value depending on how many data bytes there is to receive ( Package = one start byte + data bytes + two bytes 16 bit CRC )
#define UART_NUMBER_DATA_BYTES_TO_SEND_V19      6   // change this value depending on how many data bytes there is to send ( Package = one start byte + data bytes + two bytes 16 bit CRC )
#define UART_MAX_NUMBER_MESSAGE_ID_V19          8   // change this value depending on how many different packages there is to send

#define UART_NUMBER_DATA_BYTES_TO_RECEIVE_V20   26
#define UART_NUMBER_DATA_BYTES_TO_SEND_V20      7
#define UART_MAX_NUMBER_MESSAGE_ID_V20          6

#define UART_NUMBER_DATA_BYTES_TO_RECEIVE       25
#define UART_NUMBER_DATA_BYTES_TO_SEND          6

#define UART_NUMBER_CRC_BYTES                   2
#define UART_NUMBER_START_BYTES                 1
