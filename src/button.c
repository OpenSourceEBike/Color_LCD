/*
 * Bafang LCD SW102 Bluetooth firmware
 *
 * Copyright (C) lowPerformer, 2019.
 *
 * Released under the GPL License, Version 3
 */
#include "button.h"

/**
 * @brief Init button struct. Call once.
 */
void InitButton(Button* button, uint32_t pin_number, nrf_gpio_pin_pull_t pull_config, button_active_state active_state)
{
  /* Init GPIO */
  nrf_gpio_cfg_input(pin_number, pull_config);

  button->ActiveState = active_state;
  button->PinNumber = pin_number;
}

/**
 * @brief Process button struct. Call every 10 ms.
 */
bool PollButton(Button* button)
{
  uint32_t pinState = nrf_gpio_pin_read(button->PinNumber);
  if(button->ActiveState == BUTTON_ACTIVE_LOW)
    pinState ^= 1;

  return pinState != 0;
}


