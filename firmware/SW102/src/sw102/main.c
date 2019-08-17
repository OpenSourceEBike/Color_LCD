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

#define MIN_VOLTAGE_10X 140 // If our measured bat voltage (using ADC in the display) is lower than this, we assume we are running on a developers desk

/* Variable definition */

/* �GUI */
UG_GUI gui;

/* Buttons */
Button buttonM, buttonDWN, buttonUP, buttonPWR;

bool has_seen_motor; // true once we've received a packet from a real motor
bool is_sim_motor; // true if we are simulating a motor (and therefore not talking on serial at all)


/* App Timer */
APP_TIMER_DEF(button_poll_timer_id); /* Button timer. */
#define BUTTON_POLL_INTERVAL APP_TIMER_TICKS(10/*ms*/, APP_TIMER_PRESCALER)

// APP_TIMER_DEF(seconds_timer_id); /* Second counting timer. */
// #define SECONDS_INTERVAL APP_TIMER_TICKS(1000/*ms*/, APP_TIMER_PRESCALER)

#define MSEC_PER_TICK 20

APP_TIMER_DEF(gui_timer_id); /* GUI updates counting timer. */
#define GUI_INTERVAL APP_TIMER_TICKS(MSEC_PER_TICK, APP_TIMER_PRESCALER)
volatile uint32_t gui_ticks;


Field bootHeading = FIELD_DRAWTEXT(.msg = "OpenSource EBike");
Field bootVersion = FIELD_DRAWTEXT(.msg = VERSION_STRING);
Field bootStatus = FIELD_DRAWTEXT(.msg = "Booting...");



Screen bootScreen = {
    .fields = {
    {
        .x = 0, .y = 0,
        .field = &bootHeading,
        .font = &FONT_5X12,
    },
    {
        .x = 0, .y = 32,
        .field = &bootVersion,
        .font = &FONT_5X12,
    },
    {
        .x = 0, .y = 80,
        .field = &bootStatus,
        .font = &FONT_5X12,
    },
    {
        .field = NULL
    }
    }};

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

static void automatic_power_off_management(void)
{
  static uint32_t ui16_lcd_power_off_time_counter = 0;

  if(l3_vars.ui8_lcd_power_off_time_minutes != 0)
  {
    // see if we should reset the automatic power off minutes counter
    if ((l3_vars.ui16_wheel_speed_x10 > 0) ||   // wheel speed > 0
        (l3_vars.ui8_battery_current_x5 > 0) || // battery current > 0
        (l3_vars.ui8_braking) ||                // braking
        buttons_get_events())                                 // any button active
    {
      ui16_lcd_power_off_time_counter = 0;
    }
    else
    {
      // increment the automatic power off ticks counter
      ui16_lcd_power_off_time_counter++;

      // check if we should power off the LCD
      if(ui16_lcd_power_off_time_counter >= (l3_vars.ui8_lcd_power_off_time_minutes * 10 * 60)) // have we passed our timeout?
      {
        lcd_power_off(1);
      }
    }
  }
  else
  {
    ui16_lcd_power_off_time_counter = 0;
  }
}


// Screens in a loop, shown when the user short presses the power button
static Screen *screens[] = {
    &mainScreen,
    &infoScreen,
    &configScreen,
    NULL
};

static int nextScreen = 0;

void showNextScreen() {
  Screen *next = screens[nextScreen++];

  if(!next) {
    nextScreen = 0;
    next = screens[nextScreen++];
  }

  screenShow(next);
}


