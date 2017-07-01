/*
    global.h
    Copyright (C) 2007  Colibri <colibri_dvb@lycos.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GLOBAL_H
#define GLOBAL_H

//Uncomment if you want compile with demo sound
//#define WITH_SOUND

// Set appropriate feature flags
#define FF_UART_REDIRECT      1
#define FF_LARGE_IDLE_CLOCK   0
#define FF_LARGE_MENU         1
#define FF_ICON_TRANSPARENT   1   // enable 3 BitPerPixel icons support

typedef unsigned char BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#define false	0
#define true	1

#ifndef NULL
#define NULL	0
#endif

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

#ifndef _BV
#define _BV(bit)   (1 << (bit))
#endif

#define NUM_OF_ELEMENTS(x)   sizeof(x)/sizeof(x[0])

#define	SPEED_30	0
#define	SPEED_60	1

// Execute code in locked sequence
#define INT_LOCK(x)     {unsigned cpsr = disableIRQ (); {x;} restoreIRQ (cpsr);}

/** Let the LPC2200 sleep until the next IRQ
*
* This function tries to enter the sleep mode and stays
* there until the next interrupt wakes up the CPU.
* If that is not possible, the function returns immediately.
*/
extern void cpu_idle (void);

#define SYS_NOIDLE	0x08	// No Idle Mode
#define SYS_NOPDOWN 0x10   	// No Power Down
#define SYS_IR		0x20
#define SYS_KEY		0x40
#define SYS_TURBO	0x80

#include <stdio.h>
#define printd(...)		printf(__VA_ARGS__)
#define debugbuffersize		50
extern unsigned char debugoutbuf [debugbuffersize];

#ifndef __MAIN_C__
extern char sysInfo;
#endif /* __MAIN_C__ */

extern unsigned char debugoutbuf [];

#define PCON_IDLE	0x01
#define PCON_PDOWN	0x02

typedef unsigned short crc;

void *memcpy(void *dest,void *src,int count);
int memcmp(const void * cs,const void * ct,int count);
unsigned char GetHWversion(void);
int	puts(const char* string);
crc CRCs(char* message);

#endif
