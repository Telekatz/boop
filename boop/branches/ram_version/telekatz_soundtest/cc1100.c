/*
    batt.c - 
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

#include "lpc2220.h"
#include "cc1100.h"
#include "irq.h"

const unsigned char conf[0x2F] = { 	0x29 , 0x2E , 0x07 , 0x00 , 0xD3 , 0x91 , 0x3D , 0x0C ,
										0x05 , 0x00 , 0x00 , 0x06 , 0x00 , 0x20 , 0x28 , 0xC5 ,
										0xF5 , 0x75 , 0x83 , 0x22 , 0xE5 , 0x14 , 0x07 , 0x30 ,
										0x18 , 0x16 , 0x6C , 0x03 , 0x40 , 0x91 , 0x87 , 0x6B ,
										0xF8 , 0x56 , 0x10 , 0xE9 , 0x2A , 0x00 , 0x1F , 0x41 ,
										0x00 , 0x59 , 0x7F , 0x3F , 0x81 , 0x35 , 0x09 };
		
volatile unsigned char cc1100rx;


void cc1100_init(void) {
	
	unsigned long xx = 0x200;
	
	
	PINSEL1 &= ~((1<<3) | (1<<5) | (1<<7));
	PCONP &= 0xfffffbff;
	PCONP |= (1<<21);	
	FIOSET0 = SCK1;
	FIOCLR0 = MOSI1;
	FIOCLR0 = CS1;
	while(xx) {
		asm volatile("nop" : :);
		xx--;
	}
	FIOSET0 = CS1;
	xx=0x200;	
	while(xx) {
		asm volatile("nop" : :);
		xx--;
	}
	FIOCLR0 = CS1;
	FIOCLR0 = SCK1;	
	while (FIOPIN0 & MISO1);
	
	PINSEL1 |= 0x000002A8; 	//((1<<3) | (1<<5) | (1<<7));
	SSPCR0 = 0x0007;
	SSPCPSR = 0x02;
	SSPCR1 = 0x02;
			
	SSPDR = SRES;
	while (FIOPIN0 & MISO1);	
	while (SSPSR & (1<<4));
	xx = SSPDR;
		
	cc1100_write((0x00 | BURST ),conf,0x2f);
}


unsigned char cc1100_write(unsigned char addr,unsigned char* data, unsigned char lenght) {

	unsigned short i;
	unsigned char status;
	unsigned char x;
	
	FIOCLR0 = CS1;
	while (FIOPIN0 & MISO1);
	SSPDR = (addr | WRITE);
	while ((SSPSR & (1<<4)));
	status = SSPDR;
	for (i=0; i < lenght; i++) {
		SSPDR = data[i];
		while ((SSPSR & (1<<4)));
		x=SSPDR;
	}
	FIOSET0 = CS1;
	
	return(status);
	}

unsigned char cc1100_read(unsigned char addr, unsigned char* data, unsigned char lenght) {

	unsigned short i;
	unsigned char status;
	
	FIOCLR0 = CS1;
	while (FIOPIN0 & MISO1);
	SSPDR = (addr | READ);
	while ((SSPSR & (1<<4)));
	status = SSPDR;
	for (i=0; i < lenght; i++) {
		SSPDR = 0x00;
		while ((SSPSR & (1<<4)));
		data[i]=SSPDR;
	}
	FIOSET0 = CS1;
	
	return(status);
}

unsigned char cc1100_strobe(unsigned char cmd) {

	unsigned short status;
		
	FIOCLR0 = CS1;
	while (FIOPIN0 & MISO1);
	SSPDR = cmd;
	while ((SSPSR & (1<<4)));
	status = SSPDR;
	FIOSET0 = CS1;
	
	return(status);
}

void startcc1100IRQ(void) {
	PINSEL1 |= 1;
	EXTMODE |= 1;
	EXTPOLAR |= 1;	
	EXTINT |= 1;
	
	VICVectAddr1 = (unsigned long)&(cc1100IRQ);
	VICVectCntl1 = VIC_SLOT_EN | INT_SRC_EINT0;
	VICIntEnable = INT_EINT0;
}

void cc1100_getfifo(unsigned char* b) {
	
	unsigned char cnt;
	
	cc1100_read(RX_fifo, &cnt,1);
	cc1100_read(RX_fifo, b,cnt+2);
	b[cnt]=0;
	
}

void  __attribute__ ((interrupt("IRQ")))  cc1100IRQ (void) {
	
	EXTINT = 0x01;
	cc1100rx =1;
	
	
	VICVectAddr = 0;
}


	
