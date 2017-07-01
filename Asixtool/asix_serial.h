/*
    asix-serial.h - serial port routines (host) to access the asix chip
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

#ifndef ASIX_SERIAL_H
#define ASIX_DERIAL_H

int openSerial(char *dev, unsigned int speed);
int closeSerial(void);
int reconfSerial(unsigned int baud, unsigned char flow, unsigned char tout);
int sendBuf(unsigned int len, char *buf);
int getBuf(unsigned int len, char *buf);
void setDTR(unsigned char val);
void setRTS(unsigned char val);

#endif
