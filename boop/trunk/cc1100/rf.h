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

#define WORrx_time			200

#define packet_test			0x01
#define packet_ping			0x02
#define packet_time			0x03
#define packet_RFenc		0x04
#define packet_redirDAT		0x05
#define packet_redirCMD		0x06

#define MAX_ENDPOINTS		0x0F

struct cc1100frame_ {
	unsigned char len;	
	unsigned char destAddr;
	unsigned char srcAddr;	
	unsigned char packetType;
	unsigned char data[];
};

struct RFendpoint_ {
	union {
		unsigned char* buffer;
		struct cBuffer_* cbuffer;
		unsigned char data[4];
	};
	unsigned short buffersize;
	unsigned short bufferlen;
	volatile unsigned short flags;
	unsigned char dest;
	unsigned char type;
};
//RFendpoints_.flags
#define EPenabled		(1<<0)
#define EPoutput		(1<<1)
#define EPinput			0
#define EPtypedata		(1<<2)
#define EPtypebuffer	0
#define EPtypecbuffer	(1<<3)
#define EPtypemask		(1<<2 | 1<<3)		
#define EPnewdata		(1<<4)
#define EPoverwritten	(1<<5)
#define EPoverflow		(1<<6)
#define EPonce			(1<<7)
#define EPsendwor		(1<<8)


struct RFsettings_ {
	unsigned char address;	
	unsigned char channel;
}__attribute__((aligned(0x4)));

const struct RFsettings_ default_RFsettings;

struct RF_ {
	unsigned char addr;
	volatile unsigned char flags;
	volatile unsigned char state;
	volatile unsigned char timeout_cb;
	volatile unsigned char wortimeout_cb;
	volatile unsigned char nextstate;
	volatile unsigned char tag;
}__attribute__((aligned(0x4)));
struct RF_ RF;

//RF_.state
#define RFpwrdown		0x00
#define RFwor			0x01
#define RFidle			0x02
#define RFrx			0x03
#define RFtx			0x04
#define RFsendwor		0x05
#define RFsendworend	0x06

//RF.flags
#define PingACK			(1<<0)
#define WORenabled		(1<<1)
#define RXueop			(1<<2)		//MCSM2 RX_TIME: Until end of packet

void startRFIRQ(void);
void RF_init (void);
void RF_timed_rx(unsigned short timeout);
void RF_changestate (unsigned char state);
unsigned char RF_sendping(unsigned char dest);
struct RFendpoint_* openEP(void* buffer, unsigned short buffersize, unsigned char type);
void closeEP(struct RFendpoint_ *cur_ep);
void RF_setRegister(unsigned char reg, unsigned char data);
void load_RF_setting(void);
void RFasyncmode(unsigned char on);
#endif
