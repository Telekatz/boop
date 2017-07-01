/*
    ir_sirc.c - sony sirc protocoll encoder
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

#include "lpc2220.h"
#include "infrared.h"
#include "ir_sirc.h"
#include "encoders.h"
#include "codes.h"

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;
extern volatile unsigned long keyMap[42];

#define SIRC_IDLE	0x00
#define SIRC_START_A	0x01
#define SIRC_START_B	0x02
#define SIRC_BIT_A	0x03
#define SIRC_BIT_B	0x04
#define SIRC_WAIT	0x05

#define SIRC_STARTLEN	 96
#define SIRC_BITLEN_0	 24
#define SIRC_BITLEN_1	 48
#define SIRC_BITOFF	 24
#define SIRC_GTIME	 45

const struct irModule SIRC_Module =
{ 
	SIRC_Encode,
	SIRC_Send,
	SIRC_Repeat,
	SIRC_Stop,
	122,	// carrier 
	1,	// carrier-on
	3,	// carrier-off
};

void __attribute__ ((section(".text.fastcode"))) SIRC_Encode (void)
{
	switch(ir.state)
	{
		case SIRC_IDLE:
			mod_enable = 0;
			break;

		case SIRC_START_A:
			mod_enable = 1;
			cycles = SIRC_STARTLEN;
			ir.state++;
			break;

		case SIRC_START_B:
			mod_enable = 0;
			cycles = SIRC_BITOFF;
			ir.state++;
			break;

		case SIRC_BIT_A:
			mod_enable = 1;
			if(ir.cmd & 0x01)
				cycles = SIRC_BITLEN_1;
			else
				cycles = SIRC_BITLEN_0;
			ir.cmd >>= 1;
			ir.state++;
			break;

		case SIRC_BIT_B:
			mod_enable = 0;
			cycles = SIRC_BITOFF;
			ir.general.bit++;
			if(ir.general.bit == ir.general.numbits)
				ir.state++;
			else
				ir.state--;
			break;

		case SIRC_WAIT:
			mod_enable = 0;
			ir.general.wait++;
			cycles = SIRC_BITOFF;		// 1 ms
			if(ir.general.wait >= ir.general.pause)
			{
				ir.general.bit = 0;
				ir.general.wait = 0;
				ir.state = SIRC_IDLE;
			}
			break;

	}
}

void SIRC_Init(unsigned char map)
{
	if(map < SIRC.num_tables)
	{
		ir.cmd	= 0x00000000;
		ir.actcmd	= 0x00000000;
		ir.general.bit	= 0;
		ir.general.numbits	= 12;
		ir.general.wait	= 0;
		ir.general.pause	= SIRC_GTIME;
		ir.state	= SIRC_IDLE;
		copyMapS((unsigned short*)SIRC.table[map].codes);
		setIR(SIRC_Module);
	}
}

void SIRC_Send(unsigned long cmd)
{
	ir.actcmd = cmd;

	if(ir.actcmd & 0x800000)
		ir.general.numbits = 20;
	else if(ir.actcmd & 0x8000)
		ir.general.numbits = 15;
	else
		ir.general.numbits = 12;
	setIRspeed(SIRC_Module);
	SIRC_Repeat();
}

void SIRC_Repeat(void)
{
	if(ir.actcmd != 0x00000000)
	{
		if(ir.state == SIRC_IDLE)
		{
			ir.cmd = ir.actcmd;
			ir.state++;
			runIR();
		}
	}
}

void SIRC_Stop(void)
{
	if(ir.actcmd != 0x00000000)
	{
waitend:
		if(ir.state != SIRC_IDLE)
			goto waitend;
	}
	ir.actcmd = 0x0000;
	stopIR();
}
