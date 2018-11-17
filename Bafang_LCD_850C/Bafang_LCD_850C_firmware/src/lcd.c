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
#include <math.h>

#include "pins.h"
#include "lcd.h"
#include "buttons.h"
#include "eeprom.h"
#include "ugui_driver/ugui_bafang_500c.h"
#include "ugui/ugui.h"

static struct_motor_controller_data motor_controller_data;
static struct_configuration_variables configuration_variables;

void lcd_execute_main_screen (void);
void assist_level_state (void);
void power_off_management (void);
void lcd_power_off (uint8_t updateDistanceOdo);

// reverses a string 'str' of length 'len'
void reverse(char *str, int len)
{
    int i=0, j=len-1, temp;
    while (i<j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}

 // Converts a given integer x to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x%10) + '0';
        x = x/10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.';  // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

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

  // init variables with the stored value on EEPROM
  eeprom_init_variables ();
}

void lcd_clock(void)
{
  lcd_execute_main_screen ();

  // power off system: ONOFF long click event
  power_off_management ();
}

void lcd_draw_main_menu_mask(void)
{
  UG_DrawLine(10, 60, 310, 60, C_DIM_GRAY);

  UG_DrawLine(10, 180, 310, 180, C_DIM_GRAY);

  UG_DrawLine(10, 265, 310, 265, C_DIM_GRAY);
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
    buttons_clear_up_click_long_click_event ();
    buttons_clear_up_long_click_event ();
    buttons_clear_down_click_event ();
    buttons_clear_down_click_long_click_event ();
    buttons_clear_down_long_click_event ();

    configuration_variables.ui8_assist_level++;

// TODO
configuration_variables.ui8_number_of_assist_levels = 5;

    if (configuration_variables.ui8_assist_level > configuration_variables.ui8_number_of_assist_levels)
      { configuration_variables.ui8_assist_level = configuration_variables.ui8_number_of_assist_levels; }
  }

  if (buttons_get_down_click_event ())
  {
    buttons_clear_up_click_event ();
    buttons_clear_up_click_long_click_event ();
    buttons_clear_up_long_click_event ();
    buttons_clear_down_click_event ();
    buttons_clear_down_click_long_click_event ();
    buttons_clear_down_long_click_event ();

    if (configuration_variables.ui8_assist_level > 0)
      configuration_variables.ui8_assist_level--;
  }

  UG_SetBackcolor(C_BLACK);
  UG_SetForecolor(C_DIM_GRAY);
  UG_FontSelect(&FONT_10X16);
  UG_PutString(10, 190, "Assist");

  UG_SetForecolor(C_WHITE);
  UG_FontSelect(&FONT_32X53);
  UG_PutString(25, 210, itoa((uint32_t) configuration_variables.ui8_assist_level));
}

struct_configuration_variables* get_configuration_variables (void)
{
  return &configuration_variables;
}

struct_motor_controller_data* lcd_get_motor_controller_data (void)
{
  return &motor_controller_data;
}

void power_off_management (void)
{
  // turn off
  if (buttons_get_onoff_long_click_event ()) { lcd_power_off (1); }
}

void lcd_power_off (uint8_t updateDistanceOdo)
{
//  if (updateDistanceOdo)
//  {
//    configuration_variables.ui32_wh_x10_offset = ui32_wh_x10;
//    configuration_variables.ui32_odometer_x10 += ((uint32_t) configuration_variables.ui16_odometer_distance_x10);
//  }

  eeprom_write_variables ();

  // put screen all black and disable backlight
  UG_FillScreen(0);
  lcd_backlight(0);

  // now disable the power to all the system
  system_power(0);

  // block here
  while (1) ;
}
