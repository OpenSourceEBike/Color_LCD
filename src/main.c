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
#include "eeprom.h"
#include "stdlib.h"

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

/* UART RX/TX Test implementation */
struct_motor_controller_data motor_controller_data;
struct_configuration_variables configuration_variables;
bool decode_rx_stream(const uint8_t* p_rx_buffer, struct_motor_controller_data* p_motor_controller_data, struct_configuration_variables* p_configuration_variables);
void prepare_tx_stream(uint8_t* tx_buffer, struct_motor_controller_data* p_motor_controller_data, struct_configuration_variables* p_configuration_variables);
void init_configuration(struct_configuration_variables* p_configuration_variables);

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


  init_configuration(&configuration_variables);


  // Enter main loop.
  while (1)
  {
    static char char_buf[32];

    if (ButtonClicked(&buttonUP))
    {
      if (configuration_variables.assist_level < configuration_variables.number_of_assist_levels)
        configuration_variables.assist_level++;

      itoa(configuration_variables.assist_level, char_buf, 10);
      UG_ConsolePutString(char_buf);
    }
    if (ButtonClicked(&buttonDWN))
    {
      if (configuration_variables.assist_level > 0)
              configuration_variables.assist_level--;

      itoa(configuration_variables.assist_level, char_buf, 10);
      UG_ConsolePutString(char_buf);
    }

    /* New RX packet to decode? */
    const uint8_t* p_rx_buffer = uart_get_rx_buffer_rdy();
    if (p_rx_buffer != NULL)
    {
      decode_rx_stream(p_rx_buffer, &motor_controller_data, &configuration_variables);
      prepare_tx_stream(uart_get_tx_buffer(), &motor_controller_data, &configuration_variables);
      uart_send_tx_buffer();
    }


  }
}



/**
 * @brief Deserialize RX stream. Returns false on error
 */
bool decode_rx_stream(const uint8_t* p_rx_buffer, struct_motor_controller_data* p_motor_controller_data, struct_configuration_variables* p_configuration_variables)
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
 * @brief Serialize structs & write to tx_buffer
 */
