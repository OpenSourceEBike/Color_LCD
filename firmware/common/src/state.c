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
#include "adc.h"
#include "timer.h"
#include <stdlib.h>
#ifdef SW102
#include "ble_services.h"
#endif

//#define DEBUG_TSDZ2_FIRMWARE

typedef enum {
  FRAME_TYPE_ALIVE = 0,
  FRAME_TYPE_STATUS = 1,
  FRAME_TYPE_PERIODIC = 2,
  FRAME_TYPE_CONFIGURATIONS = 3,
  FRAME_TYPE_FIRMWARE_VERSION = 4,
} frame_type_t;

static uint8_t ui8_m_usart1_received_first_package = 0;
uint8_t ui8_g_battery_soc;
volatile uint8_t ui8_g_motorVariablesStabilized = 0;

volatile uint8_t m_get_tsdz2_firmware_version; // true if we are simulating a motor (and therefore not talking on serial at all)
volatile motor_init_state_t g_motor_init_state = MOTOR_INIT_GET_MOTOR_ALIVE;
volatile motor_init_state_config_t g_motor_init_state_conf = MOTOR_INIT_CONFIG_SEND_CONFIG;
volatile motor_init_status_t ui8_g_motor_init_status = MOTOR_INIT_STATUS_RESET;

tsdz2_firmware_version_t g_tsdz2_firmware_version = { 0xff, 0, 0 };

static void motor_init(void);

rt_vars_t rt_vars;
ui_vars_t ui_vars;

volatile bool m_reset_wh_flag = false;

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

	// motor current drain x5
  rt_vars.ui8_motor_current_x5 = fakeRandom(&curstore, 0, max_cur10x) / 2;

	rt_vars.ui16_wheel_speed_x10 = fakeRandom(&speedstore, 80, 300);
    diststore += rt_vars.ui16_wheel_speed_x10 * 2.6; // speed x 10 to millimeters per 100 ms
rt_vars.ui16_wheel_speed_x10 = 842; // for testing, just leave speed fixed

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
	rt_vars.ui8_error_states = 0; // fake(0, ERROR_MAX);

	// wheel_speed_sensor_tick_counter

    if (diststore > rt_vars.ui16_wheel_perimeter) {
        rt_vars.ui32_wheel_speed_sensor_tick_counter += 1;
        diststore -= rt_vars.ui16_wheel_perimeter;
    }
}

