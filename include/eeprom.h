#ifndef INCLUDE_EEPROM_H_
#define INCLUDE_EEPROM_H_

#include "main.h"
#include "fstorage.h"

void eeprom_init(void);
//void eeprom_read_configuration(struct_configuration_variables *p_configuration_variables);
//void eeprom_write_configuration(const struct_configuration_variables *p_configuration_variables);
uint32_t eeprom_read_value(uint8_t address);
bool eeprom_write_value(uint8_t address, uint32_t* value);
fs_ret_t eeprom_get_last_result(void);


/* VARIABLE ADDRESSES */


#define KEY                                                                 0x1f

#define ADDRESS_ASSIST_LEVEL                                                0
#define ADDRESS_WHEEL_PERIMETER                                             1
#define ADDRESS_MAX_WHEEL_SPEED                                             2
#define ADDRESS_UNITS_TYPE                                                  3
#define ADDRESS_HW_X10_OFFSET                                               4
#define ADDRESS_HW_X10_100_PERCENT_OFFSET                                   5
#define ADDRESS_BATTERY_SOC_FUNCTION_ENABLED                                6
#define ADDRESS_ODOMETER_FIELD_STATE                                        7
#define ADDRESS_BATTERY_MAX_CURRENT                                         8
#define ADDRESS_TARGET_MAX_BATTERY_POWER                                    9
#define ADDRESS_BATTERY_CELLS_NUMBER                                        10
#define ADDRESS_BATTERY_LOW_VOLTAGE_CUT_OFF_X10                             11
#define ADDRESS_CONFIG_0                                                    12
#define ADDRESS_ASSIST_LEVEL_FACTOR_1                                       13
#define ADDRESS_ASSIST_LEVEL_FACTOR_2                                       14
#define ADDRESS_ASSIST_LEVEL_FACTOR_3                                       15
#define ADDRESS_ASSIST_LEVEL_FACTOR_4                                       16
#define ADDRESS_ASSIST_LEVEL_FACTOR_5                                       17
#define ADDRESS_ASSIST_LEVEL_FACTOR_6                                       18
#define ADDRESS_ASSIST_LEVEL_FACTOR_7                                       19
#define ADDRESS_ASSIST_LEVEL_FACTOR_8                                       20
#define ADDRESS_ASSIST_LEVEL_FACTOR_9                                       21
#define ADDRESS_NUMBER_OF_ASSIST_LEVELS                                     22
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED                   23
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_STATE                             24
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1                    25
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_2                    26
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_3                    27
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_4                    28
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_5                    29
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_6                    30
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_7                    31
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_8                    32
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_9                    33
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_TIME                              34
#define ADDRESS_STARTUP_MOTOR_POWER_BOOST_FADE_TIME                         35
#define ADDRESS_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT                           36
#define ADDRESS_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT                           37
#define ADDRESS_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10                        38
#define ADDRESS_LCD_POWER_OFF_TIME                                          39
#define ADDRESS_BATTERY_PACK_RESISTANCE                                     40
#define ADDRESS_OFFROAD_FEATURE_ENABLED                                     41
#define ADDRESS_OFFROAD_MODE_ENABLED_ON_STARTUP                             42
#define ADDRESS_OFFROAD_SPEED_LIMIT                                         43
#define ADDRESS_OFFROAD_POWER_LIMIT_ENABLED                                 44
#define ADDRESS_OFFROAD_POWER_LIMIT_DIV25                                   45
#define ADDRESS_ODOMETER_X10                                                46
#define ADDRESS_TRIP_X10                                                    47
#define ADDRESS_ODOMETER_SUB_FIELD_STATE_0                                  48
#define ADDRESS_ODOMETER_SUB_FIELD_STATE_1                                  49
#define ADDRESS_ODOMETER_SUB_FIELD_STATE_2                                  50
#define ADDRESS_ODOMETER_SUB_FIELD_STATE_3                                  51
#define ADDRESS_ODOMETER_SUB_FIELD_STATE_4                                  52
#define ADDRESS_ODOMETER_SUB_FIELD_STATE_5                                  53
#define ADDRESS_ODOMETER_SUB_FIELD_STATE_6                                  54
#define ADDRESS_MAX_WHEEL_SPEED_IMPERIAL                                    55
#define ADDRESS_TIME_MEASUREMENT_FIELD_STATE                                56
#define ADDRESS_TOTAL_SECOND_TTM                                            57
#define ADDRESS_TOTAL_MINUTE_TTM                                            58
#define ADDRESS_TOTAL_HOUR_TTM                                              59
#define ADDRESS_RAMP_UP_AMPS_PER_SECOND_X10                                 60
#define ADDRESS_WALK_ASSIST_FUNCTION_ENABLED                                61
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_0                                  62
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_1                                  63
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_2                                  64
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_3                                  65
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_4                                  66
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_5                                  67
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_6                                  68
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_7                                  69
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_8                                  70
#define ADDRESS_WALK_ASSIST_LEVEL_FACTOR_9                                  71
#define ADDRESS_CRUISE_FUNCTION_ENABLED                                     72
#define ADDRESS_CRUISE_FUNCTION_SET_TARGET_SPEED_ENABLED                    73
#define ADDRESS_CRUISE_FUNCTION_TARGET_SPEED_KPH                            74
#define ADDRESS_CRUISE_FUNCTION_TARGET_SPEED_MPH                            75
#define ADDRESS_SHOW_CRUISE_FUNCTION_SET_TARGET_SPEED                       76
#define ADDRESS_WHEEL_SPEED_FIELD_STATE                                     77
#define ADDRESS_SHOW_DISTANCE_DATA_ODOMETER_FIELD                           78
#define ADDRESS_SHOW_BATTERY_STATE_ODOMETER_FIELD                           79
#define ADDRESS_SHOW_PEDAL_DATA_ODOMETER_FIELD                              80
#define ADDRESS_SHOW_TIME_MEASUREMENT_ODOMETER_FIELD                        81
#define ADDRESS_SHOW_WHEEL_SPEED_ODOMETER_FIELD                             82
#define ADDRESS_SHOW_ENERGY_DATA_ODOMETER_FIELD                             83
#define ADDRESS_SHOW_MOTOR_TEMPERATURE_ODOMETER_FIELD                       84
#define ADDRESS_SHOW_BATTERY_SOC_ODOMETER_FIELD                             85
#define ADDRESS_MAIN_SCREEN_POWER_MENU_ENABLED                              86
/* Key address always at last */
#define ADDRESS_KEY                                                         87

