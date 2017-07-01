
include ../Makefile.inc


all: tool_data.h

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
	mv tool_data.h ..

clean:
	$(RM) *.o ramtool.elf ramtool.bin romtool.elf romtool.bin *~ bin2h

### EOF
