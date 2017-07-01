/*
    ir_spaceenc.c - space modulated protocoll encoder
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
#include "ir_spaceenc.h"
#include "encoders.h"
#include "codes.h"
#include "ir_codes/spaceenc/schneider_dtv3_xtra"

const struct irModule SPACEENC_Module =
{ 
	SPACEENC_Encode,
	SPACEENC_Send,
	SPACEENC_Repeat,
	SPACEENC_Stop,
	119,	// carrier 
	1,	// carrier-on
	3	// carrier-off
};

const struct irModule SPACEENC2_Module =
{ 
	SPACEENC_Encode,
	SPACEENC2_Send,
	SPACEENC2_Repeat,
	SPACEENC2_Stop,
	119,	// carrier 
	1,	// carrier-on
	3	// carrier-off
};

static unsigned int	*ir_codes;
static unsigned char	*ir_precodes;

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;
extern volatile unsigned long keyMap[42];

#define SPACEENC_IDLE	0x00
#define SPACEENC_PRE_A	0x01
#define SPACEENC_PRE_B	0x02
#define SPACEENC_BIT_A	0x03
#define SPACEENC_BIT_B	0x04
#define SPACEENC_STOP	0x05
#define SPACEENC_WAIT	0x06

#define SPACEENC_NUMBITS	  6
#define SPACEENC_BITTIME	 20
#define SPACEENC_BIT0		 65
#define SPACEENC_BIT1		150
#define SPACEENC_WAITTIME	 59

#define SPACEENC2_BITTIME	 14
#define SPACEENC2_BIT0		  7
#define SPACEENC2_BIT1		 28
#define SPACEENC2_WAITTIME	170

void __attribute__ ((section(".text.fastcode"))) SPACEENC_Encode (void)
{
	switch(ir.state)
	{
		case SPACEENC_IDLE:
			mod_enable = 0;
			break;

		case SPACEENC_PRE_A:
		case SPACEENC_BIT_A:
			cycles = ir.general.bittime;
			mod_enable = 1;
			ir.state++;
			break;

		case SPACEENC_PRE_B:
			mod_enable = 0;
			if(ir.cmd & 0x01)
				cycles = ir.general.bittime1;
			else
				cycles = ir.general.bittime0;
			ir.cmd >>= 1;
			ir.general.bit++;
			if(ir.general.bit == ir.general.prebits)
			{
				ir.state = SPACEENC_BIT_A;
				ir.general.bit = 0;
				ir.cmd = ir_codes[ir.actcmd];
			}
			else
			{
				ir.state--;
			}
			break;

		case SPACEENC_BIT_B:
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

		case SPACEENC_STOP:
			cycles = ir.general.bittime;
			mod_enable = 1;
			ir.state++;
			break;

		case SPACEENC_WAIT:
			mod_enable = 0;
			ir.general.wait++;
			if(ir.general.wait >= ir.general.waittime)
			{
				ir.general.bit = 0;
				ir.general.wait = 0;
				ir.state = SPACEENC_IDLE;
			}
			break;
	}
}

void SPACEENC_LoadMap(unsigned char map)
{
	switch(map)
	{
		case 1:
			ir.general.numbits	= SPACEENC_NUMBITS;
			ir.general.prebits	= 0;
			ir.general.pre		= 0x00;
			ir.general.bittime	= SPACEENC_BITTIME;
			ir.general.bittime0	= SPACEENC_BIT0;
			ir.general.bittime1	= SPACEENC_BIT1;
			ir.general.waittime	= SPACEENC_WAITTIME;
			setIR(SPACEENC_Module);
			break;
		default:
			ir.general.numbits	= 32;
			ir.general.prebits	= 8;
			ir.general.bittime	= SPACEENC2_BITTIME;
			ir.general.bittime0	= SPACEENC2_BIT0;
			ir.general.bittime1	= SPACEENC2_BIT1;
			ir.general.waittime	= SPACEENC2_WAITTIME;
			ir_codes = (unsigned int*) dtv3_codes;
			ir_precodes = (unsigned char*) dtv3_pre;
			setIR(SPACEENC2_Module);
			break;
	}
}

void SPACEENC_Init(unsigned char map)
{
	if(map < SPACEENC.num_tables)
	{
		ir.state		= SPACEENC_IDLE;
		ir.cmd		= 0x0000;
		ir.actcmd		= 0x0000;
		ir.general.wait		= 0;
		ir.general.bit		= 0;
		copyMapC((unsigned char*)SPACEENC.table[map].codes);
		SPACEENC_LoadMap(map);
		cycles = SPACEENC_BITTIME;
	}
}

void SPACEENC_Send(unsigned long cmd)
{
	ir.actcmd = cmd;
	setIRspeed(SPACEENC_Module);
	SPACEENC_Repeat();
	runIR();
}

void SPACEENC_Repeat(void)
{
	if(ir.actcmd != 0x00000000)
	{
		if(ir.state == SPACEENC_IDLE)
		{
			ir.cmd = ir.actcmd;
			ir.state = SPACEENC_BIT_A;
		}
	}
}

void SPACEENC_Stop(void)
{
waitend1:
	while(ir.state != SPACEENC_IDLE)
		goto waitend1;

	if(ir.actcmd != 0x00000000)
	{
		SPACEENC_Repeat();
	
waitend2:
		if(ir.state != SPACEENC_IDLE)
			goto waitend2;
	}
	ir.actcmd = 0x00000000;
	stopIR();
}

void SPACEENC2_Send(unsigned long cmd)
{
	ir.actcmd = cmd;
	SPACEENC2_Repeat();
}

void SPACEENC2_Repeat(void)
{
	if(ir.actcmd != 0x00000000)
	{
		if(ir.state == SPACEENC_IDLE)
		{
			ir.cmd = ir_precodes[ir.actcmd];
			ir.state = SPACEENC_PRE_A;
			runIR();
		}
	}
}

void SPACEENC2_Stop(void)
{
	if(ir.actcmd != 0x00000000)
	{
waitend2:
		if(ir.state != SPACEENC_IDLE)
			goto waitend2;
	}
	ir.actcmd = 0x00000000;
	stopIR();
}


