/*
    cc1100.c - 
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

// CC1100 - http://focus.ti.com/docs/prod/folders/print/cc1100.html
//  27 MHz crystal
// http://en.wikipedia.org/wiki/ISM_band http://de.wikipedia.org/wiki/ISM-Band
// http://de.wikipedia.org/wiki/Short_Range_Devices http://en.wikipedia.org/wiki/Low-power_communication_device
//  433,05 MHz ... 434,79 MHz

#include "lpc2220.h"
#include "cc1100.h"
#include "irq.h"

//setting 6_WOR
const unsigned char conf[0x2F] = {   
	0x29,	// IOCFG2
	0x2E,	// IOCFG1
	0x06,	// IOCFG0
	0x47,	// FIFOTHR
	0xD3,	// SYNC1
	0x91,	// SYNC0
	0x3E,	// PKTLEN
	0x1A,	// PKTCTRL1
	0x45,	// PKTCTRL0
	0x01,	// ADDR
	0x01,	// CHANNR
	0x06,	// FSCTRL1
	0x00,	// FSCTRL0
	0x10,	// FREQ2		#
	0x0B,	// FREQ1		#
	0xDA,	// FREQ0		# -> 433,249969 MHz
	0x8A,	// MDMCFG4
	0x75,	// MDMCFG3
	0x13,	// MDMCFG2
	0x22,	// MDMCFG1
	0xC1,	// MDMCFG0	CHANSPC_M
	0x35,	// DEVIATN
	0x04,	// MCSM2
	0x0C,	// MCSM1 0c
	0x38,	// MCSM0
	0x16,	// FOCCFG
	0x6C,	// BSCFG
	0x43,	// AGCCTRL2
	0x40,	// AGCCTRL1
	0x91,	// AGCCTRL0
	0x46,	// WOREVT1
	0x50,	// WOREVT0
	0x78,	// WORCTRL
	0x56,	// FREND1
	0x10,	// FREND0
	0xA9,	// FSCAL3
	0x0A,	// FSCAL2
	0x00,	// FSCAL1
	0x11,	// FSCAL0
	0x41,	// RCCTRL1
	0x00,	// RCCTRL0
	0x57,	// FSTEST
	0x7F,	// PTEST
	0x3F,	// AGCTEST
	0x98,	// TEST2
	0x31,	// TEST1
	0x0B	// TEST0
};

const unsigned char confasync[0x2F] = {   
	0x0D,	// IOCFG2
	0x0D,	// IOCFG1
	0x2E,	// IOCFG0
	0x47,	// FIFOTHR
	0xD3,	// SYNC1
	0x91,	// SYNC0
	0x3E,	// PKTLEN
	0x1A,	// PKTCTRL1
	0x32,	// PKTCTRL0	| asynchronous serial mode
	0x00,	// ADDR
	0x00,	// CHANNR	channel number
	0x06,	// FSCTRL1
	0x00,	// FSCTRL0
	0x10,	// FREQ2	# 
	0x12,	// FREQ1	# 
	0x34,	// FREQ0	# *27e6/2^16 = 433,919861 MHz base frequency
	0x4A,	// MDMCFG4	| channel bandwidth
	0x84,	// MDMCFG3
	0x30,	// MDMCFG2
	// 0x00,	// MDMCFG2	// 2-FSK
	0x20,	// MDMCFG1
	0xe5,	// MDMCFG0	CHANSPC_M Chann 49,95kHz | DRATE_E[3:0]
	0x37,	// DEVIATN
	0x07,	// MCSM2
	0x30,	// MCSM1 0c
	0x18,	// MCSM0
	0x14,	// FOCCFG
	0x6C,	// BSCFG
	0x07,	// AGCCTRL2
	0x00,	// AGCCTRL1
	0x90,	// AGCCTRL0
	0x46,	// WOREVT1
	0x50,	// WOREVT0
	0x78,	// WORCTRL
	0x56,	// FREND1
	0x11,	// FREND0
	0xE9,	// FSCAL3
	0x2A,	// FSCAL2
	0x00,	// FSCAL1
	0x1F,	// FSCAL0
	0x41,	// RCCTRL1
	0x00,	// RCCTRL0
	0x59,	// FSTEST
	0x7F,	// PTEST
	0x3F,	// AGCTEST
	0x81,	// TEST2
	0x35,	// TEST1
	0x09	// TEST0
};

void cc1100_init(void) {
	
	unsigned long xx = 0x200;
	
	PINSEL1 &= 0xffffff00;					//GDO0 SCK1 MISO1 MOSI1 as GPIO
	FIODIR0 |= (CS1 | MOSI1 | SCK1); 		//output
	
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
	
	PINSEL1 |= 0x000002A8;		//SCK1 MISO1 MOSI1 as SPI1
	SSPCR0 = 0x0007;
	SSPCPSR = 0x02;
	SSPCR1 = 0x02;
			
	SSPDR = SRES;
	while (FIOPIN0 & MISO1);	
	while (SSPSR & (1<<4));
	xx = SSPDR;
		
	cc1100_write((0x00 | BURST ),(unsigned char*)conf,0x2f);
	cc1100_write1(PATABLE,0xC0);
	cc1100_strobe(SIDLE);
	cc1100_strobe(SPWD);
}

// write length bytes of data to addr in CC1100
unsigned char cc1100_write(unsigned char addr,unsigned char* data, unsigned char length) {

	unsigned short i;
	unsigned char status;
	unsigned char x;

	(void)(x);
	
	FIOCLR0 = CS1;
	while (FIOPIN0 & MISO1);
	SSPDR = (addr | WRITE);
	while ((SSPSR & (1<<4)));
	status = SSPDR;
	for (i=0; i < length; i++) {
		SSPDR = data[i];
		while ((SSPSR & (1<<4)));
		x=SSPDR;
	}
	FIOSET0 = CS1;
	
	return(status);
}

// write on byte of data to addr in CC1100
// a few instructions faster than cc1100_write(addr, data, 1)
unsigned char cc1100_write1(unsigned char addr,unsigned char data) {

	unsigned char status;
	volatile unsigned char x=0;
	
	(void)(x);

	FIOCLR0 = CS1;
	while (FIOPIN0 & MISO1);
	SSPDR = (addr | WRITE);
	while ((SSPSR & (1<<4)));
	status = SSPDR;
	
	SSPDR = data;
	while ((SSPSR & (1<<4)));
	x=SSPDR;
	
	FIOSET0 = CS1;
	
	return(status);
}


unsigned char cc1100_read(unsigned char addr, unsigned char* data, unsigned char length) {

	unsigned short i;
	unsigned char status;
		
	FIOCLR0 = CS1;
	
	while (FIOPIN0 & MISO1);
	SSPDR = (addr | READ);
	while ((SSPSR & (1<<4)));
	status = SSPDR;
	for (i=0; i < length; i++) {
		SSPDR = 0x00;
		while ((SSPSR & (1<<4)));
		data[i]=SSPDR;
	}
	FIOSET0 = CS1;
	
	return(status);
}

unsigned char cc1100_read1(unsigned char addr) {

	unsigned char r;
	
	FIOCLR0 = CS1;
	while (FIOPIN0 & MISO1);
	SSPDR = (addr | READ);
	while ((SSPSR & (1<<4)));
	r = SSPDR;
	SSPDR = 0x00;
	while ((SSPSR & (1<<4)));
	r=SSPDR;
	FIOSET0 = CS1;
	
	return(r);
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
/*
// check that PLL has locked
unsigned char cc1100_pll_lock(void)
{
	//~ To check that the PLL is in lock the user can
	//~ program register IOCFGx.GDOx_CFG to 0x0A
	//~ and use the lock detector output available on
	//~ the GDOx pin as an interrupt for the MCU (x =
	//~ 0,1, or 2). A positive transition on the GDOx
	//~ pin means that the PLL is in lock. As an
	//~ alternative the user can read register FSCAL1.
	//~ The PLL is in lock if the register content is
	//~ different from 0x3F. Refer also to the CC1100
	//~ Errata Notes [1]. For more robust operation the
	//~ source code could include a check so that the
	//~ PLL is re-calibrated until PLL lock is achieved
	//~ if the PLL does not lock the first time.
}
*/
/*
// get carrier frequency
unsigned char cc1100_carrier(void)
{
	//~ f_carrier = fxosc/(2**16) * (FREQ + CHAN * (256+CHANSPC_M) * 2**(CHANSPC_E - 2)) 
}

// set intermediate frequency
unsigned char cc1100_if(void)
{
	//~ FSCTRL1.FREQ_IF
	//~ f_ = fxosc/(2**10) * FREQ_IF
}


// set channel spacing
unsigned char cc1100_chspc(void)
{
	//~ MDMCFG0.CHANSPC_M
	//~ MDMCFG1.CHANSPC_E r
}

// set channel
unsigned char cc1100_channel(void)
{
	//~ CHANNR.CHAN
}

// set frequency
unsigned char cc1100_frequency(void)
{
	//~ FREQ2, FREQ1, FREQ0 
	//~ This word will typically be set to the centre of the lowest channel frequency that is to be used.
}

// set modulation
unsigned char cc1100_modulation(void)
{
	//~ MDMCFG2.MOD_FORMAT
	//~ MDMCFG2.MANCHESTER_EN.
}
*/
