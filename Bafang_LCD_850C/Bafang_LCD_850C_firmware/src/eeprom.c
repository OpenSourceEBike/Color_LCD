/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stdio.h"
#include "stm32f10x_flash.h"
#include "eeprom.h"
#include "main.h"

#define EEPROM_START_ADDRESS            0x0807F000
#define EEPROM_START_ADDRESS_PAGE_0     0x0807F000
#define EEPROM_START_ADDRESS_PAGE_1     0x0807F800 // last page of 2kbytes of flash memory

#define EEPROM_PAGE_KEY_ADDRESS         (1024 - 1)
#define EEPROM_PAGE_WRITE_ID_ADDRESS    (EEPROM_PAGE_KEY_ADDRESS - 1)

#define EEPROM_MAGIC_KEY                ((uint8_t) 0x5a)

static uint8_t array_default_values[EEPROM_BYTES_STORED] = {
  KEY,
  DEFAULT_VALUE_ASSIST_LEVEL,
  DEFAULT_VALUE_WHEEL_PERIMETER_0,
  DEFAULT_VALUE_WHEEL_PERIMETER_1,
  DEFAULT_VALUE_WHEEL_MAX_SPEED,
  DEFAULT_VALUE_UNITS_TYPE,
  DEFAULT_VALUE_WH_OFFSET,
  DEFAULT_VALUE_WH_OFFSET,
  DEFAULT_VALUE_WH_OFFSET,
  DEFAULT_VALUE_WH_OFFSET,
  DEFAULT_VALUE_HW_X10_100_PERCENT,
  DEFAULT_VALUE_HW_X10_100_PERCENT,
  DEFAULT_VALUE_HW_X10_100_PERCENT,
  DEFAULT_VALUE_HW_X10_100_PERCENT,
  DEAFULT_VALUE_SHOW_NUMERIC_BATTERY_SOC,
  DEFAULT_VALUE_ODOMETER_FIELD_STATE,
  DEFAULT_VALUE_BATTERY_MAX_CURRENT,
  DEFAULT_VALUE_TARGET_MAX_BATTERY_POWER,
  DEFAULT_VALUE_BATTERY_CELLS_NUMBER,
  DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10_0,
  DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10_1,
  DEFAULT_VALUE_CONFIG_0,
  DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_1,
  DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_2,
  DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_3,
  DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_4,
  DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_5,
  DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_6,
  DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_7,
  DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_8,
  DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_9,
  DEFAULT_VALUE_NUMBER_OF_ASSIST_LEVELS,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_STATE,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_2,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_3,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_4,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_5,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_6,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_7,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_8,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_9,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_TIME,
  DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FADE_TIME,
  DEFAULT_VALUE_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT,
  DEFAULT_VALUE_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT,
  DEFAULT_VALUE_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10_0,
  DEFAULT_VALUE_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10_1,
  DEFAULT_VALUE_LCD_POWER_OFF_TIME,
  DEFAULT_VALUE_LCD_BACKLIGHT_ON_BRIGHTNESS,
  DEFAULT_VALUE_LCD_BACKLIGHT_OFF_BRIGHTNESS,
  DEFAULT_VALUE_BATTERY_PACK_RESISTANCE_0,
  DEFAULT_VALUE_BATTERY_PACK_RESISTANCE_1,
  DEFAULT_VALUE_OFFROAD_FEATURE_ENABLED,
  DEFAULT_VALUE_OFFROAD_MODE_ENABLED_ON_STARTUP,
  DEFAULT_VALUE_OFFROAD_SPEED_LIMIT,
  DEFAULT_VALUE_OFFROAD_POWER_LIMIT_ENABLED,
  DEFAULT_VALUE_OFFROAD_POWER_LIMIT_DIV25,
  DEFAULT_VALUE_ODOMETER_X10,
  DEFAULT_VALUE_ODOMETER_X10,
  DEFAULT_VALUE_ODOMETER_X10
};

