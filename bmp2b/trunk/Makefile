# native builds
CC = gcc
CFLAGS = -Wall -I/usr/src/linux/include

# cross build
ARCH = arm-elf
CROSS_CC = $(ARCH)-gcc
CROSS_AS = $(ARCH)-as
CROSS_LD = $(ARCH)-ld
CROSS_OBJCOPY = $(ARCH)-objcopy
CROSS_OPTS = -mcpu=arm7tdmi-s
CROSS_CFLAGS = $(CROSS_OPTS) -Wall -Os
CROSS_ASLAGS = $(CROSS_OPTS) --gstabs
CROSS_RAM_LDFLAGS = -Tlpc2220_ram.ld -nostartfiles -nostdlib 
CROSS_ROM_LDFLAGS = -Tlpc2220_rom.ld -nostartfiles -nostdlib

# build objects
HOST_TARGET = lpcload fwdump font2b
CROSS_TARGET = fwbc.hex fwflash.hex betty.hex

# betty deps
BETTY_DEPS = system.o uart.o buttons.o spi.o display.o flash.o functions.o
BETTY_DEPS += interrupts.o pwm.o #pffs.o

# all projects
all: $(HOST_TARGET) $(CROSS_TARGET)

# bmp2b / font2b
links:

bmp.c: links

bmp.o: bmp.c
	$(CC) -c $(CFLAGS) -o $@ $<

bmp2b: bmp.o

# arm code
arm: arm_clean $(CROSS_TARGET)

# .o out of .c
%.o: %.c
	$(CROSS_CC) -c $(CROSS_CFLAGS) -o $@ $<

# .o out of .s
%.o: %.s
	$(CROSS_AS) $(CROSS_ASLAGS) -o $@ $<

# .elf out of .o
%.elf: %.o startup.o interrupts.o
	$(CROSS_LD) $(CROSS_RAM_LDFLAGS) startup.o interrupts.o -o $@ $<

# betty is special ;)
betty.elf: betty.o startup.o $(BETTY_DEPS)
	#$(CROSS_LD) $(CROSS_ROM_LDFLAGS) startup.o $(BETTY_DEPS) -o $@ $<
	$(CROSS_LD) $(CROSS_RAM_LDFLAGS) startup.o $(BETTY_DEPS) -o $@ $<

# .hex out of .elf
%.hex: %.elf
	$(CROSS_OBJCOPY) -O ihex $< $@

# host clean
clean:
	rm -vf lpcload fwdump

# arm clean
arm_clean:
	rm -vf *.o *.hex *.elf

