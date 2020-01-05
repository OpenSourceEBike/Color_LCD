/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include <math.h>
#include "stdio.h"
#include "main.h"
#include "utils.h"
#include "screen.h"
#include "rtc.h"
#include "fonts.h"
#include "uart.h"
#include "mainscreen.h"
#include "eeprom.h"
#include "buttons.h"
#include "fault.h"
#include "state.h"
#include <stdlib.h>

static uint8_t ui8_m_usart1_received_first_package = 0;
uint16_t ui16_g_battery_soc_watts_hour;
volatile uint8_t motorVariablesStabilized = 0;

bool g_has_seen_motor; // true once we've received a packet from a real motor
bool g_is_sim_motor; // true if we are simulating a motor (and therefore not talking on serial at all)

volatile uint8_t m_get_tsdz2_firmware_version; // true if we are simulating a motor (and therefore not talking on serial at all)
volatile communications_state_t g_communications_state = COMMUNICATIONS_READY;

tsdz2_firmware_version_t g_tsdz2_firmware_version = { 0xff, 0, 0 };

bool g_tsdz2_configurations_set = false;

// kevinh: I don't think volatile is probably needed here
rt_vars_t rt_vars;

ui_vars_t ui_vars;

ui_vars_t* get_ui_vars(void) {
	return &ui_vars;
}

rt_vars_t* get_rt_vars(void) {
  return &rt_vars;
}

/// Set correct backlight brightness for current headlight state
void set_lcd_backlight() {
	lcd_set_backlight_intensity(
			ui_vars.ui8_lights ?
					ui_vars.ui8_lcd_backlight_on_brightness :
					ui_vars.ui8_lcd_backlight_off_brightness);
}

static uint16_t fake(uint16_t minv, uint16_t maxv) {
	static uint16_t seed = 1; // Just generate some slightly increasing data, scaled to fit the required range

	uint16_t numval = maxv - minv + 1;

	return (seed++ % numval) + minv;
}

/// Generate a fake value that slowly loops between min and max and then back to min.  You must provide static storage for this routine to use
static uint16_t fakeWave(uint32_t *storage, uint16_t minv, uint16_t maxv) {
	(*storage)++;

	uint16_t numval = maxv - minv + 1;

	return (*storage % numval) + minv;
}

/// Generate a fake value that randomly oscillates between min and max and then back to min.  You must provide static storage for this routine to use
static uint16_t fakeRandom(uint32_t *storage, uint16_t minv, uint16_t maxv) {
    int32_t rnd = (rand() - RAND_MAX / 2) % ((maxv - minv) / 20);
    (*storage) += rnd;
    if (*storage > maxv) {
        *storage = (uint32_t)maxv;
    }
    if (*storage < minv) {
        *storage = (uint32_t)minv;
    }
    return *storage;
}

/**
 * Pretend we just received a randomized motor packet
 */
void parse_simmotor() {
  static uint32_t counter;

  // execute at a slow rate so values can be seen on the graph
  counter++;
  if (counter % (3 * 10)) // 3 seconds
    return;

	const uint32_t min_bat10x = 400;
	const uint32_t max_bat10x = 546;
	const uint32_t max_cur10x = 140;
  static uint32_t voltstore, curstore, speedstore, cadencestore, tempstore, diststore;

	// per step of ADC ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000
	// l2_vars.ui16_adc_battery_voltage = battery_voltage_10x_get() * 1000L / ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000;
	rt_vars.ui16_adc_battery_voltage = fakeWave(&voltstore, min_bat10x,
			max_bat10x) * 1000L / ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000;
	// l2_vars.ui16_adc_battery_voltage = max_bat10x * 1000L / ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000;

	// battery current drain x5
	rt_vars.ui8_battery_current_x5 = fakeRandom(&curstore, 0, max_cur10x) / 2;

	rt_vars.ui16_wheel_speed_x10 = fakeRandom(&speedstore, 80, 300);
    diststore += rt_vars.ui16_wheel_speed_x10 * 2.6; // speed x 10 to millimeters per 100 ms
//	l2_vars.ui16_wheel_speed_x10 = 200; // for testing, just leave speed fixed

	rt_vars.ui8_braking = 0; // fake(0, 1);

	rt_vars.ui8_adc_throttle = fake(0, 100);

	if (rt_vars.ui8_temperature_limit_feature_enabled) {
		rt_vars.ui8_motor_temperature = fakeWave(&tempstore, 20, 120);
	} else {
		rt_vars.ui8_throttle = fake(0, 100);
	}

	rt_vars.ui16_adc_pedal_torque_sensor = fake(0, 1023);

	rt_vars.ui8_pedal_weight = fake(0, 100);

	rt_vars.ui8_pedal_cadence = fakeRandom(&cadencestore, 0, 93);

	rt_vars.ui8_duty_cycle = fake(0, 100);

	rt_vars.ui16_motor_speed_erps = fake(0, 600);

	rt_vars.ui8_foc_angle = fake(0, 100);

	// error states
	rt_vars.ui8_error_states = NO_ERROR; // fake(0, ERROR_MAX);

	// temperature actual limiting value
	rt_vars.ui8_temperature_current_limiting_value = fake(0, 100);

	// wheel_speed_sensor_tick_counter

    if (diststore > rt_vars.ui16_wheel_perimeter) {
        rt_vars.ui32_wheel_speed_sensor_tick_counter += 1;
        diststore -= rt_vars.ui16_wheel_perimeter;
    }
}

