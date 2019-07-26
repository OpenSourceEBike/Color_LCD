# TODO sequence

* merge 850C style rx comms code with the existing SW102 code
* merge the 850C style tx comms code with the existing SW102 code
* make the rtc stubs do something
* turn on eeprom writing
* add a logical buttons abstraction and callbacks for screen button handling
* make accel button presses work
* make all other button presses (excluding config) work
* test with real motor (after triple checking wiring color questions)
* make config entry presses work
* make config system (hopefully usable on other platforms)
* turn on system power / turn off system power per button press
* delete dead code in mainscreen.c
* initial release
* merge with 850C code somewhat? (sharing behavior - just different UX layer)

# Misc notes from kevin not yet formatted

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
