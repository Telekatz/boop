/*
	rtc.c

	real-time clock PCF8563 by NXP Semiconductors
		I2C Bus
		Interrupt

	display date / time
	set date / time
	date calculations
	settings menu

	Copyright (C) 2008	<telekatz@gmx.de>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.	If not, see <http://www.gnu.org/licenses/>.
*/

#include "lpc2220.h"
#include "rtc.h"
#include "i2c.h"
#include "lcd.h"
#include "fonty.h"
#include "drawDigit.h"
#include "irq.h"
#include "adc.h"
#include "keyboard.h"
#include "rf.h"
#include "global.h"
#include "rtc.h"
#include "controls.h"
#include "timerfuncs.h"

const unsigned long default_time[1] = {0x00000000};
const unsigned char default_date[4] = {0x01, 0x02, 0x01, 0x08};
const unsigned char months[36] = "JANFEBMRZAPRMAIJUNJULAUGSEPOKTNOVDEZ";
const unsigned char days[14] = "SOMODIMIDOFRSA";

void initRTC (void)
{
	struct i2cjob_ job;
	unsigned char buffer[4];

	job.buffer = buffer;
	job.onfinish = 0;

	//init rtc
	job.len = 2;
	job.byteaddr = RTCREG_CONTROL_STATUS_1;
	job.buffer[0] = 0x08;
	job.buffer[1] = 0x01;
	do {
		job.action = I2C_write;
		i2c_jobexec(&job);
	} while(!(job.action == I2C_done));

	//init rtc alarm
	job.len = 4;
	job.byteaddr = RTCREG_MINUTE_ALARM;;
	job.buffer[0] = (1<<7);
	job.buffer[1] = (1<<7);
	job.buffer[2] = (1<<7);
	job.buffer[3] = (1<<7);
	job.action = I2C_write;
	i2c_jobexec(&job);

	//init rtc timer
	job.len = 2;
	job.byteaddr = RTCREG_TIMER_CONTROL;
	job.buffer[0] = 0x82;
	job.buffer[1] = 0x01;
	job.action = I2C_write;
	i2c_jobexec(&job);

	timeInfo =0;

	setTimeDate((struct time_*) default_time,(struct date_*) default_date);
}

unsigned char getTimeDate (struct time_* t, struct date_ *d)
{
	struct i2cjob_ job;
	unsigned char buffer[7];

	job.buffer = buffer;
	job.onfinish = 0;
	job.len = 7;
	job.byteaddr = RTCREG_SECOND;
	job.action = I2C_read;
	i2c_jobexec(&job);

	t->second	= buffer[0] & 0x7F;
	t->minute	= buffer[1] & 0x7F;
	t->hour		= buffer[2] & 0x3F;
	d->day		= buffer[3] & 0x3F;
	d->weekday	= buffer[4] & 0x07;
	d->month	= buffer[5] & 0x1F;
	d->year		= buffer[6];
	
	return buffer[0] & 0x80;	/// \return VL - clock integrity. 0 is good.
}

void setTimeDate (struct time_ *t, struct date_ *d)
{
	struct i2cjob_ job;
	unsigned char buffer[7];

	job.buffer = buffer;
	job.onfinish = 0;
	memcpy(job.buffer,(unsigned char *)t,3);
	memcpy(job.buffer+3,(unsigned char *)d,4);
	job.len = 7;
	job.byteaddr = RTCREG_SECOND;
	job.action = I2C_write;
	i2c_jobexec(&job);

}

void printTime (unsigned char x, unsigned char y, struct time_* t, unsigned char format)
{
	if (format)
	{
		// 00:00
		unsigned char w = 25;
		unsigned char h = 45;

		drawDigit (x+0,		y, w, h, (t->hour >> 4) & 0x0F,	5, LCD_COLOR_B, DRAW_PUT);
		drawDigit (x+w+2,		y, w, h, t->hour & 0x0F,			5, LCD_COLOR_B, DRAW_PUT);
		drawDigit (x+2*w	+15, y, w, h, (t->minute >> 4) & 0x0F, 5, LCD_COLOR_B, DRAW_PUT);
		drawDigit (x+3*w+2+15, y, w, h, t->minute & 0x0F,		5, LCD_COLOR_B, DRAW_PUT);

		draw_char (x+2*w+10, y+h/2, ':',		LCD_COLOR_B, DRAW_PUT);
	}
	else
	{
		unsigned char f = activefont;

		set_font (SMALLFONT);
		draw_block (x,y,42,7,LCD_COLOR_B,DRAW_ERASE);
		draw_hexC (x+30,y,t->second, LCD_COLOR_B, DRAW_PUT);
		draw_hexC (x+15,y,t->minute, LCD_COLOR_B, DRAW_PUT);
		draw_hexC (x+0,y,t->hour, LCD_COLOR_B, DRAW_PUT);
		draw_char (x+12,y,':', LCD_COLOR_B, DRAW_PUT);
		draw_char (x+27,y,':', LCD_COLOR_B, DRAW_PUT);
		set_font (f);
	}
}


