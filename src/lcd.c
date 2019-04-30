/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */

/* SPI timings SH1107 data sheet p. 52 (we are on Vdd 3.3V) */

/* Transferring the frame buffer by none-blocking SPI Transaction Manager showed that the CPU is blocked for the period of transaction
 * by ISR and library management because of very fast IRQ cadence.
 * Therefore we use standard blocking SPI transfer right away and save some complexity and flash space.
 */

#include "lcd.h"
#include "pins.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"


/* Function prototype */
static void set_cmd(void);
static void set_data(void);
static void send_cmd(uint8_t cmd);
static void send_byte(uint8_t byte);
static void spi_init(void);
static void lcd_refresh(void);
static void pset(UG_S16 x, UG_S16 y, UG_COLOR col);


/* Variable definition */

/* µGUI instance from main */
extern UG_GUI gui;

/* Frame buffer in RAM with same structure as LCD memory --> 16 pages a 64 columns (1 kB) */
uint8_t frameBuffer[16][64];

/* Init sequence sampled by casainho from original SW102 display */
const uint8_t init_array[] = { 0xAE, 0xA8, 0x3F, 0xD5, 0x50, 0xC0, 0xD3, 0x60, 0xDC, 0x00, 0x20, 0x81, 0xBF, 0xA0, 0xA4, 0xA6, 0xAD, 0x8A, 0xD9, 0x1F, 0xDB, 0x30, 0xAF };

/* SPI instance */
const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(LCD_SPI_INSTANCE);


/**
 * @brief LCD initialization including hardware layer.
 */
void lcd_init(void)
{
  spi_init();

  // LCD hold in reset since gpio_init
  // SH1107 reset time (p. 55) doubled
  nrf_delay_us(20);
  nrf_gpio_pin_set(LCD_RES__PIN);
  nrf_delay_us(4);

  // Power On Sequence SH1107 data sheet p. 44

  // Set up initialization sequence
  set_cmd();
  nrf_drv_spi_transfer(&spi, init_array, ARRAY_SIZE(init_array), NULL, 0);

  // Clear internal RAM
  lcd_refresh(); // Is already initialized to zero in bss segment.

  // Set display on
  //send_cmd(0xAF); // Works also as part of initialization sequence

  // Wait 100 ms
  nrf_delay_ms(100);  // Doesn't have to be exact this delay.

  // Setup µGUI library
  UG_Init(&gui, pset, 64, 128); // Pixel set function
  UG_SetRefresh(lcd_refresh); // LCD refresh function
}

static void set_cmd(void)
{
  nrf_gpio_pin_clear(LCD_COMMAND_DATA__PIN);
  //nrf_delay_us(1);  // Max. setup time (~150 ns)
}

static void set_data(void)
{
  nrf_gpio_pin_set(LCD_COMMAND_DATA__PIN);
  //nrf_delay_us(1);  // Max. setup time (~150 ns)
}

/**
 * @brief Sends single command byte
 */
static void send_cmd(uint8_t cmd)
{
  set_cmd();
  nrf_drv_spi_transfer(&spi, &cmd, 1, NULL, 0);
}

/**
 * @brief Sends single data byte
 */
static void send_byte(uint8_t byte)
{
  set_data();
  nrf_drv_spi_transfer(&spi, &byte, 1, NULL, 0);
}

/**
 * @brief Start transfer of frameBuffer to LCD
 */
static void lcd_refresh(void)
{
  uint8_t addr = 0xB0;

  for (uint8_t i = 0; i < 16; i++)
  {
    // New page address
    send_cmd(addr++);
    send_cmd(0x00);
    send_cmd(0x10);
    // send page data
    set_data();
    nrf_drv_spi_transfer(&spi, &frameBuffer[i][0], 64, NULL, 0);
  }
}

/**
 * @brief SPI driver initialization.
 */
static void spi_init(void)
{
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin = LCD_CHIP_SELECT__PIN;
  spi_config.mosi_pin = LCD_DATA__PIN;
  spi_config.sck_pin = LCD_CLOCK__PIN;
  /* DEFAULT_CONFIG may change */
  spi_config.frequency = NRF_SPI_FREQ_4M; // SH1107 data sheet p. 52 (Vdd 3.3 V)
  spi_config.mode = NRF_SPI_MODE_0;
  spi_config.bit_order = NRF_SPI_BIT_ORDER_MSB_FIRST;

  nrf_drv_spi_init(&spi, &spi_config, NULL);
}

/**
 * @brief µGUI pset function. This writes to a frameBuffer in SRAM.
 */
static void pset(UG_S16 x, UG_S16 y, UG_COLOR col)
{
  if (x > 63 || x < 0)
    return;

  if (y > 127 || y < 0)
    return;

  uint8_t page = y / 8;
  uint8_t pixel = y % 8;

  if (col > 0)
    SET_BIT(frameBuffer[page][x], pixel);
  else
    CLR_BIT(frameBuffer[page][x], pixel);
}
