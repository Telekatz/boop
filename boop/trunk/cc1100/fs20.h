/*
    fs20.h - FS20 functions
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

#ifndef FS20_H
#define FS20_H

#define extensionbit			0x20
#define PREAMBLELEN_FS20		10
#define PREAMBLELEN_WEATHER		7
#define PREAMBLELEN_WEATHER2	20

void fs20_decoder(void);

#endif
