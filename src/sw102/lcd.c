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
#include "common.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "ugui.h"


/* Function prototype */
static void set_cmd(void);
static void set_data(void);
// static void send_byte(uint8_t byte);
static void spi_init(void);
static void pset(UG_S16 x, UG_S16 y, UG_COLOR col);


/* Variable definition */

/* �GUI instance from main */
extern UG_GUI gui;

/* Frame buffer in RAM with same structure as LCD memory --> 16 pages a 64 columns (1 kB) */
uint8_t frameBuffer[16][64];

/* Init sequence sampled by casainho from original SW102 display */
static const uint8_t init_array[] = {
    0xAE, // 11. display on
    0xA8, 0x3F, // set multiplex ratio 3f
    0xD5, 0x50, // set display divite/oscillator ratios
    0xC0, // set common scan dir
    0xD3, 0x60, // ???
    0xDC, 0x00,  // set display start line
    0x20, // set memory address mode
    0x81, 0xFF, // Set contrast level (POR value is 0x80, but closed source software uses 0xBF
    0xA0, // set segment remap
    0xA4, // set normal display mode
    0xA6, // not inverted
    0xAD, 0x8A, // set DC-DC converter
    0xD9, 0x1F, // set discharge/precharge period
    0xDB, 0x30, // set common output voltage
    0xAF // turn display on
    };


/* SPI instance */
const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(LCD_SPI_INSTANCE);

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
static void send_cmd(const uint8_t *cmds, size_t numcmds)
{
  set_cmd();
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, cmds, numcmds, NULL, 0));
}


static UG_RESULT accel_fill_frame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c) {
  if(c == C_TRANSPARENT) // This happens a lot when drawing fonts and we don't need to bother drawing the background
    return UG_RESULT_OK;

  return UG_RESULT_FAIL;
}

static UG_RESULT accel_draw_line(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c) {
  if(c == C_TRANSPARENT) // Probably won't happen but a cheap optimization
    return UG_RESULT_OK;

  return UG_RESULT_FAIL;
}

/**
 * @brief �GUI pset function. This writes to a frameBuffer in SRAM.
 */
static void pset(UG_S16 x, UG_S16 y, UG_COLOR col)
{
  if(col == C_TRANSPARENT)
    return;

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
  send_cmd(init_array, sizeof(init_array));

  // Clear internal RAM
  lcd_refresh(); // Is already initialized to zero in bss segment.

  // Wait 100 ms
  nrf_delay_ms(100);  // Doesn't have to be exact this delay.

  // Setup �GUI library
  UG_Init(&gui, pset, 64, 128); // Pixel set function

  UG_DriverRegister(DRIVER_DRAW_LINE, (void *) accel_draw_line);
  UG_DriverRegister(DRIVER_FILL_FRAME, (void *) accel_fill_frame);

  // kevinh - I've moved this to be an explicit call, because calling lcd_refresh on each operation is super expensive
  // UG_SetRefresh(lcd_refresh); // LCD refresh function
}



/**
 * @brief Start transfer of frameBuffer to LCD
 */
void lcd_refresh(void)
{
  uint8_t addr = 0xB0;

  static uint8_t pagecmd[] = { 0, 0x00, 0x10 };

  for (uint8_t i = 0; i < 16; i++)
  {
    // New page address
    pagecmd[0] = addr++;
    send_cmd(pagecmd, sizeof(pagecmd));

    // send page data
    set_data();
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, &frameBuffer[i][0], 64, NULL, 0));
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

  APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL));
}



#if 0
void lcd_set_backlight_intensity(uint8_t ui8_intensity)
{
  // force to be min of 20% and max of 100%
  if(ui8_intensity < 4)
  {
    ui8_intensity = 4;
  }
  else if(ui8_intensity > 20)
  {
    ui8_intensity = 20;
  }

  TIM_SetCompare2(TIM3, ((uint16_t) ui8_intensity) * 2000);
  TIM_CtrlPWMOutputs(TIM3, ENABLE);
}
#endif

//SW102 version, we are an oled so if the user asks for lots of backlight we really want to dim instead
void lcd_set_backlight_intensity(uint8_t pct) {

  uint8_t level = 255 * (100 - pct);
  uint8_t cmd[] = { 0x81, level };

  send_cmd(cmd, sizeof(cmd));
}

