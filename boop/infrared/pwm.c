/*
    pwm.c - pwm control
    Copyright (C) 2017  <telekatz@gmx.de>

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

#include "global.h"
#include "lpc2220.h"

void PWM_init(void)
{
	//Set pin P0.21 IR_OUT as PWM
	PINSEL1 &= ~(3 << (10));      // IO
	PINSEL1 |=  (1 << (10));       // PWM5

	PWMTC = 0;        //Timer Counter
	PWMPR = 0;        //Prescale Register
	PWMPC = 0;        //Prescale Counter

	PWMMR0 = 416;     // pwm rate
	PWMMR5 = 0;     // pwm value

	PWMLER = 0x21;    //Latch Enable
	PWMMCR = 0x02;    //Match Control
	PWMPCR |= (1<<13);
	PWMTCR = 0x03;
	PWMTCR = 0x09;

}

void PWM_set_frequency(unsigned long f)
{
	PWMMR0 = 15000000 / f;
	PWMLER |= 0x01;    //Latch Enable

}

void PWM_set_IR_duty_cycle(unsigned char d) {
	PWMMR5 = (PWMMR0 * d) / 100;
	PWMLER |= 0x20;
}

