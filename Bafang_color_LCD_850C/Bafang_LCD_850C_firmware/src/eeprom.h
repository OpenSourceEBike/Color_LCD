/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _EEPROM_H_
#define _EEPROM_H_

#include "lcd.h"

#define KEY                                                                 0x01

#define ADDRESS_KEY                                                         0
#define ADDRESS_ASSIST_LEVEL                                                1
#define ADDRESS_WHEEL_PERIMETER_0                                           2
#define ADDRESS_WHEEL_PERIMETER_1                                           3
#define ADDRESS_MAX_WHEEL_SPEED                                             4
#define ADDRESS_UNITS_TYPE                                                  5
#define ADDRESS_HW_X10_OFFSET_0                                             6
#define ADDRESS_HW_X10_OFFSET_1                                             7
#define ADDRESS_HW_X10_OFFSET_2                                             8
#define ADDRESS_HW_X10_OFFSET_3                                             9
#define ADDRESS_HW_X10_100_PERCENT_OFFSET_0                                 10
#define ADDRESS_HW_X10_100_PERCENT_OFFSET_1                                 11
#define ADDRESS_HW_X10_100_PERCENT_OFFSET_2                                 12
#define ADDRESS_HW_X10_100_PERCENT_OFFSET_3                                 13
#define ADDRESS_SHOW_NUMERIC_BATTERY_SOC                                    14
#define ADDRESS_ODOMETER_FIELD_STATE                                        15
#define ADDRESS_BATTERY_MAX_CURRENT                                         16
#define ADDRESS_TARGET_MAX_BATTERY_POWER                                    17
#define ADDRESS_BATTERY_CELLS_NUMBER                                        18
#define ADDRESS_BATTERY_LOW_VOLTAGE_CUT_OFF_X10_0                           19
#define ADDRESS_BATTERY_LOW_VOLTAGE_CUT_OFF_X10_1                           20
#define ADDRESS_CONFIG_0                                                    21
#define ADDRESS_ASSIST_LEVEL_FACTOR_1                                       22
#define ADDRESS_ASSIST_LEVEL_FACTOR_2                                       23
#define ADDRESS_ASSIST_LEVEL_FACTOR_3                                       24
#define ADDRESS_ASSIST_LEVEL_FACTOR_4                                       25
#define ADDRESS_ASSIST_LEVEL_FACTOR_5                                       26
#define ADDRESS_ASSIST_LEVEL_FACTOR_6                                       27
#define ADDRESS_ASSIST_LEVEL_FACTOR_7                                       28
#define ADDRESS_ASSIST_LEVEL_FACTOR_8                                       29
#define ADDRESS_ASSIST_LEVEL_FACTOR_9                                       30
#define ADDRESS_NUMBER_OF_ASSIST_LEVELS                                     31
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED                   32
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_STATE                             33
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1                    34
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_2                    35
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_3                    36
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_4                    37
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_5                    38
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_6                    39
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_7                    40
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_8                    41
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_9                    42
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_TIME                              43
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_FADE_TIME                         44
#define ADDRESS_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT                           45
#define ADDRESS_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT                           46
#define ADDRESS_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10_0                      47
#define ADDRESS_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10_1                      48
#define ADDRESS_LCD_POWER_OFF_TIME                                          49
#define ADDRESS_LCD_BACKLIGHT_ON_BRIGHTNESS                                 50
#define ADDRESS_LCD_BACKLIGHT_OFF_BRIGHTNESS                                51
#define ADDRESS_BATTERY_PACK_RESISTANCE_0                                   52
#define ADDRESS_BATTERY_PACK_RESISTANCE_1                                   53
#define ADDRESS_OFFROAD_FEATURE_ENABLED                                     54
#define ADDRESS_OFFROAD_MODE_ENABLED_ON_STARTUP                             55
#define ADDRESS_OFFROAD_SPEED_LIMIT                                         56
#define ADDRESS_OFFROAD_POWER_LIMIT_ENABLED                                 57
#define ADDRESS_OFFROAD_POWER_LIMIT_DIV25                                   58
#define ADDRESS_ODOMETER_X10_0                                              59
#define ADDRESS_ODOMETER_X10_1                                              60
#define ADDRESS_ODOMETER_X10_2                                              61
#define ADDRESS_WALK_ASSIST_FEATURE_ENABLED                                 62
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_1                                  63
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_2                                  64
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_3                                  65
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_4                                  66
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_5                                  67
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_6                                  68
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_7                                  69
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_8                                  70
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_9                                  71
#define EEPROM_BYTES_STORED                                                 72 // CAN'T BE HIGHER THAN 255

void eeprom_init(void);
void eeprom_init_variables(void);
void eeprom_write_variables(void);
void eeprom_erase_key_value(void);

#endif /* _EEPROM_H_ */
