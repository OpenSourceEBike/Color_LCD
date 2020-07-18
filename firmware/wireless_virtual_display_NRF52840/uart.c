/*
 * Bafang LCD SW102 Bluetooth firmware
 * Copyright (C) lowPerformer, 2019.
 * 
 * TSDZ2 EBike wireless firmware
 * Copyright (C) Casainho, 2020
 *
 * Released under the GPL License, Version 3
 */

#include <string.h>
#include "main.h"
#include "nrf_drv_uart.h"
#include "uart.h"
// #include "utils.h"
#include "assert.h"
#include "app_util_platform.h"
#include "app_uart.h"
#include "custom_board.h"
// #include "common.h"

extern uint32_t _app_uart_init(const app_uart_comm_params_t * p_comm_params,
    app_uart_buffers_t *     p_buffers,
    app_uart_event_handler_t event_handler,
    app_irq_priority_t       irq_priority);
// extern uint8_t app_uart_get(void);

#define UART_IRQ_PRIORITY                       APP_IRQ_PRIORITY_LOW

/**
 *@breif UART configuration structure
 */
static const app_uart_comm_params_t comm_params =
{
    .rx_pin_no  = RX_PIN_NUMBER,
    .tx_pin_no  = TX_PIN_NUMBER,
    .rts_pin_no = RTS_PIN_NUMBER,
    .cts_pin_no = CTS_PIN_NUMBER,
    //Below values are defined in ser_config.h common for application and connectivity
    .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
    .use_parity   = false,
    .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud19200
};

uint8_t ui8_rx_buffer[UART_NUMBER_DATA_BYTES_TO_RECEIVE];
uint8_t ui8_tx_buffer[UART_NUMBER_DATA_BYTES_TO_SEND];
volatile uint8_t ui8_received_package_flag = 0;

uint8_t* uart_get_tx_buffer(void)
{
  return ui8_tx_buffer;
}

uint8_t* usart1_get_rx_buffer(void)
{
  return ui8_rx_buffer;
}

uint8_t usart1_received_package(void)
{
  return ui8_received_package_flag;
}

void usart1_reset_received_package(void)
{
  ui8_received_package_flag = 0;
}

void uart_evt_callback(app_uart_evt_t * uart_evt)
{
  // uint8_t ui8_byte_received;
  // static uint8_t ui8_state_machine = 0;
  // static uint8_t ui8_rx[UART_NUMBER_DATA_BYTES_TO_RECEIVE];
  // static uint8_t ui8_rx_cnt = 0;
  // uint8_t ui8_i;
  // uint16_t ui16_crc_rx;

  // switch (uart_evt->evt_type)
  // {
  //   case APP_UART_DATA:
  //     //Data is ready on the UART
  //     ui8_byte_received = app_uart_get();
  //     switch (ui8_state_machine)
  //     {
  //       case 0:
  //       if (ui8_byte_received == 0x43) { // see if we get start package byte
  //         ui8_rx[0] = ui8_byte_received;
  //         ui8_state_machine = 1;
  //       }
  //       else {
  //         ui8_state_machine = 0;
  //       }

  //       ui8_rx_cnt = 0;
  //       break;

  //       case 1:
  //         ui8_rx[1] = ui8_byte_received;
  //         ui8_state_machine = 2;
  //       break;

  //       case 2:
  //       ui8_rx[ui8_rx_cnt + 2] = ui8_byte_received;
  //       ++ui8_rx_cnt;

  //       // reset if it is the last byte of the package and index is out of bounds
  //       if (ui8_rx_cnt >= ui8_rx[1])
  //       {
  //         ui8_state_machine = 0;

  //         // just to make easy next calculations
  //         ui16_crc_rx = 0xffff;
  //         for (ui8_i = 0; ui8_i < ui8_rx[1]; ui8_i++)
  //         {
  //           crc16(ui8_rx[ui8_i], &ui16_crc_rx);
  //         }

  //         // if CRC is correct read the package
  //         if (((((uint16_t) ui8_rx[ui8_rx[1] + 1]) << 8) +
  //               ((uint16_t) ui8_rx[ui8_rx[1]])) == ui16_crc_rx)
  //         {
  //           // copy to the other buffer only if we processed already the last package
  //           if(!ui8_received_package_flag)
  //           {
  //             ui8_received_package_flag = 1;

  //             // store the received data to rx_buffer
  //             memcpy(ui8_rx_buffer, ui8_rx, ui8_rx[1] + 2);
  //           }
  //         }
  //       }
  //       break;

  //       default:
  //         ui8_state_machine = 0;
  //         break;
  //     }
  //   break;

  //   case APP_UART_TX_EMPTY:
  //     //Data has been successfully transmitted on the UART
  //     break;

  //   case APP_UART_COMMUNICATION_ERROR:
  //       ui8_state_machine = 0;
  //     break;

  //   default:
  //     break;
  // }
}

/**
 * @brief Init UART peripheral
 */
void uart_init(void)
{
  uint32_t err_code;
  app_uart_buffers_t buffers;
  static uint8_t tx_buf[128]; // must be equal or higher than UART_NUMBER_DATA_BYTES_TO_SEND and power of 2

  buffers.tx_buf = tx_buf;
  buffers.tx_buf_size = sizeof(tx_buf);
  err_code = app_uart_init(&comm_params, &buffers, uart_evt_callback, UART_IRQ_PRIORITY);

  APP_ERROR_CHECK(err_code);
}

/**
 * @brief Returns pointer to RX buffer ready for parsing or NULL
 */
const uint8_t* uart_get_rx_buffer_rdy(void)
{
  if(!usart1_received_package()) {
    return NULL;
  }

  uint8_t *r = usart1_get_rx_buffer();
  usart1_reset_received_package();
  return r;
}

/**
 * @brief Send TX buffer over UART.
 */
void uart_send_tx_buffer(uint8_t *tx_buffer, uint8_t ui8_len)
{
  uint32_t err_code;

  for (uint8_t i = 0; i < ui8_len; i++)
  {
    err_code = app_uart_put(tx_buffer[i]);
(void) err_code;
// assume that buffer will never get full, like for instance when we are debugging
//    if (err_code != 0)
//      APP_ERROR_CHECK(err_code);
  }
}
