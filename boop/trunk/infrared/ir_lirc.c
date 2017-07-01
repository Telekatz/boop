/*
    ir_lirc.c - lirc ir encoder
    Copyright (C) 2009  <telekatz@gmx.de>

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
#include "ir_lirc.h"
#include "encoders.h"
#include "codes.h"
#include "fonty.h"
#include "lcd.h"
#include "rf.h"
#include "cc1100.h"

#include "global.h"

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;
extern volatile unsigned long keyMap[42];

/*
#define RAW_IDLE	0x00
#define RAW_HI	0x01
#define RAW_LO	0x02
*/

#define LIRC_IDLE	        0x00
#define LIRC_HEAD_P	        0x01
#define LIRC_HEAD_S	        0x02
#define LIRC_LEAD_P	        0x03
#define LIRC_PRE_DAT_P		0x04
#define LIRC_PRE_DAT_S		0x05
#define LIRC_PRE_P			0x06
#define LIRC_PRE_S			0x07
#define LIRC_DATA_P			0x08
#define LIRC_DATA_S	        0x09
#define LIRC_POST_P			0x0A
#define LIRC_POST_S			0x0B
#define LIRC_POST_DAT_P		0x0C
#define LIRC_POST_DAT_S		0x0D
#define LIRC_REPEAT_P       0x0E
#define LIRC_REPEAT_S       0x0F
#define LIRC_TRAIL_P		0x10
#define LIRC_FOOT_S	        0x11
#define LIRC_FOOT_P	        0x12
#define LIRC_GAP			0x13

const struct irModule LIRC_Module =
{ 
	LIRC_Encode,
	LIRC_Send,
	LIRC_Repeat,
	LIRC_Stop,
	136,	// carrier
	1,	// carrier-on
	3	// carrier-off
};


unsigned char __attribute__ ((section(".text.fastcode"))) send_data (unsigned long data, unsigned char pulse) {
	
	const unsigned char* bittimes[4] = {&(ir.lirc.pzero), &(ir.lirc.pone), &(ir.lirc.ptwo), &(ir.lirc.pthree)};
	unsigned char notpulse = 1;
	
	if(pulse) {
		pulse = 1;
		notpulse = 0;
	}
	
	if(is_rcmm) {
		switch(data  & 0xC0000000) {
			case 0x00000000:
				cycles = bittimes[0][notpulse];
				break;
			case 0x40000000:
				cycles = bittimes[1][notpulse];
				break;
			case 0x80000000:
				cycles = bittimes[2][notpulse];
				break;
			case 0xC0000000:
				cycles = bittimes[3][notpulse];
				break;
		}
		mod_enable = pulse;
		if(!pulse) {
			ir.lirc.bit +=2;
			return(2);
		}
		return(0);
	}
	else if(is_biphase) {
		if(data  & 0x80000000) {
			cycles = bittimes[1][pulse];
			mod_enable = notpulse;
		}
		else {
			cycles = bittimes[0][notpulse];
			mod_enable = pulse;
		}
		if((ir.lirc.bit+1 == ir.lirc.rc6_bit))
			cycles <<= 1;
	}
	else {
		if(data  & 0x80000000) {
			cycles = bittimes[1][notpulse];
		}
		else {
			cycles = bittimes[0][notpulse];
		}
		mod_enable = pulse;
	}
	if(!pulse) {
		ir.lirc.bit++;
		return(1);
	}
	return(0);
}

