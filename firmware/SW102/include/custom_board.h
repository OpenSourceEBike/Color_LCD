/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
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
#ifndef CUSTOM_H
#define CUSTOM_H

#ifdef __cplusplus
extern "C" {
#endif

#define LEDS_NUMBER     0

#define BUTTONS_NUMBER  0

#define SYSTEM_POWER_HOLD__PIN    9

#define LCD_RES__PIN              5
#define LCD_COMMAND_DATA__PIN     30
#define LCD_CHIP_SELECT__PIN      4
#define LCD_CLOCK__PIN            6
#define LCD_DATA__PIN             7

#define BUTTON_UP__PIN            2
#define BUTTON_PWR__PIN           10
#define BUTTON_DOWN__PIN          19
#define BUTTON_M__PIN             14

#define UART_TX__PIN              12
#define UART_RX__PIN              11

#define TEST__PIN                 28

#define SPIM0_SCK_PIN   LCD_CLOCK__PIN
#define SPIM0_MOSI_PIN  LCD_DATA__PIN
#define SPIM0_MISO_PIN  NRF_DRV_SPI_PIN_NOT_USED
#define SPIM0_SS_PIN    LCD_CHIP_SELECT__PIN

#define RX_PIN_NUMBER   UART_RX__PIN
#define TX_PIN_NUMBER   UART_TX__PIN
#define CTS_PIN_NUMBER  NRF_UART_PSEL_DISCONNECTED
#define RTS_PIN_NUMBER  NRF_UART_PSEL_DISCONNECTED
#define HWFC            false

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_RC,  \
                                 .rc_ctiv       = 16, /* Check temperature every 4 seconds */ \
                                 .rc_temp_ctiv  = 2,  /* Calibrate at least every 8 seconds even if the temperature hasn't changed */ \
                                 .xtal_accuracy = 0   /* For the NRF_CLOCK_LF_SRC_RC clock source this parameter is ignored. */}

#ifdef __cplusplus
}
#endif

#endif
