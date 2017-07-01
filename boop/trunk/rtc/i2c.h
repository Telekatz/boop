/*
    i2c.h - 
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

#ifndef I2C_H
#define I2C_H

#define AA		0x04
#define SI		0x08
#define STO		0x10
#define STA		0x20
#define I2EN	0x40

extern struct i2cjob_ dummyjob;

typedef void(*i2c_fn)(struct i2cjob_*);

struct i2cjob_ {
	unsigned char*		buffer;
	i2c_fn				onfinish; 
	unsigned char		len;
	unsigned char		pos;
	unsigned char		byteaddr;
	unsigned char		action;
};

#define I2C_write		0
#define I2C_read		1
#define I2C_done		2
#define I2C_error		3

volatile struct i2cjob_* volatile i2ccurrentjob; 

unsigned char i2c_jobexec(struct i2cjob_* job);
void initI2C(void);

#endif
