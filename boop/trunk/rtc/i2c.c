/*
    i2c.c - 
    Copyright (C) 2008  

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
#include "i2c.h"
#include "irq.h"
#include "rtc.h"
#include "global.h"

volatile struct i2cjob_* volatile i2ccurrentjob;

unsigned char i2c_jobexec(struct i2cjob_* job) {
	
	VICVectCntl3 &= ~VIC_SLOT_EN;
	while(i2ccurrentjob);
	i2ccurrentjob = job;
	I2CONSET = STA;
	while(i2ccurrentjob);	
	VICVectCntl3 |= VIC_SLOT_EN;
	return 1;
}

void i2cIRQ(void) {
	unsigned char state;
	
	state = I2STAT;
	i2ccurrentjob->buffer;
	
	switch(state) {
		case 0x08:	//A START condition has been transmitted
			I2DAT = RTCslaveaddr | I2C_write;
			I2CONSET = AA;
			I2CONCLR = SI | STA;
			i2ccurrentjob->pos = 0;
			break;
		case 0x18:	//SLA+W has been transmitted; ACK has been received.
			I2DAT = i2ccurrentjob->byteaddr;
			I2CONSET = AA;
			I2CONCLR = SI;
			break;
		case 0x28:	//Data byte in I2DAT has been transmitted; ACK has been received.
			switch(i2ccurrentjob->action) {
				case I2C_read:
					I2CONSET = STA;
					I2CONCLR = AA | SI;
					break;
				case I2C_write:
					if(i2ccurrentjob->len > i2ccurrentjob->pos) {
						I2DAT = i2ccurrentjob->buffer[i2ccurrentjob->pos++];
						I2CONSET = AA;
						I2CONCLR = SI;
					}
					else
						i2ccurrentjob->action = I2C_done;
					break;
				default:
					i2ccurrentjob->action = I2C_error;
			}
			break;
		case 0x10:	//A repeated START condition has been transmitted	
			I2DAT = RTCslaveaddr | I2C_read;
			I2CONSET = AA;
			I2CONCLR = SI | STA;
			break;
		case 0x40:	//SLA+R has been transmitted; ACK has been received.	
			I2CONSET = AA;
			I2CONCLR = SI;
			break;
		case 0x50:	//Data byte has been received; ACK has been returned.
			i2ccurrentjob->buffer[i2ccurrentjob->pos++] = I2DAT;
			if(i2ccurrentjob->len > i2ccurrentjob->pos+1)
				I2CONSET = AA;
			else
				I2CONCLR = AA;
			I2CONCLR = SI;
			break;
		case 0x58:	//Data byte has been received; NOT ACK has been returned.
			i2ccurrentjob->buffer[i2ccurrentjob->pos] = I2DAT;
			i2ccurrentjob->action = I2C_done;
			break;
		case 0x20:	//SLA+W has been transmitted; NOT ACK has been received.
		case 0x30:	//Data byte in I2DAT has been transmitted; NOT ACK has been received.
			if(i2ccurrentjob->pos)
				i2ccurrentjob->pos--;
		default:
			i2ccurrentjob->action = I2C_error;
			break;
	}
	
	if((i2ccurrentjob->action == I2C_done) || (i2ccurrentjob->action == I2C_error)) {
		I2CONSET = STO;
		I2CONCLR = SI;
		if(i2ccurrentjob->onfinish) {
			struct i2cjob_* job;
			job =  (struct i2cjob_*)i2ccurrentjob;
			i2ccurrentjob = 0;
			job->onfinish(job);
		}
		else
			i2ccurrentjob = 0;
	}
}

void initI2C(void) {
	PINSEL0 |= (1<<4) | (1<<6);
	I2SCLL = 19;	
	I2SCLH = 19;
	
	I2CONSET = I2EN;
	i2ccurrentjob = 0;
	
	VICVectAddr4 = (unsigned long)&(i2cIRQ);
	VICVectCntl4 = VIC_SLOT_EN | INT_SRC_I2C;
	VICIntEnable = INT_I2C;
}

