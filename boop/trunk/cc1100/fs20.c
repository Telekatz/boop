/*
    fs20.c - FS20 functions
    Copyright (C) 2009  <telekatz@gmx.de>  

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
#include "fs20.h"
#include "rf.h"
#include "cc1100.h"
#include "fonty.h"
#include "rtc.h"
#include "global.h"
#include "lcd.h"
#include "keyboard.h"
#include "serial.h"
#include "timerfuncs.h"

#define FS20_BETWEEN(x, a, b) ((x >= a) && (x <= b))

extern const char hval[16];

#define PROTOCOL_UNKNOWN	0
#define PROTOCOL_FS20		1
#define PROTOCOL_WEATHER	2
#define PROTOCOL_WEATHER2	3

void fs20_decoder(void) {

	unsigned char x;
	unsigned char FS20laststate = 0;
	unsigned char FS20preamble = 0;
	unsigned char FS20parity = 0;
	unsigned char FS20state = 0;
	unsigned long FS20lasttime = 0;
	unsigned char FS20bitcounter  = 0;
	unsigned char FS20bytecounter = 0;
	unsigned char FS20inbyte[20];
	unsigned char FS20inbit = 0;
	unsigned char FS20protocol = PROTOCOL_UNKNOWN;
	unsigned short tempbyte = 0;
	unsigned char finish = 0;
	
	unsigned long pulse = 0;
	unsigned long space = 0;
	unsigned long now;
				
	lcd_fill(0);
	set_font(BOLDFONT);
    draw_string (0, 0, "FS20 Decoder", LCD_COLOR_B, DRAW_PUT);
    draw_block (0,10,128,2, LCD_COLOR_B, DRAW_PUT);
	set_font(SMALLFONT);
	
	RFasyncmode(true);

	cc1100_write1(FREQ2,0x20);
	cc1100_write1(FREQ1,0x29);
	cc1100_write1(FREQ0,0x3E);
	
	cc1100_write1(AGCCTRL2,0x07);
	cc1100_write1(AGCCTRL1,0x00);
	cc1100_write1(AGCCTRL0,0x91);
	
	cc1100_strobe(SFRX);
	cc1100_strobe(SRX);
	
	serial_puts("\n\r");
	
	while(KEY_Exit) {};
	while(!KEY_Exit)
	{

		//internal receiver
		if(FIOPIN0 & GDO2)
			FS20state = 1;
		else
			FS20state = 0;

		/*
		//external rexeiver on pin 0.14
		if(FIOPIN0 & (1<<14))
			FS20state = 1;
		else
			FS20state = 0;
*/
			
		if(FS20state != FS20laststate) {
			FS20laststate = FS20state;
			
			//Calculate pulse len
			if(!FS20state) {				
				now = T0TC;
				if(now > FS20lasttime)
					pulse = now-FS20lasttime;
				else
					pulse = (now+5000)-FS20lasttime;
				FS20lasttime = now;
			}
			//Calculate space len
			else {
				now = T0TC;
				if(now > FS20lasttime)
					space = now-FS20lasttime;
				else
					space = (now+5000)-FS20lasttime;
				FS20lasttime = now;
			}

			//Decode
			if(FS20state || (FS20protocol == PROTOCOL_WEATHER2)) {
				if(FS20protocol == PROTOCOL_UNKNOWN) {
					if((FS20_BETWEEN(pulse, 350, 450) && FS20_BETWEEN(space, 350, 450)) || (FS20_BETWEEN(pulse, 550, 650) && FS20_BETWEEN(space, 550, 650))) {
						FS20protocol = PROTOCOL_FS20;		//FS20, FHT
						FS20inbyte[0] = 'F';
						FS20bytecounter = 1;
						FS20bitcounter = 0;
						FS20preamble = PREAMBLELEN_FS20;
						FS20parity = 0;
						tempbyte = 0;
					}
					else if((FS20_BETWEEN(pulse, 200, 533) && FS20_BETWEEN(space, 685, 1020)) || (FS20_BETWEEN(pulse, 685, 1020) && FS20_BETWEEN(space, 200, 533))) {
						FS20protocol = PROTOCOL_WEATHER;	//KS200, KS300, S2XXX
						FS20inbyte[0] = 'W';
						FS20bytecounter = 1;
						FS20bitcounter = 0;
						FS20preamble = PREAMBLELEN_WEATHER;
						tempbyte = 0;
					}
					else if(FS20_BETWEEN(pulse, 200, 300) && FS20_BETWEEN(space, 200, 300)) {
						FS20protocol = PROTOCOL_WEATHER2;	//ASH500
						FS20inbyte[0] = 'S';
						FS20bytecounter = 1;
						FS20bitcounter = 0;
						FS20preamble = PREAMBLELEN_WEATHER2;
						tempbyte = 0;
					}
				}
				
				if(FS20protocol == PROTOCOL_FS20) {
					if(FS20_BETWEEN(pulse, 350, 450) && FS20_BETWEEN(space, 350, 450))
						FS20inbit = 0;
					else if(FS20_BETWEEN(pulse, 550, 650) && FS20_BETWEEN(space, 550, 650))
						FS20inbit = 1;
					else
						FS20protocol = PROTOCOL_UNKNOWN;
					
					if(FS20protocol != PROTOCOL_UNKNOWN) {
						if(FS20preamble) {
							if((FS20preamble > 1) && FS20inbit)
								FS20preamble = PREAMBLELEN_FS20;
							else if((FS20preamble > 1) && !FS20inbit) {
								FS20preamble--;
							}
							else if((FS20preamble == 1) && FS20inbit) {
								FS20preamble = 0;
							}
						}
						else {
							if(FS20bitcounter == 8) {
								if(FS20inbit == FS20parity) {
									FS20bitcounter = 0;
									FS20bytecounter++;
									FS20parity = 0;
								}
								else {
									FS20protocol = PROTOCOL_UNKNOWN;
								}
							}
							else {
								FS20inbyte[FS20bytecounter] = (FS20inbyte[FS20bytecounter]<<1) | FS20inbit;
								FS20parity ^= FS20inbit;
								FS20bitcounter++;
							}
						}
					}
				}
				
				if(FS20protocol == PROTOCOL_WEATHER) {
					if(FS20_BETWEEN(pulse, 685, 1020) && FS20_BETWEEN(space, 200, 533))
						FS20inbit = 0;
					else if(FS20_BETWEEN(pulse, 200, 533) && FS20_BETWEEN(space, 685, 1020))
						FS20inbit = (1<<3);
					else
						FS20protocol = PROTOCOL_UNKNOWN;
						
					if(FS20protocol != PROTOCOL_UNKNOWN) {
						if(FS20preamble) {
							if((FS20preamble > 1) && FS20inbit)
								FS20preamble = PREAMBLELEN_WEATHER;
							else if((FS20preamble > 1) && !FS20inbit) {
								FS20preamble--;
							}
							else if((FS20preamble == 1) && FS20inbit) {
								FS20preamble = 0;
							}
						}
						else {
							if(FS20bitcounter == 4) {
								tempbyte<<=8;
								FS20bitcounter++;
							}
							else if(FS20bitcounter == 9) {
								FS20bitcounter = 0;	
							}
							else {
								tempbyte = (tempbyte>>=1) | FS20inbit;
								if(FS20bitcounter++ == 8) {
									FS20inbyte[FS20bytecounter] = tempbyte;
									tempbyte = 0;
									FS20bytecounter++;
								}
							}
						}
					}
				}

				if(FS20protocol == PROTOCOL_WEATHER2) {
					if(FS20preamble) {
						if((FS20_BETWEEN(pulse, 200, 300) && FS20_BETWEEN(space, 200, 300))) {
							if (FS20preamble > 1)
								FS20preamble--;
						}
						else if((FS20_BETWEEN(pulse, 400, 600) && FS20_BETWEEN(space, 200, 300))) {
							if (FS20preamble == 1) {
								FS20preamble = 0;
								pulse = 0;
								space = 0;
							}
							else
								FS20protocol = PROTOCOL_UNKNOWN;
						}
						else
							FS20protocol = PROTOCOL_UNKNOWN;
					}
					else {
						if((FS20_BETWEEN(pulse + space, 400, 600)) ) {
							if(FS20state)
								FS20inbit = 1;
							else
								FS20inbit = 0;

							pulse = 0;
							space = 0;

							if(FS20inbit)
								FS20parity++;

							if(FS20bitcounter == 8) {
									if(!(FS20parity & 0x01)) {
										FS20bitcounter = 0;
										FS20bytecounter++;
										FS20parity = 0;
										if(FS20bytecounter == 0x0a) {	//packet full received
											unsigned char dec[8];

											dec[0] = FS20inbyte[1] ^ 0x89;
											for(x=9;x>1;x--) {
												FS20inbyte[x] = (FS20inbyte[x-1] + 0x24) ^ FS20inbyte[x];
											}
											FS20inbyte[1] = FS20inbyte[1] ^ 0x89;

											FS20protocol = PROTOCOL_UNKNOWN;
										}
									}
									else {
										FS20bytecounter = 0;
										FS20protocol = PROTOCOL_UNKNOWN;
									}
								}
								else {
									FS20inbyte[FS20bytecounter] = (FS20inbyte[FS20bytecounter]<<1) | FS20inbit;
									FS20bitcounter++;
								}
						}
						else if(pulse && space) {
							FS20bytecounter = 0;
							FS20protocol = PROTOCOL_UNKNOWN;
						}
					}
				}

				if((FS20protocol == PROTOCOL_UNKNOWN && FS20bytecounter > 1)) {
					char msbuffer[20];
					
					draw_block(0,32,128,24,3,DRAW_ERASE);

					//print time and protocol type
					siprintf (msbuffer, "%02x:%02x:%02x.%02u %c  ",rtc.time.hour,rtc.time.minute,rtc.time.second,(unsigned char)(T0overflow>>1) & 0xff, FS20inbyte[0]);
					draw_string (0, 32, msbuffer, LCD_COLOR_B, DRAW_NORCU);
					serial_puts(msbuffer);
					
					//print received data
					for(x=1;x<(FS20bytecounter);x++) {
						draw_hexC (((x-1)*14), 40, FS20inbyte[x], LCD_COLOR_B, DRAW_NORCU);
						serial_putbyte (' ');
						putHexC(FS20inbyte[x]);
					}

					//print decoded data
					if(FS20inbyte[0] == 'S') {
						signed short temp;

						temp = ((FS20inbyte[6] & 0x7F)<<8) | FS20inbyte[7];
						if(temp & 0x4000)		//negative
							temp |=0x8000;

						serial_putbyte (' ');
						serial_putbyte (' ');

						siprintf (msbuffer, "%4d °C %u%%", temp,FS20inbyte[8]);
						msbuffer[4] = msbuffer[3];
						msbuffer[3] = '.';
						serial_puts(msbuffer);
						msbuffer[5] = 0x15;		//change °C on display
						msbuffer[6] = ' ';
						draw_string (0, 48, msbuffer, LCD_COLOR_B, DRAW_NORCU);
					}

					serial_puts("\n\r");
					finish = 0;
					FS20bytecounter = 0;
					FS20protocol = PROTOCOL_UNKNOWN;
				}
			}
		}
	}
	
	RFasyncmode(false);
}


