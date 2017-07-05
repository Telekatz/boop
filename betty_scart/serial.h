/*
    serial.h
    Copyright (C) 2007  

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

#ifndef serial_H
#define serial_H

#define CTS				P0_4

#define baud_1200		0
#define baud_2400		1
#define baud_4800		2
#define baud_9600		3
#define baud_14400		4
#define baud_19200		5
#define baud_28800		6
#define baud_38400		7
#define baud_57600		8
#define baud_115200		9

#define baud_default	baud_38400

__sbit __at (0xFE)    PST;

extern code unsigned char crlf[];
extern unsigned char inbufindex;
extern volatile unsigned char inbuflen;
extern idata unsigned char inbuf[64];

extern unsigned char outbufindex;
extern volatile unsigned char outbuflen;
extern idata unsigned char outbuf[64];

void setbaud(unsigned char baud);
void initSerial(void);
void send_string(const unsigned char* string);
void send_hexC(unsigned char c);
void send_bytes(unsigned char* h, unsigned char l);
void send_byte(unsigned char h);
void serial_in_isr (void) __interrupt (4);
void serial_out_isr (void) __interrupt (13);
unsigned char get_inbuf(void) ;
unsigned char get_outbuf(void);
unsigned char get_inbuf_wait(void);
unsigned char get_inbuf_index(unsigned char index);
void put_inbuf(unsigned char dat);
unsigned char put_outbuf(unsigned char dat);

#endif
