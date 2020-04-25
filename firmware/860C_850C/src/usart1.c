/*
 * Bafang LCD 860C/850C firmware
 *
 * Copyright (C) Casainho, 2018, 2019, 2020
 *
 * Released under the GPL License, Version 3
 */

#include <string.h>
#include "usart1.h"
#include "stm32f10x.h"
#include "pins.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "lcd.h"
#include "utils.h"
#include "usart1.h"
#include "main.h"
#include "uart.h"

uint8_t ui8_rx_buffer[UART_NUMBER_DATA_BYTES_TO_RECEIVE];
volatile uint8_t ui8_received_package_flag = 0;

void usart1_init(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;

  // enable GPIO clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  DMA_DeInit(DMA1_Channel4);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART1->DR);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) uart_get_tx_buffer();
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = UART_NUMBER_DATA_BYTES_TO_SEND;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel4, &DMA_InitStructure);

  // USART pins
  GPIO_InitStructure.GPIO_Pin = USART1_RX__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USART1__PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = USART1_TX__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USART1__PORT, &GPIO_InitStructure);

  USART_DeInit(USART1);
  USART_InitStructure.USART_BaudRate = 19200;
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
  USART_ClearITPendingBit(USART1, USART_IT_TC);

  // enable the USART
  USART_Cmd(USART1, ENABLE);

  DMA_Cmd(DMA1_Channel4, ENABLE);
  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
  USART_Cmd(USART1, ENABLE);

  // enable USART Receive and Transmit interrupts
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

// USART1 Tx and Rx interrupt handler.
void USART1_IRQHandler()
{
  uint8_t ui8_byte_received;
  static uint8_t ui8_state_machine = 0;
  static uint8_t ui8_rx[UART_NUMBER_DATA_BYTES_TO_RECEIVE];
  static uint8_t ui8_rx_cnt = 0;
  uint8_t ui8_i;
  uint16_t ui16_crc_rx;

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
      if (ui8_byte_received == 0x43) { // see if we get start package byte
        ui8_rx[0] = ui8_byte_received;
        ui8_state_machine = 1;
      }
      else
        ui8_state_machine = 0;
      break;

      case 1:
        ui8_rx[1] = ui8_byte_received;
        ui8_state_machine = 2;
      break;

      case 2:
      ui8_rx[ui8_rx_cnt + 2] = ui8_byte_received;
      ++ui8_rx_cnt;

      // reset if it is the last byte of the package and index is out of bounds
      if (ui8_rx_cnt >= ui8_rx[1])
      {
        ui8_rx_cnt = 0;
        ui8_state_machine = 0;

        // just to make easy next calculations
        ui16_crc_rx = 0xffff;
        for (ui8_i = 0; ui8_i < ui8_rx[1]; ui8_i++)
        {
          crc16(ui8_rx[ui8_i], &ui16_crc_rx);
        }

        // if CRC is correct read the package
        if (((((uint16_t) ui8_rx[ui8_rx[1] + 1]) << 8) +
              ((uint16_t) ui8_rx[ui8_rx[1]])) == ui16_crc_rx)
        {
          // copy to the other buffer only if we processed already the last package
          if (ui8_received_package_flag == 0)
          {
            ui8_received_package_flag = 1;

            // store the received data to rx_buffer
            memcpy(ui8_rx_buffer, ui8_rx, ui8_rx[1] + 2);
          }
        }
      }
    }
  }
}

void usart1_start_dma_transfer(uint8_t ui8_len)
{
  DMA_Cmd(DMA1_Channel4, DISABLE);
  DMA_SetCurrDataCounter(DMA1_Channel4, ui8_len);
  DMA_Cmd(DMA1_Channel4, ENABLE);
}

uint8_t* usart1_get_rx_buffer(void)
{
  return ui8_rx_buffer;
}

uint8_t usart1_received_package(void)
{
  return ui8_received_package_flag;
}

void usart1_reset_received_package(void)
{
  ui8_received_package_flag = 0;
}
