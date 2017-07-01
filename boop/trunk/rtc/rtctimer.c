/*
	rtctimer.c - real-time clock timer 
	Copyright (C) 2010	<telekatz@gmx.de>

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
#include "lcd.h"
#include "fonty.h"
#include "keyboard.h"
#include "global.h"
#include "controls.h"
#include "sid.h"

void initRTCtimer (void) {

}


void RTCtimer_timer (FORM* form)
{
	((TIMEBOX*)(form->controls[0]))->time = rtc.time;
	control_draw(form->controls[0],0);				//redraw timebox1
	if(timeInfo & ALARM_PENDING) {
		form->tag=9;
		timeInfo &= ~ALARM_PENDING;
	}
	if(form->tag) {
		if(form->tag & 0x01) beep(1);
		form->tag--;
	}
}

void RTCsave_click(FORM* form, CONTROL* control) {
	struct i2cjob_ job;
	unsigned char buffer[4];

	buffer[0] = ((TIMEBOX*)(form->controls[8]))->time.minute;
	buffer[1] = ((TIMEBOX*)(form->controls[8]))->time.hour | (1<<7);
	buffer[2] = (1<<7);
	buffer[3] = (1<<7);

	job.buffer = buffer;
	job.onfinish = 0;
	job.len = 4;
	job.byteaddr = RTCREG_MINUTE_ALARM;
	job.action = I2C_write;
	i2c_jobexec(&job);

	timeInfo |= ALARM_ENABLED;
}

void RTCtimer_edit(void) {
	
	const char* listbox1items[] = {"Alarm","Tastendruck","Makro","Funktion"};
	const char* listbox2items[] = {"Stunde","Tag","Monat"};
	
	TIMEBOX timebox1		= {BC_Timebox | BC_NoTabStop,85,151,43,8,SMALLFONT,0,{0x00,0x00,0x00,0x00},NULL,NULL};
	timebox1.time	= rtc.time;

	const LABEL label1		= {BC_Labelnotab,0,20,70,9,BOLDFONT,0,"Timer",0};
	const LABEL label2		= {BC_Labelnotab,0,32,70,9,BOLDFONT,0,"Typ",0};
	const LABEL label3		= {BC_Labelnotab,0,44,70,9,BOLDFONT,0,"Wiederholung",0};
	const LABEL label4		= {BC_Labelnotab,0,56,70,9,BOLDFONT,0,"Alarmzeit",0};
	
	NUMBOX numbox1			= {BC_Numbox,76,20,20,9,SMALLFONT,1,1,1,NULL,NULL};
	LISTBOX listbox1		= {BC_Listbox,76,32,50,10,SMALLFONT,0,1,(char**)listbox1items,NULL,NULL};
	LISTBOX listbox2		= {BC_Listbox,76,44,50,10,SMALLFONT,0,1,(char**)listbox2items,NULL,NULL};
	TIMEBOX timebox2		= {BC_Timebox,76,56,43,8,SMALLFONT,0,{0x00,0x00,0x00,0x00},NULL,NULL};
	
	const BUTTON button1	= {BC_Button,0,80,60,15,SMALLFONT,0,"Speichern",RTCsave_click};

	CONTROL* controls[] = {	(CONTROL*)&timebox1,
							(CONTROL*)&label1,
							(CONTROL*)&label2,
							(CONTROL*)&label3,
							(CONTROL*)&label4,
							(CONTROL*)&numbox1,
							(CONTROL*)&listbox1,
							(CONTROL*)&listbox2,
							(CONTROL*)&timebox2,
							(CONTROL*)&button1};
	
	timebox2.time	= rtc.time;

	FORM form  = {"Timer bearbeiten","",NULL,NULL,RTCtimer_timer,0,controls,0,NUM_OF_ELEMENTS(controls),0,200};
	form_exec(&form);
	
	timeInfo &= ~ALARM_ENABLED;
}
