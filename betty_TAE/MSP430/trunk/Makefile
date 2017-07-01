# makfile configuration
NAME            = TAE_MSP
CSOURCES        = cc1100.c  hardware.c  main.c  rf.c
#~ ASOURCES        = 
CPU             = msp430f1121
CPU_CLOCK		= 4500000
COMPORT			= /dev/ttyUSB2
GDBPORT			= 3333
BSLOPT		= --invert-reset --comport=${COMPORT}

ASFLAGS         = -mmcu=${CPU} -D_GNU_ASSEMBLER_ -I .
CFLAGS          = -mmcu=${CPU} -Os -Wall -g --std=gnu99 -I . -D F_CPU=${CPU_CLOCK} -D HAS_CONFIG_H
#LDFLAGS         =  -lmspgcc

#switch the compiler (for the internal make rules)
CC              = msp430-gcc
AS              = msp430-gcc

OBJECTS         = ${CSOURCES:.c=.o} ${ASOURCES:.S=.o}


.PHONY: all FORCE clean download download-jtag download-bsl

#all should be the first target. it's built when make is run without args
all: ${NAME}.elf ${NAME}.a43 ${NAME}.lst dependencies.d

#confgigure the next line if you want to use the serial download
program: download-jtag
#~ download: download-bsl

#additional rules for files
${NAME}.elf: ${OBJECTS}
	${CC} -mmcu=${CPU} -o $@ ${OBJECTS} $(LDFLAGS)

${NAME}.a43: ${NAME}.elf
	msp430-objcopy -O ihex $^ $@

${NAME}.lst: ${NAME}.elf
#	non uniarch msp430-objdump -dSt --size-sort $^ >$@
	msp430-objdump -dSt  $^ >$@
	@echo "----- RAM/Flash Usage -----"
	msp430-size $^
	msp430-nm --size-sort $^

download-jtag: all
#	mspdebug  rf2500 "prog ${NAME}.elf"
	mspdebug -d ${COMPORT} uif -j "prog ${NAME}.elf"
#	msp430-jtag -e ${NAME}.elf

download-bsl: all
	msp430-bsl $(BSLOPT) -e ${NAME}.elf
	
debug: all
#	mspdebug rf2500 "prog ${NAME}.elf" "setbreak main" run "gdb ${GDBPORT}"
#	mspdebug -d ${COMPORT} uif -j "prog ${NAME}.elf" reset "gdb ${GDBPORT}"
	mspdebug -d ${COMPORT} uif -j "prog ${NAME}.elf" "setbreak main" run "delbreak 0" "gdb ${GDBPORT}"

clean:
	rm -f ${NAME}.elf ${NAME}.a43 ${NAME}.lst ${OBJECTS} dependencies.d

#dummy target as dependecy if something has to be build everytime
FORCE:

#project dependencies
dependencies.d:
	$(CC) -MM ${CFLAGS} ${CSOURCES} > dependencies.d
ifdef ASOURCES
	$(CC) -MM ${ASFLAGS} ${ASOURCES} >> dependencies.d
endif

-include dependencies.d

