/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */

#include "section_vars.h"
#include "eeprom.h"
#include "common.h"
#include "nrf_delay.h"

const struct_configuration_variables default_configuration_variables =
{
    .assist_level = DEFAULT_VALUE_ASSIST_LEVEL,
    .number_of_assist_levels = DEFAULT_VALUE_NUMBER_OF_ASSIST_LEVELS,
    .wheel_perimeter = DEFAULT_VALUE_WHEEL_PERIMETER,
    .wheel_max_speed = DEFAULT_VALUE_WHEEL_MAX_SPEED,
    .wheel_max_speed_imperial = DEFAULT_VALUE_MAX_WHEEL_SPEED_IMPERIAL,
    .units_type = DEFAULT_VALUE_UNITS_TYPE,
    .wh_x10_offset = DEFAULT_VALUE_WH_OFFSET,
    .wh_x10_100_percent = DEFAULT_VALUE_HW_X10_100_PERCENT,
    .battery_SOC_function_enabled = DEAFULT_VALUE_BATTERY_SOC_FUNCTION_ENABLED,
    .odometer_field_state = DEFAULT_VALUE_ODOMETER_FIELD_STATE,
    .time_measurement_field_state = DEFAULT_VALUE_TIME_MEASUREMENT_FIELD_STATE,
    .total_second_TTM = DEFAULT_VALUE_TOTAL_SECOND_TTM,
    .total_minute_TTM =  DEFAULT_VALUE_TOTAL_MINUTE_TTM,
    .total_hour_TTM =  DEFAULT_VALUE_TOTAL_HOUR_TTM,
    .odometer_sub_field_state_0 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_0,
    .odometer_sub_field_state_1 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_1,
    .odometer_sub_field_state_2 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_2,
    .odometer_sub_field_state_3 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_3,
    .odometer_sub_field_state_4 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_4,
    .odometer_sub_field_state_5 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_5,
    .odometer_sub_field_state_6 = DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_6,
    .odometer_show_field_number = DEFAULT_VALUE_ODOMETER_FIELD_STATE,
    .target_max_battery_power_div25 = DEFAULT_VALUE_TARGET_MAX_BATTERY_POWER,
    .battery_cells_number = DEFAULT_VALUE_BATTERY_CELLS_NUMBER,
    .battery_max_current = DEFAULT_VALUE_BATTERY_MAX_CURRENT,
    .battery_low_voltage_cut_off_x10 = DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10,
    .battery_voltage_reset_wh_counter_x10 = DEFAULT_VALUE_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10,
    .battery_pack_resistance_x1000 = DEFAULT_VALUE_BATTERY_PACK_RESISTANCE,
    .motor_type = (DEFAULT_VALUE_CONFIG_0 & 0b00000011),
    .motor_assistance_startup_without_pedal_rotation = ((DEFAULT_VALUE_CONFIG_0 & 0b00000100) >> 2),
    .assist_level_factor = {
        DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_1,
        DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_2,
        DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_3,
        DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_4,
        DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_5,
        DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_6,
        DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_7,
        DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_8,
        DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_9
    },
    .startup_motor_power_boost_feature_enabled = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED,
    .startup_motor_power_boost_state = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_STATE,
    .startup_motor_power_boost_time = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_TIME,
    .startup_motor_power_boost_fade_time = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FADE_TIME,
    .startup_motor_power_boost_factor = {
        DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1,
        DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_2,
        DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_3,
        DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_4,
        DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_5,
        DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_6,
        DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_7,
        DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_8,
        DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_9
    },
    .adc_motor_temperature_10b = 0,
    .temperature_limit_feature_enabled = ((DEFAULT_VALUE_CONFIG_0 & 0b00011000) >> 3),
    .motor_temperature_min_value_to_limit = DEFAULT_VALUE_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT,
    .motor_temperature_max_value_to_limit = DEFAULT_VALUE_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT,
    .temperature_field_state = ((DEFAULT_VALUE_CONFIG_0 & 0b11100000) >> 5),
    .lcd_power_off_time_minutes = DEFAULT_VALUE_LCD_POWER_OFF_TIME,
    .offroad_feature_enabled = DEFAULT_VALUE_OFFROAD_FEATURE_ENABLED,
    .offroad_enabled_on_startup = DEFAULT_VALUE_OFFROAD_MODE_ENABLED_ON_STARTUP,
    .offroad_speed_limit = DEFAULT_VALUE_OFFROAD_SPEED_LIMIT,
    .offroad_power_limit_enabled = DEFAULT_VALUE_OFFROAD_POWER_LIMIT_ENABLED,
    .offroad_power_limit_div25 = DEFAULT_VALUE_OFFROAD_POWER_LIMIT_DIV25,
    .distance_since_power_on_x10 = 0,
    .odometer_x10 = DEFAULT_VALUE_ODOMETER_X10,
    .trip_x10 = DEFAULT_VALUE_TRIP_X10,
    .ramp_up_amps_per_second_x10 = DEFAULT_VALUE_RAMP_UP_AMPS_PER_SECOND_X10,
    .walk_assist_function_enabled = DEFAULT_VALUE_WALK_ASSIST_FUNCTION_ENABLED,
    .walk_assist_level_factor = {
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_0,
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_1,
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_2,
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_3,
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_4,
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_5,
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_6,
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_7,
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_8,
        DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_9
    },
    .cruise_function_enabled = DEFAULT_VALUE_CRUISE_FUNCTION_ENABLED,
    .cruise_function_set_target_speed_enabled = DEFAULT_VALUE_CRUISE_FUNCTION_SET_TARGET_SPEED_ENABLED,
    .cruise_function_target_speed_kph = DEFAULT_VALUE_CRUISE_FUNCTION_TARGET_SPEED_KPH,
    .cruise_function_target_speed_mph = DEFAULT_VALUE_CRUISE_FUNCTION_TARGET_SPEED_MPH,
    .show_cruise_function_set_target_speed = DEFAULT_VALUE_SHOW_CRUISE_FUNCTION_SET_TARGET_SPEED,
    .wheel_speed_field_state = DEFAULT_VALUE_WHEEL_SPEED_FIELD_STATE,
    .show_distance_data_odometer_field = DEFAULT_VALUE_SHOW_DISTANCE_DATA_ODOMETER_FIELD,
    .show_battery_state_odometer_field = DEFAULT_VALUE_SHOW_BATTERY_STATE_ODOMETER_FIELD,
    .show_pedal_data_odometer_field = DEFAULT_VALUE_SHOW_PEDAL_DATA_ODOMETER_FIELD,
    .show_time_measurement_odometer_field = DEFAULT_VALUE_SHOW_TIME_MEASUREMENT_ODOMETER_FIELD,
    .show_wheel_speed_odometer_field = DEFAULT_VALUE_SHOW_WHEEL_SPEED_ODOMETER_FIELD,
    .show_energy_data_odometer_field = DEFAULT_VALUE_SHOW_ENERGY_DATA_ODOMETER_FIELD,
    .show_motor_temperature_odometer_field = DEFAULT_VALUE_SHOW_MOTOR_TEMPERATURE_ODOMETER_FIELD,
    .show_battery_SOC_odometer_field = DEFAULT_VALUE_SHOW_BATTERY_SOC_ODOMETER_FIELD,
    .main_screen_power_menu_enabled = DEFAULT_VALUE_MAIN_SCREEN_POWER_MENU_ENABLED
};

