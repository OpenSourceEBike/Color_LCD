/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */
#include "ble_uart.h"
#include "app_timer.h"
#include "main.h"
#include "button.h"
#include "lcd.h"
#include "ugui.h"
#include "fonts.h"
#include "uart.h"

#include "utils.h"

/* Variable definition */
UG_GUI gui;

/* Buttons */
Button buttonM, buttonDWN, buttonUP, buttonPWR;

/* App Timer */
APP_TIMER_DEF(button_poll_timer_id); /* Button timer. */
#define BUTTON_POLL_INTERVAL APP_TIMER_TICKS(10/*ms*/, APP_TIMER_PRESCALER)
APP_TIMER_DEF(seconds_timer_id); /* Second counting timer. */
#define SECONDS_INTERVAL APP_TIMER_TICKS(1000/*ms*/, APP_TIMER_PRESCALER)
volatile uint32_t seconds_since_startup, seconds_since_reset;

/* Function prototype */
static void system_power(bool state);
static void gpio_init(void);
static void init_app_timers(void);
static void button_poll_timer_timeout(void * p_context);
static void seconds_timer_timeout(void * p_context);

bool decode_rx_stream(uint8_t* p_rx_buffer, struct_motor_controller_data* p_motor_controller_data, struct_configuration_variables* p_configuration_variables);


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

    /* New RX packet to decode? */
    uint8_t* p_rx_buffer = uart_get_rx_buffer_rdy();
    if (p_rx_buffer != NULL)
    {
      struct_motor_controller_data motor_controller_data;
      struct_configuration_variables configuration_variables;
      decode_rx_stream(p_rx_buffer, &motor_controller_data, &configuration_variables);
    }
  }
}

/**
 * @brief Try to deserialize RX stream. Returns false on error.
 */
bool decode_rx_stream(uint8_t* p_rx_buffer, struct_motor_controller_data* p_motor_controller_data, struct_configuration_variables* p_configuration_variables)
{
  static uint32_t ui32_wss_tick_temp;

  /* Check CRC */
  uint16_t stream_crc = ((uint16_t) p_rx_buffer[25] << 8) | p_rx_buffer[24];
  uint16_t crc = 0xffff;
  for (uint8_t i = 0; i < 24; i++)
    crc16(p_rx_buffer[i], &crc);

  if (stream_crc != crc)
    return false;

  /* Decode */
#define VARIABLE_ID_MAX_NUMBER 10
  if ((p_rx_buffer[1]) == p_motor_controller_data->master_comm_package_id) // last package data ID was receipt, so send the next one
  {
    p_motor_controller_data->master_comm_package_id = (p_motor_controller_data->master_comm_package_id + 1) % VARIABLE_ID_MAX_NUMBER;
  }
  p_motor_controller_data->slave_comm_package_id = p_rx_buffer[2];
  p_motor_controller_data->adc_battery_voltage = p_rx_buffer[3];
  p_motor_controller_data->adc_battery_voltage |= ((uint16_t) (p_rx_buffer[4] & 0x30)) << 4;
  p_motor_controller_data->battery_current_x5 = p_rx_buffer[5];
  p_motor_controller_data->wheel_speed_x10 = (((uint16_t) p_rx_buffer[7]) << 8) + ((uint16_t) p_rx_buffer[6]);
  p_motor_controller_data->motor_controller_state_2 = p_rx_buffer[8];
  p_motor_controller_data->braking = p_motor_controller_data->motor_controller_state_2 & 1;

  if (p_configuration_variables->temperature_limit_feature_enabled == 1)
  {
    p_motor_controller_data->adc_throttle = p_rx_buffer[9];
    p_motor_controller_data->motor_temperature = p_rx_buffer[10];
  }
  else
  {
    p_motor_controller_data->adc_throttle = p_rx_buffer[9];
    p_motor_controller_data->throttle = p_rx_buffer[10];
  }

  p_motor_controller_data->adc_pedal_torque_sensor = p_rx_buffer[11];
  p_motor_controller_data->pedal_torque_sensor = p_rx_buffer[12];
  p_motor_controller_data->pedal_cadence = p_rx_buffer[13];
  p_motor_controller_data->pedal_human_power = p_rx_buffer[14];
  p_motor_controller_data->duty_cycle = p_rx_buffer[15];
  p_motor_controller_data->motor_speed_erps = (((uint16_t) p_rx_buffer[17]) << 8) + ((uint16_t) p_rx_buffer[16]);
  p_motor_controller_data->foc_angle = p_rx_buffer[18];

  switch (p_motor_controller_data->slave_comm_package_id)
  {
  case 0:
    // error states
    p_motor_controller_data->error_states = p_rx_buffer[19];
    break;

  case 1:
    // temperature actual limiting value
    p_motor_controller_data->temperature_current_limiting_value = p_rx_buffer[19];
    break;

  case 2:
    // wheel_speed_sensor_tick_counter
    ui32_wss_tick_temp = ((uint32_t) p_rx_buffer[19]);
    break;

  case 3:
    // wheel_speed_sensor_tick_counter
    ui32_wss_tick_temp |= (((uint32_t) p_rx_buffer[19]) << 8);
    break;

  case 4:
    // wheel_speed_sensor_tick_counter
    ui32_wss_tick_temp |= (((uint32_t) p_rx_buffer[19]) << 16);
    p_motor_controller_data->wheel_speed_sensor_tick_counter = ui32_wss_tick_temp;
    break;
  }

  p_motor_controller_data->pedal_torque_x10 = (((uint16_t) p_rx_buffer[21]) << 8) + ((uint16_t) p_rx_buffer[20]);
  p_motor_controller_data->pedal_power_x10 = (((uint16_t) p_rx_buffer[23]) << 8) + ((uint16_t) p_rx_buffer[22]);

  return true;
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

static void init_app_timers(void)
{
  // Start APP_TIMER to generate timeouts.
  APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);

  // Create&Start button_poll_timer
  app_timer_create(&button_poll_timer_id, APP_TIMER_MODE_REPEATED, button_poll_timer_timeout);
  app_timer_start(button_poll_timer_id, BUTTON_POLL_INTERVAL, NULL);

  // Create&Start timers.
  app_timer_create(&seconds_timer_id, APP_TIMER_MODE_REPEATED, seconds_timer_timeout);
  app_timer_start(seconds_timer_id, SECONDS_INTERVAL, NULL);
}



/* Event handler */

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