void printDate (unsigned char x, unsigned char y, struct date_* d, unsigned char format)
{
	unsigned char f;
	char s[17];

	f=activefont;
	set_font(SMALLFONT);

	switch(format) {
		case 1:
			draw_block (x,y,80,7,LCD_COLOR_B,DRAW_ERASE);
			s[ 0] = days[d->weekday * 2];
			s[ 1] = days[(d->weekday * 2) + 1];
			s[ 2] = ',';
			s[ 3] = ' ';
			s[ 4] = ((d->day & 0xf0)>>4) + 0x30;
			s[ 5] = ((d->day & 0x0f)) + 0x30;
			s[ 6] = '.';
			s[ 7] = ' ';
			s[ 8] = months[(d->month - 1) * 3];
			s[ 9] = months[((d->month - 1) * 3)+1];
			s[10] = months[((d->month - 1) * 3)+2];
			s[11] = ' ';
			s[12] = '2';
			s[13] = '0';
			s[14] = ((d->year & 0xf0)>>4) + 0x30;
			s[15] = ((d->year & 0x0f)) + 0x30;
			s[16] = 0;
			draw_string (x, y, s, LCD_COLOR_B, DRAW_PUT);
			break;
		default:
			draw_block(x,y,57,7,3,DRAW_ERASE);
			draw_char (x+0,y,days[d->weekday * 2], LCD_COLOR_B, DRAW_PUT);
			draw_char (x+6,y,days[ (d->weekday * 2) + 1], LCD_COLOR_B, DRAW_PUT);
			draw_hexC (x+15,y,d->day, LCD_COLOR_B, DRAW_PUT);
			draw_char (x+27,y,'.', LCD_COLOR_B, DRAW_PUT);
			draw_hexC (x+30,y,d->month, LCD_COLOR_B, DRAW_PUT);
			draw_char (x+42,y,'.', LCD_COLOR_B, DRAW_PUT);
			draw_hexC (x+45,y,d->year, LCD_COLOR_B, DRAW_PUT);
	}

	set_font(f);
}

void rtcIRQreenable (struct i2cjob_* job)
{
	EXTINT |= (1<<2);
	VICIntEnable = INT_EINT2;
}

void rtcAckIRQ (struct i2cjob_* job)
{
	if (timeInfo & SETNEWTIME) {
		rtcbuffer[1] = rtc.time.second;
		rtcbuffer[2] = rtc.time.minute;
		rtcbuffer[3] = rtc.time.hour;
		rtcbuffer[4] = rtc.date.day;
		rtcbuffer[5] = rtc.date.weekday;
		rtcbuffer[6] = rtc.date.month;
		rtcbuffer[7] = rtc.date.year;
		i2cintjob.len = 8;
		timeInfo &= ~SETNEWTIME;
		timeInfo |= TIME_ACCURATE;	// doesn't mean it's accurate, just that it was set
	}
	else {
		rtc.time.second		= job->buffer[1] & 0x7F;
		rtc.time.minute		= job->buffer[2] & 0x7F;
		rtc.time.hour		= job->buffer[3] & 0x3F;
		rtc.date.day		= job->buffer[4] & 0x3F;
		rtc.date.weekday	= job->buffer[5] & 0x07;
		rtc.date.month		= job->buffer[6] & 0x1F;
		rtc.date.year		= job->buffer[7];
		i2cintjob.len = 1;
	}

	if(job->buffer[0] & ALARM_FLAG)
		timeInfo |= ALARM_PENDING;

	timeInfo |= TIMECHANGED;
	T0overflow = 0;
	
	i2cintjob.byteaddr = RTCREG_CONTROL_STATUS_2;
	i2cintjob.buffer = rtcbuffer;
	i2cintjob.buffer[0] = 0x01 | (timeInfo & ALARM_ENABLED);
	i2cintjob.onfinish = rtcIRQreenable;
	i2cintjob.action = I2C_write;
	i2ccurrentjob = &i2cintjob;
	I2CONSET = STA;

}

void rtcIRQ (void)
{
	EXTINT = (1<<2);

	i2cintjob.buffer = rtcbuffer;
	i2cintjob.onfinish = rtcAckIRQ;
	i2cintjob.len = 8;
	i2cintjob.byteaddr = RTCREG_CONTROL_STATUS_2;
	i2cintjob.action = I2C_read;
	i2ccurrentjob = &i2cintjob;
	I2CONSET = STA;

	if (!(PLLSTAT & (1<<8))) {
		if ((rtc.time.second & 0x0f) == 9)
			calcBat(0xff);
		if ((rtc.time.second & 0x0f) == 0)
			showBat(0xff);
	}

	VICSoftIntClr = INT_EINT2;
	VICIntEnClr = INT_EINT2;
}

void startRtcIRQ (void)
{
	PINSEL0 |= (1<<31);
	EXTWAKE |= (1<<2);
	EXTINT |= (1<<2);

	VICVectAddr3 = (unsigned long)&(rtcIRQ);
	VICVectCntl3 = VIC_SLOT_EN | INT_SRC_EINT2;
	VICIntEnable = INT_EINT2;
}

