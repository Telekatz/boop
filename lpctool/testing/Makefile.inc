
###############################################################
#####
##### PATHS (default installation)
#####
###############################################################

ARMBASE=/opt/armtool/4.1.1
INCLUDEPATH=$(ARMBASE)/include
LIBPATH=$(ARMBASE)/arm-elf/lib/interwork
ARMPATH=$(ARMBASE)/bin
TOOLPREFIX=/arm-elf-

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
