###############################################################
#####
##### Makefile for ramtool and lpctool
#####
###############################################################

include Makefile.inc

all: lpctool

tool_data.h: force_look
	cd flashtool; $(MAKE) $(MFLAGS)

lpctool: tool_data.h
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
	$(RM) *.o *~ lpctool tool_data.h
	cd flashtool; make clean

force_look :
	true


### EOF
