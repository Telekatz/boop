/*
    hardware.h - TAE hardware
    Copyright (C) 2009  <telekatz@gmx.de>

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

#ifndef HARDWARE_H
#define HARDWARE_H

#define DCO_RSEL       7
#define DCO_ACLK_DELTA (((F_CPU) + 2047) / 4096)

//Port 1
#define CC_CS         (1<<0)
#define CC_SCLK       (1<<1)
#define CC_MOSI       (1<<2)
#define CC_MISO       (1<<3)
#define WD_RST        (1<<4)
#define CC1100_POWER  (1<<5)
#define EXTRA_CURRENT (1<<6)
#define CC1100_APOWER (1<<7)

//Port2
#define ACLK          (1<<0)
#define VEGA_CLK      (1<<1)
#define VEGA_MOSI     (1<<2)
#define VEGA_MISO     (1<<3)
#define ADC_IN1       (1<<4)
#define LINE_CONTROL  (1<<5)
#define XIN           (1<<6)
#define XOUT          (1<<7)


void init_clock(void);

static void __inline__ delay(register unsigned int n)
{
  __asm__ __volatile__ (
  "1: \n"
  " dec %[n] \n"
  " nop \n"
  " jne 1b \n"
        : [n] "+r"(n));
}

#endif
