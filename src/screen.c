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
#include "screen.h"
#include "lcd.h"
#include "ugui.h"

typedef void (*FieldRenderFn)(const FieldLayout *layout);

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


static void renderDrawText(const FieldLayout *layout) {
  assert(layout->width >= 1);
  assert(layout->height == -1);

  Field *field = layout->field;
  // UG_S16 width = layout->width * field->drawText.font->char_width;
  // UG_S16 height = field->drawText.font->char_height;

  // FIXME, draw/clear background and set colors
  UG_FontSelect(field->drawText.font);
  UG_SetBackcolor(getBackColor(layout));
  UG_SetForecolor(getForeColor(layout));
  UG_PutString(layout->x, layout->y, field->drawText.msg);
}

/**
 * Used to map from FieldVariant enums to rendering functions
 */
const FieldRenderFn renderers[] = {
    renderDrawText
};

static const Screen *curScreen;
static bool screenDirty;

void screenShow(const Screen *screen) {
  curScreen = screen;
  screenDirty = true;
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
  for(const FieldLayout *layout = *curScreen; layout->field; layout++) {
    if(layout->field->dirty || screenDirty) {
      renderers[layout->field->variant](layout);
      didDraw = true;
    }
  }

  // We clear the dirty bits in a separate pass because multiple layouts on the screen might share the same field
  for(const FieldLayout *layout = *curScreen; layout->field; layout++) {
    if(layout->field->dirty || screenDirty) {
      layout->field->dirty = false;
    }
  }

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

