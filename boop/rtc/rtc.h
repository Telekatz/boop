/*
    rtc.h -
    Copyright (C) 2008  <telekatz@gmx.de>

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

#ifndef RTC_H
#define RTC_H

#include "i2c.h"

#define RTCslaveaddr	0xA2

//PCF8563 registers
#define RTCREG_CONTROL_STATUS_1		0x00
#define RTCREG_CONTROL_STATUS_2		0x01
#define RTCREG_SECOND				0x02
#define RTCREG_MINUTES				0x03
#define RTCREG_HOURS				0x04
#define RTCREG_DAYS					0x05
#define RTCREG_WEEKDAYS				0x06
#define RTCREG_MONTHS_CENTURY		0x07
#define RTCREG_YEARS				0x08
#define RTCREG_MINUTE_ALARM			0x09
#define RTCREG_HOUR_ALARM			0x0A
#define RTCREG_DAY_ALARM			0x0B
#define RTCREG_WEEKDAY_ALARM		0x0C
#define RTCREG_CLKOUT_CONTROL		0x0D
#define RTCREG_TIMER_CONTROL		0x0E
#define RTCREG_TIMER				0x0F

//RTCREG_CONTROL_STATUS_2
#define TIMER_INT_EN		(1<<0)
#define ALARM_INT_EN		(1<<1)
#define TIMER_FLAG			(1<<2)
#define ALARM_FLAG			(1<<3)

struct time_
{
	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	unsigned char tag;
};

struct date_
{
	unsigned char day;
	unsigned char weekday;
	unsigned char month;
	unsigned char year;
};

struct rtc_
{
	struct date_ date;
	struct time_ time;
};

volatile struct rtc_ rtc;

volatile unsigned char timeInfo;
#define TIMECHANGED  	(1<<0)
#define ALARM_ENABLED	(1<<1)
#define SETNEWTIME		(1<<2)
#define TIME_ACCURATE	(1<<3)
#define ALARM_PENDING	(1<<3)


struct i2cjob_ i2cintjob;
unsigned char rtcbuffer[8];

const unsigned char days[14];

void initRTC(void);
unsigned char getTimeDate(struct time_* t, struct date_ *d);
void setTimeDate(struct time_* t, struct date_ *d);
void printTime (unsigned char x, unsigned char y, struct time_* t, unsigned char format);
void printDate(unsigned char x, unsigned char y, struct date_* d, unsigned char format);
void rtcIRQ (void);
void startRtcIRQ(void);
void change_clock(void);
void calcweekday(struct date_* d);

/// Send the time via RF packet @see RF_PACKET_TIME to dest
void send_time(unsigned char dest, unsigned char wor);

/// Request to receive time via RF packet @see RF_PACKET_TIME from dest
void ask_for_time(unsigned char dest);

void calcBCD(unsigned char* value, char a, char max, char min);
unsigned char checkBCD(unsigned char value, char max, char min);

#endif
