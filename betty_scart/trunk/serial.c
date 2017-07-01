/*
    serial.c
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

#include "serial.h"
#include "cmd.h"
#include <P89LPC932.h>

unsigned char inbufindex = 0;
volatile unsigned char inbuflen = 0;
idata unsigned char inbuf[64];

unsigned char outbufindex;
volatile unsigned char outbuflen;
idata unsigned char outbuf[64];

code unsigned char crlf[] = { 0x0d, 0x0a, 0x00 };

const unsigned char baudtable[10][2] = {	{45, 27},		//1200
											{22,133},		//2400
											{11, 59},		//4800
											{ 5,149},		//9600
											{ 3,180},		//14400
											{ 2,195},		//19200
											{ 1,210},		//28800
											{ 1, 89},		//38400
											{ 0,225},		//57600
											{ 0,104}};		//115200


void setbaud(unsigned char baud) {
	
	BRGR0 = baudtable[baud][1];
	BRGR1 = baudtable[baud][0];
	
}

void initSerial(void) {
	SCON   = (1<<4) | (1<<6) | (1<<1);	// Mode1; Enable serial reception
	SSTAT |= (1<<7) | (1<<5);							
	setbaud(baud_default);
	BRGCON = 0x03;
	RI = 0;
	ESR = 1;								//Enable serial receive interrupt
	TI = 1;	
	PSR = 1;								//Serial receive interrupt priority level 1
	PST = 1;								//Serial transmit interrupt priority level 1
	
}

void send_string(const unsigned char* string) {
 
	unsigned char i=0;
	
	while (string[i] != 0) {
		send_byte(string[i++]);
	} 
}

void send_bytes(unsigned char* h, unsigned char l) {
 
	unsigned char i=0;
	
	while (i<l) {
		send_byte(h[i++]);
	} 
}

void send_byte(unsigned char h) {
	
	while(!put_outbuf(h))
		EST = 1;
	EST = 1;
}

void send_hexC(unsigned char c) {
	 
	unsigned char cn;
	
	cn = (c>>4) & 0x0f;
	cn += 0x30;
	if (cn > 0x39)
		cn += 0x07;
	send_byte(cn);
	
	cn = c & 0x0f;
	cn += 0x30;
	if (cn > 0x39)
		cn += 0x07;
	send_byte(cn);
} 

void serial_in_isr (void) __interrupt (4) {
	if (inbuflen < 64) {
		put_inbuf(SBUF);
	}
	RI = 0;
	TF0 = 0;
	TH0 = 0;
}

void serial_out_isr (void) __interrupt (13) {
	if (outbuflen) {
		SBUF = get_outbuf();
		TI=0;
	}
	else {
		EST = 0;
	}	
}

unsigned char get_inbuf(void) {
	unsigned char x = 0;
	EA = 0;
	if(inbuflen) {
		inbuflen--;
		x = inbuf[(inbufindex++) & 0x3F];
	}
	EA = 1;
	return(x);
}

unsigned char get_outbuf(void) {
	unsigned char x = 0;
	
	EA = 0;
	if(outbuflen) {
		outbuflen--;
		x = outbuf[(outbufindex++) & 0x3F];
	}
	EA = 1;
	return(x);
}

unsigned char get_inbuf_wait(void) {
	unsigned char x;
	
	while(!inbuflen);
	EA = 0;
	inbuflen--;
	x = inbuf[(inbufindex++) & 0x3F];
	EA = 1;
	return(x);
}

unsigned char get_inbuf_index(unsigned char index) {
	return(inbuf[(inbufindex + index) & 0x3F]);
}

void put_inbuf(unsigned char dat) {
	if (inbuflen < 64)
		inbuf[(inbufindex + inbuflen++) & 0x3F] = dat;
}

unsigned char put_outbuf(unsigned char dat) {
	EA = 0;
	if (outbuflen < 64) {
		outbuf[(outbufindex + outbuflen++) & 0x3F] = dat;
		EA = 1;
		return 1;
	}
	EA = 1;
	return 0;
}
