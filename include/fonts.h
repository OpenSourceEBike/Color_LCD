
#ifndef INCLUDE_FONTS_H_
#define INCLUDE_FONTS_H_

#include "ugui.h"


/* Enable needed fonts here */
#define  USE_MY_FONT_8X12
#define  USE_MY_FONT_NUM_10X16
#define  USE_MY_FONT_NUM_24X40
#define  USE_MY_FONT_BATTERY

#ifdef USE_MY_FONT_BATTERY
const UG_FONT MY_FONT_BATTERY;
#endif

#ifdef USE_MY_FONT_8X12
const UG_FONT MY_FONT_8X12;
#endif

#ifdef USE_MY_FONT_NUM_10X16
const UG_FONT MY_FONT_NUM_10X16;
#endif

#ifdef USE_MY_FONT_NUM_24X40
const UG_FONT MY_FONT_NUM_24X40;
#endif

#endif /* INCLUDE_FONTS_H_ */
