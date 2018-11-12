/*
 * Bafang LCD 850C firmware
 *
 * Copyright (C) Casainho, 2018.
 *
 * Released under the GPL License, Version 3
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stdio.h"

#include "pins.h"
#include "ugui_driver/ugui_bafang_500c.h"

extern uint8_t BigFont[];

#define INT_DIGITS 19   /* enough for 64 bit integer */

uint8_t *itoa(uint8_t ui8_i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1; /* points to terminating '\0' */
  if (ui8_i >= 0) {
    do {
      *--p = '0' + (ui8_i % 10);
      ui8_i /= 10;
    } while (ui8_i != 0);
    return p;
  }
  else {      /* i < 0 */
    do {
      *--p = '0' - (ui8_i % 10);
      ui8_i /= 10;
    } while (ui8_i != 0);
    *--p = '-';
  }
  return p;
}

/* Place your initialization/startup code here (e.g. MyInst_Start()) */
void lcd_init(void)
{
  bafang_500C_lcd_init();
  UG_FillScreen(0);
  UG_FontSelect(&FONT_10X16);
}

void lcd_draw_main_menu(void)
{
//  UG_COLOR color[3];
//
//  color[0] = C_RED;
//  color[1] = C_GREEN;
//  color[2] = C_BLUE;

//  UG_FontSelect(&FONT_10X16);

  UG_PutString(10, 10, "test mode");

  //  static buttons_events_type_t events = 0;
  //  static buttons_events_type_t last_events = 0;
  //  while (1)
  //  {
  //    delay_ms(10);
  //    buttons_clock();
  //
  ////    UG_FillScreen(0);
  //    UG_PutString(10, 10, itoa(buttons_get_onoff_state ()));
  //    UG_PutString(10, 50, itoa(buttons_get_up_state ()));
  //    UG_PutString(10, 90, itoa(buttons_get_down_state ()));
  //
  //    events = buttons_get_events ();
  //    if (events != 0)
  //    {
  //      last_events = events;
  //      buttons_clear_all_events();
  //    }
  //    UG_PutString(10, 125, "   ");
  //    UG_PutString(10, 125, itoa((uint32_t) last_events));
  //  }
}
