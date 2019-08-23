#include "state.h"
#include "screen.h"

// Battery SOC symbol:
// 10 bars, each bar: with = 7, height = 24
// symbol has contour lines of 1 pixel
#define BATTERY_SOC_START_X 0
#define BATTERY_SOC_START_Y 0
#define BATTERY_SOC_BAR_WITH 7
#define BATTERY_SOC_BAR_HEIGHT 24
#define BATTERY_SOC_CONTOUR 1


void battery_soc_bar_clear(uint32_t ui32_bar_number)
{
  uint32_t ui32_x1, ui32_x2;
  uint32_t ui32_y1, ui32_y2;

  // the first nine bars share the same code
  if (ui32_bar_number < 10)
  {
    // draw the bar itself
    ui32_x1 = BATTERY_SOC_START_X + BATTERY_SOC_CONTOUR + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * (ui32_bar_number - 1));
    ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR;
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH;
    ui32_y2 = ui32_y1 + BATTERY_SOC_BAR_HEIGHT;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

    // draw bar contour
    if(ui32_bar_number < 9)
    {
      ui32_x1 = ui32_x2 + BATTERY_SOC_CONTOUR;
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_BLACK);
    }
    else
    {
      ui32_x1 = ui32_x2 + 1;
      ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR + (BATTERY_SOC_BAR_HEIGHT / 4);
      ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2);
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_BLACK);
    }
  }
  else
  {
    ui32_x1 = BATTERY_SOC_START_X + BATTERY_SOC_CONTOUR + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 9);
    ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR + (BATTERY_SOC_BAR_HEIGHT / 4);
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2);
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);
  }
}

void battery_soc_bar_set(uint32_t ui32_bar_number, uint16_t ui16_color)
{
  uint32_t ui32_x1, ui32_x2;
  uint32_t ui32_y1, ui32_y2;

  // the first nine bars share the same code
  if(ui32_bar_number < 10)
  {
    ui32_x1 = BATTERY_SOC_START_X + BATTERY_SOC_CONTOUR + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * (ui32_bar_number - 1));
    ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR;
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH;
    ui32_y2 = ui32_y1 + BATTERY_SOC_BAR_HEIGHT;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, ui16_color);

    if(ui32_bar_number < 9)
    {
      ui32_x1 = ui32_x2 + 1;
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_DIM_GRAY);
    }
    else
    {
      ui32_x1 = ui32_x2 + 1;
      ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR + (BATTERY_SOC_BAR_HEIGHT / 4);
      ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2);
      UG_DrawLine(ui32_x1, ui32_y1, ui32_x1, ui32_y2, C_DIM_GRAY);
    }
  }
  else
  {
    ui32_x1 = BATTERY_SOC_START_X + BATTERY_SOC_CONTOUR + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 9);
    ui32_y1 = BATTERY_SOC_START_Y + BATTERY_SOC_CONTOUR + (BATTERY_SOC_BAR_HEIGHT / 4);
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2);
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, ui16_color);
  }
}

