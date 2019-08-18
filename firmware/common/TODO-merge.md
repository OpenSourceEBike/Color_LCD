## Easy merges (sequence, listing source files):

* state
* eeprom
* fault
* screen
* rtc

## Mostly done

* buttons - test in sw102, add call to buttons_init

## done

* utils
* remove UG_PutString_with_length copypasta
* ugui
* fonts BITS_FONT_61X99 BITS_FONT_45X72

## Hard merges:

* configscreen
* mainscreen (excluding graphs)
* adc
* boot screen
* add graphs
* final layout for mainscreen

## other work items:

* use colors in the 850C gui
* add instrumentation to measure times of key operations (main loop, frame update etc)
* check that linker is stripping all unused functions (for both platforms)
* dramatically shrink the buttons.c code

