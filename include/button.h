
#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>
#include <stdbool.h>
#include "nrf_gpio.h"

/* Default defines if ButtonClicked / LongClicked / DoubleClicked returns true once (1) or every time it is called (0) */
#ifndef CLICKED_SIGNAL_ONCE
#define CLICKED_SIGNAL_ONCE       (1)
#endif

typedef enum
{
    BUTTON_ACTIVE_LOW = 0,
    BUTTON_ACTIVE_HIGH = 1
} button_active_state;

typedef struct
{
#if 0
  uint8_t State;
  uint32_t DebounceCnt;
  uint32_t LongClickCnt;
  uint32_t DoubleClickCnt;
#endif
  button_active_state ActiveState;
  uint32_t PinNumber;
} Button;

/* Button */
#define DEBOUNCE_TIME           5     // *10 ms
#define LONGCLICK_TIME          200   // *10 ms
#define DOUBLECLICK_TIME        20    // *10 ms
#define BTTN_CLEAR              0x00
#define BTTN_CLICK              0x01
#define BTTN_LONGCLICK          0x02
#define BTTN_DBLCLICK           0x04
#define BTTN_CLICK_PROCESSED    0x10
#define BTTN_LC_PROCESSED       0x20
#define BTTN_DC_PROCESSED       0x40
#define BTTN_RELEASED_PROCESSED 0x80

void InitButton(Button* button, uint32_t pin_number, nrf_gpio_pin_pull_t pull_config, button_active_state active_state);

// Returns true if button is currently pressed
bool PollButton(Button* button);

bool ButtonClicked(Button* button);
bool ButtonLongClicked(Button* button);
bool ButtonDoubleClicked(Button* button);
bool ButtonReleased(Button* button);

#endif /* BUTTON_H_ */