bool renderBattery(FieldLayout *layout)
{
  uint32_t ui32_x1, ui32_x2;
  uint32_t ui32_y1, ui32_y2;
  static uint32_t ui32_battery_bar_number_previous = 0;
  uint16_t ui16_color;
  static uint16_t ui16_color_previous;
  uint32_t ui32_temp;
  uint32_t ui32_i;

  layout->height = BATTERY_SOC_BAR_HEIGHT + BATTERY_SOC_CONTOUR * 2;
  layout->width = ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR) * 9) + (BATTERY_SOC_CONTOUR * 2);

  bool drawOutline = true; // For now we do this every time, because we are called to draw ourselves so rarely
  if(drawOutline)
  {
    // first, clear the full symbol area
    // first 9 bars
    ui32_x1 = BATTERY_SOC_START_X;
    ui32_y1 = BATTERY_SOC_START_Y;
    ui32_x2 = ui32_x1 + layout->width;
    ui32_y2 = ui32_y1 + layout->height;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

    // last small bar
    ui32_x1 = ui32_x2;
    ui32_y1 = BATTERY_SOC_START_Y + (BATTERY_SOC_BAR_HEIGHT / 4);
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH + (BATTERY_SOC_CONTOUR * 2);
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2) + (BATTERY_SOC_CONTOUR * 2);
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

    // now draw the empty battery symbol
    // first 9 bars
    ui32_x1 = BATTERY_SOC_START_X;
    ui32_y1 = BATTERY_SOC_START_Y;
    ui32_x2 = ui32_x1 + ((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 9) + (BATTERY_SOC_CONTOUR * 2) - 2;
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    // last bar
    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 4);
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1 + BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1;
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 2) + (BATTERY_SOC_CONTOUR * 2);
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1 - (BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1);
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 + (BATTERY_SOC_BAR_HEIGHT / 4);
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1 - (((BATTERY_SOC_BAR_WITH + BATTERY_SOC_CONTOUR + 1) * 9) + (BATTERY_SOC_CONTOUR * 2) - 2);
    ui32_y2 = ui32_y1;
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = ui32_x2;
    ui32_y1 = ui32_y2;
    ui32_x2 = ui32_x1;
    ui32_y2 = ui32_y1 - (BATTERY_SOC_BAR_HEIGHT + BATTERY_SOC_CONTOUR);
    UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);
  }

  // update battery level value only at every 1 second and this helps to visual filter the fast changing values
  // FIXME: (ui8_timmer_counter++ >= 50) || (m_lcd_vars.ui32_main_screen_draw_static_info)
  bool drawContents = true;
  if(drawContents)
  {
    uint8_t ui32_battery_bar_number = l3_vars.volt_based_soc / (90 / 10); // scale SOC so anything greater than 90% is 10 bars, and zero is zero.

    // find the color to draw the bars
    if(ui32_battery_bar_number > 3) { ui16_color = C_GREEN; }
    else if(ui32_battery_bar_number == 3) { ui16_color = C_YELLOW; }
    else if(ui32_battery_bar_number == 2) { ui16_color = C_ORANGE; }
    else if(ui32_battery_bar_number == 1) { ui16_color = C_RED; }

    // force draw of the bars if needed
    if(drawOutline)
    {
      ui32_battery_bar_number_previous = 0;
    }

    // number of vars are equal as before, nothing new to draw so return
    if(ui32_battery_bar_number == ui32_battery_bar_number_previous)
    {
      // do nothing
    }
    // draw new bars
    else if(ui32_battery_bar_number > ui32_battery_bar_number_previous)
    {
      // we need to redraw the total number of bars
      if(ui16_color != ui16_color_previous)
      {
        for(ui32_i = 1; ui32_i <= ui32_battery_bar_number; ui32_i++)
        {
          battery_soc_bar_set(ui32_i, ui16_color);
        }
      }
      else
      {
        ui32_temp = (ui32_battery_bar_number - ui32_battery_bar_number_previous) + 1;
        for(ui32_i = 1; ui32_i < ui32_temp; ui32_i++)
        {
          battery_soc_bar_set(ui32_battery_bar_number_previous + ui32_i, ui16_color);
        }
      }
    }
    // delete bars
    else if(ui32_battery_bar_number < ui32_battery_bar_number_previous)
    {
      // we need to redraw the total number of bars
      if(ui16_color != ui16_color_previous)
      {
        // first deleted the needed number of vars
        ui32_temp = ui32_battery_bar_number_previous - ui32_battery_bar_number;
        for(ui32_i = 0; ui32_i <= (ui32_temp - 1); ui32_i++)
        {
          battery_soc_bar_clear(ui32_battery_bar_number_previous - ui32_i);
        }

        // now draw the new ones with the new color
        for(ui32_i = 1; ui32_i <= ui32_battery_bar_number; ui32_i++)
        {
          battery_soc_bar_set(ui32_i, ui16_color);
        }
      }
      else
      {
        ui32_temp = ui32_battery_bar_number_previous - ui32_battery_bar_number;
        for(ui32_i = 0; ui32_i <= (ui32_temp - 1); ui32_i++)
        {
          battery_soc_bar_clear(ui32_battery_bar_number_previous - ui32_i);
        }
      }
    }

    ui32_battery_bar_number_previous = ui32_battery_bar_number;
    ui16_color_previous = ui16_color;
  }

  return true;
}
