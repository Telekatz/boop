/*
    timerirq.c - timekeeping and calllbacks
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

#include "lpc2220.h"
#include "timerirq.h"
#include "timerfuncs.h"
#include "lcd.h"

#define TIMER_PRESCALER		20

struct CB callbacks[MAX_CB];
unsigned long* timeouts[MAX_TO];
unsigned char timerPrescaler = TIMER_PRESCALER;
unsigned char bl_val, cmp_val;	// backlight PWM

// wird alle 0.25 ms aufgerufen (s. startTimerIRQ() in timerfuncs)
// bearbeitet eingetragene "timer"
void __attribute__ ((section(".text.fastcode"))) timerIRQ(void)
{
	// backlight pwm
	cmp_val += bl_val;
	if (cmp_val >= 63)
	{
		FIODIR0 |= (1<<4);		// sck0/P0.4
		cmp_val -= 63;
	}
	else
	{
		FIODIR0 &= ~(1<<4);		// sck0/P0.4
	}


	if(--timerPrescaler) {
		T0IR = 1;
		return;
	}
	timerPrescaler = TIMER_PRESCALER;

	//5 msec intervall time
	unsigned int cnt;
	struct CB *cur_cb;

	for(cnt=0; cnt<MAX_CB; cnt++)
	{
		cur_cb = &callbacks[cnt];
		if(cur_cb->state & SLOT_USED)
		{
			if(!(cur_cb->state & SLOT_HALT))
			{
				cur_cb->cur_intervall++;
				if(cur_cb->cur_intervall >= cur_cb->intervall)
				{
					if(cur_cb->exec)
						(cur_cb->exec)(cnt);		// run callback function
					cur_cb->cur_intervall = 0;
				}
			}
		}
	}
	
	for(cnt = 0; cnt < MAX_TO; cnt++) {
		if(timeouts[cnt]) {
			if(*timeouts[cnt])
				(*timeouts[cnt])--;
			else
				timeouts[cnt] = 0;
		}
	}
	
	if(oldsymbols != symbols) {
		volatile cb_fn x;
		x=refresh_symbols;
		x(0xff);
	}
	
	T0overflow++;
	T0IR = 1;
}
