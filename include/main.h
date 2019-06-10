#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_

#include "common.h"

/* Typedef of unions for handy access of single bytes */
/* Access bytewise: U16 var; var.byte[x] = z; */
/* Access value: U32 var; var.u32 = 0xFFFFFFFF; */
typedef union
{
  uint16_t u16;
  uint8_t byte[2];
} U16;

typedef union
{
  uint32_t u32;
  uint8_t byte[4];
} U32;

typedef struct
{
  uint8_t master_comm_package_id;
  uint8_t slave_comm_package_id;
  U16 adc_battery_voltage;
  uint8_t battery_current_x5;
  uint8_t motor_controller_state_1;
  uint8_t motor_controller_state_2;
  uint8_t adc_throttle;
  uint8_t throttle;
  uint8_t adc_pedal_torque_sensor;
  uint8_t pedal_torque_sensor;
  uint8_t pedal_human_power;
  uint8_t duty_cycle;
  uint8_t error_states;
  U16 wheel_speed_x10;
  uint8_t braking;
  uint8_t pedal_cadence;
  uint8_t lights;
  uint8_t walk_assist_level;
  uint8_t offroad_mode;
  U16 motor_speed_erps;
  uint8_t foc_angle;
  uint8_t temperature_current_limiting_value;
  uint8_t motor_temperature;
  U32 wheel_speed_sensor_tick_counter;
  U32 wheel_speed_sensor_tick_counter_offset;
  U16 pedal_torque_x10;
  U16 pedal_power_x10;
} struct_motor_controller_data;

typedef struct
{
  uint8_t assist_level;
  uint8_t number_of_assist_levels;
  uint16_t wheel_perimeter;
  uint8_t wheel_max_speed;
  uint8_t wheel_max_speed_imperial;
  uint8_t units_type;
  uint32_t wh_x10_offset;
  uint32_t wh_x10_100_percent;
  uint8_t battery_SOC_function_enabled;
  uint8_t odometer_field_state;
  uint8_t time_measurement_field_state;
  uint8_t total_second_TTM;
  uint16_t total_minute_TTM;
  uint16_t total_hour_TTM;
  uint8_t odometer_sub_field_state_0;
  uint8_t odometer_sub_field_state_1;
  uint8_t odometer_sub_field_state_2;
  uint8_t odometer_sub_field_state_3;
  uint8_t odometer_sub_field_state_4;
  uint8_t odometer_sub_field_state_5;
  uint8_t odometer_sub_field_state_6;
  uint8_t odometer_show_field_number;
  uint8_t target_max_battery_power_div25;
  uint8_t battery_cells_number;
  uint8_t battery_max_current;
  uint16_t battery_low_voltage_cut_off_x10;
  uint16_t battery_voltage_reset_wh_counter_x10;
  uint16_t battery_pack_resistance_x1000;
  uint8_t motor_type;
  uint8_t motor_assistance_startup_without_pedal_rotation;
  uint8_t assist_level_factor[9];
  uint8_t startup_motor_power_boost_feature_enabled;
  uint8_t startup_motor_power_boost_state;
  uint8_t startup_motor_power_boost_time;
  uint8_t startup_motor_power_boost_fade_time;
  uint8_t startup_motor_power_boost_factor[9];
  uint16_t adc_motor_temperature_10b;
  uint8_t temperature_limit_feature_enabled;
  uint8_t motor_temperature_min_value_to_limit;
  uint8_t motor_temperature_max_value_to_limit;
  uint8_t temperature_field_state;
  uint8_t lcd_power_off_time_minutes;
  uint8_t offroad_feature_enabled;
  uint8_t offroad_enabled_on_startup;
  uint8_t offroad_speed_limit;
  uint8_t offroad_power_limit_enabled;
  uint8_t offroad_power_limit_div25;
  uint16_t distance_since_power_on_x10;
  uint32_t odometer_x10;
  uint32_t trip_x10;
  uint8_t ramp_up_amps_per_second_x10;
  uint8_t walk_assist_function_enabled;
  uint8_t walk_assist_level_factor[10];
  uint8_t cruise_function_enabled;
  uint8_t cruise_function_set_target_speed_enabled;
  uint8_t cruise_function_target_speed_kph;
  uint8_t cruise_function_target_speed_mph;
  uint8_t show_cruise_function_set_target_speed;
  uint8_t wheel_speed_field_state;
  uint8_t show_distance_data_odometer_field;
  uint8_t show_battery_state_odometer_field;
  uint8_t show_pedal_data_odometer_field;
  uint8_t show_time_measurement_odometer_field;
  uint8_t show_wheel_speed_odometer_field;
  uint8_t show_energy_data_odometer_field;
  uint8_t show_motor_temperature_odometer_field;
  uint8_t show_battery_SOC_odometer_field;
  uint8_t main_screen_power_menu_enabled;
} struct_configuration_variables;

struct_configuration_variables* get_configuration_variables(void);
struct_motor_controller_data* get_motor_controller_data(void);

#endif /* INCLUDE_MAIN_H_ */
