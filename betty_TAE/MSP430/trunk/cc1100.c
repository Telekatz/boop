/*
    cc1100.c
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

#include "cc1100.h"
#include "hardware.h"
#include <msp430.h>
//#include <signal.h>
//#include <iomacros.h>
//#include <mspgcc/util.h> 

unsigned char WORsend;
unsigned char cc1100Addr;

const unsigned char conf[0x2f] = {   

0x29 , 0x06 , 0x06 , 0x47 , 0xD3 , 0x91 , 0x3E , 0x1A , 
0x45 , cc1100_addr , cc1100_chan , 0x06 , 0x00 , 0x10 , 0x0B , 0xDA , 
0x8A , 0x75 , 0x13 , 0x22 , 0xC1 , 0x35 , 0x07 , 0x0C , 
0x18 , 0x16 , 0x6C , 0x43 , 0x40 , 0x91 , 0x46 , 0x50 , 
0x78 , 0x56 , 0x10 , 0xA9 , 0x0A , 0x00 , 0x11 , 0x41 , 
0x00 , 0x57 , 0x7F , 0x3F , 0x98 , 0x31 , 0x0B 
};


void CC1100_Select(void) {
	P1OUT &= ~CC_CS;
	while (P1IN & CC_MISO);
}

void CC1100_DeSelect(void) {
  P1OUT |= CC_CS;
}

void cc1100_init(void) {
	
	unsigned char i;
	
	P1OUT  = (P1OUT & ~CC_MOSI) | (CC_CS | CC_SCLK);
	P1DIR |=  CC_CS;
	P1OUT &= ~CC_CS;
	delay(40);
	
	P1OUT |=  CC_CS;
	delay(240);
	
	CC1100_Select();
  
	spi_rw(SRES);
	while (P1IN & CC_MISO);
	
	spi_rw(0x00 | BURST);
	for (i=0; i < 0x2f; i++)
		spi_rw(conf[i]);
	CC1100_DeSelect();

	cc1100Addr = conf[0x09];
	WORsend=0;
}

unsigned char cc1100_write(unsigned char addr,unsigned char* dat, unsigned char lenght) {
 
	unsigned char i;
	unsigned char status;
 
	CC1100_Select();
	status = spi_rw(addr | WRITE);
	for (i=0; i < lenght; i++) 
		spi_rw(dat[i]); 
	CC1100_DeSelect();
 
	return(status);
} 

unsigned char cc1100_write1(unsigned char addr,unsigned char dat) {
 
	unsigned char status;
 
	CC1100_Select();
	status = spi_rw(addr | WRITE); 
	spi_rw(dat); 
	CC1100_DeSelect();
 
	return(status);
} 

unsigned char cc1100_read(unsigned char addr, unsigned char* dat, unsigned char lenght) {
 
	unsigned char i;
	unsigned char status;
 
	CC1100_Select();
	status = spi_rw(addr | READ);
	for (i=0; i < lenght; i++)
		dat[i]=spi_rw(0x00);
	CC1100_DeSelect();
 
	return(status);
}

unsigned char cc1100_read1(unsigned char addr) {
 
	unsigned char r;
 
	CC1100_Select();
	r = spi_rw(addr | READ);
	r=spi_rw(0x00);
	CC1100_DeSelect();
 
	return(r);
}

unsigned char cc1100_strobe(unsigned char cmd) {

	unsigned char status;
  
	CC1100_Select();
	status = spi_rw(cmd);
	CC1100_DeSelect();
 
 return(status);
}

unsigned char spi_rw(unsigned char write) {

	unsigned char z;

	for (z= 8; z > 0; z--) {
		P1OUT &= ~CC_SCLK;		//SCK = 0;
		if (write & 0x80)
			P1OUT |= CC_MOSI;	//MOSI1 = 1;
		else
			P1OUT &= ~CC_MOSI;		//MOSI1 = 0;
		P1OUT |= CC_SCLK;		//SCK = 1;
		write <<=1;
		if (P1IN & CC_MISO)
			write |= 0x01;
	}
	P1OUT &= ~CC_SCLK;
 
	return(write);  
}
 

