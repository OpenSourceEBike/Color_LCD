/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "usart1.h"

#include "stm32f10x.h"
#include "pins.h"
#include "stm32f10x_usart.h"
#include "lcd.h"

volatile uint8_t ui8_received_package_flag = 0;
volatile uint8_t ui8_rx_buffer[26];
volatile uint8_t ui8_rx_counter = 0;
volatile uint8_t ui8_tx_buffer[11];
volatile uint8_t ui8_tx_counter = 0;
volatile uint8_t ui8_i;
volatile uint8_t ui8_checksum;
static uint16_t ui16_crc_rx;
static uint16_t ui16_crc_tx;
static uint8_t ui8_lcd_variable_id = 0;
static uint8_t ui8_master_comm_package_id = 0;
static uint8_t ui8_slave_comm_package_id = 0;
volatile uint8_t ui8_byte_received;
volatile uint8_t ui8_state_machine = 0;
volatile uint8_t ui8_usart1_received_first_package = 0;

void usart1_init(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  // USART for bluetooth module pins
  GPIO_InitStructure.GPIO_Pin = USART1_RX__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USART1__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = USART1_TX__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USART1__PORT, &GPIO_InitStructure);

  // enable GPIO clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);

  USART_DeInit(USART1);

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);

  // enable the USARTy Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART1_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  USART_ClearITPendingBit(USART1, USART_IT_TXE);

  // enable USART1 Receive and Transmit interrupts
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  // enable the USART1
  USART_Cmd(USART1, ENABLE);
}

// USART1 Tx and Rx interrupt handler.
void USART1_IRQHandler()
{
  // The interrupt may be from Tx, Rx, or both.
  if (USART_GetITStatus(USART1, USART_IT_ORE) == SET)
  {
    USART_ReceiveData(USART1); // get ride of this interrupt flag
    return;
  }
  else if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
  {
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    return;
  }
  else if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
  {
    // receive byte
    ui8_byte_received = (uint8_t) USART1->DR;

    switch (ui8_state_machine)
    {
      case 0:
      if (ui8_byte_received == 67) // see if we get start package byte
      {
        ui8_rx_buffer[ui8_rx_counter] = ui8_byte_received;
        ui8_rx_counter++;
        ui8_state_machine = 1;
      }
      else
      {
        ui8_rx_counter = 0;
        ui8_state_machine = 0;
      }
      break;

      case 1:
      ui8_rx_buffer[ui8_rx_counter] = ui8_byte_received;
      ui8_rx_counter++;

      // see if is the last byte of the package
      if (ui8_rx_counter > 27)
      {
        ui8_rx_counter = 0;
        ui8_state_machine = 0;
        ui8_received_package_flag = 1; // signal that we have a full package to be processed
        USART_ITConfig(USART1, USART_IT_RXNE, DISABLE); // disable USART receive interrupt
      }
      break;

      default:
      break;
    }
  }
}

