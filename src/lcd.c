/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */

/* SPI timings SH1107 p. 52 (we are on Vdd 3.3V) */

#include "lcd.h"
#include "pins.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_spi.h"
#include "nrf_spi_mngr.h"

/* Function prototype */
static void transaction_send_page_finished(ret_code_t result, void * p_user_data);
static void set_cmd(void * p_user_data);
static void set_data(void * p_user_data);
static void send_cmd(uint8_t cmd);
static void send_byte(uint8_t byte);
static void spi_init(void);
void timer_lcd_refresh_handler(nrf_timer_event_t event_type, void* p_context);



/* Variable definition */

/* Frame buffer in RAM with same structure as LCD memory --> 16 pages a 64 columns (1 kB) */
uint8_t frameBuffer[16][64];
uint8_t spi_byte_tx;

/* Page address command */
uint8_t pageAddrCmd[3] = { 0xB0, 0x00, 0x10 };

/* Init sequence sampled by casainho from original SW102 display */
const uint8_t init_array[] = { 0xAE, 0xA8, 0x3F, 0xD5, 0x50, 0xC0, 0xD3, 0x60, 0xDC, 0x00, 0x20, 0x81, 0xBF, 0xA0, 0xA4, 0xA6, 0xAD, 0x8A, 0xD9, 0x1F, 0xDB, 0x30, 0xAF };

/* SPI instance */
const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(LCD_SPI_INSTANCE);

/* SPI transaction manager */
NRF_SPI_MNGR_DEF(m_nrf_spi_mngr, 4, LCD_SPI_INSTANCE);

const nrf_spi_mngr_transfer_t transfers_byte_tx[] =
{
    NRF_SPI_MNGR_TRANSFER(&spi_byte_tx, 1, NULL, 0)
};

const nrf_spi_mngr_transfer_t transfers_initialization[] =
{
    NRF_SPI_MNGR_TRANSFER(init_array, ARRAY_SIZE(init_array), NULL, 0)
};

