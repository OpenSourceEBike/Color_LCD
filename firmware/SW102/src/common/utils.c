/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */
#include "utils.h"

// from here: https://github.com/FxDev/PetitModbus/blob/master/PetitModbus.c
/*
 * Function Name        : CRC16
 * @param[in]           : ui8_data  - Data to Calculate CRC
 * @param[in/out]       : ui16_crc   - Anlik CRC degeri
 * @How to use          : First initial data has to be 0xFFFF.
 */
void crc16(uint8_t ui8_data, uint16_t* ui16_crc)
{
    unsigned int i;

    *ui16_crc = *ui16_crc ^(uint16_t) ui8_data;
    for (i = 8; i > 0; i--)
    {
        if (*ui16_crc & 0x0001)
            *ui16_crc = (*ui16_crc >> 1) ^ 0xA001;
        else
            *ui16_crc >>= 1;
    }
}
