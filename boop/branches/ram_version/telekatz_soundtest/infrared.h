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

typedef void(*ir_fn)(void);

extern ir_fn	irSend;
extern ir_fn	irRepeat;
extern ir_fn	irStop;

typedef struct irModule
{
	ir_fn		encoder;
	ir_fn		sender;
	ir_fn		repeater;
	ir_fn		stopper;
	int		tval;
	unsigned int	hi_border;
	unsigned int	lo_border;
} irModule;

extern unsigned char mod_enable;
extern unsigned char hi_border;
extern unsigned char lo_border;
extern unsigned int cycles;
extern unsigned long keyMap[42];

void   defIR(void); 
void defSender(void);
void defRepeater(void);
void defStopper(void);

void irIRQ (void)  __attribute__ ((interrupt("IRQ"))) ;

void initIR(void);
void startIrIRQ(void);
void setIR(struct irModule module);
void runIR(void);
void stopIR(void);
unsigned long getCode(void);

#endif

