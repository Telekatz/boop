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
			unsigned long		cycles_counter;
			unsigned char		bit;
			unsigned char		stop;
			unsigned char		repeats;
		} lirc;
	};
	unsigned int	cmd;
	unsigned int	actcmd;
	unsigned char	toggle;
	unsigned char	state;
	unsigned char duty_cycle;
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

