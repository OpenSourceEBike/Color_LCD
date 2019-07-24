/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stdio.h"
#include <string.h>
#include "stm32f10x_flash.h"
#include "eeprom.h"
#include "main.h"
#include "lcd_configurations.h"
#include "lcd.h"

// to make structures use only bytes and have no paddings
#pragma pack(1)

#define EEPROM_START_ADDRESS            0x0807F000
#define EEPROM_START_ADDRESS_PAGE_0     0x0807F000
#define EEPROM_START_ADDRESS_PAGE_1     0x0807F800 // last page of 2kbytes of flash memory

#define EEPROM_PAGE_KEY_ADDRESS         (1024 - 1)
#define EEPROM_PAGE_WRITE_ID_ADDRESS    (EEPROM_PAGE_KEY_ADDRESS - 1)

#define EEPROM_MAGIC_KEY                ((uint8_t) 0x5a)

#define EEPROM_ADDRESS_KEY              0 // the first address of emulated EEPROM where the key is stored

uint32_t ui32_m_eeprom_page = 0;
static eeprom_data_t m_eeprom_data;

eeprom_data_t m_eeprom_data_defaults =
{
  .ui8_assist_level = DEFAULT_VALUE_ASSIST_LEVEL,
  .ui16_wheel_perimeter = DEFAULT_VALUE_WHEEL_PERIMETER,
  .ui8_wheel_max_speed = DEFAULT_VALUE_WHEEL_MAX_SPEED,
  .ui8_units_type = DEFAULT_VALUE_UNITS_TYPE,
  .ui32_wh_x10_offset = DEFAULT_VALUE_WH_X10_OFFSET,
  .ui32_wh_x10_100_percent = DEFAULT_VALUE_HW_X10_100_PERCENT,
  .ui8_battery_soc_enable = DEAFULT_VALUE_SHOW_NUMERIC_BATTERY_SOC,
  .ui8_battery_max_current = DEFAULT_VALUE_BATTERY_MAX_CURRENT,
  .ui8_ramp_up_amps_per_second_x10 = DEFAULT_VALUE_RAMP_UP_AMPS_PER_SECOND_X10,
  .ui8_battery_cells_number = DEFAULT_VALUE_BATTERY_CELLS_NUMBER,
  .ui16_battery_low_voltage_cut_off_x10 = DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10,
  .ui8_motor_type = DEFAULT_VALUE_MOTOR_TYPE,
  .ui8_motor_assistance_startup_without_pedal_rotation = DEFAULT_VALUE_MOTOR_ASSISTANCE_WITHOUT_PEDAL_ROTATION,
  .ui8_assist_level_factor = {
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
  .ui8_number_of_assist_levels = DEFAULT_VALUE_NUMBER_OF_ASSIST_LEVELS,
  .ui8_startup_motor_power_boost_feature_enabled = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED,
  .ui8_startup_motor_power_boost_state = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_STATE,
  .ui8_startup_motor_power_boost_factor = {
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
  .ui8_startup_motor_power_boost_time = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_TIME,
  .ui8_startup_motor_power_boost_fade_time = DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FADE_TIME,
  .ui8_temperature_limit_feature_enabled = DEFAULT_VALUE_MOTOR_TEMPERATURE_FEATURE_ENABLE,
  .ui8_motor_temperature_min_value_to_limit = DEFAULT_VALUE_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT,
  .ui8_motor_temperature_max_value_to_limit = DEFAULT_VALUE_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT,
  .ui16_battery_voltage_reset_wh_counter_x10 = DEFAULT_VALUE_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10,
  .ui8_lcd_power_off_time_minutes = DEFAULT_VALUE_LCD_POWER_OFF_TIME,
  .ui8_lcd_backlight_on_brightness = DEFAULT_VALUE_LCD_BACKLIGHT_ON_BRIGHTNESS,
  .ui8_lcd_backlight_off_brightness = DEFAULT_VALUE_LCD_BACKLIGHT_OFF_BRIGHTNESS,
  .ui16_battery_pack_resistance_x1000 = DEFAULT_VALUE_BATTERY_PACK_RESISTANCE,
  .ui8_offroad_feature_enabled = DEFAULT_VALUE_OFFROAD_FEATURE_ENABLED,
  .ui8_offroad_enabled_on_startup = DEFAULT_VALUE_OFFROAD_MODE_ENABLED_ON_STARTUP,
  .ui8_offroad_speed_limit = DEFAULT_VALUE_OFFROAD_SPEED_LIMIT,
  .ui8_offroad_power_limit_enabled = DEFAULT_VALUE_OFFROAD_POWER_LIMIT_ENABLED,
  .ui8_offroad_power_limit_div25 = DEFAULT_VALUE_OFFROAD_POWER_LIMIT_DIV25,
  .ui32_odometer_x10 = DEFAULT_VALUE_ODOMETER_X10,
  .ui8_walk_assist_feature_enabled = DEFAULT_VALUE_WALK_ASSIST_FEATURE_ENABLED,
  .ui8_walk_assist_level_factor = {
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
  .lcd_configurations_menu = {
    .ui8_item_number = 1,
    .ui8_previous_item_number = 0xff,
    .ui8_item_visible_start_index = 0,
    .ui8_item_visible_index = 1,
    .ui8_refresh_full_menu_1 = 0,
    .ui8_refresh_full_menu_2 = 0,
    .ui8_battery_soc_power_used_state = 0,
  },
  .graph_id = GRAPH_PEDAL_HUMAN_POWER,
};

static void eeprom_erase_page(uint32_t ui32_eeprom_page);
static uint8_t eeprom_read(uint32_t ui32_address);
static uint8_t eeprom_read_from_page(uint32_t ui32_address, uint32_t ui32_eeprom_page);
static uint32_t eeprom_write(uint32_t ui32_address, uint8_t ui8_data);
static uint32_t eeprom_write_to_page(uint32_t ui32_address, uint8_t ui8_data, uint32_t ui32_eeprom_page);
static void eeprom_write_array(uint8_t *p_array, uint32_t ui32_len);
static void eeprom_write_defaults(void);

void eeprom_init()
{
  uint8_t ui8_data;
  uint32_t ui32_eeprom_write_id;

  // check if both EEPROM pages have invalid values and if so, write default values on page 0
  if((eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 0) != EEPROM_MAGIC_KEY) &&
      (eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 1) != EEPROM_MAGIC_KEY))
  {
    // setup page 0
    ui32_m_eeprom_page = 0;
    eeprom_erase_page(ui32_m_eeprom_page);
    // write EEPROM_MAGIC_KEY and EEPROM_PAGE WRITE_ID
    eeprom_write(EEPROM_PAGE_KEY_ADDRESS, EEPROM_MAGIC_KEY);
    eeprom_write(EEPROM_PAGE_WRITE_ID_ADDRESS, 1); // start with value 1
  }
  // check if both EEPROM pages have valid values...
  else if((eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 0) == EEPROM_MAGIC_KEY) &&
      (eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 1) == EEPROM_MAGIC_KEY))
  {
    // now find the page that has a higher write ID
    if(eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 1) > eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 0))
    {
      ui32_m_eeprom_page = 1;
    }
    else
    {
      ui32_m_eeprom_page = 0;
    }
  }
  else if(eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 0) == EEPROM_MAGIC_KEY)
  {
    ui32_m_eeprom_page = 0;
  }
  else
  {
    ui32_m_eeprom_page = 1;
  }

  // start by reading address 0 and see if value is different from our key,
  // if so mean that eeprom memory is clean (or data strucutre is invalid) and we need to populate
  ui8_data = eeprom_read(EEPROM_ADDRESS_KEY);
  if(ui8_data != KEY) // verify if our key exist
  {
    // cycle/increment ui32_eeprom_page, to next page
    ui32_m_eeprom_page = (ui32_m_eeprom_page + 1) % 2;

    // erase the new page
    eeprom_erase_page(ui32_m_eeprom_page);

    // write the key
    eeprom_write(EEPROM_ADDRESS_KEY, KEY);

    // write on the new page
    eeprom_write_defaults();
    // write EEPROM_MAGIC_KEY and EEPROM_PAGE WRITE_ID
    eeprom_write(EEPROM_PAGE_KEY_ADDRESS, EEPROM_MAGIC_KEY);
    if(ui32_m_eeprom_page == 0)
    {
      ui32_eeprom_write_id = eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 1);
    }
    else
    {
      ui32_eeprom_write_id = eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 0);
    }
    ui32_eeprom_write_id++;
    eeprom_write(EEPROM_PAGE_WRITE_ID_ADDRESS, ui32_eeprom_write_id); // write new ID
  }

  // finally initialize the variables
  eeprom_init_variables();
}

