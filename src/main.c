
#include "ble_uart.h"
#include "nrf_delay.h"
#include "main.h"
#include "pins.h"
#include "lcd.h"
#include "ugui.h"
#include "fonts.h"

#include "nrf_drv_uart.h"

/* Variable definition */
UG_GUI gui;

nrf_drv_uart_t uart0 = NRF_DRV_UART_INSTANCE(UART0);
uint8_t uart_byte_rx;

/* Function prototype */
static void system_power(bool state);
static void gpio_init(void);
static void uart_init(void);
static void uart_event_handler(nrf_drv_uart_event_t *p_event, void *p_context);

/**
 * @brief Application main entry.
 */
int main(void)
{
  gpio_init();
  lcd_init();
  uart_init();
  system_power(true);

  UG_ConsoleSetArea(0, 0, 63, 127);
  UG_ConsoleSetForecolor(C_WHITE);

  UG_FontSelect(&MY_FONT_BATTERY);
  UG_ConsolePutString("5\n");
  UG_ConsolePutString("4\n");
  UG_ConsolePutString("3\n");
  UG_ConsolePutString("2\n");
  UG_ConsolePutString("1\n");
  //UG_ConsolePutString("0\n");

  UG_FontSelect(&MY_FONT_8X12);
  static const char degC[] = { 31, 'C', 0 };
  UG_ConsolePutString(degC);

  ble_stack_init();
  gap_params_init();
  services_init();
  advertising_init();
  conn_params_init();

  // Enter main loop.
  while (1)
  {
  }
}

/**
 * @brief Hold system power (true) or not (false)
 */
static void system_power(bool state)
{
  if (state)
    nrf_gpio_pin_set(SYSTEM_POWER_HOLD__PIN);
  else
    nrf_gpio_pin_clear(SYSTEM_POWER_HOLD__PIN);
}



/* Hardware Initialization */

/**
 * @brief GPIO initialization.
 */
static void gpio_init(void)
{
  /* POWER_HOLD */
  nrf_gpio_cfg_output(SYSTEM_POWER_HOLD__PIN);

  /* LCD (none SPI) */
  nrf_gpio_cfg_output(LCD_COMMAND_DATA__PIN);
  nrf_gpio_pin_set(LCD_COMMAND_DATA__PIN);
  nrf_gpio_cfg_output(LCD_RES__PIN);
  nrf_gpio_pin_clear(LCD_RES__PIN); // Hold LCD in reset until initialization
}

static void uart_init(void)
{
  nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
  uart_config.pselrxd = UART_RX__PIN;
  uart_config.pseltxd = UART_TX__PIN;
  nrf_drv_uart_init(&uart0, &uart_config, uart_event_handler);
  /* Enable & start RX */
  nrf_drv_uart_rx_enable(&uart0);
  nrf_drv_uart_rx(&uart0, &uart_byte_rx, 1);
}


/* Event handler */
static void uart_event_handler(nrf_drv_uart_event_t *p_event, void *p_context)
{
  if (p_event->type == NRF_DRV_UART_EVT_TX_DONE)
    while (1);
}

/**@brief       Callback function for errors, asserts, and faults.
 *
 * @details     This function is called every time an error is raised in app_error, nrf_assert, or
 *              in the SoftDevice.
 *
 *              DEBUG_NRF flag must be set to trigger asserts!
 *
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
  while (1);
}
