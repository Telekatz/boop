/*
    ir_recs80.c - space modulated protocoll encoder
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
#include "ir_recs80.h"
#include "recs80_codes.h"

const struct irModule RECS80_Module =
{ 
	RECS80_Encode,
	RECS80_Send,
	RECS80_Repeat,
	RECS80_Stop,
	40,	// carrier 
	1,	// carrier-on
	4	// carrier-off
};

static unsigned int	recs80_cmd;
static unsigned int	recs80_actcmd;
static unsigned char	recs80_state;
static unsigned char	recs80_bit;
static unsigned char	recs80_wait;
static unsigned char	recs80_toggle;

#define RECS80_IDLE	0x00
#define RECS80_BIT_A	0x01
#define RECS80_BIT_B	0x02
#define RECS80_STOP	0x03
#define RECS80_WAIT	0x04

#define RECS80_NUMBITS	 11
#define RECS80_BITTIME	  7
#define RECS80_BIT0	 27*RECS80_BITTIME
#define RECS80_BIT1	 41*RECS80_BITTIME
#define RECS80_WAITTIME	 10

void  RECS80_Encode (void)
{
	switch(recs80_state)
	{
		case RECS80_IDLE:
			mod_enable = 0;
			break;

		case RECS80_BIT_A:
		case RECS80_STOP:
			cycles = RECS80_BITTIME;
			mod_enable = 1;
			recs80_state++;
			break;

		case RECS80_BIT_B:
			mod_enable = 0;
			if(recs80_cmd & 0x0400)
				cycles = RECS80_BIT1;
			else
				cycles = RECS80_BIT0;
			recs80_cmd <<= 1;
			recs80_bit++;
			if(recs80_bit == RECS80_NUMBITS)
				recs80_state++;
			else
				recs80_state--;
			break;

		case RECS80_WAIT:
			mod_enable = 0;
			cycles = RECS80_BIT0;
			recs80_wait++;
			if(recs80_wait >= RECS80_WAITTIME)
			{
				recs80_bit = 0;
				recs80_wait = 0;
				recs80_state = RECS80_IDLE;
			}
			break;
	}
}

void RECS80_Init(unsigned char map)
{
	recs80_state		= RECS80_IDLE;
	recs80_cmd		= 0x0000;
	recs80_actcmd		= 0x0000;
	recs80_bit		= 0x00;
	recs80_wait		= 0;
	recs80_toggle		= 1;
	RECS80_LoadMap(map);
	setIR(RECS80_Module);
	cycles = RECS80_BITTIME;
}

void RECS80_Send(void)
{
	recs80_actcmd = getCode();

	if(recs80_toggle & 0x01)
		recs80_actcmd |= 0x0200;
	RECS80_Repeat();
}

void RECS80_Repeat(void)
{
	if(recs80_actcmd != 0x0000)
	{
		if(recs80_state == RECS80_IDLE)
		{
			recs80_cmd = recs80_actcmd;
			recs80_state++;
			runIR();
		}
	}
}

void RECS80_Stop(void)
{
	if(recs80_actcmd != 0x00000000)
	{
waitend:
		if(recs80_state != RECS80_IDLE)
			goto waitend;
	}
	recs80_toggle++;
	stopIR();
}

void RECS80_CopyMap(unsigned short *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void RECS80_LoadMap(unsigned char map)
{
	switch(map)
	{
		case 0:
			RECS80_CopyMap((unsigned short*)RECS80_seleco_020);
			break;
		default:
			RECS80_CopyMap((unsigned short*)RECS80_seleco_157);
			break;
	}
}


