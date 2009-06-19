# ***************************************************************
# *   Makefile for Linux ARM Board bootloader			        *
# *                                                             *
# *                                                             *
# *   Kuba Odias, 07.06.2009	                                *
# ***************************************************************

NAME=lab_bootloader
VERSION=1.0
FULL_NAME=$(NAME)-$(VERSION)

OUT_FILE=$(FULL_NAME).out
BIN_FILE=$(FULL_NAME).bin
DMP_FILE=$(FULL_NAME).dmp

CROSS=/opt/gnuarm-4.0.2/bin/arm-elf-

CC=$(CROSS)gcc
LD=$(CROSS)ld -v
AR=$(CROSS)ar
AS=$(CROSS)as
CP=$(CROSS)objcopy
OD=$(CROSS)objdump
SZ=$(CROSS)size
NM=$(CROSS)nm

CFLAGS  = -Iinclude -c -fno-common -Os -g -march=armv4t -mtune=arm920t
#AFLAGS  = -ahls -mapcs-32 -Iinclude
AFLAGS  = -c -Iinclude
LFLAGS  = -T elf32-littlearm.lds -Ttext 0
CPFLAGS = --output-target=binary
ODFLAGS	= -x -D --syms

OBJECTS = cstartup_ram.o asm_isr.o at45.o com.o dataflash.o init.o main.o sdram.o stdio.o _udivsi3.o _umodsi3.o div0.o
         
all: $(OUT_FILE)
	@ echo "...copying"
	$(CP) $(CPFLAGS) $(OUT_FILE) $(BIN_FILE)
	$(OD) $(ODFLAGS) $(OUT_FILE) > $(DMP_FILE)
	$(NM) $(OUT_FILE) | sort > System.map

$(OUT_FILE): $(OBJECTS) elf32-littlearm.lds 
	@ echo "..linking"
	$(LD) $(LFLAGS) -o $(OUT_FILE) $(OBJECTS)

cstartup_ram.o: cstartup_ram.S
	@ echo ".assembling"
	$(CC) $(AFLAGS) cstartup_ram.S > cstartup_ram.lst

asm_isr.o: asm_isr.S
	@ echo ".assembling"
	$(CC) $(AFLAGS) asm_isr.S > asm_isr.lst

_udivsi3.o: _udivsi3.S
	@ echo ".assembling"
	$(CC) $(AFLAGS) _udivsi3.S > _udivsi3.lst

_umodsi3.o: _umodsi3.S
	@ echo ".assembling"
	$(CC) $(AFLAGS) _umodsi3.S > _umodsi3.lst

main.o: main.cpp include/main.h
	@ echo ".compiling"
	$(CC) $(CFLAGS) main.cpp
	
at45.o: at45.cpp
	@ echo ".compiling"
	$(CC) $(CFLAGS) at45.cpp
	
sdram.o: sdram.cpp include/sdram.h
	@ echo ".compiling"
	$(CC) $(CFLAGS) sdram.cpp

com.o: com.cpp include/com.h
	@ echo ".compiling"
	$(CC) $(CFLAGS) com.cpp
	
div0.o: div0.c
	@ echo ".compiling"
	$(CC) $(CFLAGS) div0.c
	
dataflash.o: dataflash.cpp include/dataflash.h
	@ echo ".compiling"
	$(CC) $(CFLAGS) dataflash.cpp

init.o: init.cpp
	@ echo ".compiling"
	$(CC) $(CFLAGS) init.cpp

stdio.o: stdio.cpp include/stdio.h
	@ echo ".compiling"
	$(CC) $(CFLAGS) stdio.cpp

clean:
	rm $(OBJECTS) cstartup_ram.lst asm_isr.lst _udivsi3.lst _umodsi3.lst $(OUT_FILE) $(BIN_FILE) $(DMP_FILE) System.map 2>/dev/null
