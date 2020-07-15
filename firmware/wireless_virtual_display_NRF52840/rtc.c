/*
 * SW102 firmware
 * Copyright (C) Casainho, geeksville and lowPerformer, 2018. 
 *
 * TSDZ2 EBike wireless firmware
 * Copyright (C) Casainho, 2020
 *
 * Released under the GPL License, Version 3
 */

#include "main.h"
#include "rtc.h"

// FIXME - have everyone call get_seconds instead
uint32_t ui32_seconds_since_startup = 0;

void rtc_init()
{
  // FIXME: Do something?
}

void rtc_set_time(rtc_time_t *rtc_time)
{
  // FIXME: Do something?
}

rtc_time_t* rtc_get_time(void)
{
  static rtc_time_t rtc_time;

  // FIXME - Implement own counter?

  return &rtc_time;
}

rtc_time_t* rtc_get_time_since_startup(void)
{
  uint32_t ui32_temp = get_seconds();
  static rtc_time_t rtc_time;

  rtc_time.ui8_hours = ui32_temp / 3600;
  rtc_time.ui8_minutes = (ui32_temp % 3600) / 60;

  return &rtc_time;
}
