/*
    main.c
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

#include <P89LPC932.h>  // special function register declarations
                        // for the Philips P89C931 device

#include "rf.h"
#include "serial.h"
#include "cmd.h"
#include "cc1100.h"
#include "timer.h"

volatile bit terminal;
volatile bit redirector;

__sfr __at (0xAD) CMP2_   ;

void main(void) {

	unsigned char x;
	unsigned char len;
	
	P0M1 &= 0x6c;
	P0M2 |= 0x92;
	P0 = 0x83;
	
	P1M1 &= 0x3e;
	P1M2 |= 0xcd;
	P1 = 0xcd;
	
	P3M1 &= 0xfe;
	P3M2 |= 0x01;
	P3 = 0;
		
	/*PT0AD = 0x24;
	TMOD = 0x22;
	TAMOD = 0x10;
	TCON = 0x50;
	AUXR1 = 0x20;
	TH0 = 0x22;
	TH1 = 0x80;
	CMP2_ = 0x24;
	*/
	
	initTimer0();
	initSerial();
	RF_init();
	RF_startIRQ();
	
	EA = 1;
	
	send_string(crlf);
	send_string("Ready");
	send_string(crlf);
	
	terminal = 1;
	//redirector = 1;
	len =0;
	
	while (1) {
		if(redirector) {
			if(inbuflen >= 58) {
				transmitRedir(58,redir_dest);
			}
			else if(inbuflen && TF0) {
					transmitRedir(inbuflen,redir_dest);
			}
		}
		else if (terminal) {
			if(len < inbuflen) {
				x=get_inbuf_index(len++);
				if (x  == 0x0d) {
					send_string(crlf);
					inbuf[(inbufindex + len-1) & 0x3F] = 0;
					x=get_inbuf();
					len--;
					if(len) {
						switch(x) {
							case 't':
								if (inbuflen) {
									transmit(len);
									len = 0;
									send_string(crlf);
								}
								break;
							case 'r':
								if (inbuflen) {
									transmitRedir(len,0);
									len = 0;
									send_string("TX Done");
								}
								break;	
							case 's':
								print_cc1100status();
								break;
							case 'c':
								print_cc1100confi();
								break;
							case 'b':
								terminal = 0;
								send_byte(0x01);
								break;
							case 'w':
								sendWOR(0x00);
								break;
							default:
								send_string("Syntax error");
						}
						while(len) {
							get_inbuf();
							len--;
						}
					}
					else {
						send_string("Ready");
					}
					send_string(crlf);  
				}
				else {
					send_byte(x);
				}
			}
		}
		
		else {
			if (inbuflen) {
				x=get_inbuf();
				switch(x) {
					case 0x00:
						
						break;
					case 0x01:
						transmitB();
						break;
					case 0x02:
						cmd_strobeB();
						break;
					case 0x03:
						read_byteB();
						break;
					case 0x04:
						write_byteB();
						break;
					case 0x05:
						read_burst_byteB();
						break;
					case 0x06:
						write_burst_byteB();
						break;
					case 0x07:
						send_WORb();
						break;
					default:
						send_byte(0x00);
				}
				if (!terminal)
					send_byte(0x01);
			}
		}
	}     
}
