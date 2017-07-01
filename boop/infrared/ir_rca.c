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

#include "lpc2220.h"
#include "infrared.h"
#include "ir_rca.h"
#include "encoders.h"
#include "codes.h"
#include "global.h"

const struct irModule RCA_Module =
{ 
	RCA_Encode,
	RCA_Send,
	RCA_Repeat,
	RCA_Stop,
	134,	// carrier
	1,	// carrier-on
	2	// carrier-off
};

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;
extern volatile unsigned long keyMap[42];

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
#define RCA_WAITTIME	 16

void __attribute__ ((section(".text.fastcode"))) RCA_Encode (void)
{
	switch(ir.state)
	{
		case RCA_IDLE:
			mod_enable = 0;
			break;

		case RCA_PRE_A:
			cycles = RCA_PRETIME;
			mod_enable = 1;
			ir.state++;
			break;

		case RCA_PRE_B:
			mod_enable = 0;
			ir.cmd = ir.actcmd;
			ir.state++;
			break;

		case RCA_BIT_A:
		case RCA_IBIT_A:
		case RCA_STOP:
			cycles = RCA_BITTIME;
			mod_enable = 1;
			ir.state++;
			break;

		case RCA_BIT_B:
		case RCA_IBIT_B:
			mod_enable = 0;
			if(ir.cmd & 0x0800)
				cycles = RCA_BIT1;
			else
				cycles = RCA_BIT0;
			ir.cmd <<= 1;
			ir.general.bit++;
			if(ir.general.bit == RCA_NUMBITS)
			{
				if(ir.state == RCA_BIT_B)
				{
					ir.cmd = ~ir.actcmd;
					ir.general.bit = 0;
				}
				ir.state++;
			}
			else
				ir.state--;
			break;

		case RCA_WAIT:
			mod_enable = 0;
			cycles = RCA_BITTIME;
			ir.general.wait++;
			if(ir.general.wait >= RCA_WAITTIME)
			{
				ir.general.bit = 0;
				ir.general.wait = 0;
				if (ir.general.trail)
					ir.state = RCA_PRE_A;
				else
					ir.state = RCA_IDLE;
			}
			break;
	}
}

void RCA_LoadMap(unsigned char map)
{
	switch(map)
	{
		default:
			copyMapPC(0x0500, (unsigned char*)RCA.table[map].codes);
			break;
	}
}
void RCA_Init(unsigned char map)
{
	if(map < RCA.num_tables)
	{
		ir.state		= RCA_IDLE;
		ir.cmd			= 0x0000;
		ir.actcmd		= 0x0000;
		ir.general.bit			= 0x00;
		ir.general.wait		= 0;
		RCA_LoadMap(map);
		setIR(RCA_Module);
		cycles = RCA_BITTIME;
	}
}

void RCA_Send(unsigned long cmd)
{
	ir.actcmd = cmd;
	if(ir.actcmd != 0x0000) {
		setIRspeed(RCA_Module);
		ir.general.trail = 0x01;
		sysInfo |= SYS_IR;
		if (ir.state != RCA_IDLE)
			while(ir.state != RCA_IDLE);
		ir.state = RCA_PRE_A;
		runIR();
	}
}

void RCA_Repeat(void)
{
	if(ir.actcmd != 0x0000)
	{
		
		if(ir.state == RCA_IDLE)
		{
			//ir.state++;
			//runIR();
		}
	}
}

void RCA_Stop(void)
{	
	if(ir.actcmd != 0x00000000)
	{
		ir.general.trail = 0;
		sysInfo &= ~SYS_IR;
waitend:
		if(ir.state != RCA_IDLE)
			goto waitend;
	}
	stopIR();
}