void rt_send_tx_package(uint8_t type) {
  uint8_t crc_len = 3; // minimun is 3
	uint8_t *ui8_usart1_tx_buffer = uart_get_tx_buffer();

	/************************************************************************************************/
	// send tx package
	// start up byte
	ui8_usart1_tx_buffer[0] = 0x59;
  ui8_usart1_tx_buffer[1] = crc_len;
  // type of the frame
	ui8_usart1_tx_buffer[2] = type;

	switch (type) {
	  case 0:
      if (rt_vars.ui8_walk_assist) {
        ui8_usart1_tx_buffer[3] = (uint8_t) rt_vars.ui8_walk_assist_level_factor[((rt_vars.ui8_assist_level) - 1)];
      } else if (rt_vars.ui8_assist_level) {
        ui8_usart1_tx_buffer[3] = (uint8_t) rt_vars.ui8_assist_level_factor[((rt_vars.ui8_assist_level) - 1)];
      } else {
        // if rt_vars.ui8_assist_level = 0, send 0!! always disable motor when assist level is 0
        ui8_usart1_tx_buffer[3] = 0;
      }

      ui8_usart1_tx_buffer[4] = (rt_vars.ui8_lights & 1) | ((rt_vars.ui8_walk_assist & 1) << 1);
      ui8_usart1_tx_buffer[5] = rt_vars.ui8_target_max_battery_power;

      crc_len = 6;
      ui8_usart1_tx_buffer[1] = crc_len;
	    break;

    // set configurations
	  case 1:
      // battery low voltage cut-off
      ui8_usart1_tx_buffer[3] = (uint8_t) (rt_vars.ui16_battery_low_voltage_cut_off_x10 & 0xff);
      ui8_usart1_tx_buffer[4] = (uint8_t) (rt_vars.ui16_battery_low_voltage_cut_off_x10 >> 8);

      // wheel perimeter
      ui8_usart1_tx_buffer[5] = (uint8_t) (rt_vars.ui16_wheel_perimeter & 0xff);
      ui8_usart1_tx_buffer[6] = (uint8_t) (rt_vars.ui16_wheel_perimeter >> 8);

      // wheel max speed
      ui8_usart1_tx_buffer[7] = rt_vars.ui8_wheel_max_speed;

      // battery max current
      ui8_usart1_tx_buffer[8] = rt_vars.ui8_battery_max_current;

      ui8_usart1_tx_buffer[9] = (rt_vars.ui8_startup_motor_power_boost_feature_enabled ? 1 : 0) |
          (rt_vars.ui8_startup_motor_power_boost_always ? 2 : 0) |
          (rt_vars.ui8_startup_motor_power_boost_limit_power ? 4 : 0) |
          (rt_vars.ui8_torque_sensor_calibration_feature_enabled ? 8 : 0) |
          (rt_vars.ui8_torque_sensor_calibration_pedal_ground ? 16 : 0) |
          (rt_vars.ui8_motor_assistance_startup_without_pedal_rotation ? 32 : 0) |
          (rt_vars.ui8_motor_type ? 64 : 0);

      // startup motor power boost
      ui8_usart1_tx_buffer[10] = rt_vars.ui8_startup_motor_power_boost_factor[((rt_vars.ui8_assist_level) - 1)];
      // startup motor power boost time
      ui8_usart1_tx_buffer[11] = rt_vars.ui8_startup_motor_power_boost_time;
      // startup motor power boost fade time
      ui8_usart1_tx_buffer[12] = rt_vars.ui8_startup_motor_power_boost_fade_time;

      // motor over temperature min and max values to limit
      ui8_usart1_tx_buffer[13] = rt_vars.ui8_motor_temperature_min_value_to_limit;
      ui8_usart1_tx_buffer[14] = rt_vars.ui8_motor_temperature_max_value_to_limit;

      ui8_usart1_tx_buffer[15] = rt_vars.ui8_ramp_up_amps_per_second_x10;

      // TODO
      // target speed for cruise
      ui8_usart1_tx_buffer[16] = 0;

      // motor temperature limit function or throttle
      ui8_usart1_tx_buffer[17] = rt_vars.ui8_temperature_limit_feature_enabled & 3;

      // torque sensor calibration tables
      uint8_t j = 18;
      for (uint8_t i = 0; i < 8; i++) {
        ui8_usart1_tx_buffer[j++] = (uint8_t) rt_vars.ui16_torque_sensor_calibration_table_left[i][0];
        ui8_usart1_tx_buffer[j++] = (uint8_t) (rt_vars.ui16_torque_sensor_calibration_table_left[i][0] >> 8);
        ui8_usart1_tx_buffer[j++] = (uint8_t) rt_vars.ui16_torque_sensor_calibration_table_left[i][1];
        ui8_usart1_tx_buffer[j++] = (uint8_t) (rt_vars.ui16_torque_sensor_calibration_table_left[i][1] >> 8);
      }

      for (uint8_t i = 0; i < 8; i++) {
        ui8_usart1_tx_buffer[j++] = (uint8_t) rt_vars.ui16_torque_sensor_calibration_table_right[i][0];
        ui8_usart1_tx_buffer[j++] = (uint8_t) (rt_vars.ui16_torque_sensor_calibration_table_right[i][0] >> 8);
        ui8_usart1_tx_buffer[j++] = (uint8_t) rt_vars.ui16_torque_sensor_calibration_table_right[i][1];
        ui8_usart1_tx_buffer[j++] = (uint8_t) (rt_vars.ui16_torque_sensor_calibration_table_right[i][1] >> 8);
      }

      // battery current min ADC
      ui8_usart1_tx_buffer[81] = rt_vars.ui8_battery_current_min_adc;

      crc_len = 83;
      ui8_usart1_tx_buffer[1] = crc_len;
	    break;
	}

	// prepare crc of the package
	uint16_t ui16_crc_tx = 0xffff;
	for (uint8_t ui8_i = 0; ui8_i < crc_len; ui8_i++) {
		crc16(ui8_usart1_tx_buffer[ui8_i], &ui16_crc_tx);
	}
	ui8_usart1_tx_buffer[crc_len] =
			(uint8_t) (ui16_crc_tx & 0xff);
	ui8_usart1_tx_buffer[crc_len + 1] =
			(uint8_t) (ui16_crc_tx >> 8) & 0xff;

	// send the full package to UART
	if (!g_is_sim_motor) // If we are simulating received packets never send real packets
		uart_send_tx_buffer(ui8_usart1_tx_buffer, ui8_usart1_tx_buffer[1] + 2);
}

