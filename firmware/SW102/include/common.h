#ifndef INCLUDE_DEFINITIONS_H_
#define INCLUDE_DEFINITIONS_H_

#include "boards.h"

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
#define APP_TIMER_PRESCALER       0   /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE   4   /**< Size of timer operation queues. */

/* SPI CPU -> LCD controller */
#define LCD_SPI_INSTANCE    0

/* UART Display <-> Motor */
#define UART0   0

extern bool useSoftDevice;

#endif /* INCLUDE_DEFINITIONS_H_ */
