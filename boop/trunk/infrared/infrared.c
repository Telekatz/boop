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

#include "global.h"
#include "infrared.h"
#include "infraredirq.h"
#include "irq.h"
#include "lpc2220.h"
#include "keyboard.h"
#include "encoders.h"
#include "codes.h"
#include "ir_selector.h"

volatile unsigned char mod_enable;
volatile unsigned char hi_border;
volatile unsigned char lo_border;
volatile unsigned int cycles;
volatile unsigned long keyMap[42];

ir_fn	irEncoder;
ir_sfn	irSend;
ir_fn	irRepeat;
ir_fn	irStop;

unsigned char act_encoder, act_set;

const struct irModule defModule = { defIR, defSender, defRepeater, defStopper, 50, 1, 4 };
void dummy(void)
{
	irStop = RC_setting; //make compiler happy for select_ir in mainmenu
}

void initIR(void)
{
//	irStop = select_ir; //make compiler happy for select_ir in mainmenu
	setIR(defModule);
}

void startIrIRQ(void)
{

	T1TCR		= 0x03;
	T1TC		= 0;
	T1PC		= 0;
	T1PR		= 0x00;
//	T1PR		= 0x01;
	T1MCR		= 0x03;

	//VICVectAddr1	= (unsigned long)&(irIRQ);
	//VICVectCntl1	= VIC_SLOT_EN | INT_SRC_TIMER1;
	VICIntSelect 	|= INT_TIMER1;
	VICIntEnable	= INT_TIMER1;
}

void setIR(struct irModule module)
{
	T1TCR		= 0x02;
	irEncoder	= module.encoder;
	irSend		= module.sender;
	irRepeat	= module.repeater;
	irStop		= module.stopper;
//	setIRspeed(module);
	T1MR0		= module.tval;
	hi_border	= module.hi_border;
	lo_border	= module.lo_border;
	cycles		= 100;
	ir.state	= 0x00;
	ir.actcmd	= 0;
	ir.cmd		= 0;
	T1TCR		= 0x03;
}
/*  Stubs for various interrupts (may be replaced later)  */
/*  ----------------------------------------------------  */

void __attribute__ ((section(".text.fastcode")))  defIR(void) 
{
	return;
}

void defSender(unsigned long cmd) 
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

void runIR(void)
{
	T1TCR		= 0x01;
}

void stopIR(void)
{
	T1TCR		= 0x03;
}

void copyMapC(unsigned char *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void copyMapPC(unsigned int pre, unsigned char *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		if(map[x] != 0)
			keyMap[x] = pre | map[x];
		else
			keyMap[x] = 0x00000000;
	}
}

void copyMapS(unsigned short *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void copyMapI(unsigned int *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

unsigned long setEncoder( unsigned char x, unsigned char y )
{
	if(EncIsValid(x,y)) {
		act_encoder = x;
		act_set = y; 
		encoders.encoder[act_encoder].init(act_set);
		return(1);
	}
	return(0);
}

unsigned long getCode(void) {

	int keynum = getKeynum();
	
	if (keynum < 42)
		return keyMap[keynum];
	else
		return 0xFFFFFFFF;
}