void rt_low_pass_filter_battery_voltage_current_power(void) {
	static uint32_t ui32_battery_voltage_accumulated_x10000 = 0;
	static uint16_t ui16_battery_current_accumulated_x5 = 0;

	// low pass filter battery voltage
	ui32_battery_voltage_accumulated_x10000 -=
			ui32_battery_voltage_accumulated_x10000
					>> BATTERY_VOLTAGE_FILTER_COEFFICIENT;
	ui32_battery_voltage_accumulated_x10000 +=
			(uint32_t) rt_vars.ui16_adc_battery_voltage
					* ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000;
	rt_vars.ui16_battery_voltage_filtered_x10 =
			((uint32_t) (ui32_battery_voltage_accumulated_x10000
					>> BATTERY_VOLTAGE_FILTER_COEFFICIENT)) / 1000;

	// low pass filter batery current
	ui16_battery_current_accumulated_x5 -= ui16_battery_current_accumulated_x5
			>> BATTERY_CURRENT_FILTER_COEFFICIENT;
	ui16_battery_current_accumulated_x5 +=
			(uint16_t) rt_vars.ui8_battery_current_x5;
	rt_vars.ui16_battery_current_filtered_x5 =
			ui16_battery_current_accumulated_x5
					>> BATTERY_CURRENT_FILTER_COEFFICIENT;

	// battery power
	rt_vars.ui16_battery_power_filtered_x50 =
			rt_vars.ui16_battery_current_filtered_x5
					* rt_vars.ui16_battery_voltage_filtered_x10;
	rt_vars.ui16_battery_power_filtered =
			rt_vars.ui16_battery_power_filtered_x50 / 50;
}

void rt_low_pass_filter_pedal_power(void) {
	static uint32_t ui32_pedal_power_accumulated = 0;

	// low pass filter
	ui32_pedal_power_accumulated -= ui32_pedal_power_accumulated
			>> PEDAL_POWER_FILTER_COEFFICIENT;
	ui32_pedal_power_accumulated += (uint32_t) rt_vars.ui16_pedal_power_x10
			/ 10;
	rt_vars.ui16_pedal_power_filtered =
			((uint32_t) (ui32_pedal_power_accumulated
					>> PEDAL_POWER_FILTER_COEFFICIENT));
}

void rt_calc_battery_voltage_soc(void) {
	uint16_t ui16_fluctuate_battery_voltage_x10;

	// calculate flutuate voltage, that depends on the current and battery pack resistance
	ui16_fluctuate_battery_voltage_x10 =
			(uint16_t) ((((uint32_t) rt_vars.ui16_battery_pack_resistance_x1000)
					* ((uint32_t) rt_vars.ui16_battery_current_filtered_x5))
					/ ((uint32_t) 500));
	// now add fluctuate voltage value
	rt_vars.ui16_battery_voltage_soc_x10 =
			rt_vars.ui16_battery_voltage_filtered_x10
					+ ui16_fluctuate_battery_voltage_x10;
}