volatile fs_ret_t last_fs_ret;



/* Function prototype */
static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result);



FS_REGISTER_CFG(fs_config_t fs_config) =
{
    .callback  = fs_evt_handler,  // Function for event callbacks.
    .num_pages = 1,               // Number of physical flash pages required (see also FS_PAGE_SIZE_WORDS for actual sizes).
                                  // NRF51: 256 words (32 bit).
    .priority  = 0xF0             // Priority for flash usage.
};

static uint32_t flash_read_word(uint8_t offset)
{
  return fs_config.p_start_addr[offset];
}

static fs_ret_t flash_write_words(uint8_t offset, uint32_t* value, uint16_t length_words)
{
  fs_ret_t ret;
  uint32_t cnt = 0;

  do
  {
    last_fs_ret = 0xFF;
    ret = fs_store(&fs_config, &fs_config.p_start_addr[offset], value, length_words, NULL);
    /* Wait some time if fstore queue is full and retry */
    if (ret == FS_ERR_QUEUE_FULL)
      nrf_delay_us(100);
  }
  while (ret == FS_ERR_QUEUE_FULL && cnt++ < 10);

  return ret;
}

/**
 * @brief Init eeprom emulation system
 */
void eeprom_init(void)
{
  UNUSED_VARIABLE(fs_config);  // To avoid 'Unused declaration' warning. Compiler doesn't see the section/linker magic in use.
  static uint32_t ee_key;

  fs_init();

  ee_key = flash_read_word(ADDRESS_KEY);
  /* Init eeprom to default if KEY is not valid */
  if (ee_key != KEY)
  {
    /* Write default config to eeprom */
    eeprom_write_configuration(&default_configuration_variables);
    uint32_t cnt = 0;
    while (last_fs_ret == 0xFF && cnt++ < 10)
      nrf_delay_us(100);

    /* Write new KEY */
    ee_key = KEY;
    flash_write_words(ADDRESS_KEY, &ee_key, 1);
    cnt = 0;
    while (last_fs_ret == 0xFF && cnt++ < 10)
      nrf_delay_us(100);
  }
}

