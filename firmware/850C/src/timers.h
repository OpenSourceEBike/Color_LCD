/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef TIMERS_H_
#define TIMERS_H_

void systick_init (void);
volatile uint32_t get_time_base_counter_1ms (void);
void delay_ms (uint32_t ms);
void timer3_init(void);
void timer4_init(void);
void Display850C_rt_processing_stop(void);
void Display850C_rt_processing_start(void);

#endif /* TIMERS_H_ */
