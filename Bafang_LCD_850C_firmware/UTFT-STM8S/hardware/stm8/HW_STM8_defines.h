#include "stm32f10x_gpio.h"

// *** Hardwarespecific defines ***
#define cbi(reg, bitmask) (GPIO_ResetBits(reg, bitmask));
#define sbi(reg, bitmask) (GPIO_SetBits(reg, bitmask));

#define pulse_high(reg, bitmask) sbi(reg, bitmask); delay(10); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); delay(10); sbi(reg, bitmask);

#define cport(port, data) port &= data
#define sport(port, data) port |= data

#define swap(type, i, j) {type t = i; i = j; j = t;}

#define pgm_read_word(data) *data
#define pgm_read_byte(data) *data

#define fontbyte(x) UTFT_cfont.font[x]

// for STM8
//#define regtype volatile uint8_t
//#define regsize uint8_t

// for STM32F103
#define regtype volatile uint32_t
#define regsize uint32_t

#define bitmapdatatype unsigned int*

#define sin(x) sinf(x)
#define cos(x) cosf(x)