void rt_send_tx_package(frame_type_t type) {
  uint8_t crc_len = 3; // minimun is 3
	uint8_t *ui8_usart1_tx_buffer = uart_get_tx_buffer();

	/************************************************************************************************/
	// send tx package
	// start up byte
	ui8_usart1_tx_buffer[0] = 0x59;
  ui8_usart1_tx_buffer[1] = crc_len;
  // type of the frame
	ui8_usart1_tx_buffer[2] = (uint8_t) type;

	switch (type) {
	  case FRAME_TYPE_PERIODIC:
      if (rt_vars.ui8_walk_assist) {
        ui8_usart1_tx_buffer[3] = (uint8_t) rt_vars.ui8_walk_assist_level_factor[((rt_vars.ui8_assist_level) - 1)];
        ui8_usart1_tx_buffer[4] = 0;
      } else if (rt_vars.ui8_assist_level) {
        uint16_t ui16_temp = rt_vars.ui16_assist_level_factor[((rt_vars.ui8_assist_level) - 1)];
        ui8_usart1_tx_buffer[3] = (uint8_t) (ui16_temp & 0xff);
        ui8_usart1_tx_buffer[4] = (uint8_t) (ui16_temp >> 8);
      } else {
        // if rt_vars.ui8_assist_level = 0, send 0!! always disable motor when assist level is 0
        ui8_usart1_tx_buffer[3] = 0;
        ui8_usart1_tx_buffer[4] = 0;
      }

      ui8_usart1_tx_buffer[5] = (rt_vars.ui8_lights & 1) | ((rt_vars.ui8_walk_assist & 1) << 1);

      // battery power limit
      if (rt_vars.ui8_street_mode_enabled)
      {
        ui8_usart1_tx_buffer[6] = rt_vars.ui8_street_mode_power_limit_div25;
      }
      else
      {
        ui8_usart1_tx_buffer[6] = rt_vars.ui8_target_max_battery_power_div25;
      }

      // startup motor power boost
      uint16_t ui16_temp = (uint8_t) rt_vars.ui16_startup_motor_power_boost_factor[((rt_vars.ui8_assist_level) - 1)];
      ui8_usart1_tx_buffer[7] = (uint8_t) (ui16_temp & 0xff);
      ui8_usart1_tx_buffer[8] = (uint8_t) (ui16_temp >> 8);

      // wheel max speed
      if (rt_vars.ui8_street_mode_enabled)
      {
        ui8_usart1_tx_buffer[9] = rt_vars.ui8_street_mode_speed_limit;
      }
      else
      {
        ui8_usart1_tx_buffer[9] = rt_vars.ui8_wheel_max_speed;
      }

      // motor temperature limit function or throttle
      if (rt_vars.ui8_street_mode_enabled &&
          rt_vars.ui8_street_mode_throttle_enabled)
      {
        ui8_usart1_tx_buffer[10] = rt_vars.ui8_temperature_limit_feature_enabled & 1;
      }
      else
      {
        ui8_usart1_tx_buffer[10] = rt_vars.ui8_temperature_limit_feature_enabled & 3;
      }

      // virtual throttle
      ui8_usart1_tx_buffer[11] = (uint8_t) ((((uint16_t) rt_vars.ui8_throttle_virtual) * 255) / 100);

      crc_len = 13;
      ui8_usart1_tx_buffer[1] = crc_len;
	    break;

    // set configurations
	  case FRAME_TYPE_CONFIGURATIONS:
      // battery low voltage cut-off
      ui8_usart1_tx_buffer[3] = (uint8_t) (rt_vars.ui16_battery_low_voltage_cut_off_x10 & 0xff);
      ui8_usart1_tx_buffer[4] = (uint8_t) (rt_vars.ui16_battery_low_voltage_cut_off_x10 >> 8);

      // wheel perimeter
      ui8_usart1_tx_buffer[5] = (uint8_t) (rt_vars.ui16_wheel_perimeter & 0xff);
      ui8_usart1_tx_buffer[6] = (uint8_t) (rt_vars.ui16_wheel_perimeter >> 8);

      // battery max current
      ui8_usart1_tx_buffer[7] = rt_vars.ui8_battery_max_current;

      ui8_usart1_tx_buffer[8] = rt_vars.ui8_startup_motor_power_boost_feature_enabled |
          (rt_vars.ui8_startup_motor_power_boost_always << 1) |
          (rt_vars.ui8_startup_motor_power_boost_limit_power << 2) |
          (rt_vars.ui8_torque_sensor_calibration_feature_enabled << 3) |
          (rt_vars.ui8_torque_sensor_calibration_pedal_ground << 4) |
          (rt_vars.ui8_motor_assistance_startup_without_pedal_rotation << 5) |
          ((rt_vars.ui8_motor_type & 1) << 6);

      // motor max current
      ui8_usart1_tx_buffer[9] = rt_vars.ui8_motor_max_current;
      // startup motor power boost time
      ui8_usart1_tx_buffer[10] = rt_vars.ui8_startup_motor_power_boost_time;
      // startup motor power boost fade time
      ui8_usart1_tx_buffer[11] = rt_vars.ui8_startup_motor_power_boost_fade_time;

      // motor over temperature min and max values to limit
      ui8_usart1_tx_buffer[12] = rt_vars.ui8_motor_temperature_min_value_to_limit;
      ui8_usart1_tx_buffer[13] = rt_vars.ui8_motor_temperature_max_value_to_limit;

      ui8_usart1_tx_buffer[14] = rt_vars.ui8_ramp_up_amps_per_second_x10;

      // TODO
      // target speed for cruise
      ui8_usart1_tx_buffer[15] = 0;

      // torque sensor calibration tables
      uint8_t j = 16;
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
      ui8_usart1_tx_buffer[79] = rt_vars.ui8_motor_current_min_adc;
      ui8_usart1_tx_buffer[80] = (rt_vars.ui8_pedal_cadence_fast_stop |
          (rt_vars.ui8_field_weakening << 1) |
          (rt_vars.ui8_coast_brake_enable << 2) |
          (rt_vars.ui8_motor_current_control_mode << 3));
      ui8_usart1_tx_buffer[81] = rt_vars.ui8_coast_brake_adc;
      ui8_usart1_tx_buffer[82] = rt_vars.ui8_adc_lights_current_offset;
      ui8_usart1_tx_buffer[83] = rt_vars.ui8_torque_sensor_filter;
      ui8_usart1_tx_buffer[84] = rt_vars.ui8_torque_sensor_adc_threshold;

      crc_len = 86;
      ui8_usart1_tx_buffer[1] = crc_len;
	    break;

	    case FRAME_TYPE_STATUS:
	    case FRAME_TYPE_FIRMWARE_VERSION:
	      // nothing to add to the package
	      break;

	    default:
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
	if (g_motor_init_state != MOTOR_INIT_SIMULATING) // If we are simulating received packets never send real packets
		uart_send_tx_buffer(ui8_usart1_tx_buffer, ui8_usart1_tx_buffer[1] + 2);
}

void rt_low_pass_filter_battery_voltage_current_power(void) {
	static uint32_t ui32_battery_voltage_accumulated_x10000 = 0;
	static uint16_t ui16_battery_current_accumulated_x5 = 0;
  static uint16_t ui16_motor_current_accumulated_x5 = 0;

	// low pass filter battery voltage
	ui32_battery_voltage_accumulated_x10000 -=
	    (ui32_battery_voltage_accumulated_x10000 >> BATTERY_VOLTAGE_FILTER_COEFFICIENT);

	ui32_battery_voltage_accumulated_x10000 +=
			((uint32_t) rt_vars.ui16_adc_battery_voltage * ADC_BATTERY_VOLTAGE_PER_ADC_STEP_X10000);

	rt_vars.ui16_battery_voltage_filtered_x10 =
			(((uint32_t) (ui32_battery_voltage_accumulated_x10000 >> BATTERY_VOLTAGE_FILTER_COEFFICIENT)) / 1000);

	// low pass filter battery current
	ui16_battery_current_accumulated_x5 -= ui16_battery_current_accumulated_x5
			>> BATTERY_CURRENT_FILTER_COEFFICIENT;
	ui16_battery_current_accumulated_x5 +=
			(uint16_t) rt_vars.ui8_battery_current_x5;
	rt_vars.ui16_battery_current_filtered_x5 =
			ui16_battery_current_accumulated_x5
					>> BATTERY_CURRENT_FILTER_COEFFICIENT;

  // low pass filter motor current
  ui16_motor_current_accumulated_x5 -= ui16_motor_current_accumulated_x5
      >> MOTOR_CURRENT_FILTER_COEFFICIENT;
  ui16_motor_current_accumulated_x5 +=
      (uint16_t) rt_vars.ui8_motor_current_x5;
  rt_vars.ui16_motor_current_filtered_x5 =
      ui16_motor_current_accumulated_x5
          >> MOTOR_CURRENT_FILTER_COEFFICIENT;

	// full battery power, considering the power loss also inside the battery and cables, because we are using the battery resistance
  //
  uint16_t ui16_battery_power_filtered_x50 = rt_vars.ui16_battery_current_filtered_x5 * rt_vars.ui16_battery_voltage_filtered_x10;
  rt_vars.ui16_battery_power_filtered = ui16_battery_power_filtered_x50 / 50;

  // P = R * I^2
  uint32_t ui32_temp = (uint32_t) rt_vars.ui16_battery_current_filtered_x5;
  ui32_temp = ui32_temp * ui32_temp; // I * I
  ui32_temp /= 25;

  ui32_temp *= (uint32_t) rt_vars.ui16_battery_pack_resistance_x1000; // R * I * I
  ui32_temp /= 20; // now is _x50
  rt_vars.ui16_battery_power_loss = (uint16_t) (ui32_temp / 50);

  rt_vars.ui16_full_battery_power_filtered_x50 = ui16_battery_power_filtered_x50 + (uint16_t) ui32_temp;
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

	if (m_reset_wh_flag == false) {
    if (rt_vars.ui16_full_battery_power_filtered_x50 > 0) {
      rt_vars.ui32_wh_sum_x5 += rt_vars.ui16_full_battery_power_filtered_x50 / 10;
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
}

void reset_wh(void) {
  m_reset_wh_flag = true;
  rt_vars.ui32_wh_sum_x5 = 0;
  rt_vars.ui32_wh_sum_counter = 0;
  m_reset_wh_flag = false;
}

static void rt_calc_odometer(void) {
  static uint8_t ui8_1s_timer_counter;
  static uint32_t ui32_remainder = 0;
	uint8_t ui8_01km_flag = 0;

	// calc at 1s rate
	if (++ui8_1s_timer_counter >= 10) {
		ui8_1s_timer_counter = 0;

		// calculate how many revolutions since last reset and convert to distance traveled
		uint32_t ui32_temp = (rt_vars.ui32_wheel_speed_sensor_tick_counter
				- rt_vars.ui32_wheel_speed_sensor_tick_counter_offset)
				* ((uint32_t) rt_vars.ui16_wheel_perimeter) + ui32_remainder;

		// if traveled distance is more than 100 meters update all distance variables and reset
		if (ui32_temp >= 100000) { // 100000 -> 100000 mm -> 0.1 km
			// update all distance variables
			// ui_vars.ui16_distance_since_power_on_x10 += 1;
			rt_vars.ui32_odometer_x10 += 1;
			ui8_01km_flag = 1;
			ui32_remainder = ui32_temp - 100000;

			// reset the always incrementing value (up to motor controller power reset) by setting the offset to current value
			rt_vars.ui32_wheel_speed_sensor_tick_counter_offset =
					rt_vars.ui32_wheel_speed_sensor_tick_counter;
		}
	}

  // calc battery energy per km
#define BATTERY_ENERGY_H_KM_FACTOR_X2 1800 // (60 * 60) / 2, each step at fixed interval of 100ms and apply 1 / 2 for have value from _x50 to _x100

	// keep accumulating the energy
  rt_vars.battery_energy_h_km.ui32_sum_x50 += rt_vars.ui16_full_battery_power_filtered_x50;

  static uint16_t ui16_one_km_timeout_counter = 0;

  // reset value if riding at very low speed or being stopped for 2 minutes
  if (++ui16_one_km_timeout_counter >= 600) { // 600 equals min of average 2km/h for 2 minutes, at least
    ui16_one_km_timeout_counter = 600; // keep on this state...
    rt_vars.battery_energy_h_km.ui32_value_x100 = 0;
    rt_vars.battery_energy_h_km.ui32_value_x10 = 0;
    rt_vars.battery_energy_h_km.ui32_sum_x50 = 0;
  }

	if (ui8_01km_flag) {
    ui16_one_km_timeout_counter = 0;
    rt_vars.battery_energy_h_km.ui32_value_x100 = rt_vars.battery_energy_h_km.ui32_sum_x50 / BATTERY_ENERGY_H_KM_FACTOR_X2;
    rt_vars.battery_energy_h_km.ui32_value_x10 = rt_vars.battery_energy_h_km.ui32_value_x100 / 10;
    rt_vars.battery_energy_h_km.ui32_sum_x50 = 0;
  }
}

static void rt_calc_trips(void) {
  static uint8_t ui8_1s_timer_counter = 0;
  static uint8_t ui8_3s_timer_counter = 0;
  static uint32_t ui32_wheel_speed_sensor_tick_counter_offset = 0;
  static uint32_t ui32_remainder = 0;
  
  // used to determine if trip avg speed values have to be calculated :
  // - on first time this function is called ; so set by dfault to 1
  // - then every 1 meter traveled
  static uint8_t ui8_calc_avg_speed_flag = 1;

  // calculate how many revolutions since last reset ...
  uint32_t wheel_ticks = rt_vars.ui32_wheel_speed_sensor_tick_counter
      - ui32_wheel_speed_sensor_tick_counter_offset;

  // ... and convert to distance traveled
  uint32_t ui32_temp = wheel_ticks * ((uint32_t) rt_vars.ui16_wheel_perimeter) + ui32_remainder;

  // if traveled distance is more than 1 wheel turn update trip variables and reset
  if (wheel_ticks >= 1) { 
 
    ui8_calc_avg_speed_flag = 1;

    // update all trip distance variables
    rt_vars.ui32_trip_a_distance_x1000 += (ui32_temp / 1000);
    rt_vars.ui32_trip_b_distance_x1000 += (ui32_temp / 1000);
    ui32_remainder = ui32_temp % 1000;

    // update trip A max speed
    if (rt_vars.ui16_wheel_speed_x10 > rt_vars.ui16_trip_a_max_speed_x10)
      rt_vars.ui16_trip_a_max_speed_x10 = rt_vars.ui16_wheel_speed_x10;

    // update trip B max speed
    if (rt_vars.ui16_wheel_speed_x10 > rt_vars.ui16_trip_b_max_speed_x10)
      rt_vars.ui16_trip_b_max_speed_x10 = rt_vars.ui16_wheel_speed_x10;
    
    // reset the always incrementing value (up to motor controller power reset) by setting the offset to current value
    ui32_wheel_speed_sensor_tick_counter_offset =	rt_vars.ui32_wheel_speed_sensor_tick_counter;

  }

  // calculate trip A and B average speeds (every 3s)
  if (ui8_calc_avg_speed_flag == 1 && ++ui8_3s_timer_counter >= 30) {
    rt_vars.ui16_trip_a_avg_speed_x10 = rt_vars.ui32_trip_a_time ? (rt_vars.ui32_trip_a_distance_x1000 * 36) / rt_vars.ui32_trip_a_time : 0;
    rt_vars.ui16_trip_b_avg_speed_x10 = rt_vars.ui32_trip_b_time ? (rt_vars.ui32_trip_b_distance_x1000 * 36) / rt_vars.ui32_trip_b_time : 0;
    
    // reset 3s timer counter and flag
    ui8_calc_avg_speed_flag = 0;    
    ui8_3s_timer_counter = 0;
  }

  // at 1s rate : update all trip time variables if wheel is turning
  if (++ui8_1s_timer_counter >= 10) {
    if (rt_vars.ui16_wheel_speed_x10 > 0) {
      rt_vars.ui32_trip_a_time += 1;
      rt_vars.ui32_trip_b_time += 1;
      
#ifndef SW102
      rt_vars.ui32_trip_a_last_update_time = RTC_GetCounter();
      rt_vars.ui32_trip_b_last_update_time = RTC_GetCounter();
#endif

    }
    ui8_1s_timer_counter = 0;
  }

#ifndef SW102
  uint32_t current_time = RTC_GetCounter();

  if (ui_vars.ui8_trip_a_auto_reset && (current_time - rt_vars.ui32_trip_a_last_update_time >= ui_vars.ui16_trip_a_auto_reset_hours * 3600)) {
    rt_vars.ui32_trip_a_last_update_time = current_time;
    rt_vars.ui32_trip_a_distance_x1000 = 0;
    rt_vars.ui32_trip_a_time = 0;
    rt_vars.ui16_trip_a_avg_speed_x10 = 0;
    rt_vars.ui16_trip_a_max_speed_x10 = 0;
  }

  if (ui_vars.ui8_trip_b_auto_reset && (current_time - rt_vars.ui32_trip_b_last_update_time >= ui_vars.ui16_trip_b_auto_reset_hours * 3600)) {
    rt_vars.ui32_trip_b_last_update_time = current_time;
    rt_vars.ui32_trip_b_distance_x1000 = 0;
    rt_vars.ui32_trip_b_time = 0;
    rt_vars.ui16_trip_b_avg_speed_x10 = 0;
    rt_vars.ui16_trip_b_max_speed_x10 = 0;
  }
#endif

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
  if (ui8_g_motorVariablesStabilized == 0 &&
      ((g_motor_init_state == MOTOR_INIT_READY) ||
      (g_motor_init_state == MOTOR_INIT_SIMULATING)))
    if (++ui32_counter > 50) {
      ui8_g_motorVariablesStabilized = 1;
#ifndef SW102
      extern Field *activeGraphs; // FIXME, move this extern someplace better, placing here for review purposes
  	  activeGraphs = &(*graphs[g_showNextScreenIndex]); // allow graph plotting to start
#endif
    }

	// don't update LCD up to we get first communication package from the motor controller
	if (ui8_motor_controller_init
			&& (ui8_m_usart1_received_first_package < 10)) {
		ui8_status = 1;
	}
	// this will be executed only 1 time at startup
  else if (ui8_motor_controller_init &&
      ui8_g_motorVariablesStabilized) {

    ui8_motor_controller_init = 0;

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

void rt_calc_battery_soc(void) {
	uint32_t ui32_temp;

	ui32_temp = rt_vars.ui32_wh_x10 * 100;

	if (rt_vars.ui32_wh_x10_100_percent > 0) {
		ui32_temp /= rt_vars.ui32_wh_x10_100_percent;
	} else {
		ui32_temp = 0;
	}

	if (ui32_temp > 100)
		ui32_temp = 100;

  ui8_g_battery_soc = (uint8_t) (100 - ui32_temp);
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
	ui_vars.ui16_battery_power_loss = rt_vars.ui16_battery_power_loss;
	ui_vars.ui8_motor_current_x5 = rt_vars.ui8_motor_current_x5;
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
	ui_vars.ui8_motor_temperature = rt_vars.ui8_motor_temperature;
	ui_vars.ui32_wheel_speed_sensor_tick_counter =
			rt_vars.ui32_wheel_speed_sensor_tick_counter;
	ui_vars.ui16_battery_voltage_filtered_x10 =
			rt_vars.ui16_battery_voltage_filtered_x10;
	ui_vars.ui16_battery_current_filtered_x5 =
			rt_vars.ui16_battery_current_filtered_x5;
  ui_vars.ui16_motor_current_filtered_x5 =
      rt_vars.ui16_motor_current_filtered_x5;
	ui_vars.ui16_full_battery_power_filtered_x50 =
			rt_vars.ui16_full_battery_power_filtered_x50;
	ui_vars.ui16_battery_power = rt_vars.ui16_battery_power_filtered;
	ui_vars.ui16_pedal_power = rt_vars.ui16_pedal_power_filtered;
	ui_vars.ui16_battery_voltage_soc_x10 = rt_vars.ui16_battery_voltage_soc_x10;
	ui_vars.ui32_wh_sum_x5 = rt_vars.ui32_wh_sum_x5;
	ui_vars.ui32_wh_sum_counter = rt_vars.ui32_wh_sum_counter;
	ui_vars.ui32_wh_x10 = rt_vars.ui32_wh_x10;
	ui_vars.ui8_braking = rt_vars.ui8_braking;
	ui_vars.ui8_foc_angle = (((uint16_t) rt_vars.ui8_foc_angle) * 14) / 10; // each units is equal to 1.4 degrees ((360 degrees / 256) = 1.4)

#ifndef SW102
  ui_vars.ui32_trip_a_last_update_time = rt_vars.ui32_trip_a_last_update_time;
  ui_vars.ui32_trip_b_last_update_time = rt_vars.ui32_trip_b_last_update_time;
#endif

	ui_vars.ui32_trip_a_distance_x1000 = rt_vars.ui32_trip_a_distance_x1000;
  ui_vars.ui32_trip_a_distance_x100 = rt_vars.ui32_trip_a_distance_x1000 / 10;  
  ui_vars.ui32_trip_a_time = rt_vars.ui32_trip_a_time;
  ui_vars.ui16_trip_a_avg_speed_x10 = rt_vars.ui16_trip_a_avg_speed_x10;
  ui_vars.ui16_trip_a_max_speed_x10 = rt_vars.ui16_trip_a_max_speed_x10;

  ui_vars.ui32_trip_b_distance_x1000 = rt_vars.ui32_trip_b_distance_x1000;
  ui_vars.ui32_trip_b_distance_x100 = rt_vars.ui32_trip_b_distance_x1000 / 10;
  ui_vars.ui32_trip_b_time = rt_vars.ui32_trip_b_time;
  ui_vars.ui16_trip_b_avg_speed_x10 = rt_vars.ui16_trip_b_avg_speed_x10;
  ui_vars.ui16_trip_b_max_speed_x10 = rt_vars.ui16_trip_b_max_speed_x10;

	ui_vars.ui32_odometer_x10 = rt_vars.ui32_odometer_x10;
	ui_vars.battery_energy_km_value_x100 = rt_vars.battery_energy_h_km.ui32_value_x100;
  ui_vars.ui16_adc_battery_current = rt_vars.ui16_adc_battery_current;

  rt_vars.ui32_wh_x10_100_percent = ui_vars.ui32_wh_x10_100_percent;
	rt_vars.ui32_wh_x10_offset = ui_vars.ui32_wh_x10_offset;
	rt_vars.ui16_battery_pack_resistance_x1000 = ui_vars.ui16_battery_pack_resistance_x1000;
	rt_vars.ui8_assist_level = ui_vars.ui8_assist_level;
	for (uint8_t i = 0; i < ASSIST_LEVEL_NUMBER; i++) {
	  rt_vars.ui16_assist_level_factor[i] = ui_vars.ui16_assist_level_factor[i];
	}
  for (uint8_t i = 0; i < ASSIST_LEVEL_NUMBER; i++) {
    rt_vars.ui8_walk_assist_level_factor[i] = ui_vars.ui8_walk_assist_level_factor[i];
  }
	rt_vars.ui8_lights = ui_vars.ui8_lights;
	rt_vars.ui8_walk_assist = ui_vars.ui8_walk_assist;
	rt_vars.ui8_offroad_mode = ui_vars.ui8_offroad_mode;
	rt_vars.ui8_battery_max_current = ui_vars.ui8_battery_max_current;
	rt_vars.ui8_motor_max_current = ui_vars.ui8_motor_max_current;
	rt_vars.ui8_motor_current_min_adc = ui_vars.ui8_motor_current_min_adc;
	rt_vars.ui8_field_weakening = ui_vars.ui8_field_weakening;
	rt_vars.ui8_ramp_up_amps_per_second_x10 =
			ui_vars.ui8_ramp_up_amps_per_second_x10;
	rt_vars.ui8_target_max_battery_power_div25 = ui_vars.ui8_target_max_battery_power_div25;
	rt_vars.ui16_battery_low_voltage_cut_off_x10 =
			ui_vars.ui16_battery_low_voltage_cut_off_x10;
	rt_vars.ui16_wheel_perimeter = ui_vars.ui16_wheel_perimeter;
	rt_vars.ui8_wheel_max_speed = ui_vars.wheel_max_speed_x10 / 10;
	rt_vars.ui8_motor_type = ui_vars.ui8_motor_type;
	rt_vars.ui8_motor_current_control_mode = ui_vars.ui8_motor_current_control_mode;
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
    rt_vars.ui16_startup_motor_power_boost_factor[i] = ui_vars.ui16_startup_motor_power_boost_factor[i];
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

  rt_vars.ui8_street_mode_enabled = ui_vars.ui8_street_mode_enabled;
  rt_vars.ui8_street_mode_speed_limit = ui_vars.ui8_street_mode_speed_limit;
  rt_vars.ui8_street_mode_power_limit_div25 = ui_vars.ui8_street_mode_power_limit_div25;
  rt_vars.ui8_street_mode_throttle_enabled = ui_vars.ui8_street_mode_throttle_enabled;

  rt_vars.ui8_pedal_cadence_fast_stop = ui_vars.ui8_pedal_cadence_fast_stop;
  rt_vars.ui8_coast_brake_adc = ui_vars.ui8_coast_brake_adc;
  rt_vars.ui8_adc_lights_current_offset = ui_vars.ui8_adc_lights_current_offset;
  rt_vars.ui8_throttle_virtual = ui_vars.ui8_throttle_virtual;
  rt_vars.ui8_torque_sensor_filter = ui_vars.ui8_torque_sensor_filter;
  rt_vars.ui8_torque_sensor_adc_threshold = ui_vars.ui8_torque_sensor_adc_threshold;
  rt_vars.ui8_coast_brake_enable = ui_vars.ui8_coast_brake_enable;
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
  frame_type_t ui8_frame;
  uint8_t process_frame = 0;
  uint16_t ui16_temp;

  const uint8_t *p_rx_buffer = uart_get_rx_buffer_rdy();

  // process rx package if we are simulating or the UART had a packet
  if ((g_motor_init_state == MOTOR_INIT_SIMULATING) || p_rx_buffer) {
    if (g_motor_init_state == MOTOR_INIT_SIMULATING)
      parse_simmotor();
    else if (p_rx_buffer) {
      // now process rx data
      ui8_frame = (frame_type_t) p_rx_buffer[2];

      switch (g_motor_init_state) {
        case MOTOR_INIT_WAIT_MOTOR_ALIVE:
          if (ui8_frame == FRAME_TYPE_ALIVE)
            g_motor_init_state = MOTOR_INIT_GET_MOTOR_FIRMWARE_VERSION;
          break;

        case MOTOR_INIT_WAIT_MOTOR_FIRMWARE_VERSION:
          if (ui8_frame == FRAME_TYPE_FIRMWARE_VERSION)
            process_frame = 1;
          break;

        case MOTOR_INIT_WAIT_CONFIGURATIONS_OK:
        case MOTOR_INIT_WAIT_GOT_CONFIGURATIONS_OK:
          if (ui8_frame == FRAME_TYPE_STATUS)
            process_frame = 1;
          break;

        case MOTOR_INIT_READY:
            process_frame = 1;
          break;
      }

      if (process_frame) {
        switch (ui8_frame) {
          case FRAME_TYPE_STATUS:
            ui8_g_motor_init_status = p_rx_buffer[3];
            break;

          case FRAME_TYPE_PERIODIC:
            rt_vars.ui16_adc_battery_voltage = p_rx_buffer[3] | (((uint16_t) (p_rx_buffer[4] & 0x30)) << 4);
            rt_vars.ui8_battery_current_x5 = p_rx_buffer[5];
            ui16_temp = ((uint16_t) p_rx_buffer[6]) | (((uint16_t) p_rx_buffer[7] << 8));
            rt_vars.ui16_wheel_speed_x10 = ui16_temp & 0x7ff; // 0x7ff = 204.7km/h as the other bits are used for other things

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
            rt_vars.ui8_motor_current_x5 = p_rx_buffer[20];

            uint32_t ui32_wheel_speed_sensor_tick_temp;
            ui32_wheel_speed_sensor_tick_temp = ((uint32_t) p_rx_buffer[21]) |
                (((uint32_t) p_rx_buffer[22]) << 8) | (((uint32_t) p_rx_buffer[23]) << 16);
            rt_vars.ui32_wheel_speed_sensor_tick_counter = ui32_wheel_speed_sensor_tick_temp;

            rt_vars.ui16_pedal_power_x10 = ((uint16_t) p_rx_buffer[24]) | ((uint16_t) p_rx_buffer[25] << 8);

            ui16_temp = (uint16_t) p_rx_buffer[26];
            rt_vars.ui16_adc_battery_current = ui16_temp | ((uint16_t) ((p_rx_buffer[7] & 0x18) << 5));
            break;

          case FRAME_TYPE_FIRMWARE_VERSION:
            rt_vars.ui8_error_states = p_rx_buffer[3];
            g_tsdz2_firmware_version.major = p_rx_buffer[4];
            g_tsdz2_firmware_version.minor = p_rx_buffer[5];
            g_tsdz2_firmware_version.patch = p_rx_buffer[6];
            g_motor_init_state = MOTOR_INIT_GOT_MOTOR_FIRMWARE_VERSION;
            break;
        }
      }
    }

    // let's wait for 10 packages, seems that first ADC battery voltages have incorrect values
    ui8_m_usart1_received_first_package++;
    if (ui8_m_usart1_received_first_package > 10)
      ui8_m_usart1_received_first_package = 10;
  }

  if (g_motor_init_state == MOTOR_INIT_READY)
    rt_send_tx_package(FRAME_TYPE_PERIODIC);
}

// Note: this called from ISR context every 100ms
void rt_processing(void)
{
  communications();

#ifdef SW102
  send_bluetooth(&rt_vars);
#endif

  // called here because this state machine for motor_init should run every 100ms
  // montor init processing must be done when exiting the configurations menu
  // once motor is initialized, this should take almost no processing time
  motor_init();

  /************************************************************************************************/
  // now do all the calculations that must be done every 100ms
  rt_low_pass_filter_battery_voltage_current_power();
  rt_low_pass_filter_pedal_power();
  rt_low_pass_filter_pedal_cadence();
  rt_calc_battery_voltage_soc();
  rt_calc_odometer();
  rt_calc_trips();
  rt_calc_wh();
  rt_graph_process();
  /************************************************************************************************/
  rt_first_time_management();
  rt_calc_battery_soc();
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

#define MIN_VOLTAGE_10X 140 // If our measured bat voltage (using ADC in the display) is lower than this, we assume we are running on a developers desk

static void motor_init(void) {
  static uint8_t ui8_once = 1;
  static uint16_t ui16_motor_init_command_error_cnt = 0;
  static uint8_t ui8_motor_init_status_cnt = 0;

  if ((g_motor_init_state != MOTOR_INIT_ERROR) &&
      (g_motor_init_state != MOTOR_INIT_ERROR_GET_FIRMWARE_VERSION) &&
      (g_motor_init_state != MOTOR_INIT_ERROR_FIRMWARE_VERSION) &&
      (g_motor_init_state != MOTOR_INIT_READY) &&
      (g_motor_init_state != MOTOR_INIT_SIMULATING)
#ifdef DEBUG_TSDZ2_FIRMWARE
      && (buttons_get_onoff_state() == 0))
#else
      )
#endif
  {
    if (ui8_once) {
      ui8_once = 0;

      // are we simulating?
      bool sim = (battery_voltage_10x_get() < MIN_VOLTAGE_10X);
      if (sim) {
        fieldPrintf(&bootStatus2, _S("SIMULATING TSDZ2!", "SIMULATING"));
        g_motor_init_state = MOTOR_INIT_SIMULATING;
      } else {
        fieldPrintf(&bootStatus2, _S("Wait TSDZ2", "Wait TSDZ2"));
      }
    }

    switch (g_motor_init_state) {
      case MOTOR_INIT_GET_MOTOR_ALIVE:
        ui16_motor_init_command_error_cnt = 500;
        g_motor_init_state = MOTOR_INIT_WAIT_MOTOR_ALIVE;
        // not break here to follow for next case

      case MOTOR_INIT_WAIT_MOTOR_ALIVE:
        // check timeout
        ui16_motor_init_command_error_cnt--;
        if (ui16_motor_init_command_error_cnt == 0) {
          fieldPrintf(&bootStatus2, _S("Error brakes", "e: brakes"));
          g_motor_init_state = MOTOR_INIT_GET_MOTOR_ALIVE;
        }
        break;

      case MOTOR_INIT_GET_MOTOR_FIRMWARE_VERSION:
        ui16_motor_init_command_error_cnt = 500;
        g_motor_init_state = MOTOR_INIT_WAIT_MOTOR_FIRMWARE_VERSION;
        // not break here to follow for next case

      case MOTOR_INIT_WAIT_MOTOR_FIRMWARE_VERSION:
        rt_send_tx_package(FRAME_TYPE_FIRMWARE_VERSION);
        // check timeout
        ui16_motor_init_command_error_cnt--;
        if (ui16_motor_init_command_error_cnt == 0) {
          fieldPrintf(&bootStatus2, _S("Error RX line", "e: RX"));
          g_motor_init_state = MOTOR_INIT_ERROR_GET_FIRMWARE_VERSION;
        }
        break;

      case MOTOR_INIT_GOT_MOTOR_FIRMWARE_VERSION:
        if (g_tsdz2_firmware_version.major == atoi(TSDZ2_FIRMWARE_MAJOR) &&
            g_tsdz2_firmware_version.minor == atoi(TSDZ2_FIRMWARE_MINOR)) {

            g_motor_init_state = MOTOR_INIT_SET_CONFIGURATIONS;
            // not break here to follow for next case
        } else {
          fieldPrintf(&bootStatus2, _S("TSDZ2 firmware error", "e: firmwa"));
          g_motor_init_state = MOTOR_INIT_ERROR_FIRMWARE_VERSION;
          break;
        }

      case MOTOR_INIT_SET_CONFIGURATIONS:
        ui16_motor_init_command_error_cnt = 500;
        g_motor_init_state_conf = MOTOR_INIT_CONFIG_SEND_CONFIG;
        g_motor_init_state = MOTOR_INIT_WAIT_CONFIGURATIONS_OK;
        // not break here to follow for next case

      case MOTOR_INIT_WAIT_CONFIGURATIONS_OK:
      case MOTOR_INIT_WAIT_GOT_CONFIGURATIONS_OK:
        // check timeout
        ui16_motor_init_command_error_cnt--;
        if (ui16_motor_init_command_error_cnt == 0) {
          fieldPrintf(&bootStatus2, _S("Error set config", "e: config")); // in the case we are on the boot screen
          g_motor_init_state = MOTOR_INIT_ERROR_SET_CONFIGURATIONS;
          break;
        }

        switch (g_motor_init_state_conf) {
          case MOTOR_INIT_CONFIG_SEND_CONFIG:
            rt_send_tx_package(FRAME_TYPE_CONFIGURATIONS);
            ui8_motor_init_status_cnt = 5;
            g_motor_init_state_conf = MOTOR_INIT_CONFIG_GET_STATUS;
            break;

          case MOTOR_INIT_CONFIG_GET_STATUS:
            rt_send_tx_package(FRAME_TYPE_STATUS);
            g_motor_init_state_conf = MOTOR_INIT_CONFIG_CHECK_STATUS;
            break;

          case MOTOR_INIT_CONFIG_CHECK_STATUS:
            if (ui8_g_motor_init_status == MOTOR_INIT_STATUS_RESET) {
              ui8_motor_init_status_cnt--;
              if (ui8_motor_init_status_cnt == 0) {
                g_motor_init_state_conf = MOTOR_INIT_CONFIG_SEND_CONFIG;
                break;
              }
            }

            if (ui8_g_motor_init_status == MOTOR_INIT_STATUS_RESET) {
              g_motor_init_state_conf = MOTOR_INIT_CONFIG_GET_STATUS;

            } else if (ui8_g_motor_init_status == MOTOR_INIT_STATUS_GOT_CONFIG) {

              g_motor_init_state = MOTOR_INIT_WAIT_GOT_CONFIGURATIONS_OK;
              g_motor_init_state_conf = MOTOR_INIT_CONFIG_GET_STATUS;

            } else if (ui8_g_motor_init_status == MOTOR_INIT_STATUS_INIT_OK) {

              g_motor_init_state = MOTOR_INIT_READY; // finally

              // reset state vars
              g_motor_init_state_conf = MOTOR_INIT_CONFIG_SEND_CONFIG;
              ui8_g_motor_init_status = MOTOR_INIT_STATUS_RESET;
            }
            break;
        }
    }
  }
}
