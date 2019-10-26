/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stdio.h"
#include "main.h"
#include "stm32f10x_bkp.h"
#include "rtc.h"

#define SECONDS_IN_DAY 86399
#define CONFIGURATION_DONE 0xAAAA
#define CONFIGURATION_RESET 0x0000

uint32_t ui32_seconds_since_startup = 0;

void RTC_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(RTC_IRQn);
  RTC_ClearITPendingBit(RTC_IT_SEC);

  // reset counter if more than 1 day passed in power down/Low Power Mode
  if((RTC_GetCounter() / SECONDS_IN_DAY) != 0)
  {
    RTC_WaitForLastTask();
    RTC_SetCounter(RTC_GetCounter() % SECONDS_IN_DAY);
    RTC_WaitForLastTask();
  }

  ui32_seconds_since_startup++;
}

void rtc_init()
{
  NVIC_InitTypeDef NVIC_InitStructure;
  uint16_t WaitForOscSource;

  /*Enables the clock to Backup and power interface peripherals    */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);

  /* Configure one bit for preemption priority */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = RTC_INTERRUT_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /*Allow access to Backup Registers*/
  PWR_BackupAccessCmd(ENABLE);

  if(BKP_ReadBackupRegister(BKP_DR1) == CONFIGURATION_RESET)
  {
    /*Enables the clock to Backup and power interface peripherals    */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR,ENABLE);

    /* Backup Domain Reset */
    BKP_DeInit();

    /*Enable 32.768 kHz external oscillator */
    RCC_LSEConfig(RCC_LSE_ON);

    for(WaitForOscSource = 0; WaitForOscSource < 5000; WaitForOscSource++) { }

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    /* RTC Enabled */
    RCC_RTCCLKCmd(ENABLE);
    RTC_WaitForLastTask();
    /*Wait for RTC registers synchronisation */
    RTC_WaitForSynchro();
    RTC_WaitForLastTask();
    /* Setting RTC Interrupts-Seconds interrupt enabled */
    /* Enable the RTC Second */
    RTC_ITConfig(RTC_IT_SEC , ENABLE);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Set RTC prescaler: set RTC period to 1 sec */
    RTC_SetPrescaler(32765); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
    /* Prescaler is set to 32766 instead of 32768 to compensate for
      lower as well as higher frequencies*/
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    // set default time to 0h0m0s
    RTC_WaitForLastTask();
    rtc_time_t rtc_time =
      {
        .ui8_hours = 0,
        .ui8_minutes = 0
      };
    rtc_set_time(&rtc_time);
    RTC_WaitForLastTask();

    BKP_WriteBackupRegister(BKP_DR1, CONFIGURATION_DONE);
  }
  else
  {
    /* PWR and BKP clocks selection */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    for(WaitForOscSource = 0; WaitForOscSource < 5000; WaitForOscSource++) { }
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Enable the RTC Second */
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    RTC_WaitForLastTask();
  }

  // reset counter if more than 1 day passed in power down/Low Power Mode
  if((RTC_GetCounter() / SECONDS_IN_DAY) != 0)
  {
    RTC_WaitForLastTask();
    RTC_SetCounter(RTC_GetCounter() % SECONDS_IN_DAY);
    RTC_WaitForLastTask();
  }

  BKP_RTCOutputConfig(BKP_RTCOutputSource_None);
}

void rtc_set_time(rtc_time_t *rtc_time)
{
  RTC_WaitForLastTask();
  RTC_SetCounter((((uint32_t) rtc_time->ui8_hours) * 3600) + (((uint32_t) rtc_time->ui8_minutes) * 60));
  RTC_WaitForLastTask();
}

rtc_time_t* rtc_get_time(void)
{
  uint32_t ui32_temp;
  static rtc_time_t rtc_time;

  ui32_temp = RTC_GetCounter() % SECONDS_IN_DAY;
  rtc_time.ui8_hours = ui32_temp / 3600;
  rtc_time.ui8_minutes = (ui32_temp % 3600) / 60;

  return &rtc_time;
}

rtc_time_t* rtc_get_time_since_startup(void)
{
  uint32_t ui32_temp;
  static rtc_time_t rtc_time;

  ui32_temp = ui32_seconds_since_startup % SECONDS_IN_DAY;
  rtc_time.ui8_hours = ui32_temp / 3600;
  rtc_time.ui8_minutes = (ui32_temp % 3600) / 60;

  return &rtc_time;
}
