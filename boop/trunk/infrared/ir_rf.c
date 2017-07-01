/*
    ir_rf.c - RF IR encoder
    Copyright (C) 2008  <telekatz@gmx.de>

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
#include "ir_rf.h"
#include "encoders.h"
#include "codes.h"
#include "rf.h"

const struct irModule IRRF_Module =
{ 
	IRRF_Encode,
	IRRF_Send,
	IRRF_Repeat,
	IRRF_Stop,
	75,	// carrier turbo mode 
	1,	// carrier-on
	4	// carrier-off
};

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;
extern volatile unsigned long keyMap[42];

#define IRRF_IDLE	0x00
#define IRRF_WAIT	0x01

#define IRRF_BITTIME	 40
#define IRRF_WAITTIME	125

void __attribute__ ((section(".text.fastcode"))) IRRF_Encode (void)
{
	switch(ir.state)
	{
		case IRRF_IDLE:
			mod_enable = 0;
			break;

		case IRRF_WAIT:
			mod_enable = 0;
			ir.general.wait++;
			if(ir.general.wait >= IRRF_WAITTIME)
			{
				ir.general.bit = 0;
				ir.general.wait = 0;
				ir.state = IRRF_IDLE;
			}
			break;
	}
}

void IRRF_Init(unsigned char map)
{
	if(map < IRRF.num_tables)
	{
		ir.state	= IRRF_IDLE;
		ir.cmd		= 0x0000;
		ir.actcmd	= 0x0000;
		ir.general.bit		= 0x00;
		ir.general.wait	= 0;
		copyMapS((unsigned short*)IRRF.table[map].codes);
		setIR(IRRF_Module);
		cycles = IRRF_BITTIME;
	}
}

void IRRF_Send(unsigned long cmd)
{
	ir.actcmd = cmd;
	ir.general.trail=(unsigned int)openEP(0,0, packet_RFenc);
	setIRspeed(IRRF_Module);

	if((ir.toggle & 0x01) &&(ir.actcmd != 0x0000))
		ir.actcmd |= 0x0080;
	IRRF_Repeat();
}

void IRRF_Repeat(void)
{
	if(ir.actcmd != 0x0000)
	{
		if(ir.state == IRRF_IDLE)
		{
			struct RFendpoint_* cur_ep;
			cur_ep = (struct RFendpoint_*)ir.general.trail;
			if((cur_ep) && !(cur_ep->flags & EPnewdata)) {
				cur_ep->dest = (ir.actcmd & 0xff00) >> 8;
				cur_ep->data[0] = (ir.actcmd & 0x00ff);
				cur_ep->bufferlen = 1;
				cur_ep->flags |= EPenabled | EPoutput | EPnewdata;
				
				RF_changestate(RFtx);
			}
			ir.state++;
			runIR();
		}
	}
}

void IRRF_Stop(void)
{
	if(ir.actcmd != 0x00000000)
		while(ir.state != IRRF_IDLE);
	
	closeEP((struct RFendpoint_*)ir.general.trail);
	ir.toggle++;
	ir.actcmd = 0x0000;
	stopIR();
}


