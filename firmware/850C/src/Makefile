#  Copyright (C) 2015 Joerg Hoener
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.


# uncomment next lines to build for bootloader
# USE_WITH_BOOTLOADER = true
ifdef USE_WITH_BOOTLOADER
CFLAGS = -DUSE_WITH_BOOTLOADER
LFLAGS = -Xlinker --defsym=USE_WITH_BOOTLOADER=1
endif

TCPREFIX  = arm-none-eabi-
CC      = $(TCPREFIX)gcc
AS      = $(TCPREFIX)as 
LD      = $(TCPREFIX)gcc -v # use GCC and not LD so the math functions work like atan2()
CP      = $(TCPREFIX)objcopy
OD      = $(TCPREFIX)objdump
GDB     = $(TCPREFIX)gdb
SIZE     = $(TCPREFIX)size

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = Turn off optimization. Reduce compilation time and make debugging
#     produce the expected results.
# 1 = The compiler tries to reduce code size and execution time, without
#     performing any optimizations that take a great deal of compilation time.
# 2 = GCC performs nearly all supported optimizations that do not involve a 
#     space-speed tradeoff. As compared to -O1, this option increases
#     both compilation time and the performance of the generated code.
# 3 = Optimize yet more. Turns on -finline-functions and more.
# s = -Os enables all -O2 optimizations that do not typically increase code
#     size.
# (See gcc manual for further information)
OPT = 0

# -mfix-cortex-m3-ldrd should be enabled by default for Cortex M3.
# CFLAGS -H show header files

AFLAGS  = -I./GD32F10x_standard_peripheral/Include -I -Ispl/CMSIS -Ispl/inc -c -g -mcpu=cortex-m3 -mthumb -l libgcc
CFLAGS  += -I./GD32F10x_standard_peripheral/Include -I./ -I./spl/CMSIS -I./spl/CMSIS/inc -I./spl/inc -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -c -fno-common -O$(OPT) -g -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections -l libgcc 
# Need following option for LTO as LTO will treat retarget functions as
# unused without following option
CFLAGS+ = -fno-builtin
CFLAGS += -std=c99 --specs=nano.specs --specs=nosys.specs -Wall -Werror -I../../common/include

LFLAGS  += -Tstm32_flash.ld -L/usr/lib/gcc/arm-none-eabi/4.9.3/armv7-m -lgcc -lm -nostartfiles -lnosys -mcpu=cortex-m3 -mthumb -Wl,--gc-sections

LFLAGS += --specs=nano.specs --specs=nosys.specs # to use newlib nano
LFLAGS += -Xlinker -Map=main.map
#LFLAGS += -u _printf_float # newlib nano printf use floats
#LFLAGS += -u _scanf_float # newlib nano scanf use floats
CPFLAGS = -Obinary 
ODFLAGS = -S

include ../../common/Makefile.common

COMMONSRC = ../../common/src
SOURCES=$(shell find spl ugui_driver *.c -type f -iname '*.c') $(COMMONSRC)/fault.c $(COMMONSRC)/buttons.c $(COMMONSRC)/utils.c $(COMMONSRC)/ugui.c $(COMMONSRC)/fonts.c $(COMMONSRC)/state.c $(COMMONSRC)/screen.c $(COMMONSRC)/mainscreen.c $(COMMONSRC)/configscreen.c
OBJECTS=$(foreach x, $(basename $(SOURCES)), $(x).o)

# dev platform specific.
# OPENOCD_PATH := E:/nrf5/Toolchain/OpenOCD/0.10.0-12-20190422-2015/bin
# OPENOCD_BIN := openocd.exe
# NRFUTIL := $(SDK_ROOT)/external_tools/Windows/nrfutil.exe

OPENOCD_PATH := /usr/local/share/openocd/bin
OPENOCD_BIN := openocd
NRFUTIL := nrfutil

OPENOCD := '$(OPENOCD_PATH)/$(OPENOCD_BIN)' -f $(OPENOCD_PATH)/../scripts/interface/stlink.cfg -f $(OPENOCD_PATH)/../scripts/target/stm32f1x.cfg

all: main.bin size

clean: 
	rm -f main.lst main.elf main.bin
	rm $(COMMONSRC)/*.o
	find *.o | xargs rm
	find spl/src/*.o | xargs rm
	find spl/CMSIS/*.o | xargs rm

flash_serial: main.bin 
	$(STM32FLASH) main.bin

# before using this unlock cpu with
# > stm32f1x unlock 0
# stm32f1x unlocked.
# INFO: a reset or power cycle is required for the new settings to take effect.
flash_jtag: main.bin
	$(OPENOCD) -c "init; reset halt; sleep 500; stm32f1x mass_erase 0; flash write_bank 0 main.bin 0; shutdown"

run_jtag: flash_jtag
	$(OPENOCD) -c "init; reset halt; reset run; shutdown"

# Just start an openocd session
openocd:
	$(OPENOCD) 

size:
	@echo "Size:"
	$(SIZE) main.elf

main.bin: main.elf
	@echo "...copying"
	$(CP) $(CPFLAGS) main.elf main.bin
	$(OD) $(ODFLAGS) main.elf > main.lst

main.elf: $(OBJECTS) startup_stm32f10x_md.o
	@echo "..linking"
	$(LD)  $^ $(LFLAGS) -o $@

%.o: %.c
	@echo ".compiling"
	$(CC) $(CFLAGS) $< -o $@

startup_stm32f10x_md.o:
	$(AS) $(ASFLAGS) startup_stm32f10x_md.s -o startup_stm32f10x_md.o
