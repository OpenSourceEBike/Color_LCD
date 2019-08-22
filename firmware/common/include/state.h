#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  GRAPH_PEDAL_HUMAN_POWER = 0,
  GRAPH_PEDAL_CADENCE,
  GRAPH_WHEEL_SPEED,
  GRAPH_BATTERY_VOLTAGE,
  GRAPH_BATTERY_CURRENT,
  GRAPH_BATTERY_SOC,
  GRAPH_MOTOR_POWER,
  GRAPH_MOTOR_TEMPERATURE,
  GRAPH_MOTOR_PWM_DUTY_CYCLE,
  GRAPH_MOTOR_ERPS,
  GRAPH_MOTOR_FOC_ANGLE,
  NUMBER_OF_GRAPHS_ID,
} graphs_id_t;


typedef struct l2_vars_struct
{
  uint16_t ui16_adc_battery_voltage;
  uint8_t ui8_battery_current_x5;
  uint8_t ui8_adc_throttle;
  uint8_t ui8_throttle;
  uint8_t ui8_adc_pedal_torque_sensor;
  uint8_t ui8_pedal_torque_sensor;
  uint8_t ui8_pedal_human_power;
  uint8_t ui8_duty_cycle;
  uint8_t ui8_error_states;
  uint16_t ui16_wheel_speed_x10;
  uint8_t ui8_pedal_cadence;
  uint16_t ui16_motor_speed_erps;
  uint8_t ui8_foc_angle;
  uint8_t ui8_temperature_current_limiting_value;
  uint8_t ui8_motor_temperature;
  uint32_t ui32_wheel_speed_sensor_tick_counter;
  uint16_t ui16_pedal_torque_x10;
  uint16_t ui16_pedal_power_x10;
  uint16_t ui16_battery_voltage_filtered_x10;
  uint16_t ui16_battery_current_filtered_x5;
  uint16_t ui16_battery_power_filtered_x50;
  uint16_t ui16_battery_power_filtered;
  uint16_t ui16_pedal_torque_filtered;
  uint16_t ui16_pedal_power_filtered;
  uint8_t ui8_pedal_cadence_filtered;
  uint16_t ui16_battery_voltage_soc_x10;
  uint32_t ui32_wh_sum_x5;
  uint32_t ui32_wh_sum_counter;
  uint32_t ui32_wh_x10;

  uint8_t ui8_assist_level;
  uint8_t ui8_number_of_assist_levels;
  uint16_t ui16_wheel_perimeter;
  uint16_t ui16_wheel_perimeter_imperial_x10;
  uint8_t ui8_wheel_max_speed;
  uint8_t ui8_wheel_max_speed_imperial;
  uint8_t ui8_units_type;
  uint32_t ui32_wh_x10_offset;
  uint32_t ui32_wh_x10_100_percent;
  uint8_t ui8_battery_soc_enable;
  uint8_t ui8_battery_soc_increment_decrement;
  uint8_t ui8_target_max_battery_power;
  uint8_t ui8_battery_cells_number;
  uint8_t ui8_battery_max_current;
  uint8_t ui8_ramp_up_amps_per_second_x10;
  uint16_t ui16_battery_low_voltage_cut_off_x10;
  uint16_t ui16_battery_voltage_reset_wh_counter_x10;
  uint16_t ui16_battery_pack_resistance_x1000;
  uint8_t ui8_motor_type;
  uint8_t ui8_motor_assistance_startup_without_pedal_rotation;
  uint8_t ui8_assist_level_factor[10];
  uint8_t ui8_walk_assist_feature_enabled;
  uint8_t ui8_walk_assist_level_factor[10];
  uint8_t ui8_startup_motor_power_boost_feature_enabled;
  uint8_t ui8_startup_motor_power_boost_always;
  uint8_t ui8_startup_motor_power_boost_limit_power;
  uint8_t ui8_startup_motor_power_boost_time;
  uint8_t ui8_startup_motor_power_boost_fade_time;
  uint8_t ui8_startup_motor_power_boost_factor[10];
  uint8_t ui8_temperature_limit_feature_enabled;
  uint8_t ui8_motor_temperature_min_value_to_limit;
  uint8_t ui8_motor_temperature_min_value_to_limit_imperial;
  uint8_t ui8_motor_temperature_max_value_to_limit;
  uint8_t ui8_motor_temperature_max_value_to_limit_imperial;
  // uint8_t ui8_lcd_power_off_time_minutes;
  uint8_t ui8_lcd_backlight_on_brightness;
  uint8_t ui8_lcd_backlight_off_brightness;
  uint8_t ui8_offroad_feature_enabled;
  uint8_t ui8_offroad_enabled_on_startup;
  uint8_t ui8_offroad_speed_limit;
  uint8_t ui8_offroad_power_limit_enabled;
  uint8_t ui8_offroad_power_limit_div25;
  // uint16_t ui16_odometer_distance_x10;
  uint32_t ui32_odometer_x10;

  uint8_t ui8_lights;
  uint8_t ui8_braking;
  uint8_t ui8_walk_assist;
  uint8_t ui8_offroad_mode;
} l2_vars_t;

