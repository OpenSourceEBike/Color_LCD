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
#include "nrf_delay.h"
#include "nrf_soc.h"
#include "adc.h"

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

APP_TIMER_DEF(seconds_timer_id); /* Second counting timer. */
#define SECONDS_INTERVAL APP_TIMER_TICKS(1000/*ms*/, APP_TIMER_PRESCALER)
volatile uint32_t seconds_since_startup, seconds_since_reset;

APP_TIMER_DEF(gui_timer_id); /* GUI updates counting timer. */
#define GUI_INTERVAL APP_TIMER_TICKS(20/*ms*/, APP_TIMER_PRESCALER)
volatile uint32_t gui_ticks;

Field faultHeading = FIELD_DRAWTEXT(&MY_FONT_8X12, .msg = "FAULT");
Field faultCode = FIELD_DRAWTEXT(&FONT_5X12);
Field addrHeading = FIELD_DRAWTEXT(&FONT_5X12, .msg = "PC");
Field addrCode = FIELD_DRAWTEXT(&FONT_5X12);
Field infoHeading = FIELD_DRAWTEXT(&FONT_5X12, .msg = "Info");
Field infoCode = FIELD_DRAWTEXT(&FONT_5X12);

Screen faultScreen = {
    .fields = {
    { .height = -1, .color = ColorInvert, .field = &faultHeading },
    { .y = FONT12_Y, .height = -1, .color = ColorNormal, .field = &faultCode },
    { .y = 2 * FONT12_Y, .height = -1, .color = ColorNormal,
    .field = &addrHeading },
    { .y = 3 * FONT12_Y, .height = -1, .color = ColorNormal, .field = &addrCode },
    { .y = 4 * FONT12_Y,
    .width = 0, .height = -1, .color = ColorNormal, .field = &infoHeading },
    { .y = 5 * FONT12_Y, .height = -1, .color = ColorNormal, .field = &infoCode },
    { .field = NULL }
    } };


Field bootHeading = FIELD_DRAWTEXT(&FONT_5X12, .msg = "OpenSource EBike");
Field bootVersion = FIELD_DRAWTEXT(&FONT_5X12, .msg = VERSION_STRING);
Field bootStatus = FIELD_DRAWTEXT(&FONT_5X12, .msg = "No motor?");