uint32_t ui32_eeprom_page = 0;

void eeprom_erase_page(uint32_t ui32_eeprom_page);
uint8_t eeprom_read(uint32_t ui32_address);
uint8_t eeprom_read_from_page(uint32_t ui32_address, uint32_t ui32_eeprom_page);
uint32_t eeprom_write(uint32_t ui32_address, uint8_t ui8_data);
uint32_t eeprom_write_to_page(uint32_t ui32_address, uint8_t ui8_data, uint32_t ui32_eeprom_page);
static void eeprom_write_array(uint8_t *p_array, uint32_t ui32_len);
static void eeprom_read_values_to_variables(void);
static void variables_to_array(uint8_t *ui8_array);

void eeprom_init()
{
  uint8_t ui8_data;
  uint32_t ui32_eeprom_write_id;

  // check if both EEPROM pages have invalid values and if so, write default values on page 0
  if((eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 0) != EEPROM_MAGIC_KEY) &&
      (eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 1) != EEPROM_MAGIC_KEY))
  {
    // setup page 0
    ui32_eeprom_page = 0;
    eeprom_erase_page(ui32_eeprom_page);
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
      ui32_eeprom_page = 1;
    }
    else
    {
      ui32_eeprom_page = 0;
    }
  }
  else if(eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 0) == EEPROM_MAGIC_KEY)
  {
    ui32_eeprom_page = 0;
  }
  else
  {
    ui32_eeprom_page = 1;
  }

  // start by reading address 0 and see if value is different from our key,
  // if so mean that eeprom memory is clean (or data strucutre is invalid) and we need to populate
  ui8_data = eeprom_read(ADDRESS_KEY);
  if(ui8_data != KEY) // verify if our key exist
  {
    // cycle/increment ui32_eeprom_page, to next page
    ui32_eeprom_page = (ui32_eeprom_page + 1) % 2;

    // erase the new page
    eeprom_erase_page(ui32_eeprom_page);
    // write on the new page
    eeprom_write_array(array_default_values, ((uint8_t) EEPROM_BYTES_STORED));
    // write EEPROM_MAGIC_KEY and EEPROM_PAGE WRITE_ID
    if(ui32_eeprom_page == 0)
    {
      eeprom_write(EEPROM_PAGE_KEY_ADDRESS, EEPROM_MAGIC_KEY);
      ui32_eeprom_write_id = eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 1);
      ui32_eeprom_write_id++;
      eeprom_write(EEPROM_PAGE_WRITE_ID_ADDRESS, ui32_eeprom_write_id); // write new ID
    }
    else
    {
      eeprom_write(EEPROM_PAGE_KEY_ADDRESS, EEPROM_MAGIC_KEY);
      ui32_eeprom_write_id = eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 0);
      ui32_eeprom_write_id++;
      eeprom_write(EEPROM_PAGE_WRITE_ID_ADDRESS, ui32_eeprom_write_id); // write new ID
    }
  }

  // finally initialize the variables
  eeprom_init_variables();
}

void eeprom_init_variables(void)
{
  struct_configuration_variables *p_configuration_variables;
  p_configuration_variables = get_configuration_variables();

  eeprom_read_values_to_variables();
}

