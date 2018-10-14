# UTFT-STM8S
This is a modified version of the UTFT Universal TFT display library:
http://www.rinkydinkelectronics.com/library.php?id=51

It is "converted" from C++ to C so it can be used with SDuino/SDCC for STM8S microcontrollers.

Motivation was to use it for the XH18-LCD from the Tongsheng TSDZ2 ebike motor.

## Changes
* Functions and variables now have a "UTFT_" prefix and are no more bundled in a class
  * UTFT::InitLCD() -> UTFT_InitLCD()
* Some overloaded functions or functions with optional parameters exist now with multiple names
  * e.g. UTFT_setColor() and UTFT_setColor_rgb()
* An offset for x and y can be defined via variables:
  * UTFT_offset_x, UTFT_offset_y
* 8bit interface support for ST7735

## Limitations
* Most of the hardware specific part is just implemented for the needs of the XH18-LCD:
  * 8bit parallel mode for a crude pin mapping
  * NOT for SPI or 16bit mode
* Drivers for other LCDs should work but are not tested and disabled in memorysaver.h to save space
* <ins>Most is yet untested!</ins>

## Usage
* Get and install SDuino: https://github.com/tenbaht/sduino
* Put files these files in Documents\\Arduino\\libraries\\UTFT
* Add or adjust hardware specific part as needed
  * See original UTFT library for examples
* Lookup function names in UTFT.h
* See example: [examples/UTFT_xh18lcd.ino](/examples/UTFT_xh18lcd.ino)
![XH18-LCD Example](/examples/UTFT_xh18lcd.jpg)
