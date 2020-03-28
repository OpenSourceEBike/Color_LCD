#include "timer.h"

/// msecs since boot (note: will roll over every 50 days)
uint32_t get_time_base_counter_1ms (void)
{
  return time_base_counter_1ms;
}