static void eeprom_read_values_to_variables(void)
{
  uint8_t ui8_temp;
  uint16_t ui16_temp;
  uint32_t ui32_temp;
  uint8_t ui8_index;

  struct_configuration_variables *p_configuration_variables;
  p_configuration_variables = get_configuration_variables();

  p_configuration_variables->ui8_assist_level = eeprom_read(ADDRESS_ASSIST_LEVEL);

  ui16_temp = eeprom_read(ADDRESS_WHEEL_PERIMETER_0);
  ui8_temp = eeprom_read(ADDRESS_WHEEL_PERIMETER_1);
  ui16_temp += (((uint16_t) ui8_temp << 8) & 0xff00);
  p_configuration_variables->ui16_wheel_perimeter = ui16_temp;

  p_configuration_variables->ui8_wheel_max_speed = eeprom_read(ADDRESS_MAX_WHEEL_SPEED);
  p_configuration_variables->ui8_units_type = eeprom_read(ADDRESS_UNITS_TYPE);

  ui32_temp = eeprom_read(ADDRESS_HW_X10_OFFSET_0);
  ui8_temp = eeprom_read(ADDRESS_HW_X10_OFFSET_1);
  ui32_temp += (((uint32_t) ui8_temp << 8) & 0xff00);
  ui8_temp = eeprom_read(ADDRESS_HW_X10_OFFSET_2);
  ui32_temp += (((uint32_t) ui8_temp << 16) & 0xff0000);
  ui8_temp = eeprom_read(ADDRESS_HW_X10_OFFSET_3);
  ui32_temp += (((uint32_t) ui8_temp << 24) & 0xff000000);
  __disable_irq();
  p_configuration_variables->ui32_wh_x10_offset = ui32_temp;
  __enable_irq();

  ui32_temp = eeprom_read(ADDRESS_HW_X10_100_PERCENT_OFFSET_0);
  ui8_temp = eeprom_read(ADDRESS_HW_X10_100_PERCENT_OFFSET_1);
  ui32_temp += (((uint32_t) ui8_temp << 8) & 0xff00);
  ui8_temp = eeprom_read(ADDRESS_HW_X10_100_PERCENT_OFFSET_2);
  ui32_temp += (((uint32_t) ui8_temp << 16) & 0xff0000);
  ui8_temp = eeprom_read(ADDRESS_HW_X10_100_PERCENT_OFFSET_3);
  ui32_temp += (((uint32_t) ui8_temp << 24) & 0xff000000);
  p_configuration_variables->ui32_wh_x10_100_percent = ui32_temp;

  p_configuration_variables->ui8_battery_soc_enable = eeprom_read(ADDRESS_SHOW_NUMERIC_BATTERY_SOC);
  p_configuration_variables->ui8_odometer_field_state = eeprom_read(ADDRESS_ODOMETER_FIELD_STATE);
  p_configuration_variables->ui8_battery_max_current = eeprom_read(ADDRESS_BATTERY_MAX_CURRENT);
  p_configuration_variables->ui8_target_max_battery_power = eeprom_read(ADDRESS_TARGET_MAX_BATTERY_POWER);
  p_configuration_variables->ui8_battery_cells_number = eeprom_read(ADDRESS_BATTERY_CELLS_NUMBER);

  ui16_temp = eeprom_read(ADDRESS_BATTERY_LOW_VOLTAGE_CUT_OFF_X10_0);
  ui8_temp = eeprom_read(ADDRESS_BATTERY_LOW_VOLTAGE_CUT_OFF_X10_1);
  ui16_temp += (((uint16_t) ui8_temp << 8) & 0xff00);
  p_configuration_variables->ui16_battery_low_voltage_cut_off_x10 = ui16_temp;

  ui8_temp = eeprom_read(ADDRESS_CONFIG_0);
  p_configuration_variables->ui8_motor_type = ui8_temp & 3;
  p_configuration_variables->ui8_motor_assistance_startup_without_pedal_rotation = (ui8_temp & 4) >> 2;
  p_configuration_variables->ui8_temperature_limit_feature_enabled = (ui8_temp & 8) >> 3;
  p_configuration_variables->ui8_temperature_field_config = (ui8_temp & 48) >> 4;

  p_configuration_variables->ui8_number_of_assist_levels = eeprom_read(ADDRESS_NUMBER_OF_ASSIST_LEVELS);
  for (ui8_index = 0; ui8_index < 9; ui8_index++)
  {
    p_configuration_variables->ui8_assist_level_factor [ui8_index] = eeprom_read(ADDRESS_ASSIST_LEVEL_FACTOR_1 + ui8_index);
  }

  p_configuration_variables->ui8_startup_motor_power_boost_feature_enabled = eeprom_read(ADDRESS_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED);
  p_configuration_variables->ui8_startup_motor_power_boost_state = eeprom_read(ADDRESS_STARTUP_MOTOR_POWER_BOOST_STATE);
  p_configuration_variables->ui8_startup_motor_power_boost_time = eeprom_read(ADDRESS_STARTUP_MOTOR_POWER_BOOST_TIME);
  for (ui8_index = 0; ui8_index < 9; ui8_index++)
  {
    p_configuration_variables->ui8_startup_motor_power_boost_factor [ui8_index] = eeprom_read(ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1 + ui8_index);
  }

  p_configuration_variables->ui8_startup_motor_power_boost_fade_time = eeprom_read(ADDRESS_STARTUP_MOTOR_POWER_BOOST_FADE_TIME);

  p_configuration_variables->ui8_motor_temperature_min_value_to_limit = eeprom_read(ADDRESS_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT);
  p_configuration_variables->ui8_motor_temperature_max_value_to_limit = eeprom_read(ADDRESS_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT);

  ui16_temp = eeprom_read(ADDRESS_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10_0);
  ui8_temp = eeprom_read(ADDRESS_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10_1);
  ui16_temp += (((uint16_t) ui8_temp << 8) & 0xff00);
  p_configuration_variables->ui16_battery_voltage_reset_wh_counter_x10 = ui16_temp;

  p_configuration_variables->ui8_lcd_power_off_time_minutes = eeprom_read(ADDRESS_LCD_POWER_OFF_TIME);
  p_configuration_variables->ui8_lcd_backlight_on_brightness = eeprom_read(ADDRESS_LCD_BACKLIGHT_ON_BRIGHTNESS);
  p_configuration_variables->ui8_lcd_backlight_off_brightness = eeprom_read(ADDRESS_LCD_BACKLIGHT_OFF_BRIGHTNESS);

  ui16_temp = eeprom_read(ADDRESS_BATTERY_PACK_RESISTANCE_0);
  ui8_temp = eeprom_read(ADDRESS_BATTERY_PACK_RESISTANCE_1);
  ui16_temp += (((uint16_t) ui8_temp << 8) & 0xff00);
  p_configuration_variables->ui16_battery_pack_resistance_x1000 = ui16_temp;

  p_configuration_variables->ui8_offroad_feature_enabled = eeprom_read(ADDRESS_OFFROAD_FEATURE_ENABLED);
  p_configuration_variables->ui8_offroad_enabled_on_startup = eeprom_read(ADDRESS_OFFROAD_MODE_ENABLED_ON_STARTUP);
  p_configuration_variables->ui8_offroad_speed_limit = eeprom_read(ADDRESS_OFFROAD_SPEED_LIMIT);
  p_configuration_variables->ui8_offroad_power_limit_enabled = eeprom_read(ADDRESS_OFFROAD_POWER_LIMIT_ENABLED);
  p_configuration_variables->ui8_offroad_power_limit_div25 = eeprom_read(ADDRESS_OFFROAD_POWER_LIMIT_DIV25);

  ui32_temp = eeprom_read(ADDRESS_ODOMETER_X10_0);
  ui8_temp = eeprom_read(ADDRESS_ODOMETER_X10_1);
  ui32_temp += (((uint32_t) ui8_temp << 8) & 0xff00);
  ui8_temp = eeprom_read(ADDRESS_ODOMETER_X10_2);
  ui32_temp += (((uint32_t) ui8_temp << 16) & 0xff0000);
  p_configuration_variables->ui32_odometer_x10 = ui32_temp;
}

