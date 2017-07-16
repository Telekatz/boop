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

//typedef volatile unsigned int *reg32_t;

#include "lpc2220.h"


#include "lcd.h"
#include "irq.h"
#include "keyboard.h"
#include "lcd.h"
#include "fonty.h"
#include "sid.h"
#include "sound.h"
//#include "sounds.h"
//#include "sound3.h"
#include "infrared.h"
#include "pwm.h"
#include "codes.h"
#include "encoders.h"
#include "ir_selector.h"
#include "serial.h"
#include "flash.h"
#include "rf.h"
#include "menu.h"
#include "testmenu.h"
#include "mainmenu.h"
#include "timerfuncs.h"
#define __MAIN_C__
#include "global.h"
#include "adc.h"
#include "backlight.h"
#include "rtc.h"
#include "i2c.h"
#include "bfs.h"
#include "settingsmenu.h"
#include "ir_raw.h"
#include "ir_capture.h"
#include "controls.h"
#include "buffer.h"

unsigned char sysInfo;

extern volatile char timeout;

extern ir_sfn	irSend;
extern ir_fn	irRepeat;
extern ir_fn	irStop;

const char hval[16] = "0123456789ABCDEF";

unsigned int oldkeys[2];
unsigned int key_state;

extern unsigned char act_encoder, act_set;

extern unsigned char bl_val;


// betty runs on 10 MHz * 1/4 on reset
// this enables the PLL clock generation and sets the prescaler
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

			BCFG0	= 0x10001CA0;
			BCFG2	= 0x10001CA0;
			BCFG1	= 0x00000C21;
			sysInfo |= SYS_TURBO;
			break;

		case SPEED_30:	// normal speed (30 mhz)
		default:
			BCFG0	= 0x100005EF;
			PLLCFG	= 0x42;		// 0100.0010	= PSEL=10=4	MSEL=00010=3
			PLLFEED = 0xAA;
			PLLFEED = 0x55;

			VPBDIV = 0x02;

			BCFG0	= 0x10000A20;
			BCFG2	= 0x10000A20;
			BCFG1	= 0x00000400;
			sysInfo &= ~SYS_TURBO;
			break;
	}
	enableIRQ();
}

/*
// draw "emitting waves" symbol
// used when a button is pressed
void draw_ir (unsigned char x, unsigned char y, unsigned char m)
{
	//~ 0 1:  #####
	//~ 1 2: #	  #
	//~ 2 4:	###
	//~ 3 8:  #	#
	//~ 4 1:
	//~ 5 2:	###
	//~ 6 4:  #####
	
	draw_hline(x+1,y,5,3,m);	// 0 
	
	draw_pixel(x  ,y+1,3,m);	// 1 
	draw_pixel(x+6,y+1,3,m);	// 1 
	
	draw_hline(x+2,y+2,3,3,m);	// 2 
	
	draw_pixel(x+1,y+3,3,m);	// 3 
	draw_pixel(x+5,y+3,3,m);	// 3 
	
	draw_hline(x+2,y+5,3,3,m);	// 5 
	
	draw_hline(x+1,y+6,5,3,m);	// 6 
	
	draw_symbol(x,y,7,(unsigned char*)symIR,LCD_COLOR_B,m);
	
}*/



// shows current IR set on top of main screen
void drawSet (void)
{
	set_font(SMALLFONT);
	draw_block (0, 0, 99, 9, LCD_COLOR_B, DRAW_ERASE);
	draw_string (0,0,getSetName (irDevTab.device[irDevTab.active].encoder, irDevTab.device[irDevTab.active].set), LCD_COLOR_B, DRAW_PUT);
}


// 
void drawMainscreen (void)
{
	//~ set_font(SMALLFONT);	// done in drawSet()
	
	lcd_fill(0);
	draw_logo();

	// top bar
	drawSet();

/*	is updated in main loop
	// time
#if FF_LARGE_IDLE_CLOCK
	printTime (6,20, (struct time_ *)&rtc.time, 1);
#else
	printTime (86,152, (struct time_ *)&rtc.time, 0);
	printDate(0,152,(struct date_ *)&rtc.date,0);
#endif
*/
	showBat(0xff);			// battery indicator
	redraw_symbols();		// 
}


//unsigned char i;

/** let the LPC2200 sleep until the next IRQ
 *
 * This function tries to enter the sleep mode and stays
 * there until the next interrupt wakes up the CPU.
 * If that is not possible, the function returns immediately.
 */
extern unsigned char sact;
void cpu_idle ()
{
	/* fall through if a key was pressed */
	if (ANYKEY)
		return;
	
	//fall through if UART0 Scratch pad register is not empty
	//OpenOCD can set this register with "mwb 0xE000C01C 0x01" to disable idle and power down mode 
	if(U0SCR)
		return;
		
	return;

	/* only idle mode instead of power down when:
	 *	* backlight on
	 *	* IR transmission
	 *	* RF is activated
	 *	* sound is playing
	 *	* I2C access
	 *	* SYS_NOPDOWN in sysInfo is set
	 */
	if (bl_val != 0 || key_state != KEY_IDLE ||
			(RF.state > RFidle) ||
	  (SID.flags & SIDenable) || sact || i2ccurrentjob || (sysInfo & SYS_NOPDOWN))
	{
		if(sysInfo & SYS_NOIDLE)
			return;
		/* WARNING: no code reuse here:
		 * PCON = <dynamic_variable> causes timing problems */
		EXTINT = 0x08;
		PCON = PCON_IDLE;
		PLLFEED = 0xAA;
		PLLFEED = 0x55;
	}
	else
	{
		refresh_symbols(0xff);
		EXTINT = 0x08;
		sysInfo |= SYS_NOIDLE;
		PCON = PCON_PDOWN;
		PLLFEED = 0xAA;
		PLLFEED = 0x55;
	}
}	// cpu_idle

