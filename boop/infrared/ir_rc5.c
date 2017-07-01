/*
    ir_rc5.c - phillips rc5 protocoll encoder
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
#include "ir_rc5.h"
#include "encoders.h"
#include "codes.h"

const struct irModule RC5_Module =
{
	RC5_Encode,
	RC5_Send,
	RC5_Repeat,
	RC5_Stop,
	139,	// carrier
	1,	// carrier-on
	3	// carrier-off
};

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;
extern volatile unsigned long keyMap[42];

#define RC5_IDLE	0x00
#define RC5_START_A	0x01
#define RC5_START_B	0x02
#define RC5_BIT_A	0x03
#define RC5_BIT_B	0x04
#define RC5_WAIT	0x05


#define RC5_BITTIME	 32
#define RC5_WAITTIME	100

void __attribute__ ((section(".text.fastcode"))) RC5_Encode (void)
{
	switch(ir.state)
	{
		case RC5_IDLE:
			mod_enable = 0;
			break;

		case RC5_START_A:
			mod_enable = 0;
			ir.state++;
			break;

		case RC5_START_B:
			mod_enable = 1;
			ir.state++;
			break;

		case RC5_BIT_A:
			if(ir.cmd & 0x1000)
				mod_enable = 0;
			else
				mod_enable = 1;
			ir.cmd <<= 1;
			ir.state++;
			break;

		case RC5_BIT_B:
			if(mod_enable == 0)
				mod_enable = 1;
			else
				mod_enable = 0;
			ir.general.bit++;
			if(ir.general.bit == 13)
				ir.state++;
			else
				ir.state--;
			break;

		case RC5_WAIT:
			mod_enable = 0;
			ir.general.wait++;
			if(ir.general.wait >= RC5_WAITTIME)
			{
				ir.general.bit = 0;
				ir.general.wait = 0;
				ir.state = RC5_IDLE;
			}
			break;
	}
}

void RC5_Init(unsigned char map)
{
	if(map < RC5.num_tables)
	{
		ir.state	= RC5_IDLE;
		ir.cmd		= 0x0000;
		ir.actcmd	= 0x0000;
		ir.general.bit		= 0x00;
		ir.general.wait	= 0;
		copyMapS((unsigned short*)RC5.table[map].codes);
		setIR(RC5_Module);
		cycles = RC5_BITTIME;
	}
}

void RC5_Send(unsigned long cmd)
{
	ir.actcmd = cmd;
	setIRspeed(RC5_Module);
	if((ir.actcmd != 0x0000) && (ir.actcmd != 0x1000))
	{
		//toggle bit sollte nicht über dir Konfiguration gesetzt werden
		if(ir.toggle & 0x01)
			ir.actcmd &= 0xF7FF;
		else
			ir.actcmd |= 0x0800;
		RC5_Repeat();
	}
}

void RC5_Repeat(void)
{
	if((ir.actcmd != 0x0000) && (ir.actcmd != 0x1000))
	{
		if(ir.state == RC5_IDLE)
		{
			ir.cmd = ir.actcmd;
			ir.state++;
			runIR();
		}
	}
}

void RC5_Stop(void)
{
	if((ir.actcmd != 0x0000) && (ir.actcmd != 0x1000))
	{
waitend:
		if(ir.state != RC5_IDLE)
			goto waitend;
	}

	ir.toggle++;
	ir.actcmd = 0x0000;
	stopIR();
}


