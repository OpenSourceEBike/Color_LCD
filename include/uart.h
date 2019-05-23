#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

void uart_init(void);
const uint8_t* uart_get_rx_buffer_rdy(void);
uint8_t* uart_get_tx_buffer(void);
bool uart_send_tx_buffer(void);

#endif /* INCLUDE_UART_H_ */
