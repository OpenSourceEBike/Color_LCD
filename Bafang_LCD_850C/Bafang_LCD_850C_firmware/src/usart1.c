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
#include "utils.h"
#include "usart1.h"
#include "main.h"

static volatile uint8_t ui8_rx[26];
volatile uint8_t ui8_rx_buffer[24];
static volatile uint8_t ui8_rx_counter = 0;
static volatile uint8_t ui8_tx[11];
volatile uint8_t ui8_tx_buffer[11];
static volatile uint8_t ui8_tx_counter = 0;
static volatile uint8_t ui8_i;
static volatile uint8_t ui8_checksum;
static uint16_t ui16_crc_rx;
static uint16_t ui16_crc_tx;
static uint8_t ui8_lcd_variable_id = 0;
static uint8_t ui8_master_comm_package_id = 0;
static uint8_t ui8_slave_comm_package_id = 0;
static volatile uint8_t ui8_byte_received;
static volatile uint8_t ui8_state_machine = 0;
static volatile uint8_t ui8_usart1_received_first_package = 0;

static uart_rx_vars_t *mp_uart_rx_data;

void usart1_init(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  // USART pins
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

  // enable the USART Interrupt
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART1_INTERRUPT_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  USART_ClearITPendingBit(USART1, USART_IT_TXE);

  // enable USART Receive and Transmit interrupts
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  // enable the USART
  USART_Cmd(USART1, ENABLE);
}

// USART1 Tx and Rx interrupt handler.
void USART1_IRQHandler()
{
  // The interrupt may be from Tx, Rx, or both.
  if(USART_GetITStatus(USART1, USART_IT_ORE) == SET)
  {
    USART_ReceiveData(USART1); // get ride of this interrupt flag
    return;
  }
  else if(USART_GetITStatus(USART1, USART_IT_TXE) == SET)
  {
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
    return;
  }
  else if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
  {
    // receive byte
    ui8_byte_received = (uint8_t) USART1->DR;

    switch(ui8_state_machine)
    {
      case 0:
      if(ui8_byte_received == 67) // see if we get start package byte
      {
        ui8_rx[ui8_rx_counter] = ui8_byte_received;
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
      ui8_rx[ui8_rx_counter] = ui8_byte_received;
      ui8_rx_counter++;

      // see if is the last byte of the package
      if(ui8_rx_counter > 27)
      {
        ui8_rx_counter = 0;
        ui8_state_machine = 0;

        // validation of the package data
        // last byte is the checksum
        ui16_crc_rx = 0xffff;
        for(ui8_i = 0; ui8_i <= 23; ui8_i++)
        {
          crc16(ui8_rx[ui8_i], &ui16_crc_rx);
        }

        if(((((uint16_t) ui8_rx [25]) << 8) + ((uint16_t) ui8_rx [24])) == ui16_crc_rx)
        {
          // now store the received data to rx_buffer
          memcpy(&ui8_rx_buffer[0], &ui8_rx[0], 24);

          // TODO: sent TX buffer
//          // prepare crc of the package
//          ui16_crc_tx = 0xffff;
//          for(ui8_i = 0; ui8_i <= 8; ui8_i++)
//          {
//            crc16 (ui8_tx[ui8_i], &ui16_crc_tx);
//          }
//          ui8_tx[9] = (uint8_t) (ui16_crc_tx & 0xff);
//          ui8_tx[10] = (uint8_t) (ui16_crc_tx >> 8) & 0xff;
//
//          // send the full package to UART
//          for(ui8_i = 0; ui8_i <= 10; ui8_i++)
//          {
//            // wait for any previous data to be sent
//            while (USART_GetFlagStatus (USART1, USART_FLAG_TXE) == RESET) ;
//            USART_SendData (USART1, ui8_tx[ui8_i]);
//          }

        }
        else
      }
      break;

      default:
      break;
    }
  }
}

uint8_t* usart1_get_rx_buffer(void)
{
  return &ui8_rx_buffer[0];
}