void usart1_data_clock (void)
{
  static uint32_t ui32_wss_tick_temp;
  struct_motor_controller_data *p_motor_controller_data;
  struct_configuration_variables *p_configuration_variables;

  if (ui8_received_package_flag)
  {
    // validation of the package data
    // last byte is the checksum
    ui16_crc_rx = 0xffff;
    for (ui8_i = 0; ui8_i <= 23; ui8_i++)
    {
      crc16 (ui8_rx_buffer[ui8_i], &ui16_crc_rx);
    }

    if (((((uint16_t) ui8_rx_buffer [25]) << 8) + ((uint16_t) ui8_rx_buffer [24])) == ui16_crc_rx)
    {
      p_motor_controller_data = get_motor_controller_data ();
      p_configuration_variables = get_configuration_variables ();

      // send a variable for each package sent but first verify if the last one was received otherwise, keep repeating
      // keep cycling so all variables are sent
#define VARIABLE_ID_MAX_NUMBER 9
      if ((ui8_rx_buffer [1]) == ui8_master_comm_package_id) // last package data ID was receipt, so send the next one
      {
        ui8_master_comm_package_id = (ui8_master_comm_package_id + 1) % VARIABLE_ID_MAX_NUMBER;
      }

      ui8_slave_comm_package_id = ui8_rx_buffer[2];

      p_motor_controller_data->ui16_adc_battery_voltage = ui8_rx_buffer[3];
      p_motor_controller_data->ui16_adc_battery_voltage |= ((uint16_t) (ui8_rx_buffer[4] & 0x30)) << 4;
      p_motor_controller_data->ui8_battery_current_x5 = ui8_rx_buffer[5];
      p_motor_controller_data->ui16_wheel_speed_x10 = (((uint16_t) ui8_rx_buffer [7]) << 8) + ((uint16_t) ui8_rx_buffer [6]);
      p_motor_controller_data->ui8_motor_controller_state_2 = ui8_rx_buffer[8];
      p_motor_controller_data->ui8_braking = p_motor_controller_data->ui8_motor_controller_state_2 & 1;

      if (p_configuration_variables->ui8_temperature_limit_feature_enabled)
      {
        p_motor_controller_data->ui8_adc_throttle = ui8_rx_buffer[9];
        p_motor_controller_data->ui8_motor_temperature = ui8_rx_buffer[10];
      }
      else
      {
        p_motor_controller_data->ui8_adc_throttle = ui8_rx_buffer[9];
        p_motor_controller_data->ui8_throttle = ui8_rx_buffer[10];
      }

      p_motor_controller_data->ui8_adc_pedal_torque_sensor = ui8_rx_buffer[11];
      p_motor_controller_data->ui8_pedal_torque_sensor = ui8_rx_buffer[12];
      p_motor_controller_data->ui8_pedal_cadence = ui8_rx_buffer[13];
      p_motor_controller_data->ui8_pedal_human_power = ui8_rx_buffer[14];
      p_motor_controller_data->ui8_duty_cycle = ui8_rx_buffer[15];
      p_motor_controller_data->ui16_motor_speed_erps = (((uint16_t) ui8_rx_buffer [17]) << 8) + ((uint16_t) ui8_rx_buffer [16]);
      p_motor_controller_data->ui8_foc_angle = ui8_rx_buffer[18];

      switch (ui8_slave_comm_package_id)
      {
        case 0:
          // error states
          p_motor_controller_data->ui8_error_states = ui8_rx_buffer[19];
        break;

        case 1:
          // temperature actual limiting value
          p_motor_controller_data->ui8_temperature_current_limiting_value = ui8_rx_buffer[19];
        break;

        case 2:
          // wheel_speed_sensor_tick_counter
          ui32_wss_tick_temp = ((uint32_t) ui8_rx_buffer[19]);
        break;

        case 3:
          // wheel_speed_sensor_tick_counter
          ui32_wss_tick_temp |= (((uint32_t) ui8_rx_buffer[19]) << 8);
        break;

        case 4:
          // wheel_speed_sensor_tick_counter
          ui32_wss_tick_temp |= (((uint32_t) ui8_rx_buffer[19]) << 16);
          p_motor_controller_data->ui32_wheel_speed_sensor_tick_counter = ui32_wss_tick_temp;
        break;
      }

      // ui16_pedal_torque_x10
      p_motor_controller_data->ui16_pedal_torque_x10 = (((uint16_t) ui8_rx_buffer [21]) << 8) + ((uint16_t) ui8_rx_buffer [20]);
      // ui16_pedal_power_x10
      p_motor_controller_data->ui16_pedal_power_x10 = (((uint16_t) ui8_rx_buffer [23]) << 8) + ((uint16_t) ui8_rx_buffer [22]);

      // signal that we processed the full package
      ui8_received_package_flag = 0;

      // now send the data to the motor controller
      // start up byte
      ui8_tx_buffer[0] = 0x59;
      ui8_tx_buffer[1] = ui8_master_comm_package_id;
      ui8_tx_buffer[2] = ui8_slave_comm_package_id;

      // set assist level value
      if (p_configuration_variables->ui8_assist_level)
      {
        ui8_tx_buffer[3] = p_configuration_variables->ui8_assist_level_factor [((p_configuration_variables->ui8_assist_level) - 1)];
      }
      else
      {
        ui8_tx_buffer[3] = 0;
      }

      // set lights state
      // walk assist level state
      // set offroad state
      ui8_tx_buffer[4] = (p_motor_controller_data->ui8_lights & 1) |
          ((p_motor_controller_data->ui8_walk_assist_level & 1) << 1) |
          ((p_motor_controller_data->ui8_offroad_mode & 1) << 2);

      // battery max current in amps
      ui8_tx_buffer[5] = p_configuration_variables->ui8_battery_max_current;

      // battery power
      ui8_tx_buffer[6] = p_configuration_variables->ui8_target_max_battery_power;

      switch (ui8_master_comm_package_id)
      {
        case 0:
          // battery low voltage cut-off
          ui8_tx_buffer[7] = (uint8_t) (p_configuration_variables->ui16_battery_low_voltage_cut_off_x10 & 0xff);
          ui8_tx_buffer[8] = (uint8_t) (p_configuration_variables->ui16_battery_low_voltage_cut_off_x10 >> 8);
        break;

        case 1:
          // wheel perimeter
          ui8_tx_buffer[7] = (uint8_t) (p_configuration_variables->ui16_wheel_perimeter & 0xff);
          ui8_tx_buffer[8] = (uint8_t) (p_configuration_variables->ui16_wheel_perimeter >> 8);
        break;

        case 2:
          // wheel max speed
          ui8_tx_buffer[7] = p_configuration_variables->ui8_wheel_max_speed;
        break;

        case 3:
          // bit 0: cruise control
          // bit 1: motor voltage type: 36V or 48V
          // bit 2: MOTOR_ASSISTANCE_CAN_START_WITHOUT_PEDAL_ROTATION
          ui8_tx_buffer[7] = ((p_configuration_variables->ui8_cruise_control & 1) |
                             ((p_configuration_variables->ui8_motor_type & 3) << 1) |
                              ((p_configuration_variables->ui8_motor_assistance_startup_without_pedal_rotation & 1) << 3) |
                              ((p_configuration_variables->ui8_temperature_limit_feature_enabled & 1) << 4));
          ui8_tx_buffer[8] = p_configuration_variables->ui8_startup_motor_power_boost_state;
        break;

        case 4:
          // startup motor power boost
          ui8_tx_buffer[7] = p_configuration_variables->ui8_startup_motor_power_boost_factor [((p_configuration_variables->ui8_assist_level) - 1)];
          // startup motor power boost time
          ui8_tx_buffer[8] = p_configuration_variables->ui8_startup_motor_power_boost_time;
        break;

        case 5:
          // startup motor power boost fade time
          ui8_tx_buffer[7] = p_configuration_variables->ui8_startup_motor_power_boost_fade_time;
          // boost feature enabled
          ui8_tx_buffer[8] = (p_configuration_variables->ui8_startup_motor_power_boost_feature_enabled & 1) ? 1 : 0;
        break;

        case 6:
          // motor over temperature min and max values to limit
          ui8_tx_buffer[7] = p_configuration_variables->ui8_motor_temperature_min_value_to_limit;
          ui8_tx_buffer[8] = p_configuration_variables->ui8_motor_temperature_max_value_to_limit;
        break;

        case 7:
          // offroad mode configuration
          ui8_tx_buffer[7] = ((p_configuration_variables->ui8_offroad_feature_enabled & 1) |
                                ((p_configuration_variables->ui8_offroad_enabled_on_startup & 1) << 1));
          ui8_tx_buffer[8] = p_configuration_variables->ui8_offroad_speed_limit;
        break;

        case 8:
          // offroad mode power limit configuration
          ui8_tx_buffer[7] = p_configuration_variables->ui8_offroad_power_limit_enabled & 1;
          ui8_tx_buffer[8] = p_configuration_variables->ui8_offroad_power_limit_div25;
        break;

        default:
          ui8_lcd_variable_id = 0;
        break;
      }

      // prepare crc of the package
      ui16_crc_tx = 0xffff;
      for (ui8_i = 0; ui8_i <= 8; ui8_i++)
      {
        crc16 (ui8_tx_buffer[ui8_i], &ui16_crc_tx);
      }
      ui8_tx_buffer[9] = (uint8_t) (ui16_crc_tx & 0xff);
      ui8_tx_buffer[10] = (uint8_t) (ui16_crc_tx >> 8) & 0xff;

      // send the full package to UART
      for (ui8_i = 0; ui8_i <= 10; ui8_i++)
      {
        // wait for any previous data to be sent
        while (USART_GetFlagStatus (USART1, USART_FLAG_TXE) == RESET) ;
        USART_SendData (USART1, ui8_tx_buffer[ui8_i]);
      }

      // let's wait for 10 packages, seems that first ADC battery voltage is an incorrect value
      ui8_usart1_received_first_package++;
      if (ui8_usart1_received_first_package > 10)
        ui8_usart1_received_first_package = 10;
    }

    // enable USART1 receive interrupt as we are now ready to receive a new package
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // disable USART receive interrupt
  }
}

uint8_t usart1_received_first_package (void)
{
  return (ui8_usart1_received_first_package == 10) ? 1: 0;
}
