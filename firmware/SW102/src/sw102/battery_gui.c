#include "state.h"
#include "screen.h"
#include "lcd.h"

#define BATTERY_SOC_START_X 1
#define BATTERY_SOC_START_Y 0
#define BATTERY_SOC_WITH    25
#define BATTERY_SOC_HEIGHT  11

bool renderBattery(FieldLayout *layout)
{
  uint32_t ui32_x1, ui32_x2;
  uint32_t ui32_y1, ui32_y2;
  int16_t height = BATTERY_SOC_HEIGHT;
  int16_t width = BATTERY_SOC_WITH;
  uint8_t battery_bars;

  if (ui8_g_battery_soc > 0) {
    battery_bars = ui8_g_battery_soc / 10;
    battery_bars++; // always show an higher bar, like 82% will show 9 bars and not 8
  } else {
    battery_bars = 0;
  }

  // first, clear the full symbol area
  ui32_x1 = BATTERY_SOC_START_X + 2;
  ui32_y1 = BATTERY_SOC_START_Y + 2;
  ui32_x2 = ui32_x1 + 18 - 1;
  ui32_y2 = ui32_y1 + 7 - 1;
  UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

  ui32_x1 = BATTERY_SOC_START_X + 2 + 18;
  ui32_y1 = BATTERY_SOC_START_Y + 2 + 2;
  ui32_x2 = ui32_x1 + 2;
  ui32_y2 = ui32_y1 + 2;
  UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

  // draw the bars
  if (battery_bars > 0 &&
      battery_bars < 10) {
    ui32_x1 = BATTERY_SOC_START_X + 2;
    ui32_y1 = BATTERY_SOC_START_Y + 2;
    ui32_x2 = ui32_x1 + (battery_bars * 2) - 1;
    ui32_y2 = ui32_y1 + 7 - 1;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

  } else if (battery_bars >= 10) {
    ui32_x1 = BATTERY_SOC_START_X + 2;
    ui32_y1 = BATTERY_SOC_START_Y + 2;
    ui32_x2 = ui32_x1 + (9 * 2) - 1;
    ui32_y2 = ui32_y1 + 7 - 1;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

    ui32_x1 = BATTERY_SOC_START_X + 2 + 18;
    ui32_y1 = BATTERY_SOC_START_Y + 2 + 2;
    ui32_x2 = ui32_x1 + 2;
    ui32_y2 = ui32_y1 + 2;
    UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);
  }

  return true;
}

bool batteryClearSymbol(void)
{
  uint32_t ui32_x1, ui32_x2;
  uint32_t ui32_y1, ui32_y2;
  uint16_t ui16_color;
  uint32_t ui32_i;

  int16_t height = BATTERY_SOC_HEIGHT;
  int16_t width = BATTERY_SOC_WITH;

  // first, clear the full symbol area
  ui32_x1 = BATTERY_SOC_START_X;
  ui32_y1 = BATTERY_SOC_START_Y;
  ui32_x2 = ui32_x1 + width;
  ui32_y2 = ui32_y1 + height;
  UG_FillFrame(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_BLACK);

  ui32_x1 = BATTERY_SOC_START_X;
  ui32_y1 = BATTERY_SOC_START_Y;
  ui32_x2 = ui32_x1 + 21;
  ui32_y2 = ui32_y1;
  UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

  ui32_x1 = ui32_x2;
  ui32_y1 = ui32_y2;
  ui32_x2 = ui32_x1;
  ui32_y2 = ui32_y1 + 2;
  UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

  ui32_x1 = ui32_x2;
  ui32_y1 = ui32_y2;
  ui32_x2 = ui32_x1 + 3;
  ui32_y2 = ui32_y1;
  UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

  ui32_x1 = ui32_x2;
  ui32_y1 = ui32_y2;
  ui32_x2 = ui32_x1;
  ui32_y2 = ui32_y1 + 6;
  UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

  ui32_x1 = ui32_x2;
  ui32_y1 = ui32_y2;
  ui32_x2 = ui32_x1 - 3;
  ui32_y2 = ui32_y1;
  UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

  ui32_x1 = ui32_x2;
  ui32_y1 = ui32_y2;
  ui32_x2 = ui32_x1;
  ui32_y2 = ui32_y1 + 2;
  UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

  ui32_x1 = ui32_x2;
  ui32_y1 = ui32_y2;
  ui32_x2 = ui32_x1 - 21;
  ui32_y2 = ui32_y1;
  UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);

  ui32_x1 = ui32_x2;
  ui32_y1 = ui32_y2;
  ui32_x2 = ui32_x1;
  ui32_y2 = ui32_y1 - 9;
  UG_DrawLine(ui32_x1, ui32_y1, ui32_x2, ui32_y2, C_WHITE);
}
