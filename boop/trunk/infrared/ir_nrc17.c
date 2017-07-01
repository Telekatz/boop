/*
    ir_nrc17.c - nokia remote control 17 encoder
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
#include "ir_nrc17.h"
#include "encoders.h"
#include "codes.h"

#define NRC17_IDLE	0x00
#define NRC17_PRE_A	0x01
#define NRC17_PRE_B	0x02
#define NRC17_START_A	0x03
#define NRC17_START_B	0x04
#define NRC17_BIT_A	0x05
#define NRC17_BIT_B	0x06
#define NRC17_WAIT	0x07

#define NRC17_BONTIME	19
#define NRC17_BOFFTIME	19
#define NRC17_PTIME	95
#define NRC17_GTIME	156
#define NRC17_WTIME	38

#define NRC17_STARTSTOP_DBOX2	0xFEFF
#define NRC17_STARTSTOP_TP720	0x01FF

const struct irModule NRC17_Module =
{ 
	NRC17_Encode,
	NRC17_Send,
	NRC17_Repeat,
	NRC17_Stop,
	132,	// carrier
	1,	// carrier-on
	3	// carrier-off
};

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;
extern volatile unsigned long keyMap[42];

void __attribute__ ((section(".text.fastcode"))) NRC17_Encode(void) 
{
	switch(ir.state)
	{
		case NRC17_IDLE:
			mod_enable = 0;
			break;

		case NRC17_PRE_A:
		case NRC17_START_A:
			cycles = NRC17_BONTIME;	// 0.5 ms
			mod_enable = 1;
			ir.state++;
			break;

		case NRC17_PRE_B:
		case NRC17_START_B:
			if(ir.state == NRC17_PRE_B)
				cycles = NRC17_PTIME;	// 2,5 ms
			else
				cycles = NRC17_BOFFTIME;	// 0.5 ms
			mod_enable = 0;
			ir.state++;
			break;

		case NRC17_BIT_A:
			if(ir.cmd & 0x0001)
				mod_enable = 1;
			else
				mod_enable = 0;
			ir.cmd >>= 1;
			ir.state++;
			break;

		case NRC17_BIT_B:
			if(mod_enable == 1)
				mod_enable = 0;
			else
				mod_enable = 1;
			ir.general.bit++;
			if(ir.general.bit == ir.general.numbits)
				ir.state++;
			else
				ir.state--;
			break;

		case NRC17_WAIT:
			mod_enable = 0;
			ir.general.wait++;
			cycles = NRC17_BONTIME;		// 1 ms
			if(ir.general.wait >= ir.general.pause)
			{
				ir.general.bit = 0;
				ir.general.wait = 0;
				ir.general.pause = NRC17_PTIME;
				ir.state = NRC17_IDLE;
			}
			break;

	}
}
/*
void NRC17_CopyMap(unsigned char xtra, unsigned short *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}*/

void NRC17_LoadMap(unsigned char map)
{	
	switch(map)
	{
		case 1:		// tp720 - tv, 9 bit data
			ir.general.startstop = NRC17_STARTSTOP_TP720;
			ir.general.numbits = 9;
			break;
		default:	// nokia dbox2, 16 bit data
			ir.general.startstop = NRC17_STARTSTOP_DBOX2;
			ir.general.numbits = 16;
			break;
	}

}

void NRC17_Init(unsigned char map)
{
	if(map < NRC17.num_tables)
	{
		ir.state	= NRC17_IDLE;
		ir.cmd	= 0x0000;
		ir.actcmd	= 0x0000;
		ir.general.bit	= 0x00;
		ir.general.numbits	= 16;
		ir.general.wait	= 0;
		ir.general.pause	= NRC17_BONTIME;
		copyMapS((unsigned short*)NRC17.table[map].codes);
		NRC17_LoadMap(map);
		setIR(NRC17_Module);
	}
}

void NRC17_Send(unsigned long cmd)
{
	ir.actcmd = cmd;
	if(ir.actcmd != 0x0000)
	{
		setIRspeed(NRC17_Module);
		if(ir.state == NRC17_IDLE)
		{
			ir.general.pause = NRC17_WTIME;
			ir.cmd = ir.general.startstop;
			ir.state++;
			runIR();
		}
	}
}

void NRC17_Repeat(void)
{
	if(ir.actcmd != 0x0000)
	{
		if(ir.state == NRC17_IDLE)
		{
			ir.general.pause = NRC17_GTIME;
			ir.cmd = ir.actcmd;
			ir.state++;
		}
	}
}

void NRC17_Stop(void)
{
	if(ir.actcmd != 0x0000)
	{
retry:
		if(ir.state == NRC17_IDLE)
		{
			ir.cmd = ir.general.startstop;
			ir.state++;
		}
		else
			goto retry;
	
waitend:
		if(ir.state != NRC17_IDLE)
			goto waitend;
	}
	ir.actcmd = 0x0000;
	stopIR();
}	

