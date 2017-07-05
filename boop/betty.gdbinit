monitor reset 100
monitor halt
monitor endian little

# BCFG0: 16bit, rble, 6wst - 60 mhz : Betty: FLASH 0 @ 0x80000000
monitor memU32 0xFFE00000 = 0x100004A0

#BCFG2: 16bit, rble, 6wst - 60 mhz : Betty: FLASH 1 @ 0x82000000
monitor memU32 0xFFE00008 = 0x100004A0

#PINSEL0
monitor memU32 0xE002C000 = 0x00008005

#PINSEL1
monitor memU32 0xE002C004 = 0x00000000

#PINSEL2
monitor memU32 0xE002C014 = 0x0de049d4

#IO2SET
monitor memU32 0xE0028024 = 0x1FC0000

#IO2DIR
monitor memU32 0xe0028028 = 0x1FC0000

#IO0DIR
monitor memU32 0xE0028008 = 0x002018D0

#PLLCON: Enable PLL, connect PLL
monitor memU32 0xe01fc080 = 0xe01fc000

# VBPDIV:
monitor memU32 0xe01fc100 = 0x00000000

#PLLCFG: 
monitor memU32 0xe01fc084 = 0x00000045

#Activate PLL settings
monitor memU32 0xe01fc08c = 0xaa
monitor memU32 0xe01fc08c = 0x55


# Enable Flash Breakpoints and Download
monitor WorkRAM = 0x40000000 - 0x2000FBFF
monitor flash CFI = 0x80000000 - 0x800FFFFF
monitor flash breakpoints = 1
monitor flash download = 1