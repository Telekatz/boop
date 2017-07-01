###############################################################
#####
##### Makefile for ramtool and lpctool
#####
###############################################################

###############################################################
#####
##### PATHS (default installation)
#####
###############################################################

ARMBASE=/opt/summon-arm-toolchain
INCLUDEPATH=$(ARMBASE)/include
LIBPATH=$(ARMBASE)/arm-elf/lib/interwork
ARMPATH=$(ARMBASE)/bin
TOOLPREFIX=/arm-none-eabi-

INSTALLDIR_BIN=/usr/local/bin

-include Makefile.local

###############################################################
#####
##### Compiler, Linker and Tools
#####
###############################################################

CC=$(ARMPATH)$(TOOLPREFIX)gcc
AS=$(ARMPATH)$(TOOLPREFIX)as
LD=$(ARMPATH)$(TOOLPREFIX)ld
OC=$(ARMPATH)$(TOOLPREFIX)objcopy
OD=$(ARMPATH)$(TOOLPREFIX)objdump

CPUFLAGS=-mcpu=arm7tdmi-s
OPTFLAGS=-Os
#OPTFLAGS=-O0
#OPTFLAGS=
CFLAGS=$(CPUFLAGS) -c -Wall -mthumb-interwork -msoft-float -I$(INCLUDEPATH)
ASFLAGS=$(CPUFLAGS) -D --gstabs -mthumb-interwork -mfpu=softfpa
LDFLAGS=$(OPTFLAGS) -Tlpc2220.ld
ROMLDFLAGS=$(OPTFLAGS) -Tlpc2220_rom.ld

###############################################################
#####
##### Do the whole shabang
#####
###############################################################

all: ramtool.bin romtool.bin lpctool

ramtool.bin: ramtool.elf
	$(OC) -O binary ramtool.elf ramtool.bin

ramtool.elf: crt.o serial.o lcd.o fonty.o flash.o ramtool.o  lpc2220.ld
	$(LD) $(LDFLAGS) -o ramtool.elf crt.o serial.o lcd.o fonty.o flash.o ramtool.o

romtool.bin: romtool.elf
	$(OC) -O binary romtool.elf romtool.bin

romtool.elf: crt.o serial.o lcd.o fonty.o flash.o romtool.o  lpc2220_rom.ld
	$(LD) $(ROMLDFLAGS) -o romtool.elf crt.o serial.o lcd.o fonty.o flash.o romtool.o

crt.o: crt.s
	$(AS) $(ASFLAGS) -o crt.o crt.s

flash.o: flash.c flash.h serial.h lcd.h
	$(CC) $(CFLAGS) -o flash.o flash.c

lcd.o: lcd.c lcd.h lpc2220.h
	$(CC) $(CFLAGS) -o lcd.o lcd.c

fonty.o: fonty.c fonty.h charset.bits charset.info lcd.h
	$(CC) $(CFLAGS) -o fonty.o fonty.c

serial.o: serial.c serial.h lcd.h lpc2220.h
	$(CC) $(CFLAGS) -o serial.o serial.c

ramtool.o: ramtool.c serial.o lcd.h flash.h lpc2220.h
	$(CC) $(CFLAGS) -o ramtool.o ramtool.c

romtool.o: ramtool.c serial.o lcd.h flash.h lpc2220.h
	$(CC) $(CFLAGS) -DRESIDENT -o romtool.o ramtool.c

bin2h: bin2h.c ramtool.bin
	gcc -Os bin2h.c -o bin2h
	strip bin2h

tool_data.h: bin2h
	./bin2h > tool_data.h

lpctool: infohelper.c lpctool_serial.c bootcom.c toolcom.c lpctool.c infohelper.h lpctool_serial.h bootcom.h toolcom.h tool_data.h
	gcc -Os infohelper.c lpctool_serial.c bootcom.c toolcom.c lpctool.c -o lpctool
	strip lpctool

upload: lpctool
	./lpctool -i -v

resident: romtool.bin lpctool
	./lpctool -S -i -a romtool.bin -v

uresident: romtool.bin lpctool
	./lpctool -d /dev/ttyUSB0 -S -i -a romtool.bin -v

install: lpctool
	cp lpctool $(INSTALLDIR_BIN)

uninstall:
	rm $(INSTALLDIR_BIN)/lpctool

clean:
	$(RM) *.o ramtool.elf ramtool.bin romtool.elf romtool.bin *~ lpctool tool_data.h bin2h

### EOF
