/*
    bootcom.h - com routines for the asix on.chip bootloader
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

#ifndef BOOTCOM_H
#define BOOTCOM_H

int readFlash(char *name, unsigned int start, unsigned int len);
unsigned int writeFlash(char *name, unsigned int start, unsigned int len);
int eraseFlash(unsigned int adr);
int eraseSector(unsigned int adr);


#endif
