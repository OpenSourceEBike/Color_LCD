/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Released under the GPL License, Version 3
 */
#include <ble_services.h>
#include <eeprom_hw.h>
#include "app_timer.h"
#include "main.h"
#include "button.h"
#include "buttons.h"
#include "lcd.h"
#include "ugui.h"
#include "fonts.h"
#include "uart.h"
#include "utils.h"
#include "screen.h"
#include "eeprom.h"
#include "mainscreen.h"
#include "configscreen.h"
#include "nrf_soc.h"
#include "adc.h"
#include "hardfault.h"
#include "fault.h"
#include "nrf_nvic.h"
#include "rtc.h"
#include "nrf_drv_wdt.h"
#include "nrf_power.h"

/* Variable definition */

/* �GUI */
UG_GUI gui;

/* Buttons */
Button buttonM, buttonDWN, buttonUP, buttonPWR;

/* App Timer */
//APP_TIMER_DEF(button_poll_timer_id); /* Button timer. */
// #define BUTTON_POLL_INTERVAL APP_TIMER_TICKS(10/*ms*/, APP_TIMER_PRESCALER)

// APP_TIMER_DEF(seconds_timer_id); /* Second counting timer. */
// #define SECONDS_INTERVAL APP_TIMER_TICKS(1000/*ms*/, APP_TIMER_PRESCALER)

#define MSEC_PER_TICK 20

APP_TIMER_DEF(gui_timer_id); /* GUI updates counting timer. */
#define GUI_INTERVAL APP_TIMER_TICKS(MSEC_PER_TICK, APP_TIMER_PRESCALER)
volatile uint32_t gui_ticks;

// assume we should until we init_softdevice()
bool useSoftDevice = true;

/* Function prototype */
static void gpio_init(void);
static void init_app_timers(void);
/* UART RX/TX */

void lcd_power_off(uint8_t updateDistanceOdo)
{
  l3_vars.ui32_wh_x10_offset = l3_vars.ui32_wh_x10;

// save the variables on EEPROM
  eeprom_write_variables();

  // put screen all black and disable backlight
  UG_FillScreen(0);
  lcd_refresh();
  // lcd_set_backlight_intensity(0);

  // FIXME: wait for flash write to complete before powering down
  // now disable the power to all the system
  system_power(0);

  if(is_sim_motor) {
    // we are running from a bench supply on a developer's desk, so just reboot because the power supply will never die
    sd_nvic_SystemReset();
  }

  // block here till we die
  while (1)
    ;
}


/** Called just before watchdog reset, use to do whatever seems important
 * @brief WDT events handler.
 */
void wdt_event_handler(void)
{
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
    // currently we don't do anything
}

static nrf_drv_wdt_channel_id m_channel_id;

void watchdog_start() {
  // was this current boot caused because of a watchdog failure?
  uint32_t reason;

  // don't check reset reason if no soft device found, because it is not enabled if we are debugging without bluetooth
  if(sd_power_reset_reason_get(&reason) == NRF_SUCCESS) {
    if(reason & NRF_POWER_RESETREAS_DOG_MASK) {
      APP_ERROR_CHECK(sd_power_reset_reason_clr(NRF_POWER_RESETREAS_DOG_MASK));
      wd_failure_detected = true;
    }
  }

  //Configure WDT.
  nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
  APP_ERROR_CHECK(nrf_drv_wdt_init(&config, wdt_event_handler));
  APP_ERROR_CHECK(nrf_drv_wdt_channel_alloc(&m_channel_id));
  nrf_drv_wdt_enable();
}

// Will timeout after 2 secs
void watchdog_service() {
  nrf_drv_wdt_channel_feed(m_channel_id);
}

extern uint32_t __StackTop;
extern uint32_t __StackLimit;

// Returns # of unused bytes in stack
uint32_t stack_overflow_debug(void)
{
    // uint32_t stack_usage = 0;
    uint32_t offset = 0;
    volatile uint32_t * value_addr = (uint32_t *) &__StackLimit;

    for (; offset <  ((uint32_t)&__StackTop - (uint32_t)&__StackLimit); offset=offset+4)
    {
        uint32_t new_val = *(value_addr + offset);
        if (new_val != 0xDEADBEEF )
        {
            break;
        }
    }
    // stack_usage = ((uint32_t)&__StackTop - (uint32_t)&__StackLimit) - offset;

    return offset;
}

/**
 * Check if we should use the softdevice.
 */
void init_softdevice() {
  uint32_t *softdeviceaddr = (uint32_t *) 0x1000;

  if(*softdeviceaddr == 0xffffffff) // definitely no soft device
    useSoftDevice = false;

#if 0
  uint32_t *bootloaderaddr = (uint32_t *) 0x10001014;

  // If we don't have a bootloader, assume a developer is working and wants to use the debugger
  if(*bootloaderaddr == 0xffffffff)
    useSoftDevice = false; // FIXME, instead just use the supply voltage and see if it is below 14V
#endif

  // don't use softdevice while debugging
  // FIXME check if under debugger instead (using openocd callbacks)
  // useSoftDevice = false;
}


