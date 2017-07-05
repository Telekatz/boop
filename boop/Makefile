###############################################################
#####
##### Makefile for boop - communicate with betty
##### Created at 30.8.2007 02:26 am 
#####
##### boop V0.1 by netguy - ck@mamalala.net
##### Makefile V0.1 by alterego - alteregon@gmx.net
#####
###############################################################

###############################################################
#####
##### PATHS (default installation)
#####
##### You can put your path-config into Makefile.local 
##### to override these defaults
#####
###############################################################


ARMBASE=/opt/toolchains/gcc-arm-none-eabi-4_9-2014q4
INCLUDEPATH=$(ARMBASE)/include
ARMPATH=$(ARMBASE)/bin
TOOLPREFIX=/arm-none-eabi-
BOOTLOADER_DEVICE=/dev/ttyUSB0

###############################################################
#####
##### Compiler, Linker and Tools
#####
###############################################################

CC=$(ARMPATH)$(TOOLPREFIX)gcc
AS=$(ARMPATH)$(TOOLPREFIX)as
#LD=$(ARMPATH)$(TOOLPREFIX)ld
LD=$(CC)
OC=$(ARMPATH)$(TOOLPREFIX)objcopy
OD=$(ARMPATH)$(TOOLPREFIX)objdump

CPUFLAGS=-mcpu=arm7tdmi-s
OPTFLAGS=-Os
CFLAGS=$(CPUFLAGS) -c -Wall -mthumb-interwork -msoft-float -I$(INCLUDEPATH) -ggdb
ASFLAGS=$(CPUFLAGS) -D --gstabs -mthumb-interwork -mfpu=softfpa
LDFLAGS = -mthumb-interwork -nostartfiles -Xlinker -Map -Xlinker boop.map -Tlpc2220.ld

THUMBFLAGS=-mthumb

-include Makefile.local

###############################################################
#####
##### Do the boop
#####
###############################################################

all: boop_rom.bin

boop_rom.bin: boop_ram.elf
	$(OC) -O binary boop_ram.elf boop_ram.bin
	$(OC) -O ihex boop_ram.elf boop_ram.hex

boop_ram.elf: crt.o lcd.o fonty.o main.o irq.o keyboard.o soundcore.o
	$(LD) -o  boop_ram.elf crt.o lcd.o fonty.o main.o irq.o keyboard.o soundcore.o  $(LDFLAGS)

test: boop_ram.elf
	$(OD) -h boop_ram.elf

crt.o: crt.s
	$(AS) $(ASFLAGS) -o crt.o crt.s

irq.o: irq.c irq.h
	$(CC) $(CFLAGS) $(OPTFLAGS) -o irq.o irq.c

keyboard.o: keyboard.c keyboard.h
	$(CC) $(CFLAGS) -o keyboard.o keyboard.c

soundcore.o: soundcore.c soundcore.h
	$(CC) $(CFLAGS) -o soundcore.o soundcore.c

lcd.o: lcd.c lcd.h
	$(CC) $(CFLAGS) $(OPTFLAGS) $(THUMBFLAGS) -o lcd.o lcd.c

fonty.o: fonty.c fonty.h
	$(CC) $(CFLAGS) $(OPTFLAGS) $(THUMBFLAGS) -o fonty.o fonty.c

main.o: main.c
	$(CC) $(CFLAGS) $(OPTFLAGS) -o main.o main.c

ramload: boop_ram.bin
	lpctool -d $(BOOTLOADER_DEVICE) -r boop_ram.bin

resident: boop_ram.bin
	lpctool -d $(BOOTLOADER_DEVICE) -i -v -e -a boop_ram.bin

clean:
	$(RM) -v *.o *.elf *.bin *.hex *~

### EOF
