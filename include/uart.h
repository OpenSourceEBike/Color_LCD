#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

void uart_init(void);
const uint8_t* uart_get_rx_buffer_rdy(void);
uint8_t* uart_get_tx_buffer(void);
void uart_send_tx_buffer(uint8_t* tx_buffer);

#define UART_NUMBER_DATA_BYTES_TO_RECEIVE   25  // change this value depending on how many data bytes there is to receive ( Package = one start byte + data bytes + two bytes 16 bit CRC )
#define UART_NUMBER_DATA_BYTES_TO_SEND      6   // change this value depending on how many data bytes there is to send ( Package = one start byte + data bytes + two bytes 16 bit CRC )
#define UART_MAX_NUMBER_MESSAGE_ID          8   // change this value depending on how many different packages there is to send

// Public only for debugging/fault messages
extern uint8_t uart_rx_state_machine;

#endif /* INCLUDE_UART_H_ */