/**
 * @brief Read struct_configuration_variables to memory
 */
void eeprom_read_configuration(struct_configuration_variables *p_configuration_variables)
{
  p_configuration_variables->assist_level = flash_read_word(ADDRESS_ASSIST_LEVEL);
  p_configuration_variables->wheel_perimeter = flash_read_word(ADDRESS_WHEEL_PERIMETER);
  p_configuration_variables->wheel_max_speed = flash_read_word(ADDRESS_MAX_WHEEL_SPEED);
  p_configuration_variables->units_type = flash_read_word(ADDRESS_UNITS_TYPE);
  p_configuration_variables->wh_x10_offset = flash_read_word(ADDRESS_HW_X10_OFFSET);
  p_configuration_variables->wh_x10_100_percent = flash_read_word(ADDRESS_HW_X10_100_PERCENT_OFFSET);
  p_configuration_variables->battery_SOC_function_enabled = flash_read_word(ADDRESS_BATTERY_SOC_FUNCTION_ENABLED);
  p_configuration_variables->odometer_field_state = flash_read_word(ADDRESS_ODOMETER_FIELD_STATE);
  p_configuration_variables->battery_max_current = flash_read_word(ADDRESS_BATTERY_MAX_CURRENT);
  p_configuration_variables->target_max_battery_power_div25 = flash_read_word(ADDRESS_TARGET_MAX_BATTERY_POWER);
  p_configuration_variables->battery_cells_number = flash_read_word(ADDRESS_BATTERY_CELLS_NUMBER);
  p_configuration_variables->battery_low_voltage_cut_off_x10 = flash_read_word(ADDRESS_BATTERY_LOW_VOLTAGE_CUT_OFF_X10);
  uint8_t config0 = flash_read_word(ADDRESS_CONFIG_0);
  p_configuration_variables->motor_type = config0 & 0b00000011;
  p_configuration_variables->motor_assistance_startup_without_pedal_rotation = (config0 & 0b00000100) >> 2;
  p_configuration_variables->temperature_limit_feature_enabled = (config0 & 0b00011000) >> 3;
  p_configuration_variables->temperature_field_state = (config0 & 0b11100000) >> 5;
  for (uint8_t i = 0; i < 9; i++)
    p_configuration_variables->assist_level_factor[i] = flash_read_word(ADDRESS_ASSIST_LEVEL_FACTOR_1 + i);

  p_configuration_variables->number_of_assist_levels = flash_read_word(ADDRESS_NUMBER_OF_ASSIST_LEVELS);
  p_configuration_variables->startup_motor_power_boost_feature_enabled = flash_read_word(ADDRESS_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED);
  p_configuration_variables->startup_motor_power_boost_state = flash_read_word(ADDRESS_STARTUP_MOTOR_POWER_BOOST_STATE);
  for (uint8_t i = 0; i < 9; i++)
    p_configuration_variables->startup_motor_power_boost_factor[i] = flash_read_word(ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1 + i);

  p_configuration_variables->startup_motor_power_boost_time = flash_read_word(ADDRESS_STARTUP_MOTOR_POWER_BOOST_TIME);
  p_configuration_variables->startup_motor_power_boost_fade_time = flash_read_word(ADDRESS_STARTUP_MOTOR_POWER_BOOST_FADE_TIME);
  p_configuration_variables->motor_temperature_min_value_to_limit = flash_read_word(ADDRESS_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT);
  p_configuration_variables->motor_temperature_max_value_to_limit = flash_read_word(ADDRESS_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT);
  p_configuration_variables->battery_voltage_reset_wh_counter_x10 = flash_read_word(ADDRESS_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10);
  p_configuration_variables->lcd_power_off_time_minutes = flash_read_word(ADDRESS_LCD_POWER_OFF_TIME);
  p_configuration_variables->battery_pack_resistance_x1000 = flash_read_word(ADDRESS_BATTERY_PACK_RESISTANCE);
  p_configuration_variables->offroad_feature_enabled = flash_read_word(ADDRESS_OFFROAD_FEATURE_ENABLED);
  p_configuration_variables->offroad_enabled_on_startup = flash_read_word(ADDRESS_OFFROAD_MODE_ENABLED_ON_STARTUP);
  p_configuration_variables->offroad_speed_limit = flash_read_word(ADDRESS_OFFROAD_SPEED_LIMIT);
  p_configuration_variables->offroad_power_limit_enabled = flash_read_word(ADDRESS_OFFROAD_POWER_LIMIT_ENABLED);
  p_configuration_variables->offroad_power_limit_div25 = flash_read_word(ADDRESS_OFFROAD_POWER_LIMIT_DIV25);
  p_configuration_variables->odometer_x10 = flash_read_word(ADDRESS_ODOMETER_X10);
  p_configuration_variables->trip_x10 = flash_read_word(ADDRESS_TRIP_X10);
  p_configuration_variables->odometer_sub_field_state_0 = flash_read_word(ADDRESS_ODOMETER_SUB_FIELD_STATE_0);
  p_configuration_variables->odometer_sub_field_state_1 = flash_read_word(ADDRESS_ODOMETER_SUB_FIELD_STATE_1);
  p_configuration_variables->odometer_sub_field_state_2 = flash_read_word(ADDRESS_ODOMETER_SUB_FIELD_STATE_2);
  p_configuration_variables->odometer_sub_field_state_3 = flash_read_word(ADDRESS_ODOMETER_SUB_FIELD_STATE_3);
  p_configuration_variables->odometer_sub_field_state_4 = flash_read_word(ADDRESS_ODOMETER_SUB_FIELD_STATE_4);
  p_configuration_variables->odometer_sub_field_state_5 = flash_read_word(ADDRESS_ODOMETER_SUB_FIELD_STATE_5);
  p_configuration_variables->odometer_sub_field_state_6 = flash_read_word(ADDRESS_ODOMETER_SUB_FIELD_STATE_6);
  p_configuration_variables->wheel_max_speed_imperial = flash_read_word(ADDRESS_MAX_WHEEL_SPEED_IMPERIAL);
  p_configuration_variables->time_measurement_field_state = flash_read_word(ADDRESS_TIME_MEASUREMENT_FIELD_STATE);
  p_configuration_variables->total_second_TTM = flash_read_word(ADDRESS_TOTAL_SECOND_TTM);
  p_configuration_variables->total_minute_TTM = flash_read_word(ADDRESS_TOTAL_MINUTE_TTM);
  p_configuration_variables->total_hour_TTM = flash_read_word(ADDRESS_TOTAL_HOUR_TTM);
  p_configuration_variables->ramp_up_amps_per_second_x10 = flash_read_word(ADDRESS_RAMP_UP_AMPS_PER_SECOND_X10);
  p_configuration_variables->walk_assist_function_enabled = flash_read_word(ADDRESS_WALK_ASSIST_FUNCTION_ENABLED);
  for (uint8_t i = 0; i < 10; i++)
    p_configuration_variables->walk_assist_level_factor[i] = flash_read_word(ADDRESS_WALK_ASSIST_LEVEL_FACTOR_0 + i);

  p_configuration_variables->cruise_function_enabled = flash_read_word(ADDRESS_CRUISE_FUNCTION_ENABLED);
  p_configuration_variables->cruise_function_set_target_speed_enabled =flash_read_word(ADDRESS_CRUISE_FUNCTION_SET_TARGET_SPEED_ENABLED);
  p_configuration_variables->cruise_function_target_speed_kph =flash_read_word(ADDRESS_CRUISE_FUNCTION_TARGET_SPEED_KPH);
  p_configuration_variables->cruise_function_target_speed_mph =flash_read_word(ADDRESS_CRUISE_FUNCTION_TARGET_SPEED_MPH);
  p_configuration_variables->show_cruise_function_set_target_speed =flash_read_word(ADDRESS_SHOW_CRUISE_FUNCTION_SET_TARGET_SPEED);
  p_configuration_variables->wheel_speed_field_state = flash_read_word(ADDRESS_WHEEL_SPEED_FIELD_STATE);
  p_configuration_variables->show_distance_data_odometer_field = flash_read_word(ADDRESS_SHOW_DISTANCE_DATA_ODOMETER_FIELD);
  p_configuration_variables->show_battery_state_odometer_field = flash_read_word(ADDRESS_SHOW_BATTERY_STATE_ODOMETER_FIELD);
  p_configuration_variables->show_pedal_data_odometer_field = flash_read_word(ADDRESS_SHOW_PEDAL_DATA_ODOMETER_FIELD);
  p_configuration_variables->show_time_measurement_odometer_field = flash_read_word(ADDRESS_SHOW_TIME_MEASUREMENT_ODOMETER_FIELD);
  p_configuration_variables->show_wheel_speed_odometer_field = flash_read_word(ADDRESS_SHOW_WHEEL_SPEED_ODOMETER_FIELD);
  p_configuration_variables->show_energy_data_odometer_field = flash_read_word(ADDRESS_SHOW_ENERGY_DATA_ODOMETER_FIELD);
  p_configuration_variables->show_motor_temperature_odometer_field = flash_read_word(ADDRESS_SHOW_MOTOR_TEMPERATURE_ODOMETER_FIELD);
  p_configuration_variables->show_battery_SOC_odometer_field = flash_read_word(ADDRESS_SHOW_BATTERY_SOC_ODOMETER_FIELD);
  p_configuration_variables->main_screen_power_menu_enabled = flash_read_word(ADDRESS_MAIN_SCREEN_POWER_MENU_ENABLED);
}

