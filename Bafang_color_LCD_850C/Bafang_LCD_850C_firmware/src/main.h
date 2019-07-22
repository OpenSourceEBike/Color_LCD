/*
 * LCD3 firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _MAIN_H_
#define _MAIN_H_

// INTERRUPTS PRIORITIES
// Define for the NVIC IRQChannel Preemption Priority
// lower number has higher priority
#define USART1_INTERRUPT_PRIORITY       3
#define USART1_DMA_INTERRUPT_PRIORITY   4
#define TIM4_INTERRUPT_PRIORITY         5
#define RTC_INTERRUT_PRIORITY           6

#define  SMALL_TEXT_FONT              FONT_10X16
#define  REGULAR_TEXT_FONT            FONT_16X26

#define  TITLE_TEXT_FONT              FONT_16X26
#define  MEDIUM_NUMBERS_TEXT_FONT     FONT_24X40
#define  BIG_NUMBERS_TEXT_FONT        FONT_32X53

#define  CONFIGURATIONS_TEXT_FONT     FONT_12X20


#define  MAIN_SCREEN_FIELD_LABELS_COLOR C_DARK_GRAY

// *************************************************************************** //
// EEPROM memory variables default values
#define DEFAULT_VALUE_ASSIST_LEVEL                                  3
#define DEFAULT_VALUE_NUMBER_OF_ASSIST_LEVELS                       5
#define DEFAULT_VALUE_WHEEL_PERIMETER                               2050 // 26'' wheel: 2050mm perimeter
#define DEFAULT_VALUE_WHEEL_MAX_SPEED                               50
#define DEFAULT_VALUE_UNITS_TYPE                                    0 // 0 = km/h
#define DEFAULT_VALUE_WH_X10_OFFSET                                 0
#define DEFAULT_VALUE_HW_X10_100_PERCENT                            0
#define DEAFULT_VALUE_SHOW_NUMERIC_BATTERY_SOC                      0
#define DEFAULT_VALUE_BATTERY_MAX_CURRENT                           16 // 16 amps
#define DEFAULT_VALUE_RAMP_UP_AMPS_PER_SECOND_X10                   50 // 5.0 amps per second ramp up
#define DEFAULT_VALUE_TARGET_MAX_BATTERY_POWER                      0 // e.g. 20 = 20 * 25 = 500, 0 is disabled
#define DEFAULT_VALUE_BATTERY_CELLS_NUMBER                          13 // 13 --> 48V
#define DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10               390 // 48v battery, LVC = 39.0 (3.0 * 13)
#define DEFAULT_VALUE_MOTOR_TYPE                                    0 // ui8_motor_type = 0 = 48V
#define DEFAULT_VALUE_MOTOR_ASSISTANCE_WITHOUT_PEDAL_ROTATION       0 // 0 to keep this feature disable
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_1                         2 // 0.2
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_2                         3
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_3                         4
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_4                         6
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_5                         9
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_6                         14
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_7                         21
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_8                         32
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_9                         48
#define DEFAULT_VALUE_WALK_ASSIST_FEATURE_ENABLED                   1
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_1                    5
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_2                    6
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_3                    8
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_4                    10
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_5                    12
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_6                    14
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_7                    16
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_8                    18
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_9                    20
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED     0
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_STATE               1
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1      4
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_2      7
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_3      10
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_4      13
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_5      16
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_6      19
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_7      22
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_8      25
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_9      28
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_TIME                20 // 2.0 seconds
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FADE_TIME           35 // 3.5 seconds
#define DEFAULT_VALUE_MOTOR_TEMPERATURE_FEATURE_ENABLE              0
#define DEFAULT_VALUE_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT             75 // 75 degrees celsius
#define DEFAULT_VALUE_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT             85
#define DEFAULT_VALUE_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10          542 // 48v battery, 54.2 volts fully charged
#define DEFAULT_VALUE_LCD_POWER_OFF_TIME                            15 // 15 minutes, each unit 1 minute
#define DEFAULT_VALUE_LCD_BACKLIGHT_ON_BRIGHTNESS                   8 // 8 = 40%
#define DEFAULT_VALUE_LCD_BACKLIGHT_OFF_BRIGHTNESS                  20 // 20 = 100%
#define DEFAULT_VALUE_BATTERY_PACK_RESISTANCE                       130 // 48v battery, 13S5P measured 130 milli ohms
#define DEFAULT_VALUE_OFFROAD_FEATURE_ENABLED                       0
#define DEFAULT_VALUE_OFFROAD_MODE_ENABLED_ON_STARTUP               0
#define DEFAULT_VALUE_OFFROAD_SPEED_LIMIT                           25
#define DEFAULT_VALUE_OFFROAD_POWER_LIMIT_ENABLED                   0
#define DEFAULT_VALUE_OFFROAD_POWER_LIMIT_DIV25                     10 //10 * 25 = 250W
#define DEFAULT_VALUE_ODOMETER_X10                                  0

// *************************************************************************** //

// Torque sensor value found experimentaly
// measuring with a cheap digital hook scale, we found that each torque sensor unit is equal to 0.556 Nm
// using the scale, was found that each 1kg was measured as 3 torque sensor units
// Force (Nm) = Kg * 9.18 * 0.17 (arm cranks size)
#define TORQUE_SENSOR_FORCE_SCALE_X1000 556

// *************************************************************************** //
// BATTERY

// ADC Battery voltage
// 0.344 per ADC_8bits step: 17.9V --> ADC_8bits = 52; 40V --> ADC_8bits = 116; this signal atenuated by the opamp 358
#define ADC10BITS_BATTERY_VOLTAGE_PER_ADC_STEP_X512 44
#define ADC10BITS_BATTERY_VOLTAGE_PER_ADC_STEP_X256 (ADC10BITS_BATTERY_VOLTAGE_PER_ADC_STEP_X512 >> 1)
#define ADC8BITS_BATTERY_VOLTAGE_PER_ADC_STEP 0.344

// ADC Battery current
// 1A per 5 steps of ADC_10bits
#define ADC_BATTERY_CURRENT_PER_ADC_STEP_X512 102
// *************************************************************************** //

#endif // _MAIN_H_