void eeprom_init_variables(void)
{
  uint32_t ui32_counter;
  uint8_t ui8_array[sizeof(m_eeprom_data)];
  l3_vars_t *p_l3_output_vars;
  lcd_configurations_menu_t *p_lcd_configurations_menu;
  p_l3_output_vars = get_l3_vars();
  p_lcd_configurations_menu = get_lcd_configurations_menu();

  // read the values from EEPROM to array
  memset(ui8_array, 0, sizeof(m_eeprom_data));
  for(ui32_counter = 0; ui32_counter < sizeof(m_eeprom_data); ui32_counter++)
  {
    // we start at EEPROM address 1 as 0 is already in use by the KEY
    ui8_array[ui32_counter] = eeprom_read(1 + ui32_counter);
  }

  // copy data from the array to the structure
  memset(&m_eeprom_data, 0, sizeof(m_eeprom_data));
  memcpy(&m_eeprom_data, ui8_array, sizeof(m_eeprom_data));

  // copy data final variables
  p_l3_output_vars->ui8_assist_level = m_eeprom_data.ui8_assist_level;
  p_l3_output_vars->ui16_wheel_perimeter = m_eeprom_data.ui16_wheel_perimeter;
  p_l3_output_vars->ui8_wheel_max_speed = m_eeprom_data.ui8_wheel_max_speed;
  p_l3_output_vars->ui8_units_type = m_eeprom_data.ui8_units_type;
  p_l3_output_vars->ui32_wh_x10_offset = m_eeprom_data.ui32_wh_x10_offset;
  p_l3_output_vars->ui32_wh_x10_100_percent = m_eeprom_data.ui32_wh_x10_100_percent;
  p_l3_output_vars->ui8_battery_soc_enable = m_eeprom_data.ui8_battery_soc_enable;
  p_l3_output_vars->ui8_battery_max_current = m_eeprom_data.ui8_battery_max_current;
  p_l3_output_vars->ui8_ramp_up_amps_per_second_x10 = m_eeprom_data.ui8_ramp_up_amps_per_second_x10;
  p_l3_output_vars->ui8_battery_cells_number = m_eeprom_data.ui8_battery_cells_number;
  p_l3_output_vars->ui16_battery_low_voltage_cut_off_x10 = m_eeprom_data.ui16_battery_low_voltage_cut_off_x10;
  p_l3_output_vars->ui8_motor_type = m_eeprom_data.ui8_motor_type;
  p_l3_output_vars->ui8_motor_assistance_startup_without_pedal_rotation = m_eeprom_data.ui8_motor_assistance_startup_without_pedal_rotation;
  p_l3_output_vars->ui8_temperature_limit_feature_enabled = m_eeprom_data.ui8_temperature_limit_feature_enabled;
  p_l3_output_vars->ui8_assist_level_factor[0] = m_eeprom_data.ui8_assist_level_factor[0];
  p_l3_output_vars->ui8_assist_level_factor[1] = m_eeprom_data.ui8_assist_level_factor[1];
  p_l3_output_vars->ui8_assist_level_factor[2] = m_eeprom_data.ui8_assist_level_factor[2];
  p_l3_output_vars->ui8_assist_level_factor[3] = m_eeprom_data.ui8_assist_level_factor[3];
  p_l3_output_vars->ui8_assist_level_factor[4] = m_eeprom_data.ui8_assist_level_factor[4];
  p_l3_output_vars->ui8_assist_level_factor[5] = m_eeprom_data.ui8_assist_level_factor[5];
  p_l3_output_vars->ui8_assist_level_factor[6] = m_eeprom_data.ui8_assist_level_factor[6];
  p_l3_output_vars->ui8_assist_level_factor[7] = m_eeprom_data.ui8_assist_level_factor[7];
  p_l3_output_vars->ui8_assist_level_factor[8] = m_eeprom_data.ui8_assist_level_factor[8];
  p_l3_output_vars->ui8_assist_level_factor[9] = m_eeprom_data.ui8_assist_level_factor[9];
  p_l3_output_vars->ui8_number_of_assist_levels = m_eeprom_data.ui8_number_of_assist_levels;
  p_l3_output_vars->ui8_startup_motor_power_boost_feature_enabled = m_eeprom_data.ui8_startup_motor_power_boost_feature_enabled;
  p_l3_output_vars->ui8_startup_motor_power_boost_state = m_eeprom_data.ui8_startup_motor_power_boost_state;
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[0] = m_eeprom_data.ui8_startup_motor_power_boost_factor[0];
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[1] = m_eeprom_data.ui8_startup_motor_power_boost_factor[1];
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[2] = m_eeprom_data.ui8_startup_motor_power_boost_factor[2];
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[3] = m_eeprom_data.ui8_startup_motor_power_boost_factor[3];
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[4] = m_eeprom_data.ui8_startup_motor_power_boost_factor[4];
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[5] = m_eeprom_data.ui8_startup_motor_power_boost_factor[5];
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[6] = m_eeprom_data.ui8_startup_motor_power_boost_factor[6];
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[7] = m_eeprom_data.ui8_startup_motor_power_boost_factor[7];
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[8] = m_eeprom_data.ui8_startup_motor_power_boost_factor[8];
  p_l3_output_vars->ui8_startup_motor_power_boost_factor[9] = m_eeprom_data.ui8_startup_motor_power_boost_factor[9];
  p_l3_output_vars->ui8_startup_motor_power_boost_time = m_eeprom_data.ui8_startup_motor_power_boost_time;
  p_l3_output_vars->ui8_startup_motor_power_boost_fade_time = m_eeprom_data.ui8_startup_motor_power_boost_fade_time;
  p_l3_output_vars->ui8_motor_temperature_min_value_to_limit = m_eeprom_data.ui8_motor_temperature_min_value_to_limit;
  p_l3_output_vars->ui8_motor_temperature_max_value_to_limit = m_eeprom_data.ui8_motor_temperature_max_value_to_limit;
  p_l3_output_vars->ui16_battery_voltage_reset_wh_counter_x10 = m_eeprom_data.ui16_battery_voltage_reset_wh_counter_x10;
  p_l3_output_vars->ui8_lcd_power_off_time_minutes = m_eeprom_data.ui8_lcd_power_off_time_minutes;
  p_l3_output_vars->ui8_lcd_backlight_on_brightness = m_eeprom_data.ui8_lcd_backlight_on_brightness;
  p_l3_output_vars->ui8_lcd_backlight_off_brightness = m_eeprom_data.ui8_lcd_backlight_off_brightness;
  p_l3_output_vars->ui16_battery_pack_resistance_x1000 = m_eeprom_data.ui16_battery_pack_resistance_x1000;
  p_l3_output_vars->ui8_offroad_feature_enabled = m_eeprom_data.ui8_offroad_feature_enabled;
  p_l3_output_vars->ui8_offroad_enabled_on_startup = m_eeprom_data.ui8_offroad_enabled_on_startup;
  p_l3_output_vars->ui8_offroad_speed_limit = m_eeprom_data.ui8_offroad_speed_limit;
  p_l3_output_vars->ui8_offroad_power_limit_enabled = m_eeprom_data.ui8_offroad_power_limit_enabled;
  p_l3_output_vars->ui8_offroad_power_limit_div25 = m_eeprom_data.ui8_offroad_power_limit_div25;
  p_l3_output_vars->ui32_odometer_x10 = m_eeprom_data.ui32_odometer_x10;
  p_l3_output_vars->ui8_walk_assist_feature_enabled = m_eeprom_data.ui8_walk_assist_feature_enabled;
  p_l3_output_vars->ui8_walk_assist_level_factor[0] = m_eeprom_data.ui8_walk_assist_level_factor[0];
  p_l3_output_vars->ui8_walk_assist_level_factor[1] = m_eeprom_data.ui8_walk_assist_level_factor[1];
  p_l3_output_vars->ui8_walk_assist_level_factor[2] = m_eeprom_data.ui8_walk_assist_level_factor[2];
  p_l3_output_vars->ui8_walk_assist_level_factor[3] = m_eeprom_data.ui8_walk_assist_level_factor[3];
  p_l3_output_vars->ui8_walk_assist_level_factor[4] = m_eeprom_data.ui8_walk_assist_level_factor[4];
  p_l3_output_vars->ui8_walk_assist_level_factor[5] = m_eeprom_data.ui8_walk_assist_level_factor[5];
  p_l3_output_vars->ui8_walk_assist_level_factor[6] = m_eeprom_data.ui8_walk_assist_level_factor[6];
  p_l3_output_vars->ui8_walk_assist_level_factor[7] = m_eeprom_data.ui8_walk_assist_level_factor[7];
  p_l3_output_vars->ui8_walk_assist_level_factor[8] = m_eeprom_data.ui8_walk_assist_level_factor[8];
  p_l3_output_vars->ui8_walk_assist_level_factor[9] = m_eeprom_data.ui8_walk_assist_level_factor[9];
  p_lcd_configurations_menu->ui8_item_number = m_eeprom_data.lcd_configurations_menu.ui8_item_number;
  p_lcd_configurations_menu->ui8_item_visible_start_index = m_eeprom_data.lcd_configurations_menu.ui8_item_visible_start_index;
  p_lcd_configurations_menu->ui8_item_visible_index = m_eeprom_data.lcd_configurations_menu.ui8_item_visible_index;
  p_lcd_configurations_menu->ui8_refresh_full_menu_1 = m_eeprom_data.lcd_configurations_menu.ui8_refresh_full_menu_1;
  p_lcd_configurations_menu->ui8_refresh_full_menu_2 = m_eeprom_data.lcd_configurations_menu.ui8_refresh_full_menu_2;
  p_lcd_configurations_menu->ui8_battery_soc_power_used_state = m_eeprom_data.lcd_configurations_menu.ui8_battery_soc_power_used_state;
  p_l3_output_vars->graph_id = m_eeprom_data.graph_id;
}

