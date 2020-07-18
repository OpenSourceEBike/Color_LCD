/*
 * TSDZ2 EBike wireless firmware
 *
 * Copyright (C) Casainho, 2020
 *
 * Released under the GPL License, Version 3
 */

#include <stdio.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf.h"
#include "hardfault.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ant.h"
#include "ant_key_manager.h"
#include "ant_lev.h"
#include "pins.h"
#include "rtc.h"
#include "state.h"
#include "uart.h"
#include "eeprom.h"
#include "lcd.h"
#include "rtc.h"

#define MSEC_PER_TICK 20

APP_TIMER_DEF(gui_timer_id); /* GUI updates counting timer. */
#define GUI_INTERVAL APP_TIMER_TICKS(MSEC_PER_TICK)

volatile uint32_t gui_ticks;

// assume we should until we init_softdevice()
bool useSoftDevice = true;

#define LEV_HW_REVISION 1
#define LEV_MANUFACTURER_ID (UINT16_MAX - 1)
#define LEV_MODEL_NUMBER 1
#define LEV_SW_REVISION_MAJOR 1
#define LEV_SW_REVISION_MINOR 1
#define LEV_SERIAL_NUMBER 1234567
#define LEV_CHANNEL_NUM 0 // ?? seems can be any value from 0 to 8
#define CHAN_ID_TRANS_TYPE 5 // LEV table 4.2
#define CHAN_ID_DEV_NUM 1 // [1 - 65535], LEV table 4.2
#define ANTPLUS_NETWORK_NUM 0
#define ANT_LEV_ANT_OBSERVER_PRIO 1

void ant_lev_evt_handler(ant_lev_profile_t * p_profile, ant_lev_evt_t event);

LEV_SENS_CHANNEL_CONFIG_DEF(m_ant_lev,
                            LEV_CHANNEL_NUM,
                            CHAN_ID_TRANS_TYPE,
                            CHAN_ID_DEV_NUM,
                            ANTPLUS_NETWORK_NUM);
LEV_SENS_PROFILE_CONFIG_DEF(m_ant_lev,
                            ant_lev_evt_handler);

static ant_lev_profile_t m_ant_lev;

NRF_SDH_ANT_OBSERVER(m_ant_observer, ANT_LEV_ANT_OBSERVER_PRIO, ant_lev_sens_evt_handler, &m_ant_lev);

bool m_rt_processing_stop = false;

void ant_lev_evt_handler(ant_lev_profile_t * p_profile, ant_lev_evt_t event)
{
    nrf_pwr_mgmt_feed();

    switch (event)
    {
        case ANT_LEV_PAGE_1_UPDATED:
            break;

        case ANT_LEV_PAGE_2_UPDATED:
            break;

        case ANT_LEV_PAGE_3_UPDATED:
            break;

        case ANT_LEV_PAGE_4_UPDATED:
            break;

        case ANT_LEV_PAGE_5_UPDATED:
            break;

        case ANT_LEV_PAGE_34_UPDATED:
            break;

        case ANT_LEV_PAGE_16_UPDATED:
            if ((p_profile->page_16.travel_mode & 0x38) > 0)
                motor_power_enable(false);
            else
                motor_power_enable(true);

            // update the assist level
            p_profile->common.travel_mode_state = p_profile->page_16.travel_mode;

            break;

        case ANT_LEV_PAGE_80_UPDATED:
            break;

        case ANT_LEV_PAGE_81_UPDATED:
            break;

        case ANT_LEV_PAGE_REQUEST_SUCCESS:
            break;

        case ANT_LEV_PAGE_REQUEST_FAILED:
            break;

        default:
            break;
    }
}

