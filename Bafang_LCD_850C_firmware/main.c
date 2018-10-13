/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stdio.h"

#define TEST__PIN               GPIO_Pin_3
#define TEST__PORT              GPIOB

int main(void)
{
  /* Enable clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = TEST__PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(TEST__PORT, &GPIO_InitStructure);

  while (1)
  {
    GPIO_ResetBits(TEST__PORT, TEST__PIN);
    GPIO_SetBits(TEST__PORT, TEST__PIN);
  }
}

