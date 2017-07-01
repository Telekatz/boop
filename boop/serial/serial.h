/*
    serial.h - serial port functions
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

#ifndef BOOP_SERIAL_H
#define BOOP_SERIAL_H

#define	USRRxData      	(1 << 0)
#define	USRTxHoldEmpty 	(1 << 6)

#define IER_RBR		1
#define IER_THRE	2
#define IER_RXLS	4

#define IIR_RLS		0x06
#define IIR_RDA		0x04
#define IIR_CTI		0x0c
#define IIR_THRE	0x02

#define GET_CHAR(p)	p
#define PUT_CHAR(p,c)  	(p= (unsigned )(c))
#define TX_READY(s)    	((s) & USRTxHoldEmpty)
#define RX_DATA(s)     	((s) & USRRxData)

#define DEFAULTBRG	115200

struct cBuffer_* uart0_incbuffer;
struct cBuffer_* uart0_outcbuffer;

int serial_flush_input(void);
int serial_flush_output(void);
void serial_setbrg (unsigned int baudrate);
int serial_init (void);
void serial_putc (const char c);
void serial_putbyte (const char c);
int serial_tstc (void);
int serial_getc (void);
void serial_puts (const char *s);

void putHexC(const unsigned char v);
void putHexS(const unsigned short v);
void putHexW(const unsigned long v);

void serial_interactive(void);

#endif /* BOOP_SERIAL_H */
