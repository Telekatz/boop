/*
    infohelper.c - info-display routines (host)
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

#include <stdio.h>

#include "infohelper.h"

char verbosity;

int printError(char *err)
{
	printf("ERROR: %s\n",err);
	return 0;
}

int printErrorS(char *err, char *xtra)
{
	if(xtra)
		printf("ERROR: %s %s\n", err, xtra);
	else
		printf("ERROR: %s\n", err);
	return 0;
}

int printInfoS(unsigned char level, char *msg, char *xtra)
{
	if(level <= verbosity)
	{
	if(xtra)
		printf("Info : %s %s\n", msg, xtra);
	else
		printf("Info : %s\n", msg);
		return 1;
	}
	return 0;
}

int printInfoH(unsigned char level, char *msg, unsigned int val)
{
	if(level <= verbosity)
	{
		printf("Info : %s 0x%08X\n", msg, val);
		return 1;
	}
	return 0;
}

const char heartbeat[] = { ".oOo" };

int printProgress(unsigned char level, char *msg, float cval, float maxval)
{
	unsigned int cur, cnt;

	static int beat = 0;

	if(level <= verbosity)
	{
		cur = (35 / maxval) * cval;
	
		cnt = 35-cur;
	
		printf("Progress: [");
	
		while(cur--)
			printf("*");

		if(cnt)
			printf("%c",heartbeat[beat]);
		else
			printf("*");
	
		while(cnt--)
			printf(" ");
	
		printf("\b]\r");
		fflush(stdout);
		beat++;
		beat &= 0x03;
	}
	return 0;
}

