/*
    ir_itt.c - itt protocoll encoder
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
#include "ir_itt.h"
#include "encoders.h"
#include "codes.h"

const struct irModule ITT_Module =
{ 
	ITT_Encode,
	ITT_Send,
	ITT_Repeat,
	ITT_Stop,
	103,	// carrier
	2,	// carrier-on
	3	// carrier-off
};

const struct irModule ITT2_Module =
{ 
	ITT_Encode,
	ITT_Send,
	ITT_Repeat,
	ITT_Stop,
	111,	// carrier 
	1,	// carrier-on
	3	// carrier-off
};

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;
extern volatile unsigned long keyMap[42];

#define ITT_IDLE	0x00
#define ITT_LEADIN_A	0x01
#define ITT_LEADIN_B	0x02
#define ITT_START_A	0x03
#define ITT_START_B	0x04
#define ITT_BIT_A	0x05
#define ITT_BIT_B	0x06
#define ITT_LEADOUT_A	0x07
#define ITT_LEADOUT_B	0x08
#define ITT_STOP	0x09
#define ITT_WAIT	0x0A

#define ITT_LEADTIME	 13
#define ITT_BITTIME_0	  4
#define ITT_BITTIME_1	  8
#define ITT_WAITTIME	130

#define ITT2_LEADTIME	 15
#define ITT2_BITTIME_0	  4
#define ITT2_BITTIME_1	  9

void __attribute__ ((section(".text.fastcode"))) ITT_Encode (void)
{
	switch(ir.state)
	{
		case ITT_IDLE:
			mod_enable = 0;
			break;

		case ITT_LEADIN_A:
		case ITT_START_A:
		case ITT_BIT_A:
		case ITT_LEADOUT_A:
		case ITT_STOP:
			cycles = 1;
			mod_enable = 1;
			ir.state++;
			break;

		case ITT_LEADIN_B:
		case ITT_LEADOUT_B:
			mod_enable = 0;
			cycles = ir.general.leadtime;
			ir.state++;
			break;

		case ITT_START_B:
			mod_enable = 0;
			cycles = ir.general.bittime0;
			ir.state++;
			break;

		case ITT_BIT_B:
			mod_enable = 0;
			if(ir.cmd & 0x01)
				cycles = ir.general.bittime1;
			else
				cycles = ir.general.bittime0;
			ir.cmd >>= 1;
			ir.general.bit++;
			if(ir.general.bit == ir.general.numbits)
				ir.state++;
			else
				ir.state--;
			break;

		case ITT_WAIT:
			mod_enable = 0;
			cycles = 40;
			ir.general.wait++;
			if(ir.general.wait >= ITT_WAITTIME)
			{
				ir.general.bit = 0;
				ir.general.wait = 0;
				ir.state = ITT_IDLE;
			}
			break;
	}
}

void ITT_LoadMap(unsigned char map)
{
	switch(map)
	{
		case 2:
			ir.general.leadtime	= ITT2_LEADTIME;
			ir.general.bittime0	= ITT2_BITTIME_0;
			ir.general.bittime1	= ITT2_BITTIME_1;
			setIR(ITT2_Module);
			break;
		default:
			ir.general.leadtime	= ITT_LEADTIME;
			ir.general.bittime0	= ITT_BITTIME_0;
			ir.general.bittime1	= ITT_BITTIME_1;
			setIR(ITT_Module);
			break;
	}
}

void ITT_Init(unsigned char map)
{
	if(map < ITT.num_tables)
	{
		ir.state	= ITT_IDLE;
		ir.cmd		= 0x0000;
		ir.actcmd	= 0x0000;
		ir.general.bit		= 0x00;
		ir.general.numbits	= 0x0A;
		ir.general.wait	= 0;
		copyMapS((unsigned short*)ITT.table[map].codes);
		ITT_LoadMap(map);
		cycles = 1;
	}
}

void ITT_Send(unsigned long cmd)
{
	ir.actcmd = cmd;
	setIRspeed(ITT_Module);
	if(ir.actcmd != 0x0000)
	{
		if(ir.state == ITT_IDLE)
		{
			ir.cmd = ir.actcmd;
			ir.state++;
			runIR();
		}
	}
}

void ITT_Repeat(void)
{
	if(ir.actcmd != 0x0000)
	{
		if(ir.state == ITT_IDLE)
		{
			ir.cmd = ir.actcmd ^ 0x00f;
			ir.state++;
			runIR();
		}
	}
}

void ITT_Stop(void)
{
/*
waitend1:
	if(ir.state != ITT_IDLE)
		goto waitend1;

	ir.state++;
*/
	if(ir.actcmd != 0x00000000)
	{
waitend2:
		if(ir.state != ITT_IDLE)
			goto waitend2;
	}
	ir.actcmd = 0x0000;
	stopIR();
}


