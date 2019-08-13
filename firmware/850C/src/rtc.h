/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef _RTC_H_
#define _RTC_H_

#include "stdio.h"
#include "rtc.h"

typedef struct _rtc_time
{
  uint8_t ui8_hours;
  uint8_t ui8_minutes;
} struct_rtc_time_t;

void rtc_init(void);
void rtc_set_time(struct_rtc_time_t *rtc_time);
struct_rtc_time_t* rtc_get_time(void);
struct_rtc_time_t* rtc_get_time_since_startup(void);

#endif /* _RTC_H_ */
