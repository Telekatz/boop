/*
    ir_rcmm.c - phillips rc5 protocoll encoder
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
#include "ir_rcmm.h"
#include "rcmm_codes.h"

const struct irModule RCMM_Module =
{ 
	RCMM_Encode,
	RCMM_Send,
	RCMM_Repeat,
	RCMM_Stop,
	42,	// carrier 
	1,	// carrier-on
	4	// carrier-off
};

static unsigned int	rcmm_cmd;
static unsigned int	rcmm_actcmd;
static unsigned char	rcmm_state;
static unsigned char	rcmm_bit;
static unsigned short	rcmm_wait;
static unsigned short	rcmm_pause;
static unsigned char	rcmm_len;
static unsigned char	rcmm_trail;

#define RCMM_IDLE	0x00
#define RCMM_HEADER_A	0x01
#define RCMM_HEADER_B	0x02
#define RCMM_BIT_A	0x03
#define RCMM_BIT_B	0x04
#define RCMM_TRAIL	0x05
#define RCMM_WAIT	0x06

#define RCMM_HEADER_ON	 15
#define RCMM_HEADER_OFF  10
#define RCMM_BIT_ON	 6
#define RCMM_BIT_00	 10
#define RCMM_BIT_01	 16
#define RCMM_BIT_10	 22
#define RCMM_BIT_11	 28
#define RCMM_PTIME	 110;

const unsigned char rcmm_times[4] = { RCMM_BIT_00, RCMM_BIT_01, RCMM_BIT_10, RCMM_BIT_11 };

void  RCMM_Encode (void)
{
	switch(rcmm_state)
	{
		case RCMM_IDLE:
			mod_enable = 0;
			break;

		case RCMM_HEADER_A:
		case RCMM_BIT_A:
			mod_enable = 1;
			if(rcmm_state == RCMM_HEADER_A)
				cycles = RCMM_HEADER_ON;
			else
				cycles = RCMM_BIT_ON;
			rcmm_state++;
			break;

		case RCMM_HEADER_B:
			mod_enable = 0;
			cycles = RCMM_HEADER_OFF;
			rcmm_state++;
			break;

		case RCMM_BIT_B:
			mod_enable = 0;
			cycles = rcmm_times[(rcmm_cmd & 0xC0000000) >> 30];
			rcmm_cmd <<= 2;
			rcmm_bit += 2;
			if(rcmm_bit == rcmm_len)
				rcmm_state++;
			else
				rcmm_state--;
			break;

		case RCMM_TRAIL:
			if(rcmm_trail)
			{
				mod_enable = 1;
				cycles = RCMM_BIT_ON;
			}
			else
				cycles = 1;
			rcmm_state++;
			break;

		case RCMM_WAIT:
			mod_enable = 0;
			rcmm_wait++;
			cycles = RCMM_BIT_ON;		// 1 ms
			if(rcmm_wait >= rcmm_pause)
			{
				rcmm_bit = 0;
				rcmm_wait = 0;
				rcmm_pause = RCMM_PTIME;
				rcmm_state = RCMM_IDLE;
			}
			break;

		
	}
}

void RCMM_Init(unsigned char map)
{
	rcmm_cmd	= 0x00000000;
	rcmm_actcmd	= 0x00000000;
	rcmm_state	= RCMM_IDLE;
	rcmm_bit	= 0;
	rcmm_wait	= 0;
	rcmm_pause	= RCMM_PTIME;
	RCMM_LoadMap(map);
	setIR(RCMM_Module);
}

void RCMM_Send(void)
{
	rcmm_actcmd = getCode();
	RCMM_Repeat();
}

void RCMM_Repeat(void)
{
	if(rcmm_actcmd != 0x00000000)
	{
		if(rcmm_state == RCMM_IDLE)
		{
			rcmm_cmd = rcmm_actcmd;
			rcmm_state++;
			runIR();
		}
	}
}

void RCMM_Stop(void)
{
	if(rcmm_actcmd != 0x00000000)
	{
waitend:
		if(rcmm_state != RCMM_IDLE)
			goto waitend;
	}
	rcmm_actcmd = 0x0000;
	stopIR();
}

void RCMM_CopyMapPrefix(unsigned int prefix, unsigned char *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = prefix + map[x];
	}
	keyMap[0] = 0x00000000;
}

void RCMM_LoadMap(unsigned char map)
{
	switch(map)
	{
		default:
			RCMM_CopyMapPrefix(0x0D500800, (unsigned char*)RCMM_galaxis);
			rcmm_len	= 32;
			rcmm_trail	= 1;
			break;
	}
}


