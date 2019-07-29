#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_

#include "common.h"

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


void system_power(bool state);

extern volatile uint32_t seconds_since_startup, seconds_since_reset;

#endif /* INCLUDE_MAIN_H_ */
