/*
 * SW102 firmware
 *
 * Copyright (C) Casainho, geeksville and lowPerformer, 2018.
 *
 * Released under the GPL License, Version 3
 */

/*
 * SW102 does not have a battery buffered RTC.
 * So it's all fake! Welcome to reality!.
 */

#include "rtc.h"
#include "main.h"

// FIXME - have everyone call get_seconds instead
uint32_t ui32_seconds_since_startup = 0;

void rtc_init()
{
  // FIXME - Do something?
}

void rtc_set_time(rtc_time_t *rtc_time)
{
  // FIXME - Do something?
}

rtc_time_t* rtc_get_time(void)
{
  static rtc_time_t rtc_time;

  // FIXME - Implement extra counter which we can set in rtc_set_time?

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
