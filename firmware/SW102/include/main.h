#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_

#include "common.h"
#include "button.h"

/* Typedef of unions for handy access of single bytes */
/* Access bytewise: U16 var; var.byte[x] = z; */
/* Access value: U32 var; var.u32 = 0xFFFFFFFF; */
typedef union
{
  uint16_t u16;
  uint8_t byte[2];
} U16;

typedef union
{
  uint32_t u32;
  uint8_t byte[4];
} U32;

//#define  MAIN_SCREEN_FIELD_LABELS_COLOR C_GRAY
#define  MAIN_SCREEN_FIELD_LABELS_COLOR C_WHITE_SMOKE

void system_power(bool state);

uint32_t get_seconds(); // how many seconds since boot

/// msecs since boot (note: will roll over every 50 days)
uint32_t get_msecs();

void SW102_rt_processing_stop(void);
void SW102_rt_processing_start(void);

extern Button buttonM, buttonDWN, buttonUP, buttonPWR;

#endif /* INCLUDE_MAIN_H_ */