#define EEPROM_WORDS_STORED                                                 88


/* DEFAULT PARAMETER */


// default values for assist levels
#define DEFAULT_VALUE_ASSIST_LEVEL                                  3
#define DEFAULT_VALUE_NUMBER_OF_ASSIST_LEVELS                       5

// default values for bike wheel parameters
#define DEFAULT_VALUE_WHEEL_PERIMETER                               2050  // 26 inch wheel: 2050 mm perimeter
#define DEFAULT_VALUE_WHEEL_MAX_SPEED                               50  // 50 kph
#define DEFAULT_VALUE_MAX_WHEEL_SPEED_IMPERIAL                      20  // 20 mph

// default value for system units
#define DEFAULT_VALUE_UNITS_TYPE                                    0   // 0 - metric, 1 - imperial

// default values for battery capacity variables
#define DEFAULT_VALUE_WH_OFFSET                                     0
#define DEFAULT_VALUE_HW_X10_100_PERCENT                            0
#define DEAFULT_VALUE_BATTERY_SOC_FUNCTION_ENABLED                  0

// default values for battery parameters
#define DEFAULT_VALUE_BATTERY_MAX_CURRENT                           16  // 16 amps
#define DEFAULT_VALUE_TARGET_MAX_BATTERY_POWER                      10  // e.g. 10 = 10 * 25 = 250, 0 is disabled
#define DEFAULT_VALUE_BATTERY_CELLS_NUMBER                          13  // 13 --> 48V
#define DEFAULT_VALUE_BATTERY_LOW_VOLTAGE_CUT_OFF_X10               390 // 48 V battery, LVC = 39.0
#define DEFAULT_VALUE_CONFIG_0                                      0   // motor type, assistance without pedal rotation, temperature limit enabled, temperature field state

