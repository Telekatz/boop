/*
    rf.c
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

#include <P89LPC932.h>
#include "cc1100.h"
#include "cmd.h"
#include "rf.h"
#include "serial.h"

bit WORsend;
unsigned char cc1100Addr;
extern volatile bit terminal;
extern volatile bit redirector;

void RF_init (void) {
	cc1100_init();
	WORsend=0;
	cc1100Addr = conf[0x09];
}

void RF_startIRQ (void) {
	KBPATN = 0x40;
	KBMASK = 0x40;
	KBCON = 0x02;
	EKBI = 1;
}

void RF_isr (void) __interrupt (7) {
	struct cc1100frameheader_ RXframe;
	unsigned char i;
	
	if(KBPATN & 0x40) {
		KBPATN = 0x00;		//IRQ on low level
		KBCON = 0x02;
		return;
	}
	
	RXframe.len = cc1100_read1(RXBYTES);
	if (RXframe.len) {
		bit startTX = 0;
		if(RXframe.len > 3)
			cc1100_read(RX_fifo, &RXframe.len,4);
		else
			cc1100_read(RX_fifo, &RXframe.len,RXframe.len);
		
		if (RXframe.len > 3) {
			if(terminal || redirector) {
				bit ACK = 0;
				switch ( RXframe.packetType) {
					case packet_ping:
						if (cc1100_read1(RX_fifo) == 0x01) {
							RXframe.len = 0x04;
							RXframe.destAddr = RXframe.srcAddr;
							RXframe.srcAddr = cc1100Addr;
							RXframe.packetType = packet_ping;
							cc1100_write(TX_fifo,(unsigned char*)RXframe,4);
							cc1100_write1(TX_fifo,0x02);		
							startTX = 1;
						}
						break;
					case packet_redirDAT:
						cc1100_single(RX_fifo | BURST | READ,0);
						for(i=3;i<RXframe.len;i++) {
							send_byte(cc1100_single(0,0));
						}
						CS = 1;
						break;
					case packet_redirCMD:
						switch (cc1100_read1(RX_fifo)) {
							case redir_ACK:
								break;
							case redir_SET:
								if (RXframe.len > 4) {
									unsigned char x;
									x = cc1100_read1(RX_fifo);
									if(x <= baud_115200) {
										redir_baud = x;
									}
								ACK = 1;
								}
								break;
							case redir_START:
								redir_dest = RXframe.srcAddr;
								setbaud(redir_baud);
								redirector = 1;
								ACK = 1;
								break;
							case redir_STOP:
								redirector = 0;
								setbaud(baud_default);
								ACK = 1;
								break;
							case redir_PING:
								ACK = 1;
								break;
						}
						if(ACK) {
							RXframe.len = 0x04;
							RXframe.destAddr = RXframe.srcAddr;
							RXframe.srcAddr = cc1100Addr;
							RXframe.packetType = packet_redirCMD;
							cc1100_write(TX_fifo,(unsigned char*)RXframe,4);
							cc1100_write1(TX_fifo,redir_ACK);	
							startTX = 1;
						}
						break;	
					default:
						if ((RXframe.len > 3) && terminal) {
								send_string(crlf);
								send_string("RX: ");
								cc1100_single(RX_fifo | BURST | READ,0);
								for(i=3;i<RXframe.len;i++) {
									send_byte(cc1100_single(0,0));
								}
								CS = 1;	
								send_string(crlf);
						}
				}
			}
			else {
				send_byte(0x02);
				send_bytes(&RXframe.len, 4);
				cc1100_single(RX_fifo | BURST | READ,0);
				for(i=3;i<RXframe.len;i++) {
					send_byte(cc1100_single(0,0));
				}
				CS = 1;
			} 
		}
		
		cc1100_strobe(SIDLE);
		while (cc1100_read1(MARCSTATE) != MARCSTATE_IDLE);
		cc1100_strobe(SFRX);
		if(startTX)
			cc1100_strobe(STX);
		else
			cc1100_strobe(SRX);
	}
	else {
		if(cc1100_read1(MARCSTATE) != MARCSTATE_RX) {
			cc1100_strobe(SIDLE);
			while (cc1100_read1(MARCSTATE) != MARCSTATE_IDLE);
			cc1100_strobe(SFRX);
			cc1100_strobe(SFTX);
			cc1100_strobe(SRX);
		}
	}
	
	KBPATN = 0x40;		//IRQ on high level
	KBCON = 0x02;
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
				if (WORsend)
					cc1100_strobe(SIDLE);
				x=0;
				break;
			case  0x00:
				if (!(WORsend))
					cc1100_strobe(SRX);
				x=0;
				break;
		}	
	}
}

void sendWOR(unsigned char addr) {

	unsigned char b[2];
	unsigned int i;
	
	b[0]=0x01;
	b[1]=addr;
	
	EKBI = 0;
	WORsend=1;
	cc1100_write1(0x18,conf[0x18] & 0xCF);
	
	cc1100_strobe(SIDLE);
	cc1100_strobe(SCAL);
	waitTX();
	
	i=0;
	while (i++ < 250) {
		cc1100_write(TX_fifo | BURST,b,2);
		cc1100_strobe(STX);
		waitTX();
	}
	
	cc1100_write1(0x18,conf[0x18]);
	WORsend=0;
	waitTX();
	KBCON = 0x02;
	EKBI = 1;
	
}
