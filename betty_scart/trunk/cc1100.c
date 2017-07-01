/*
    cc1100.c
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

#include <P89LPC932.h>
#include "cc1100.h"

code unsigned char conf[0x2F] = {   
	0x29 , 	//IOCFG2
	0x2E , 	//IOCFG1
	0x06 , 	//IOCFG0
	0x47 , 	//FIFOTHR
	0xD3 , 	//SYNC1
	0x91 , 	//SYNC0
	0x3E , 	//PKTLEN
	0x1A , 	//PKTCTRL1
	0x45 , 	//PKTCTRL0
	0x01 , 	//ADDR
	0x01 , 	//CHANNR
	0x06 , 	//FSCTRL1
	0x00 , 	//FSCTRL0
	0x10 , 	//FREQ2
	0x0B , 	//FREQ1
	0xDA , 	//FREQ0
	0x8A , 	//MDMCFG4
	0x75 , 	//MDMCFG3
	0x13 , 	//MDMCFG2
	0x22 , 	//MDMCFG1
	0xC1 , 	//MDMCFG0
	0x35 , 	//DEVIATN
	0x07 , 	//MCSM2
	0x03 , 	//MCSM1 0c
	0x38 , 	//MCSM0
	0x16 , 	//FOCCFG
	0x6C , 	//BSCFG
	0x43 , 	//AGCCTRL2
	0x40 , 	//AGCCTRL1
	0x91 , 	//AGCCTRL0
	0x46 , 	//WOREVT1
	0x50 , 	//WOREVT0
	0x78 , 	//WORCTRL
	0x56 , 	//FREND1
	0x10 , 	//FREND0
	0xA9 , 	//FSCAL3
	0x0A , 	//FSCAL2
	0x00 , 	//FSCAL1
	0x11 , 	//FSCAL0
	0x41 , 	//RCCTRL1
	0x00 , 	//RCCTRL0
	0x57 , 	//FSTEST
	0x7F , 	//PTEST
	0x3F , 	//AGCTEST
	0x98 , 	//TEST2
	0x31 , 	//TEST1
	0x0B 	//TEST0
};

unsigned char spi_rw(unsigned char write) {

	unsigned char z;
	
	for (z= 8; z; z--) {
		SCK = 0;
		MOSI1 = (write & 0x80);
		SCK = 1;
		write <<=1;
		if (MISO1)
			write |= 0x01;
	}
	SCK = 0;
 
	return(write);  
}

unsigned char spi_r() {

	unsigned char z;
	unsigned char ret = 0;
	MOSI1 = 0;
	SCK = 0;
	for (z= 8; z; z--) {
		SCK = 1;
		ret <<=1;
		if (MISO1)
			ret |= 0x01;
		SCK = 0;
	}
 
	return(ret);  
}

void cc1100_init(void) {
 
	unsigned char i = 0xff;
  
	SCK = 1;
	MOSI1 = 0;
	CS = 0;
	while(i) {
		i--;
	}
	CS = 1;
	i=0xff; 
	while(i) {
		i--;
	}
	CS = 0;
	SCK = 0; 
	while (MISO1);  
	spi_rw(SRES);
	while (MISO1);
	
	spi_rw(0x00 | BURST);
	for (i=0; i < 0x2f; i++)
		spi_rw(conf[i]);
	CS = 1;
	
	cc1100_write1(PATABLE,0xC0);
	cc1100_strobe(SIDLE);
	cc1100_strobe(SCAL);
	cc1100_strobe(SFRX);
	cc1100_strobe(SRX);
}

unsigned char cc1100_write(unsigned char addr, unsigned char* dat, unsigned char lenght) {
 
	unsigned char i;
	unsigned char status;
 
	CS = 0;
	while (MISO1);
	status = spi_rw(addr | WRITE);
	for (i=0; i < lenght; i++) 
		spi_rw(dat[i]); 
	CS = 1;
 
	return(status);
} 

unsigned char cc1100_write1(unsigned char addr,unsigned char dat) {
 
	unsigned char status;
 
	CS = 0;
	while (MISO1);
	status = spi_rw(addr | WRITE); 
	spi_rw(dat); 
	CS = 1;
 
	return(status);
} 

unsigned char cc1100_read(unsigned char addr, unsigned char* dat, unsigned char lenght) {
 
	unsigned char i;
	unsigned char status;
 
	CS = 0;
	while (MISO1);
	status = spi_rw(addr | READ);
	for (i=0;lenght; lenght--)
		dat[i++]=spi_r();
	CS = 1;
 
	return(status);
}

unsigned char cc1100_read1(unsigned char addr) {
 
	unsigned char r;
 
	CS = 0;
	while (MISO1);
	r = spi_rw(addr | READ);
	r=spi_rw(0x00);
	CS = 1;
 
	return(r);
}

unsigned char cc1100_strobe(unsigned char cmd) {

	unsigned char status;
  
	CS = 0;
	while (MISO1);
	status = spi_rw(cmd);
	CS = 1;
 
 return(status);
}

unsigned char cc1100_single(unsigned char b,bit end) {

	unsigned char status;
  
	if (CS == 1) {
		CS = 0;
		while (MISO1);
	}
	status = spi_rw(b);
	if (end)
		CS = 1;
 
 return(status);
}

