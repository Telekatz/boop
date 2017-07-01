/*
    rf.c - 
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

#include "lpc2220.h"
#include "rf.h"
#include "cc1100.h"
#include "timerfuncs.h"
#include "irq.h"
#include "cc1100.h"
#include "lcd.h"
#include "fonty.h"
#include "rtc.h"
#include "global.h"
#include "buffer.h"
#include "bfs.h"

unsigned char RFbuf[64];
struct RFendpoint_ rfendpoint[MAX_ENDPOINTS];
struct RF_ RF;

const struct RFsettings_ default_RFsettings =
{
	0x01,	//address
	0x01	//channel
};

//*********************
//* Private functions *
//*********************

void switch_to_idle() {
	cc1100_strobe(SIDLE);
	while (cc1100_read1(MARCSTATE) != 01);
}

void WORsend_timeout(unsigned int cb) {
	RF.state = RFsendworend;
}

void RF_timed_rx_timeout(unsigned int cb) {
	stopCB(RF.timeout_cb);
	removeTimerCB(RF.timeout_cb);
	RF.timeout_cb = 0xff;
	if(RF.flags & WORenabled)
		RF_changestate(RFwor);
	else
		RF_changestate(RFidle);

}

void sendendpoint(struct RFendpoint_ *currentEP) {

	unsigned char* epdata = 0;
	unsigned short eplen;
	struct cc1100frame_ *TXframe;
	
	TXframe = (struct cc1100frame_ *)RFbuf;
	
	//write header
	eplen = currentEP->bufferlen;
	switch ( currentEP->flags & EPtypemask) {
		case EPtypedata:
			if(eplen > 6)
				eplen = 6;
			epdata = currentEP->data;
			break;
		case EPtypebuffer:
			if(eplen > 59)
				eplen = 59;
			epdata = currentEP->buffer;
			break;
		case EPtypecbuffer:
			if(eplen > 59)
				eplen = 59;
			if(eplen > currentEP->cbuffer->len)
				eplen = currentEP->cbuffer->len;
			break;
	}
	TXframe->len = eplen + 3;
	TXframe->destAddr = currentEP->dest;
	TXframe->srcAddr = RF.addr;	
	TXframe->packetType = currentEP->type;
	cc1100_write(TX_fifo | BURST,(unsigned char*)TXframe,4);
	
	//write data
	switch ( currentEP->flags & EPtypemask) {
		case EPtypedata:
		case EPtypebuffer:
			cc1100_write(TX_fifo | BURST,epdata,eplen);
			break;
		case EPtypecbuffer:
			
			if(currentEP->cbuffer->index + eplen <= currentEP->cbuffer->size) {
				cc1100_write(TX_fifo | BURST,&(currentEP->cbuffer->data[currentEP->cbuffer->index]),eplen);
			}
			else {
				cc1100_write(TX_fifo | BURST,&(currentEP->cbuffer->data[currentEP->cbuffer->index]), currentEP->cbuffer->size - currentEP->cbuffer->index);
				cc1100_write(TX_fifo | BURST,currentEP->cbuffer->data,eplen - (currentEP->cbuffer->size - currentEP->cbuffer->index));
			}
			currentEP->cbuffer->index = (currentEP->cbuffer->index + eplen) % currentEP->cbuffer->size;
			currentEP->cbuffer->len -= eplen;
			break;
	}
	
	cc1100_strobe(STX);
	
	currentEP->flags &= ~EPnewdata;
	
	if(currentEP->flags & EPonce)
		closeEP(currentEP);
}

void receiveendpoint(struct RFendpoint_ *currentEP) {
	
	unsigned short eplen;
	struct cc1100frame_ *RXframe;

	RXframe = (struct cc1100frame_ *)RFbuf;
		
	currentEP->dest = RXframe->srcAddr;
	eplen = RXframe->len-3;
	
	switch ( currentEP->flags & EPtypemask) {
		case EPtypedata:
			if(eplen > 6) {
				eplen = 6;
				currentEP->flags |= EPoverflow;
			}
			memcpy(&(currentEP->data[0]),RXframe->data,eplen);
			break;
		case EPtypebuffer:
			if(eplen > currentEP->buffersize) {
				eplen = currentEP->buffersize;
				currentEP->flags |= EPoverflow;
			}
			memcpy(currentEP->buffer,RXframe->data,eplen);
			break;
		case EPtypecbuffer:
			if(eplen > currentEP->cbuffer->size - currentEP->cbuffer->len) {
				eplen = currentEP->cbuffer->size - currentEP->cbuffer->len;
				currentEP->flags |= EPoverflow;
			}
			cBufferAddBuffer(currentEP->cbuffer, RXframe->data, eplen);
			break;
	}
	
	currentEP->bufferlen = eplen;
	if(currentEP->flags & EPnewdata)
		currentEP->flags |= EPoverwritten;
	else
		currentEP->flags |= EPnewdata;
	if(currentEP->flags & EPonce)
		closeEP(currentEP);
}

void cc1100IRQ (void) {
	
	unsigned char x;
	
	switch_to_idle();
	
	if (cc1100_read1(RXBYTES) > 0) {
		cc1100_read(RX_fifo, RFbuf,1);
		cc1100_read(RX_fifo, &RFbuf[1],RFbuf[0]);
		
		struct cc1100frame_ *RXframe;
		RXframe = (struct cc1100frame_ *)RFbuf;
		
		if ((RXframe->len == 1) && (RF.flags & WORenabled) && ((RXframe->destAddr == RF.addr)  || (RXframe->destAddr == 0x00))) {
			if(RF.timeout_cb != 0xff || RF.state != RFrx)
				RF_timed_rx(WORrx_time);
		}
			
		if (RXframe->len > 3) {
			if (RF.timeout_cb != 0xff)
				setCBIntervall(RF.timeout_cb,WORrx_time);
			switch ( RXframe->packetType) {
				case packet_test:
					draw_block(0,50,128,10,3,DRAW_ERASE);
					draw_string(0, 50, (char*)&RXframe->data[0], 3, DRAW_PUT);
					break;
				case packet_ping:
					if (RXframe->data[0] == 0x01) {
						struct RFendpoint_* cur_ep;
						cur_ep = openEP(0,0, packet_ping);
						if(cur_ep) {
							cur_ep->dest = RXframe->srcAddr;
							cur_ep->data[0] = 2;
							cur_ep->bufferlen = 1;
							cur_ep->flags |= EPenabled | EPoutput | EPnewdata | EPonce;
							RF_changestate(RFtx);
						}
					}
					else if (RXframe->data[0] == 0x02) {
						RF.flags |= PingACK;
					}
					break;
				case packet_time:
					if ((RXframe->data[0] == 0xFF) && (timeInfo & TIME_ACCURATE)) {
						send_time(RXframe->srcAddr,0);
					}
					else if (RXframe->len == 10) {
						memcpy((unsigned char *)&rtc,&RXframe->data[0],7);
						timeInfo |= SETNEWTIME;
						VICSoftInt = INT_EINT2;
					}
					break;
				default:
					for(x=0; x<MAX_ENDPOINTS; x++) {
						if((rfendpoint[x].type == RXframe->packetType) && ((rfendpoint[x].flags & (EPoutput | EPenabled)) == EPenabled)) {
							receiveendpoint(&rfendpoint[x]);
							break;
						}
					}
			}
		}
	}
	
	if(RF.state == RFtx) {
		for(x=0; x<MAX_ENDPOINTS; x++) {
			if((rfendpoint[x].type) && ((rfendpoint[x].flags & (EPoutput | EPnewdata | EPenabled)) == (EPoutput | EPnewdata | EPenabled))) {
				if(rfendpoint[x].flags & EPsendwor) {
					rfendpoint[x].flags &= ~EPsendwor;
					cc1100_write1(MCSM0,conf[MCSM0] & 0xCF);
					cc1100_strobe(SCAL);
					while (cc1100_read1(MARCSTATE) != 01);
					RF.wortimeout_cb = addTimerCB(WORsend_timeout, 101);
					startCB(RF.wortimeout_cb);
					RF.state = RFsendwor;
					RF.tag = x;
				}
				else
					sendendpoint(&rfendpoint[x]);
				break;
			}
		}
		if(x == MAX_ENDPOINTS) {
			RF.state = RF.nextstate;
		}
	}

	if(RF.state == RFsendwor) {
		unsigned char b[2];
		b[0]=0x01;
		b[1]=rfendpoint[RF.tag].dest;
		cc1100_write(TX_fifo | BURST,b,2);
		cc1100_strobe(STX);
	}

	if(RF.state == RFsendworend) {
		stopCB(RF.wortimeout_cb);
		removeTimerCB(RF.wortimeout_cb);
		RF.wortimeout_cb = 0xff;
		
		cc1100_write1(MCSM0,conf[MCSM0]);
		RF.state = RFtx;
		sendendpoint(&rfendpoint[RF.tag]);
	}

	switch (RF.state) {
		case RFpwrdown:
			cc1100_strobe(SPWD);
			break;
		case RFwor:
			if(!(RF.flags & RXueop)) {
				cc1100_write1(0x16,conf[0x16]);
				RF.flags |= RXueop;
			}
			cc1100_strobe(SWOR);
			break;
		case RFidle:
			break;
		case RFrx:
			if(RF.flags & RXueop) {
				cc1100_write1(0x16,0x07);
				RF.flags &= ~RXueop;
			}
			cc1100_strobe(SFRX);
			cc1100_strobe(SRX);
			break;
		case RFtx:
		case RFsendwor:
			break;
	}
	
	if(RF.state <= RFidle)
		symbols &= ~symbolRF;
	else
		symbols |= symbolRF;
	
	VICSoftIntClr = INT_EINT0;
	EXTINT = 0x01;
}

//********************
//* Public functions *
//********************

unsigned char RF_sendping(unsigned char dest) {
	struct RFendpoint_* cur_ep;
	
	cur_ep = openEP(0,0, packet_ping);
	if(cur_ep) {
		cur_ep->dest = dest;
		cur_ep->data[0] = 1;
		cur_ep->bufferlen = 1;
		cur_ep->flags |= EPenabled | EPoutput | EPnewdata | EPonce | EPsendwor;
		
		RF.flags &= ~PingACK;
		RF_changestate(RFtx);
			
		RF_timed_rx(WORrx_time);
		
		while ((RF.timeout_cb != 0xff) && !(RF.flags & PingACK));
		
		if(RF.flags & PingACK)
			return(1);
	}
	return(0);
}

void RF_init (void) {
	cc1100_init();
	RF.flags = 0;
	RF.addr = conf[0x09];
	RF.state = RFpwrdown;
	RF.nextstate = RFpwrdown;
	RF.timeout_cb = 0xff;
}

void RF_changestate (unsigned char state) {
	unsigned char oldstate;
	
	VICIntEnClr = INT_EINT0;
	
	if(RF.state != state) {
		oldstate = RF.state;
		
		if((RF.state >= RFtx) && (state < RFtx)) {
			RF.nextstate = state;
		}
		else {
			switch (state) {
				case RFpwrdown:
					RF.state = RFpwrdown;
					break;
				case RFwor:
					RF.state = RFwor;
					RF.flags |= WORenabled;
					break;
				case RFidle:
					RF.state = RFidle;
					break;
				case RFrx:
					RF.state = RFrx;
					break;
				case RFtx:
					if(RF.state < RFtx)
						RF.state = RFtx;
					if(oldstate == RFrx)
						RF.nextstate = RFrx;
					else if(RF.flags & WORenabled)
						RF.nextstate = RFwor;
					else
						RF.nextstate = RFidle;
					break;
				case RFsendwor:
					break;
			}
		}
		if(oldstate != RF.state) {
			VICSoftInt = INT_EINT0;
		}		
	}
	VICIntEnable = INT_EINT0;
}

void RF_timed_rx(unsigned short timeout) {
	if (RF.timeout_cb != 0xff)
		setCBIntervall(RF.timeout_cb,timeout);
	else {
		RF.timeout_cb = addTimerCB(RF_timed_rx_timeout, timeout);
		startCB(RF.timeout_cb);
		RF_changestate(RFrx);
	}
}

void startRFIRQ(void) {
	
	PINSEL1 |= 1;		//GDO0 as EINT0
	EXTMODE |= 1;		//edge sensitive
	EXTPOLAR |= 0;		//falling edge	
	EXTWAKE |= 1;
	EXTINT |= 1;
	
	VICVectAddr2 = (unsigned long)&(cc1100IRQ);
	VICVectCntl2 = VIC_SLOT_EN | INT_SRC_EINT0;
	VICIntEnable = INT_EINT0;
}

inline void stopRFIRQ(void) {
	VICIntEnClr = INT_EINT0;
}


struct RFendpoint_* openEP(void* buffer, unsigned short buffersize, unsigned char type) {
	
	int i;
	struct RFendpoint_* cur_ep;

	if(!type)
		return 0;
	
	for(i=0; i<MAX_ENDPOINTS;i++) {
		cur_ep = &rfendpoint[i];
		if(!(cur_ep->type)) {
			cur_ep->buffer = buffer;
			cur_ep->buffersize = buffersize;
			cur_ep->type = type;
			cur_ep->bufferlen = 0;
			cur_ep->dest = 0;
			cur_ep->flags = 0;
			if(!(buffer))
				cur_ep->flags |= EPtypedata;
			return cur_ep;
		}
	}
	return 0;

}

void closeEP(struct RFendpoint_ *cur_ep) {
	if(cur_ep) {
		cur_ep->flags = 0;
		cur_ep->buffer = 0;
		cur_ep->type = 0;
	}
}

void RF_setRegister(unsigned char reg, unsigned char data) {
	
	unsigned long VICInt;
	
	VICInt = VICIntEnable & INT_EINT0;
	VICIntEnClr = INT_EINT0;
	
	cc1100_write1(reg,data);
	
	VICIntEnable |= VICInt;
}

void load_RF_setting(void) {
	struct RFsettings_ RFsettings;
	
	if(BFS_LoadFile(BFS_ID_RFsettings, sizeof(struct RFsettings_), (unsigned char*) &RFsettings) != sizeof(struct RFsettings_)) {
		RFsettings = default_RFsettings;
		BFS_SaveFile(BFS_ID_RFsettings, sizeof(struct RFsettings_), (unsigned char*) &RFsettings);
		draw_string(0, 50, "default RF load", 3, DRAW_PUT);
	}
	
	RF_setRegister(0x09, RFsettings.address);
	RF_setRegister(0x0a, RFsettings.channel);
	RF.addr = RFsettings.address;
}


void RFasyncmode(unsigned char on) {

	if(on) {
		RF_changestate(RFidle);
		while(RF.state != RFidle);
		stopRFIRQ();
		cc1100_write((0x00 | BURST ),(unsigned char*)confasync,0x2f);
		cc1100_write1(PATABLE,0xf0);
		PINSEL1 &= 0xfffffffc;			// GDO0 as GPIO
		FIODIR0 |= GDO0;				// output
	}
	else {
		PINSEL1 |= 1;					// GDO0 as EINT0
		cc1100_write((0x00 | BURST ),(unsigned char*)conf,0x2f);
		cc1100_write1(PATABLE,0xC0);
		cc1100_strobe(SIDLE);
		load_RF_setting();
		startRFIRQ();
		RF_changestate(RFwor);
	}
}
