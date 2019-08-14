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

nrf_drv_uart_t uart0 = NRF_DRV_UART_INSTANCE(UART0);
uint8_t uart_buffer0_rx[UART_NUMBER_DATA_BYTES_TO_RECEIVE + 3];
uint8_t uart_buffer0_tx[11];
volatile bool uart_rx_new_package;

/* Function prototype */
static void uart_event_handler(nrf_drv_uart_event_t *p_event, void *p_context);

/**
 * @brief Init UART peripheral
 */
void uart_init(void)
{
  /* Init driver */
  nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
  uart_config.pselrxd = UART_RX__PIN;
  uart_config.pseltxd = UART_TX__PIN;

  APP_ERROR_CHECK(nrf_drv_uart_init(&uart0, &uart_config, uart_event_handler));
  /* Enable & start RX (bytewise scanning for start byte) */
  nrf_drv_uart_rx_enable(&uart0);
  APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_buffer0_rx[0], 1));
}

/**
 * @brief Returns pointer to RX buffer ready for parsing or NULL
 */
const uint8_t* uart_get_rx_buffer_rdy(void)
{
  if (uart_rx_new_package)
  {
    uart_rx_new_package = false;  // pull only once
    return uart_buffer0_rx;
  }

  return NULL;
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
void uart_send_tx_buffer(uint8_t *tx_buffer)
{
  ret_code_t err_code = nrf_drv_uart_tx(&uart0, tx_buffer,
  UART_NUMBER_DATA_BYTES_TO_SEND + 3);

  APP_ERROR_CHECK(err_code);
}

/* Event handler */

static void uart_event_handler(nrf_drv_uart_event_t *p_event, void *p_context)
{
  static uint8_t uart_rx_state_machine;

  switch (p_event->type)
  {
  case NRF_DRV_UART_EVT_TX_DONE:
    break;

  case NRF_DRV_UART_EVT_ERROR:
    // Just restart our reads and try to keep going
    // The only error we expect is overrun or framing
    // assert(p_event->data.error.error_mask & (UART_ERRORSRC_OVERRUN_Msk | UART_ERRORSRC_FRAMING_Msk | UART_ERRORSRC_BREAK_Msk));

    uart_rx_state_machine = 0;
    APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_buffer0_rx[0], 1));
    break;

  case NRF_DRV_UART_EVT_RX_DONE:
    switch (uart_rx_state_machine)
    {
    /* End of bytewise RX */
    case 0:
      uart_rx_new_package = false;  // RX ongoing. Invalidate flag.
      if (uart_buffer0_rx[0] == 0x43) // see if we get start package byte
      {
        uart_rx_state_machine = 1;
        APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_buffer0_rx[1],
            UART_NUMBER_DATA_BYTES_TO_RECEIVE + 2)); // Start RX of the remaining stream at once
      }
      else
        APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_buffer0_rx[0], 1)); // Next bytewise RX to check for start byte
      break;

    /* End of stream RX */
    case 1: {
      // validation of the package data
      // last byte is the checksum
      uint16_t ui16_crc_rx = 0xffff;
      for (uint8_t ui8_i = 0; ui8_i <= UART_NUMBER_DATA_BYTES_TO_RECEIVE;
          ui8_i++)
      {
        crc16(uart_buffer0_rx[ui8_i], &ui16_crc_rx);
      }

      if (((((uint16_t) uart_buffer0_rx[UART_NUMBER_DATA_BYTES_TO_RECEIVE + 2])
          << 8)
          + ((uint16_t) uart_buffer0_rx[UART_NUMBER_DATA_BYTES_TO_RECEIVE + 1]))
          == ui16_crc_rx)
        // {
        // copy to the other buffer only if we processed already the last package
        // FIXME - this flipflop between buffers can be done with just changing a pointer, no need for an expensive memcpy
        // if(!ui8_received_package_flag)
        //{
        uart_rx_new_package = true; // signal that we have a full package to be processed

      // store the received data to rx_buffer
      // memcpy(ui8_rx_buffer, ui8_rx, UART_NUMBER_DATA_BYTES_TO_RECEIVE + 1);
      //}

      /* Start bytewise RX again (regardless of if we liked the current packet or not) */
      uart_rx_state_machine = 0;
      APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_buffer0_rx[0], 1));
    }
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
