/*
    main.c - TAE MSP
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
#include <signal.h>
//#include <mspgcc/util.h>                // delay()
#include "rf.h"


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
  
  P1OUT = 0;//CC_CS | CC1100_POWER | CC1100_APOWER | EXTRA_CURRENT;
  P1DIR = CC_SCLK | CC_MOSI | CC_CS | WD_RST | CC1100_POWER | CC1100_APOWER | EXTRA_CURRENT;
  P1SEL = 0;
  P1IES  =  CC_MISO;
  P1IFG  =  0;
  
  P2OUT = 0;
  P2DIR = ACLK | VEGA_MISO | ADC_IN1 | LINE_CONTROL;
  P2SEL = ACLK | XIN | XOUT;
  
  
  delay(0xffff);
  P1OUT = CC_CS | CC1100_POWER | EXTRA_CURRENT;
  P2OUT = LINE_CONTROL;
  delay(0xffff);
  
  init_clock();
  
  RF_init();
  RF_sendWOR(0);
  
  eint();
  
  for (;;)
  {
	
	TACTL = TASSEL_1 //ACLK 
		| MC_2 /* Timer A mode control: 2 - Continous up */
		| TACLR;
	while(TAR < 0x3200);
	P1OUT ^= WD_RST;
	//P1OUT ^= CC_SCLK;
	//delay(0x8000);
	//RF_sendWOR(0);

  }
}

