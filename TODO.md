# TODO sequence

* DONE merge 850C style rx comms code with the existing SW102 code
* DONE merge the 850C style tx comms code with the existing SW102 code
* DONE merge the 850C eeprom code, keeping as much as possible in common
* DONE call the rx/tx comms loop every 20ms from the main loop (needed to fix app timers first)
* DONE make the rtc stubs do something
* DONE turn on eeprom writing
* DONE semi-cruftily port over the 850C button code 
* DONE make accel button presses work
* DONE make all other button presses (excluding config) work
* DONE add a logical buttons abstraction and callbacks for screen button handling
* DONE turn on system power / turn off system power per button press
* DONE wait for last flash write to complete before powering down
* show current selection in selection menus
* make config entry presses work
* handle hierarchical menu entry/exit correctly
* Make short press of power - cycle to next screen?
* change fault screen to be serviced via the regular main loop (but only for first fault, to prevent bricking the user's ability to powerdown)
* make a shutdown screen
* make a boot screen (show until user releases power button)
* DONE test to make sure only one EEPROM record is getting written
* test with real motor (after triple checking wiring color questions)
* make config system (hopefully usable on other platforms)
* delete dead code in mainscreen.c
* initial release
* FIXME - pingpong between two rx buffers, current implementation allows ISR to overwrite the buffer being used by
the GUI thread.  Use two buffers + a ptr.
* use battery icon
* pack & align eeprom
* FIXME - make width 0, and height 0 mean "auto select based on display size"
* do eeprom GC if flash is full (currently only done at boot)
* merge with 850C code somewhat? (sharing behavior - just different UX layer and HAL)
* clean up button handling and take advantage of extra button on the SW102

# Misc notes from kevin not yet formatted

* for reference old 0.18 version of comms is described here: https://github.com/OpenSource-EBike-firmware/TSDZ2-Smart-EBike/blob/v0.18.2/src/display/KT-LCD3/uart.c

* install eabi toolchain from https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update/+download/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2
per https://launchpadlibrarian.net/218827232/How-to-build-toolchain.pdf

* nrf on linux instructions here: https://gustavovelascoh.wordpress.com/2017/01/23/starting-development-with-nordic-nrf5x-and-gcc-on-linux/

stlink setup:
You have to connect the following 4 lines: SWDIO, SWCLK, VCC and GND.
https://devzone.nordicsemi.com/f/nordic-q-a/13869/openocd-promgram-nrf51822-with-st-link-v2-mini
https://devzone.nordicsemi.com/f/nordic-q-a/4984/st-link-nrf51822-and-openocd <- best next steps
https://devzone.nordicsemi.com/f/nordic-q-a/20872/how-to-load-hex-to-nrf51-with-openocd

eclipse debug setup:
http://embeddedsoftdev.blogspot.com/p/ehal-nrf51.html
