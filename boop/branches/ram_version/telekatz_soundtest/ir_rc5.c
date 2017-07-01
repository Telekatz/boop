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

#include "infrared.h"
#include "ir_rc5.h"
#include "rc5_codes.h"

const struct irModule RC5_Module =
{ 
	RC5_Encode,
	RC5_Send,
	RC5_Repeat,
	RC5_Stop,
	42,	// carrier 
	1,	// carrier-on
	4	// carrier-off
};

static unsigned short	rc5_cmd;
static unsigned short	rc5_actcmd;
static unsigned char	rc5_state;
static unsigned char	rc5_bit;
static unsigned char	rc5_wait;
static unsigned char	rc5_toggle;

#define RC5_IDLE	0x00
#define RC5_START_A	0x01
#define RC5_START_B	0x02
#define RC5_BIT_A	0x03
#define RC5_BIT_B	0x04
#define RC5_WAIT	0x05


#define RC5_BITTIME	 32
#define RC5_WAITTIME	100

void  RC5_Encode (void)
{
	switch(rc5_state)
	{
		case RC5_IDLE:
			mod_enable = 0;
			break;

		case RC5_START_A:
			mod_enable = 0;
			rc5_state++;
			break;

		case RC5_START_B:
			mod_enable = 1;
			rc5_state++;
			break;

		case RC5_BIT_A:
			if(rc5_cmd & 0x1000)
				mod_enable = 0;
			else
				mod_enable = 1;
			rc5_cmd <<= 1;
			rc5_state++;
			break;

		case RC5_BIT_B:
			if(mod_enable == 0)
				mod_enable = 1;
			else
				mod_enable = 0;
			rc5_bit++;
			if(rc5_bit == 13)
				rc5_state++;
			else
				rc5_state--;
			break;

		case RC5_WAIT:
			mod_enable = 0;
			rc5_wait++;
			if(rc5_wait >= RC5_WAITTIME)
			{
				rc5_bit = 0;
				rc5_wait = 0;
				rc5_state = RC5_IDLE;
			}
			break;
	}
}

void RC5_Init(unsigned char map)
{
	rc5_state	= RC5_IDLE;
	rc5_cmd		= 0x0000;
	rc5_actcmd	= 0x0000;
	rc5_bit		= 0x00;
	rc5_wait	= 0;
	RC5_LoadMap(map);
	setIR(RC5_Module);
	cycles = RC5_BITTIME;
}

void RC5_Send(void)
{
	rc5_actcmd = getCode();

	if(rc5_toggle & 0x01)
		rc5_actcmd &= 0xF7FF;
	RC5_Repeat();
}

void RC5_Repeat(void)
{
	if(rc5_actcmd != 0x0000)
	{
		if(rc5_state == RC5_IDLE)
		{
			rc5_cmd = rc5_actcmd;
			rc5_state++;
			runIR();
		}
	}
}

void RC5_Stop(void)
{
	if(rc5_actcmd != 0x00000000)
	{
waitend:
		if(rc5_state != RC5_IDLE)
			goto waitend;
	}

	rc5_toggle++;
	rc5_actcmd = 0x0000;
	stopIR();
}

void RC5_CopyMap(unsigned short *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void RC5_LoadMap(unsigned char map)
{
	switch(map)
	{
		default:
			RC5_CopyMap((unsigned short*)RC5_tv);
			break;
	}
}


