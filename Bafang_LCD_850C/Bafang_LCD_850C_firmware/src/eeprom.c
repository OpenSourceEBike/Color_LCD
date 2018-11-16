/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stdio.h"
#include "eeprom.h"
#include "stm32f10x_flash.h"

#define EEPROM_START_ADDRESS  0x0807F800 // last page of 2kbytes of flash memory

uint32_t eeprom_read(uint16_t ui16_address, uint8_t *ui8_p_data)
{
  uint32_t *ui32_p_address = (uint32_t *) ((uint32_t) EEPROM_START_ADDRESS + (uint32_t) ui16_address);

  *ui8_p_data = (uint8_t) (*ui32_p_address);

  return 0;
}

uint32_t eeprom_write(uint16_t ui16_address, uint8_t ui8_p_data)
{
  uint32_t ui32_address;
  uint32_t ui32_data;
  uint32_t ui32_read_data;

  ui32_address = (uint32_t) EEPROM_START_ADDRESS + (uint32_t) ui16_address;
  ui32_data = (uint32_t) ui8_p_data;

  FLASH_Unlock();

  FLASH_ErasePage(ui32_address);

  if(FLASH_ProgramHalfWord(ui32_address, ui32_data) != FLASH_COMPLETE)
  {
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_Lock();
    return 1;
  }
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  FLASH_Lock();

  uint32_t *ui32_p_address = (uint32_t *) ((uint32_t) EEPROM_START_ADDRESS + (uint32_t) ui16_address);

  ui32_read_data = (*ui32_p_address);

  return 0;
}
