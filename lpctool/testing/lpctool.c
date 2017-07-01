#include "infohelper.h"
#include "lpctool_serial.h"
#include "bootcom.h"
#include "toolcom.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 9

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

unsigned long radr, f0adr, f1adr, gadr;

void printHeader(void)
{
	printf("\nlpctool V%i.%i\n\n", VERSION_MAJOR, VERSION_MINOR);
	printf("This Software is free Software under GNU GPL 3 or later\n");
	printf("(c) 2007 Ch. Klippel <ck@mamalala.net>\n\n");
}

void printUsage(void)
{
	printf("usage: lpctool [-d NAME] [-i] [-r [NAME]] [-R ADDRESS] [-X [ADDRESS]] [-a [NAME] [-A ADRDESS]] [-e] [-b [NAME] [-B ADDRESS]] [-E] [-s] [-S] [-c] [-C] [-v] [-V]\n\n");
	printf("	-d NAME		specify serial device, default is /dev/ttyS0\n");
	printf("	-i 		use compiled-in flash-tool instead of external ram image file\n");
	printf("	-r NAME		specify file for ram,  default is ramtool.bin\n");
	printf("	-R ADDRESS 	optionally specifies the start-address to load into ram\n");
	printf("			default is 0x40000200 - must be specified in hex\n");
	printf("	-X ADDRESS	set the execution address for the uploaded ram image\n");
	printf("			defualt is 0x40000200, omiting the address disables execution\n");
	printf("	-a NAME		specify file for flash #0, default is flash0.bin\n");
	printf("	-A ADDRESS 	optionally specifies the start-address to load into flash #0\n");
	printf("			default is 0x80000000 - must be specified in hex\n");
	printf("	-e		completely erase the first flash before writing\n");
	printf("			default is no erase\n");
	printf("	-b NAME		specify file for flash #1, default is flash1.bin\n");
	printf("	-B ADDRESS 	optionally specifies the start-address to load into flash #1\n");
	printf("			default is 0x82000000 - must be specified in hex\n");
	printf("	-E		completely erase the second flash before writing\n");
	printf("			defualt is no erase\n");
	printf("	-S		only erase the first sector in flash before writing\n");
	printf("			this is a lot faster if only the first sector is used\n");
	printf("	-c		write flash0 in chunk mode instead of write-all\n");
	printf("	-C		write flash1 in chunk mode instead of write-all\n");
	printf("			in chunk mode, parts with 4 consectuive 0xFFFFFFFF or more\n");
	printf("			in the image are skipped. speeds upload when the image has\n");
	printf("			gaps. defualt is off\n");
	printf("	-N		do not reset the device after uploading to flash\n");
	printf("			by default the device gets reset after upload\n");
	printf("	-? 		print this help\n");
	printf("	-v 		print version\n");
	printf("	-V 		be more verbose\n");
	printf("\n");
	printf("at least -r or -i must be given to be usefull. everything in [] is optional\n");
	printf("examples:\n");
	printf("\n");
	printf("to just load ramtool.bin, starting at 0x40000200, using serial device /dev/ttyS0 :\n");
	printf("lpcload -r\n\n");
	printf("to do the same, but using serial device /dev/ttyS1 instead :\n");
	printf("lpcload -r -d /dev/ttyS1\n\n");
	printf("using the file image.bin as source for the ram, use /dev/ttyS1 as serial device :\n");
	printf("lpcload -r image.bin -d /dev/ttyS1\n");
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

const char hval[] = { "0123456789ABCDEF" };

void printHexC(unsigned char num)
{
	printf("%c%c", hval[(num&0xF0)>>4], hval[num&0x0F]);
}

void printHexS(unsigned short num)
{
	printHexC((num & 0xFF00) >> 8);
	printHexC(num & 0x00FF);
}

void printHexW(unsigned long num)
{
	printHexS((num & 0xFFFF0000) >> 16);
	printHexS(num & 0x0000FFFF);
}
/*
void enterBootLD(void)
{
	setDTR(1);
	usleep(10);
	setRTS(1);
	usleep(100);
	setRTS(0);
	usleep(10);
	setDTR(0);
	usleep(100);
}

void resetFB(void)
{
	setRTS(1);
	usleep(100);
	setRTS(0);
}
*/

// eINT1 low on RESET to enter bootloader
void enterBootLD(void)
{
	setRTS(1);
	usleep(1000);
	setDTR(1);
	usleep(1000);
	setDTR(0);
	usleep(1000);
	setRTS(0);
	usleep(1000);
}

void resetFB(void)
{
	setDTR(1);
	usleep(100);
	setDTR(0);
}

int main(int argc,char **argv)
{
	char sname[256];
	char rname[256];
	char f0name[256];
	char f1name[256];
	unsigned char i, cmd;
	unsigned int uflags;
	int fd;

	sprintf(sname,"/dev/ttyS0");
	sprintf(rname,"ramtool.bin");
	sprintf(f0name,"flash0.bin");
	sprintf(f1name,"flash1.bin");

	verbosity = 0;
	uflags = 0;

	radr  = 0x40000200;
	f0adr = 0x80000000;
	f1adr = 0x82000000;
	gadr  = radr;

	printHeader();

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

			switch(cmd)
			{
				case 'v':
						verbosity = 2;
						break;
				case 'V':
						goto end;
				case 'd':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							strncpy(sname,argv[++i],256);
							printInfoS(1,"using serial device",sname);
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
							printInfoS(1,"using ram image",rname);
						}
						uflags |= 0x01;
						break;
				case 'R':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							radr = parseHex(argv[++i]);
							printInfoH(1,"loading ram @",radr);
						}
						else
							printInfoH(1,"used -R but no address given, using default", radr);
						break;
				case 'a':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							strncpy(f0name,argv[++i],256);
							printInfoS(1,"using flash #0 image",f0name);
						}
						uflags |= 0x02;
						break;
				case 'A':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							f0adr = parseHex(argv[++i]);
							printInfoH(1,"setting address for flash #0 @",f0adr);
						}
						else
							printInfoH(1,"used -A but no address given, using default",f0adr);
						break;
				case 'b':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							strncpy(f1name,argv[++i],256);
							printInfoS(1,"using flash #1 image",f1name);
						}
						uflags |= 0x04;
						break;
				case 'B':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							f1adr = parseHex(argv[++i]);
							printInfoH(1,"setting address for flash #1 @",f1adr);
						}
						else
							printInfoH(1,"used -B but no address given, using default",f1adr);
						break;
				case 'X':
						if((argc > i+1) && (argv[i+1][0] != '-'))
						{
							gadr = parseHex(argv[++i]);
							printInfoH(1,"setting ram execute address @",gadr);
							uflags |= 0x08;
						}
						else
							printInfoS(1,"disabling auto-run",0);
						break;
				case 'e':
						printInfoS(1,"erasing flash #0 before write",0);
						uflags |= 0x10;
						break;
				case 'E':
						printInfoS(1,"erasing flash #1 before write",0);
						uflags |= 0x20;
						break;
				case 'S':
						printInfoS(1,"only erasing start sector",0);
						uflags |= 0x40;
						break;
				case 's':
						printInfoS(1,"auto-erasing sectors as needed",0);
						uflags |= 0x80;
						break;
				case 'N':
						printInfoS(1,"don't reset after flash upload is done",0);
						uflags |= 0x100;
						break;
				case 'c':
						printInfoS(1,"using chunk mode for flash #0",0);
						uflags |= 0x200;
						break;
				case 'C':
						printInfoS(1,"using chunk mode for flash #1",0);
						uflags |= 0x400;
						break;
				case 'i':
						uflags |= 0x800;
						break;
				default:
						goto bailout;
			}
		}
		else
		{
bailout:
			printUsage();
			goto end;
		}
	}

	if(!uflags)
	{
		printUsage();
		printInfoS(0,"\nnothing to do!",0);
		goto end;
	}

	printInfoS(2,"opening serial device",sname);
	if(openSerial(sname) <= 0)
	{
		printError("Error opening serial device");
		goto end;
	}

	if((uflags & 0x01) || (uflags & 0x800))
	{
		if(!(uflags & 0x800))
		{
			printInfoS(2,"opening ram image",rname);
			fd = open(rname,O_RDONLY);
			if(fd < 0)
			{
				printErrorS("can't open",rname);
				goto end;
			}
		}
		else
		{
			fd = 0;
		}

		#if TOOLBAUD <= 38400
		printInfoS(2,"wait for buggy serial ports",0);
		setRTS(0);
		usleep(100000);
		#endif

		printInfoS(2,"reset into bootmode",0);
		enterBootLD();

		printInfoS(2,"opening bootloader",0);
		if(!openBootLoader(10000))
		{
			printError("Error opening BootLoader");
			goto end;
		}

		printInfoS(2,"unlocking bootloader",0);
		if(!unlockBootLoader())
		{
			printError("Error unlocking BootLoader");
			goto end;
		}
		if(!(uflags & 0x800))
		{
			printInfoS(1,"uploading ram image",rname);
		}
		else
		{
			printInfoS(1,"uploading compiled-in ram image",0);
		}
		if(uploadRAM(fd, radr))
		{
			printInfoS(1,"ram uploaded",0);
			if(!(uflags&0x08))
			{
				printInfoH(1,"executing code at ram address",gadr);
				if(!goBootLoader(gadr))
				{
					printError("Error starting from ram");
					goto end;
				}
				printInfoH(2,"code execution started",gadr);
			}
		}
		if(fd)
		{
			close(fd);
		}
	}

	//~ reconfSerial(TOOLBAUD, 0, 5);

	if(uflags & 0x06)
	{
		printInfoS(2,"opening ram tool",0);
		if(openRamTool())
			printInfoS(2,"opened ramtool",0);
		else
			goto end;

		if(uflags & 0x02)
		{
			printInfoS(2,"opening flash0 image",f0name);
			fd = open(f0name,O_RDONLY);
			if(fd < 0)
			{
				printErrorS("cant open",f0name);
				goto end;
			}
			printInfoS(1,"uploading flash0 image",f0name);
			if(uflags & 0x200)
			{
				if(uploadChunks(fd, f0adr, (uflags & 0x50)))
					printInfoS(1,"flash #0 uploaded",0);
				else
					goto end;
			}
			else
			{
				if(uploadFlash(fd, f0adr, (uflags & 0x50)))
					printInfoS(1,"flash #0 uploaded",0);
				else
					goto end;
			}

            close(fd);
		}



		if(uflags & 0x04)
		{
			printInfoS(2,"opening flash1 image",f1name);
			fd = open(f1name,O_RDONLY);
			if(fd < 0)
			{
				printErrorS("cant open",f1name);
				goto end;
			}
			printInfoS(1,"uploading flash1 image",f1name);

			if(uflags & 0x400)
			{
				if(uploadChunks(fd, f1adr, (uflags & 0x60)))
					printInfoS(1,"flash #1 uploaded",0);
				else
					goto end;
			}
			else
			{
				if(uploadFlash(fd, f1adr, (uflags & 0x60)))
					printInfoS(1,"flash #1 uploaded",0);
				else
					goto end;
			}
            close(fd);
		}

		if((uflags & 06) && !(uflags & 0x100))
		{
			printInfoS(2,"reset device",0);
			resetFB();
		}
	}

end:
	if(fd)
	{
		printInfoS(2,"closing file descriptor",0);
		close(fd);
	}
	printInfoS(2,"closing serial device",sname);
	closeSerial();
	printf("\n");
	return 0;
}
