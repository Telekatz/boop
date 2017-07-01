/*
    keyboard.c - button handling
    Copyright (C) 2007  Ch. Klippel <ck@mamalala.net>

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

#include "keyboard.h"
#include "lpc2220.h"
#include "irq.h"

unsigned int keys[2];
unsigned char autorepeat;
unsigned char batcnt;

static unsigned int xkeys[2];
static unsigned int keytest;
static unsigned char half;
static int x;

void initKeys(void)
{
	keys[0] = 0;
	keys[1] = 0;
}

void startKeyIRQ(void)
{
	autorepeat = 0;

	T0TCR = 0x02;
	T0TC = 187490;
	T0PR = 0x03; 	// 7.500.000 Hz / 4 = 1.875.000 Hz
	T0MR0 = 187500;	// 1.875.000 Hz / 187.500 = 10 Hz
	T0MCR = 0x03;
	T0TCR = 0x01;

	VICVectAddr15 = (unsigned long)&(keyIRQ);
	VICVectCntl15 = VIC_SLOT_EN | INT_SRC_TIMER0;
	VICIntEnable = INT_TIMER0;
}

/*  Stubs for various interrupts (may be replaced later)  */
/*  ----------------------------------------------------  */

void  __attribute__ ((interrupt("IRQ")))  keyIRQ(void)
{
	xkeys[0] = 0;
	xkeys[1] = 0;
	keytest = 0x01;
	half = 0;
	
	batcnt--;
	
	for(x=(1<<18); x!=(1<<23); x<<=1)
	{
		IOCLR2=x;
		testLine(FIOPIN0&(1<<28));
		testLine(FIOPIN0&(1<<27));
		testLine(FIOPIN0&(1<<22));
		testLine(FIOPIN0&(1<<13));
		testLine(IOPIN3&(1<<21));
		testLine(IOPIN3&(1<<20));
		IOSET2=x;
	}

	keytest = 0x01;
	half = 1;

	for(; x!=(1<<25); x<<=1)
	{
		IOCLR2=x;
		testLine(FIOPIN0&(1<<28));
		testLine(FIOPIN0&(1<<27));
		testLine(FIOPIN0&(1<<22));
		testLine(FIOPIN0&(1<<13));
		testLine(IOPIN3&(1<<21));
		testLine(IOPIN3&(1<<20));
		IOSET2=x;
	}

	keys[0] = xkeys[0];
	keys[1] = xkeys[1];
	if(autorepeat < AUTO_TIMEOUT)
		autorepeat++;
	T0IR = 1;
	VICVectAddr = 0;
	
	
}