int main(void)
{
	sysInfo = SYS_NOIDLE;

	SCS |= 0x01;
	FIODIR0 |= ((1<<21) | (1<<11) | (1<<6) | (1<<23) | (1<<19) | (1<<17));
	FIODIR0 |= (1<<12);
	FIOSET0 |= (1<<12);
	FIOCLR0 |= (1<<4);

	setSpeed(SPEED_60);
	BFS_Mount();		// flash file system

	lcd_init(0);
	serial_init();

	startTimerIRQ();
	startADC();

	initKeys();

	initSound();
	//startSoundIRQ();

	initIR();
	startIrIRQ();

	PWM_init();

	RF_init();
	load_RF_setting();
	startRFIRQ();
	RF_changestate(RFwor);

	initI2C();
	initRTC();
	startRtcIRQ();

	enableIRQ();

	//testmenu_init();
	init_menu();

	initBacklight();

	oldkeys[0] = keys[0];
	oldkeys[1] = keys[0];

	key_state = KEY_IDLE;

	set_font(BOLDFONT);

	load_setting();		// display settings
	load_RC_setting();	// learned remote codes
	
	// recorded raw IR commands
	{
		struct RAWset_ RAWset;
		unsigned char x;
		unsigned long RAWcmdbase;

		RAWcmdbase = FLASH1_BASE +(secaddr[0]<<1);	// data address
		x=memcmp((void*)RAWcmdbase,"RC01",4);		// check ID

		if(!x)										// 
		{
			memcpy(&RAWset,(void *)RAWcmdbase,sizeof(struct RAWset_));
			RAWset.name[7] = 0;
			BFS_SaveFile(BFS_ID_RAWslot0, sizeof(struct RAWset_), (unsigned char*) &RAWset);
			eraseSector(1,0);
		}
	}

	setEncoder(irDevTab.device[irDevTab.active].encoder, irDevTab.device[irDevTab.active].set);

	drawMainscreen();
	ask_for_time(0);

/*
	playSound((unsigned char*)sound1_data, sound1_len);
	waitSound();
	playSound((unsigned char*)sound2_data, sound2_len);
*/

	// main loop
	while (1)
	{
		// detect keyboard action (keyboard matrix scanned by interrupt)
		if(keys[0] != oldkeys[0] || keys[1] != oldkeys[1])
		{
			oldkeys[0] = keys[0];
			oldkeys[1] = keys[1];
			sysInfo |= SYS_KEY;
		}

		// determine keyboard actions on main screen
		switch(key_state)
		{
			case KEY_IDLE:
				if(sysInfo & SYS_KEY)
				{
					sysInfo &= ~SYS_KEY;
					if(KEY_Betty)
					{
						setBacklight(BL_AUTO);	// activate backlight
						menu_exec(&mainMenu);	// select menu
						setEncoder(irDevTab.device[irDevTab.active].encoder, irDevTab.device[irDevTab.active].set);
						drawMainscreen();		// draw new menu
						while(KEY_Exit);		// block until
					}
					//~ else if(KEY_2)
					//~ {
						//setSpeed(SPEED_30);
					//~ }
					//~ else if(KEY_3)
					//~ {
						//setSpeed(SPEED_60);
					//~ }
					else if(KEY_A || KEY_B || KEY_C || KEY_D)
					{
						// select encoder
						unsigned char x;
						//playSound((unsigned char*)sound3_data, sound3_len);
						x=0;
						if (KEY_B) x=1;
						if (KEY_C) x=2;
						if (KEY_D) x=3;

						if (setEncoder(irDevTab.device[x].encoder, irDevTab.device[x].set)) {
							setBacklight(BL_AUTO);
							irDevTab.active = x;
							//~ drawMainscreen();
							drawSet();
						}
					}

					// (last-minute detection of regular key?)
					if((keys[0] != 0) || (keys[1] != 0))
						key_state = KEY_PRESS;
				}
				break;
			
			case KEY_PRESS:
				irSend(getCode());
				key_state = KEY_HOLD;
				if(!(T1TCR & 0x02))
					draw_symbol(90,0,7,(unsigned char*)symIR,LCD_COLOR_B,DRAW_PUT);
//				autorepeat = 0;
				break;
			
			case KEY_HOLD:
//				if(autorepeat >= AUTO_TIMEOUT)
					irRepeat();
				//draw_ir(90, 0, DRAW_XOR);
				if(keys[0] == 0 && keys[1] == 0)
					key_state = KEY_RELEASE;

				break;
			
			case KEY_RELEASE:
				if(!(T1TCR & 0x02))
					draw_symbol(90,0,7,(unsigned char*)symIR,LCD_COLOR_B,DRAW_ERASE);
				irStop();
				key_state = KEY_IDLE;
				break;
		}

		cpu_idle();	// let the LPC2200 sleep until the next IRQ

		// display time 
		if (timeInfo & TIMECHANGED)
		{
#if FF_LARGE_IDLE_CLOCK
			// time large, no date
			printTime(  6,  20, (struct time_ *)&rtc.time, 1);
#else
			// time and date small
			printTime( 86, 152, (struct time_ *)&rtc.time, 0);
			printDate(  0, 152, (struct date_ *)&rtc.date, 0);
#endif
			timeInfo &= ~TIMECHANGED;	// clear update flag
		}

	}	// while (1)
	return 0;	// never returns
}	// main
