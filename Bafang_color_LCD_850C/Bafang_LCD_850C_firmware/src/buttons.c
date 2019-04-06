/*
 * Bafang / Aptdevelop 850C color display  firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stdio.h"
#include "pins.h"
#include "buttons.h"

uint32_t ui32_onoff_button_state = 0;
uint32_t ui32_onoff_button_state_counter = 0;
uint32_t ui32_down_button_state = 0;
uint32_t ui32_down_button_state_counter = 0;
uint32_t ui32_up_button_state = 0;
uint32_t ui32_up_button_state_counter = 0;

buttons_events_t buttons_events = 0;

uint32_t buttons_get_up_state (void)
{
  return GPIO_ReadInputDataBit(BUTTON_UP__PORT, BUTTON_UP__PIN) != 0 ? 0: 1;
}

uint32_t buttons_get_up_click_event (void)
{
  return (buttons_events & UP_CLICK) ? 1: 0;
}

uint32_t buttons_get_up_long_click_event (void)
{
  return (buttons_events & UP_LONG_CLICK) ? 1: 0;
}

void buttons_clear_up_click_event (void)
{
  buttons_events &= ~UP_CLICK;
}

uint32_t buttons_get_up_click_long_click_event (void)
{
  return (buttons_events & UP_CLICK_LONG_CLICK) ? 1: 0;
}

void buttons_clear_up_long_click_event (void)
{
  buttons_events &= ~UP_LONG_CLICK;
}

void buttons_clear_up_click_long_click_event (void)
{
  buttons_events &= ~UP_CLICK_LONG_CLICK;
}

uint32_t buttons_get_down_state (void)
{
  return GPIO_ReadInputDataBit(BUTTON_DOWN__PORT, BUTTON_DOWN__PIN) != 0 ? 0: 1;
}

uint32_t buttons_get_down_click_event (void)
{
  return (buttons_events & DOWN_CLICK) ? 1: 0;
}

uint32_t buttons_get_down_long_click_event (void)
{
  return (buttons_events & DOWN_LONG_CLICK) ? 1: 0;
}

void buttons_clear_down_click_event (void)
{
  buttons_events &= ~DOWN_CLICK;
}

void buttons_clear_down_click_long_click_event (void)
{
  buttons_events &= ~DOWN_CLICK_LONG_CLICK;
}

uint32_t buttons_get_down_click_long_click_event (void)
{
  return (buttons_events & DOWN_CLICK_LONG_CLICK) ? 1: 0;
}

void buttons_clear_down_long_click_event (void)
{
  buttons_events &= ~DOWN_LONG_CLICK;
}

uint32_t buttons_get_onoff_state (void)
{
  return GPIO_ReadInputDataBit(BUTTON_ONOFF__PORT, BUTTON_ONOFF__PIN) != 0 ? 0: 1;
}

uint32_t buttons_get_onoff_click_event (void)
{
  return (buttons_events & ONOFF_CLICK) ? 1: 0;
}

uint32_t buttons_get_onoff_long_click_event (void)
{
  return (buttons_events & ONOFF_LONG_CLICK) ? 1: 0;
}

uint32_t buttons_get_onoff_click_long_click_event (void)
{
  return (buttons_events & ONOFF_CLICK_LONG_CLICK) ? 1: 0;
}

void buttons_clear_onoff_click_event (void)
{
  buttons_events &= ~ONOFF_CLICK;
}

void buttons_clear_onoff_click_long_click_event (void)
{
  buttons_events &= ~ONOFF_CLICK_LONG_CLICK;
}

void buttons_clear_onoff_long_click_event (void)
{
  buttons_events &= ~ONOFF_LONG_CLICK;
}

uint32_t buttons_get_up_down_click_event (void)
{
  return (buttons_events & UPDOWN_CLICK) ? 1: 0;
}

void buttons_clear_up_down_click_event (void)
{
  buttons_events &= ~UPDOWN_CLICK;
}

buttons_events_t buttons_get_events (void)
{
  return buttons_events;
}

void buttons_set_events (buttons_events_t events)
{
  buttons_events |= events;
}

void buttons_clear_all_events (void)
{
  buttons_events = 0;
  ui32_onoff_button_state = 0;
  ui32_up_button_state = 0;
  ui32_down_button_state = 0;
}

void buttons_clock (void)
{
  // needed if the event is not cleared anywhere else
  buttons_clear_onoff_click_long_click_event();

  switch (ui32_onoff_button_state)
  {
    case 0:
      if (!buttons_get_onoff_click_event() &&
          !buttons_get_onoff_long_click_event() &&
          !buttons_get_onoff_click_long_click_event() &&
          buttons_get_onoff_state ())
        {
          ui32_onoff_button_state_counter = 0;
          ui32_onoff_button_state = 1;
        }
    break;

    case 1:
      ui32_onoff_button_state_counter++;

      // event long click
      if (ui32_onoff_button_state_counter > 100) // 2 seconds
      {
        buttons_set_events(ONOFF_LONG_CLICK);
        ui32_onoff_button_state = 2;
        break;
      }

      // if button release
      if (!buttons_get_onoff_state ())
      {
        // let's validade if will be a quick click + long click
        if (ui32_onoff_button_state_counter <= 2) // 0.3 second
        {
          ui32_onoff_button_state_counter = 0;
          ui32_onoff_button_state = 3;
          break;
        }
        // event click
        else
        {
          buttons_set_events(ONOFF_CLICK);
          ui32_onoff_button_state = 0;
          break;
        }
      }
    break;

    case 2:
      // wait for button release
      if (!buttons_get_onoff_state ())
      {
        ui32_onoff_button_state = 0;
        break;
      }
    break;

    case 3:
      ui32_onoff_button_state_counter++;

      // on next step, start counting for long click
      if (buttons_get_onoff_state ())
      {
        ui32_onoff_button_state_counter = 0;
        ui32_onoff_button_state = 4;
        break;
      }

      // event click
      if (ui32_onoff_button_state_counter > 20)
      {
        buttons_set_events(ONOFF_CLICK);
        ui32_onoff_button_state = 0;
        break;
      }
    break;

    case 4:
      ui32_onoff_button_state_counter++;

      // event click, but this time it is: click + long click
      if (ui32_onoff_button_state_counter > 50)
      {
        buttons_set_events(ONOFF_CLICK_LONG_CLICK);
        ui32_onoff_button_state = 2;
        break;
      }

      // button release
      if (!buttons_get_onoff_state ())
      {
        buttons_set_events(ONOFF_CLICK);
        ui32_onoff_button_state = 0;
        break;
      }
    break;

    default:
      ui32_onoff_button_state = 0;
    break;
  }

  switch (ui32_up_button_state)
  {
    case 0:
      if (!buttons_get_up_click_event() &&
          !buttons_get_up_long_click_event() &&
          !buttons_get_up_click_long_click_event() &&
          !buttons_get_up_down_click_event() &&
          buttons_get_up_state())
        {
          ui32_up_button_state_counter = 0;
          ui32_up_button_state = 1;
        }
    break;

    case 1:
      ui32_up_button_state_counter++;

      // event long click
      if (ui32_up_button_state_counter++ > 100) // 2 seconds
      {
        // up and down button click
        if (ui32_down_button_state == 1)
        {
          buttons_set_events(UPDOWN_CLICK);
          ui32_down_button_state = 2;
        }
        else
        {
          buttons_set_events(UP_LONG_CLICK);
        }

        ui32_up_button_state = 2;
        ui32_up_button_state_counter = 0;
        break;
      }

      // if button release
      if (!buttons_get_up_state ())
      {
        // let's validade if will be a quick click + long click
        if (ui32_up_button_state_counter <= 2) // 0.3 second
        {
          ui32_up_button_state_counter = 0;
          ui32_up_button_state = 3;
          break;
        }
        // event click
        else
        {
          buttons_set_events(UP_CLICK);
          ui32_up_button_state = 0;
          break;
        }
      }
    break;

    case 2:
      // wait for button release
      if (!buttons_get_up_state ())
      {
        ui32_up_button_state = 0;
        break;
      }
    break;

    case 3:
      ui32_up_button_state_counter++;

      // on next step, start counting for long click
      if (buttons_get_up_state ())
      {
        ui32_up_button_state_counter = 0;
        ui32_up_button_state = 4;
        break;
      }

      // event click
      if (ui32_up_button_state_counter > 20)
      {
        buttons_set_events(UP_CLICK);
        ui32_up_button_state = 0;
        break;
      }
    break;

    case 4:
      ui32_up_button_state_counter++;

      // event click, but this time it is: click + long click
      if (ui32_up_button_state_counter > 50)
      {
        buttons_set_events(UP_CLICK_LONG_CLICK);
        ui32_up_button_state = 2;
        break;
      }

      // button release
      if (!buttons_get_up_state ())
      {
        buttons_set_events(UP_CLICK);
        ui32_up_button_state = 0;
        break;
      }
    break;

    default:
      ui32_up_button_state = 0;
    break;
  }

  switch (ui32_down_button_state)
  {
    case 0:
      if (!buttons_get_down_click_event() &&
          !buttons_get_down_long_click_event() &&
          !buttons_get_down_click_long_click_event() &&
          !buttons_get_up_down_click_event() &&
          buttons_get_down_state())
        {
          ui32_down_button_state_counter = 0;
          ui32_down_button_state = 1;
        }
    break;

    case 1:
      ui32_down_button_state_counter++;

      // event long click
      if (ui32_down_button_state_counter++ > 100) // 2 seconds
      {
        // up and down button click
        if (ui32_up_button_state == 1)
        {
          buttons_set_events(UPDOWN_CLICK);
          ui32_up_button_state = 2;
        }
        else
        {
          buttons_set_events(DOWN_LONG_CLICK);
        }

        ui32_down_button_state = 2;
        ui32_down_button_state_counter = 0;
        break;
      }


      // if button release
      if (!buttons_get_down_state ())
      {
        // let's validade if will be a quick click + long click
        if (ui32_down_button_state_counter <= 2) // 0.3 second
        {
          ui32_down_button_state_counter = 0;
          ui32_down_button_state = 3;
          break;
        }
        // event click
        else
        {
          buttons_set_events(DOWN_CLICK);
          ui32_down_button_state = 0;
          break;
        }
      }
    break;

    case 2:
      // wait for button release
      if (!buttons_get_down_state ())
      {
        ui32_down_button_state = 0;
        break;
      }
    break;

    case 3:
      ui32_down_button_state_counter++;

      // on next step, start counting for long click
      if (buttons_get_down_state ())
      {
        ui32_down_button_state_counter = 0;
        ui32_down_button_state = 4;
        break;
      }

      // event click
      if (ui32_down_button_state_counter > 15)
      {
        buttons_set_events(DOWN_CLICK);
        ui32_down_button_state = 0;
        break;
      }
    break;

    case 4:
      ui32_down_button_state_counter++;

      // event click, but this time it is: click + long click
      if (ui32_down_button_state_counter > 50)
      {
        buttons_set_events(DOWN_CLICK_LONG_CLICK);
        ui32_down_button_state = 2;
        break;
      }

      // button release
      if (!buttons_get_down_state ())
      {
        buttons_set_events(DOWN_CLICK);
        ui32_down_button_state = 0;
        break;
      }
    break;

    default:
      ui32_down_button_state = 0;
    break;
  }
}
