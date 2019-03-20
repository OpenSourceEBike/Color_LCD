/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
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

uint8_t ui8_rx_buffer[24];
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
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)USART1;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ui8_g_usart1_tx_buffer;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 11;
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

//  // enable DMA USART1 interrupt
//  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART1_DMA_INTERRUPT_PRIORITY;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);

  USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  USART_ClearITPendingBit(USART1, USART_IT_TXE);
//  DMA_ClearITPendingBit(DMA1_IT_TC4);

  // enable the USART
  USART_Cmd(USART1, ENABLE);

  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
  DMA_Cmd(DMA1_Channel4, ENABLE);
  USART_Cmd(USART1, ENABLE);

  // enable USART Receive and Transmit interrupts
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//  DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
}

// USART1 Tx and Rx interrupt handler.
void USART1_IRQHandler()
{
  uint8_t ui8_byte_received;
  static uint8_t ui8_state_machine = 0;
  static uint8_t ui8_rx[26];
  static uint8_t ui8_rx_counter = 0;
  uint8_t ui8_i;
  uint16_t ui16_crc_rx;

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

        if(((((uint16_t) ui8_rx[25]) << 8) + ((uint16_t) ui8_rx[24])) == ui16_crc_rx)
        {
          // copy to the other buffer only if we processed already the last package
          if(!ui8_received_package_flag)
          {
            ui8_received_package_flag = 1;

            // store the received data to rx_buffer
            memcpy(ui8_rx_buffer, ui8_rx, 24);
          }
        }
      }
      break;

      default:
      break;
    }
  }
}

void DMA1_Channel4_IRQHandler()
{
  if(DMA_GetITStatus(DMA1_IT_TC4) == SET)
  {
    ui32_g_dma_usart_tx_ongoing = 0;
    DMA_ClearITPendingBit(DMA1_IT_TC4 | DMA1_IT_GL4);
  }

  DMA_ClearITPendingBit(DMA1_IT_TC4);
  DMA_ClearITPendingBit(DMA1_IT_GL4);
  DMA_ClearITPendingBit(DMA1_IT_HT4);
  DMA_ClearITPendingBit(DMA1_IT_TE4);

  DMA_Cmd(DMA1_Channel4, DISABLE);
}

void usart1_start_dma_transfer(void)
{
  DMA_Cmd(DMA1_Channel4, DISABLE);
  DMA_SetCurrDataCounter(DMA1_Channel4, 11);
  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
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

