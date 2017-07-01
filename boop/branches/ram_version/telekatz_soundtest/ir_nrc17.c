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

#include "infrared.h"
#include "ir_nrc17.h"
#include "nrc17_codes.h"

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
	40,	// carrier 
	1,	// carrier-on
	4	// carrier-off
};


static unsigned short	nrc17_cmd;
static unsigned short	nrc17_actcmd;
static unsigned short	nrc17_startstop;
static unsigned char	nrc17_state;
static unsigned char	nrc17_bit;
static unsigned char	nrc17_numbits;
static unsigned short	nrc17_wait;
static unsigned short	nrc17_pause;

void  NRC17_Encode(void) 
{
	switch(nrc17_state)
	{
		case NRC17_IDLE:
			mod_enable = 0;
			break;

		case NRC17_PRE_A:
		case NRC17_START_A:
			cycles = NRC17_BONTIME;	// 0.5 ms
			mod_enable = 1;
			nrc17_state++;
			break;

		case NRC17_PRE_B:
		case NRC17_START_B:
			if(nrc17_state == NRC17_PRE_B)
				cycles = NRC17_PTIME;	// 2,5 ms
			else
				cycles = NRC17_BOFFTIME;	// 0.5 ms
			mod_enable = 0;
			nrc17_state++;
			break;

		case NRC17_BIT_A:
			if(nrc17_cmd & 0x0001)
				mod_enable = 1;
			else
				mod_enable = 0;
			nrc17_cmd >>= 1;
			nrc17_state++;
			break;

		case NRC17_BIT_B:
			if(mod_enable == 1)
				mod_enable = 0;
			else
				mod_enable = 1;
			nrc17_bit++;
			if(nrc17_bit == nrc17_numbits)
				nrc17_state++;
			else
				nrc17_state--;
			break;

		case NRC17_WAIT:
			mod_enable = 0;
			nrc17_wait++;
			cycles = NRC17_BONTIME;		// 1 ms
			if(nrc17_wait >= nrc17_pause)
			{
				nrc17_bit = 0;
				nrc17_wait = 0;
				nrc17_pause = NRC17_PTIME;
				nrc17_state = NRC17_IDLE;
			}
			break;

	}
}

void NRC17_CopyMap(unsigned char xtra, unsigned short *map)
{
	int x;
	for(x=0;x<42;x++)
	{
		keyMap[x] = map[x];
	}
}

void NRC17_LoadMap(unsigned char map)
{	
	switch(map)
	{
		case 0:		// nokia dbox2, 16 bit data
			NRC17_CopyMap(0x00, (unsigned short*)NRC17_dbox2);
			nrc17_startstop = NRC17_STARTSTOP_DBOX2;
			nrc17_numbits = 16;
			break;
		case 1:		// tp720 - tv, 9 bit data
			NRC17_CopyMap(0x00, (unsigned short*)NRC17_grundig_tp720);
			nrc17_startstop = NRC17_STARTSTOP_TP720;
			nrc17_numbits = 9;
			break;
		case 2 :	// tp720 - sat, 9 bit data
			NRC17_CopyMap(0x40, (unsigned short*)NRC17_grundig_tp720);
			nrc17_startstop = NRC17_STARTSTOP_TP720;
			nrc17_numbits = 9;
			break;
		default :	// tp720 - video, 9 bit data
			NRC17_CopyMap(0x80, (unsigned short*)NRC17_grundig_tp720);
			nrc17_startstop = NRC17_STARTSTOP_TP720;
			nrc17_numbits = 9;
			break;
	}
}

void NRC17_Init(unsigned char map)
{
	nrc17_state	= NRC17_IDLE;
	nrc17_cmd	= 0x0000;
	nrc17_actcmd	= 0x0000;
	nrc17_bit	= 0x00;
	nrc17_numbits	= 16;
	nrc17_wait	= 0;
	nrc17_pause	= NRC17_BONTIME;
	NRC17_LoadMap(map);
	setIR(NRC17_Module);
}

void NRC17_Send(void)
{
	nrc17_actcmd = getCode();
	if(nrc17_actcmd != 0x0000)
	{
		if(nrc17_state == NRC17_IDLE)
		{
			nrc17_pause = NRC17_WTIME;
			nrc17_cmd = nrc17_startstop;
			nrc17_state++;
			runIR();
		}
	}
}

void NRC17_Repeat(void)
{
	if(nrc17_actcmd != 0x0000)
	{
		if(nrc17_state == NRC17_IDLE)
		{
			nrc17_pause = NRC17_GTIME;
			nrc17_cmd = nrc17_actcmd;
			nrc17_state++;
		}
	}
}

void NRC17_Stop(void)
{
	if(nrc17_actcmd != 0x0000)
	{
retry:
		if(nrc17_state == NRC17_IDLE)
		{
			nrc17_cmd = nrc17_startstop;
			nrc17_state++;
		}
		else
			goto retry;
	
waitend:
		if(nrc17_state != NRC17_IDLE)
			goto waitend;
	}
	nrc17_actcmd = 0x0000;
	stopIR();
}	

