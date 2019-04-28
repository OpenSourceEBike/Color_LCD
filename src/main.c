
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "boards.h"
#include "main.h"
#include "pins.h"
#include "lcd.h"
#include "ugui.h"
#include "fonts.h"
#include "nrfx_uart.h"

/* Variable definition */
UG_GUI gui;

const nrfx_uart_t uart0 = NRFX_UART_INSTANCE(UART0);
uint8_t uart_byte_rx;



/* Function prototype */
static void system_power(bool state);
static void gpio_init(void);
static void uart_init(void);
static void uart_event_handler(nrfx_uart_event_t const *p_event, void *p_context);



/**
 * @brief Application main entry.
 */
int main(void)
{
  gpio_init();
  uart_init();
  lcd_init();
  system_power(true);

  UG_ConsoleSetArea(0, 0, 63, 127);
  UG_ConsoleSetForecolor(C_WHITE);

  UG_FontSelect(&MY_FONT_BATTERY);
  UG_ConsolePutString("5\n");
  UG_ConsolePutString("4\n");
  UG_ConsolePutString("3\n");
  UG_ConsolePutString("2\n");
  UG_ConsolePutString("1\n");
  UG_ConsolePutString("0\n");

  UG_FontSelect(&MY_FONT_8X12);
  static const char degC[] = { 31, 'C' };
  UG_ConsolePutString(degC);

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
  nrfx_uart_config_t uart_config = NRFX_UART_DEFAULT_CONFIG;
  uart_config.pselrxd = UART_RX__PIN;
  uart_config.pseltxd = UART_TX__PIN;
  nrfx_uart_init(&uart0, &uart_config, uart_event_handler);
  /* Enable & start RX */
  nrfx_uart_rx_enable(&uart0);
  nrfx_uart_rx(&uart0, &uart_byte_rx, 1);
}

static void uart_event_handler(nrfx_uart_event_t const *p_event, void *p_context)
{

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
