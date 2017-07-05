/*
    ir_rca.c - space modulated protocoll encoder
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
#include "ir_rca.h"
#include "rca_codes.h"

const struct irModule RCA_Module =
{ 
	RCA_Encode,
	RCA_Send,
	RCA_Repeat,
	RCA_Stop,
	37,	// carrier 
	1,	// carrier-on
	3	// carrier-off
};

static unsigned int	rca_cmd;
static unsigned int	rca_actcmd;
static unsigned char	rca_state;
static unsigned char	rca_bit;
static unsigned char	rca_wait;

#define RCA_IDLE	0x00
#define RCA_PRE_A	0x01
#define RCA_PRE_B	0x02
#define RCA_BIT_A	0x03
#define RCA_BIT_B	0x04
#define RCA_IBIT_A	0x05
#define RCA_IBIT_B	0x06
#define RCA_STOP	0x07
#define RCA_WAIT	0x08

#define RCA_NUMBITS	 12
#define RCA_BITTIME	 28
#define RCA_BIT0	 2*RCA_BITTIME
#define RCA_BIT1	 4*RCA_BITTIME
#define RCA_PRETIME	 8*RCA_BITTIME
#define RCA_WAITTIME	 30

void  RCA_Encode (void)
{
	switch(rca_state)
	{
		case RCA_IDLE:
			mod_enable = 0;
			break;

		case RCA_PRE_A:
			cycles = RCA_PRETIME;
			mod_enable = 1;
			rca_state++;
			break;

		case RCA_PRE_B:
			mod_enable = 0;
			rca_cmd = rca_actcmd;
			rca_state++;
			break;

		case RCA_BIT_A:
		case RCA_IBIT_A:
		case RCA_STOP:
			cycles = RCA_BITTIME;
			mod_enable = 1;
			rca_state++;
			break;

		case RCA_BIT_B:
		case RCA_IBIT_B:
			mod_enable = 0;
			if(rca_cmd & 0x0800)
				cycles = RCA_BIT1;
			else
				cycles = RCA_BIT0;
			rca_cmd <<= 1;
			rca_bit++;
			if(rca_bit == RCA_NUMBITS)
			{
				if(rca_state == RCA_BIT_B)
				{
					rca_cmd = ~rca_actcmd;
					rca_bit = 0;
				}
				rca_state++;
			}
			else
				rca_state--;
			break;

		case RCA_WAIT:
			mod_enable = 0;
			cycles = RCA_BITTIME;
			rca_wait++;
			if(rca_wait >= RCA_WAITTIME)
			{
				rca_bit = 0;
				rca_wait = 0;
				rca_state = RCA_IDLE;
			}
			break;
	}
}

void RCA_Init(unsigned char map)
{
	rca_state		= RCA_IDLE;
	rca_cmd			= 0x0000;
	rca_actcmd		= 0x0000;
	rca_bit			= 0x00;
	rca_wait		= 0;
	RCA_LoadMap(map);
	setIR(RCA_Module);
	cycles = RCA_BITTIME;
}

void RCA_Send(void)
{
	rca_actcmd = getCode();
	RCA_Repeat();
}

void RCA_Repeat(void)
{
	if(rca_actcmd != 0x0000)
	{
		if(rca_state == RCA_IDLE)
		{
			rca_state++;
			runIR();
		}
	}
}

void RCA_Stop(void)
{
	if(rca_actcmd != 0x00000000)
	{
waitend:
		if(rca_state != RCA_IDLE)
			goto waitend;
	}
	stopIR();
}

void RCA_CopyMapPC(unsigned short pre, unsigned char *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		if(map[x] != 0)
			keyMap[x] = pre | map[x];
		else
			keyMap[x] = 0x0000;
	}
}

void RCA_LoadMap(unsigned char map)
{
	switch(map)
	{
		default:
			RCA_CopyMapPC(0x0500, (unsigned char*)RCA_xbox);
			break;
	}
}