void prepare_tx_stream(uint8_t* tx_buffer, struct_motor_controller_data* p_motor_controller_data, struct_configuration_variables* p_configuration_variables)
{
  // start up byte
  tx_buffer[0] = 0x59;
  tx_buffer[1] = p_motor_controller_data->master_comm_package_id;
  tx_buffer[2] = p_motor_controller_data->slave_comm_package_id;

  // assist level
  if (p_configuration_variables->assist_level) // send assist level factor for normal operation
  {
    tx_buffer[3] = p_configuration_variables->assist_level_factor[((p_configuration_variables->assist_level) - 1)];
  }
  else if (p_motor_controller_data->walk_assist_level) // if walk assist function is enabled, send walk assist level factor
  {
    tx_buffer[3] = p_configuration_variables->walk_assist_level_factor[(p_configuration_variables->assist_level)];
  }
  else // send nothing
  {
    tx_buffer[3] = 0;
  }

  // set lights state
  // walk assist level state
  // set offroad state
  tx_buffer[4] = ((p_motor_controller_data->lights & 1)
      | ((p_motor_controller_data->walk_assist_level & 1) << 1)
      | ((p_motor_controller_data->offroad_mode & 1) << 2));

  // battery max current in amps
  tx_buffer[5] = p_configuration_variables->battery_max_current;

  // battery power
  tx_buffer[6] = p_configuration_variables->target_max_battery_power_div25;

  switch (p_motor_controller_data->master_comm_package_id)
  {
  case 0:
    // battery low voltage cut-off
    tx_buffer[7] = (uint8_t) (p_configuration_variables->battery_low_voltage_cut_off_x10 & 0xff);
    tx_buffer[8] = (uint8_t) (p_configuration_variables->battery_low_voltage_cut_off_x10 >> 8);
    break;

  case 1:
    // wheel perimeter
    tx_buffer[7] = (uint8_t) (p_configuration_variables->wheel_perimeter & 0xff);
    tx_buffer[8] = (uint8_t) (p_configuration_variables->wheel_perimeter >> 8);
    break;

  case 2:
    // wheel max speed
    tx_buffer[7] = p_configuration_variables->wheel_max_speed;
    break;

  case 3:
    // set motor type
    // enable/disable motor assistance without pedal rotation
    // enable/disable motor temperature limit function
    tx_buffer[7] = ((p_configuration_variables->motor_type & 3)
            | ((p_configuration_variables->motor_assistance_startup_without_pedal_rotation & 1) << 2)
            | ((p_configuration_variables->temperature_limit_feature_enabled & 3) << 3));

    // motor power boost startup state
    tx_buffer[8] = p_configuration_variables->startup_motor_power_boost_state;
    break;

  case 4:
    // startup motor power boost
    tx_buffer[7] = p_configuration_variables->startup_motor_power_boost_factor[((p_configuration_variables->assist_level) - 1)];
    // startup motor power boost time
    tx_buffer[8] = p_configuration_variables->startup_motor_power_boost_time;
    break;

  case 5:
    // startup motor power boost fade time
    tx_buffer[7] = p_configuration_variables->startup_motor_power_boost_fade_time;
    // boost feature enabled
    tx_buffer[8] = (p_configuration_variables->startup_motor_power_boost_feature_enabled & 1) ? 1 : 0;
    break;

  case 6:
    // motor over temperature min and max values to limit
    tx_buffer[7] = p_configuration_variables->motor_temperature_min_value_to_limit;
    tx_buffer[8] = p_configuration_variables->motor_temperature_max_value_to_limit;
    break;

  case 7:
    // offroad mode configuration
    tx_buffer[7] = ((p_configuration_variables->offroad_feature_enabled & 1)
            | ((p_configuration_variables->offroad_enabled_on_startup & 1) << 1));
    tx_buffer[8] = p_configuration_variables->offroad_speed_limit;
    break;

  case 8:
    // offroad mode power limit configuration
    tx_buffer[7] = p_configuration_variables->offroad_power_limit_enabled & 1;
    tx_buffer[8] = p_configuration_variables->offroad_power_limit_div25;
    break;

  case 9:
    // ramp up, amps per second
    tx_buffer[7] = p_configuration_variables->ramp_up_amps_per_second_x10;
    // cruise target speed
    if (p_configuration_variables->cruise_function_set_target_speed_enabled)
    {
      tx_buffer[8] = p_configuration_variables->cruise_function_target_speed_kph;
    }
    else
    {
      tx_buffer[8] = 0;
    }
    break;
  }

  // prepare crc of the package
  uint16_t crc_tx = 0xffff;
  for (uint8_t i = 0; i < 9; i++)
  {
    crc16(tx_buffer[i], &crc_tx);
  }
  tx_buffer[9] = (crc_tx & 0xff);
  tx_buffer[10] = (crc_tx >> 8) & 0xff;
}

/**
 * @brief Initialize struct_configuration_variables to defaults
 */
