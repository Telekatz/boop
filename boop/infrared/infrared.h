/*
    infrared.h - ir carrier generation and encoding core
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

#ifndef INFRARED_H
#define INFRARED_H

volatile unsigned char hi_border;
volatile unsigned char lo_border;

typedef void(*ir_fn)(void);
typedef void(*ir_sfn)(unsigned long cmd);

typedef struct irModule
{
	ir_fn		encoder;
	ir_sfn		sender;
	ir_fn		repeater;
	ir_fn		stopper;
	unsigned char	tval;
	unsigned char	hi_border;
	unsigned char	lo_border;
} irModule;

struct IR_VARS_ {
	union {
		struct IR_GENERAL_ {		//63 Bytes
			unsigned int		header_cmd;
			unsigned int		startstop;
			unsigned int		bit;
			unsigned int		header_bit;
			unsigned int		numbits;
			unsigned int		wait;
			unsigned int		pause;
			unsigned int		leadtime;
			unsigned int		bittime0;
			unsigned int		bittime1;
			unsigned int		togglebit;
			unsigned int		len;
			unsigned int		trail;
			unsigned int		preamble;
			unsigned int		pre;
			unsigned char		prebits;
			unsigned char		waittime;
			unsigned char		bittime;
		} general;
		struct IR_LIRC_ {		//61 Bytes
			unsigned long		pre_data;			// data which the remote sends before actual keycode
			unsigned long		actpre_data;
			unsigned long		post_data;          // data which the remote sends after actual keycode
			unsigned long		actpost_data;
			unsigned short		flags;				// flags
			unsigned short		phead,shead;		// header
			unsigned short		plead;				// leading pulse
			unsigned short		ptrail;				// trailing pulse
			unsigned short		pfoot,sfoot;		// foot
			unsigned short		pre_p,pre_s;        // signal between pre_data and keycode
			unsigned short		post_p,post_s;      // signal between keycode and post_code
			unsigned short		gap;
			unsigned short		repeat_gap;
			unsigned short		prepeat,srepeat;
			unsigned short		cycles_counter;
			unsigned char		bits;				// bits (length of code)
			unsigned char		pre_data_bits;		// length of pre_data
			unsigned char		post_data_bits;		// length of post_data
			unsigned char		rc6_bit;			// doubles signal length of this bit (only used for RC-6)
			unsigned char		pthree,sthree;		// 3 (only used for RC-MM)
			unsigned char		ptwo,stwo;			// 2 (only used for RC-MM)
			unsigned char		pone,sone;			// 1
			unsigned char		pzero,szero;		// 0
			unsigned char		min_repeat;
			unsigned char		bit;
			unsigned char		map;
			unsigned char		stop;
			unsigned char		repeats;
		} lirc;
	};
	unsigned int	cmd;
	unsigned int	actcmd;
	unsigned char	toggle;
	unsigned char	state;
} ir;


//#define setIRspeed( _m ) { if(sysInfo & 0x80) T1MR0 = _m.tval1; else T1MR0 = _m.tval; }
#define setIRspeed( _m ) { T1MR0 = _m.tval; }

void __attribute__ ((section(".text.fastcode")))  defIR(void); 
void defSender(unsigned long cmd);
void defRepeater(void);
void defStopper(void);

void initIR(void);
void startIrIRQ(void);
void setIR(struct irModule module);
void runIR(void);
void stopIR(void);

unsigned long setEncoder( unsigned char _x, unsigned char _y );

void copyMapC(unsigned char *map);
void copyMapPC(unsigned int pre, unsigned char *map);
void copyMapS(unsigned short *map);
void copyMapI(unsigned int *map);

unsigned long getCode(void);

#endif

