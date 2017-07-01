/*
    serirq.c - serial interrupr
    Copyright (C) 2009

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

#include "backlight.h"
#include "lpc2220.h"
#include "serial.h"
#include "buffer.h"

//volatile int uart0_active = 0;

void uart0_irq(void) {
	char st = U0IIR & 0x0f;

	// IRQ only valid when IIR_bit_0 == 0
	if (st & 1)
		return;

	switch (st) {
	case IIR_RLS: // Receive Line Status
		putHexW(U0LSR);
		serial_puts(" RLS?!\n");
		
		break;
	case IIR_RDA: // Receive Data Available
	case IIR_CTI: // Character Time-out indication
		if(uart0_incbuffer) {
			while(U0LSR & (1 << 0)) {
				cBufferAdd(uart0_incbuffer,U0RBR);
			}
		}
		else {
			setBacklight(BL_AUTO);
			serial_interactive();
		}
		break;
	case IIR_THRE:
		if(uart0_outcbuffer) {
			if(uart0_outcbuffer->len) {
				unsigned char x = 14;
				while(x-- && uart0_outcbuffer->len) {
					U0THR = cBufferGet(uart0_outcbuffer);
				}
			}
			else {
				U0IER &= ~IER_THRE;
			}
		}
		else {
			serial_puts("THRE?!\n");
		}
		
	}

}


