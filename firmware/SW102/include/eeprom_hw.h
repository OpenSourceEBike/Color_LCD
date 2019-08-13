
#pragma once
#include "stdint.h"
#include "stdbool.h"

void eeprom_hw_init(void);

bool flash_write_words(const void *value, uint16_t length_words);
bool flash_read_words(void *dest, uint16_t length_words);