/**
 * @brief Write struct_configuration_variables to eeprom
 */
void eeprom_write_configuration(const struct_configuration_variables *p_configuration_variables)
{
  static uint32_t ee_data[(EEPROM_WORDS_STORED - 1)];

  /* fs_store is asynchronous. So wee need to copy to static memory first. */
  ee_data[ADDRESS_ASSIST_LEVEL] = p_configuration_variables->assist_level;
  ee_data[ADDRESS_WHEEL_PERIMETER] = p_configuration_variables->wheel_perimeter;
  ee_data[ADDRESS_MAX_WHEEL_SPEED] = p_configuration_variables->wheel_max_speed;
  ee_data[ADDRESS_UNITS_TYPE] = p_configuration_variables->units_type;
  ee_data[ADDRESS_HW_X10_OFFSET] = p_configuration_variables->wh_x10_offset;
  ee_data[ADDRESS_HW_X10_100_PERCENT_OFFSET] = p_configuration_variables->wh_x10_100_percent;
  ee_data[ADDRESS_BATTERY_SOC_FUNCTION_ENABLED] = p_configuration_variables->battery_SOC_function_enabled;
  ee_data[ADDRESS_ODOMETER_FIELD_STATE] = p_configuration_variables->odometer_field_state;
  ee_data[ADDRESS_BATTERY_MAX_CURRENT] = p_configuration_variables->battery_max_current;
  ee_data[ADDRESS_TARGET_MAX_BATTERY_POWER] = p_configuration_variables->target_max_battery_power_div25;
  ee_data[ADDRESS_BATTERY_CELLS_NUMBER] = p_configuration_variables->battery_cells_number;
  ee_data[ADDRESS_BATTERY_LOW_VOLTAGE_CUT_OFF_X10] = p_configuration_variables->battery_low_voltage_cut_off_x10;
  uint8_t config0 = p_configuration_variables->motor_type;
  config0 |= p_configuration_variables->motor_assistance_startup_without_pedal_rotation << 2;
  config0 |= p_configuration_variables->temperature_limit_feature_enabled << 3;
  config0 |= p_configuration_variables->temperature_field_state << 5;
  ee_data[ADDRESS_CONFIG_0] = config0;
  for (uint8_t i = 0; i < 9; i++)
    ee_data[ADDRESS_ASSIST_LEVEL_FACTOR_1 + i] = p_configuration_variables->assist_level_factor[i];

  ee_data[ADDRESS_NUMBER_OF_ASSIST_LEVELS] = p_configuration_variables->number_of_assist_levels;
  ee_data[ADDRESS_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED] = p_configuration_variables->startup_motor_power_boost_feature_enabled;
  ee_data[ADDRESS_STARTUP_MOTOR_POWER_BOOST_STATE] = p_configuration_variables->startup_motor_power_boost_state;
  for (uint8_t i = 0; i < 9; i++)
    ee_data[ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1 + i] = p_configuration_variables->startup_motor_power_boost_factor[i];

  ee_data[ADDRESS_STARTUP_MOTOR_POWER_BOOST_TIME] = p_configuration_variables->startup_motor_power_boost_time;
  ee_data[ADDRESS_STARTUP_MOTOR_POWER_BOOST_FADE_TIME] = p_configuration_variables->startup_motor_power_boost_fade_time;
  ee_data[ADDRESS_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT] = p_configuration_variables->motor_temperature_min_value_to_limit;
  ee_data[ADDRESS_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT] = p_configuration_variables->motor_temperature_max_value_to_limit;
  ee_data[ADDRESS_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10] = p_configuration_variables->battery_voltage_reset_wh_counter_x10;
  ee_data[ADDRESS_LCD_POWER_OFF_TIME] = p_configuration_variables->lcd_power_off_time_minutes;
  ee_data[ADDRESS_BATTERY_PACK_RESISTANCE] = p_configuration_variables->battery_pack_resistance_x1000;
  ee_data[ADDRESS_OFFROAD_FEATURE_ENABLED] = p_configuration_variables->offroad_feature_enabled;
  ee_data[ADDRESS_OFFROAD_MODE_ENABLED_ON_STARTUP] = p_configuration_variables->offroad_enabled_on_startup;
  ee_data[ADDRESS_OFFROAD_SPEED_LIMIT] = p_configuration_variables->offroad_speed_limit;
  ee_data[ADDRESS_OFFROAD_POWER_LIMIT_ENABLED] = p_configuration_variables->offroad_power_limit_enabled;
  ee_data[ADDRESS_OFFROAD_POWER_LIMIT_DIV25] = p_configuration_variables->offroad_power_limit_div25;
  ee_data[ADDRESS_ODOMETER_X10] = p_configuration_variables->odometer_x10;
  ee_data[ADDRESS_TRIP_X10] = p_configuration_variables->trip_x10;
  ee_data[ADDRESS_ODOMETER_SUB_FIELD_STATE_0] = p_configuration_variables->odometer_sub_field_state_0;
  ee_data[ADDRESS_ODOMETER_SUB_FIELD_STATE_1] = p_configuration_variables->odometer_sub_field_state_1;
  ee_data[ADDRESS_ODOMETER_SUB_FIELD_STATE_2] = p_configuration_variables->odometer_sub_field_state_2;
  ee_data[ADDRESS_ODOMETER_SUB_FIELD_STATE_3] = p_configuration_variables->odometer_sub_field_state_3;
  ee_data[ADDRESS_ODOMETER_SUB_FIELD_STATE_4] = p_configuration_variables->odometer_sub_field_state_4;
  ee_data[ADDRESS_ODOMETER_SUB_FIELD_STATE_5] = p_configuration_variables->odometer_sub_field_state_5;
  ee_data[ADDRESS_ODOMETER_SUB_FIELD_STATE_6] = p_configuration_variables->odometer_sub_field_state_6;
  ee_data[ADDRESS_MAX_WHEEL_SPEED_IMPERIAL] = p_configuration_variables->wheel_max_speed_imperial;
  ee_data[ADDRESS_TIME_MEASUREMENT_FIELD_STATE] = p_configuration_variables->time_measurement_field_state;
  ee_data[ADDRESS_TOTAL_SECOND_TTM] = p_configuration_variables->total_second_TTM;
  ee_data[ADDRESS_TOTAL_MINUTE_TTM] = p_configuration_variables->total_minute_TTM;
  ee_data[ADDRESS_TOTAL_HOUR_TTM] = p_configuration_variables->total_hour_TTM;
  ee_data[ADDRESS_RAMP_UP_AMPS_PER_SECOND_X10] = p_configuration_variables->ramp_up_amps_per_second_x10;
  ee_data[ADDRESS_WALK_ASSIST_FUNCTION_ENABLED] = p_configuration_variables->walk_assist_function_enabled;
  for (uint8_t i = 0; i < 10; i++)
    ee_data[ADDRESS_WALK_ASSIST_LEVEL_FACTOR_0 + i] = p_configuration_variables->walk_assist_level_factor[i];

  ee_data[ADDRESS_CRUISE_FUNCTION_ENABLED] = p_configuration_variables->cruise_function_enabled;
  ee_data[ADDRESS_CRUISE_FUNCTION_SET_TARGET_SPEED_ENABLED] = p_configuration_variables->cruise_function_set_target_speed_enabled;
  ee_data[ADDRESS_CRUISE_FUNCTION_TARGET_SPEED_KPH] = p_configuration_variables->cruise_function_target_speed_kph;
  ee_data[ADDRESS_CRUISE_FUNCTION_TARGET_SPEED_MPH] = p_configuration_variables->cruise_function_target_speed_mph;
  ee_data[ADDRESS_SHOW_CRUISE_FUNCTION_SET_TARGET_SPEED] = p_configuration_variables->show_cruise_function_set_target_speed;
  ee_data[ADDRESS_WHEEL_SPEED_FIELD_STATE] = p_configuration_variables->wheel_speed_field_state;
  ee_data[ADDRESS_SHOW_DISTANCE_DATA_ODOMETER_FIELD] = p_configuration_variables->show_distance_data_odometer_field;
  ee_data[ADDRESS_SHOW_BATTERY_STATE_ODOMETER_FIELD] = p_configuration_variables->show_battery_state_odometer_field;
  ee_data[ADDRESS_SHOW_PEDAL_DATA_ODOMETER_FIELD] = p_configuration_variables->show_pedal_data_odometer_field;
  ee_data[ADDRESS_SHOW_TIME_MEASUREMENT_ODOMETER_FIELD] = p_configuration_variables->show_time_measurement_odometer_field;
  ee_data[ADDRESS_SHOW_WHEEL_SPEED_ODOMETER_FIELD] = p_configuration_variables->show_wheel_speed_odometer_field;
  ee_data[ADDRESS_SHOW_ENERGY_DATA_ODOMETER_FIELD] = p_configuration_variables->show_energy_data_odometer_field;
  ee_data[ADDRESS_SHOW_MOTOR_TEMPERATURE_ODOMETER_FIELD] = p_configuration_variables->show_motor_temperature_odometer_field;
  ee_data[ADDRESS_SHOW_BATTERY_SOC_ODOMETER_FIELD] = p_configuration_variables->show_battery_SOC_odometer_field;
  ee_data[ADDRESS_MAIN_SCREEN_POWER_MENU_ENABLED] = p_configuration_variables->main_screen_power_menu_enabled;

  /* Write to flash without ADDRESS_KEY */
  flash_write_words(0, ee_data, (EEPROM_WORDS_STORED - 1));
}

/**
 * @brief Read value from eeprom
 */
uint32_t eeprom_read_value(uint8_t address)
{
  return flash_read_word(address);
}

/**
 * @brief Write value to eeprom. Writing data is asynchronous. So make sure, pointer to value stays valid!
 */
bool eeprom_write_value(uint8_t address, uint32_t* value)
{
  return (flash_write_words(address, value, 1) == FS_SUCCESS) ? true : false;
}

/**
 * @brief Poll last eeprom_write_* result.
 * Returns 0xFF if ongoing and fs_ret_t on completion.
 */
fs_ret_t eeprom_get_last_result(void)
{
  return last_fs_ret;
}



/* Event handler */

/* Register fs_sys_event_handler with softdevice_sys_evt_handler_set in ble_stack_init or this doesn't fire! */
static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result)
{
  last_fs_ret = result;
}
