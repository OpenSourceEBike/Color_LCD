
#include "ble_uart.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "main.h"
#include "button.h"
#include "lcd.h"
#include "ugui.h"
#include "fonts.h"

#include "nrf_drv_uart.h"

/* Variable definition */
UG_GUI gui;

nrf_drv_uart_t uart0 = NRF_DRV_UART_INSTANCE(UART0);
uint8_t uart_buffer_rx[26];
volatile bool uart_rx_new_package;

/* Buttons */
Button buttonM, buttonDWN, buttonUP, buttonPWR;

/* App Timer */
#define OP_QUEUES_SIZE      4
#define APP_TIMER_PRESCALER 0
APP_TIMER_DEF(button_poll_timer_id); /* Button timer. */
#define BUTTON_POLL_INTERVAL APP_TIMER_TICKS(10/*ms*/, APP_TIMER_PRESCALER)
APP_TIMER_DEF(seconds_timer_id); /* Second counting timer. */
#define SECONDS_INTERVAL APP_TIMER_TICKS(1000/*ms*/, APP_TIMER_PRESCALER)
volatile uint32_t seconds_since_startup, seconds_since_reset;
const uint8_t test_ping[] = {'P', 'i', 'n', 'g', '\n' };

/* Function prototype */
static void system_power(bool state);
static void gpio_init(void);
static void uart_init(void);
static void init_app_timers(void);
static void uart_event_handler(nrf_drv_uart_event_t *p_event, void *p_context);
static void button_poll_timer_timeout(void * p_context);
static void seconds_timer_timeout(void * p_context);



/**
 * @brief Application main entry.
 */
int main(void)
{
  gpio_init();
  lcd_init();
  uart_init();
  ble_init();
  init_app_timers();
  system_power(true);

  UG_ConsoleSetArea(0, 0, 63, 127);
  UG_ConsoleSetForecolor(C_WHITE);
/*
  UG_FontSelect(&MY_FONT_BATTERY);
  UG_ConsolePutString("5\n");
  UG_ConsolePutString("4\n");
  UG_ConsolePutString("3\n");
  UG_ConsolePutString("2\n");
  UG_ConsolePutString("1\n");
  UG_ConsolePutString("0\n");

  UG_FontSelect(&MY_FONT_8X12);
  static const char degC[] = { 31, 'C', 0 };
  UG_ConsolePutString(degC);
*/
  UG_FontSelect(&MY_FONT_8X12);
  // Enter main loop.
  while (1)
  {
    if (ButtonDoubleClicked(&buttonDWN))
      UG_ConsolePutString("D_Click\n");

    if (ButtonClicked(&buttonDWN))
      UG_ConsolePutString("Click\n");

    if (ButtonLongClicked(&buttonDWN))
      UG_ConsolePutString("L_Click\n");

    if (ButtonReleased(&buttonDWN))
      UG_ConsolePutString("Release\n");
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

static void gpio_init(void)
{
  /* POWER_HOLD */
  nrf_gpio_cfg_output(SYSTEM_POWER_HOLD__PIN);

  /* LCD (none SPI) */
  nrf_gpio_cfg_output(LCD_COMMAND_DATA__PIN);
  nrf_gpio_pin_set(LCD_COMMAND_DATA__PIN);
  nrf_gpio_cfg_output(LCD_RES__PIN);
  nrf_gpio_pin_clear(LCD_RES__PIN); // Hold LCD in reset until initialization

  /* Buttons */
  InitButton(&buttonPWR, BUTTON_PWR__PIN, NRF_GPIO_PIN_NOPULL, BUTTON_ACTIVE_HIGH);
  InitButton(&buttonM, BUTTON_M__PIN, NRF_GPIO_PIN_PULLUP, BUTTON_ACTIVE_LOW);
  InitButton(&buttonUP, BUTTON_UP__PIN, NRF_GPIO_PIN_PULLUP, BUTTON_ACTIVE_LOW);
  InitButton(&buttonDWN, BUTTON_DOWN__PIN, NRF_GPIO_PIN_PULLUP, BUTTON_ACTIVE_LOW);
}

static void uart_init(void)
{
  nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
  uart_config.pselrxd = UART_RX__PIN;
  uart_config.pseltxd = UART_TX__PIN;
  nrf_drv_uart_init(&uart0, &uart_config, uart_event_handler);
  /* Enable & start RX */
  nrf_drv_uart_rx_enable(&uart0);
  nrf_drv_uart_rx(&uart0, &uart_buffer_rx[0], 1);
}

static void init_app_timers(void)
{
  // Start APP_TIMER to generate timeouts.
  APP_TIMER_INIT(APP_TIMER_PRESCALER, OP_QUEUES_SIZE, NULL);

  // Create&Start button_poll_timer
  app_timer_create(&button_poll_timer_id, APP_TIMER_MODE_REPEATED, button_poll_timer_timeout);
  app_timer_start(button_poll_timer_id, BUTTON_POLL_INTERVAL, NULL);

  // Create&Start timers.
  app_timer_create(&seconds_timer_id, APP_TIMER_MODE_REPEATED, seconds_timer_timeout);
  app_timer_start(seconds_timer_id, SECONDS_INTERVAL, NULL);
}



/* Event handler */

static void uart_event_handler(nrf_drv_uart_event_t *p_event, void *p_context)
{
  static uint8_t uart_rx_state_machine;

  if (p_event->type == NRF_DRV_UART_EVT_RX_DONE)
  {
    switch (uart_rx_state_machine)
    {
    case 0:
      if (uart_buffer_rx[0] == 0x43) // see if we get start package byte
      {
        nrf_drv_uart_rx(&uart0, &uart_buffer_rx[1], 25);  // Start RX of the remaining stream
        uart_rx_state_machine = 1;
      }
      else
        nrf_drv_uart_rx(&uart0, &uart_buffer_rx[0], 1);  // Next byte-wise RX to check for start byte
      break;

    case 1:
      /* Start byte-wise RX again after the package has been processed */
      uart_rx_state_machine = 0;
      uart_rx_new_package = true; // signal that we have a full package to be processed
      break;

    default:
      uart_rx_state_machine = 0;
      break;
    }
  }
}

static void button_poll_timer_timeout(void *p_context)
{
    UNUSED_PARAMETER(p_context);

    PollButton(&buttonPWR);
    PollButton(&buttonM);
    PollButton(&buttonUP);
    PollButton(&buttonDWN);
}

static void seconds_timer_timeout(void *p_context)
{
    UNUSED_PARAMETER(p_context);

    seconds_since_startup++;
    seconds_since_reset++;

    nrf_drv_uart_tx(&uart0, test_ping, 5);
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
