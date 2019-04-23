
#include "app_util_platform.h"
#include "nrf_drv_uart.h"
#include "nrf_delay.h"
#include "boards.h"
#include "main.h"
#include "pins.h"
#include "lcd.h"
#include "ugui.h"
#include "fonts.h"

/* Variable definition */
UG_GUI gui;



/* Function prototype */
static void system_power(bool state);
static void gpio_init(void);



/**
 * @brief Application main entry.
 */
int main(void)
{
  gpio_init();
  lcd_init();
  system_power(true);

  UG_ConsoleSetArea(0, 0, 63, 127);
  UG_ConsoleSetForecolor(C_WHITE);

  UG_FontSelect(&MY_FONT_8X12);
  static const char degC[] = { 31, 'C' };
  UG_ConsolePutString("23 ");
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