static bool appwide_onpress(buttons_events_t events)
{
  if (events & ONOFF_LONG_CLICK)
  {
    lcd_power_off(1);
    return true;
  }

  if(events & ONOFF_CLICK) {
    showNextScreen();
    return true;
  }

  return false;
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



/// Called every 20ms to check for button events and dispatch to our handlers
static void handle_buttons() {
  if (buttons_events)
  {
    bool handled = false;

    if (!handled)
      handled |= screenOnPress(buttons_events);

    // Note: this must be after the screen/menu handlers have had their shot
    if (!handled)
      handled |= appwide_onpress(buttons_events);

    if (handled)
      buttons_clear_all_events();
  }

  buttons_clock(); // Note: this is done _after_ button events is checked to provide a 20ms debounce
}

/**
 * @brief Application main entry.
 */
int main(void)
{
  gpio_init();
  lcd_init();
  uart_init();
  battery_voltage_init();

  init_app_timers(); // Must be before ble_init! because it sets app timer prescaler

  // kevinh FIXME - turn off ble for now because somtimes it calls app_error_fault_handler(1...) from nrf51822_sw102_ble_advdata
  ble_init();

  /* eeprom_init AFTER ble_init! */
  eeprom_init();
  // FIXME
  // eeprom_read_configuration(get_configuration_variables());
  system_power(true);




  /*   UG_ConsoleSetArea(0, 0, 63, 127);
  UG_ConsoleSetForecolor(C_WHITE);

   UG_FontSelect(&MY_FONT_8X12);
   static const char degC[] = { 31, 'C', 0 };
   UG_ConsolePutString(degC);
   */


  screenShow(&bootScreen);

  // After we show the bootscreen...
  // If a button is currently pressed (likely unless developing), wait for the release (so future click events are not confused
  while(buttons_get_onoff_state() || buttons_get_m_state() || buttons_get_up_state() || buttons_get_down_state())
    ;

  // Enter main loop.

  uint32_t lasttick = gui_ticks;
  uint32_t start_time = get_seconds();
  uint32_t tickshandled = 0; // we might miss ticks if running behind, so we use our own local count to figure out if we need to run our 100ms services
  uint32_t ticksmissed = 0;
  while (1)
  {
    uint32_t tick = gui_ticks;
    if (tick != lasttick)
    {
      if(tick != lasttick + 1) {
        ticksmissed += (tick - lasttick - 1); // Error!  We fell behind and missed some ticks (probably due to screen draw taking more than 20 msec)

        // if(is_sim_motor) app_error_fault_handler(FAULT_MISSEDTICK, 0, ticksmissed);
      }

      lasttick = tick;

      if(tickshandled++ % (100 / MSEC_PER_TICK) == 0) { // every 100ms

        if(stack_overflow_debug() < 128) // we are close to running out of stack
          APP_ERROR_HANDLER(FAULT_STACKOVERFLOW);
      }

      screen_clock();

      handle_buttons();
      automatic_power_off_management(); // Note: this was moved from layer_2() because it does eeprom operations which should not be used from ISR

      if(getCurrentScreen() == &bootScreen) { // FIXME move this into an onIdle callback on the screen
        uint16_t bvolt = battery_voltage_10x_get();

        is_sim_motor = (bvolt < MIN_VOLTAGE_10X);

        if(is_sim_motor)
          fieldPrintf(&bootStatus, "SIMULATING motor!");
        else if(has_seen_motor)
          fieldPrintf(&bootStatus, "Found motor");
        else
          fieldPrintf(&bootStatus, "No motor? (%u.%uV)", bvolt / 10, bvolt % 10);

        // Stop showing the boot screen after a few seconds (once we've found a motor)
        if(get_seconds() - start_time >= 5 && (has_seen_motor || is_sim_motor))
          showNextScreen();
      }
    }

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


static uint32_t seconds = 0;

static void gui_timer_timeout(void *p_context)
{
  UNUSED_PARAMETER(p_context);

  gui_ticks++;

  if(gui_ticks % (100 / MSEC_PER_TICK) == 0) // every 100ms
    layer_2();

  if(gui_ticks % (1000 / MSEC_PER_TICK) == 0)
    seconds++;
}


/// msecs since boot (note: will roll over every 50 days)
uint32_t get_msecs() {
  return gui_ticks * MSEC_PER_TICK;
}

uint32_t get_seconds() {
  return seconds;
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