void eeprom_write_variables(void)
{
  uint32_t ui32_eeprom_write_id;
  uint8_t array_variables[EEPROM_BYTES_STORED];

  // put all the variables in one array
  variables_to_array(array_variables);

  // cycle/increment ui32_eeprom_page, to next page
  ui32_eeprom_page = (ui32_eeprom_page + 1) % 2;

  // erase the new page
  eeprom_erase_page(ui32_eeprom_page);
  // write on the new page
  eeprom_write_array(array_variables, ((uint8_t) EEPROM_BYTES_STORED));
  // write EEPROM_MAGIC_KEY and EEPROM_PAGE WRITE_ID
  if(ui32_eeprom_page == 0)
  {
    eeprom_write(EEPROM_PAGE_KEY_ADDRESS, EEPROM_MAGIC_KEY);
    ui32_eeprom_write_id = eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 1);
    ui32_eeprom_write_id++;
    eeprom_write(EEPROM_PAGE_WRITE_ID_ADDRESS, ui32_eeprom_write_id); // write new ID
  }
  else
  {
    eeprom_write(EEPROM_PAGE_KEY_ADDRESS, EEPROM_MAGIC_KEY);
    ui32_eeprom_write_id = eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 0);
    ui32_eeprom_write_id++;
    eeprom_write(EEPROM_PAGE_WRITE_ID_ADDRESS, ui32_eeprom_write_id); // write new ID
  }
}

