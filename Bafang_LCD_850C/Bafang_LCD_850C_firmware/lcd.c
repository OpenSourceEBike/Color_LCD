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
#include "lcd.h"
#include "buttons.h"
#include "ugui_driver/ugui_bafang_500c.h"
#include "ugui/ugui.h"

static struct_motor_controller_data motor_controller_data;
static struct_configuration_variables configuration_variables;

void lcd_execute_main_screen (void);
void assist_level_state (void);


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
}

void lcd_draw_main_menu(void)
{
  lcd_execute_main_screen ();
}

void lcd_execute_main_screen (void)
{
//  temperature ();
  assist_level_state ();
//  odometer ();
//  wheel_speed ();
//  walk_assist_state ();
//  offroad_mode ();
//  power ();
//  battery_soc ();
//  lights_state ();
//  brake ();
}

static buttons_events_type_t events = 0;
static buttons_events_type_t last_events = 0;

void assist_level_state (void)
{
  if (buttons_get_up_click_event ())
  {
    buttons_clear_up_click_event ();

    configuration_variables.ui8_assist_level++;

// TODO
configuration_variables.ui8_number_of_assist_levels = 5;

    if (configuration_variables.ui8_assist_level > configuration_variables.ui8_number_of_assist_levels)
      { configuration_variables.ui8_assist_level = configuration_variables.ui8_number_of_assist_levels; }
  }

  if (buttons_get_down_click_event ())
  {
    buttons_clear_down_click_event ();

    if (configuration_variables.ui8_assist_level > 0)
      configuration_variables.ui8_assist_level--;
  }

  UG_FontSelect(&FONT_16X26);
  UG_PutString(10, 10, "Assist");
  UG_FontSelect(&FONT_32X53);
  UG_PutString(10, 50, itoa((uint32_t) configuration_variables.ui8_assist_level));
}