typedef struct l3_vars_struct
{
  uint16_t ui16_adc_battery_voltage;
  uint8_t ui8_battery_current_x5;
  uint8_t ui8_adc_throttle;
  uint8_t ui8_throttle;
  uint8_t ui8_adc_pedal_torque_sensor;
  uint8_t ui8_pedal_torque_sensor;
  uint8_t ui8_pedal_human_power;
  uint8_t ui8_duty_cycle;
  uint8_t ui8_error_states;
  uint16_t ui16_wheel_speed_x10;
  uint8_t ui8_pedal_cadence;
  uint16_t ui16_motor_speed_erps;
  uint8_t ui8_foc_angle;
  uint8_t ui8_temperature_current_limiting_value;
  uint8_t ui8_motor_temperature;
  uint32_t ui32_wheel_speed_sensor_tick_counter;
  uint32_t ui32_wheel_speed_sensor_tick_counter_offset;
  uint16_t ui16_pedal_torque_x10;
  uint16_t ui16_pedal_power_x10;
  uint16_t ui16_battery_voltage_filtered_x10;
  uint16_t ui16_battery_current_filtered_x5;
  uint16_t ui16_battery_power_filtered_x50;
  uint16_t ui16_battery_power_filtered;
  uint16_t ui16_pedal_torque_filtered;
  uint16_t ui16_pedal_power_filtered;
  uint8_t ui8_pedal_cadence_filtered;
  uint16_t ui16_battery_voltage_soc_x10;
  uint32_t ui32_wh_sum_x5;
  uint32_t ui32_wh_sum_counter;
  uint32_t ui32_wh_x10;

  uint8_t ui8_assist_level;
  uint8_t ui8_number_of_assist_levels;
  uint16_t ui16_wheel_perimeter;
  uint16_t ui16_wheel_perimeter_imperial_x10;
  uint8_t ui8_wheel_max_speed;
  uint8_t ui8_wheel_max_speed_imperial;
  uint8_t ui8_units_type;
  uint32_t ui32_wh_x10_offset;
  uint32_t ui32_wh_x10_100_percent;
  uint8_t ui8_battery_soc_enable;
  uint8_t ui8_battery_soc_increment_decrement;
  uint8_t ui8_target_max_battery_power;
  uint8_t ui8_battery_cells_number;
  uint8_t ui8_battery_max_current;
  uint8_t ui8_ramp_up_amps_per_second_x10;
  uint16_t ui16_battery_low_voltage_cut_off_x10;
  uint16_t ui16_battery_voltage_reset_wh_counter_x10;
  uint16_t ui16_battery_pack_resistance_x1000;
  uint8_t ui8_motor_type;
  uint8_t ui8_motor_assistance_startup_without_pedal_rotation;
  uint8_t ui8_assist_level_factor[9];
  uint8_t ui8_walk_assist_feature_enabled;
  uint8_t ui8_walk_assist_level_factor[9];
  uint8_t ui8_startup_motor_power_boost_feature_enabled;
  uint8_t ui8_startup_motor_power_boost_always;
  uint8_t ui8_startup_motor_power_boost_limit_power;
  uint8_t ui8_startup_motor_power_boost_time;
  uint8_t ui8_startup_motor_power_boost_fade_time;
  uint8_t ui8_startup_motor_power_boost_factor[9];
  uint8_t ui8_temperature_limit_feature_enabled;
  uint8_t ui8_motor_temperature_min_value_to_limit;
  uint8_t ui8_motor_temperature_min_value_to_limit_imperial;
  uint8_t ui8_motor_temperature_max_value_to_limit;
  uint8_t ui8_motor_temperature_max_value_to_limit_imperial;
  uint8_t ui8_lcd_power_off_time_minutes;
  uint8_t ui8_lcd_backlight_on_brightness;
  uint8_t ui8_lcd_backlight_off_brightness;
  uint8_t ui8_offroad_feature_enabled;
  uint8_t ui8_offroad_enabled_on_startup;
  uint8_t ui8_offroad_speed_limit;
  uint8_t ui8_offroad_power_limit_enabled;
  uint8_t ui8_offroad_power_limit_div25;
  // uint16_t ui16_odometer_distance_x10;
  uint32_t ui32_odometer_x10;
  // uint16_t ui16_distance_since_power_on_x10;
  uint32_t ui32_trip_x10;

  uint8_t ui8_lights;
  uint8_t ui8_braking;
  uint8_t ui8_walk_assist;
  uint8_t ui8_offroad_mode;
  graphs_id_t graph_id;
  uint8_t ui8_buttons_up_down_invert;

  uint8_t volt_based_soc; // a SOC generated only based on pack voltage
} l3_vars_t;

// deprecated FIXME, delete
l3_vars_t* get_l3_vars(void);

extern volatile l2_vars_t l2_vars; // FIXME - this shouldn't be exposed outside of state.c - but currently mid merge
extern l3_vars_t l3_vars;

void layer_2(void);

/**
 * Called from the main thread every 100ms
 *
 */
void copy_layer_2_layer_3_vars(void);

/// must be called from main() idle loop
void automatic_power_off_management(void);

void lcd_power_off(uint8_t updateDistanceOdo); // provided by LCD

/// Set correct backlight brightness for current headlight state
void set_lcd_backlight();

extern uint16_t ui16_m_battery_soc_watts_hour;
extern uint16_t ui16_m_battery_soc_watts_hour_fixed;
extern volatile uint32_t ui32_g_layer_2_can_execute;

extern bool has_seen_motor; // true once we've received a packet from a real motor
extern bool is_sim_motor; // true if we are simulating a motor (and therefore not talking on serial at all)