void __attribute__ ((section(".text.fastcode"))) LIRC_Encode (void) {
	unsigned short gap;

	ir.lirc.cycles_counter += cycles;
	
	switch(ir.state)
	{
		case LIRC_IDLE:
			cycles = ir.lirc.pone;
			mod_enable = 0;
			break;
		case LIRC_HEAD_P:
			ir.lirc.cycles_counter = 0;
			if(has_header && (!ir.lirc.repeats || (!(ir.lirc.flags&LIRC_NO_HEAD_REP) && !has_repeat) || (ir.lirc.flags&LIRC_REPEAT_HEADER)))  {	// 
				mod_enable = 1;
				cycles = ir.lirc.phead;
				ir.state++;
				break;
		case LIRC_HEAD_S:
				mod_enable = 0;
				cycles = ir.lirc.shead;
				ir.state++;
				break;
			}
			ir.state = LIRC_LEAD_P;
		case LIRC_LEAD_P:
			if(ir.lirc.plead) {
				mod_enable = 1;
				cycles = ir.lirc.plead;
				ir.state++;
				break;
			}
			ir.state++;
		case LIRC_PRE_DAT_P:
			if(!has_repeat || !(ir.lirc.repeats)) {
				if(ir.lirc.pre_data_bits) {
					send_data(ir.lirc.pre_data,1);
					ir.state++;
					break;
		case LIRC_PRE_DAT_S:
					ir.lirc.pre_data <<= send_data(ir.lirc.pre_data,0);
					if(ir.lirc.bit >= ir.lirc.pre_data_bits)
						ir.state++;
					else
						ir.state--;
					break;		
		case LIRC_PRE_P:
					if(ir.lirc.pre_p && ir.lirc.pre_s) {
						mod_enable = 1;
						cycles = ir.lirc.pre_p;
						ir.state++;
						break;
		case LIRC_PRE_S:
						mod_enable = 0;
						cycles = ir.lirc.pre_s;
						ir.state++;
						break;
					}
				}
				ir.state = LIRC_DATA_P;	
		case LIRC_DATA_P:
				send_data(ir.cmd,1);
				ir.state++;
				break;
		case LIRC_DATA_S:
				ir.cmd <<= send_data(ir.cmd,0);
				if(ir.lirc.bit >= ir.lirc.pre_data_bits + ir.lirc.bits)
					ir.state++;
				else
					ir.state--;
				break;
		case LIRC_POST_P:
				if(ir.lirc.post_data_bits) {
					if(ir.lirc.post_p && ir.lirc.post_s) {
						mod_enable = 1;
						cycles = ir.lirc.post_p;
						ir.state++;
						break;
		case LIRC_POST_S:
						mod_enable = 0;
						cycles = ir.lirc.post_s;
						ir.state++;
						break;
					}
					ir.state = LIRC_POST_DAT_P;
		case LIRC_POST_DAT_P:
					send_data(ir.lirc.post_data,1);
					ir.state++;
					break;
		case LIRC_POST_DAT_S:
					ir.lirc.post_data <<= send_data(ir.lirc.post_data,0);
					if(ir.lirc.bit >= ir.lirc.pre_data_bits + ir.lirc.bits +ir.lirc.post_data_bits)
						ir.state = LIRC_TRAIL_P;
					else
						ir.state--;
					break;		
				}
			}
			ir.state = LIRC_REPEAT_P;
		case LIRC_REPEAT_P:
			if(has_repeat && ir.lirc.repeats) {
				mod_enable = 1;
				cycles = ir.lirc.prepeat;
				ir.state++;
				break;
		case LIRC_REPEAT_S:
				mod_enable = 0;
				cycles = ir.lirc.srepeat;
				ir.state++;
				break;
			}
			ir.state = LIRC_TRAIL_P;
		case LIRC_TRAIL_P:
			if(ir.lirc.ptrail) {
				mod_enable = 1;
				cycles = ir.lirc.ptrail;
				ir.state++;
				break;
			}
			ir.state++;
		case LIRC_FOOT_S:
			if(has_foot && (!ir.lirc.repeats || !(ir.lirc.flags&LIRC_NO_FOOT_REP)) && (!has_repeat || !ir.lirc.repeats)) {
				mod_enable = 0;
				cycles = ir.lirc.sfoot;
				ir.state++;
				break;
		case LIRC_FOOT_P:
				mod_enable = 1;
				cycles = ir.lirc.pfoot;
				ir.state++;
				break;
			}
		case LIRC_GAP:
		default:
			mod_enable = 0;
			ir.lirc.bit = 0;
			ir.cmd = ir.actcmd;
			ir.lirc.pre_data = ir.lirc.actpre_data;
			ir.lirc.post_data = ir.lirc.actpost_data;
			
			if((ir.lirc.repeat_gap && has_repeat && ir.lirc.repeats) | (is_RF && (ir.lirc.repeats >= ir.lirc.min_repeat)))
				gap = ir.lirc.repeat_gap;
			else
				gap = ir.lirc.gap;
			
			if(is_const && (ir.lirc.cycles_counter < gap))
				cycles = gap - ir.lirc.cycles_counter;
			else
				cycles = gap;
			
			if((ir.lirc.repeats >= ir.lirc.min_repeat) && ir.lirc.stop) {
				ir.state = LIRC_IDLE;
			}
			else {
				if(ir.lirc.repeats < 0xff)
					ir.lirc.repeats++;
				ir.state = LIRC_HEAD_P;
			}
	}
}

