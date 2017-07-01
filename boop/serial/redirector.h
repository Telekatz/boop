/*
    redirector.h - serial port redirector
    Copyright (C) 2009  Telekatz <telekatz@gmx.de>

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

#ifndef REDIRECTOR_H
#define REDIRECTOR_H

#define redir_ACK		0
#define redir_SET		1
#define redir_START		2
#define redir_STOP		3
#define redir_PING		4

#define baud_1200		0
#define baud_2400		1
#define baud_4800		2
#define baud_9600		3
#define baud_14400		4
#define baud_19200		5
#define baud_28800		6
#define baud_38400		7
#define baud_57600		8
#define baud_115200		9

void uarto_redirector(void);

#endif
