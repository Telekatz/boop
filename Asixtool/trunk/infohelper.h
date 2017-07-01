/*
    infohelper.h - info-display routines (host)
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

#ifndef TOOL_INFOHELPER
#define TOOL_INFOHELPER

extern char verbosity;

int printError(char *err);
int printErrorS(char *err, char *xtra);
int printInfoS(unsigned char level, char *msg, char *xtra);
int printInfoH(unsigned char level, char *msg, unsigned int val);
int printProgress(unsigned char level, char *msg, float cval, float maxval);

#endif
