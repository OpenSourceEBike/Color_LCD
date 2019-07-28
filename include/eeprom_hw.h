
#pragma once
#include "stdint.h"
#include "stdbool.h"

void eeprom_hw_init(void);

bool flash_write_words(uint8_t offset, const void *value, uint16_t length_words);
void flash_read_words(uint8_t offset, void *dest, uint16_t length_words);