void init_configuration(struct_configuration_variables* p_configuration_variables)
{
  p_configuration_variables->assist_level = DEFAULT_VALUE_ASSIST_LEVEL;
  p_configuration_variables->wheel_perimeter = DEFAULT_VALUE_WHEEL_PERIMETER_1 << 8;
  p_configuration_variables->wheel_perimeter += DEFAULT_VALUE_WHEEL_PERIMETER_0;
  p_configuration_variables->wheel_max_speed = DEFAULT_VALUE_WHEEL_MAX_SPEED;
  p_configuration_variables->wheel_max_speed_imperial = DEFAULT_VALUE_MAX_WHEEL_SPEED_IMPERIAL;
  p_configuration_variables->units_type = DEFAULT_VALUE_UNITS_TYPE;
  p_configuration_variables->wh_x10_offset = DEFAULT_VALUE_WH_OFFSET;
  p_configuration_variables->wh_x10_100_percent = DEFAULT_VALUE_HW_X10_100_PERCENT;
  p_configuration_variables->battery_SOC_function_enabled = DEAFULT_VALUE_BATTERY_SOC_FUNCTION_ENABLED;

  p_configuration_variables->odometer_field_state = DEFAULT_VALUE_ODOMETER_FIELD_STATE;
  p_configuration_variables->odometer_sub_field_state_0 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_0;
  p_configuration_variables->odometer_sub_field_state_1 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_1;
  p_configuration_variables->odometer_sub_field_state_2 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_2;
  p_configuration_variables->odometer_sub_field_state_3 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_3;
  p_configuration_variables->odometer_sub_field_state_4 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_4;
  p_configuration_variables->odometer_sub_field_state_5 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_5;
  p_configuration_variables->odometer_sub_field_state_6 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_6;

  p_configuration_variables->time_measurement_field_state = DEFAULT_VALUE_TIME_MEASUREMENT_FIELD_STATE;
  p_configuration_variables->total_second_TTM = DEFAULT_VALUE_TOTAL_SECOND_TTM;
  p_configuration_variables->total_minute_TTM = DEFAULT_VALUE_TOTAL_MINUTE_TTM;
  p_configuration_variables->total_hour_TTM = DEFAULT_VALUE_TOTAL_HOUR_TTM_1 << 8;
  p_configuration_variables->total_hour_TTM += DEFAULT_VALUE_TOTAL_HOUR_TTM_0;

  p_configuration_variables->battery_max_current = DEFAULT_VALUE_BATTERY_MAX_CURRENT;
  p_configuration_variables->target_max_battery_power_div25 = DEFAULT_VALUE_TARGET_MAX_BATTERY_POWER;
  p_configuration_variables->battery_cells_number = DEFAULT_VALUE_BATTERY_CELLS_NUMBER;
  p_configuration_variables->battery_low_voltage_cut_off_x10 = DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10_1 << 8;
  p_configuration_variables->battery_low_voltage_cut_off_x10 += DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10_0;

  p_configuration_variables->motor_type = DEFAULT_VALUE_CONFIG_0 & 3;
  p_configuration_variables->motor_assistance_startup_without_pedal_rotation = (DEFAULT_VALUE_CONFIG_0 & 4) >> 2;
  p_configuration_variables->temperature_limit_feature_enabled = (DEFAULT_VALUE_CONFIG_0 & 24) >> 3;
  p_configuration_variables->temperature_field_state = (DEFAULT_VALUE_CONFIG_0 & 224) >> 5;

  p_configuration_variables->number_of_assist_levels = DEFAULT_VALUE_NUMBER_OF_ASSIST_LEVELS;
  p_configuration_variables->assist_level_factor[0] = DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_1;
  p_configuration_variables->assist_level_factor[1] = DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_2;
  p_configuration_variables->assist_level_factor[2] = DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_3;
  p_configuration_variables->assist_level_factor[3] = DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_4;
  p_configuration_variables->assist_level_factor[4] = DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_5;

  p_configuration_variables->startup_motor_power_boost_feature_enabled = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED;
  p_configuration_variables->startup_motor_power_boost_state = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_STATE;
  p_configuration_variables->startup_motor_power_boost_time = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_TIME;
  p_configuration_variables->startup_motor_power_boost_factor[0] = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1;
  //...
  p_configuration_variables->startup_motor_power_boost_fade_time = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FADE_TIME;

  p_configuration_variables->motor_temperature_min_value_to_limit = DEFAULT_VALUE_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT;
  p_configuration_variables->motor_temperature_max_value_to_limit = DEFAULT_VALUE_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT;

  p_configuration_variables->battery_voltage_reset_wh_counter_x10 = DEFAULT_VALUE_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10_1 << 8;
  p_configuration_variables->battery_voltage_reset_wh_counter_x10 += DEFAULT_VALUE_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10_0;

  p_configuration_variables->lcd_power_off_time_minutes = DEFAULT_VALUE_LCD_POWER_OFF_TIME;
  p_configuration_variables->lcd_backlight_on_brightness = DEFAULT_VALUE_LCD_BACKLIGHT_ON_BRIGHTNESS;
  p_configuration_variables->lcd_backlight_off_brightness = DEFAULT_VALUE_LCD_BACKLIGHT_OFF_BRIGHTNESS;

  p_configuration_variables->battery_pack_resistance_x1000 = DEFAULT_VALUE_BATTERY_PACK_RESISTANCE_1 << 8;
  p_configuration_variables->battery_pack_resistance_x1000 += DEFAULT_VALUE_BATTERY_PACK_RESISTANCE_0;

  p_configuration_variables->offroad_feature_enabled = DEFAULT_VALUE_OFFROAD_FEATURE_ENABLED;
  p_configuration_variables->offroad_enabled_on_startup = DEFAULT_VALUE_OFFROAD_MODE_ENABLED_ON_STARTUP;
  p_configuration_variables->offroad_speed_limit = DEFAULT_VALUE_OFFROAD_SPEED_LIMIT;
  p_configuration_variables->offroad_power_limit_enabled = DEFAULT_VALUE_OFFROAD_POWER_LIMIT_ENABLED;
  p_configuration_variables->offroad_power_limit_div25 = DEFAULT_VALUE_OFFROAD_POWER_LIMIT_DIV25;

  p_configuration_variables->odometer_x10 = DEFAULT_VALUE_ODOMETER_X10;
  p_configuration_variables->trip_x10 = DEFAULT_VALUE_TRIP_X10;

  p_configuration_variables->ramp_up_amps_per_second_x10 = DEFAULT_VALUE_RAMP_UP_AMPS_PER_SECOND_X10;

  p_configuration_variables->walk_assist_function_enabled = DEFAULT_VALUE_WALK_ASSIST_FUNCTION_ENABLED;
  p_configuration_variables->walk_assist_level_factor[0] = DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_0;
  p_configuration_variables->walk_assist_level_factor[1] = DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_1;
  //...

  p_configuration_variables->cruise_function_enabled = DEFAULT_VALUE_CRUISE_FUNCTION_ENABLED;
  p_configuration_variables->cruise_function_set_target_speed_enabled = DEFAULT_VALUE_CRUISE_FUNCTION_SET_TARGET_SPEED_ENABLED;
  p_configuration_variables->cruise_function_target_speed_kph = DEFAULT_VALUE_CRUISE_FUNCTION_TARGET_SPEED_KPH;
  p_configuration_variables->cruise_function_target_speed_mph = DEFAULT_VALUE_CRUISE_FUNCTION_TARGET_SPEED_MPH;
  p_configuration_variables->show_cruise_function_set_target_speed = DEFAULT_VALUE_SHOW_CRUISE_FUNCTION_SET_TARGET_SPEED;

  p_configuration_variables->wheel_speed_field_state = DEFAULT_VALUE_WHEEL_SPEED_FIELD_STATE;

  p_configuration_variables->show_distance_data_odometer_field = DEFAULT_VALUE_SHOW_DISTANCE_DATA_ODOMETER_FIELD;
  p_configuration_variables->show_battery_state_odometer_field = DEFAULT_VALUE_SHOW_BATTERY_STATE_ODOMETER_FIELD;
  p_configuration_variables->show_pedal_data_odometer_field = DEFAULT_VALUE_SHOW_PEDAL_DATA_ODOMETER_FIELD;
  p_configuration_variables->show_time_measurement_odometer_field = DEFAULT_VALUE_SHOW_TIME_MEASUREMENT_ODOMETER_FIELD;
  p_configuration_variables->show_wheel_speed_odometer_field = DEFAULT_VALUE_SHOW_WHEEL_SPEED_ODOMETER_FIELD;
  p_configuration_variables->show_energy_data_odometer_field = DEFAULT_VALUE_SHOW_ENERGY_DATA_ODOMETER_FIELD;
  p_configuration_variables->show_motor_temperature_odometer_field = DEFAULT_VALUE_SHOW_MOTOR_TEMPERATURE_ODOMETER_FIELD;
  p_configuration_variables->show_battery_SOC_odometer_field = DEFAULT_VALUE_SHOW_BATTERY_SOC_ODOMETER_FIELD;

  p_configuration_variables->main_screen_power_menu_enabled = DEFAULT_VALUE_MAIN_SCREEN_POWER_MENU_ENABLED;
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