void eeprom_write_variables(void)
{
  uint32_t ui32_counter;
  uint8_t ui8_array[sizeof(m_eeprom_data)];
  l3_vars_t *p_l3_output_vars;
  volatile lcd_configurations_menu_t *p_lcd_configurations_menu;
  p_l3_output_vars = get_l3_vars();
  p_lcd_configurations_menu = get_lcd_configurations_menu();
  uint32_t ui32_eeprom_write_id;

  // write vars to eeprom struct
  memset(&m_eeprom_data, 0, sizeof(m_eeprom_data));
  m_eeprom_data.ui8_assist_level = p_l3_output_vars->ui8_assist_level;
  m_eeprom_data.ui16_wheel_perimeter = p_l3_output_vars->ui16_wheel_perimeter;
  m_eeprom_data.ui8_wheel_max_speed = p_l3_output_vars->ui8_wheel_max_speed;
  m_eeprom_data.ui8_units_type = p_l3_output_vars->ui8_units_type;
  m_eeprom_data.ui32_wh_x10_offset = p_l3_output_vars->ui32_wh_x10_offset;
  m_eeprom_data.ui32_wh_x10_100_percent = p_l3_output_vars->ui32_wh_x10_100_percent;
  m_eeprom_data.ui8_battery_soc_enable = p_l3_output_vars->ui8_battery_soc_enable;
  m_eeprom_data.ui8_battery_max_current = p_l3_output_vars->ui8_battery_max_current;
  m_eeprom_data.ui8_ramp_up_amps_per_second_x10 = p_l3_output_vars->ui8_ramp_up_amps_per_second_x10;
  m_eeprom_data.ui8_battery_cells_number = p_l3_output_vars->ui8_battery_cells_number;
  m_eeprom_data.ui16_battery_low_voltage_cut_off_x10 = p_l3_output_vars->ui16_battery_low_voltage_cut_off_x10;
  m_eeprom_data.ui8_motor_type = p_l3_output_vars->ui8_motor_type;
  m_eeprom_data.ui8_motor_assistance_startup_without_pedal_rotation = p_l3_output_vars->ui8_motor_assistance_startup_without_pedal_rotation;
  m_eeprom_data.ui8_temperature_limit_feature_enabled = p_l3_output_vars->ui8_temperature_limit_feature_enabled;
  m_eeprom_data.ui8_assist_level_factor[0] = p_l3_output_vars->ui8_assist_level_factor[0];
  m_eeprom_data.ui8_assist_level_factor[1] = p_l3_output_vars->ui8_assist_level_factor[1];
  m_eeprom_data.ui8_assist_level_factor[2] = p_l3_output_vars->ui8_assist_level_factor[2];
  m_eeprom_data.ui8_assist_level_factor[3] = p_l3_output_vars->ui8_assist_level_factor[3];
  m_eeprom_data.ui8_assist_level_factor[4] = p_l3_output_vars->ui8_assist_level_factor[4];
  m_eeprom_data.ui8_assist_level_factor[5] = p_l3_output_vars->ui8_assist_level_factor[5];
  m_eeprom_data.ui8_assist_level_factor[6] = p_l3_output_vars->ui8_assist_level_factor[6];
  m_eeprom_data.ui8_assist_level_factor[7] = p_l3_output_vars->ui8_assist_level_factor[7];
  m_eeprom_data.ui8_assist_level_factor[8] = p_l3_output_vars->ui8_assist_level_factor[8];
  m_eeprom_data.ui8_assist_level_factor[9] = p_l3_output_vars->ui8_assist_level_factor[9];
  m_eeprom_data.ui8_number_of_assist_levels = p_l3_output_vars->ui8_number_of_assist_levels;
  m_eeprom_data.ui8_startup_motor_power_boost_feature_enabled = p_l3_output_vars->ui8_startup_motor_power_boost_feature_enabled;
  m_eeprom_data.ui8_startup_motor_power_boost_state = p_l3_output_vars->ui8_startup_motor_power_boost_state;
  m_eeprom_data.ui8_startup_motor_power_boost_factor[0] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[0];
  m_eeprom_data.ui8_startup_motor_power_boost_factor[1] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[1];
  m_eeprom_data.ui8_startup_motor_power_boost_factor[2] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[2];
  m_eeprom_data.ui8_startup_motor_power_boost_factor[3] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[3];
  m_eeprom_data.ui8_startup_motor_power_boost_factor[4] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[4];
  m_eeprom_data.ui8_startup_motor_power_boost_factor[5] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[5];
  m_eeprom_data.ui8_startup_motor_power_boost_factor[6] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[6];
  m_eeprom_data.ui8_startup_motor_power_boost_factor[7] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[7];
  m_eeprom_data.ui8_startup_motor_power_boost_factor[8] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[8];
  m_eeprom_data.ui8_startup_motor_power_boost_factor[9] = p_l3_output_vars->ui8_startup_motor_power_boost_factor[9];
  m_eeprom_data.ui8_startup_motor_power_boost_time = p_l3_output_vars->ui8_startup_motor_power_boost_time;
  m_eeprom_data.ui8_startup_motor_power_boost_fade_time = p_l3_output_vars->ui8_startup_motor_power_boost_fade_time;
  m_eeprom_data.ui8_motor_temperature_min_value_to_limit = p_l3_output_vars->ui8_motor_temperature_min_value_to_limit;
  m_eeprom_data.ui8_motor_temperature_max_value_to_limit = p_l3_output_vars->ui8_motor_temperature_max_value_to_limit;
  m_eeprom_data.ui16_battery_voltage_reset_wh_counter_x10 = p_l3_output_vars->ui16_battery_voltage_reset_wh_counter_x10;
  m_eeprom_data.ui8_lcd_power_off_time_minutes = p_l3_output_vars->ui8_lcd_power_off_time_minutes;
  m_eeprom_data.ui8_lcd_backlight_on_brightness = p_l3_output_vars->ui8_lcd_backlight_on_brightness;
  m_eeprom_data.ui8_lcd_backlight_off_brightness = p_l3_output_vars->ui8_lcd_backlight_off_brightness;
  m_eeprom_data.ui16_battery_pack_resistance_x1000 = p_l3_output_vars->ui16_battery_pack_resistance_x1000;
  m_eeprom_data.ui8_offroad_feature_enabled = p_l3_output_vars->ui8_offroad_feature_enabled;
  m_eeprom_data.ui8_offroad_enabled_on_startup = p_l3_output_vars->ui8_offroad_enabled_on_startup;
  m_eeprom_data.ui8_offroad_speed_limit = p_l3_output_vars->ui8_offroad_speed_limit;
  m_eeprom_data.ui8_offroad_power_limit_enabled = p_l3_output_vars->ui8_offroad_power_limit_enabled;
  m_eeprom_data.ui8_offroad_power_limit_div25 = p_l3_output_vars->ui8_offroad_power_limit_div25;
  m_eeprom_data.ui32_odometer_x10 = p_l3_output_vars->ui32_odometer_x10;
  m_eeprom_data.ui8_walk_assist_feature_enabled = p_l3_output_vars->ui8_walk_assist_feature_enabled;
  m_eeprom_data.ui8_walk_assist_level_factor[0] = p_l3_output_vars->ui8_walk_assist_level_factor[0];
  m_eeprom_data.ui8_walk_assist_level_factor[1] = p_l3_output_vars->ui8_walk_assist_level_factor[1];
  m_eeprom_data.ui8_walk_assist_level_factor[2] = p_l3_output_vars->ui8_walk_assist_level_factor[2];
  m_eeprom_data.ui8_walk_assist_level_factor[3] = p_l3_output_vars->ui8_walk_assist_level_factor[3];
  m_eeprom_data.ui8_walk_assist_level_factor[4] = p_l3_output_vars->ui8_walk_assist_level_factor[4];
  m_eeprom_data.ui8_walk_assist_level_factor[5] = p_l3_output_vars->ui8_walk_assist_level_factor[5];
  m_eeprom_data.ui8_walk_assist_level_factor[6] = p_l3_output_vars->ui8_walk_assist_level_factor[6];
  m_eeprom_data.ui8_walk_assist_level_factor[7] = p_l3_output_vars->ui8_walk_assist_level_factor[7];
  m_eeprom_data.ui8_walk_assist_level_factor[8] = p_l3_output_vars->ui8_walk_assist_level_factor[8];
  m_eeprom_data.ui8_walk_assist_level_factor[9] = p_l3_output_vars->ui8_walk_assist_level_factor[9];
  m_eeprom_data.lcd_configurations_menu.ui8_item_number = p_lcd_configurations_menu->ui8_item_number;
  m_eeprom_data.lcd_configurations_menu.ui8_item_visible_start_index = p_lcd_configurations_menu->ui8_item_visible_start_index;
  m_eeprom_data.lcd_configurations_menu.ui8_item_visible_index = p_lcd_configurations_menu->ui8_item_visible_index;
  m_eeprom_data.lcd_configurations_menu.ui8_refresh_full_menu_1 = p_lcd_configurations_menu->ui8_refresh_full_menu_1;
  m_eeprom_data.lcd_configurations_menu.ui8_refresh_full_menu_2 = p_lcd_configurations_menu->ui8_refresh_full_menu_2;
  m_eeprom_data.lcd_configurations_menu.ui8_battery_soc_power_used_state = p_lcd_configurations_menu->ui8_battery_soc_power_used_state;
  m_eeprom_data.graph_id = p_l3_output_vars->graph_id;

  // eeprom structure to array
  memset(ui8_array, 0, sizeof(m_eeprom_data));
  memcpy(&ui8_array, &m_eeprom_data, sizeof(m_eeprom_data));

  // cycle/increment ui32_eeprom_page, to next page
  ui32_m_eeprom_page = (ui32_m_eeprom_page + 1) % 2;

  // erase the new page
  eeprom_erase_page(ui32_m_eeprom_page);
  // write on the new page
  eeprom_write_array(ui8_array, sizeof(m_eeprom_data));
  // write key
  eeprom_write(EEPROM_ADDRESS_KEY, KEY);
  // write EEPROM_MAGIC_KEY and EEPROM_PAGE WRITE_ID
  eeprom_write(EEPROM_PAGE_KEY_ADDRESS, EEPROM_MAGIC_KEY);
  if(ui32_m_eeprom_page == 0)
  {
    ui32_eeprom_write_id = eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 1);
  }
  else
  {
    ui32_eeprom_write_id = eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 0);
  }
  ui32_eeprom_write_id++;
  eeprom_write(EEPROM_PAGE_WRITE_ID_ADDRESS, ui32_eeprom_write_id); // write new ID
}

