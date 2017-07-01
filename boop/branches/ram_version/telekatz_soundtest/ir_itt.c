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

#include "infrared.h"
#include "ir_itt.h"
#include "itt_codes.h"

const struct irModule ITT_Module =
{ 
	ITT_Encode,
	ITT_Send,
	ITT_Repeat,
	ITT_Stop,
	43,	// carrier 
	2,	// carrier-on
	3	// carrier-off
};

const struct irModule ITT2_Module =
{ 
	ITT_Encode,
	ITT_Send,
	ITT_Repeat,
	ITT_Stop,
	46,	// carrier 
	1,	// carrier-on
	3	// carrier-off
};

static unsigned short	itt_cmd;
static unsigned short	itt_actcmd;
static unsigned char	itt_state;
static unsigned char	itt_bit;
static unsigned char	itt_numbits;
static unsigned char	itt_wait;
static unsigned char	itt_leadtime;
static unsigned char	itt_bittime0;
static unsigned char	itt_bittime1;

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

void  ITT_Encode (void)
{
	switch(itt_state)
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
			itt_state++;
			break;

		case ITT_LEADIN_B:
		case ITT_LEADOUT_B:
			mod_enable = 0;
			cycles = itt_leadtime;
			itt_state++;
			break;

		case ITT_START_B:
			mod_enable = 0;
			cycles = itt_bittime0;
			itt_state++;
			break;

		case ITT_BIT_B:
			mod_enable = 0;
			if(itt_cmd & 0x01)
				cycles = itt_bittime1;
			else
				cycles = itt_bittime0;
			itt_cmd >>= 1;
			itt_bit++;
			if(itt_bit == itt_numbits)
				itt_state++;
			else
				itt_state--;
			break;

		case ITT_WAIT:
			mod_enable = 0;
			cycles = 40;
			itt_wait++;
			if(itt_wait >= ITT_WAITTIME)
			{
				itt_bit = 0;
				itt_wait = 0;
				itt_state = ITT_IDLE;
			}
			break;
	}
}

void ITT_Init(unsigned char map)
{
	itt_state	= ITT_IDLE;
	itt_cmd		= 0x0000;
	itt_actcmd	= 0x0000;
	itt_bit		= 0x00;
	itt_numbits	= 0x0A;
	itt_wait	= 0;
	ITT_LoadMap(map);
	cycles = 1;
}

void ITT_Send(void)
{
	itt_actcmd = getCode();
	ITT_Repeat();
}

void ITT_Repeat(void)
{
	if(itt_actcmd != 0x0000)
	{
		if(itt_state == ITT_IDLE)
		{
			itt_cmd = itt_actcmd;
			itt_state++;
			runIR();
		}
	}
}

void ITT_Stop(void)
{
waitend1:
	if(itt_state != ITT_IDLE)
		goto waitend1;

	itt_state++;

	if(itt_actcmd != 0x00000000)
	{
waitend2:
		if(itt_state != ITT_IDLE)
			goto waitend2;
	}
	itt_actcmd = 0x0000;
	stopIR();
}

void ITT_CopyMap(unsigned short *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void ITT_LoadMap(unsigned char map)
{
	switch(map)
	{
		case 0:
			itt_leadtime	= ITT_LEADTIME;
			itt_bittime0	= ITT_BITTIME_0;
			itt_bittime1	= ITT_BITTIME_1;
			setIR(ITT_Module);
			ITT_CopyMap((unsigned short*)ITT_seleco_019);
			break;
		case 1:
			itt_leadtime	= ITT_LEADTIME;
			itt_bittime0	= ITT_BITTIME_0;
			itt_bittime1	= ITT_BITTIME_1;
			setIR(ITT_Module);
			ITT_CopyMap((unsigned short*)ITT_seleco_049);
			break;
		case 2:
			itt_leadtime	= ITT2_LEADTIME;
			itt_bittime0	= ITT2_BITTIME_0;
			itt_bittime1	= ITT2_BITTIME_1;
			setIR(ITT2_Module);
			ITT_CopyMap((unsigned short*)ITT_seleco_108);
			break;
		default:
			itt_leadtime	= ITT_LEADTIME;
			itt_bittime0	= ITT_BITTIME_0;
			itt_bittime1	= ITT_BITTIME_1;
			setIR(ITT_Module);
			ITT_CopyMap((unsigned short*)ITT_seleco_131);
			break;
	}
}


