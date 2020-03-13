/*
 * Bafang LCD 860C/850C firmware
 *
 * Copyright (C) Casainho, 2018, 2019, 2020
 *
 * Released under the GPL License, Version 3
 */

#include "stdio.h"
#include <string.h>
#include "stm32f10x_flash.h"
#include "eeprom_hw.h"

#define EEPROM_START_ADDRESS            0x0807F000
#define EEPROM_START_ADDRESS_PAGE_0     0x0807F000
#define EEPROM_START_ADDRESS_PAGE_1     0x0807F800 // last page of 2kbytes of flash memory

#define EEPROM_PAGE_KEY_ADDRESS         (1024 - 1)
#define EEPROM_PAGE_WRITE_ID_ADDRESS    (EEPROM_PAGE_KEY_ADDRESS - 1)

#define EEPROM_MAGIC_KEY                ((uint8_t) 0x5a)

static uint32_t ui32_m_eeprom_page = 0;

static void eeprom_erase_page(uint32_t ui32_eeprom_page);
static uint8_t eeprom_read(uint32_t ui32_address);
static uint8_t eeprom_read_from_page(uint32_t ui32_address, uint32_t ui32_eeprom_page);
// static uint32_t eeprom_write_to_page(uint32_t ui32_address, uint8_t ui8_data, uint32_t ui32_eeprom_page);

void eeprom_hw_init() {
	// Nothing needed on 850C
}

// Set eeprom_page based on which page seems like it might be valid
void find_valid_page()
{
  // check if both EEPROM pages have invalid values and if so, write default values on page 0
  if((eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 0) != EEPROM_MAGIC_KEY) &&
	  (eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 1) != EEPROM_MAGIC_KEY))
  {
    // setup page 0
    ui32_m_eeprom_page = 0;
    eeprom_erase_page(ui32_m_eeprom_page);
    // write EEPROM_MAGIC_KEY and EEPROM_PAGE WRITE_ID
    eeprom_write(EEPROM_PAGE_KEY_ADDRESS, EEPROM_MAGIC_KEY);
    eeprom_write(EEPROM_PAGE_WRITE_ID_ADDRESS, 1); // start with value 1
  }
  // check if both EEPROM pages have valid values...
  else if((eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 0) == EEPROM_MAGIC_KEY) &&
	  (eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 1) == EEPROM_MAGIC_KEY))
  {
    // now find the page that has a higher write ID
    if(eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 1) > eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, 0))
    {
      ui32_m_eeprom_page = 1;
    }
    else
    {
      ui32_m_eeprom_page = 0;
    }
  }
  else if(eeprom_read_from_page(EEPROM_PAGE_KEY_ADDRESS, 0) == EEPROM_MAGIC_KEY)
  {
      ui32_m_eeprom_page = 0;
  }
  else
  {
      ui32_m_eeprom_page = 1;
  }
}

// Read raw EEPROM data, return false if it is blank or malformatted
bool flash_read_words(void *dest, uint16_t length_words)
{
  uint8_t ui8_data;

  find_valid_page();

  // start by reading address 0 and see if value is different from our key,
  // if so mean that eeprom memory is clean (or data strucutre is invalid) and we need to populate
  ui8_data = eeprom_read(ADDRESS_KEY);
  if(ui8_data != KEY) // verify if our key exist
	  return false;

  // read the values from EEPROM to array
  for(int i = 0; i < sizeof(uint32_t) * length_words; i++)
  {
     // we start at EEPROM address 1 as 0 is already in use by the KEY
     ((uint8_t *) dest)[i] = eeprom_read(1 + i);
  }

  return true;
}

static void eeprom_write_array(const uint8_t *p_array, uint32_t ui32_len)
{
  // write the full array
  for(uint32_t ui32_i = 0; ui32_i < ui32_len; ui32_i++)
  {
    // start at 1 since 0 address is for KEY
    eeprom_write(1 + ui32_i, *p_array++);
  }
}

bool flash_write_words(const void *value, uint16_t length_words)
{
  // cycle/increment ui32_eeprom_page, to next page
  ui32_m_eeprom_page = (ui32_m_eeprom_page + 1) % 2;

  // erase the new page
  eeprom_erase_page(ui32_m_eeprom_page);
  // write on the new page
  eeprom_write_array(value, sizeof(uint32_t) * length_words);
  // write key
  eeprom_write(ADDRESS_KEY, KEY);
  // write EEPROM_MAGIC_KEY and EEPROM_PAGE WRITE_ID
  eeprom_write(EEPROM_PAGE_KEY_ADDRESS, EEPROM_MAGIC_KEY);
  uint8_t ui32_eeprom_write_id = eeprom_read_from_page(EEPROM_PAGE_WRITE_ID_ADDRESS, (ui32_m_eeprom_page == 0) ? 1 : 0) + 1;
  eeprom_write(EEPROM_PAGE_WRITE_ID_ADDRESS, ui32_eeprom_write_id); // write new ID

  return true;
}

uint8_t eeprom_read(uint32_t ui32_address)
{
  uint16_t *ui16_p_address = (uint16_t *) (((uint32_t) EEPROM_START_ADDRESS) + (ui32_m_eeprom_page * 2048) + (ui32_address * 2));
  return (uint8_t) (*ui16_p_address);
}

uint8_t eeprom_read_from_page(uint32_t ui32_address, uint32_t ui32_eeprom_page)
{
  uint16_t *ui16_p_address = (uint16_t *) (((uint32_t) EEPROM_START_ADDRESS) + (ui32_eeprom_page * 2048) + (ui32_address * 2));
  return (uint8_t) (*ui16_p_address);
}

uint32_t eeprom_write(uint32_t ui32_address, uint8_t ui8_data)
{
  ui32_address = ((uint32_t) EEPROM_START_ADDRESS) + (ui32_m_eeprom_page * 2048) + (ui32_address * 2);

  FLASH_Unlock();
  if(FLASH_ProgramHalfWord(ui32_address, (uint16_t) ui8_data) != FLASH_COMPLETE)
  {
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_Lock();
    return 1;
  }
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  FLASH_Lock();

  return 0;
}

void eeprom_erase_page(uint32_t ui32_eeprom_page)
{
  FLASH_Unlock();
  FLASH_ErasePage(((uint32_t) EEPROM_START_ADDRESS) + (ui32_eeprom_page * 2048));
  FLASH_Lock();
}