void rt_calc_wh(void) {
	static uint8_t ui8_1s_timer_counter = 0;
	uint32_t ui32_temp = 0;

	if (rt_vars.ui16_battery_power_filtered_x50 > 0) {
		rt_vars.ui32_wh_sum_x5 += rt_vars.ui16_battery_power_filtered_x50 / 10;
		rt_vars.ui32_wh_sum_counter++;
	}

	// calc at 1s rate
	if (++ui8_1s_timer_counter >= 10) {
		ui8_1s_timer_counter = 0;

		// avoid zero divisison
		if (rt_vars.ui32_wh_sum_counter != 0) {
			ui32_temp = rt_vars.ui32_wh_sum_counter / 36;
			ui32_temp = (ui32_temp
					* (rt_vars.ui32_wh_sum_x5 / rt_vars.ui32_wh_sum_counter))
					/ 500;
		}

		rt_vars.ui32_wh_x10 = rt_vars.ui32_wh_x10_offset + ui32_temp;
	}
}

static void rt_calc_odometer(void) {
  static uint8_t ui8_1s_timer_counter;
	uint32_t uint32_temp;

	// calc at 1s rate
	if (++ui8_1s_timer_counter >= 10) {
		ui8_1s_timer_counter = 0;

		uint32_temp = (rt_vars.ui32_wheel_speed_sensor_tick_counter
				- ui_vars.ui32_wheel_speed_sensor_tick_counter_offset)
				* ((uint32_t) rt_vars.ui16_wheel_perimeter);
		// avoid division by 0
		if (uint32_temp > 100000) {
			uint32_temp /= 100000;
		}  // milimmeters to 0.1kms
		else {
			uint32_temp = 0;
		}

		// now store the value on the global variable
		// l2_vars.ui16_odometer_distance_x10 = (uint16_t) uint32_temp;

		// calculate how many revolutions since last reset and convert to distance traveled
		uint32_t ui32_temp = (rt_vars.ui32_wheel_speed_sensor_tick_counter
				- rt_vars.ui32_wheel_speed_sensor_tick_counter_offset)
				* ((uint32_t) rt_vars.ui16_wheel_perimeter);

		// if traveled distance is more than 100 meters update all distance variables and reset
		if (ui32_temp >= 100000) { // 100000 -> 100000 mm -> 0.1 km
			// update all distance variables
			// ui_vars.ui16_distance_since_power_on_x10 += 1;
			rt_vars.ui32_odometer_x10 += 1;
			rt_vars.ui32_trip_x10 += 1;

			// reset the always incrementing value (up to motor controller power reset) by setting the offset to current value
			rt_vars.ui32_wheel_speed_sensor_tick_counter_offset =
					rt_vars.ui32_wheel_speed_sensor_tick_counter;
		}
	}
}

static void rt_low_pass_filter_pedal_cadence(void) {
	static uint16_t ui16_pedal_cadence_accumulated = 0;

	// low pass filter
	ui16_pedal_cadence_accumulated -= (ui16_pedal_cadence_accumulated
			>> PEDAL_CADENCE_FILTER_COEFFICIENT);
	ui16_pedal_cadence_accumulated += (uint16_t) rt_vars.ui8_pedal_cadence;

	// consider the filtered value only for medium and high values of the unfiltered value
	if (rt_vars.ui8_pedal_cadence > 20) {
		rt_vars.ui8_pedal_cadence_filtered =
				(uint8_t) (ui16_pedal_cadence_accumulated
						>> PEDAL_CADENCE_FILTER_COEFFICIENT);
	} else {
		rt_vars.ui8_pedal_cadence_filtered = rt_vars.ui8_pedal_cadence;
	}
}

uint8_t rt_first_time_management(void) {
  static uint32_t ui32_counter = 0;
	static uint8_t ui8_motor_controller_init = 1;
	uint8_t ui8_status = 0;

  // wait 5 seconds to help motor variables data stabilize
  if (motorVariablesStabilized == 0)
    if (++ui32_counter > 50) {
      motorVariablesStabilized = 1;
#ifndef SW102
      extern Field *activeGraphs; // FIXME, move this extern someplace better, placing here for review purposes

  	  activeGraphs = &graphs; // allow graph plotting to start
#endif
    }

	// don't update LCD up to we get first communication package from the motor controller
	if (ui8_motor_controller_init
			&& (ui8_m_usart1_received_first_package < 10)) {
		ui8_status = 1;
	}
	// this will be executed only 1 time at startup
  else if(ui8_motor_controller_init &&
      motorVariablesStabilized) {
    // reset Wh value if battery voltage is over ui16_battery_voltage_reset_wh_counter_x10 (value configured by user)
    if (((uint32_t) ui_vars.ui16_adc_battery_voltage *
    ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000)
        > ((uint32_t) ui_vars.ui16_battery_voltage_reset_wh_counter_x10
            * 1000)) {
      ui_vars.ui32_wh_x10_offset = 0;
    }

    if (ui_vars.ui8_offroad_feature_enabled
        && ui_vars.ui8_offroad_enabled_on_startup) {
      ui_vars.ui8_offroad_mode = 1;
    }
  }

	return ui8_status;
}

