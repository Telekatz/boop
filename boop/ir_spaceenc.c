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

#include "infrared.h"
#include "ir_spaceenc.h"
#include "spaceenc_codes.h"

const struct irModule SPACEENC_Module =
{ 
	SPACEENC_Encode,
	SPACEENC_Send,
	SPACEENC_Repeat,
	SPACEENC_Stop,
	37,	// carrier 
	1,	// carrier-on
	4	// carrier-off
};

const struct irModule SPACEENC2_Module =
{ 
	SPACEENC_Encode,
	SPACEENC2_Send,
	SPACEENC2_Repeat,
	SPACEENC2_Stop,
	37,	// carrier 
	1,	// carrier-on
	4	// carrier-off
};

static unsigned int	spaceenc_cmd;
static unsigned int	spaceenc_pre;
static unsigned int	spaceenc_actcmd;
static unsigned int	spaceenc_bit;
static unsigned char	spaceenc_state;
static unsigned char	spaceenc_numbits;
static unsigned char	spaceenc_prebits;
static unsigned char	spaceenc_wait;
static unsigned char	spaceenc_waittime;

static unsigned char	spaceenc_bittime;
static unsigned char	spaceenc_bit0;
static unsigned char	spaceenc_bit1;

static unsigned int	*spaceenc_codes;
static unsigned char	*spaceenc_precodes;

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

void  SPACEENC_Encode (void)
{
	switch(spaceenc_state)
	{
		case SPACEENC_IDLE:
			mod_enable = 0;
			break;

		case SPACEENC_PRE_A:
		case SPACEENC_BIT_A:
			cycles = spaceenc_bittime;
			mod_enable = 1;
			spaceenc_state++;
			break;

		case SPACEENC_PRE_B:
			mod_enable = 0;
			if(spaceenc_cmd & 0x01)
				cycles = spaceenc_bit1;
			else
				cycles = spaceenc_bit0;
			spaceenc_cmd >>= 1;
			spaceenc_bit++;
			if(spaceenc_bit == spaceenc_prebits)
			{
				spaceenc_state = SPACEENC_BIT_A;
				spaceenc_bit = 0;
				spaceenc_cmd = spaceenc_codes[spaceenc_actcmd];
			}
			else
			{
				spaceenc_state--;
			}
			break;

		case SPACEENC_BIT_B:
			mod_enable = 0;
			if(spaceenc_cmd & 0x01)
				cycles = spaceenc_bit1;
			else
				cycles = spaceenc_bit0;
			spaceenc_cmd >>= 1;
			spaceenc_bit++;
			if(spaceenc_bit == spaceenc_numbits)
				spaceenc_state++;
			else
				spaceenc_state--;
			break;

		case SPACEENC_STOP:
			cycles = spaceenc_bittime;
			mod_enable = 1;
			spaceenc_state++;
			break;

		case SPACEENC_WAIT:
			mod_enable = 0;
			spaceenc_wait++;
			if(spaceenc_wait >= spaceenc_waittime)
			{
				spaceenc_bit = 0;
				spaceenc_wait = 0;
				spaceenc_state = SPACEENC_IDLE;
			}
			break;
	}
}

void SPACEENC_Init(unsigned char map)
{
	spaceenc_state		= SPACEENC_IDLE;
	spaceenc_cmd		= 0x0000;
	spaceenc_actcmd		= 0x0000;
	spaceenc_wait		= 0;
	spaceenc_bit		= 0;
	SPACEENC_LoadMap(map);
	cycles = SPACEENC_BITTIME;
}

void SPACEENC_Send(void)
{
	spaceenc_actcmd = getCode();
	SPACEENC_Repeat();
	runIR();
}

void SPACEENC_Repeat(void)
{
	if(spaceenc_actcmd != 0x00000000)
	{
		if(spaceenc_state == SPACEENC_IDLE)
		{
			spaceenc_cmd = spaceenc_actcmd;
			spaceenc_state = SPACEENC_BIT_A;
		}
	}
}

void SPACEENC_Stop(void)
{
waitend1:
	while(spaceenc_state != SPACEENC_IDLE)
		goto waitend1;

	if(spaceenc_actcmd != 0x00000000)
	{
		SPACEENC_Repeat();
	
waitend2:
		if(spaceenc_state != SPACEENC_IDLE)
			goto waitend2;
	}
	spaceenc_actcmd = 0x00000000;
	stopIR();
}

void SPACEENC2_Send(void)
{
	spaceenc_actcmd = getCode();
	SPACEENC2_Repeat();
}

void SPACEENC2_Repeat(void)
{
	if(spaceenc_actcmd != 0x00000000)
	{
		if(spaceenc_state == SPACEENC_IDLE)
		{
			spaceenc_cmd = spaceenc_precodes[spaceenc_actcmd];
			spaceenc_state = SPACEENC_PRE_A;
			runIR();
		}
	}
}

void SPACEENC2_Stop(void)
{
	if(spaceenc_actcmd != 0x00000000)
	{
waitend2:
		if(spaceenc_state != SPACEENC_IDLE)
			goto waitend2;
	}
	spaceenc_actcmd = 0x00000000;
	stopIR();
}

void SPACEENC_CopyMapC(unsigned char *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void SPACEENC_CopyMapS(unsigned short *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void SPACEENC_CopyMapI(unsigned int *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void SPACEENC_LoadMap(unsigned char map)
{
	switch(map)
	{
		case 0:
			SPACEENC_CopyMapC((unsigned char*)SPACEENC_seleco_027);
			spaceenc_numbits	= SPACEENC_NUMBITS;
			spaceenc_prebits	= 0;
			spaceenc_pre		= 0x00;
			spaceenc_bittime	= SPACEENC_BITTIME;
			spaceenc_bit0		= SPACEENC_BIT0;
			spaceenc_bit1		= SPACEENC_BIT1;
			spaceenc_waittime	= SPACEENC_WAITTIME;
			setIR(SPACEENC_Module);
			break;
		default:
			SPACEENC_CopyMapC((unsigned char*)SPACEENC_schneider_dtv3);
			spaceenc_numbits	= 32;
			spaceenc_prebits	= 8;
			spaceenc_bittime	= SPACEENC2_BITTIME;
			spaceenc_bit0		= SPACEENC2_BIT0;
			spaceenc_bit1		= SPACEENC2_BIT1;
			spaceenc_waittime	= SPACEENC2_WAITTIME;
			spaceenc_codes = (unsigned int*) dtv3_codes;
			spaceenc_precodes = (unsigned char*) dtv3_pre;
			setIR(SPACEENC2_Module);
			break;
	}
}


