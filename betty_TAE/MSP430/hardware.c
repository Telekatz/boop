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

#include "hardware.h"
#include <msp430.h>

void init_clock(void) {
	unsigned char step;
	int last, delta;


  DCOCTL  = 0x70;
  BCSCTL1 = XT2OFF | DIVA_3 | (DCO_RSEL & 0xF);
  BCSCTL2 = 0x00;

  last = 0;
  TACTL = TASSEL_2  /* Timer A clock source select: 2 - SMCLK */
	| MC_2 /* Timer A mode control: 2 - Continous up */
	| TACLR;
  TACCTL2 = CM1  /* Capture mode 1 */ 
	| CCIS_1 | SCS | CAP;
  for (step = 0x40; step > 0; step >>= 1) {
    while (!(TACCTL2 & CCIFG));

    delta = TACCR2 - last;
    if (delta < DCO_ACLK_DELTA) {
		DCOCTL += step;
    }
    else {
		DCOCTL -= step;
    }

    last = TACCR2;
    TACCTL2 &= ~CCIFG;
  }

  BCSCTL1 &= ~DIVA_3;

  BCSCTL2 = DIVS_1;

  TACTL = TASSEL1 /* Timer A clock source select 0 */
	| MC_2 /* Timer A mode control: 2 - Continous up */
	| TACLR;

}

