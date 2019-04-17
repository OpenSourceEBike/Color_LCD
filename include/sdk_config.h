/**
 * Copyright (c) 2017 - 2017, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H
// <<< Use Configuration Wizard in Context Menu >>>\n
#ifdef USE_APP_CONFIG
#include "app_config.h"
#endif

// </h>
//==========================================================

// </h>
//==========================================================

// <h> nRF_Drivers

// <e> CLOCK_ENABLED - nrf_drv_clock - CLOCK peripheral driver
//==========================================================
#ifndef CLOCK_ENABLED
#define CLOCK_ENABLED 0
#endif
// <o> CLOCK_CONFIG_XTAL_FREQ  - HF XTAL Frequency

// <0=> Default (64 MHz)

#ifndef CLOCK_CONFIG_XTAL_FREQ
#define CLOCK_CONFIG_XTAL_FREQ 0
#endif

// <o> CLOCK_CONFIG_LF_SRC  - LF Clock Source

// <0=> RC
// <1=> XTAL
// <2=> Synth

#ifndef CLOCK_CONFIG_LF_SRC
#define CLOCK_CONFIG_LF_SRC 1
#endif

// <o> CLOCK_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef CLOCK_CONFIG_IRQ_PRIORITY
#define CLOCK_CONFIG_IRQ_PRIORITY 7
#endif

// </e>

// <e> GPIOTE_ENABLED - nrf_drv_gpiote - GPIOTE peripheral driver
//==========================================================
#ifndef GPIOTE_ENABLED
#define GPIOTE_ENABLED 0
#endif
// <o> GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS - Number of lower power input pins
#ifndef GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS
#define GPIOTE_CONFIG_NUM_OF_LOW_POWER_EVENTS 4
#endif

// <o> GPIOTE_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef GPIOTE_CONFIG_IRQ_PRIORITY
#define GPIOTE_CONFIG_IRQ_PRIORITY 7
#endif

// </e>

// <e> POWER_ENABLED - nrf_drv_power - POWER peripheral driver
//==========================================================
#ifndef POWER_ENABLED
#define POWER_ENABLED 0
#endif
// <o> POWER_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef POWER_CONFIG_IRQ_PRIORITY
#define POWER_CONFIG_IRQ_PRIORITY 7
#endif

// <q> POWER_CONFIG_DEFAULT_DCDCEN  - The default configuration of main DCDC regulator


// <i> This settings means only that components for DCDC regulator are installed and it can be enabled.

#ifndef POWER_CONFIG_DEFAULT_DCDCEN
#define POWER_CONFIG_DEFAULT_DCDCEN 0
#endif

// <q> POWER_CONFIG_DEFAULT_DCDCENHV  - The default configuration of High Voltage DCDC regulator


// <i> This settings means only that components for DCDC regulator are installed and it can be enabled.

#ifndef POWER_CONFIG_DEFAULT_DCDCENHV
#define POWER_CONFIG_DEFAULT_DCDCENHV 0
#endif

// </e>

// <q> PPI_ENABLED  - nrf_drv_ppi - PPI peripheral driver


#ifndef PPI_ENABLED
#define PPI_ENABLED 0
#endif

// <e> RNG_ENABLED - nrf_drv_rng - RNG peripheral driver
//==========================================================
#ifndef RNG_ENABLED
#define RNG_ENABLED 0
#endif
// <q> RNG_CONFIG_ERROR_CORRECTION  - Error correction


#ifndef RNG_CONFIG_ERROR_CORRECTION
#define RNG_CONFIG_ERROR_CORRECTION 0
#endif

// <o> RNG_CONFIG_POOL_SIZE - Pool size
#ifndef RNG_CONFIG_POOL_SIZE
#define RNG_CONFIG_POOL_SIZE 32
#endif

// <o> RNG_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef RNG_CONFIG_IRQ_PRIORITY
#define RNG_CONFIG_IRQ_PRIORITY 7
#endif

// </e>

// <e> RTC_ENABLED - nrf_drv_rtc - RTC peripheral driver
//==========================================================
#ifndef RTC_ENABLED
#define RTC_ENABLED 0
#endif
// <o> RTC_DEFAULT_CONFIG_FREQUENCY - Frequency  <16-32768>


#ifndef RTC_DEFAULT_CONFIG_FREQUENCY
#define RTC_DEFAULT_CONFIG_FREQUENCY 32768
#endif

// <q> RTC_DEFAULT_CONFIG_RELIABLE  - Ensures safe compare event triggering


#ifndef RTC_DEFAULT_CONFIG_RELIABLE
#define RTC_DEFAULT_CONFIG_RELIABLE 0
#endif

// <o> RTC_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef RTC_DEFAULT_CONFIG_IRQ_PRIORITY
#define RTC_DEFAULT_CONFIG_IRQ_PRIORITY 7
#endif

// <q> RTC0_ENABLED  - Enable RTC0 instance


#ifndef RTC0_ENABLED
#define RTC0_ENABLED 0
#endif

// <q> RTC1_ENABLED  - Enable RTC1 instance


#ifndef RTC1_ENABLED
#define RTC1_ENABLED 0
#endif

// <o> NRF_MAXIMUM_LATENCY_US - Maximum possible time[us] in highest priority interrupt
#ifndef NRF_MAXIMUM_LATENCY_US
#define NRF_MAXIMUM_LATENCY_US 2000
#endif

// </e>


// <e> SPI_ENABLED - nrf_drv_spi - SPI/SPIM peripheral driver
//==========================================================
#ifndef SPI_ENABLED
#define SPI_ENABLED 1
#endif
// <o> SPI_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef SPI_DEFAULT_CONFIG_IRQ_PRIORITY
#define SPI_DEFAULT_CONFIG_IRQ_PRIORITY 7
#endif

// <o> NRF_SPI_DRV_MISO_PULLUP_CFG  - MISO PIN pull-up configuration.

// <0=> NRF_GPIO_PIN_NOPULL
// <1=> NRF_GPIO_PIN_PULLDOWN
// <3=> NRF_GPIO_PIN_PULLUP

#ifndef NRF_SPI_DRV_MISO_PULLUP_CFG
#define NRF_SPI_DRV_MISO_PULLUP_CFG 0
#endif

// <e> SPI0_ENABLED - Enable SPI0 instance
//==========================================================
#ifndef SPI0_ENABLED
#define SPI0_ENABLED 1
#endif

// <o> SPI0_DEFAULT_FREQUENCY  - SPI frequency

// <33554432=> 125 kHz
// <67108864=> 250 kHz
// <134217728=> 500 kHz
// <268435456=> 1 MHz
// <536870912=> 2 MHz
// <1073741824=> 4 MHz
// <2147483648=> 8 MHz

#ifndef SPI0_DEFAULT_FREQUENCY
#define SPI0_DEFAULT_FREQUENCY 1073741824
#endif

// </e>

// <e> SPI1_ENABLED - Enable SPI1 instance
//==========================================================
#ifndef SPI1_ENABLED
#define SPI1_ENABLED 0
#endif

// <o> SPI1_DEFAULT_FREQUENCY  - SPI frequency

// <33554432=> 125 kHz
// <67108864=> 250 kHz
// <134217728=> 500 kHz
// <268435456=> 1 MHz
// <536870912=> 2 MHz
// <1073741824=> 4 MHz
// <2147483648=> 8 MHz

#ifndef SPI1_DEFAULT_FREQUENCY
#define SPI1_DEFAULT_FREQUENCY 1073741824
#endif

// </e>

// <e> NRF_QUEUE_ENABLED - nrf_queue - Queue module
//==========================================================
#ifndef NRF_QUEUE_ENABLED
#define NRF_QUEUE_ENABLED 1
#endif

// <q> NRF_SPI_MNGR_ENABLED  - nrf_spi_mngr - SPI transaction manager


#ifndef NRF_SPI_MNGR_ENABLED
#define NRF_SPI_MNGR_ENABLED 1
#endif


// <e> TIMER_ENABLED - nrf_drv_timer - TIMER periperal driver
//==========================================================
#ifndef TIMER_ENABLED
#define TIMER_ENABLED 1
#endif
// <o> TIMER_DEFAULT_CONFIG_FREQUENCY  - Timer frequency if in Timer mode

// <0=> 16 MHz
// <1=> 8 MHz
// <2=> 4 MHz
// <3=> 2 MHz
// <4=> 1 MHz
// <5=> 500 kHz
// <6=> 250 kHz
// <7=> 125 kHz
// <8=> 62.5 kHz
// <9=> 31.25 kHz

#ifndef TIMER_DEFAULT_CONFIG_FREQUENCY
#define TIMER_DEFAULT_CONFIG_FREQUENCY 7
#endif

// <o> TIMER_DEFAULT_CONFIG_MODE  - Timer mode or operation

// <0=> Timer
// <1=> Counter

#ifndef TIMER_DEFAULT_CONFIG_MODE
#define TIMER_DEFAULT_CONFIG_MODE 0
#endif

// <o> TIMER_DEFAULT_CONFIG_BIT_WIDTH  - Timer counter bit width

// <0=> 16 bit
// <1=> 8 bit
// <2=> 24 bit
// <3=> 32 bit

#ifndef TIMER_DEFAULT_CONFIG_BIT_WIDTH
#define TIMER_DEFAULT_CONFIG_BIT_WIDTH 0
#endif

// <o> TIMER_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef TIMER_DEFAULT_CONFIG_IRQ_PRIORITY
#define TIMER_DEFAULT_CONFIG_IRQ_PRIORITY 6
#endif

// <q> TIMER0_ENABLED  - Enable TIMER0 instance


#ifndef TIMER0_ENABLED
#define TIMER0_ENABLED 1
#endif

// <q> TIMER1_ENABLED  - Enable TIMER1 instance


#ifndef TIMER1_ENABLED
#define TIMER1_ENABLED 0
#endif

// <q> TIMER2_ENABLED  - Enable TIMER2 instance


#ifndef TIMER2_ENABLED
#define TIMER2_ENABLED 0
#endif

// </e>

// <e> UART_ENABLED - nrf_drv_uart - UART/UARTE peripheral driver
//==========================================================
#ifndef UART_ENABLED
#define UART_ENABLED 1
#endif
// <o> UART_DEFAULT_CONFIG_HWFC  - Hardware Flow Control

// <0=> Disabled
// <1=> Enabled

#ifndef UART_DEFAULT_CONFIG_HWFC
#define UART_DEFAULT_CONFIG_HWFC 0
#endif

// <o> UART_DEFAULT_CONFIG_PARITY  - Parity

// <0=> Excluded
// <14=> Included

#ifndef UART_DEFAULT_CONFIG_PARITY
#define UART_DEFAULT_CONFIG_PARITY 0
#endif

// <o> UART_DEFAULT_CONFIG_BAUDRATE  - Default Baudrate

// <323584=> 1200 baud
// <643072=> 2400 baud
// <1290240=> 4800 baud
// <2576384=> 9600 baud
// <3862528=> 14400 baud
// <5152768=> 19200 baud
// <7716864=> 28800 baud
// <10289152=> 38400 baud
// <15400960=> 57600 baud
// <20615168=> 76800 baud
// <30801920=> 115200 baud
// <61865984=> 230400 baud
// <67108864=> 250000 baud
// <121634816=> 460800 baud
// <251658240=> 921600 baud
// <268435456=> 1000000 baud

#ifndef UART_DEFAULT_CONFIG_BAUDRATE
#define UART_DEFAULT_CONFIG_BAUDRATE 30801920
#endif

// <o> UART_DEFAULT_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef UART_DEFAULT_CONFIG_IRQ_PRIORITY
#define UART_DEFAULT_CONFIG_IRQ_PRIORITY 7
#endif

// <q> UART_LEGACY_SUPPORT  - Driver supporting Legacy mode


#ifndef UART_LEGACY_SUPPORT
#define UART_LEGACY_SUPPORT 1
#endif

// <e> UART0_ENABLED - Enable UART0 instance
//==========================================================
#ifndef UART0_ENABLED
#define UART0_ENABLED 1
#endif

// </e>


// <e> WDT_ENABLED - nrf_drv_wdt - WDT peripheral driver
//==========================================================
#ifndef WDT_ENABLED
#define WDT_ENABLED 0
#endif
// <o> WDT_CONFIG_BEHAVIOUR  - WDT behavior in CPU SLEEP or HALT mode

// <1=> Run in SLEEP, Pause in HALT
// <8=> Pause in SLEEP, Run in HALT
// <9=> Run in SLEEP and HALT
// <0=> Pause in SLEEP and HALT

#ifndef WDT_CONFIG_BEHAVIOUR
#define WDT_CONFIG_BEHAVIOUR 1
#endif

// <o> WDT_CONFIG_RELOAD_VALUE - Reload value  <15-4294967295>


#ifndef WDT_CONFIG_RELOAD_VALUE
#define WDT_CONFIG_RELOAD_VALUE 2000
#endif

// <o> WDT_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef WDT_CONFIG_IRQ_PRIORITY
#define WDT_CONFIG_IRQ_PRIORITY 7
#endif

// </e>

// </h>
//==========================================================

// <h> nRF_Libraries

//==========================================================
// <q> APP_GPIOTE_ENABLED  - app_gpiote - GPIOTE events dispatcher


#ifndef APP_GPIOTE_ENABLED
#define APP_GPIOTE_ENABLED 0
#endif

// <e> APP_SCHEDULER_ENABLED - app_scheduler - Events scheduler
//==========================================================
#ifndef APP_SCHEDULER_ENABLED
#define APP_SCHEDULER_ENABLED 0
#endif
// <q> APP_SCHEDULER_WITH_PAUSE  - Enabling pause feature


#ifndef APP_SCHEDULER_WITH_PAUSE
#define APP_SCHEDULER_WITH_PAUSE 0
#endif

// <q> APP_SCHEDULER_WITH_PROFILER  - Enabling scheduler profiling


#ifndef APP_SCHEDULER_WITH_PROFILER
#define APP_SCHEDULER_WITH_PROFILER 0
#endif

// </e>

// <e> APP_TIMER_ENABLED - app_timer - Application timer functionality
//==========================================================
#ifndef APP_TIMER_ENABLED
#define APP_TIMER_ENABLED 0
#endif
// <o> APP_TIMER_CONFIG_RTC_FREQUENCY  - Configure RTC prescaler.

// <0=> 32768 Hz
// <1=> 16384 Hz
// <3=> 8192 Hz
// <7=> 4096 Hz
// <15=> 2048 Hz
// <31=> 1024 Hz

#ifndef APP_TIMER_CONFIG_RTC_FREQUENCY
#define APP_TIMER_CONFIG_RTC_FREQUENCY 0
#endif

// <o> APP_TIMER_CONFIG_IRQ_PRIORITY  - Interrupt priority


// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest)
// <1=> 1
// <2=> 2
// <3=> 3
// <4=> 4
// <5=> 5
// <6=> 6
// <7=> 7

#ifndef APP_TIMER_CONFIG_IRQ_PRIORITY
#define APP_TIMER_CONFIG_IRQ_PRIORITY 7
#endif

// <o> APP_TIMER_CONFIG_OP_QUEUE_SIZE - Capacity of timer requests queue.
// <i> Size of the queue depends on how many timers are used
// <i> in the system, how often timers are started and overall
// <i> system latency. If queue size is too small app_timer calls
// <i> will fail.

#ifndef APP_TIMER_CONFIG_OP_QUEUE_SIZE
#define APP_TIMER_CONFIG_OP_QUEUE_SIZE 10
#endif

// <q> APP_TIMER_CONFIG_USE_SCHEDULER  - Enable scheduling app_timer events to app_scheduler


#ifndef APP_TIMER_CONFIG_USE_SCHEDULER
#define APP_TIMER_CONFIG_USE_SCHEDULER 0
#endif

// <q> APP_TIMER_WITH_PROFILER  - Enable app_timer profiling


#ifndef APP_TIMER_WITH_PROFILER
#define APP_TIMER_WITH_PROFILER 0
#endif

// <q> APP_TIMER_KEEPS_RTC_ACTIVE  - Enable RTC always on


// <i> If option is enabled RTC is kept running even if there is no active timers.
// <i> This option can be used when app_timer is used for timestamping.

#ifndef APP_TIMER_KEEPS_RTC_ACTIVE
#define APP_TIMER_KEEPS_RTC_ACTIVE 0
#endif

// <o> APP_TIMER_CONFIG_SWI_NUMBER  - Configure SWI instance used.

// <0=> 0
// <1=> 1

#ifndef APP_TIMER_CONFIG_SWI_NUMBER
#define APP_TIMER_CONFIG_SWI_NUMBER 0
#endif

// </e>

// <e> APP_UART_ENABLED - app_uart - UART driver
//==========================================================
#ifndef APP_UART_ENABLED
#define APP_UART_ENABLED 0
#endif
// <o> APP_UART_DRIVER_INSTANCE  - UART instance used

// <0=> 0

#ifndef APP_UART_DRIVER_INSTANCE
#define APP_UART_DRIVER_INSTANCE 0
#endif

// </e>


// <q> BUTTON_ENABLED  - app_button - buttons handling module


#ifndef BUTTON_ENABLED
#define BUTTON_ENABLED 0
#endif

// <q> CRC16_ENABLED  - crc16 - CRC16 calculation routines


#ifndef CRC16_ENABLED
#define CRC16_ENABLED 0
#endif

// <q> CRC32_ENABLED  - crc32 - CRC32 calculation routines


#ifndef CRC32_ENABLED
#define CRC32_ENABLED 0
#endif

// <q> ECC_ENABLED  - ecc - Elliptic Curve Cryptography Library


#ifndef ECC_ENABLED
#define ECC_ENABLED 0
#endif

// <e> FDS_ENABLED - fds - Flash data storage module
//==========================================================
#ifndef FDS_ENABLED
#define FDS_ENABLED 0
#endif
// <h> Pages - Virtual page settings

// <i> Configure the number of virtual pages to use and their size.
//==========================================================
// <o> FDS_VIRTUAL_PAGES - Number of virtual flash pages to use.
// <i> One of the virtual pages is reserved by the system for garbage collection.
// <i> Therefore, the minimum is two virtual pages: one page to store data and one page to be used by the system for garbage collection.
// <i> The total amount of flash memory that is used by FDS amounts to @ref FDS_VIRTUAL_PAGES * @ref FDS_VIRTUAL_PAGE_SIZE * 4 bytes.

#ifndef FDS_VIRTUAL_PAGES
#define FDS_VIRTUAL_PAGES 3
#endif

// <o> FDS_VIRTUAL_PAGE_SIZE  - The size of a virtual flash page.


// <i> Expressed in number of 4-byte words.
// <i> By default, a virtual page is the same size as a physical page.
// <i> The size of a virtual page must be a multiple of the size of a physical page.
// <1024=> 1024
// <2048=> 2048

#ifndef FDS_VIRTUAL_PAGE_SIZE
#define FDS_VIRTUAL_PAGE_SIZE 1024
#endif

// </h>
//==========================================================

// <h> Backend - Backend configuration

// <i> Configure which nrf_fstorage backend is used by FDS to write to flash.
//==========================================================
// <o> FDS_BACKEND  - FDS flash backend.


// <i> NRF_FSTORAGE_SD uses the nrf_fstorage_sd backend implementation using the SoftDevice API. Use this if you have a SoftDevice present.
// <i> NRF_FSTORAGE_NVMC uses the nrf_fstorage_nvmc implementation. Use this setting if you don't use the SoftDevice.
// <1=> NRF_FSTORAGE_NVMC
// <2=> NRF_FSTORAGE_SD

#ifndef FDS_BACKEND
#define FDS_BACKEND 2
#endif

// </h>
//==========================================================

// <h> Queue - Queue settings

//==========================================================
// <o> FDS_OP_QUEUE_SIZE - Size of the internal queue.
// <i> Increase this value if you frequently get synchronous FDS_ERR_NO_SPACE_IN_QUEUES errors.

#ifndef FDS_OP_QUEUE_SIZE
#define FDS_OP_QUEUE_SIZE 4
#endif

// </h>
//==========================================================

// <h> CRC - CRC functionality

//==========================================================
// <e> FDS_CRC_CHECK_ON_READ - Enable CRC checks.

// <i> Save a record's CRC when it is written to flash and check it when the record is opened.
// <i> Records with an incorrect CRC can still be 'seen' by the user using FDS functions, but they cannot be opened.
// <i> Additionally, they will not be garbage collected until they are deleted.
//==========================================================
#ifndef FDS_CRC_CHECK_ON_READ
#define FDS_CRC_CHECK_ON_READ 0
#endif
// <o> FDS_CRC_CHECK_ON_WRITE  - Perform a CRC check on newly written records.


// <i> Perform a CRC check on newly written records.
// <i> This setting can be used to make sure that the record data was not altered while being written to flash.
// <1=> Enabled
// <0=> Disabled

#ifndef FDS_CRC_CHECK_ON_WRITE
#define FDS_CRC_CHECK_ON_WRITE 0
#endif

// </e>

// </h>
//==========================================================

// <h> Users - Number of users

//==========================================================
// <o> FDS_MAX_USERS - Maximum number of callbacks that can be registered.
#ifndef FDS_MAX_USERS
#define FDS_MAX_USERS 4
#endif

// </h>
//==========================================================

// </e>

// <e> HARDFAULT_HANDLER_ENABLED - hardfault_default - HardFault default handler for debugging and release
//==========================================================
#ifndef HARDFAULT_HANDLER_ENABLED
#define HARDFAULT_HANDLER_ENABLED 0
#endif
// <q> HARDFAULT_HANDLER_GDB_PSP_BACKTRACE  - Bypass the GDB problem with multiple stack pointers backtrace


// <i> There is a known bug in GDB which causes it to incorrectly backtrace the code
// <i> when multiple stack pointers are used (main and process stack pointers).
// <i> This option enables the fix for that problem and allows to see the proper backtrace info.
// <i> It makes it possible to trace the code to the exact point where a HardFault appeared.
// <i> This option requires additional commands and may temporarily switch MSP stack to store data on PSP space.
// <i> This is an optional parameter - enable it while debugging.
// <i> Before a HardFault handler exits, the stack will be reverted to its previous value.

#ifndef HARDFAULT_HANDLER_GDB_PSP_BACKTRACE
#define HARDFAULT_HANDLER_GDB_PSP_BACKTRACE 1
#endif

// </e>

// <e> NRF_FSTORAGE_ENABLED - nrf_fstorage - Flash abstraction library
//==========================================================
#ifndef NRF_FSTORAGE_ENABLED
#define NRF_FSTORAGE_ENABLED 0
#endif
// <h> nrf_fstorage_sd - Implementation using the SoftDevice.

// <i> Configuration options for the fstorage implementation using the SoftDevice.
//==========================================================
// <o> NRF_FSTORAGE_SD_QUEUE_SIZE - Size of the internal queue of operations.
// <i> Increase this value if API calls frequently return the error @ref NRF_ERROR_NO_MEM.

#ifndef NRF_FSTORAGE_SD_QUEUE_SIZE
#define NRF_FSTORAGE_SD_QUEUE_SIZE 4
#endif

// <o> NRF_FSTORAGE_SD_MAX_RETRIES - Maximum number of attempts at executing an operation when the SoftDevice is busy.
// <i> Increase this value if events frequently return the @ref NRF_ERROR_TIMEOUT error.
// <i> The SoftDevice might fail to schedule flash access due to high BLE activity.

#ifndef NRF_FSTORAGE_SD_MAX_RETRIES
#define NRF_FSTORAGE_SD_MAX_RETRIES 8
#endif

// <o> NRF_FSTORAGE_SD_MAX_WRITE_SIZE - Maximum number of bytes to be written to flash in a single operation.
// <i> This value must be a multiple of four.
// <i> Lowering this value can increase the chances of the SoftDevice being able to execute flash operations in between radio activity.
// <i> This value is bound by the maximum number of bytes that can be written to flash in a single call to @ref sd_flash_write.
// <i> That is 1024 bytes for nRF51 ICs and 4096 bytes for nRF52 ICs.

#ifndef NRF_FSTORAGE_SD_MAX_WRITE_SIZE
#define NRF_FSTORAGE_SD_MAX_WRITE_SIZE 4096
#endif

// <e> NRF_PWR_MGMT_ENABLED - nrf_pwr_mgmt - Power management module
//==========================================================
#ifndef NRF_PWR_MGMT_ENABLED
#define NRF_PWR_MGMT_ENABLED 0
#endif

// <e> NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED - Enables pin debug in the module.

// <i> Selected pin will be set when CPU is in sleep mode.
//==========================================================
#ifndef NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED
#define NRF_PWR_MGMT_CONFIG_DEBUG_PIN_ENABLED 0
#endif
// <o> NRF_PWR_MGMT_SLEEP_DEBUG_PIN  - Pin number

// <0=> 0 (P0.0)
// <1=> 1 (P0.1)
// <2=> 2 (P0.2)
// <3=> 3 (P0.3)
// <4=> 4 (P0.4)
// <5=> 5 (P0.5)
// <6=> 6 (P0.6)
// <7=> 7 (P0.7)
// <8=> 8 (P0.8)
// <9=> 9 (P0.9)
// <10=> 10 (P0.10)
// <11=> 11 (P0.11)
// <12=> 12 (P0.12)
// <13=> 13 (P0.13)
// <14=> 14 (P0.14)
// <15=> 15 (P0.15)
// <16=> 16 (P0.16)
// <17=> 17 (P0.17)
// <18=> 18 (P0.18)
// <19=> 19 (P0.19)
// <20=> 20 (P0.20)
// <21=> 21 (P0.21)
// <22=> 22 (P0.22)
// <23=> 23 (P0.23)
// <24=> 24 (P0.24)
// <25=> 25 (P0.25)
// <26=> 26 (P0.26)
// <27=> 27 (P0.27)
// <28=> 28 (P0.28)
// <29=> 29 (P0.29)
// <30=> 30 (P0.30)
// <31=> 31 (P0.31)
// <32=> 32 (P1.0)
// <33=> 33 (P1.1)
// <34=> 34 (P1.2)
// <35=> 35 (P1.3)
// <36=> 36 (P1.4)
// <37=> 37 (P1.5)
// <38=> 38 (P1.6)
// <39=> 39 (P1.7)
// <40=> 40 (P1.8)
// <41=> 41 (P1.9)
// <42=> 42 (P1.10)
// <43=> 43 (P1.11)
// <44=> 44 (P1.12)
// <45=> 45 (P1.13)
// <46=> 46 (P1.14)
// <47=> 47 (P1.15)
// <4294967295=> Not connected

#ifndef NRF_PWR_MGMT_SLEEP_DEBUG_PIN
#define NRF_PWR_MGMT_SLEEP_DEBUG_PIN 31
#endif

// </e>

// <q> NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED  - Enables CPU usage monitor.


// <i> Module will trace percentage of CPU usage in one second intervals.

#ifndef NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED
#define NRF_PWR_MGMT_CONFIG_CPU_USAGE_MONITOR_ENABLED 0
#endif

// <e> NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED - Enable standby timeout.
//==========================================================
#ifndef NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED
#define NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_ENABLED 0
#endif
// <o> NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S - Standby timeout (in seconds).
// <i> Shutdown procedure will begin no earlier than after this number of seconds.

#ifndef NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S
#define NRF_PWR_MGMT_CONFIG_STANDBY_TIMEOUT_S 3
#endif

// </e>

// <q> NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED  - Enables FPU event cleaning.


#ifndef NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED
#define NRF_PWR_MGMT_CONFIG_FPU_SUPPORT_ENABLED 1
#endif

// <q> NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY  - Blocked shutdown procedure will be retried every second.


#ifndef NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY
#define NRF_PWR_MGMT_CONFIG_AUTO_SHUTDOWN_RETRY 0
#endif

// <q> NRF_PWR_MGMT_CONFIG_USE_SCHEDULER  - Module will use @ref app_scheduler.


#ifndef NRF_PWR_MGMT_CONFIG_USE_SCHEDULER
#define NRF_PWR_MGMT_CONFIG_USE_SCHEDULER 0
#endif

// <o> NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT - The number of priorities for module handlers.
// <i> The number of stages of the shutdown process.

#ifndef NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT
#define NRF_PWR_MGMT_CONFIG_HANDLER_PRIORITY_COUNT 3
#endif

// </e>

// <q> NRF_QUEUE_ENABLED  - nrf_queue - Queue module


#ifndef NRF_QUEUE_ENABLED
#define NRF_QUEUE_ENABLED 0
#endif

// <q> NRF_SECTION_ITER_ENABLED  - nrf_section_iter - Section iterator


#ifndef NRF_SECTION_ITER_ENABLED
#define NRF_SECTION_ITER_ENABLED 0
#endif

// <q> NRF_STRERROR_ENABLED  - nrf_strerror - Library for converting error code to string.


#ifndef NRF_STRERROR_ENABLED
#define NRF_STRERROR_ENABLED 0
#endif

// <q> SLIP_ENABLED  - slip - SLIP encoding and decoding


#ifndef SLIP_ENABLED
#define SLIP_ENABLED 0
#endif

// </e>

// </h>
//==========================================================

// </h>
//==========================================================

// <h> nRF_Log


// </e>

// <h> nrf_log - Logging

//==========================================================
// <e> NRF_LOG_ENABLED - Logging module for nRF5 SDK
//==========================================================
#ifndef NRF_LOG_ENABLED
#define NRF_LOG_ENABLED 0
#endif
// <e> NRF_LOG_USES_COLORS - If enabled then ANSI escape code for colors is prefixed to every string
//==========================================================
#ifndef NRF_LOG_USES_COLORS
#define NRF_LOG_USES_COLORS 0
#endif
// <o> NRF_LOG_COLOR_DEFAULT  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_LOG_COLOR_DEFAULT
#define NRF_LOG_COLOR_DEFAULT 0
#endif

// <o> NRF_LOG_ERROR_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_LOG_ERROR_COLOR
#define NRF_LOG_ERROR_COLOR 0
#endif

// <o> NRF_LOG_WARNING_COLOR  - ANSI escape code prefix.

// <0=> Default
// <1=> Black
// <2=> Red
// <3=> Green
// <4=> Yellow
// <5=> Blue
// <6=> Magenta
// <7=> Cyan
// <8=> White

#ifndef NRF_LOG_WARNING_COLOR
#define NRF_LOG_WARNING_COLOR 0
#endif

// </e>

// <o> NRF_LOG_DEFAULT_LEVEL  - Default Severity level

// <0=> Off
// <1=> Error
// <2=> Warning
// <3=> Info
// <4=> Debug

#ifndef NRF_LOG_DEFAULT_LEVEL
#define NRF_LOG_DEFAULT_LEVEL 3
#endif

// <q> NRF_LOG_DEFERRED  - Enable deffered logger.


// <i> Log data is buffered and can be processed in idle.

#ifndef NRF_LOG_DEFERRED
#define NRF_LOG_DEFERRED 1
#endif

// <o> NRF_LOG_BUFSIZE  - Size of the buffer for storing logs (in bytes).


// <i> Must be power of 2 and multiple of 4.
// <i> If NRF_LOG_DEFERRED = 0 then buffer size can be reduced to minimum.
// <128=> 128
// <256=> 256
// <512=> 512
// <1024=> 1024
// <2048=> 2048
// <4096=> 4096
// <8192=> 8192
// <16384=> 16384

#ifndef NRF_LOG_BUFSIZE
#define NRF_LOG_BUFSIZE 1024
#endif

// <q> NRF_LOG_ALLOW_OVERFLOW  - Configures behavior when circular buffer is full.


// <i> If set then oldest logs are overwritten. Otherwise a
// <i> marker is injected informing about overflow.

#ifndef NRF_LOG_ALLOW_OVERFLOW
#define NRF_LOG_ALLOW_OVERFLOW 1
#endif

// <q> NRF_LOG_USES_TIMESTAMP  - Enable timestamping


// <i> Function for getting the timestamp is provided by the user

#ifndef NRF_LOG_USES_TIMESTAMP
#define NRF_LOG_USES_TIMESTAMP 0
#endif

// <q> NRF_LOG_FILTERS_ENABLED  - Enable dynamic filtering of logs.


#ifndef NRF_LOG_FILTERS_ENABLED
#define NRF_LOG_FILTERS_ENABLED 1
#endif

// <q> NRF_LOG_CLI_CMDS  - Enable CLI commands for the module.


#ifndef NRF_LOG_CLI_CMDS
#define NRF_LOG_CLI_CMDS 1
#endif

// <h> Log message pool - Configuration of log message pool

//==========================================================
// <o> NRF_LOG_MSGPOOL_ELEMENT_SIZE - Size of a single element in the pool of memory objects.
// <i> If a small value is set, then performance of logs processing
// <i> is degraded because data is fragmented. Bigger value impacts
// <i> RAM memory utilization. The size is set to fit a message with
// <i> a timestamp and up to 2 arguments in a single memory object.

#ifndef NRF_LOG_MSGPOOL_ELEMENT_SIZE
#define NRF_LOG_MSGPOOL_ELEMENT_SIZE 20
#endif

// <o> NRF_LOG_MSGPOOL_ELEMENT_COUNT - Number of elements in the pool of memory objects
// <i> If a small value is set, then it may lead to a deadlock
// <i> in certain cases if backend has high latency and holds
// <i> multiple messages for long time. Bigger value impacts
// <i> RAM memory usage.

#ifndef NRF_LOG_MSGPOOL_ELEMENT_COUNT
#define NRF_LOG_MSGPOOL_ELEMENT_COUNT 8
#endif


// </h>
//==========================================================

// </h>
//==========================================================

// <h> nRF_SoftDevice

//==========================================================
// <e> NRF_SDH_ANT_ENABLED - nrf_sdh_ant - SoftDevice ANT event handler
//==========================================================
#ifndef NRF_SDH_ANT_ENABLED
#define NRF_SDH_ANT_ENABLED 0
#endif
// <h> ANT Channels

//==========================================================
// <o> NRF_SDH_ANT_TOTAL_CHANNELS_ALLOCATED - Allocated ANT channels.
#ifndef NRF_SDH_ANT_TOTAL_CHANNELS_ALLOCATED
#define NRF_SDH_ANT_TOTAL_CHANNELS_ALLOCATED 0
#endif

// <o> NRF_SDH_ANT_ENCRYPTED_CHANNELS - Encrypted ANT channels.
#ifndef NRF_SDH_ANT_ENCRYPTED_CHANNELS
#define NRF_SDH_ANT_ENCRYPTED_CHANNELS 0
#endif

// </h>
//==========================================================

// <h> ANT Queues

//==========================================================
// <o> NRF_SDH_ANT_EVENT_QUEUE_SIZE - Event queue size.
#ifndef NRF_SDH_ANT_EVENT_QUEUE_SIZE
#define NRF_SDH_ANT_EVENT_QUEUE_SIZE 32
#endif

// <o> NRF_SDH_ANT_BURST_QUEUE_SIZE - ANT burst queue size.
#ifndef NRF_SDH_ANT_BURST_QUEUE_SIZE
#define NRF_SDH_ANT_BURST_QUEUE_SIZE 128
#endif

// </h>
//==========================================================

// <h> ANT Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_ANT_OBSERVER_PRIO_LEVELS - Total number of priority levels for ANT observers.
// <i> This setting configures the number of priority levels available for the ANT event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_ANT_OBSERVER_PRIO_LEVELS
#define NRF_SDH_ANT_OBSERVER_PRIO_LEVELS 2
#endif

// <h> ANT Observers priorities - Invididual priorities

//==========================================================
// <o> ANT_BPWR_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Bicycle Power Profile.

#ifndef ANT_BPWR_ANT_OBSERVER_PRIO
#define ANT_BPWR_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_BSC_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Bicycle Speed and Cadence Profile.

#ifndef ANT_BSC_ANT_OBSERVER_PRIO
#define ANT_BSC_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_ENCRYPT_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Cryptographic ANT stack configuration module.

#ifndef ANT_ENCRYPT_ANT_OBSERVER_PRIO
#define ANT_ENCRYPT_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_HRM_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Heart Rate Monitor.

#ifndef ANT_HRM_ANT_OBSERVER_PRIO
#define ANT_HRM_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_SDM_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Stride Based Speed and Distance Monitor Profile.

#ifndef ANT_SDM_ANT_OBSERVER_PRIO
#define ANT_SDM_ANT_OBSERVER_PRIO 1
#endif

// <o> ANT_STATE_INDICATOR_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the ANT state indicator module.

#ifndef ANT_STATE_INDICATOR_ANT_OBSERVER_PRIO
#define ANT_STATE_INDICATOR_ANT_OBSERVER_PRIO 1
#endif

// <o> BSP_BTN_ANT_OBSERVER_PRIO
// <i> Priority with which ANT events are dispatched to the Button Control module.

#ifndef BSP_BTN_ANT_OBSERVER_PRIO
#define BSP_BTN_ANT_OBSERVER_PRIO 1
#endif

// </h>
//==========================================================

// </h>
//==========================================================


// </e>

// <e> NRF_SDH_BLE_ENABLED - nrf_sdh_ble - SoftDevice BLE event handler
//==========================================================
#ifndef NRF_SDH_BLE_ENABLED
#define NRF_SDH_BLE_ENABLED 0
#endif
// <h> BLE Stack configuration - Stack configuration parameters

// <i> These values are not used directly by the SoftDevice handler but the application or other libraries might depend on them.
// <i> Keep them up-to-date with the desired configuration.
//==========================================================
// <o> NRF_SDH_BLE_PERIPHERAL_LINK_COUNT - Maximum number of peripheral links.
#ifndef NRF_SDH_BLE_PERIPHERAL_LINK_COUNT
#define NRF_SDH_BLE_PERIPHERAL_LINK_COUNT 0
#endif

// <o> NRF_SDH_BLE_CENTRAL_LINK_COUNT - Maximum number of central links.
#ifndef NRF_SDH_BLE_CENTRAL_LINK_COUNT
#define NRF_SDH_BLE_CENTRAL_LINK_COUNT 0
#endif

// <o> NRF_SDH_BLE_TOTAL_LINK_COUNT - Maximum number of total concurrent connections using the default configuration.
#ifndef NRF_SDH_BLE_TOTAL_LINK_COUNT
#define NRF_SDH_BLE_TOTAL_LINK_COUNT 1
#endif

// <o> NRF_SDH_BLE_GAP_EVENT_LENGTH - The time set aside for this connection on every connection interval in 1.25 ms units.
#ifndef NRF_SDH_BLE_GAP_EVENT_LENGTH
#define NRF_SDH_BLE_GAP_EVENT_LENGTH 3
#endif

// <o> NRF_SDH_BLE_GATT_MAX_MTU_SIZE - Static maximum MTU size.
#ifndef NRF_SDH_BLE_GATT_MAX_MTU_SIZE
#define NRF_SDH_BLE_GATT_MAX_MTU_SIZE 23
#endif

// <o> NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE - Attribute Table size in bytes. The size must be a multiple of 4.
#ifndef NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE
#define NRF_SDH_BLE_GATTS_ATTR_TAB_SIZE 1408
#endif

// <o> NRF_SDH_BLE_VS_UUID_COUNT - The number of vendor-specific UUIDs.
#ifndef NRF_SDH_BLE_VS_UUID_COUNT
#define NRF_SDH_BLE_VS_UUID_COUNT 0
#endif

// <q> NRF_SDH_BLE_SERVICE_CHANGED  - Include the Service Changed characteristic in the Attribute Table.


#ifndef NRF_SDH_BLE_SERVICE_CHANGED
#define NRF_SDH_BLE_SERVICE_CHANGED 0
#endif

// </h>
//==========================================================

// <h> BLE Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_BLE_OBSERVER_PRIO_LEVELS - Total number of priority levels for BLE observers.
// <i> This setting configures the number of priority levels available for BLE event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_BLE_OBSERVER_PRIO_LEVELS
#define NRF_SDH_BLE_OBSERVER_PRIO_LEVELS 3
#endif

// <h> BLE Observers priorities - Invididual priorities

//==========================================================
// <o> BLE_ADV_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Advertising module.

#ifndef BLE_ADV_BLE_OBSERVER_PRIO
#define BLE_ADV_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_ANCS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Apple Notification Service Client.

#ifndef BLE_ANCS_C_BLE_OBSERVER_PRIO
#define BLE_ANCS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_ANS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Alert Notification Service Client.

#ifndef BLE_ANS_C_BLE_OBSERVER_PRIO
#define BLE_ANS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_BAS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Battery Service.

#ifndef BLE_BAS_BLE_OBSERVER_PRIO
#define BLE_BAS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_BAS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Battery Service Client.

#ifndef BLE_BAS_C_BLE_OBSERVER_PRIO
#define BLE_BAS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_BPS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Blood Pressure Service.

#ifndef BLE_BPS_BLE_OBSERVER_PRIO
#define BLE_BPS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_CONN_PARAMS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Connection parameters module.

#ifndef BLE_CONN_PARAMS_BLE_OBSERVER_PRIO
#define BLE_CONN_PARAMS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_CONN_STATE_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Connection State module.

#ifndef BLE_CONN_STATE_BLE_OBSERVER_PRIO
#define BLE_CONN_STATE_BLE_OBSERVER_PRIO 0
#endif

// <o> BLE_CSCS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Cycling Speed and Cadence Service.

#ifndef BLE_CSCS_BLE_OBSERVER_PRIO
#define BLE_CSCS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_CTS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Current Time Service Client.

#ifndef BLE_CTS_C_BLE_OBSERVER_PRIO
#define BLE_CTS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_DB_DISC_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Database Discovery module.

#ifndef BLE_DB_DISC_BLE_OBSERVER_PRIO
#define BLE_DB_DISC_BLE_OBSERVER_PRIO 1
#endif

// <o> BLE_DFU_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the DFU Service.

#ifndef BLE_DFU_BLE_OBSERVER_PRIO
#define BLE_DFU_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_GLS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Glucose Service.

#ifndef BLE_GLS_BLE_OBSERVER_PRIO
#define BLE_GLS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HIDS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Human Interface Device Service.

#ifndef BLE_HIDS_BLE_OBSERVER_PRIO
#define BLE_HIDS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HRS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Heart Rate Service.

#ifndef BLE_HRS_BLE_OBSERVER_PRIO
#define BLE_HRS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HRS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Heart Rate Service Client.

#ifndef BLE_HRS_C_BLE_OBSERVER_PRIO
#define BLE_HRS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_HTS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Health Thermometer Service.

#ifndef BLE_HTS_BLE_OBSERVER_PRIO
#define BLE_HTS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_IAS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Immediate Alert Service.

#ifndef BLE_IAS_BLE_OBSERVER_PRIO
#define BLE_IAS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_IAS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Immediate Alert Service Client.

#ifndef BLE_IAS_C_BLE_OBSERVER_PRIO
#define BLE_IAS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LBS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the LED Button Service.

#ifndef BLE_LBS_BLE_OBSERVER_PRIO
#define BLE_LBS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LBS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the LED Button Service Client.

#ifndef BLE_LBS_C_BLE_OBSERVER_PRIO
#define BLE_LBS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LLS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Link Loss Service.

#ifndef BLE_LLS_BLE_OBSERVER_PRIO
#define BLE_LLS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_LNS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Location Navigation Service.

#ifndef BLE_LNS_BLE_OBSERVER_PRIO
#define BLE_LNS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_NUS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the UART Service.

#ifndef BLE_NUS_BLE_OBSERVER_PRIO
#define BLE_NUS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_NUS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the UART Central Service.

#ifndef BLE_NUS_C_BLE_OBSERVER_PRIO
#define BLE_NUS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_OTS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Object transfer service.

#ifndef BLE_OTS_BLE_OBSERVER_PRIO
#define BLE_OTS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_OTS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Object transfer service client.

#ifndef BLE_OTS_C_BLE_OBSERVER_PRIO
#define BLE_OTS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_RSCS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Running Speed and Cadence Service.

#ifndef BLE_RSCS_BLE_OBSERVER_PRIO
#define BLE_RSCS_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_RSCS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Running Speed and Cadence Client.

#ifndef BLE_RSCS_C_BLE_OBSERVER_PRIO
#define BLE_RSCS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> BLE_TPS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the TX Power Service.

#ifndef BLE_TPS_BLE_OBSERVER_PRIO
#define BLE_TPS_BLE_OBSERVER_PRIO 2
#endif

// <o> BSP_BTN_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Button Control module.

#ifndef BSP_BTN_BLE_OBSERVER_PRIO
#define BSP_BTN_BLE_OBSERVER_PRIO 1
#endif

// <o> NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the NFC pairing library.

#ifndef NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO
#define NFC_BLE_PAIR_LIB_BLE_OBSERVER_PRIO 1
#endif

// <o> NRF_BLE_BMS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Bond Management Service.

#ifndef NRF_BLE_BMS_BLE_OBSERVER_PRIO
#define NRF_BLE_BMS_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_CGMS_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Contiuon Glucose Monitoring Service.

#ifndef NRF_BLE_CGMS_BLE_OBSERVER_PRIO
#define NRF_BLE_CGMS_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the GATT Service Client.

#ifndef NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO
#define NRF_BLE_GATTS_C_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_GATT_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the GATT module.

#ifndef NRF_BLE_GATT_BLE_OBSERVER_PRIO
#define NRF_BLE_GATT_BLE_OBSERVER_PRIO 2
#endif

// <o> NRF_BLE_QWR_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Queued writes module.

#ifndef NRF_BLE_QWR_BLE_OBSERVER_PRIO
#define NRF_BLE_QWR_BLE_OBSERVER_PRIO 2
#endif

// <o> PM_BLE_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the Peer Manager module.

#ifndef PM_BLE_OBSERVER_PRIO
#define PM_BLE_OBSERVER_PRIO 2
#endif

// </h>
//==========================================================

// </h>
//==========================================================


// </e>

// <e> NRF_SDH_ENABLED - nrf_sdh - SoftDevice handler
//==========================================================
#ifndef NRF_SDH_ENABLED
#define NRF_SDH_ENABLED 0
#endif
// <h> Dispatch model

// <i> This setting configures how Stack events are dispatched to the application.
//==========================================================
// <o> NRF_SDH_DISPATCH_MODEL


// <i> NRF_SDH_DISPATCH_MODEL_INTERRUPT: SoftDevice events are passed to the application from the interrupt context.
// <i> NRF_SDH_DISPATCH_MODEL_APPSH: SoftDevice events are scheduled using @ref app_scheduler.
// <i> NRF_SDH_DISPATCH_MODEL_POLLING: SoftDevice events are to be fetched manually.
// <0=> NRF_SDH_DISPATCH_MODEL_INTERRUPT
// <1=> NRF_SDH_DISPATCH_MODEL_APPSH
// <2=> NRF_SDH_DISPATCH_MODEL_POLLING

#ifndef NRF_SDH_DISPATCH_MODEL
#define NRF_SDH_DISPATCH_MODEL 0
#endif

// </h>
//==========================================================

// <h> Clock - SoftDevice clock configuration

//==========================================================
// <o> NRF_SDH_CLOCK_LF_SRC  - SoftDevice clock source.

// <0=> NRF_CLOCK_LF_SRC_RC
// <1=> NRF_CLOCK_LF_SRC_XTAL
// <2=> NRF_CLOCK_LF_SRC_SYNTH

#ifndef NRF_SDH_CLOCK_LF_SRC
#define NRF_SDH_CLOCK_LF_SRC 1
#endif

// <o> NRF_SDH_CLOCK_LF_RC_CTIV - SoftDevice calibration timer interval.
#ifndef NRF_SDH_CLOCK_LF_RC_CTIV
#define NRF_SDH_CLOCK_LF_RC_CTIV 0
#endif

// <o> NRF_SDH_CLOCK_LF_RC_TEMP_CTIV - SoftDevice calibration timer interval under constant temperature.
// <i> How often (in number of calibration intervals) the RC oscillator shall be calibrated
// <i>  if the temperature has not changed.

#ifndef NRF_SDH_CLOCK_LF_RC_TEMP_CTIV
#define NRF_SDH_CLOCK_LF_RC_TEMP_CTIV 0
#endif

// <o> NRF_SDH_CLOCK_LF_XTAL_ACCURACY  - External crystal clock accuracy used in the LL to compute timing windows.

// <0=> NRF_CLOCK_LF_XTAL_ACCURACY_250_PPM
// <1=> NRF_CLOCK_LF_XTAL_ACCURACY_500_PPM
// <2=> NRF_CLOCK_LF_XTAL_ACCURACY_150_PPM
// <3=> NRF_CLOCK_LF_XTAL_ACCURACY_100_PPM
// <4=> NRF_CLOCK_LF_XTAL_ACCURACY_75_PPM
// <5=> NRF_CLOCK_LF_XTAL_ACCURACY_50_PPM
// <6=> NRF_CLOCK_LF_XTAL_ACCURACY_30_PPM
// <7=> NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM

#ifndef NRF_SDH_CLOCK_LF_XTAL_ACCURACY
#define NRF_SDH_CLOCK_LF_XTAL_ACCURACY 7
#endif

// </h>
//==========================================================

// <h> SDH Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_REQ_OBSERVER_PRIO_LEVELS - Total number of priority levels for request observers.
// <i> This setting configures the number of priority levels available for the SoftDevice request event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_REQ_OBSERVER_PRIO_LEVELS
#define NRF_SDH_REQ_OBSERVER_PRIO_LEVELS 2
#endif

// <o> NRF_SDH_STATE_OBSERVER_PRIO_LEVELS - Total number of priority levels for state observers.
// <i> This setting configures the number of priority levels available for the SoftDevice state event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_STATE_OBSERVER_PRIO_LEVELS
#define NRF_SDH_STATE_OBSERVER_PRIO_LEVELS 2
#endif

// <o> NRF_SDH_STACK_OBSERVER_PRIO_LEVELS - Total number of priority levels for stack event observers.
// <i> This setting configures the number of priority levels available for the SoftDevice stack event handlers (ANT, BLE, SoC).
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_STACK_OBSERVER_PRIO_LEVELS
#define NRF_SDH_STACK_OBSERVER_PRIO_LEVELS 2
#endif


// <h> State Observers priorities - Invididual priorities

//==========================================================
// <o> CLOCK_CONFIG_STATE_OBSERVER_PRIO
// <i> Priority with which state events are dispatched to the Clock driver.

#ifndef CLOCK_CONFIG_STATE_OBSERVER_PRIO
#define CLOCK_CONFIG_STATE_OBSERVER_PRIO 0
#endif

// <o> POWER_CONFIG_STATE_OBSERVER_PRIO
// <i> Priority with which state events are dispatched to the Power driver.

#ifndef POWER_CONFIG_STATE_OBSERVER_PRIO
#define POWER_CONFIG_STATE_OBSERVER_PRIO 0
#endif

// <o> RNG_CONFIG_STATE_OBSERVER_PRIO
// <i> Priority with which state events are dispatched to this module.

#ifndef RNG_CONFIG_STATE_OBSERVER_PRIO
#define RNG_CONFIG_STATE_OBSERVER_PRIO 0
#endif

// </h>
//==========================================================

// <h> Stack Event Observers priorities - Invididual priorities

//==========================================================
// <o> NRF_SDH_ANT_STACK_OBSERVER_PRIO
// <i> This setting configures the priority with which ANT events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have ANT events dispatched before or after other stack events, such as BLE or SoC.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_ANT_STACK_OBSERVER_PRIO
#define NRF_SDH_ANT_STACK_OBSERVER_PRIO 0
#endif

// <o> NRF_SDH_BLE_STACK_OBSERVER_PRIO
// <i> This setting configures the priority with which BLE events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have BLE events dispatched before or after other stack events, such as ANT or SoC.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_BLE_STACK_OBSERVER_PRIO
#define NRF_SDH_BLE_STACK_OBSERVER_PRIO 0
#endif

// <o> NRF_SDH_SOC_STACK_OBSERVER_PRIO
// <i> This setting configures the priority with which SoC events are processed with respect to other events coming from the stack.
// <i> Modify this setting if you need to have SoC events dispatched before or after other stack events, such as ANT or BLE.
// <i> Zero is the highest priority.

#ifndef NRF_SDH_SOC_STACK_OBSERVER_PRIO
#define NRF_SDH_SOC_STACK_OBSERVER_PRIO 0
#endif

// </h>
//==========================================================

// </h>
//==========================================================


// </e>

// <e> NRF_SDH_SOC_ENABLED - nrf_sdh_soc - SoftDevice SoC event handler
//==========================================================
#ifndef NRF_SDH_SOC_ENABLED
#define NRF_SDH_SOC_ENABLED 0
#endif
// <h> SoC Observers - Observers and priority levels

//==========================================================
// <o> NRF_SDH_SOC_OBSERVER_PRIO_LEVELS - Total number of priority levels for SoC observers.
// <i> This setting configures the number of priority levels available for the SoC event handlers.
// <i> The priority level of a handler determines the order in which it receives events, with respect to other handlers.

#ifndef NRF_SDH_SOC_OBSERVER_PRIO_LEVELS
#define NRF_SDH_SOC_OBSERVER_PRIO_LEVELS 2
#endif

// <h> SoC Observers priorities - Invididual priorities

//==========================================================
// <o> BLE_ADV_SOC_OBSERVER_PRIO
// <i> Priority with which SoC events are dispatched to the Advertising module.

#ifndef BLE_ADV_SOC_OBSERVER_PRIO
#define BLE_ADV_SOC_OBSERVER_PRIO 1
#endif

// <o> BLE_DFU_SOC_OBSERVER_PRIO
// <i> Priority with which BLE events are dispatched to the DFU Service.

#ifndef BLE_DFU_SOC_OBSERVER_PRIO
#define BLE_DFU_SOC_OBSERVER_PRIO 1
#endif

// <o> CLOCK_CONFIG_SOC_OBSERVER_PRIO
// <i> Priority with which SoC events are dispatched to the Clock driver.

#ifndef CLOCK_CONFIG_SOC_OBSERVER_PRIO
#define CLOCK_CONFIG_SOC_OBSERVER_PRIO 0
#endif

// <o> POWER_CONFIG_SOC_OBSERVER_PRIO
// <i> Priority with which SoC events are dispatched to the Power driver.

#ifndef POWER_CONFIG_SOC_OBSERVER_PRIO
#define POWER_CONFIG_SOC_OBSERVER_PRIO 0
#endif

// </h>
//==========================================================

// </h>
//==========================================================


// </e>

// </h>
//==========================================================

// <<< end of configuration section >>>
#endif //SDK_CONFIG_H

