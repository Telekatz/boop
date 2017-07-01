/*
    ir_rec80.c - nec rec80 and derivates ir encoder
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
#include "ir_rec80.h"
#include "rec80_codes.h"

const struct irModule REC80_Module =
{ 
	REC80_Encode,
	REC80_Send,
	REC80_Repeat,
	REC80_Stop,
	47,	// carrier 
	1,	// carrier-on
	4,	// carrier-off
};

static unsigned int	rec80_cmd;
static unsigned int	rec80_actcmd;
static unsigned int	rec80_preamble;
static unsigned char	rec80_state;
static unsigned char	rec80_bit;
static unsigned char	rec80_numbits;
static unsigned short	rec80_wait;
static unsigned short	rec80_pause;

#define REC80_IDLE	0x00
#define REC80_HEAD_A	0x01
#define REC80_HEAD_B	0x02
#define REC80_PRE_A	0x03
#define REC80_PRE_B	0x04
#define REC80_BIT_A	0x05
#define REC80_BIT_B	0x06
#define REC80_TRAIL	0x07
#define REC80_WAIT	0x08

#define REC80_HEADER_1	128
#define REC80_HEADER_0	51
#define REC80_BONTIME	13
#define REC80_BITTIME_0	13
#define REC80_BITTIME_1	38
#define REC80_WAITTIME	187

void  REC80_Encode (void)
{
	switch(rec80_state)
	{
		case REC80_IDLE:
			mod_enable = 0;
			break;

		case REC80_HEAD_B:
			mod_enable = 0;
			cycles = REC80_HEADER_0;
			rec80_cmd = rec80_preamble;
			rec80_bit = 0;
			rec80_state++;
			break;

		case REC80_HEAD_A:
		case REC80_PRE_A:
		case REC80_BIT_A:
		case REC80_TRAIL:
			mod_enable = 1;
			if(rec80_state == REC80_HEAD_A)
				cycles = REC80_HEADER_1;
			else
				cycles = REC80_BONTIME;
			rec80_state++;
			break;

		case REC80_PRE_B:
		case REC80_BIT_B:
			mod_enable = 0;
			if(rec80_cmd & 0x01)
				cycles = REC80_BITTIME_1;
			else
				cycles = REC80_BITTIME_0;
			rec80_cmd >>= 1;
			rec80_bit++;
			if(rec80_bit == rec80_numbits)
			{
				if(rec80_state == REC80_PRE_B)
				{
					rec80_bit = 0;
					rec80_cmd = rec80_actcmd;
				}
				rec80_state++;
			}
			else
				rec80_state--;
			break;

		case REC80_WAIT:
			mod_enable = 0;
			rec80_wait++;
			cycles = REC80_BONTIME;		// 1 ms
			if(rec80_wait >= rec80_pause)
			{
				rec80_bit = 0;
				rec80_wait = 0;
				rec80_state = REC80_IDLE;
			}
			break;
	}
}

void REC80_Init(unsigned char map)
{
	rec80_cmd	= 0x00000000;
	rec80_actcmd	= 0x00000000;
	rec80_bit	= 0;
	rec80_numbits	= 24;
	rec80_wait	= 0;
	rec80_pause	= REC80_WAITTIME;
	rec80_state	= REC80_IDLE;
	REC80_LoadMap(map);
	setIR(REC80_Module);
}

void REC80_Send(void)
{
	rec80_actcmd = getCode();

	if(rec80_actcmd & 0x80000000)
		rec80_numbits = 24;
	else
		rec80_numbits = 0;

 	rec80_actcmd &= 0x0FFFFFFF;
	
	REC80_Repeat();
}

void REC80_Repeat(void)
{
	if(rec80_actcmd != 0x00000000)
	{
		if(rec80_state == REC80_IDLE)
		{
			rec80_state++;
			runIR();
		}
	}
}

void REC80_Stop(void)
{
	if(rec80_actcmd != 0x00000000)
	{
waitend:
		if(rec80_state != REC80_IDLE)
			goto waitend;
	}
	rec80_actcmd = 0x00000000;
	stopIR();
}

void REC80_CopyMap(unsigned int *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void REC80_LoadMap(unsigned char map)
{
	switch(map)
	{
		default:
			rec80_preamble	= 0x00A02002;
			REC80_CopyMap((unsigned int*)REC80_rak_sc957wk);
			break;
	}
}


