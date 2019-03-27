/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) Casainho, 2019.
 *
 * Released under the GPL License, Version 3
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_gpio.h"

#include "pins.h"

void pins_init(void)
{
  nrf_gpio_cfg_output(SYSTEM_POWER_ON_OFF__PIN);
}

void system_power(uint32_t state)
{
  if(state)
  {
    nrf_gpio_pin_set(SYSTEM_POWER_ON_OFF__PIN);
  }
  else
  {
    nrf_gpio_pin_clear(SYSTEM_POWER_ON_OFF__PIN);
  }
}
