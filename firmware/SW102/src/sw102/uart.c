/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */
#include "common.h"
#include "nrf_drv_uart.h"
#include "uart.h"
#include "utils.h"
#include "assert.h"
#include "app_util_platform.h"

nrf_drv_uart_t uart0 = NRF_DRV_UART_INSTANCE(UART0);
typedef struct uart_rx_buff_typedef uart_rx_buff_typedef;
struct uart_rx_buff_typedef
{
  uint8_t uart_rx_data[UART_NUMBER_DATA_BYTES_TO_RECEIVE];
  uart_rx_buff_typedef* next_uart_rx_buff;
};
uart_rx_buff_typedef* uart_rx_buffer;
volatile uint8_t* uart_rx_data_rdy;
uint8_t uart_number_bytes_rx, uart_number_bytes_tx;

uint8_t uart_buffer0_tx[UART_NUMBER_DATA_BYTES_TO_SEND];

uint8_t m_uart_rx_len;

/* Function prototype */
static void uart_event_handler(nrf_drv_uart_event_t *p_event, void *p_context);

/**
 * @brief Init UART peripheral
 */
void uart_init(void)
{
  /* Init RX buffer */
  static uart_rx_buff_typedef rxb1, rxb2;
  rxb1.next_uart_rx_buff = &rxb2;
  rxb2.next_uart_rx_buff = &rxb1;
  uart_rx_buffer = &rxb1;

  /* Init driver */
  nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
  uart_config.pselrxd = UART_RX__PIN;
  uart_config.pseltxd = UART_TX__PIN;

  APP_ERROR_CHECK(nrf_drv_uart_init(&uart0, &uart_config, uart_event_handler));
  /* Enable & start RX (bytewise scanning for start byte) */
  nrf_drv_uart_rx_enable(&uart0);
  APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_rx_buffer->uart_rx_data[0], 1));
}

/**
 * @brief Returns pointer to RX buffer ready for parsing or NULL
 */
const uint8_t* uart_get_rx_buffer_rdy(void)
{
  uint8_t* rx_rdy;

  // VERY paranoid but it is possible that uart_rx_data_rdy
  // is set from IRQ during hand-over and gets NULLed right away.
  CRITICAL_REGION_ENTER();
  {
    rx_rdy = (uint8_t*) uart_rx_data_rdy;
    uart_rx_data_rdy = NULL;
  }
  CRITICAL_REGION_EXIT();

  if (rx_rdy != NULL)
  {
    uint16_t crc_rx = 0xffff;
    for (uint8_t ui8_i = 0; ui8_i < m_uart_rx_len; ui8_i++)
      crc16(rx_rdy[ui8_i], &crc_rx);

    if (((((uint16_t) rx_rdy[m_uart_rx_len + 1]) << 8)
        + ((uint16_t) rx_rdy[m_uart_rx_len])) != crc_rx)
      rx_rdy = NULL;  // Invalidate buffer if CRC not OK
  }

  return rx_rdy;
}

/**
 * @brief Returns pointer to TX buffer
 */
uint8_t* uart_get_tx_buffer(void)
{
  return uart_buffer0_tx;
}

/**
 * @brief Send TX buffer over UART. Returns false on error
 */
void uart_send_tx_buffer(uint8_t *tx_buffer, uint8_t ui8_len)
{
  ret_code_t err_code = nrf_drv_uart_tx(&uart0, tx_buffer, ui8_len);

  APP_ERROR_CHECK(err_code);
}

/* Event handler */

static void uart_event_handler(nrf_drv_uart_event_t *p_event, void *p_context)
{
  static uint8_t uart_rx_state_machine = 0;

  switch (p_event->type)
  {
    case NRF_DRV_UART_EVT_TX_DONE:
      break;

    case NRF_DRV_UART_EVT_ERROR:
      // Just restart our reads and try to keep going
      // The only error we expect is overrun or framing
      // assert(p_event->data.error.error_mask & (UART_ERRORSRC_OVERRUN_Msk | UART_ERRORSRC_FRAMING_Msk | UART_ERRORSRC_BREAK_Msk));

      uart_rx_state_machine = 0;
      APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_rx_buffer->uart_rx_data[0], 1));
      break;

    case NRF_DRV_UART_EVT_RX_DONE:
      switch (uart_rx_state_machine)
      {
        /* End of bytewise RX */
        case 0:
          if (uart_rx_buffer->uart_rx_data[0] == 0x43) // see if we get start package byte
          {
            uart_rx_state_machine = 1;
            APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_rx_buffer->uart_rx_data[1], 1));
          }
          else
            APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_rx_buffer->uart_rx_data[0], 1));
          break;

        // get the lenght of the package and receive such number of bytes
        case 1:
          m_uart_rx_len = uart_rx_buffer->uart_rx_data[1];
          uart_rx_state_machine = 2;
          APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_rx_buffer->uart_rx_data[2], m_uart_rx_len));
          break;

        /* End of stream RX */
        case 2:
          /* Signal that we have a full package to be processed */
          uart_rx_data_rdy = uart_rx_buffer->uart_rx_data;
          /* Switch buffer */
          uart_rx_buffer = uart_rx_buffer->next_uart_rx_buff;
          /* Start bytewise RX again */
          uart_rx_state_machine = 0;
          APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_rx_buffer->uart_rx_data[0], 1));
          break;

        default:
          assert(0);
          break;
      }
      break;

    default:
      break;
  }
}
