/*
    rf.h - 
    Copyright (C) 2008  

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

#ifndef RF_H
#define RF_H

#define Received		0x01
#define WORsend			0x02

struct cc1100frame_ {
	unsigned char len;	
	unsigned char destAddr;
	unsigned char srcAddr;	
	unsigned char packetType;
	unsigned char data[];
};
#define headerLen		0x04

unsigned char RFbuf[64];

volatile unsigned char RFstatus;

void RF_IRQ (void);
void RF_init (void);
void RF_getfifo(unsigned char* b);
void RF_send(unsigned char* b);
void RF_sendWOR(unsigned char addr);
void waitTX(void);

#endif
