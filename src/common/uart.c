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
  uint8_t uart_rx_data[UART_NUMBER_START_BYTES + UART_NUMBER_DATA_BYTES_TO_RECEIVE + UART_NUMBER_CRC_BYTES];
  uart_rx_buff_typedef* next_uart_rx_buff;
};
uart_rx_buff_typedef* uart_rx_buffer;
volatile uint8_t* uart_rx_data_rdy;

uint8_t uart_buffer0_tx[UART_NUMBER_START_BYTES + UART_NUMBER_DATA_BYTES_TO_SEND + UART_NUMBER_CRC_BYTES];

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
  // is set from IRQ during hand-over
  CRITICAL_REGION_ENTER();
  {
    rx_rdy = (uint8_t*) uart_rx_data_rdy;
    uart_rx_data_rdy = NULL;
  }
  CRITICAL_REGION_EXIT();

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
void uart_send_tx_buffer(uint8_t *tx_buffer)
{
  ret_code_t err_code = nrf_drv_uart_tx(&uart0, tx_buffer,
  UART_NUMBER_START_BYTES + UART_NUMBER_DATA_BYTES_TO_SEND + UART_NUMBER_CRC_BYTES);

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
        APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_rx_buffer->uart_rx_data[1],
            UART_NUMBER_DATA_BYTES_TO_RECEIVE + UART_NUMBER_CRC_BYTES)); // Start RX of the remaining stream at once
      }
      else
        APP_ERROR_CHECK(nrf_drv_uart_rx(&uart0, &uart_rx_buffer->uart_rx_data[0], 1)); // Next bytewise RX to check for start byte
      break;

    /* End of stream RX */
    case 1:
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
