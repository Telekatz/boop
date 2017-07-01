/*
    main.c - main program
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

#include "stdio.h"
#include "asix_serial.h"
#include "infohelper.h"
#include "fcntl.h"
#include "unistd.h"
#include <string.h>
#include "bootcom.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

extern char verbosity;
extern int do_erase;

unsigned long radr, f0adr, f0len;

void printHeader(void)
{
	printf("\nasixtool V%i.%i\n\n", VERSION_MAJOR, VERSION_MINOR);
	printf("This Software is free Software under GNU GPL 3 or later\n");
	printf("(c) 2007 Ch. Klippel <ck@mamalala.net>\n\n");
}

void printUsage(void)
{
	printf("usage: asixtool [-d NAME] [-a [NAME] [-A ADRDESS]] [-e] [-S] [-c] [-N]\n\n");
	printf("	-d NAME		specify serial device, default is /dev/ttyS0\n");
	printf("	-r NAME		specify file to read flash into, default is flash_dump.bin\n");
	printf("	-a NAME		specify file to write to flash, default is flash.bin\n");
	printf("	-A ADDRESS 	optionally specifies the start-address to load into/read from flash\n");
	printf("			default is 0x000000 - must be specified in hex\n");
	printf("	-L ADDRESS 	optionally specifies the maximum number of bytes to load into/read from flash\n");
	printf("			default is 0x080000 - must be specified in hex\n");
	printf("	-e		completely erase the flash before writing\n");
	printf("			default is no erase\n");
	printf("	-s		erase flash sectors as needed.\n");
	printf("			this is a lot faster if only a few sectors are used\n");
	printf("	-? 		print this help\n");
	printf("\n");
	printf("ATTENTION: commands are order-sensitive! for example, to set an address, you MUST\n");
	printf("specify the address with -A before you specify any of the -r or -w commands. this\n");
	printf("allows you to upload serveral binary files to serveral places with one command.\n");
}

unsigned long parseHex(char *buf)
{
	unsigned char cnt;
	unsigned long adr;

	cnt = 0;
	adr = 0;

	while(buf[cnt])
	{
		adr <<= 4;
		if(buf[cnt] >= '0' && buf[cnt] <= '9')
		{
			adr += (buf[cnt] - '0');
		}
		else if(buf[cnt] >= 'A' && buf[cnt] <= 'F')
		{
			adr += (buf[cnt] - 'A' + 10);
		}
		else if(buf[cnt] >= 'a' && buf[cnt] <= 'f')
		{
			adr += (buf[cnt] - 'a' + 10);
		}
		else if(buf[cnt] == 'x' || buf[cnt] == 'X')
		{
			adr = 0;
		}
		else
		{
		}
		cnt++;
	}
	return adr;
}

int main(int argc,char **argv)
{
	char sname[256];
	char rname[256];
	char f0name[256];
	int isOpen;
	int numw;

	unsigned int baud;

	unsigned char i, cmd; 

	baud = 115200;
	do_erase = 0;
	numw = 0;
	isOpen = 0;
	sprintf(sname,"/dev/ttyS0");
	sprintf(rname,"flash_dump.bin");
	sprintf(f0name,"flash.bin");

	f0adr = 0x000000;
	f0len = 0x080000;

	printHeader();
	verbosity = 1;

	if(argc <= 1)
		goto bailout;

rescan:
	for(i=1;i<argc;i++) 
	{
		if(argv[i][0] == '-')
		{
			cmd = 0;
			if(argv[i][1] == '-')
			{
				cmd = argv[i][2];
			}
			else
			{
				cmd = argv[i][1];
			}
			if(isOpen == 0)
			{
				switch(cmd)
				{
					case 'h':
						baud = 921600;
						break;
					case 'd':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							strncpy(sname,argv[++i],256);
							printInfoS(1,"opening serial device",sname);
							if(isOpen)
							{
								if(closeSerial() == 0)
								{
									isOpen = 0;
								}
								else
								{
									printError("cant close port");
									return 0;
								}
							}
							if(openSerial(sname, baud))
							{
								isOpen = 1;
							}
							else
							{
								printError("cant open port");
								return 0;
							}
						}
						else
						{
							printError("Error in device argument");
							goto bailout;
						}
						break;
				}
			}
			else
			{
				switch(cmd)
				{
					case 'd':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							strncpy(sname,argv[++i],256);
							printInfoS(1,"closing serial device",sname);
							if(closeSerial() == 0)
							{
								isOpen = 0;
							}
							else
							{
								printError("cant close port");
								return 0;
							}
							printInfoS(1,"opening serial device",sname);
							if(openSerial(sname, baud))
							{
								isOpen = 1;
							}
							else
							{
								printError("cant open port");
								return 0;
							}
						}
						else
						{
							printError("Error in device argument");
							goto bailout;
						}
						break;
					case 'r':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							strncpy(rname,argv[++i],256);
						}
						printInfoS(1,"reading flash image",rname);
						numw = readFlash(rname, f0adr, f0len); 
						if(numw)
						{
							printInfoS(1, "done reading memory                     ",0);
						}
						else
						{
							printErrorS("Error in reading memory-image",rname);
							return 0;
						}
						break;
					case 'a':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							strncpy(f0name,argv[++i],256);
						}
						printInfoS(1,"writing flash image",f0name);
						numw = writeFlash(f0name, f0adr, f0len); 
						if(numw)
						{
							printInfoS(1, "done writing memory                     ",0);
							f0len = numw;
						}
						else
						{
							printErrorS("Error in writing memory-image",f0name);
							return 0;
						}
						break;
					case 'A':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							f0adr = parseHex(argv[++i]);
							printInfoH(1,"setting address for flash @",f0adr);
						}
						else
							printInfoH(1,"used -A but no address given, using default",f0adr);
						f0len = 0x080000;
						break;
					case 'L':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							f0len = parseHex(argv[++i]);
							printInfoH(1,"setting length for flash @",f0len);
						}
						else
							printInfoH(1,"used -L but no address given, using default",f0len);
						break;
					case 'e':
							printInfoS(1,"erasing flash before write",0);
							if(!eraseFlash(f0adr))
							{
								printError("cant erase flash");
								return 0;
							}

							break;
					case 's':
							printInfoS(1,"erasing sectors as needed",0);
							do_erase = 1;
							break;
				}
			}
		}
		else
		{
bailout:
			printUsage();
			return 0;
		}
	}

	if((isOpen == 0) && (argc > 1))
	{
		printInfoS(1,"using serial device",sname);
		if(openSerial(sname, baud))
		{
			isOpen = 1;
			goto rescan;
		}
	}
	else if(isOpen != 0)
	{
		printInfoS(1,"closing serial device",sname);
		closeSerial();
	}

	return 1;
}
