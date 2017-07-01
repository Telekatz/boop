/*
    main.c - main program routines
    Copyright (C) 2007  Ch. Klippel <ck@mamalala.net>

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

//#define WITH_SOUND

typedef volatile unsigned int *reg32_t;

#include "lpc2220.h"
#include "lcd.h"
#include "irq.h"
#include "soundcore.h"
#include "fonty.h"
#include "keyboard.h"
#include "externs.h"

static unsigned int old_tval;

unsigned long checksum;
unsigned short count;
unsigned long adr;
const char hval[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

unsigned char idle;
unsigned char ccbuf[64];

extern unsigned char cc1100rx;
extern unsigned char batcnt;

#define		KEY_IDLE	0x00
#define		KEY_PRESS	0x01
#define		KEY_HOLD	0x02
#define		KEY_RELEASE	0x03

#define	SPEED_30	0
#define	SPEED_60	1

void setSpeed(unsigned char sp)
{
	disableIRQ();
	switch(sp)
	{
		case SPEED_60:		// turbo speed (60 mhz)
			BCFG0	= 0x100005EF;
			PLLCFG	= 0x45;		// 0100.0101	= PSEL=10=4	MSEL=00101=6 = 240mhz Fcco
			PLLFEED = 0xAA;
			PLLFEED = 0x55;

			VPBDIV = 0x00;

			BCFG0	= 0x100004A0;
			BCFG2	= 0x100004A0;
			BCFG1	= 0x00000C21;
			break;

		case SPEED_30:	// normal speed (30 mhz)
		default:
			BCFG0	= 0x100005EF;
			PLLCFG	= 0x42;		// 0100.0010	= PSEL=10=4	MSEL=00010=3
			PLLFEED = 0xAA;
			PLLFEED = 0x55;

			VPBDIV = 0x02;

			BCFG0	= 0x10000420;
			BCFG2	= 0x10000420;
			BCFG1	= 0x00000400;
			break;
	}
	enableIRQ();
}


int main(void)
{
	unsigned char key_state, bl_vol;
	unsigned int oldkeys[2], ok[2];
	
	setSpeed(SPEED_30);
	
	//*************************************************************************************
	
	
	
	//*************************************************************************************
	oldkeys[0] = 0;
	oldkeys[1] = 0;
	ok[0] = 0;
	ok[1] = 0;
	key_state = KEY_IDLE;
	old_tval = 0;
	bl_vol = 0xff;
	lcd_init(0);
	SCS |= 0x01;
	FIODIR0 |= ((1<<21)  | (1<<4) | (1<<11) | (1<<6) | (1<<23) | (1<<19) | (1<<17));
	
	initKeys();
	//initIR();
	IODIR0 |= (1<<12);
	//VPBDIV = 0x02;
	startKeyIRQ();
	startPWMIRQ();

	idle =0;
	enableIRQ();
	
	while (1)
	{	
		
		switch(key_state)
		{
			case KEY_IDLE:
				if(keys[0] != oldkeys[0] || keys[1] != oldkeys[1])
				{
					if(keys[0] != 0 || keys[1] != 0)
					{
						draw_hexW(72,140,ok[0],3,DRAW_ERASE);
						draw_hexW(72,150,ok[1],3,DRAW_ERASE);
						draw_hexW(72,140,keys[0],3,DRAW_PUT);
						draw_hexW(72,150,keys[1],3,DRAW_PUT);
						ok[0] = keys[0];
						ok[1] = keys[1];
						key_state = KEY_PRESS;
					}

					oldkeys[0] = keys[0];
					oldkeys[1] = keys[1];

					if(KEY_A)
					{
						if(timeout & 0x08)
							timeout = 0;
						else
							timeout = 0x08;
					}
					else if(KEY_B)
					{

						
					}
					else if(KEY_C)
					{

					}
					else if(KEY_D)
					{

					}
					else if(KEY_Vminus)
					{
						FIOCLR0 |= (1<<11);
					}
					else if(KEY_Vplus)
					{
						FIOSET0 |= (1<<11);
						playSID();
					}
					else if(KEY_OK)
					{
						
					}
					else if(KEY_Mute)
					{
				
					}
					else if(KEY_1)
					{

					}
					else if(KEY_2)
					{
						
					}
					else if(KEY_3)
					{
						
					}
					else if(KEY_4)
					{
						
					}
					else if(KEY_5)
					{
						
					}
					else if(KEY_6)
					{
						
					}
					else if(KEY_7)
					{
						
					}
					else if(KEY_8)
					{
						
					}
					else if(KEY_9)
					{
						
					}
				}
				break;
			case KEY_PRESS:
				//irSend();
				key_state = KEY_HOLD;
				autorepeat = 0;
				break;
			case KEY_HOLD:
				if(autorepeat >= AUTO_TIMEOUT)
					//irRepeat();

				if(keys[0] == 0 && keys[1] == 0)
					key_state = KEY_RELEASE;

				break;
			case KEY_RELEASE:
				//irStop();
				key_state = KEY_IDLE;
				break;
		}

	
	}
	return 0;
}


