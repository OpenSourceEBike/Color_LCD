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

nrf_drv_uart_t uart0 = NRF_DRV_UART_INSTANCE(UART0);
uint8_t uart_buffer0_rx[26];
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
  nrf_drv_uart_init(&uart0, &uart_config, uart_event_handler);
  /* Enable & start RX (bytewise scanning for start byte) */
  nrf_drv_uart_rx_enable(&uart0);
  nrf_drv_uart_rx(&uart0, &uart_buffer0_rx[0], 1);
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
bool uart_send_tx_buffer(void)
{
  ret_code_t err_code = nrf_drv_uart_tx(&uart0, uart_buffer0_tx, 11);

  return (err_code == NRF_SUCCESS) ? true : false;
}



/* Event handler */

static void uart_event_handler(nrf_drv_uart_event_t *p_event, void *p_context)
{
  static uint8_t uart_rx_state_machine;

  if (p_event->type == NRF_DRV_UART_EVT_RX_DONE)
  {
    switch (uart_rx_state_machine)
    {
    /* End of bytewise RX */
    case 0:
      uart_rx_new_package = false;  // RX ongoing. Invalidate flag.
      if (uart_buffer0_rx[0] == 0x43) // see if we get start package byte
      {
        nrf_drv_uart_rx(&uart0, &uart_buffer0_rx[1], 25);  // Start RX of the remaining stream at once
        uart_rx_state_machine = 1;
      }
      else
        nrf_drv_uart_rx(&uart0, &uart_buffer0_rx[0], 1);  // Next bytewise RX to check for start byte
      break;

    /* End of stream RX */
    case 1:
      /* Start bytewise RX again */
      nrf_drv_uart_rx(&uart0, &uart_buffer0_rx[0], 1);
      uart_rx_state_machine = 0;

      uart_rx_new_package = true; // signal that we have a full package to be processed
      break;
    }
  }
}