Screen bootScreen = {
    .fields = {
    {
        .x = 0, .y = 0,
        .field = &bootHeading
    },
    {
        .x = 0, .y = 32,
        .field = &bootVersion
    },
    {
        .x = 0, .y = 80,
        .field = &bootStatus
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
//  if (updateDistanceOdo)
//  {
  l3_vars.ui32_wh_x10_offset = l3_vars.ui32_wh_x10;
//    l3_vars.ui32_odometer_x10 += ((uint32_t) l3_vars.ui16_odometer_distance_x10);
//  }

// save the variables on EEPROM
  eeprom_write_variables();

  // put screen all black and disable backlight
  UG_FillScreen(0);
  lcd_refresh();
  // lcd_set_backlight_intensity(0);

  // FIXME: wait for flash write to complete before powering down
  // now disable the power to all the system
  system_power(0);

  // block here till we die
  while (1)
    ;
}

// Screens in a loop, shown when the user short presses the power button
static Screen *screens[] = {
    &mainScreen,
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
   */

  /*
   UG_FontSelect(&MY_FONT_8X12);
   static const char degC[] = { 31, 'C', 0 };
   UG_ConsolePutString(degC);
   */


  screenShow(&bootScreen);

  // After we show the bootscreen...
  // If a button is currently pressed (likely unless developing), wait for the release (so future click events are not confused
  while(buttons_get_onoff_state() || buttons_get_m_state() || buttons_get_up_state() || buttons_get_down_state())
    ;

  // APP_ERROR_HANDLER(5);

  // Enter main loop.

  uint32_t lasttick = 0;
  while (1)
  {
    uint32_t tick = gui_ticks;
    if (tick != lasttick)
    {
      lasttick = tick;
      screen_clock();

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


    if(getCurrentScreen() == &bootScreen) {
      uint16_t bvolt = battery_voltage_10x_get();

      is_sim_motor = (bvolt < MIN_VOLTAGE_10X);

      if(is_sim_motor)
        fieldPrintf(&bootStatus, "SIMULATING motor!");
      else if(has_seen_motor)
        fieldPrintf(&bootStatus, "Found motor");
      else
        fieldPrintf(&bootStatus, "No motor? (%u.%uV)", bvolt / 10, bvolt % 10);

      // Stop showing the boot screen after a few seconds
      if(seconds_since_startup >= 3)
        showNextScreen();
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

static void seconds_timer_timeout(void *p_context)
{
  UNUSED_PARAMETER(p_context);

  seconds_since_startup++;
  seconds_since_reset++;
}

static void gui_timer_timeout(void *p_context)
{
  UNUSED_PARAMETER(p_context);

  gui_ticks++;
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
      app_timer_create(&seconds_timer_id, APP_TIMER_MODE_REPEATED,
          seconds_timer_timeout));
  APP_ERROR_CHECK(app_timer_start(seconds_timer_id, SECONDS_INTERVAL, NULL));
  APP_ERROR_CHECK(
      app_timer_create(&gui_timer_id, APP_TIMER_MODE_REPEATED,
          gui_timer_timeout));
  APP_ERROR_CHECK(app_timer_start(gui_timer_id, GUI_INTERVAL, NULL));
}

static inline void debugger_break(void)
{
  __asm volatile(
      "bkpt #0x01\n\t"
      "mov pc, lr\n\t"
  );
}

// Standard app error codes
#define FAULT_SOFTDEVICE 1
#define FAULT_GCC_ASSERT 10

// handle standard gcc assert failures
void __attribute__((noreturn)) __assert_func(const char *file, int line,
    const char *func, const char *failedexpr)
{
  error_info_t errinfo = { .line_num = (uint16_t) line, .p_file_name = (uint8_t const *) file, .err_code = FAULT_GCC_ASSERT };

  app_error_fault_handler(FAULT_GCC_ASSERT, 0, (uint32_t) &errinfo);
  abort();
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
  app_error_handler(0xdeadbeef, line_num, p_file_name);
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
  /*
   UG_FontSelect(&MY_FONT_8X12);
   char buf[32];
   sprintf(buf, "ERR 0x%lx\n", id);
   UG_ConsolePutString(buf);

   UG_ConsolePutString("PC\n");
   UG_FontSelect(&FONT_5X12);
   sprintf(buf, "0x%lx\n", pc);
   UG_ConsolePutString(buf);
   UG_FontSelect(&MY_FONT_8X12);
   UG_ConsolePutString("INFO\n");
   UG_FontSelect(&FONT_5X12);
   sprintf(buf, "0x%lx\n", info);
   UG_ConsolePutString(buf);
   lcd_refresh();
   */

  fieldPrintf(&faultCode, "0x%lx", id);
  fieldPrintf(&addrCode, "0x%06lx", pc);

  error_info_t *einfo = (error_info_t*) info;

  switch (id)
  {
  case NRF_FAULT_ID_SDK_ERROR:
    // app errors include filename and line
    fieldPrintf(&infoCode, "%s:%d (%d)",
        einfo->p_file_name ? (const char*) einfo->p_file_name : "nofile",
        einfo->line_num, einfo->err_code);
    break;
  case FAULT_GCC_ASSERT:
    fieldPrintf(&infoCode, "%s:%d",
        einfo->p_file_name ? (const char*) einfo->p_file_name : "",
        einfo->line_num);
    break;
  case FAULT_SOFTDEVICE:
    fieldPrintf(&infoCode, "softdevice");
    break;
  default:
    fieldPrintf(&infoCode, "%08lx", info);
    break;
  }

  panicScreenShow(&faultScreen);

  if(id == FAULT_SOFTDEVICE)
    return; // kevinh, see if we can silently continue - softdevice might be messed up but at least we can continue debugging?

  debugger_break(); // FIXME, only do if debugging, instead show the end user error screen

  // FIXME - instead we should wait a few seconds and then reboot
  while (1) {
    nrf_delay_ms(1000);
  }
}