static void ant_setup(void)
{
  // Softdevice setup
  ret_code_t err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  ASSERT(nrf_sdh_is_enabled());

  err_code = nrf_sdh_ant_enable();
  APP_ERROR_CHECK(err_code);

  err_code = ant_plus_key_set(ANTPLUS_NETWORK_NUM);
  APP_ERROR_CHECK(err_code);

  // ANT+ profile setup
  err_code = ant_lev_sens_init(&m_ant_lev,
                                LEV_SENS_CHANNEL_CONFIG(m_ant_lev),
                                LEV_SENS_PROFILE_CONFIG(m_ant_lev));
  APP_ERROR_CHECK(err_code);

  // fill manufacturer's common data page.
  m_ant_lev.page_80 = ANT_COMMON_page80(LEV_HW_REVISION,
                                        LEV_MANUFACTURER_ID,
                                        LEV_MODEL_NUMBER);
  // fill product's common data page.
  m_ant_lev.page_81 = ANT_COMMON_page81(LEV_SW_REVISION_MAJOR,
                                        LEV_SW_REVISION_MINOR,
                                        LEV_SERIAL_NUMBER);

  err_code = ant_lev_sens_open(&m_ant_lev);
  APP_ERROR_CHECK(err_code);
}

void wireless_virtual_rt_processing_stop(void) {
  m_rt_processing_stop = true;
}

void wireless_virtual_rt_processing_start(void) {
  m_rt_processing_stop = false;
}

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void gui_timer_timeout(void *p_context)
{
  UNUSED_PARAMETER(p_context);

  gui_ticks++;

  if(gui_ticks % (1000 / MSEC_PER_TICK) == 0)
    ui32_seconds_since_startup++;
  
  if((gui_ticks % (100 / MSEC_PER_TICK) == 0) && // every 100ms
      m_rt_processing_stop == false)
    ; // rt_processing();
}

/// msecs since boot (note: will roll over every 50 days)
uint32_t get_time_base_counter_1ms() {
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

  // create and start timers
  APP_ERROR_CHECK(app_timer_create(&gui_timer_id, APP_TIMER_MODE_REPEATED, gui_timer_timeout));
  APP_ERROR_CHECK(app_timer_start(gui_timer_id, GUI_INTERVAL, NULL));
}

/// Call every 20ms from the main thread.
void main_idle() {
  // static int counter_time_ms = 0;
  // int time_ms = 0;

  // // no point to processing less than every 100ms, as the data comming from the motor is only updated every 100ms, not less
  // time_ms = get_time_base_counter_1ms();
  // if((time_ms - counter_time_ms) >= 100) // not least than evey 100ms
  // {
  //   counter_time_ms = time_ms;
  //   automatic_power_off_management();
  // }

	// handle_buttons();
	screen_clock(); // This is _after_ handle_buttons so if a button was pressed this tick, we immediately update the GUI
}

int main(void)
{
  log_init();
  pins_init();
  ant_setup();
  uart_init();
  init_app_timers(); // Must be before ble_init! because it sets app timer prescaler
  eeprom_init(); // must be after BLE init

  // Enter main loop.
  uint32_t lasttick = gui_ticks;
  uint32_t tickshandled = 0; // we might miss ticks if running behind, so we use our own local count to figure out if we need to run our 100ms services
  uint32_t ticksmissed = 0;
  while (1)
  {
    uint32_t tick = gui_ticks;
    if (tick != lasttick)
    {
      if (tick != lasttick + 1) {
        ticksmissed += (tick - lasttick - 1); // Error!  We fell behind and missed some ticks (probably due to screen draw taking more than 20 msec)

        // if(is_sim_motor) app_error_fault_handler(FAULT_MISSEDTICK, 0, ticksmissed);
      }

      lasttick = tick;

      if(tickshandled++ % (100 / MSEC_PER_TICK) == 0) { // every 100ms

        // if(stack_overflow_debug() < 128) // we are close to running out of stack
        //   APP_ERROR_HANDLER(FAULT_STACKOVERFLOW);
      }

      main_idle();
    }

    if(useSoftDevice)
      sd_app_evt_wait(); // let OS threads have time to run
  }

}