void rt_calc_battery_soc_watts_hour(void) {
	uint32_t ui32_temp;

	ui32_temp = rt_vars.ui32_wh_x10 * 100;

	if (rt_vars.ui32_wh_x10_100_percent > 0) {
		ui32_temp /= rt_vars.ui32_wh_x10_100_percent;
	} else {
		ui32_temp = 0;
	}

	if (ui32_temp > 100)
		ui32_temp = 100;

  ui16_g_battery_soc_watts_hour = 100 - ui32_temp;
}

void rt_processing_stop(void) {
#ifndef SW102
  Display850C_rt_processing_stop();
#else
  SW102_rt_processing_stop();
#endif
}

void rt_processing_start(void) {
#ifndef SW102
  Display850C_rt_processing_start();
#else
  SW102_rt_processing_start();
#endif
}

/**
 * Called from the main thread every 100ms
 *
 */
void copy_rt_to_ui_vars(void) {
	ui_vars.ui16_adc_battery_voltage = rt_vars.ui16_adc_battery_voltage;
	ui_vars.ui8_battery_current_x5 = rt_vars.ui8_battery_current_x5;
	ui_vars.ui8_throttle = rt_vars.ui8_throttle;
	ui_vars.ui16_adc_pedal_torque_sensor = rt_vars.ui16_adc_pedal_torque_sensor;
	ui_vars.ui8_pedal_weight_with_offset = rt_vars.ui8_pedal_weight_with_offset;
	ui_vars.ui8_pedal_weight = rt_vars.ui8_pedal_weight;
	ui_vars.ui8_duty_cycle = rt_vars.ui8_duty_cycle;
	ui_vars.ui8_error_states = rt_vars.ui8_error_states;
	ui_vars.ui16_wheel_speed_x10 = rt_vars.ui16_wheel_speed_x10;
	ui_vars.ui8_pedal_cadence = rt_vars.ui8_pedal_cadence;
	ui_vars.ui8_pedal_cadence_filtered = rt_vars.ui8_pedal_cadence_filtered;
	ui_vars.ui16_motor_speed_erps = rt_vars.ui16_motor_speed_erps;
	ui_vars.ui8_motor_hall_sensors = rt_vars.ui8_motor_hall_sensors;
	ui_vars.ui8_pas_pedal_right = rt_vars.ui8_pas_pedal_right;
	ui_vars.ui8_temperature_current_limiting_value =
			rt_vars.ui8_temperature_current_limiting_value;
	ui_vars.ui8_motor_temperature = rt_vars.ui8_motor_temperature;
	ui_vars.ui32_wheel_speed_sensor_tick_counter =
			rt_vars.ui32_wheel_speed_sensor_tick_counter;
	ui_vars.ui16_battery_voltage_filtered_x10 =
			rt_vars.ui16_battery_voltage_filtered_x10;
	ui_vars.ui16_battery_current_filtered_x5 =
			rt_vars.ui16_battery_current_filtered_x5;
	ui_vars.ui16_battery_power_filtered_x50 =
			rt_vars.ui16_battery_power_filtered_x50;
	ui_vars.ui16_battery_power = rt_vars.ui16_battery_power_filtered;
	ui_vars.ui16_pedal_power = rt_vars.ui16_pedal_power_filtered;
	ui_vars.ui16_battery_voltage_soc_x10 = rt_vars.ui16_battery_voltage_soc_x10;
	ui_vars.ui32_wh_sum_x5 = rt_vars.ui32_wh_sum_x5;
	ui_vars.ui32_wh_sum_counter = rt_vars.ui32_wh_sum_counter;
	ui_vars.ui32_wh_x10 = rt_vars.ui32_wh_x10;
	ui_vars.ui8_braking = rt_vars.ui8_braking;
	ui_vars.ui8_foc_angle = (((uint16_t) rt_vars.ui8_foc_angle) * 14) / 10; // each units is equal to 1.4 degrees ((360 degrees / 256) = 1.4)
	ui_vars.ui32_trip_x10 = rt_vars.ui32_trip_x10;
	ui_vars.ui32_odometer_x10 = rt_vars.ui32_odometer_x10;

  rt_vars.ui32_wh_x10_100_percent = ui_vars.ui32_wh_x10_100_percent;
	rt_vars.ui32_wh_x10_offset = ui_vars.ui32_wh_x10_offset;
	rt_vars.ui16_battery_pack_resistance_x1000 = ui_vars.ui16_battery_pack_resistance_x1000;
	rt_vars.ui8_assist_level = ui_vars.ui8_assist_level;
	for (uint8_t i = 0; i < 9; i++) {
	  rt_vars.ui8_assist_level_factor[i] = ui_vars.ui8_assist_level_factor[i];
	}
  for (uint8_t i = 0; i < 9; i++) {
    rt_vars.ui8_walk_assist_level_factor[i] = ui_vars.ui8_walk_assist_level_factor[i];
  }
	rt_vars.ui8_lights = ui_vars.ui8_lights;
	rt_vars.ui8_walk_assist = ui_vars.ui8_walk_assist;
	rt_vars.ui8_offroad_mode = ui_vars.ui8_offroad_mode;
	rt_vars.ui8_battery_max_current = ui_vars.ui8_battery_max_current;
	rt_vars.ui8_battery_current_min_adc = ui_vars.ui8_battery_current_min_adc;
	rt_vars.ui8_ramp_up_amps_per_second_x10 =
			ui_vars.ui8_ramp_up_amps_per_second_x10;
	rt_vars.ui8_target_max_battery_power = ui_vars.ui8_target_max_battery_power;
	rt_vars.ui16_battery_low_voltage_cut_off_x10 =
			ui_vars.ui16_battery_low_voltage_cut_off_x10;
	rt_vars.ui16_wheel_perimeter = ui_vars.ui16_wheel_perimeter;
	rt_vars.ui8_wheel_max_speed = ui_vars.wheel_max_speed_x10 / 10;
	rt_vars.ui8_motor_type = ui_vars.ui8_motor_type;
	rt_vars.ui8_motor_assistance_startup_without_pedal_rotation =
			ui_vars.ui8_motor_assistance_startup_without_pedal_rotation;
	rt_vars.ui8_temperature_limit_feature_enabled =
			ui_vars.ui8_temperature_limit_feature_enabled;
	rt_vars.ui8_startup_motor_power_boost_always =
			ui_vars.ui8_startup_motor_power_boost_always;
	rt_vars.ui8_startup_motor_power_boost_limit_power =
			ui_vars.ui8_startup_motor_power_boost_limit_power;
	rt_vars.ui8_startup_motor_power_boost_time =
			ui_vars.ui8_startup_motor_power_boost_time;
  for (uint8_t i = 0; i < 9; i++) {
    rt_vars.ui8_startup_motor_power_boost_factor[i] = ui_vars.ui8_startup_motor_power_boost_factor[i];
  }
	rt_vars.ui8_startup_motor_power_boost_fade_time =
			ui_vars.ui8_startup_motor_power_boost_fade_time;
	rt_vars.ui8_startup_motor_power_boost_feature_enabled =
			ui_vars.ui8_startup_motor_power_boost_feature_enabled;
	rt_vars.ui8_motor_temperature_min_value_to_limit =
			ui_vars.ui8_motor_temperature_min_value_to_limit;
	rt_vars.ui8_motor_temperature_max_value_to_limit =
			ui_vars.ui8_motor_temperature_max_value_to_limit;
	rt_vars.ui8_offroad_feature_enabled = ui_vars.ui8_offroad_feature_enabled;
	rt_vars.ui8_offroad_enabled_on_startup =
			ui_vars.ui8_offroad_enabled_on_startup;
	rt_vars.ui8_offroad_speed_limit = ui_vars.ui8_offroad_speed_limit;
	rt_vars.ui8_offroad_power_limit_enabled =
			ui_vars.ui8_offroad_power_limit_enabled;
	rt_vars.ui8_offroad_power_limit_div25 =
			ui_vars.ui8_offroad_power_limit_div25;
  rt_vars.ui8_torque_sensor_calibration_pedal_ground =
      ui_vars.ui8_torque_sensor_calibration_pedal_ground;

  rt_vars.ui8_torque_sensor_calibration_feature_enabled = ui_vars.ui8_torque_sensor_calibration_feature_enabled;
  rt_vars.ui8_torque_sensor_calibration_pedal_ground = ui_vars.ui8_torque_sensor_calibration_pedal_ground;

  // Some l3 vars are derived only from other l3 vars
  uint32_t ui32_battery_cells_number_x10 =
      (uint32_t) (ui_vars.ui8_battery_cells_number * 10);

  uint8_t volt_based_soc;
  if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_90))) {
    volt_based_soc = 95;
  } else if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_80))) {
    volt_based_soc = 85;
  } else if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_70))) {
    volt_based_soc = 75;
  } else if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_60))) {
    volt_based_soc = 65;
  } else if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_50))) {
    volt_based_soc = 55;
  } else if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_40))) {
    volt_based_soc = 45;
  } else if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_30))) {
    volt_based_soc = 35;
  } else if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_20))) {
    volt_based_soc = 25;
  } else if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_10))) {
    volt_based_soc = 15;
  } else if (ui_vars.ui16_battery_voltage_soc_x10
      > ((uint16_t) ((float) ui32_battery_cells_number_x10
          * LI_ION_CELL_VOLTS_0))) {
    volt_based_soc = 5;
  } else {
    volt_based_soc = 0;
  }
  ui_vars.volt_based_soc = volt_based_soc;
}