void LIRC_Init(unsigned char map)
{	
	unsigned long freq;
	struct CODE_TABLE_L *lirctable;

	if(map < LIRC.num_tables) {
		
		lirctable = (struct CODE_TABLE_L*)&(LIRC.table[map]);
		
		copyMapI((unsigned int*)LIRC.table[map].codes);
		setIR(LIRC_Module);
		
		if(lirctable->flags&LIRC_RF) {
			hi_border = 0;
			lo_border = 1;
			freq = 20000;
		}
		else {
			freq = lirctable->freq;
			if(!freq)
				freq = 38000;
			
			if(lirctable->duty_cycle == 0) {	//default 50%
				hi_border = 1;
				lo_border = 2;		
			}
			else if(lirctable->duty_cycle <= 25) {
				hi_border = 1;
				lo_border = 4;
			}
			else if(lirctable->duty_cycle <= 33) {
				hi_border = 1;
				lo_border = 3;
			}
			else if(lirctable->duty_cycle <= 50) {
				hi_border = 1;
				lo_border = 2;
			}
			else if(lirctable->duty_cycle <= 66) {
				hi_border = 2;
				lo_border = 3;
			}
			else {	//75%
				hi_border = 3;
				lo_border = 4;
			}
		}
		
		T1MR0 = 15000000 / (freq * lo_border);
		
		
		ir.lirc.phead = (lirctable->phead * freq) / 1000000;
		ir.lirc.shead = (lirctable->shead * freq) / 1000000;
		
		ir.lirc.plead = (lirctable->plead * freq) / 1000000;
				
		ir.lirc.actpre_data = (lirctable->pre_data)<<(32-lirctable->pre_data_bits);
		ir.lirc.pre_data_bits = lirctable->pre_data_bits;
		ir.lirc.pre_p = (lirctable->pre_p * freq) / 1000000;
		ir.lirc.pre_s = (lirctable->pre_s * freq) / 1000000;
		
		ir.lirc.post_p = (lirctable->post_p * freq) / 1000000;
		ir.lirc.post_s = (lirctable->post_s * freq) / 1000000;
		ir.lirc.actpost_data = (lirctable->post_data)<<(32-lirctable->post_data_bits);
		ir.lirc.post_data_bits = lirctable->post_data_bits;
		
		ir.lirc.ptrail = (lirctable->ptrail * freq) / 1000000;
		
		ir.lirc.pfoot = (lirctable->pfoot * freq) / 1000000;
		ir.lirc.sfoot = (lirctable->sfoot * freq) / 1000000;
		
		ir.lirc.prepeat = (lirctable->prepeat * freq) / 1000000;
		ir.lirc.srepeat = (lirctable->srepeat * freq) / 1000000;
		
		ir.lirc.pzero = (lirctable->pzero * freq) / 1000000;
		ir.lirc.szero = (lirctable->szero * freq) / 1000000;
		ir.lirc.pone = (lirctable->pone * freq) / 1000000;
		ir.lirc.sone = (lirctable->sone * freq) / 1000000;
		ir.lirc.ptwo = (lirctable->ptwo * freq) / 1000000;
		ir.lirc.stwo = (lirctable->stwo * freq) / 1000000;
		ir.lirc.pthree = (lirctable->pthree * freq) / 1000000;
		ir.lirc.sthree = (lirctable->sthree * freq) / 1000000;
		
		ir.lirc.gap = (lirctable->gap * freq) / 1000000;
		ir.lirc.repeat_gap = (lirctable->repeat_gap * freq) / 1000000;
		
		ir.lirc.rc6_bit = lirctable->rc6_bit;
		ir.lirc.flags = lirctable->flags;
		ir.lirc.bits = lirctable->bits;
		ir.lirc.min_repeat = lirctable->min_repeat;
		
		ir.cmd = 0;
		ir.actcmd = 0;
		ir.lirc.stop = 0;
		ir.state = LIRC_IDLE;
		ir.lirc.bit = 0;
		ir.lirc.map = map;
		
	}
}

void LIRC_Send(unsigned long cmd)
{	
	unsigned long togglemask;
	if(cmd != 0x0000) {
	
		ir.lirc.pre_data = ir.lirc.actpre_data;
		ir.actcmd = cmd<<(32-ir.lirc.bits);
		ir.lirc.post_data = ir.lirc.actpost_data;
		ir.lirc.stop = 0;
		ir.lirc.repeats = 0;
		 
		if(ir.toggle & 0x01) {
			togglemask = (unsigned long)(LIRC.table[ir.lirc.map].toggle_bit_mask) << (32-ir.lirc.post_data_bits);
			ir.lirc.post_data ^= togglemask;
			
			togglemask = (unsigned long)(LIRC.table[ir.lirc.map].toggle_bit_mask>>ir.lirc.post_data_bits) << (32-ir.lirc.bits);
			ir.actcmd ^= togglemask;
			
			togglemask = (unsigned long)(LIRC.table[ir.lirc.map].toggle_bit_mask>>(ir.lirc.post_data_bits + ir.lirc.bits)) << (32-ir.lirc.pre_data_bits);
			ir.lirc.pre_data ^= togglemask;
		}
		
		if(ir.state == LIRC_IDLE) {	
			ir.cmd = ir.actcmd;
			ir.lirc.bit = 0;
			ir.state++;
			if(is_RF) {
				RFasyncmode(true);
				cc1100_write1(FREQ2,((LIRC.table[ir.lirc.map].freq)>>16) & 0xFF);
				cc1100_write1(FREQ1,((LIRC.table[ir.lirc.map].freq)>>8) & 0xFF);
				cc1100_write1(FREQ0,(LIRC.table[ir.lirc.map].freq) & 0xFF);
				cc1100_strobe(STX);
			}
			runIR();
		}
	}
}

void LIRC_Repeat(void) {	

}

void LIRC_Stop(void)
{	
	ir.lirc.stop = 1;
    if(ir.lirc.bits){
		while(ir.state != LIRC_IDLE);
	}

	ir.toggle++;
	ir.actcmd = 0x0000;
	stopIR();
	if(is_RF) {
		RFasyncmode(false);
	}
}
