/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */

#include <string.h>
#include "section_vars.h"
#include "eeprom_hw.h"
#include "common.h"
#include "nrf_delay.h"
#include "fstorage.h"


volatile fs_ret_t last_fs_ret;


/* Event handler */

/* Register fs_sys_event_handler with softdevice_sys_evt_handler_set in ble_stack_init or this doesn't fire! */
static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result)
{
  last_fs_ret = result;
}


FS_REGISTER_CFG(fs_config_t fs_config) =
{
    .callback  = fs_evt_handler,  // Function for event callbacks.
    .num_pages = 1,               // Number of physical flash pages required (see also FS_PAGE_SIZE_WORDS for actual sizes).
                                  // NRF51: 256 words (32 bit).
    .priority  = 0xF0             // Priority for flash usage.
};

void flash_read_words(uint8_t offset, void *dest, uint16_t length_words)
{
  memcpy(dest, &fs_config.p_start_addr[offset], length_words / sizeof(uint32_t));
  // return fs_config.p_start_addr[offset];
}

bool flash_write_words(uint8_t offset, const void* value, uint16_t length_words)
{
  fs_ret_t ret;
  uint32_t cnt = 0;

  do
  {
    last_fs_ret = 0xFF;
    ret = fs_store(&fs_config, &fs_config.p_start_addr[offset], value, length_words, NULL);
    /* Wait some time if fstore queue is full and retry */
    if (ret == FS_ERR_QUEUE_FULL)
      nrf_delay_us(100);
  }
  while (ret == FS_ERR_QUEUE_FULL && cnt++ < 10);

  return ret == FS_SUCCESS ? true : false;
}

/**
 * @brief Init eeprom emulation system
 */
void eeprom_hw_init(void)
{
  UNUSED_VARIABLE(fs_config);  // To avoid 'Unused declaration' warning. Compiler doesn't see the section/linker magic in use.

  fs_init();

#if 0
  static uint32_t ee_key = flash_read_word(ADDRESS_KEY);
  /* Init eeprom to default if KEY is not valid */
  if (ee_key != KEY)
  {
    /* Write default config to eeprom */
    // FIXME - init eeprom contents
    // eeprom_write_configuration(&default_configuration_variables);
    uint32_t cnt = 0;
    while (last_fs_ret == 0xFF && cnt++ < 10)
      nrf_delay_us(100);

    /* Write new KEY */
    ee_key = KEY;
    flash_write_words(ADDRESS_KEY, &ee_key, 1);
    cnt = 0;
    while (last_fs_ret == 0xFF && cnt++ < 10)
      nrf_delay_us(100);
  }
#endif
}




