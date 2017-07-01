###############################################################
#####
##### Makefile for asixtool
#####
###############################################################

###############################################################
#####
##### PATHS (default installation)
#####
###############################################################

INSTALLDIR_BIN=/usr/local/bin

-include Makefile.local

###############################################################
#####
##### Do the whole shabang
#####
###############################################################

all: asixtool

asixtool: asix_serial.c asix_serial.h bootcom.c bootcom.h infohelper.h infohelper.c main.c 
	gcc -Os -Wall asix_serial.c bootcom.c infohelper.c main.c -o asixtool
	strip asixtool


clean:
	$(RM) *.o *~ asixtool

### EOF