static void eeprom_write_array(uint8_t *p_array, uint32_t ui32_len)
{
  uint32_t ui32_i;
  uint8_t *p_array_data;

  // write the full array
  p_array_data = p_array;
  for(ui32_i = 0; ui32_i < ui32_len; ui32_i++)
  {
    // start at 1 since 0 address is for KEY
    eeprom_write(1 + ui32_i, *p_array_data++);
  }
}

void eeprom_init_defaults(void)
{
  // first force KEY value to 0
  eeprom_write(ADDRESS_KEY, 0);

  // eeprom_init() will read the default values now
  eeprom_init();
}

uint8_t eeprom_read(uint32_t ui32_address)
{
  uint16_t *ui16_p_address = (uint16_t *) (((uint32_t) EEPROM_START_ADDRESS) + (ui32_m_eeprom_page * 2048) + (ui32_address * 2));
  return (uint8_t) (*ui16_p_address);
}

uint8_t eeprom_read_from_page(uint32_t ui32_address, uint32_t ui32_eeprom_page)
{
  uint16_t *ui16_p_address = (uint16_t *) (((uint32_t) EEPROM_START_ADDRESS) + (ui32_eeprom_page * 2048) + (ui32_address * 2));
  return (uint8_t) (*ui16_p_address);
}