/// must be called from main() idle loop
void automatic_power_off_management(void) {
	static uint32_t ui16_lcd_power_off_time_counter = 0;

	if (ui_vars.ui8_lcd_power_off_time_minutes != 0) {
		// see if we should reset the automatic power off minutes counter
		if ((ui_vars.ui16_wheel_speed_x10 > 0) ||   // wheel speed > 0
				(ui_vars.ui8_battery_current_x5 > 0) || // battery current > 0
				(ui_vars.ui8_braking) ||                // braking
				buttons_get_events()) {                 // any button active
			ui16_lcd_power_off_time_counter = 0;
		} else {
			// increment the automatic power off ticks counter
			ui16_lcd_power_off_time_counter++;

			// check if we should power off the LCD
			if (ui16_lcd_power_off_time_counter
					>= (ui_vars.ui8_lcd_power_off_time_minutes * 10 * 60)) { // have we passed our timeout?
				lcd_power_off(1);
			}
		}
	} else {
		ui16_lcd_power_off_time_counter = 0;
	}
}

void communications(void) {
//  static uint8_t state = 0;
  static uint32_t num_missed_packets = 0;
  uint8_t ui8_frame_type = 0;
  static uint8_t ui8_cnt = 0;
  bool periodic_answer_received = false;

  const uint8_t *p_rx_buffer = uart_get_rx_buffer_rdy();

  // process rx package if we are simulating or the UART had a packet
  if (g_is_sim_motor || p_rx_buffer) {
    if (g_is_sim_motor)
      parse_simmotor();
    else if (p_rx_buffer) {
      // now process rx data
      num_missed_packets = 0; // reset missed packet count

      ui8_frame_type = p_rx_buffer[2];
      switch (ui8_frame_type) {
        case 0:
          rt_vars.ui16_adc_battery_voltage = p_rx_buffer[3] | (((uint16_t) (p_rx_buffer[4] & 0x30)) << 4);
          rt_vars.ui8_battery_current_x5 = p_rx_buffer[5];
          rt_vars.ui16_wheel_speed_x10 = ((uint16_t) p_rx_buffer[6]) | (((uint16_t) p_rx_buffer[7] << 8));

          // for some reason, the previous value of rt_vars.ui16_wheel_speed_x10 is 16384, because p_rx_buffer[7] is 64,
          // this even when rx_buffer[6] and rx_buffer[7] are both 0 on the motor controller
          rt_vars.ui16_wheel_speed_x10 = rt_vars.ui16_wheel_speed_x10 & 0x7ff; // 0x7ff = 204.7km/h

          uint8_t ui8_temp = p_rx_buffer[8];
          rt_vars.ui8_braking = ui8_temp & 1;
          rt_vars.ui8_motor_hall_sensors = (ui8_temp >> 1) & 7;
          rt_vars.ui8_pas_pedal_right = (ui8_temp >> 4) & 1;
          rt_vars.ui8_adc_throttle = p_rx_buffer[9];

          if (rt_vars.ui8_temperature_limit_feature_enabled) {
            rt_vars.ui8_motor_temperature = p_rx_buffer[10];
          } else {
            rt_vars.ui8_throttle = p_rx_buffer[10];
          }

          rt_vars.ui16_adc_pedal_torque_sensor = ((uint16_t) p_rx_buffer[11]) | (((uint16_t) (p_rx_buffer[7] & 0xC0)) << 2);
          rt_vars.ui8_pedal_weight_with_offset = p_rx_buffer[12];
          rt_vars.ui8_pedal_weight = p_rx_buffer[13];

          rt_vars.ui8_pedal_cadence = p_rx_buffer[14];
          rt_vars.ui8_duty_cycle = p_rx_buffer[15];
          rt_vars.ui16_motor_speed_erps = ((uint16_t) p_rx_buffer[16]) | ((uint16_t) p_rx_buffer[17] << 8);
          rt_vars.ui8_foc_angle = p_rx_buffer[18];
          rt_vars.ui8_error_states = p_rx_buffer[19];
          rt_vars.ui8_temperature_current_limiting_value = p_rx_buffer[20];

          uint32_t ui32_wheel_speed_sensor_tick_temp;
          ui32_wheel_speed_sensor_tick_temp = ((uint32_t) p_rx_buffer[21]) |
              (((uint32_t) p_rx_buffer[22]) << 8) | (((uint32_t) p_rx_buffer[23]) << 16);
          rt_vars.ui32_wheel_speed_sensor_tick_counter = ui32_wheel_speed_sensor_tick_temp;

          rt_vars.ui16_pedal_power_x10 = ((uint16_t) p_rx_buffer[24]) | ((uint16_t) p_rx_buffer[25] << 8);

          periodic_answer_received = true;
          break;

        case 1:
          g_tsdz2_configurations_set = true;
          g_communications_state = COMMUNICATIONS_READY;
          ui8_cnt = 0;
          g_has_seen_motor = true; // after setting the configurations, we can consider the motor is ready
          break;

        // firmware version
        case 2:
          g_tsdz2_firmware_version.major = p_rx_buffer[3];
          g_tsdz2_firmware_version.minor = p_rx_buffer[4];
          g_tsdz2_firmware_version.patch = p_rx_buffer[5];

          // check to see if the firmware is valid
          if (g_tsdz2_firmware_version.major != atoi(TSDZ2_FIRMWARE_MAJOR) ||
              g_tsdz2_firmware_version.minor != atoi(TSDZ2_FIRMWARE_MINOR)) {
//            APP_ERROR_HANDLER(FAULT_TSDZ2_WRONG_FIRMWARE);
            fieldPrintf(&bootStatus2, "TSDZ2 firmware error");
            while (1); // block here

            g_tsdz2_firmware_version.major = 0xff; // invalidate the version
          }

          g_communications_state = COMMUNICATIONS_READY;
          ui8_cnt = 0;
          break;
      }
    }

    // let's wait for 10 packages, seems that first ADC battery voltages have incorrect values
    ui8_m_usart1_received_first_package++;
    if (ui8_m_usart1_received_first_package > 10)
      ui8_m_usart1_received_first_package = 10;
  } else {
    // We expected a packet during this 100ms window but one did not arrive.  This might happen if the motor is still booting and we don't want to declare failure
    // unless something is seriously busted (because we will be raising the fault screen and eventually forcing the bike to shutdown) so be very conservative
    // and wait for 10 seconds of missed packets.
    if (g_has_seen_motor && num_missed_packets++ == 50)
      APP_ERROR_HANDLER(FAULT_LOSTRX);
  }

  // only if we are receiving communications from TSDZ2
  switch (g_communications_state) {
    case COMMUNICATIONS_GET_MOTOR_FIRMWARE_VERSION:
      rt_send_tx_package(2);
      g_communications_state = COMMUNICATIONS_WAIT_MOTOR_FIRMWARE_VERSION;
      break;

    case COMMUNICATIONS_WAIT_MOTOR_FIRMWARE_VERSION:
      rt_send_tx_package(2);
      ++ui8_cnt;
      if (ui8_cnt > 5)
        APP_ERROR_HANDLER(FAULT_LOSTRX);
      break;

    case COMMUNICATIONS_READY:
      if (periodic_answer_received)
        rt_send_tx_package(0);
      break;

    case COMMUNICATIONS_SET_CONFIGURATIONS:
      rt_send_tx_package(1);
      g_communications_state = COMMUNICATIONS_WAIT_CONFIGURATIONS;
      break;

    case COMMUNICATIONS_WAIT_CONFIGURATIONS:
      rt_send_tx_package(1);
      ++ui8_cnt;
      if (ui8_cnt > 5)
        APP_ERROR_HANDLER(FAULT_LOSTRX);
      break;

    default:
      break;
  }
}

