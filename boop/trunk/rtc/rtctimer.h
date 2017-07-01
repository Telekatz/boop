/*
    rtctimer.h - real-time clock timer 
	Copyright (C) 2010	<telekatz@gmx.de>

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

#ifndef RTCTIMER_H
#define RTCTIMER_H

struct RTCtimer_ {
	unsigned char flags;
	unsigned char minute;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned char repeat;		//!< repeat alarm: Bit0=So...Bit6=Sa
	unsigned char arg[4];		//!< argument for timer execution
};
#endif

//RTCtimer_ flags
#define RTCTIMER_TYPEMASK	0x07
#define RTCTIMER_ALARM		0x00
#define RTCTIMER_KEY		0x01
#define RTCTIMER_MACRO		0x02
#define RTCTIMER_FUNCTION	0x03
#define RTCTIMER_OBSOLETE	(1<<6)	
#define RTCTIMER_SLOTFREE	(1<<7)

void RTCtimer_edit(void);