/**
 * @brief Application main entry.
 */
int main(void)
{
  init_softdevice();
  gpio_init();
  lcd_init();
  uart_init();
  battery_voltage_init();

  init_app_timers(); // Must be before ble_init! because it sets app timer prescaler

  if(useSoftDevice)
    ble_init();

  /* eeprom_init AFTER ble_init! */
  eeprom_init();
  system_power(true);

  screenShow(&bootScreen);

  // After we show the bootscreen...
  // If a button is currently pressed (likely unless developing), wait for the release (so future click events are not confused
  while(buttons_get_onoff_state() || buttons_get_m_state() || buttons_get_up_state() || buttons_get_down_state())
    ;

  watchdog_start();

  // Enter main loop.

  uint32_t lasttick = gui_ticks;
  uint32_t tickshandled = 0; // we might miss ticks if running behind, so we use our own local count to figure out if we need to run our 100ms services
  uint32_t ticksmissed = 0;
  while (1)
  {
    uint32_t tick = gui_ticks;
    if (tick != lasttick)
    {
      // if(tick < 50 * 5) // uncomment to force a watchdog failure after 5 seconds
      watchdog_service(); // we only service the watchdog if we see our ticks are still increasing

      if(tick != lasttick + 1) {
        ticksmissed += (tick - lasttick - 1); // Error!  We fell behind and missed some ticks (probably due to screen draw taking more than 20 msec)

        // if(is_sim_motor) app_error_fault_handler(FAULT_MISSEDTICK, 0, ticksmissed);
      }

      lasttick = tick;

      if(tickshandled++ % (100 / MSEC_PER_TICK) == 0) { // every 100ms

        if(stack_overflow_debug() < 128) // we are close to running out of stack
          APP_ERROR_HANDLER(FAULT_STACKOVERFLOW);
      }

      main_idle();
    }

    if(useSoftDevice)
      sd_app_evt_wait(); // let OS threads have time to run
  }

}

/**
 * @brief Hold system power (true) or not (false)
 */
void system_power(bool state)
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
  InitButton(&buttonPWR, BUTTON_PWR__PIN, NRF_GPIO_PIN_NOPULL,
      BUTTON_ACTIVE_HIGH);
  InitButton(&buttonM, BUTTON_M__PIN, NRF_GPIO_PIN_PULLUP, BUTTON_ACTIVE_LOW);
  InitButton(&buttonUP, BUTTON_UP__PIN, NRF_GPIO_PIN_PULLUP, BUTTON_ACTIVE_LOW);
  InitButton(&buttonDWN, BUTTON_DOWN__PIN, NRF_GPIO_PIN_PULLUP,
      BUTTON_ACTIVE_LOW);
}

#if 0
static void button_poll_timer_timeout(void *p_context)
{
    UNUSED_PARAMETER(p_context);

    PollButton(&buttonPWR);
    PollButton(&buttonM);
    PollButton(&buttonUP);
    PollButton(&buttonDWN);
}
#endif


static void gui_timer_timeout(void *p_context)
{
  UNUSED_PARAMETER(p_context);

  gui_ticks++;

  if(gui_ticks % (1000 / MSEC_PER_TICK) == 0)
    ui32_seconds_since_startup++;
  
  if(gui_ticks % (100 / MSEC_PER_TICK) == 0) // every 100ms
    realtime_processing();
}


/// msecs since boot (note: will roll over every 50 days)
uint32_t get_msecs() {
  return gui_ticks * MSEC_PER_TICK;
}

uint32_t get_seconds() {
  return ui32_seconds_since_startup;
}

static void init_app_timers(void)
{
  // FIXME - not sure why I needed to do this manually: https://devzone.nordicsemi.com/f/nordic-q-a/31982/can-t-make-app_timer-work
  if (!NRF_CLOCK->EVENTS_LFCLKSTARTED)
  {
    NRF_CLOCK->TASKS_LFCLKSTART = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
      ;
  }

  // Start APP_TIMER to generate timeouts.
  APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);

#if 0
  // Create&Start button_poll_timer
  APP_ERROR_CHECK(app_timer_create(&button_poll_timer_id, APP_TIMER_MODE_REPEATED, button_poll_timer_timeout));
  APP_ERROR_CHECK(app_timer_start(button_poll_timer_id, BUTTON_POLL_INTERVAL, NULL));
#endif

  // Create&Start timers.
  APP_ERROR_CHECK(
      app_timer_create(&gui_timer_id, APP_TIMER_MODE_REPEATED,
          gui_timer_timeout));
  APP_ERROR_CHECK(app_timer_start(gui_timer_id, GUI_INTERVAL, NULL));
}


