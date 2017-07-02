###############################################################
#####
##### Makefile for boop - OpenSource firmware for Betty
##### Created at 30.8.2007 02:26 am 
#####
##### boop V0.1 by netguy - ck@mamalala.net
##### Makefile V0.1 by alterego - alteregon@gmx.net
##### Makefile v0.2 by Tobias - tobias-betty@23.gs
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


#ARMBASE = F:/Tools/GNU_Tools_ARM_Embedded/5.4
#INCLUDEPATH = $(ARMBASE)/arm-none-eabi/include
#LIBPATH = $(ARMBASE)/arm-none-eabi/lib
#ARMPATH = $(ARMBASE)/bin/
TOOLPREFIX = arm-none-eabi-
LPCTOOL = lpctool
OPENOCDPATH = F:\Tools\OpenOCD
OPENOCD = $(OPENOCDPATH)\openocd.exe -f betty.cfg -f $(OPENOCDPATH)\interface\uniprog.cfg 

###############################################################
#####
##### Compiler, Linker and Tools
#####
###############################################################

CC = $(ARMPATH)$(TOOLPREFIX)gcc
AS = $(ARMPATH)$(TOOLPREFIX)gcc
LD = $(ARMPATH)$(TOOLPREFIX)gcc
OC = $(ARMPATH)$(TOOLPREFIX)objcopy
OD = $(ARMPATH)$(TOOLPREFIX)objdump
SIZE = $(ARMPATH)$(TOOLPREFIX)size

CPUFLAGS = -mcpu=arm7tdmi-s
OPTFLAGS = -Os
CFLAGS = -Wall -mthumb-interwork 
INC = -I. -Iinterrupt -Idisplay -Ikeyboard -Iaudio -Iinfrared -Iserial -Iflash -Icc1100 -Igui -Itimer -Igames -Iadc -Irtc  -Itools
ASFLAGS = -g -I. -mthumb-interwork 
#LDFLAGS = -Tlpc2220_rom.ld -Map boop.map
#LIBS = -lc -lgcc
LDFLAGS = -mthumb-interwork -nostartfiles -Xlinker -Map -Xlinker boop.map -Tlpc2220_rom.ld
THUMBFLAGS = -mthumb

COMPILE = $(CC) $(CPUFLAGS) $(CFLAGS) $(INC)

ifeq ($(MAKECMDGOALS),debug)
	COMPILE += -D DEBUGMODE
endif

ifeq ($(MAKECMDGOALS),release)
	COMPILE += -D RELEASE
endif

-include Makefile.local

###############################################################
#####
##### Do the boop
#####
###############################################################

# Recursive expansion of Makefile rules.
define expand_dir
 # Reset vars for subdir for the case that Make.conf does not exist
 SUBDIRS :=
 SRCS :=
 THUMBSRCS :=
 THUMBSRCSUNOPT :=
 -include $(1)Make.conf
 ALLSRCS += $$(SRCS:%=$(1)%)
 ALLTHUMBSRCS += $$(THUMBSRCS:%=$(1)%)
 ALLTHUMBSRCSUNOPT += $$(THUMBSRCSUNOPT:%=$(1)%)
 DEPS += $(1).deps
 $$(foreach adir,$$(SUBDIRS),$$(eval $$(call expand_dir,$(1)$$(adir)/)))
endef

ALLSRCS :=
ALLTHUMBSRCS :=
ALLTHUMBSRCSUNOPT :=

$(eval $(call expand_dir,))

OBJS := $(patsubst %.s,%.o,$(ALLSRCS:.c=.o)) $(ALLTHUMBSRCS:.c=.thumb.o) $(ALLTHUMBSRCSUNOPT:.c=.thumbunopt.o)

all:	$(DEPS) boop_rom.bin boop_rom.hex

debug:	$(DEPS) boop_rom.bin boop_rom.hex

release: clean version $(DEPS) boop_rom.bin boop_rom.hex
	@echo -n '\n\nRelease erstellt SVN Version ++'
	@cat .svn/entries | sed -n '4p'
	
version: 
	echo -n '#define SVNVERSION ' > version.h
	svnversion -n |cut -f1 -d M -n >> version.h
	echo >> version.h

version.h:
	echo -n '#define SVNVERSION ' > version.h
	svnversion -n|cut -f1 -d M >> version.h
	echo >> version.h
	
test: boop_rom.elf
	$(OD) -h $<

%.bin: %.elf
	$(OC) -O binary $< $@

%.hex: %.elf
	$(OC) -O ihex $< $@

boop_rom.elf: $(OBJS)
	$(LD) $(LDFLAGS) -L$(LIBGCCPATH) -L$(LIBCPATH) -o $@ $^ $(LIBS)
	arm-none-eabi-size $@

%.o: %.s
	$(CC) $(ASFLAGS) -c -o $@ $<

%.o: %.c
	$(COMPILE) $(OPTFLAGS) -c -MMD -MF $(dir $<).deps/$(notdir $@) -o $@ $<

%.thumb.o: %.c
	$(COMPILE) $(THUMBFLAGS) $(OPTFLAGS) -c -MMD -MF $(dir $<).deps/$(notdir $@) -o $@ $<
	
%.thumbunopt.o: %.c
	$(COMPILE) $(THUMBFLAGS) -c -MMD -MF $(dir $<).deps/$(notdir $@) -o $@ $<

$(DEPS):
	mkdir -p $@	

uresident: resident
resident: boop_rom.bin
	$(LPCTOOL) -i -v -e -a $<

program: boop_rom.bin 
	$(OPENOCD) -c init -c 'flash_boop $<' -c shutdown

clean:
	-rm -Rf $(DEPS)
	-rm -f $(OBJS) *.elf *.bin *.hex *.map *~

clean2:
	-rm -Rf $(DEPS)
	-rm -f $(OBJS) *.elf *.hex *.map *~
	
-include $(DEPS:=/*)