// default values for assist levels
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_0                         0   // 0
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_1                         3   // 0.3
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_2                         6   // 0.6
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_3                         9
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_4                         12
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_5                         15
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_6                         18
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_7                         21
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_8                         24
#define DEFAULT_VALUE_ASSIST_LEVEL_FACTOR_9                         30

// default values for BOOST function
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FEATURE_ENABLED     0   // disabled by default
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_STATE               1
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_1      1
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_2      2
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_3      4
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_4      7   // 0.7
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_5      12  // 1.2
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_6      18
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_7      23
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_8      25
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_ASSIST_LEVEL_9      30
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_TIME                20  // 2.0 seconds
#define DEFAULT_VALUE_STARTUP_MOTOR_POWER_BOOST_FADE_TIME           35  // 3.5 seconds

// default values for motor temperature limit function
#define DEFAULT_VALUE_MOTOR_TEMPERATURE_MIN_VALUE_LIMIT             75  // 75 degrees celsius
#define DEFAULT_VALUE_MOTOR_TEMPERATURE_MAX_VALUE_LIMIT             85

// default values for battery voltage
#define DEFAULT_VALUE_BATTERY_VOLTAGE_RESET_WH_COUNTER_X10          542  // 48 V battery, 54.2 volts fully charged.

// default values for screen parameters
#define DEFAULT_VALUE_LCD_POWER_OFF_TIME                            15  // 15 minutes, each unit 1 minute
#define DEFAULT_VALUE_LCD_BACKLIGHT_ON_BRIGHTNESS                   16  // 16 = 80 %
#define DEFAULT_VALUE_LCD_BACKLIGHT_OFF_BRIGHTNESS                  1   // 1 = 5%

// default values for internal resistance of battery
#define DEFAULT_VALUE_BATTERY_PACK_RESISTANCE                       130 // 48 V battery, 13S5P measured 130 milli ohms

// default values for offroad function
#define DEFAULT_VALUE_OFFROAD_FEATURE_ENABLED                       0
#define DEFAULT_VALUE_OFFROAD_MODE_ENABLED_ON_STARTUP               0
#define DEFAULT_VALUE_OFFROAD_SPEED_LIMIT                           25
#define DEFAULT_VALUE_OFFROAD_POWER_LIMIT_ENABLED                   0
#define DEFAULT_VALUE_OFFROAD_POWER_LIMIT_DIV25                     10  // 10 * 25 = 250 W

// default values for distance measurement
#define DEFAULT_VALUE_ODOMETER_X10                                  0
#define DEFAULT_VALUE_TRIP_X10                                      0

// default values for the odometer field and sub field states
#define DEFAULT_VALUE_ODOMETER_FIELD_STATE                          0
#define DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_0                    0
#define DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_1                    0
#define DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_2                    0
#define DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_3                    0
#define DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_4                    0
#define DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_5                    0
#define DEFAULT_VALUE_ODOMETER_SUB_FIELD_STATE_6                    0

// default values for time measurement
#define DEFAULT_VALUE_TIME_MEASUREMENT_FIELD_STATE                  1   // 1 = display time measurement since power on (TM)
#define DEFAULT_VALUE_TOTAL_SECOND_TTM                              0
#define DEFAULT_VALUE_TOTAL_MINUTE_TTM                              0
#define DEFAULT_VALUE_TOTAL_HOUR_TTM                                0

// default values for ramp up
#define DEFAULT_VALUE_RAMP_UP_AMPS_PER_SECOND_X10                   50  // 5.0 amps per second ramp up

