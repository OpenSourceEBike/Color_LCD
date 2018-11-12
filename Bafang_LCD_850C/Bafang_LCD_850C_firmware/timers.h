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
uint32_t get_timer_base_counter_1ms (void);
void delay_ms (uint32_t ms);

#endif /* TIMERS_H_ */
