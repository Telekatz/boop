###############################################################
#####
##### Makefile for betty_scart
##### Created at 30.8.2007 02:26 am 
#####
##### scart_terminal V0.1 by telekatz - telekatz@gmx.de
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



SDCCBASE=F:/Tools/SDCC
INCLUDEPATH=$(SDCCBASE)/include
SDCCPATH=$(SDCCBASE)/bin

PACKER   = $(SDCCPATH)/packihx
IMGMAKER = $(SDCCPATH)/makebin
CC=$(SDCCPATH)/sdcc

-include Makefile.local

# Target file name.
TARGET = betty_scart


# List C source files here.
SRC = 	main.c \
		serial.c \
		cc1100.c \
		cmd.c \
		rf.c \
		timer.c


# Compiler flags.
FLAGS = -mmcs51 --iram-size 0x100
FLAGS += --model-small
FLAGS += --code-size 8096
FLAGS += -I$(INCLUDEPATH)
FLAGS += --std-sdcc99
# For newer SDCC
FLAGS += -Dcode=__code -Didata=__idata -Dbit=__bit

# Define all object files.
OBJ = $(SRC:.c=.rel)


# Default target.
all: $(TARGET).bin

program: $(TARGET).bin
	#ScartProg -a -u -d 'UniProg A'
	ScartProg -a 

main.rel: main.c
	$(CC) $(FLAGS) -c main.c
	
serial.rel: serial.c serial.h
	$(CC) $(FLAGS) -c serial.c
	
cc1100.rel: cc1100.c cc1100.h
	$(CC) $(FLAGS) -c cc1100.c
	
cmd.rel: cmd.c cmd.h
	$(CC) $(FLAGS) -c cmd.c

rf.rel: rf.c rf.h
	$(CC) $(FLAGS) -c rf.c
	
timer.rel: timer.c timer.h
	$(CC) $(FLAGS) -c timer.c

$(TARGET).bin: $(TARGET).hex
	cat $(TARGET).hex | $(IMGMAKER) -s 8096 > $(TARGET).bin


$(TARGET).hex: $(TARGET).ihx
	$(PACKER) $(TARGET).ihx > $(TARGET).hex

$(TARGET).ihx: $(OBJ)
	@echo 
	@echo Linking $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(TARGET).ihx
	
clean:
	$(RM) -v *.rel *.rst *.sym *.lst *.hex *.asm *.mem *.map *.lnk *.ihx *.bin *~
