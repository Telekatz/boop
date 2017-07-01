/*
    buffer.c - functions for buffers
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

#include "buffer.h"
#include "global.h"
#include "irq.h"

void cBufferInit(struct cBuffer_ *buffer, unsigned char *start, unsigned short size) {
	
	unsigned cpsr;
	cpsr = disableIRQ();

	buffer->data = start;
	buffer->size = size;
	buffer->len = 0;
	buffer->index = 0;
	
	restoreIRQ(cpsr);
}

void cBufferFlush(struct cBuffer_ *buffer) {
	
	unsigned cpsr;
	cpsr = disableIRQ();

	buffer->len = 0;
	buffer->index = 0;
	
	restoreIRQ(cpsr);
}

unsigned char cBufferAdd(struct cBuffer_ *buffer, unsigned char data) {
	
	unsigned cpsr;
	cpsr = disableIRQ();
	
	if(buffer->size > buffer->len) {
		buffer->data[(buffer->index + buffer->len++) % buffer->size] = data;
		restoreIRQ(cpsr);
		return 1;
	}
	restoreIRQ(cpsr);
	return 0;
	
}

unsigned short cBufferAddBuffer(struct cBuffer_ *buffer, unsigned char* data, unsigned short len) {

	unsigned short lenw;
	unsigned short start;
	unsigned short end;
	
	unsigned cpsr;
	cpsr = disableIRQ();
	
	lenw = 0;
	
	if(buffer->size > buffer->len) {
		start = (buffer->index + buffer->len) % buffer->size;
		if(buffer->index)
			end = buffer->index;
		else
			end = buffer->size;
		
		if(end <= start) {
			if(buffer->size - start < len) {
				lenw = buffer->size - start;
				memcpy(&buffer->data[start],data,lenw);
			}
			else {
				memcpy(&buffer->data[start],data,len);
				buffer->len += len;
				restoreIRQ(cpsr);
				return len;
			}
			start = 0;
		}

		if(end - start < len - lenw) {
			memcpy(&buffer->data[start],&data[lenw],end - start);
			buffer->len = buffer->size;
			restoreIRQ(cpsr);
			return lenw + end - start;
		}
		else {
			memcpy(&buffer->data[start],&data[lenw],len-lenw);
			buffer->len += len;
			restoreIRQ(cpsr);
			return len;
		}
	}
	restoreIRQ(cpsr);
	return 0;
	
}

unsigned char cBufferGet(struct cBuffer_ *buffer) {

	unsigned char data = 0;
	
	unsigned cpsr;
	cpsr = disableIRQ();
	
	if(buffer->len) {
		data = buffer->data[buffer->index++];
		buffer->index %= buffer->size;
		buffer->len--;
	}
	restoreIRQ(cpsr);
	return data;
}

unsigned short cBufferGetBuffer(struct cBuffer_ *buffer, unsigned char* data, unsigned short len) {
	
	unsigned cpsr;
	cpsr = disableIRQ();
	
	if(buffer->len) {
		if(len > buffer->len)
			len = buffer->len;
		
		if(buffer->index + len <= buffer->size) {
			memcpy(data,&buffer->data[buffer->index],len);
		}
		else {
			memcpy(data,&buffer->data[buffer->index],buffer->size - buffer->index);
			memcpy(&data[buffer->size - buffer->index],buffer->data,len - (buffer->size - buffer->index));
		}
		buffer->index = (buffer->index + len) % buffer->size;
		buffer->len -= len;
		restoreIRQ(cpsr);
		return len;
		
	}
	restoreIRQ(cpsr);
	return 0;
	
}
