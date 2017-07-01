/*
    rf.h
    Copyright (C) 2009 Telekatz <telekatz@gmx.de>

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

#define packet_test			0x01
#define packet_ping			0x02
#define packet_time			0x03
#define packet_RFenc		0x04
#define packet_redirDAT		0x05
#define packet_redirCMD		0x06

extern unsigned char cc1100Addr;

struct cc1100frame_ {
	unsigned char len;	
	unsigned char destAddr;
	unsigned char srcAddr;	
	unsigned char packetType;
	unsigned char dat[];
};
#define headerLen		0x04

struct cc1100frameheader_ {
	unsigned char len;	
	unsigned char destAddr;
	unsigned char srcAddr;	
	unsigned char packetType;
};

void RF_init (void);
void waitTX(void);
void sendWOR(unsigned char addr);
void RF_isr (void) __interrupt (7);
void RF_startIRQ (void);

#endif