static void variables_to_array(uint8_t *ui8_array)
{
  uint8_t ui8_index;

  struct_configuration_variables *p_configuration_variables;
  p_configuration_variables = get_configuration_variables();

  ui8_array [0] = KEY;
  ui8_array [1] = p_configuration_variables->ui8_assist_level;
  ui8_array [2] = p_configuration_variables->ui16_wheel_perimeter & 255;
  ui8_array [3] = (p_configuration_variables->ui16_wheel_perimeter >> 8) & 255;
  ui8_array [4] = p_configuration_variables->ui8_wheel_max_speed;
  ui8_array [5] = p_configuration_variables->ui8_units_type;
  __disable_irq();
  ui8_array [6] = p_configuration_variables->ui32_wh_x10_offset & 255;
  ui8_array [7] = (p_configuration_variables->ui32_wh_x10_offset >> 8) & 255;
  ui8_array [8] = (p_configuration_variables->ui32_wh_x10_offset >> 16) & 255;
  ui8_array [9] = (p_configuration_variables->ui32_wh_x10_offset >> 24) & 255;
  __enable_irq();
  ui8_array [10] = p_configuration_variables->ui32_wh_x10_100_percent & 255;
  ui8_array [11] = (p_configuration_variables->ui32_wh_x10_100_percent >> 8) & 255;
  ui8_array [12] = (p_configuration_variables->ui32_wh_x10_100_percent >> 16) & 255;
  ui8_array [13] = (p_configuration_variables->ui32_wh_x10_100_percent >> 24) & 255;
  ui8_array [14] = p_configuration_variables->ui8_battery_soc_enable;
  ui8_array [15] = p_configuration_variables->ui8_odometer_field_state;
  ui8_array [16] = p_configuration_variables->ui8_battery_max_current;
  ui8_array [17] = p_configuration_variables->ui8_target_max_battery_power;
  ui8_array [18] = p_configuration_variables->ui8_battery_cells_number;
  ui8_array [19] = p_configuration_variables->ui16_battery_low_voltage_cut_off_x10 & 255;
  ui8_array [20] = (p_configuration_variables->ui16_battery_low_voltage_cut_off_x10 >> 8) & 255;
  ui8_array [21] = (p_configuration_variables->ui8_motor_type & 3) |
                      ((p_configuration_variables->ui8_motor_assistance_startup_without_pedal_rotation & 1) << 2) |
                      ((p_configuration_variables->ui8_temperature_limit_feature_enabled & 1) << 3) |
                      ((p_configuration_variables->ui8_temperature_field_config & 3) << 4);

  for (ui8_index = 0; ui8_index < 9; ui8_index++)
  {
    ui8_array [22 + ui8_index] = p_configuration_variables->ui8_assist_level_factor [ui8_index];
  }
  ui8_array [31] = p_configuration_variables->ui8_number_of_assist_levels;

  ui8_array [32] = p_configuration_variables->ui8_startup_motor_power_boost_feature_enabled;
  ui8_array [33] = p_configuration_variables->ui8_startup_motor_power_boost_state;
  for (ui8_index = 0; ui8_index < 9; ui8_index++)
  {
    ui8_array [34 + ui8_index] = p_configuration_variables->ui8_startup_motor_power_boost_factor [ui8_index];
  }
  ui8_array [43] = p_configuration_variables->ui8_startup_motor_power_boost_time;
  ui8_array [44] = p_configuration_variables->ui8_startup_motor_power_boost_fade_time;
  ui8_array [45] = p_configuration_variables->ui8_motor_temperature_min_value_to_limit;
  ui8_array [46] = p_configuration_variables->ui8_motor_temperature_max_value_to_limit;

  ui8_array [47] = p_configuration_variables->ui16_battery_voltage_reset_wh_counter_x10 & 255;
  ui8_array [48] = (p_configuration_variables->ui16_battery_voltage_reset_wh_counter_x10 >> 8) & 255;

  ui8_array [49] = p_configuration_variables->ui8_lcd_power_off_time_minutes;
  ui8_array [50] = p_configuration_variables->ui8_lcd_backlight_on_brightness;
  ui8_array [51] = p_configuration_variables->ui8_lcd_backlight_off_brightness;

  ui8_array [52] = p_configuration_variables->ui16_battery_pack_resistance_x1000 & 255;
  ui8_array [53] = (p_configuration_variables->ui16_battery_pack_resistance_x1000 >> 8) & 255;

  ui8_array [54] = p_configuration_variables->ui8_offroad_feature_enabled;
  ui8_array [55] = p_configuration_variables->ui8_offroad_enabled_on_startup;
  ui8_array [56] = p_configuration_variables->ui8_offroad_speed_limit;
  ui8_array [57] = p_configuration_variables->ui8_offroad_power_limit_enabled;
  ui8_array [58] = p_configuration_variables->ui8_offroad_power_limit_div25;

  ui8_array [59] = p_configuration_variables->ui32_odometer_x10 & 255;
  ui8_array [60] = (p_configuration_variables->ui32_odometer_x10 >> 8) & 255;
  ui8_array [61] = (p_configuration_variables->ui32_odometer_x10 >> 16) & 255;
}

static void eeprom_write_array(uint8_t *p_array, uint32_t ui32_len)
{
  uint32_t ui32_i;
  uint8_t *p_array_data;

  // write the full array
  p_array_data = p_array;
  for(ui32_i = 0; ui32_i < ui32_len; ui32_i++)
  {
    eeprom_write(ui32_i, *p_array_data++);
  }
}

void eeprom_erase_key_value(void)
{
  eeprom_write(ADDRESS_KEY, 0);
}

uint8_t eeprom_read(uint32_t ui32_address)
{
  uint16_t *ui16_p_address = (uint16_t *) (((uint32_t) EEPROM_START_ADDRESS) + (ui32_eeprom_page * 2048) + (ui32_address * 2));
  return (uint8_t) (*ui16_p_address);
}

uint8_t eeprom_read_from_page(uint32_t ui32_address, uint32_t ui32_eeprom_page)
{
  uint16_t *ui16_p_address = (uint16_t *) (((uint32_t) EEPROM_START_ADDRESS) + (ui32_eeprom_page * 2048) + (ui32_address * 2));
  return (uint8_t) (*ui16_p_address);
}

uint32_t eeprom_write(uint32_t ui32_address, uint8_t ui8_data)
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
