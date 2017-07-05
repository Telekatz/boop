/*
    bootcom.c - com routines for the asix on.chip bootloader
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

#include "bootcom.h"
#include "stdio.h"
#include "asix_serial.h"
#include "infohelper.h"
#include "fcntl.h"
#include "unistd.h"

#define MAXSECT	11

extern char verbosity;
char buf[5];
char fbuf[0x080000];
int do_erase;

unsigned int sectors[MAXSECT+1] = {
	0x00000,
	0x04000,
	0x06000,
	0x08000,
	0x10000,
	0x20000,
	0x30000,
	0x40000,
	0x50000,
	0x60000,
	0x70000,
	0x80000
};

int findSect(unsigned long adr)
{
	int secnum = 0;
	while((sectors[secnum] < adr) && (secnum < MAXSECT))
	{
		secnum++;
	}

	if(sectors[secnum] > adr)
		return secnum-1;
	else
		return secnum;
}


void dumpBuf()
{
	printf("%02X %02X %02X %02X %02X\n",(unsigned char)buf[0],(unsigned char)buf[1],(unsigned char)buf[2],(unsigned char)buf[3],(unsigned char)buf[4]); 
}

void sendCMD(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e) 
{
	buf[0] = a;
	buf[1] = b;
	buf[2] = c;
	buf[3] = d;
	buf[4] = e;
	sendBuf(5, buf);
}
int setAccessTime(unsigned char val)
{
	sendCMD(0x81, 0x00, 0x00, 0x00, val);
	getBuf(1, buf);

	if(buf[0] == val)
		return 1;

	return 0;
}

int setIntervallTime(unsigned char val)
{
	sendCMD(0x82, 0x00, 0x00, 0x00, val);
	getBuf(1, buf);

	if(buf[0] == val)
		return 1;

	return 0;
}

int readFlash(char *name, unsigned int start, unsigned int len)
{
	FILE *outfile;
	unsigned int ret, count, cdone;
	unsigned char rollover = 0;

	outfile = fopen(name,"w");
	
	sendCMD(0x00, 0x00, 0x00, 0x00, 0x00);
	ret = getBuf(5, buf);
	cdone = 0;
	count = start+len;

	printInfoH(1, "bytes to read: ",len);

	for(;start<count;start++)
	{
		sendCMD(0x00, start & 0xFF, (start & 0xFF00) >> 8, (start & 0xFF0000) >> 16, 0x00);

		ret = getBuf(1, buf);
		
		if(ret == 1)
		{
			fprintf(outfile, "%c",(unsigned char)buf[0]);
			fflush(outfile);
		}
		else
		{
			printf("FAIL: %02i - %06X - %02X\n",ret, start, (unsigned char) buf[0]);
			fprintf(outfile, "%c",(unsigned char)buf[0]);
			fflush(outfile);
		}

		cdone++;

		if(rollover == 0)
			printProgress(0, "reading", cdone, len);

		rollover++;
		rollover &= 0x3F;
	}
	printProgress(0, "reading", cdone, len);
	fclose(outfile);
	return 1;	
}

void resetFlash(unsigned int start)
{
	sendCMD(0x80, 0x33, 0x03, (start & 0xFF0000) >> 16, 0xF0);
	getBuf(1, buf);
}

void closeWrite(unsigned int start)
{
	sendCMD(0x80, 0x33, 0x03, (start & 0xFF0000) >> 16, 0x90);
	getBuf(1, buf);
	resetFlash(start);
}

int eraseFlash(unsigned int adr)
{

	unsigned char base = (adr & 0xFF0000) >> 16;

	printInfoH(1, "erasing flash at ", adr & 0x00FF0000);

	sendCMD(0x80, 0x55, 0x05, base, 0xAA);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, 0xAA, 0x02, base, 0x55);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, 0x55, 0x05, base, 0x80);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, 0x55, 0x05, base, 0xAA);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, 0xAA, 0x02, base, 0x55);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, 0x55, 0x05, base, 0x10);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

waitErase:
	sendCMD(0x00, 0x00, 0x00, base, 0x00);
	if(!getBuf(1, buf))
	{
		goto bailOut1;
	}
	if(buf[0] == 0x0A || buf[0] == 0x4E)
		goto waitErase;
	if((unsigned char)buf[0] != 0xFF)
		goto bailOut2;

	printInfoS(1, "successfully erased flash", 0);
	resetFlash(adr);
	return 1;

bailOut:
	return printError("cant issue chip erase sequence, aborting!");
bailOut1:
	return printError("no response during chip erase active, aborting");
bailOut2:
	return printError("cant erase chip, aborting");
}


int eraseSector(unsigned int adr)
{

	unsigned char baseH = (adr & 0xFF0000) >> 16;
	unsigned char baseM = (adr & 0xFF00) >> 8;
	unsigned char baseL = adr & 0xFF;

	printInfoH(1, "erasing sector at ", adr);

	sendCMD(0x80, 0x55, 0x05, 0x00, 0xAA);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, 0xAA, 0x02, 0x00, 0x55);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, 0x55, 0x05, 0x00, 0x80);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, 0x55, 0x05, 0x00, 0xAA);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, 0xAA, 0x02, 0x00, 0x55);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

	sendCMD(0x80, baseL, baseM, baseH, 0x30);
	if(!getBuf(1, buf))
	{
		goto bailOut;
	}

waitErase:
	sendCMD(0x00, baseL, baseM, baseH, 0x00);
	if(!getBuf(1, buf))
	{
		goto bailOut1;
	}

	if(buf[0] == 0x0A || buf[0] == 0x4E)
		goto waitErase;



	if((unsigned char)buf[0] != 0xFF)
		goto bailOut2;

	printInfoS(1, "successfully erased sector",0);
	resetFlash(adr);
	return 1;

bailOut:
	return printError("cant issue sector erase sequence");
bailOut1:
	return printError("no response during secttor erase active");
bailOut2:
	return printError("cant erase sector");
}


unsigned int writeFlash(char *name, unsigned int start, unsigned int len)
{
	int infile, flen;
	unsigned int ret, count, cdone;
	unsigned char rollover = 0;

	sendCMD(0x00, 0x00, 0x00, 0x00, 0x00);
	ret = getBuf(1, buf);

	infile = open(name,O_RDONLY);
	flen = read(infile, fbuf, 0x80000);

	if(flen < len)
	{
		printInfoH(1, "adjusting number of bytes to ",flen);
		len = flen;
	}
	
	cdone = 0;
	count = start+len;

	if(do_erase)
	{
		unsigned char startsec, endsec;
		startsec = findSect(start);
		endsec = findSect(start+len-1);
nextsect:
		if(!eraseSector(sectors[startsec]))
		{
			return(printError("error at auto-erase"));
		}
		if(startsec != endsec)
		{
			startsec++;
			goto nextsect;
		}
	}

	printInfoH(1, "bytes to write: ",len);

retry:

	sendCMD(0x80, 0x55, 0x05, (start & 0xFF0000) >> 16, 0xAA);
	ret = getBuf(1, buf);

	sendCMD(0x80, 0xAA, 0x02, (start & 0xFF0000) >> 16, 0x55);
	ret = getBuf(1, buf);

	sendCMD(0x80, 0x55, 0x05, (start & 0xFF0000) >> 16, 0x20);
	ret = getBuf(1, buf);

	for(;start<count;start++)
	{
		sendCMD(0x80, 0x33, 0x03, (start & 0xFF0000) >> 16, 0xA0);
		ret = getBuf(1, buf);

		sendCMD(0xC0, start & 0xFF, (start & 0xFF00) >> 8, (start & 0xFF0000) >> 16, (unsigned char) fbuf[cdone]);
		ret = getBuf(2, buf);
	
		if(ret == 1)
		{
			sleep(1);
			closeWrite(start);
			printf("\nretrying\n");
			goto retry;
		}
	
		if((ret != 2))
		{
			closeWrite(start);
			return printError("error during write: short response");
		}

		if((unsigned char)fbuf[cdone] != (unsigned char)buf[1])
		{
			closeWrite(start);
			return printError("error during write: data mismatch");
		}

		cdone++;

		if(rollover == 0)
			printProgress(0, "writing", cdone, len);

		rollover++;
		rollover &= 0x3F;
	}
	printProgress(0, "writing", cdone, len);
	closeWrite(start);
	close(infile);
	return len;	
}
