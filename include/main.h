#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_

#include "boards.h"
#include "nrf.h"
#include "nrf51822_peripherals.h"
#include "nordic_common.h"
#include "app_util_platform.h"


/*  Peripheral usage by SoftDevice 130 */
/* Blocked
 *  TIMER0
 *  RTC0
 *  CCM
 *  AAR
 *  SWI2, 4, 5
 *  FICR
*/
/* Restricted
 *  TEMP
 *  RNG
 *  ECB
 *  SWI1
 *  NVMC
 *  UICR
 *  NVIC
 */

/* TIMER / RTC */


/* SPI CPU -> LCD controller */
#define LCD_SPI_INSTANCE    0

/* UART Display <-> Motor */
#define UART0   0

#endif /* INCLUDE_MAIN_H_ */