const nrf_spi_mngr_transfer_t transfers_page_data[] =
{
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[0][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[1][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[2][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[3][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[4][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[5][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[6][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[7][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[8][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[9][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[10][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[11][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[12][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[13][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[14][0], 64, NULL, 0),
    NRF_SPI_MNGR_TRANSFER(&frameBuffer[15][0], 64, NULL, 0)
};

const nrf_spi_mngr_transfer_t transfers_page_cmd[] =
{
    NRF_SPI_MNGR_TRANSFER(pageAddrCmd, ARRAY_SIZE(pageAddrCmd), NULL, 0)
};

nrf_spi_mngr_transaction_t transaction_page_data =
{
    .begin_callback      = set_data,
    .end_callback        = transaction_send_page_finished,
    .p_user_data         = NULL,
    .p_transfers         = &transfers_page_data[0],
    .number_of_transfers = 1,
    .p_required_spi_cfg  = NULL
};

nrf_spi_mngr_transaction_t transaction_page_addr_cmd =
{
    .begin_callback      = set_cmd,
    .end_callback        = NULL,
    .p_user_data         = NULL,
    .p_transfers         = transfers_page_cmd,
    .number_of_transfers = 1,
    .p_required_spi_cfg  = NULL
};

/* LCD refresh timer */
const nrf_drv_timer_t lcdRefreshTimer = NRF_DRV_TIMER_INSTANCE(LCD_REFRESH_TIMER_INSTANCE);


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

  // Power On Sequence SH1107 p. 44

  // Set up initialization sequence
  set_cmd(NULL);
  nrf_spi_mngr_perform(&m_nrf_spi_mngr, NULL, transfers_initialization, ARRAY_SIZE(transfers_initialization), NULL);

  // Clear internal RAM
  lcd_refresh(); // Is already initialized to zero in bss segment.

  // Set display on
  //send_cmd(0xAF); // Works also as part of initialization sequence

  // Wait 100 ms
  nrf_delay_ms(100);  // Doesn't have to be exact this delay.

  // Configure TIMER0 for lcd refresh
  nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
  nrf_drv_timer_init(&lcdRefreshTimer, &timer_cfg, timer_lcd_refresh_handler);
  uint32_t time_ticks = nrf_drv_timer_ms_to_ticks(&lcdRefreshTimer, 50/*ms*/);
  nrf_drv_timer_extended_compare(&lcdRefreshTimer, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
  nrf_drv_timer_enable(&lcdRefreshTimer);
}

/**
 * @brief SPI manager send frameBuffer page by page. After each page this function is called where the transaction of the next page is set
 */
static void transaction_send_page_finished(ret_code_t result, void * p_user_data)
{
  static uint8_t transactionPageCounter;
  transactionPageCounter++;

  if (result != NRF_SUCCESS)  // Set to defaults on error
    transactionPageCounter = 0xFF;

  if (transactionPageCounter < 16)  // Transaction ongoing
  {
    /* Set new Page & Address */
    transaction_page_data.p_transfers = &transfers_page_data[transactionPageCounter];
    pageAddrCmd[0] = 0xB0 + transactionPageCounter;
    /* Schedule transaction */
    ret_code_t result = nrf_spi_mngr_schedule(&m_nrf_spi_mngr, &transaction_page_addr_cmd);
    if (result == NRF_SUCCESS)
      result = nrf_spi_mngr_schedule(&m_nrf_spi_mngr, &transaction_page_data);

    if (result != NRF_SUCCESS)  // To defaults on error
    {
      /* Defaults */
      transactionPageCounter = 0;
      transaction_page_data.p_transfers = &transfers_page_data[0];
      pageAddrCmd[0] = 0xB0;
    }
  }
  else  // Transaction finished
  {
    /* Defaults */
    transactionPageCounter = 0;
    transaction_page_data.p_transfers = &transfers_page_data[0];
    pageAddrCmd[0] = 0xB0;
  }
}

static void set_cmd(void * p_user_data)
{
  nrf_gpio_pin_clear(LCD_COMMAND_DATA__PIN);
  //nrf_delay_us(1);  // Max. setup time (~150 ns)
}

static void set_data(void * p_user_data)
{
  nrf_gpio_pin_set(LCD_COMMAND_DATA__PIN);
  //nrf_delay_us(1);  // Max. setup time (~150 ns)
}

/**
 * @brief Sends single command byte in blocking mode
 */
static void send_cmd(uint8_t cmd)
{
  set_cmd(NULL);
  spi_byte_tx = cmd;
  nrf_spi_mngr_perform(&m_nrf_spi_mngr, NULL, transfers_byte_tx, 1, NULL);
}

/**
 * @brief Sends single data byte in blocking mode
 */
static void send_byte(uint8_t byte)
{
  set_data(NULL);
  spi_byte_tx = byte;
  nrf_spi_mngr_perform(&m_nrf_spi_mngr, NULL, transfers_byte_tx, 1, NULL);
}

/**
 * @brief Start transfer of frameBuffer to LCD
 */
void lcd_refresh(void)
{
  /* TODO: Deadlock prevention? */
  while (!nrf_spi_mngr_is_idle(&m_nrf_spi_mngr))
    nrf_delay_us(1);

  /* Schedule transaction */
  /* Corresponding values should be already set to default (on startup & transaction_send_page_finished) */
  ret_code_t result = nrf_spi_mngr_schedule(&m_nrf_spi_mngr, &transaction_page_addr_cmd);
  if (result == NRF_SUCCESS)
    result = nrf_spi_mngr_schedule(&m_nrf_spi_mngr, &transaction_page_data);
}

/**
 * @brief SPI driver initialization.
 */
static void spi_init(void)
{
  nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
  spi_config.ss_pin = LCD_CHIP_SELECT__PIN;
  spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED; // No data from LCD
  spi_config.mosi_pin = LCD_DATA__PIN;
  spi_config.sck_pin = LCD_CLOCK__PIN;
  /* DEFAULT_CONFIG may change */
  spi_config.frequency = NRF_DRV_SPI_FREQ_4M; // SH1107 p. 52 (Vdd 3.3 V)
  spi_config.mode = NRF_DRV_SPI_MODE_0;
  spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

  nrf_spi_mngr_init(&m_nrf_spi_mngr, &spi_config);  // transaction manager init
}

/**
 * @brief µGUI pset function. This writes to a frameBuffer in SRAM.
 */
void pset(UG_S16 x, UG_S16 y, UG_COLOR col)
{
  if (x > 63 || x < 0)
    return;

  if (y > 127 || y < 0)
    return;

  uint8_t page = y / 8;
  uint8_t pixel = y % 8;

  if (col > 0)
    frameBuffer[page][x] |= (1U << pixel);  // SET
  else
    frameBuffer[page][x] &= ~(1U << pixel); // CLEAR
}



/* Event Handler */

/**
 * @brief Handler for timer event which trigger lcd_refresh.
 */
void timer_lcd_refresh_handler(nrf_timer_event_t event_type, void* p_context)
{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
          lcd_refresh();
            break;

        default:
            // Do nothing.
            break;
    }
}
