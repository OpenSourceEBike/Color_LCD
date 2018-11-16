/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "stdio.h"

uint32_t eeprom_read(uint16_t ui16_address, uint8_t *ui8_p_data);
uint32_t eeprom_write(uint16_t ui16_address, uint8_t ui8_p_data);

#endif /* EEPROM_H_ */
