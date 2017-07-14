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
#include "pwm.h"

#include "global.h"

extern volatile unsigned char mod_enable;
extern volatile unsigned int cycles;

struct CODE_TABLE_L *lirctable;

unsigned int prev_cycles;  /*  needed for handling of b&o specific protocol added MN2017325 */

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
	
	const unsigned short* bittimes[4] = {&(lirctable->pzero), &(lirctable->pone), &(lirctable->ptwo), &(lirctable->pthree)};
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
		if((ir.lirc.bit+1 == lirctable->rc6_bit))
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
	unsigned long gap;

	ir.lirc.cycles_counter += cycles;
	
	switch(ir.state)
	{
		case LIRC_IDLE:
			cycles = lirctable->pone;
			mod_enable = 0;
			break;
		case LIRC_HEAD_P:
			ir.lirc.cycles_counter = 0;
			if(has_header && (!ir.lirc.repeats || (!(lirctable->flags&LIRC_NO_HEAD_REP) && !has_repeat) || (lirctable->flags&LIRC_REPEAT_HEADER)))  {	//
				mod_enable = 1;
				cycles = lirctable->phead;
				ir.state++;
				break;
		case LIRC_HEAD_S:
				mod_enable = 0;
				cycles = lirctable->shead;
				ir.state++;
				break;
			}
			ir.state = LIRC_LEAD_P;
		case LIRC_LEAD_P:
			if(lirctable->plead) {
				mod_enable = 1;
				cycles = lirctable->plead;
				ir.state++;
				break;
			}
			ir.state++;
		case LIRC_PRE_DAT_P:
			if(!has_repeat || !(ir.lirc.repeats)) {
				if(lirctable->pre_data_bits) {
					send_data(lirctable->pre_data,1);
					ir.state++;
					break;
		case LIRC_PRE_DAT_S:
		      ir.lirc.pre_data <<= send_data(lirctable->pre_data,0);
					if(ir.lirc.bit >= lirctable->pre_data_bits)
						ir.state++;
					else
						ir.state--;
					break;		
		case LIRC_PRE_P:
					if(lirctable->pre_p && lirctable->pre_s) {
						mod_enable = 1;
						cycles = lirctable->pre_p;
						ir.state++;
						break;
		case LIRC_PRE_S:
						mod_enable = 0;
						cycles = lirctable->pre_s;
						ir.state++;
						break;
					}
				}
				ir.state = LIRC_DATA_P;
				prev_cycles = 0;
		case LIRC_DATA_P:
				send_data(ir.cmd,1);
				ir.state++;
				break;
		case LIRC_DATA_S:
				ir.cmd <<= send_data(ir.cmd,0);
				/*  handling for b&o specific protocol added MN2017325
				special r-bit coding, if current bit is equal previous bit     */
				if(is_BO) {
					if (prev_cycles  == cycles) {
						prev_cycles = cycles;
						cycles   = lirctable->szero * 2 ;
					} else {
						prev_cycles = cycles;
					}
				}
				if(ir.lirc.bit >= lirctable->pre_data_bits + lirctable->bits)
					ir.state++;
				else
					ir.state--;
				break;
		case LIRC_POST_P:
				if(lirctable->post_data_bits) {
					if(lirctable->post_p && lirctable->post_s) {
						mod_enable = 1;
						cycles = lirctable->post_p;
						ir.state++;
						break;
		case LIRC_POST_S:
						mod_enable = 0;
						cycles = lirctable->post_s;
						ir.state++;
						break;
					}
					ir.state = LIRC_POST_DAT_P;
		case LIRC_POST_DAT_P:
					send_data(lirctable->post_data,1);
					ir.state++;
					break;
		case LIRC_POST_DAT_S:
		  ir.lirc.post_data <<= send_data(lirctable->post_data,0);
					if(ir.lirc.bit >= lirctable->pre_data_bits + lirctable->bits + lirctable->post_data_bits)
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
				cycles = lirctable->prepeat;
				ir.state++;
				break;
		case LIRC_REPEAT_S:
				mod_enable = 0;
				cycles = lirctable->srepeat;
				ir.state++;
				break;
			}
			ir.state = LIRC_TRAIL_P;
		case LIRC_TRAIL_P:
			if(lirctable->ptrail) {
				mod_enable = 1;
				cycles = lirctable->ptrail;
				ir.state++;
				break;
			}
			ir.state++;
		case LIRC_FOOT_S:
			if(has_foot && (!ir.lirc.repeats || !(lirctable->flags&LIRC_NO_FOOT_REP)) && (!has_repeat || !ir.lirc.repeats)) {
				mod_enable = 0;
				cycles = lirctable->sfoot;
				ir.state++;
				break;
		case LIRC_FOOT_P:
				mod_enable = 1;
				cycles = lirctable->pfoot;
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
			
			if((lirctable->repeat_gap && has_repeat && ir.lirc.repeats) | (is_RF && (ir.lirc.repeats >= lirctable->min_repeat)))
				gap = lirctable->repeat_gap;
			else
				gap = lirctable->gap;
			
			if(is_const && (ir.lirc.cycles_counter < gap))
				cycles = gap - ir.lirc.cycles_counter;
			else
				cycles = gap;
			
			if((ir.lirc.repeats >= lirctable->min_repeat) && ir.lirc.stop) {
				ir.state = LIRC_IDLE;
			}
			else {
				if(ir.lirc.repeats < 0xff)
					ir.lirc.repeats++;
				ir.state = LIRC_HEAD_P;
			}
	}


	T1MR0 = 15 * cycles;

}

void LIRC_Init(unsigned char map)
{	
	unsigned long freq;

	if(map < LIRC.num_tables) {
		
		lirctable = (struct CODE_TABLE_L*)&(LIRC.table[map]);
		
		copyMapI((unsigned int*)LIRC.table[map].codes);
		setIR(LIRC_Module);
		
		if(lirctable->flags&LIRC_RF) {
		  ir.duty_cycle = 50;
			freq = 20000;
		}
		else {
			freq = lirctable->freq;
			if(!freq)
				freq = 38000;

			ir.duty_cycle = lirctable->duty_cycle;
			if(!lirctable->duty_cycle) {	//default 50%
				ir.duty_cycle = 50;
			}
		}
		
		PWM_set_frequency(freq);

		T1MR0 = 15000000 / (freq);
		
		ir.lirc.actpre_data = (lirctable->pre_data)<<(32-lirctable->pre_data_bits);
		ir.lirc.actpost_data = (lirctable->post_data)<<(32-lirctable->post_data_bits);

		ir.cmd = 0;
		ir.actcmd = 0;
		ir.lirc.stop = 0;
		ir.state = LIRC_IDLE;
		ir.lirc.bit = 0;
		
	}
}

void LIRC_Send(unsigned long cmd)
{	
	unsigned long togglemask;
	if(cmd != 0x0000) {
	
		ir.lirc.pre_data = ir.lirc.actpre_data;
		ir.actcmd = cmd<<(32-lirctable->bits);
		ir.lirc.post_data = ir.lirc.actpost_data;
		ir.lirc.stop = 0;
		ir.lirc.repeats = 0;
		 
		if(ir.toggle & 0x01) {
			togglemask = (unsigned long)(lirctable->toggle_bit_mask) << (32-lirctable->post_data_bits);
			ir.lirc.post_data ^= togglemask;
			
			togglemask = (unsigned long)(lirctable->toggle_bit_mask>>lirctable->post_data_bits) << (32-lirctable->bits);
			ir.actcmd ^= togglemask;
			
			togglemask = (unsigned long)(lirctable->toggle_bit_mask>>(lirctable->post_data_bits + lirctable->bits)) << (32-lirctable->pre_data_bits);
			ir.lirc.pre_data ^= togglemask;
		}
		
		if(ir.state == LIRC_IDLE) {	
			ir.cmd = ir.actcmd;
			ir.lirc.bit = 0;
			ir.state++;
			if(is_RF) {
				RFasyncmode(true);
				cc1100_write1(FREQ2,((lirctable->freq)>>16) & 0xFF);
				cc1100_write1(FREQ1,((lirctable->freq)>>8) & 0xFF);
				cc1100_write1(FREQ0,(lirctable->freq) & 0xFF);
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
	if(lirctable->bits){
		while(ir.state != LIRC_IDLE);
	}

	ir.toggle++;
	ir.actcmd = 0x0000;
	stopIR();
	if(is_RF) {
		RFasyncmode(false);
	}
}