uint32_t eeprom_write(uint32_t ui32_address, uint8_t ui8_data)
{
  ui32_address = ((uint32_t) EEPROM_START_ADDRESS) + (ui32_m_eeprom_page * 2048) + (ui32_address * 2);

  FLASH_Unlock();
  if(FLASH_ProgramHalfWord(ui32_address, (uint16_t) ui8_data) != FLASH_COMPLETE)
  {
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_Lock();
    return 1;
  }
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  FLASH_Lock();

  return 0;
}

uint32_t eeprom_write_to_page(uint32_t ui32_address, uint8_t ui8_data, uint32_t ui32_eeprom_page)
{
  ui32_address = ((uint32_t) EEPROM_START_ADDRESS) + (ui32_eeprom_page * 2048) + (ui32_address * 2);

  FLASH_Unlock();
  if(FLASH_ProgramHalfWord(ui32_address, (uint16_t) ui8_data) != FLASH_COMPLETE)
  {
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_Lock();
    return 1;
  }
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  FLASH_Lock();

  return 0;
}

void eeprom_erase_page(uint32_t ui32_eeprom_page)
{
  FLASH_Unlock();
  FLASH_ErasePage(((uint32_t) EEPROM_START_ADDRESS) + (ui32_eeprom_page * 2048));
  FLASH_Lock();
}

void eeprom_write_defaults(void)
{
  uint32_t ui32_i;
  uint8_t *p_array_data;
  uint8_t ui8_array[sizeof(m_eeprom_data_defaults)];

  // read the values from EEPROM to array
  memset(ui8_array, 0, sizeof(m_eeprom_data_defaults));
  memcpy(&ui8_array, &m_eeprom_data_defaults, sizeof(m_eeprom_data_defaults));

  // write the full array
  p_array_data = ui8_array;
  for(ui32_i = 0; ui32_i < sizeof(m_eeprom_data_defaults); ui32_i++)
  {
    // start at 1 since 0 address is for KEY
    eeprom_write(1 + ui32_i, *p_array_data++);
  }
}
