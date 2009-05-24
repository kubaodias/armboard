# ***************************************************************
# *   Makefile for Atmel Romboot			        *
# *                                                             *
# *                                                             *
# *   Kuba Odias, 24.05.2009	                                *
# ***************************************************************

NAME = romboot

CROSS=/opt/gnuarm-4.0.2/bin/arm-elf-
#CROSS=/opt/cross/bin/arm-linux-
#GCC="$CROSS"gcc
CC=$(CROSS)gcc
LD=$(CROSS)ld -v
AR=$(CROSS)ar
AS=$(CROSS)as
CP=$(CROSS)objcopy
OD=$(CROSS)objdump
SZ=$(CROSS)size

CFLAGS  = -Iinclude -c -fno-common -O0 -g
#AFLAGS  = -ahls -mapcs-32 -Iinclude
AFLAGS  = -c -Iinclude
LFLAGS  = -T elf32-littlearm.lds -Ttext 0
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

OBJECTS = cstartup_ram.o asm_isr.o jump.o at45.o com.o dataflash.o init.o main.o stdio.o _udivsi3.o _umodsi3.o div0.o
         
#make target called by Eclipse  (Project -> Build Project)
all:  romboot.out
	@ echo "...copying"
	$(CP) $(CPFLAGS) romboot.out romboot.bin
	$(OD) $(ODFLAGS) romboot.out > romboot.dmp

romboot.out: $(OBJECTS) elf32-littlearm.lds 
	@ echo "..linking"
	$(LD) $(LFLAGS) -o romboot.out $(OBJECTS)

cstartup_ram.o: cstartup_ram.S
	@ echo ".assembling"
	$(CC) $(AFLAGS) cstartup_ram.S > cstartup_ram.lst

asm_isr.o: asm_isr.S
	@ echo ".assembling"
	$(CC) $(AFLAGS) asm_isr.S > asm_isr.lst

jump.o: jump.S
	@ echo ".assembling"
	$(CC) $(AFLAGS) jump.S > jump.lst

_udivsi3.o: _udivsi3.S
	@ echo ".assembling"
	$(CC) $(AFLAGS) _udivsi3.S > _udivsi3.lst

_umodsi3.o: _umodsi3.S
	@ echo ".assembling"
	$(CC) $(AFLAGS) _umodsi3.S > _umodsi3.lst

main.o: main.cpp main.h
	@ echo ".compiling"
	$(CC) $(CFLAGS) main.cpp
	
at45.o: at45.cpp
	@ echo ".compiling"
	$(CC) $(CFLAGS) at45.cpp
	
com.o: com.cpp
	@ echo ".compiling"
	$(CC) $(CFLAGS) com.cpp
	
div0.o: div0.c
	@ echo ".compiling"
	$(CC) $(CFLAGS) div0.c
	
dataflash.o: dataflash.cpp
	@ echo ".compiling"
	$(CC) $(CFLAGS) dataflash.cpp

init.o: init.cpp
	@ echo ".compiling"
	$(CC) $(CFLAGS) init.cpp

stdio.o: stdio.cpp
	@ echo ".compiling"
	$(CC) $(CFLAGS) stdio.cpp

# make target called by Eclipse (Project -> Clean ...)
clean:
	rm $(OBJECTS) cstartup_ram.lst asm_isr.lst jump.lst _udivsi3.lst _umodsi3.lst romboot.out romboot.bin romboot.dmp 2>/dev/null

# **********************************************************************************************
#                            FLASH PROGRAMMING                                         
#
# Alternate make target for flash programming only
#
# You must create a special Eclipse make target (program) to run this part of the makefile 
# (Project -> Create Make Target...  then set the Target Name and Make Target to "program")
#
# OpenOCD is run in "batch" mode with a special configuration file and a script file containing
# the flash commands. When flash programming completes, OpenOCD terminates.
#
# Note that the script file of flash commands (script.ocd) is part of the project
#
# Programmers: Martin Thomas, Joseph M Dupre, James P Lynch
# **********************************************************************************************

# specify output filename here (must be *.bin file)
TARGET = main.bin

# specify the directory where openocd configuration files reside
OPENOCD_DIR = '/mnt/work/Dyplom/software/openocd/'

# OpenOCD executable file
OPENOCD = openocd

# specify OpenOCD configuration files
OPENOCD_CFG = $(OPENOCD_DIR)openocd.cfg
OPENOCD_FLASH_CFG = $(OPENOCD_DIR)openocd_flash.cfg
OPENOCD_INTERFACE_CFG = $(OPENOCD_DIR)interface/turtelizer2.cfg
OPENOCD_TARGET_CFG = $(OPENOCD_DIR)target/at91sam7s64.cfg
OPENOCD_TARGET_FLASH_CFG = $(OPENOCD_DIR)target/at91sam7s64_flash.cfg

# program the AT91SAM7S256 internal flash memory
program: $(TARGET)
	@echo "Flash Programming with OpenOCD..."			# display a message on the console
	# program the onchip FLASH here
	$(OPENOCD) -f $(OPENOCD_INTERFACE_CFG) -f $(OPENOCD_TARGET_FLASH_CFG) -f $(OPENOCD_FLASH_CFG)
	@echo "Flash Programming Finished."					# display a message on the console