// default values for walk assist function
#define DEFAULT_VALUE_WALK_ASSIST_FUNCTION_ENABLED                  0   // disabled by default
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_0                    0   // target PWM for walk assist
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_1                    25
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_2                    30
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_3                    35
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_4                    40
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_5                    45
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_6                    50
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_7                    55
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_8                    60
#define DEFAULT_VALUE_WALK_ASSIST_LEVEL_FACTOR_9                    65

// default values for cruise function
#define DEFAULT_VALUE_CRUISE_FUNCTION_ENABLED                       0   // disabled by default
#define DEFAULT_VALUE_CRUISE_FUNCTION_SET_TARGET_SPEED_ENABLED      0   // disabled by default
#define DEFAULT_VALUE_CRUISE_FUNCTION_TARGET_SPEED_KPH              25  // 25 kph
#define DEFAULT_VALUE_CRUISE_FUNCTION_TARGET_SPEED_MPH              15  // 15 mph
#define DEFAULT_VALUE_SHOW_CRUISE_FUNCTION_SET_TARGET_SPEED         0   // disabled by default

// default values wheel speed field state
#define DEFAULT_VALUE_WHEEL_SPEED_FIELD_STATE                       0   // 0 = display wheel speed, 1 = display average wheel speed, 2 = display max measured wheel speed

// default values for showing odometer variables
#define DEFAULT_VALUE_SHOW_DISTANCE_DATA_ODOMETER_FIELD             1   // enabled by default
#define DEFAULT_VALUE_SHOW_BATTERY_STATE_ODOMETER_FIELD             1   // enabled by default
#define DEFAULT_VALUE_SHOW_PEDAL_DATA_ODOMETER_FIELD                1   // enabled by default
#define DEFAULT_VALUE_SHOW_TIME_MEASUREMENT_ODOMETER_FIELD          1   // enabled by default
#define DEFAULT_VALUE_SHOW_WHEEL_SPEED_ODOMETER_FIELD               1   // enabled by default
#define DEFAULT_VALUE_SHOW_ENERGY_DATA_ODOMETER_FIELD               1   // enabled by default
#define DEFAULT_VALUE_SHOW_MOTOR_TEMPERATURE_ODOMETER_FIELD         1   // enabled by default
#define DEFAULT_VALUE_SHOW_BATTERY_SOC_ODOMETER_FIELD               1   // enabled by default

// default value for the main screen power menu
#define DEFAULT_VALUE_MAIN_SCREEN_POWER_MENU_ENABLED                1   // enabled by default

// default value for the torque sensor force scale
#define TORQUE_SENSOR_FORCE_SCALE_X1000                             556 // see note below

/*---------------------------------------------------------
  NOTE: regarding TORQUE_SENSOR_FORCE_SCALE_X1000
  Torque sensor value found experimentaly.

  Measured with a cheap digital hook scale. We found that
  each torque sensor unit is equal to 0.556 Nm. Using the
  scale, it was found that each 1kg was measured as 3
  torque sensor units.
  Force (Nm) = Kg * 9.18 * 0.17 (arm cranks size)
---------------------------------------------------------*/

// default values for battery voltage ADC, see note below
#define ADC10BITS_BATTERY_VOLTAGE_PER_ADC_STEP_X512                 44
#define ADC10BITS_BATTERY_VOLTAGE_PER_ADC_STEP_X256                 (ADC10BITS_BATTERY_VOLTAGE_PER_ADC_STEP_X512 >> 1)
#define ADC8BITS_BATTERY_VOLTAGE_PER_ADC_STEP                       0.344

/*---------------------------------------------------------
  NOTE: regarding default values for battery voltage ADC
  0.344 per ADC_8bits step:
  17.9 V --> ADC_8bits = 52;
  40 V --> ADC_8bits = 116; this signal is atenuated by
  the opamp 358
---------------------------------------------------------*/

// default value battery current ADC, see note below
#define ADC_BATTERY_CURRENT_PER_ADC_STEP_X512                       102

/*---------------------------------------------------------
  NOTE: regarding default value battery current ADC
  1 A per 5 steps of ADC_10bits
---------------------------------------------------------*/

#endif /* INCLUDE_EEPROM_H_ */
