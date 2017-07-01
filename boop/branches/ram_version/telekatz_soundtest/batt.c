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
#include "batt.h"
#include "lcd.h"


void initADC(void)
{

	/* Setup A/D: 10-bit AIN0 @ 4,2MHz "Non-Burst"-Mode */
	
	//PINSEL1 |= (1UL<<28); // set function P0.29 as AIN2
	
	// sample AIN0 only => bit 0 = 1 
	// CLKDIV = 14 (59/14 = 4.21 < 4.5 MHz) => Bits 8ff = 14-1
	// BURST = 1 => set Bit 16 - wuff: disabled below
	// PDN   = 1 => set Bit 21
	ADCR = ( 1<<5 | ((14-1)<<8) /*| (1UL<<16)*/ | (1UL<<21) );
	
}
unsigned int getADCvalue(void)
{	
	unsigned int val=0;
	ADCR = ( 1<<5 | ((14-1)<<8) /*| (1UL<<16)*/ | (1UL<<21) );
	ADCR  |= (1UL<<24);  /* Start A/D Conversion (START:0=1) */
	while ((ADDR & (1UL<<31)) == 0); /* Wait for the conversion to complete (DONE=1)*/
	val = ((ADDR >> 6) & 0x03FF);	/* Extract the A/D result */
	ADCR &=~(1UL<<21);
	return val;
}

void draw_battery(unsigned char x, unsigned char y)
{	
	unsigned int val=0;
	unsigned char w=0;
	
	draw_rect(x,y,14,7,1,3,DRAW_PUT);
	draw_vline(x+14,y+2,3,3,DRAW_PUT);
	draw_block(x+1,y+1,12,5,2,DRAW_ERASE);
	
	val=getADCvalue();
		
	if (val<0x320)
		val=0x320;
	if (val>0x380)
		val=0x380;
			
	w= (val-0x320)/8;
		
	draw_block(x+1,y+1,w,5,2,DRAW_PUT);
}

void draw_ant(unsigned char x, unsigned char y, unsigned char m) {

	draw_vline(x+3,y,7,3,m);
	draw_pixel(x,y,3,m);
	draw_pixel(x+1,y+1,3,m);
	draw_pixel(x+2,y+2,3,m);
	draw_pixel(x+6,y,3,m);
	draw_pixel(x+5,y+1,3,m);
	draw_pixel(x+4,y+2,3,m);
	
}
