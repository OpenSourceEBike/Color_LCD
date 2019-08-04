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
#include "fds.h"
#include "nrf_delay.h"
#include "assert.h"

// volatile fs_ret_t last_fs_ret;

/* Event handler */

volatile static bool gc_done, init_done, write_done;

/* Register fs_sys_event_handler with softdevice_sys_evt_handler_set in ble_stack_init or this doesn't fire! */
static void fds_evt_handler(fds_evt_t const *const evt)
{
  switch (evt->id)
  {
  case FDS_EVT_INIT:
    APP_ERROR_CHECK(evt->result);
    init_done = true;
    break;
  case FDS_EVT_GC:
    gc_done = true;
    break;
  case FDS_EVT_UPDATE:
  case FDS_EVT_WRITE:
    write_done = true;
    break;
  default:
    break;
  }
}

#define FILE_ID     0x1001
#define REC_KEY     0x2002

// returns true if our preferences were found
bool flash_read_words(void *dest, uint16_t length_words)
{
  fds_flash_record_t flash_record;
  fds_record_desc_t record_desc;
  fds_find_token_t ftok;

  bool did_read = false;

  memset(&ftok, 0x00, sizeof(fds_find_token_t));
  // Loop until all records with the given key and file ID have been found.
  while (fds_record_find(FILE_ID, REC_KEY, &record_desc, &ftok) == FDS_SUCCESS)
  {
    APP_ERROR_CHECK(fds_record_open(&record_desc, &flash_record));

    // Access the record through the flash_record structure.
    memcpy(dest, flash_record.p_data, length_words * sizeof(uint32_t));
    did_read = true;

    // Close the record when done.
    APP_ERROR_CHECK(fds_record_close(&record_desc));
  }

  return did_read;
}

bool flash_write_words(const void *value, uint16_t length_words)
{
  fds_record_t record;
  fds_record_desc_t record_desc;
  fds_record_chunk_t record_chunk;
  fds_find_token_t ftok;

  // Do we already have one of these records?
  bool has_old = fds_record_find(FILE_ID, REC_KEY, &record_desc, &ftok)
      == FDS_SUCCESS;

// Set up data.
  record_chunk.p_data = value;
  record_chunk.length_words = length_words;

// Set up record.
  record.file_id = FILE_ID;
  record.key = REC_KEY;
  record.data.p_chunks = &record_chunk;
  record.data.num_chunks = 1;

  write_done = false;

  // either make a new record or update an old one (if we lose power during update the old record is preserved)
  if (has_old)
    APP_ERROR_CHECK(fds_record_update(&record_desc, &record));
  else
    APP_ERROR_CHECK(fds_record_write(&record_desc, &record));

  for (volatile int count = 0; count < 1000 && !write_done; count++)
    nrf_delay_ms(1);

  return true;
}

static void wait_gc()
{
  gc_done = false;
  APP_ERROR_CHECK(fds_gc());
  for (volatile int count = 0; count < 5000 && !gc_done; count++)
    nrf_delay_ms(1);
  // FIXME - why does this fail sometimes
  // assert(gc_done);
}

/**
 * @brief Init eeprom emulation system
 */
void eeprom_hw_init(void)
{
  ret_code_t ret = fds_register(fds_evt_handler);
  APP_ERROR_CHECK(ret);

  APP_ERROR_CHECK(fds_init());
  for (volatile int count = 0; count < 5000 && !init_done; count++)
    nrf_delay_ms(1);
  assert(init_done);

  wait_gc();
}

