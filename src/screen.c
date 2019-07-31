/*
 Screen layer for ugui

Copyright 2019, S. Kevin Hester, kevinh@geeksville.com

(MIT License)
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "screen.h"
#include "lcd.h"
#include "ugui.h"


static UG_COLOR getBackColor(const FieldLayout *layout) {
  switch(layout->color) {
    case ColorInvert: return C_WHITE;

    case ColorNormal:
    default:
      return C_BLACK;
  }
}

static UG_COLOR getForeColor(const FieldLayout *layout) {
  switch(layout->color) {
    case ColorInvert: return C_BLACK;

    case ColorNormal:
    default:
      return C_WHITE;
  }
}

// Returns true if we decided to draw something
typedef bool (*FieldRenderFn)(FieldLayout *layout);

static const FieldRenderFn renderers[];

static bool renderDrawText(FieldLayout *layout) {
  assert(layout->width >= -1);
  assert(layout->height == -1);

  Field *field = layout->field;
  UG_S16 width = (layout->width == -1 ? strlen(field->drawText.msg) : layout->width) * field->drawText.font->char_width;
  UG_S16 height = field->drawText.font->char_height;

  // FIXME, draw/clear background and set colors
  UG_FontSelect(field->drawText.font);
  UG_COLOR back = getBackColor(layout);
  UG_SetBackcolor(back);
  UG_SetForecolor(getForeColor(layout));

  // ug fonts include no blank space at the beginning, so we always include one col of padding
  UG_FillFrame(layout->x, layout->y, layout->x + width -1, layout->y + height -1, back);
  UG_PutString(layout->x + 1, layout->y, field->drawText.msg);
  return true;
}

static bool renderFill(FieldLayout *layout) {
  assert(layout->width >= 1);
  assert(layout->height >= 1);

  UG_FillFrame(layout->x, layout->y, layout->x + layout->width -1, layout->y + layout->height -1, getForeColor(layout));
  return true;
}


static bool renderMesh(FieldLayout *layout) {
  assert(layout->width >= 1);
  assert(layout->height >= 1);

  UG_DrawMesh(layout->x, layout->y, layout->x + layout->width -1, layout->y + layout->height -1, getForeColor(layout));
  return true;
}

#define MAX_SCROLLABLE_ROWS 4 // Max number of rows we can show on one screen (including header)


const Coord screenWidth = 64, screenHeight = 128; // FIXME, for larger devices allow screen objcts to nest inside other screens

const bool renderLayouts(FieldLayout *layouts, bool forceRender) {
  bool didDraw = false; // we only render to hardware if something changed

  // For each field if that field is dirty (or the screen is) redraw it
  for(FieldLayout *layout = layouts; layout->field; layout++) {
    if(layout->field->dirty || forceRender) {
      if(layout->width == 0)
        layout->width = screenWidth - layout->x;

      if(layout->height == 0)
        layout->height = screenHeight - layout->y;

      didDraw |= renderers[layout->field->variant](layout);
    }
  }

  // We clear the dirty bits in a separate pass because multiple layouts on the screen might share the same field
  for(const FieldLayout *layout = layouts; layout->field; layout++) {
    layout->field->dirty = false;
  }

  return didDraw;
}

// If true, the scroll position changed and force a complete redraw
// FIXME - heading shouldn't be redrawn
// FIXME - currently limited to one scrollable per screen
static bool forceScrollableRelayout;

static bool renderScrollable(FieldLayout *layout) {
  const Coord rowHeight = 32; // 3 data rows 32 pixels tall + one 32 pixel header

  // The (currently only one allowed per screen) scrollable that is currently being shown to the user.
  // if the scrollable changes, we'll need to regenerate the entire render
  static Field *curActiveScrollable = NULL;

  static FieldLayout rows[MAX_SCROLLABLE_ROWS + 1]; // Used to layout each of the currently visible rows + heading
  static Field blankRows[MAX_SCROLLABLE_ROWS]; // Used to fill with blank space if necessary

  static Field heading = FIELD_DRAWTEXT(&FONT_5X12);

  Field *field = layout->field;
  forceScrollableRelayout |= (field != curActiveScrollable);

  curActiveScrollable = layout->field;

  if(forceScrollableRelayout) {
    bool hasMoreRows = true; // Once we reach an invalid row we stop rendering and instead fill with blank space

    forceScrollableRelayout = false;
    for(int i = 0; i < MAX_SCROLLABLE_ROWS; i++) {
      FieldLayout *r = rows + i;

      r->x = layout->x;
      r->y = layout->y + rowHeight * i;
      r->width = layout->width;
      r->height = rowHeight;

      if(i == 0) { // heading
        fieldPrintf(&heading, "%s", field->scrollable.label);
        r->field = &heading;
        r->color = ColorInvert;
      }
      else {
        // visible menu rows, starting with where the user has scrolled to
        const int entryNum = field->scrollable.first + i - 1;
        Field *entry = &field->scrollable.entries[entryNum];

        if(entry->variant == FieldEnd)
          hasMoreRows = false;

        // if the current row is valid, render that, otherwise render blank space
        if(hasMoreRows)
          r->field = entry;
        else {
          r->field = &blankRows[i];
          r->field->variant = FieldFill;
          r->color = (entryNum == field->scrollable.selected) ? ColorSelected : ColorNormal;
        }

        r->field->dirty = true; // Force rerender
      }

      rows[MAX_SCROLLABLE_ROWS].field = NULL; // mark end of array (for rendering)
    }
  }

  // draw (or redraw if necessary) our current set of visible rows
  return renderLayouts(rows, false);
}

static bool renderEditable(FieldLayout *layout) {
  return true;
}

static bool renderEnd(FieldLayout *layout) {
  assert(0); // This should never be called I think
  return true;
}


void onPressScrollable() {
  // FIXME: if first or selected changed, mark our scrollable as dirty (so child editables can be drawn)

}
/**
 * Used to map from FieldVariant enums to rendering functions
 */
static const FieldRenderFn renderers[] = {
    renderDrawText,
    renderFill,
    renderMesh,
    renderScrollable,
    renderEditable,
    renderEnd
};

static Screen *curScreen;
static bool screenDirty;

void screenShow(Screen *screen) {
  curScreen = screen;
  screenDirty = true;
  screenUpdate(); // Force a draw immediately
}



void screenUpdate() {
  if(!curScreen)
    return;

  bool didDraw = false; // we only render to hardware if something changed

  if(screenDirty) {
    ; // clear screen (to prevent turds from old screen staying around)
    UG_FillScreen(C_BLACK);
    didDraw = true;
  }

  // For each field if that field is dirty (or the screen is) redraw it
  didDraw |= renderLayouts(*curScreen, screenDirty);

  // flush the screen to the hardware
  if(didDraw) {
    lcd_refresh();
  }

  screenDirty = false;
}

void fieldPrintf(Field *field, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  char buf[sizeof(field->drawText.msg)] = "";
  vsnprintf(buf, sizeof(buf), fmt, argp);
  if(strcmp(buf, field->drawText.msg) != 0) {
    strcpy(field->drawText.msg, buf);
    field->dirty = true;
  }

  va_end(argp);
}

