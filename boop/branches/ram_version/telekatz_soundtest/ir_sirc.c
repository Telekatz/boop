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

#include "infrared.h"
#include "ir_sirc.h"
#include "sirc_codes.h"

static unsigned int	sirc_cmd;
static unsigned int	sirc_actcmd;
static unsigned char	sirc_state;
static unsigned char	sirc_bit;
static unsigned char	sirc_numbits;
static unsigned short	sirc_wait;
static unsigned short	sirc_pause;

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
	38,	// carrier 
	1,	// carrier-on
	4,	// carrier-off
};

void  SIRC_Encode (void)
{
	switch(sirc_state)
	{
		case SIRC_IDLE:
			mod_enable = 0;
			break;

		case SIRC_START_A:
			mod_enable = 1;
			cycles = SIRC_STARTLEN;
			sirc_state++;
			break;

		case SIRC_START_B:
			mod_enable = 0;
			cycles = SIRC_BITOFF;
			sirc_state++;
			break;

		case SIRC_BIT_A:
			mod_enable = 1;
			if(sirc_cmd & 0x01)
				cycles = SIRC_BITLEN_1;
			else
				cycles = SIRC_BITLEN_0;
			sirc_cmd >>= 1;
			sirc_state++;
			break;

		case SIRC_BIT_B:
			mod_enable = 0;
			cycles = SIRC_BITOFF;
			sirc_bit++;
			if(sirc_bit == sirc_numbits)
				sirc_state++;
			else
				sirc_state--;
			break;

		case SIRC_WAIT:
			mod_enable = 0;
			sirc_wait++;
			cycles = SIRC_BITOFF;		// 1 ms
			if(sirc_wait >= sirc_pause)
			{
				sirc_bit = 0;
				sirc_wait = 0;
				sirc_state = SIRC_IDLE;
			}
			break;

	}
}

void SIRC_Init(unsigned char map)
{
	sirc_cmd	= 0x00000000;
	sirc_actcmd	= 0x00000000;
	sirc_bit	= 0;
	sirc_numbits	= 12;
	sirc_wait	= 0;
	sirc_pause	= SIRC_GTIME;
	sirc_state	= SIRC_IDLE;
	SIRC_LoadMap(map);
	setIR(SIRC_Module);
}

void SIRC_Send(void)
{
	sirc_actcmd = getCode();

	if(sirc_actcmd & 0x800000)
		sirc_numbits = 20;
	else if(sirc_actcmd & 0x8000)
		sirc_numbits = 15;
	else
		sirc_numbits = 12;
	SIRC_Repeat();
}

void SIRC_Repeat(void)
{
	if(sirc_actcmd != 0x00000000)
	{
		if(sirc_state == SIRC_IDLE)
		{
			sirc_cmd = sirc_actcmd;
			sirc_state++;
			runIR();
		}
	}
}

void SIRC_Stop(void)
{
	if(sirc_actcmd != 0x00000000)
	{
waitend:
		if(sirc_state != SIRC_IDLE)
			goto waitend;
	}
	sirc_actcmd = 0x0000;
	stopIR();
}

void SIRC_CopyMapS(unsigned int pre, unsigned short *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void SIRC_CopyMapI(unsigned int pre, unsigned int *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void SIRC_LoadMap(unsigned char map)
{
	switch(map)
	{
		case 0:
			SIRC_CopyMapS(0x8000, (unsigned short*)SIRC_rm_pp411_av2);
			break;
		default:
			SIRC_CopyMapS(0, (unsigned short*)SIRC_tv);
			break;
	}
}

