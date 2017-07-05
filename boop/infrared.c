/*
    infrared.c - ir carrier generation and encoding core
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

#include "infrared.h"
#include "irq.h"
#include "lpc2220.h"
#include "keyboard.h"

#include "externs.h"

static unsigned char c_cnt;
static unsigned int b_len;

unsigned char mod_enable;
unsigned char hi_border;
unsigned char lo_border;
unsigned int cycles;

ir_fn	irEncoder;
ir_fn	irSend;
ir_fn	irRepeat;
ir_fn	irStop;

const struct irModule defModule = { defIR, defSender, defRepeater, defStopper, 20, 0, 3 };

unsigned long keyMap[42];

void initIR(void)
{
	setIR(defModule);
}

void startIrIRQ(void)
{

	T1TCR		= 0x02;
	T1TC		= 0;
	T1PC		= 0;
	T1PR		= 0x00;
	T1MCR		= 0x03;
	T1TCR		= 0x01;

	VICVectAddr1	= (unsigned long)&(irIRQ);
	VICVectCntl1	= VIC_SLOT_EN | INT_SRC_TIMER1;
	VICIntEnable	= INT_TIMER1;
}

void setIR(struct irModule module)
{
	T1TCR		= 0x02;
	irEncoder	= module.encoder;
	irSend		= module.sender;
	irRepeat	= module.repeater;
	irStop		= module.stopper;
	T1MR0		= module.tval;
	hi_border	= module.hi_border;
	lo_border	= module.lo_border;
	cycles		= 100;
	T1TCR		= 0x01;
}
/*  Stubs for various interrupts (may be replaced later)  */
/*  ----------------------------------------------------  */

void defIR(void) 
{
	return;
}

void defSender(void) 
{
	return;
}

void defRepeater(void) 
{
	return;
}
void defStopper(void)
{
	return;
}

void __attribute__ ((interrupt("IRQ")))  irIRQ(void)
{
	c_cnt++;
	if(c_cnt <= hi_border)
	{
		FIOSET0 = (mod_enable<<21);
	}
	else
	{
		FIOCLR0 = (1<<21);
		if(c_cnt >= lo_border)
		{
			c_cnt = 0;
			b_len++;
			if(b_len >= cycles)
			{
				irEncoder();
				b_len = 0;
			}
		}
	}

	T1IR = 1;
	VICVectAddr = 0;
}

void runIR(void)
{
	T1TCR		= 0x01;
}

void stopIR(void)
{
	T1TCR		= 0x03;
}

#define doKey( _X, _Y ) if(_X) return keyMap[_Y]; else 

unsigned long getCode(void)
{
		doKey(KEY_A, 0);
		doKey(KEY_B, 1);
		doKey(KEY_C, 2);
		doKey(KEY_D, 3);
		doKey(KEY_Betty, 4);
		doKey(KEY_Exit, 5);
		doKey(KEY_Up, 6);
		doKey(KEY_Down, 7);
		doKey(KEY_Left, 8);
		doKey(KEY_Right, 9);
		doKey(KEY_OK, 10);
		doKey(KEY_Vplus, 11);
		doKey(KEY_Vminus, 12);
		doKey(KEY_Mute, 13);
		doKey(KEY_Pplus, 14);
		doKey(KEY_Pminus, 15);
		doKey(KEY_1, 16);
		doKey(KEY_2, 17);
		doKey(KEY_3, 18);
		doKey(KEY_4, 19);
		doKey(KEY_5, 20);
		doKey(KEY_6, 21);
		doKey(KEY_7, 22);
		doKey(KEY_8, 23);
		doKey(KEY_9, 24);
		doKey(KEY_0, 25);
		doKey(KEY_Minus, 26);
		doKey(KEY_AV, 27);
		doKey(KEY_Menu, 28);
		doKey(KEY_PiP, 29);
		doKey(KEY_AB, 30);
		doKey(KEY_16_9, 31);
		doKey(KEY_Info, 32);
		doKey(KEY_VTX1, 33);
		doKey(KEY_VTX2, 34);
		doKey(KEY_VTX3, 35);
		doKey(KEY_Blue, 36);
		doKey(KEY_Yellow, 37);
		doKey(KEY_Green, 38);
		doKey(KEY_Red, 39);
		doKey(KEY_TV, 40);
		doKey(KEY_Power, 41);
		return 0xFFFFFFFF;
}
