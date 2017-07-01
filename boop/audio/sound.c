/*
    sound.c - sound core routines
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

#include "irq.h"
#include "sound.h"
#include "soundirq.h"
#include "lpc2220.h"

#define AUTO_OFF_TIME	0x4000


extern unsigned int tval;
extern unsigned char bl_val, cmp_val;
extern unsigned int slen;
extern unsigned char *sdata;
extern volatile unsigned char sact;
extern char out1;
extern char last_sample;
extern unsigned char sound_shutdown;

unsigned char timeout;
unsigned int auto_timeout;

void startSoundIRQ(void)
{
	timeout = 0;
	auto_timeout = 0;
	out1 = 0;

	PWMTC = 0;
//	PWMPR = 0;
	PWMPR = 7;
	PWMMR0 = 0x1E6;	// pwm rate
	PWMMR2 = 0x00;	// pwm value
	PWMLER = 0x05;
	PWMMCR = 0x03;
	PWMPCR = (1<<10);
	PWMTCR = 0x03;
	PWMTCR = 0x09;

	//VICVectAddr0 = (unsigned long)&(soundIRQ);
	//VICVectCntl0 = VIC_SLOT_EN | INT_SRC_PWM;
	VICIntSelect |= INT_PWM;
	VICIntEnable = INT_PWM;
}

void initSound(void)
{
	slen = 0;
	sdata = 0;
	sact = 0;
	tval = 0;
	last_sample = 0;
	bl_val = 0x3F;
}

void switchSound(unsigned char onoff)
{
	if(onoff)
	{
		sound_shutdown = 0;
		PINSEL0 &= ~(3 << (2 * SND_PWM)); 			// IO
		PINSEL0 |=  (2 << (2 * SND_PWM)); 			// PWM
		PINSEL0 &= ~(3 << (2 * SND_EN));			// IO
		PINSEL0 &= ~(3 << (2 * SND_ON));			// IO
		FIODIR0 |= (1<<SND_ON) | (1<<SND_EN);
		FIOSET0  = (1<<SND_EN);
		FIOCLR0  = (1<<SND_ON);
	}
	else
	{
		FIOCLR0 |= (1<<SND_EN);
		sound_shutdown = 0xff;
	}
}

void playSound(unsigned char *sound, unsigned long len)
{
	PWMPR = 0;
	PWMPC = 0;
	sdata = sound;
	slen = len;
	tval = 0;
	sact = 1;
	switchSound(SOUND_ON);
}

void stopSound(void)
{
	switchSound(SOUND_OFF);
	PWMPR = 7;
	sact = 0;
}

void waitSound(void)
{
	while(sact) { }
}

