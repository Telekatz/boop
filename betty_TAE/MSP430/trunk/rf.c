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

#include "cc1100.h"
#include "hardware.h"
#include "rf.h"
#include <msp430.h>
#include <signal.h>
//#include <mspgcc/util.h> 

unsigned char RFbuf[64];
volatile unsigned char RFstatus;
unsigned char cc1100Addr;

void switch_to_idle(void) {
	cc1100_strobe(SIDLE);
	while (cc1100_read1(0x35 | READ | BURST) != 01);
}

void RF_init (void) {
	cc1100_init();
	RFstatus = 0;
	cc1100Addr = conf[0x09];
	cc1100_strobe(SRX);
	P1IE  = CC_MISO;
	
}

void waitTX(void)
{
	unsigned char status;
	unsigned char x;
	
	x=1;
	while (x) {
		status = cc1100_strobe(SNOP);
		switch ( 0xf0 & status) {
			case 0x70:
					cc1100_strobe(SFTX);
				break;
			case  0x10:
				if (RFstatus & WORsend)
					cc1100_strobe(SIDLE);
				x=0;
				break;
			case  0x00:
				if (!(RFstatus & WORsend))
					cc1100_strobe(SRX);
				x=0;
				break;
		}	
	}
}

void RF_send(unsigned char* b) {
	
	unsigned char P1IE_state;
	
	P1IE_state = (P1IE & CC_MISO);
	P1IE  &= ~CC_MISO;

	cc1100_write(TX_fifo | BURST,b,b[0]+1);
	cc1100_strobe(STX);
	waitTX();
	P1IE  |= P1IE_state;
}

void RF_getfifo(unsigned char* b) {

	cc1100_read(RX_fifo, b,1);
	cc1100_read(RX_fifo, &b[1],b[0]);
	
}

void RF_sendWOR(unsigned char addr) {

	unsigned char b[2];
	
	b[0]=0x01;
	b[1]=addr;
	
	RFstatus |= WORsend;
	cc1100_write1(0x18,conf[0x18] & 0xCF);
	
	cc1100_strobe(SIDLE);
	cc1100_strobe(SCAL);
	waitTX();
	
	TACTL = TASSEL_1 //ACLK 
		| MC_2 /* Timer A mode control: 2 - Continous up */
		| TACLR;
	while(TAR < 0x3800) {
		cc1100_write(TX_fifo | BURST,b,2);
		cc1100_strobe(STX);
		waitTX();
	}
	
	cc1100_write1(0x18,conf[0x18]);
	RFstatus &= ~WORsend;
	waitTX();
	P1OUT ^= WD_RST;
	
}

interrupt (PORT1_VECTOR) RF_IRQ (void) {
		
	if (cc1100_read1(0x3B | READ | BURST) > 0) {
		RF_getfifo(RFbuf);
		
		cc1100_strobe(SIDLE);
		cc1100_strobe(SFRX);
		cc1100_strobe(SRX);
	
		struct cc1100frame_ *RXframe;
		RXframe = (struct cc1100frame_ *)&RFbuf[0];
		
		if (RXframe->len > 3) {
			switch ( RXframe->packetType) {
				case 0x01:
				break;
				case 0x02:
					if (RXframe->data[0] == 0x01) {
						RXframe->destAddr = RXframe->srcAddr;
						RXframe->srcAddr = cc1100Addr;
						RXframe->data[0] = 0x02;
						RXframe->len = 0x04;
						switch_to_idle();
						RF_send(RFbuf);
						P1OUT ^= WD_RST;
					}
					else if (RFbuf[headerLen] == 0x02) {
						RFstatus |= Received;
					}
				break;
				case 0x03:
				break;
			}
		}
	}
	
	waitTX();
	P1IFG  =  0;
	P1OUT ^= WD_RST;
}



	
