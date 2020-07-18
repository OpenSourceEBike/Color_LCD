#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdbool.h>

void system_power(bool state);

uint32_t get_seconds(); // how many seconds since boot
uint32_t get_time_base_counter_1ms();

void wireless_virtual_rt_processing_stop(void);
void wireless_virtual_rt_processing_start(void);

#endif /* MAIN_H_ */
