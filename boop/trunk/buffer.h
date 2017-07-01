/*
    buffer.h - functions for buffers
    Copyright (C) 2008 Telekatz <telekatz@gmx.de>

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

#ifndef BUFFER_H
#define BUFFER_H

struct cBuffer_ {
	unsigned char* data;
	unsigned short size;
	unsigned short len;
	unsigned short index;
};

void cBufferInit(struct cBuffer_ *buffer, unsigned char *start, unsigned short size);
void cBufferFlush(struct cBuffer_ *buffer);
unsigned char cBufferAdd(struct cBuffer_ *buffer, unsigned char data);
unsigned short cBufferAddBuffer(struct cBuffer_ *buffer, unsigned char* data, unsigned short len);
unsigned char cBufferGet(struct cBuffer_ *buffer);
unsigned short cBufferGetBuffer(struct cBuffer_ *buffer, unsigned char* data, unsigned short len);
#endif
