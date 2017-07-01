/*
    timerfuncs.h - timer control functions
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

#ifndef TIMERFUNCS_H
#define TIMERFUNCS_H

////////////////////////////////////////////////
///@Attention: Timer are using 5msec intervall!
////////////////////////////////////////////////
#define TIME_MSEC2TICKS(t)   (t/5) 

#include "timerirq.h"

volatile unsigned long T0overflow;

void startTimerIRQ(void);

/** 
@brief Alocate a timer, set the time but do *NOT* start.

       Use @see startCB to start the timer

@Param  Timer expiry callback, time intervall

@return index of the allocated timer on success
        0xFF is no timer left
*/
unsigned int addTimerCB(cb_fn cb, unsigned short intervall);
unsigned int removeTimerCB(unsigned int cb);
void startCB(unsigned int cb);
void stopCB(unsigned int cb);
void setCBIntervall(unsigned int cb, unsigned short intervall);
void wait5ms(void);
void waitus(unsigned long d);
unsigned char addTimeout(volatile unsigned long* counter, unsigned long value);
unsigned char removeTimeout(volatile unsigned long* counter);

#endif