// Note: this called from ISR context every 100ms
void rt_processing(void)
{
  communications();
  /************************************************************************************************/
  // now do all the calculations that must be done every 100ms
  rt_low_pass_filter_battery_voltage_current_power();
  rt_low_pass_filter_pedal_power();
  rt_low_pass_filter_pedal_cadence();
  rt_calc_battery_voltage_soc();
  rt_calc_odometer();
  rt_calc_wh();
  rt_graph_process();
  /************************************************************************************************/
  rt_first_time_management();
  rt_calc_battery_soc_watts_hour();
}

void prepare_torque_sensor_calibration_table(void) {
  static bool first_time = true;

  // we need to make this atomic
  rt_processing_stop();

  // at the very first time, copy the ADC values from one table to the other
  if (first_time) {
    first_time = false;

    for (uint8_t i = 0; i < 8; i++) {
        rt_vars.ui16_torque_sensor_calibration_table_left[i][0] = ui_vars.ui16_torque_sensor_calibration_table_left[i][1];
        rt_vars.ui16_torque_sensor_calibration_table_right[i][0] = ui_vars.ui16_torque_sensor_calibration_table_right[i][1];
    }
  }

  // get the delta values of ADC steps per kg
  for (uint8_t i = 1; i < 8; i++) {
    // get the deltas x100
    rt_vars.ui16_torque_sensor_calibration_table_left[i][1] =
        ((ui_vars.ui16_torque_sensor_calibration_table_left[i][0] - ui_vars.ui16_torque_sensor_calibration_table_left[i - 1][0]) * 100) /
        (ui_vars.ui16_torque_sensor_calibration_table_left[i][1] - ui_vars.ui16_torque_sensor_calibration_table_left[i - 1][1]);

    rt_vars.ui16_torque_sensor_calibration_table_right[i][1] =
        ((ui_vars.ui16_torque_sensor_calibration_table_right[i][0] - ui_vars.ui16_torque_sensor_calibration_table_right[i - 1][0]) * 100) /
        (ui_vars.ui16_torque_sensor_calibration_table_right[i][1] - ui_vars.ui16_torque_sensor_calibration_table_right[i - 1][1]);
  }
  // very first table value need to the calculated here
  rt_vars.ui16_torque_sensor_calibration_table_left[0][1] = rt_vars.ui16_torque_sensor_calibration_table_left[1][1]; // the first delta is equal the the second one
  rt_vars.ui16_torque_sensor_calibration_table_right[0][1] = rt_vars.ui16_torque_sensor_calibration_table_right[1][1]; // the first delta is equal the the second one


  rt_processing_start();
}