void calcBCD (unsigned char* value, char a, char max, char min)
{
	signed char x;
	unsigned char y;

	x = (((*value & 0xf0)>>4) * 10 + (*value & 0x0f)) + a;
	if (x > max)
		x -= (1 + max - min);
	if (x < min)
		x += (1 + max - min);
	y = x % 10;
	y |= ((x-y) / 10)<<4;
	*value = y;
}

unsigned char checkBCD (unsigned char value, char max, char min)
{
	signed char x;
	unsigned char y;

	x = (((value & 0xf0)>>4) * 10 + (value & 0x0f));
	if (x > max)
		x = max;
	if (x < min)
		x = min;
	y = x % 10;
	y |= ((x-y) / 10)<<4;
	return(y);
}

void send_time (unsigned char dest, unsigned char wor)
{
	struct RFendpoint_* cur_ep;

	cur_ep = openEP((unsigned char*)&rtc,0, packet_time);
	if (cur_ep)
	{
		cur_ep->dest = dest;
		cur_ep->bufferlen = 7;

		cur_ep->flags |= EPenabled | EPoutput | EPnewdata | EPonce;
		if(wor)
			cur_ep->flags |= EPsendwor;
		RF_changestate(RFtx);
	}
}

void ask_for_time (unsigned char dest)
{
	struct RFendpoint_* cur_ep;

	cur_ep = openEP(0,0, packet_time);
	if(cur_ep)
    {
		cur_ep->dest = dest;
		cur_ep->data[0] = 0xFF;
		cur_ep->bufferlen = 1;

		cur_ep->flags |= EPenabled | EPoutput | EPnewdata | EPonce | EPsendwor;

		RF_changestate(RFtx);
		RF_timed_rx(400);
	}
}

void settime_click (FORM* form, CONTROL* control)
{
	VICIntEnClr = INT_EINT2;
	rtc.date = ((DATEBOX*)(form->controls[0]))->date;
	rtc.time = ((TIMEBOX*)(form->controls[1]))->time;
	send_time(0,1);
	setTimeDate((struct time_ *)&rtc.time, (struct date_ *)&rtc.date);
	timeInfo |= TIME_ACCURATE;
	VICIntEnable = INT_EINT2;

	form->flags |= BC_Form_exit;
}


// Settings > Clock menu
//
void change_clock (void)
{
	const LABEL label1		= {BC_Labelnotab,0,15,100,10,BOLDFONT,0,"Datum",0};
	DATEBOX datebox1		= {BC_Datebox,0,25,58,9,SMALLFONT,0,{0x00,0x00,0x00,0x00},NULL,NULL};
	const LABEL label2		= {BC_Labelnotab,0,37,100,10,BOLDFONT,0,"Uhrzeit",0};
	TIMEBOX timebox1		= {BC_Timebox,0,47,43,9,SMALLFONT,0,{0x00,0x00,0x00,0x00},NULL,NULL};
	const BUTTON button1	= {BC_Button,0,63,50,15,SMALLFONT,0,"Speichern",settime_click};

	timebox1.time	= rtc.time;
	datebox1.date = rtc.date;

	CONTROL* controls[5] = {	(CONTROL*)&datebox1,
								(CONTROL*)&timebox1,
								(CONTROL*)&label1,
								(CONTROL*)&label2,
								(CONTROL*)&button1};

	FORM form	= {"Datum & Uhrzeit"," ",0,0,0,0,controls,0,5,0,0};

	form_exec(&form);
}

/* Globale zur Verfügung stehenden Variablen:
//Variablen für die Zeit:
u08 dayofweek = 1, day = 1, month = 1;
u16 year = 2004;
u08 isschaltjahr;
*/

unsigned char isschaltjahr(unsigned int year) {

	if ( (((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0) )
		return 1;
	else
		return 0;
}

void calcweekday(struct date_* d) {

	unsigned int tagdesjahres;
	unsigned int jahrdiff;
	unsigned int tempwochentag;
	unsigned char day;
	unsigned char month;
	unsigned char year;

	day = (d->day >> 4)*10 + (d->day & 0x0f);
	month = (d->month >> 4)*10 + (d->month & 0x0f);
	year = 2000+(d->year >> 4)*10 + (d->year & 0x0f);

	tagdesjahres = 31*(month-1);
	if (month > 2) tagdesjahres = tagdesjahres -3 +isschaltjahr(year);
	if (month > 4) tagdesjahres--;
	if (month > 6) tagdesjahres--;
	if (month > 9) tagdesjahres--;
	if (month > 11) tagdesjahres--;
	tagdesjahres += day;

	jahrdiff = year -2000;
	tempwochentag = jahrdiff + (jahrdiff-1) / 4 - (jahrdiff-1) / 100 + (jahrdiff-1) / 400 + tagdesjahres;
	if (jahrdiff > 0)
		tempwochentag++;
	tempwochentag = (tempwochentag % 7)+3;
	if (tempwochentag > 6)
		tempwochentag -= 7;
	d->weekday =	tempwochentag;
}
