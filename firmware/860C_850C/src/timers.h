/*
 * Bafang LCD 860C/850C firmware
 *
 * Copyright (C) Casainho, 2018, 2019, 2020
 *
 * Released under the GPL License, Version 3
 */

#ifndef TIMERS_H_
#define TIMERS_H_

void systick_init (void);

void delay_ms (uint32_t ms);
void timer3_init(void);
void timer4_init(void);
void Display850C_rt_processing_stop(void);
void Display850C_rt_processing_start(void);

#endif /* TIMERS_H_ */
