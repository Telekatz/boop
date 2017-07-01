#include "infohelper.h"
#include "lpctool_serial.h"
#include "bootcom.h"
#include "tool_data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

unsigned int uuencode(unsigned char *ibuf, unsigned char *obuf, int len) 
{
	unsigned int slen, r,s;
	unsigned char q;
	slen = len;

	r = 0;
	s = 0;

	while(slen >= 45)
	{

		obuf[r++]=77;
		for(q=0;q<45;q+=3)
		{

			obuf[r++]=0x20+(( ibuf[s]>>2)&0x3f);
			obuf[r++]=0x20+(((ibuf[s]<<4) | (ibuf[s+1]>>4))&0x3f);
			s++;
			obuf[r++]=0x20+(((ibuf[s]<<2)|(ibuf[s+1]>>6))&0x3f);
			s++;
			obuf[r++]=0x20+(  ibuf[s++]&0x3f);
		}
		obuf[r++] = '\r';
		obuf[r++] = '\n';
		slen -= 45;
	}

	if(slen)
	{
		obuf[r++]=0x20+slen;
		for(q=0;q<slen;q+=3)
		{
				obuf[r++]=0x20+(( ibuf[s]>>2)&0x3f);
				obuf[r++]=0x20+(((ibuf[s]<<4) | (ibuf[s+1]>>4))&0x3f);
				s++;
				obuf[r++]=0x20+(((ibuf[s]<<2)|(ibuf[s+1]>>6))&0x3f);
				s++;
				obuf[r++]=0x20+(  ibuf[s++]&0x3f);
		}
		obuf[r++] = '\r';
		obuf[r++] = '\n';
	}

	return r;
}

void printBuf(unsigned char len, unsigned char *buf)
{
	unsigned char cnt;
	cnt = 0;
	while(len--)
		printf("%c",buf[cnt++]);
}

int getOK(unsigned char oktype)
{
	char buf[4];
	unsigned char q;

	switch(oktype)
	{
		case OK_LIT:
			q = getBuf(4, buf);
			if(strncmp(buf, "OK\r\n", 4))
			{
				return printError("wrong response, expected OK");
			}
			return 1;
			break;

		case OK_0:
			q = getBuf(3, buf);
			if(strncmp(buf, "0", 1))
			{
				return printError("wrong response, expected 0");
			}
			return 1;
			break;
	}
}

int openBootLoader(unsigned int cfreq)
{
	char buf[20];
	unsigned char q;	

	printInfoS(2,"initiating auto-baud",0);
	sendBuf(sprintf(buf, "?\r\n"), buf);

	printInfoS(2,"awaiting autobaud response",0);

	q = getBuf(14, buf);

	if(strncmp(buf, "Synchronized\r\n", 14))
	{
		return printError("autobaud detection failed");
	}

	printInfoS(2,"got response, answering",0);

	q = sendBufE(q, buf);

	if(!getOK(OK_LIT))
	{
		return printError("cant synchronize bootloader");
	}

	printInfoS(2,"setting frequency",0);

	sendBufE(sprintf(buf, "%i\r\n", cfreq), buf);

	if(!getOK(OK_LIT))
	{
		return printError("cant set frequency");
	}

	printInfoS(2,"switching baudrate",0);
	 
	sendBufE(sprintf(buf, "B 38400 1\r\n"), buf);

	if(!getOK(OK_0))
	{
		return printError("cant set baudrate");
	}

	reconfSerial(38400, 1, 10);

	printInfoS(2,"disabling echo",0);

	sendBufE(sprintf(buf, "A 0\r\n"), buf);

	if(!getOK(OK_0))
	{
		return printError("cant disable echo");
	}
	return 1;
}

int unlockBootLoader(void)
{
	char buf[20];

	sendBuf(sprintf(buf, "U 23130\r\n"), buf);

	if(!getOK(OK_0))
	{
		return printError("cant unlock bootloader");
	}
	return 1;

}

int goBootLoader(unsigned long addr)
{
	char buf[20];

	sendBuf(sprintf(buf, "G %i A\r\n", addr), buf);

	if(!getOK(OK_0))
	{
		return printError("cant issue go command");;
	}
	return 1;
}

#define BSIZE 400

int uploadData(unsigned long addr, unsigned long len, unsigned char *ibuf)
{
	char buf[128];
	unsigned char uubuf[BSIZE*3];
	unsigned int blen, csum;
	unsigned long slen, saddr, soff;
	slen = len;
	saddr = addr;
	soff = 0;

	if(addr & 0x3)
	{
		return printError("RAM address not on dword boundary");
	}

	while(slen >= BSIZE)
	{
		sendBuf(sprintf(buf, "W %i %i\r\n", saddr+soff, BSIZE), buf);
		if(!getOK(OK_0))
		{
			return printError("cant write data");
		}

		csum = 0;

		for(blen = 0; blen < BSIZE; blen++)
		{
			csum += ibuf[soff+blen];
		}

		blen = uuencode(ibuf+soff, uubuf, BSIZE);
		sendBuf(blen, (char*)uubuf);
		sendBuf(sprintf(buf, "%u\r\n", csum), buf);
		if(!getOK(OK_LIT))
		{
			return printError("error in checksum");
		}
		slen -= BSIZE;
		soff += BSIZE;
	}

	if(slen)
	{
		sendBuf(sprintf(buf, "W %i %i\r\n", saddr+soff, slen), buf);
		if(!getOK(OK_0))
		{
			
			return printError("cant write remaining data");
		}
	
		csum = 0;
	
		for(blen = 0; blen < slen; blen++)
		{
			csum += ibuf[soff+blen];
		}
	
		blen = uuencode(ibuf+soff, uubuf, slen);
		sendBuf(blen, (char*)uubuf);
		sendBuf(sprintf(buf, "%i\r\n", csum), buf);

		if(!getOK(OK_LIT))
		{
			return printError("error in checksum for remaining data");
		}

	}
	return 1;
}

int uploadRAM(int fd, int loc)
{
	unsigned char buf[70000];

	unsigned long fx, total, size;
	total = 0;

	if(fd)
	{
		printInfoS(2,"reading image into buffer", 0);
		fx = read(fd,buf,70000);
	}
	else
	{
		for(fx=0; fx<tool_len; fx++)
		{
			buf[fx] = tool_data[fx];
		}
		fx = tool_len;
	}

	size = fx;

	printInfoS(0,"starting ram upload", 0);

	printProgress(0,"RAM",total,fx);

	while(fx)
	{
		if(fx >= 256)
		{
			if(!uploadData(loc+total, 256, buf+total))
			{
				return printError("Error in upload");
			}
			total += 256;
			fx -= 256;
		}
		else
		{
			if(!uploadData(loc+total, fx, buf+total))
			{
				return printError("Error in upload");
			}
			total += fx;
			fx -= fx;
		}

		printProgress(0,"RAM",total,size);
	}

	printInfoH(0,"ram upload complete, total bytes:", total);

	return 1;
}
