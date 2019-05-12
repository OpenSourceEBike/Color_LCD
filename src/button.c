#include "button.h"

/**
 * @brief Init button struct. Call once.
 */
void InitButton(Button* button, uint32_t pin_number, nrf_gpio_pin_pull_t pull_config, button_active_state active_state)
{
  /* Init GPIO */
  nrf_gpio_cfg_input(pin_number, pull_config);

  button->State = (BTTN_CLEAR | BTTN_RELEASED_PROCESSED); // if we just set to BTTN_CLEAR, ButtonReleased() triggers on startup
  button->DebounceCnt = button->LongClickCnt = 0;
  button->DoubleClickCnt = DOUBLECLICK_TIME;
  button->ActiveState = active_state;
  button->PinNumber = pin_number;
}

/**
 * @brief Process button struct. Call every 10 ms.
 */
void PollButton(Button* button)
{
  uint32_t pinState = nrf_gpio_pin_read(button->PinNumber);
  pinState ^= button->ActiveState;

  /* Not Active */
  if (pinState != 0)
  {
    button->DebounceCnt = button->LongClickCnt = 0;
    button->DoubleClickCnt++;
    /*
     * Reset only after clicked state reached. Important not just to set button->State to BTTN_CLEAR
     * or ButtonReleased() triggers over and over again!
     */
    if (button->State & BTTN_CLICK)
    {
      button->State = BTTN_CLEAR;
      button->DoubleClickCnt = 0;
    }
  }
  /* Active */
  else
  {
    /* Click & DoubleClick */
    if (button->DebounceCnt++ >= DEBOUNCE_TIME)
    {
      button->State |= BTTN_CLICK;

      if (button->DoubleClickCnt < DOUBLECLICK_TIME)
        button->State |= BTTN_DBLCLICK;
    }

    /* Long Click */
    if (button->LongClickCnt++ >= LONGCLICK_TIME)
      button->State |= BTTN_LONGCLICK;
  }
}

/**
 * @brief Check if Button is clicked & return true (once if CLICKED_SIGNAL_ONCE > 0)
 */
bool ButtonClicked(Button* button)
{
  if ((button->State & BTTN_CLICK) && !(button->State & BTTN_CLICK_PROCESSED))
  {
#if (CLICKED_SIGNAL_ONCE > 0)
    button->State |= BTTN_CLICK_PROCESSED;
#endif
    return true;
  }

  return false;
}

/**
 * @brief Check if Button is long clicked & return true (once if LONGCLICKED_SIGNAL_ONCE > 0)
 */
bool ButtonLongClicked(Button* button)
{
  if ((button->State & BTTN_LONGCLICK) && !(button->State & BTTN_LC_PROCESSED))
  {
#if (LONGCLICKED_SIGNAL_ONCE > 0)
    button->State |= BTTN_LC_PROCESSED;
#endif
    return true;
  }

  return false;
}

/**
 * @brief Check if Button is double clicked & return true (once if DOUBLECLICKED_SIGNAL_ONCE > 0)
 */
bool ButtonDoubleClicked(Button* button)
{
  if ((button->State & BTTN_DBLCLICK) && !(button->State & BTTN_DC_PROCESSED))
  {
#if (DOUBLECLICKED_SIGNAL_ONCE > 0)
    button->State |= BTTN_DC_PROCESSED;
#endif
    return true;
  }

  return false;
}

/**
 * @brief Check if Button was released after clicked & return true once
 */
bool ButtonReleased(Button* button)
{
  if (button->State == BTTN_CLEAR)
  {
    button->State |= BTTN_RELEASED_PROCESSED;
    return true;
  }

  return false;
}
