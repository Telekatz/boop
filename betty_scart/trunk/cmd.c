/*
    cmd.c
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
#include "cc1100.h"
#include <P89LPC932.h>
#include "cmd.h"
#include "rf.h"

unsigned char redir_dest = 0;
unsigned char redir_baud = baud_19200;


void transmit(unsigned char len) {
	
	struct cc1100frame_ header;
	
	sendWOR(0);
	EKBI = 0;
	cc1100_strobe(SIDLE);
	while (cc1100_read1(0x35 | READ | BURST) != 01);
	header.len = len + headerLen - 1;
	header.destAddr = 0;
	header.srcAddr = cc1100Addr;
	header.packetType = packet_test;
	cc1100_write(TX_fifo,(unsigned char *)&header,headerLen);
	cc1100_single(TX_fifo | BURST,0);
	while(len--) {
		cc1100_single(get_inbuf(),0);
	}
	CS = 1;
	cc1100_strobe(STX);
	EKBI = 1;

	send_string("TX Done");
}

void transmitRedir(unsigned char len, unsigned char dest) {
	
	struct cc1100frame_ header;
	
	header.len = len + headerLen - 1;
	header.destAddr = dest;
	header.srcAddr = cc1100Addr;
	header.packetType = packet_redirDAT;
	EKBI = 0;
	cc1100_write(TX_fifo,(unsigned char *)&header,headerLen);
	cc1100_single(TX_fifo | BURST,0);
	while(len--) {
		cc1100_single(get_inbuf(),0);
	}
	CS = 1;
	cc1100_strobe(STX);
	
	EKBI = 1;
}

void transmitB(void) {
	
	unsigned char l;
	unsigned char x;
	unsigned char i;
		
	while(!inbuflen);
	l=get_inbuf();
	
	if (l < 62) {
		cc1100_single(TX_fifo ,0);
		cc1100_single(l ,0);
		for(i=0; i<l; i++) {
			while(!inbuflen);
			x=get_inbuf();
			cc1100_single(x ,0 );
		}
	CS = 1;
	cc1100_strobe(STX);
	waitTX();	
	}
	else {
		send_byte(0x00);
	}

}

void print_cc1100confi(void) {
	
	unsigned char x;
	
	cc1100_single(0x00 | BURST | READ,0);
	for (x=0 ;x < 0x2F; x++) {  
		send_string("Reg 0x");
		send_hexC(x);
		send_string(" = 0x");
		send_hexC(cc1100_single(0x00,0));
		if ( (x & 3) == 3)
			send_string(crlf);
		else 
			send_string("   ");
	}
	CS = 1;
	
	send_string(crlf);
	cc1100_single(PATABLE | BURST | READ,0);
	for (x=0 ;x < 0x08; x++) {  
		send_string("PATABLE[");
		send_hexC(x);
		send_string("] = 0x");
		send_hexC(cc1100_single(0x00,0));
		if ( (x & 3) == 3)
			send_string(crlf);
		else 
			send_string("  ");
	}
	CS = 1;
}

void print_cc1100status(void) {
	send_string("CC1100 Read Status: ");
	send_hexC(cc1100_strobe(SNOP | READ));
	send_string("  Write Status: ");
	send_hexC(cc1100_strobe(SNOP | WRITE));
}

void read_byteB(void) {

	unsigned char x;
	
	x=get_inbuf_wait();
	cc1100_single(x | READ,0);
	x = cc1100_single(0,1);
	send_byte(x);
}

void write_byteB(void) {

	unsigned char x;
	
	x=get_inbuf_wait();
	cc1100_single(x | WRITE ,0);
	while (!RI);
	x = SBUF;
	RI = 0;
	cc1100_single(x,1);
}

void read_burst_byteB(void) {
	
	unsigned char l;
	unsigned char x;
	unsigned char i;
	
	l=get_inbuf_wait();
	x=get_inbuf_wait();
	
	cc1100_single(x | READ | BURST ,0);
	for(i=0; i<l; i++) {
		x= cc1100_single(0 ,0 );
		send_byte(x);
	}
	CS = 1;
	
}

void write_burst_byteB(void) {
	
	unsigned char l;
	unsigned char x;
	unsigned char i;
	
	l=get_inbuf_wait();
	x=get_inbuf_wait();
	
	cc1100_single(x | WRITE | BURST ,0);
	for(i=0; i<l; i++) {
		while (!RI);
			x = SBUF;
			RI = 0;
			cc1100_single(x ,0 );
	}
	CS = 1;
	
}

void cmd_strobeB(void) {
	
	unsigned char x;
	
	x=get_inbuf_wait();
	
	send_byte(cc1100_strobe(x));
	
}

void send_WORb(void) {
	
	unsigned char x;
	
	x=get_inbuf_wait();
	sendWOR(x);
	
}

