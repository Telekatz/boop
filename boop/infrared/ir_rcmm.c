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

#include "lpc2220.h"
#include "infrared.h"
#include "ir_rcmm.h"
#include "encoders.h"
#include "codes.h"

const struct irModule RCMM_Module =
{ 
	RCMM_Encode,
	RCMM_Send,
	RCMM_Repeat,
	RCMM_Stop,
	139,	// carrier
	1,	// carrier-on
	3	// carrier-off
};

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;
extern volatile unsigned long keyMap[42];

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

const unsigned char ir_times[4] = { RCMM_BIT_00, RCMM_BIT_01, RCMM_BIT_10, RCMM_BIT_11 };

void __attribute__ ((section(".text.fastcode"))) RCMM_Encode (void)
{
	switch(ir.state)
	{
		case RCMM_IDLE:
			mod_enable = 0;
			break;

		case RCMM_HEADER_A:
		case RCMM_BIT_A:
			mod_enable = 1;
			if(ir.state == RCMM_HEADER_A)
				cycles = RCMM_HEADER_ON;
			else
				cycles = RCMM_BIT_ON;
			ir.state++;
			break;

		case RCMM_HEADER_B:
			mod_enable = 0;
			cycles = RCMM_HEADER_OFF;
			ir.state++;
			break;

		case RCMM_BIT_B:
			mod_enable = 0;
			cycles = ir_times[(ir.cmd & 0xC0000000) >> 30];
			ir.cmd <<= 2;
			ir.general.bit += 2;
			if(ir.general.bit == ir.general.len)
				ir.state++;
			else
				ir.state--;
			break;

		case RCMM_TRAIL:
			if(ir.general.trail)
			{
				mod_enable = 1;
				cycles = RCMM_BIT_ON;
			}
			else
				cycles = 1;
			ir.state++;
			break;

		case RCMM_WAIT:
			mod_enable = 0;
			ir.general.wait++;
			cycles = RCMM_BIT_ON;		// 1 ms
			if(ir.general.wait >= ir.general.pause)
			{
				ir.general.bit = 0;
				ir.general.wait = 0;
				ir.general.pause = RCMM_PTIME;
				ir.state = RCMM_IDLE;
			}
			break;

		
	}
}

void RCMM_LoadMap(unsigned char map)
{
	copyMapI((unsigned int*)RCMM.table[map].codes);
	
	switch(map)
	{
		case 1:
			ir.general.len	= 32;
			ir.general.trail	= 1;
			ir.general.togglebit = 16;
			break;
		default:
			ir.general.len	= 32;
			ir.general.trail	= 1;
			ir.general.togglebit = 0;
			break;
	}
}

void RCMM_Init(unsigned char map)
{
	if(map < RCMM.num_tables)
	{
		ir.cmd	= 0x00000000;
		ir.actcmd	= 0x00000000;
		ir.state	= RCMM_IDLE;
		ir.general.bit	= 0;
		ir.general.wait	= 0;
		ir.general.pause	= RCMM_PTIME;
		RCMM_LoadMap(map);
		setIR(RCMM_Module);
	}
}

void RCMM_Send(unsigned long cmd)
{
	ir.actcmd = cmd;
	if(ir.actcmd != 0x00000000) {	
		if(ir.general.togglebit) {
			if(ir.toggle & 0x01)
				ir.actcmd &= ~(1<<(ir.general.togglebit-1));
			else
				ir.actcmd |= 1<<(ir.general.togglebit-1);
		}
		setIRspeed(RCMM_Module);
		RCMM_Repeat();
	}
}

void RCMM_Repeat(void)
{
	if(ir.actcmd != 0x00000000)
	{
		if(ir.state == RCMM_IDLE)
		{
			ir.cmd = ir.actcmd;
			ir.state++;
			runIR();
		}
	}
}

void RCMM_Stop(void)
{
	if(ir.actcmd != 0x00000000)
	{
waitend:
		if(ir.state != RCMM_IDLE)
			goto waitend;
	}
	ir.actcmd = 0x0000;
	ir.toggle++;
	stopIR();
}


