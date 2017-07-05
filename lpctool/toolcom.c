#include "infohelper.h"
#include "lpctool_serial.h"
#include "toolcom.h"
#include "toolcmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

const unsigned long secinfo[20] =
{  0x00000,
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
   0x80000,
   0x90000,
   0xA0000,
   0xB0000,
   0xC0000,
   0xD0000,
   0xE0000,
   0xF0000,
  0x100000 };


struct t_chunk *chunklist = NULL;

union {
	unsigned char cbuf[MAXBUF];
	unsigned short sbuf[MAXBUF/2];
	unsigned long lbuf[MAXBUF/4];
} buffs;

int makeList(unsigned long *buf, unsigned long len)
{
	unsigned long mlen;
	t_chunk *mlist;
	int cnt;
	unsigned long gap, pos;
	if(chunklist)
		freeList();

	mlen = len>>2;
	gap = 0;
	pos = 0;
	cnt = 1;

	if(mlen)
	{
		printInfoS(2,"creating list of chunks",0);

		chunklist = malloc(sizeof(t_chunk));

		mlist = chunklist;

		if(mlist)
		{
			mlist->start = 0x80000000;
			mlist->len = 0;
			mlist->prev = NULL;
			mlist->next = NULL;
		}

		while(mlen)
		{
			gap = 0;
			if(buf[pos] != 0xFFFFFFFF)
			{
				mlist->len += 4;
				mlen --;
				pos ++;
			}
			else while(buf[pos] == 0xFFFFFFFF)
			{
				gap += 4;
				pos ++;
				mlen --;
			}
			if(gap)
			{
				if(gap >= MIN_GAP)
				{
					mlist->next = malloc(sizeof(t_chunk));
					if(mlist->next)
					{
						cnt++;
						mlist->next->start = chunklist->start + (pos*4);
						mlist->next->prev = mlist;
						mlist->next->next = NULL;
						mlist->next->len = 0;
						mlist = mlist->next;
					}

				}
				else
				{
					mlist->len += gap;
				}
			}
		}
	}
	printInfoH(2, "chunk entries created:",cnt);
	return(cnt);
}

void freeList(void)
{
	t_chunk *mlist;
	int cnt;

	if(chunklist)
	{
		mlist = chunklist;
		cnt = 0;

		while(mlist->next != NULL)
		{
			mlist = mlist->next;
			cnt++;
			free(mlist->prev);
		}
		cnt++;
		free(mlist);
	}
}


int getRESULT(unsigned char acktype)
{
	char buf[4];
	unsigned char q;

	switch(acktype)
	{
		case ACK:
			q = getBuf(4, buf);
			if(strncmp(buf, ACKTOKEN, 4))
			{
				return printError("wrong response, expected ACK!");
			}
			return 1;
			break;

		case HELO:
			q = getBuf(4, buf);
			if(strncmp(buf, HELOTOKEN, 4))
			{
				return printError("wrong response, expected HELO");
			}
			return 1;
			break;
		case BACK:
			q = getBuf(1, buf);
			if(strncmp(buf, BACKTOKEN, 1))
			{
				return printError("wrong response, expected !");
			}
			return 1;
			break;
	}
}

int openRamTool()
{
	char buf[1];
	unsigned char q;

	buf[0] = HELO_CMD;

	sendBuf(1, buf);

	reconfSerial(TOOLBAUD, 0, 50);
	q = getRESULT(HELO);
	reconfSerial(TOOLBAUD, 0, 5);

	return q;
}

int setToolAdr(unsigned long adr)
{
	char buf[5];

	buf[0] = SETADR_CMD;
	buf[1] =(adr & 0xFF000000) >> 24;
	buf[2] =(adr & 0x00FF0000) >> 16;
	buf[3] =(adr & 0x0000FF00) >> 8;
	buf[4] = adr & 0x000000FF;

	sendBuf(5, buf);

	return getRESULT(ACK);
}

int eraseFlash(void)
{
	char buf[1];

	printInfoS(0,"erasing flash", 0);

	buf[0] = FULLERASE_CMD;

	sendBuf(1, buf);

	return getRESULT(ACK);
}

int eraseFlashSector(void)
{
	char buf[1];

	buf[0] = SECTERASE_CMD;

	sendBuf(1, buf);

	return getRESULT(ACK);
}

int enableBulk(unsigned char b)
{
	char buf[1];

	if(b)
	{
		buf[0] = BULKENABLE_CMD;
	}
	else
	{
		buf[0] = BULKDISABLE_CMD;
	}

	sendBuf(1, buf);

	return getRESULT(ACK);
}

int setNumBytes(unsigned long numBytes)
{
	char buf[5];

	buf[0] = SETNUMBYTES_CMD;
	buf[1] =(numBytes & 0xFF000000) >> 24;
	buf[2] =(numBytes & 0x00FF0000) >> 16;
	buf[3] =(numBytes & 0x0000FF00) >> 8;
	buf[4] = numBytes & 0x000000FF;

	sendBuf(5, buf);

	return getRESULT(ACK);
}

int writeFlash(void)
{
	char buf[2];

	buf[0] = WRITE_CMD;
	buf[1] = WRITEGO_CMD;

	sendBuf(2, buf);

	return getRESULT(ACK);
}

int uploadBuf(unsigned char nwords, unsigned char* buf)
{
	unsigned short cnt,cnt2, cnt3;
	unsigned long checksum;
	char tbuf[6];
	checksum = 0;
	tbuf[0] = FILLBUF_CMD;
	tbuf[1] = nwords;

	if(!sendBuf(2,tbuf))
		return 0;

	cnt = nwords << 2;
	cnt2 = 0;

#define XFERSIZE 16

	while(cnt >= XFERSIZE)
	{

		cnt3=XFERSIZE;
		while(cnt3--)
			checksum += buf[cnt2++];

		if(!sendBuf(XFERSIZE, (char*)buf+cnt2-XFERSIZE))
			return 0;

		if(!getRESULT(BACK))
			return 0;

		cnt -= XFERSIZE;
	}

	if(cnt)
	{
		cnt3 = cnt;
		while(cnt--)
			checksum += buf[cnt2++];

		if(!sendBuf(cnt3, (char*)buf+cnt2-cnt3))
			return 0;

		if(!getRESULT(BACK))
			return 0;

	}


	tbuf[0] = (checksum >> 24) & 0xFF;
	tbuf[1] = (checksum >> 16) & 0xFF;
	tbuf[2] = (checksum >> 8) & 0xFF;
	tbuf[3] = checksum & 0xFF;
	if(!sendBuf(4, tbuf))
		return 0;

	return getRESULT(ACK);
}

int uploadChunks(int fd, unsigned long loc, unsigned int erase)
{
	unsigned long fsize, cnt, total, csize;
	unsigned char buf[MAXBUF];
	unsigned int numChunks;
	t_chunk *curr_chunk;

	if(!setToolAdr(loc))
	{
		freeList();
		return printError("failed to set address");
	}

	if(erase & 0x40)
	{
		printInfoS(2,"erasing flash sector", 0);
		if(!eraseFlashSector())
			return printError("failed to erase start sector");
		printInfoS(2,"flash sector erased", 0);
	}
	else if((erase & 0x20) || (erase & 0x10))
	{
		printInfoS(2,"erasing whole flash", 0);
		reconfSerial(TOOLBAUD, 0, 200);

		if(!eraseFlash())
			return printError("failed to erase whole flash");
		reconfSerial(TOOLBAUD, 0, 5);

		printInfoS(2,"whole flash erased", 0);
	}

	printInfoS(2,"enabling bulk write",0);
	if(!enableBulk(1))
		return printError("bulk write enable failed");

	fsize = read(fd, buf, MAXBUF);
	total = fsize;
	cnt = 0;

	printInfoH(2,"setting total number of bytes to", fsize);
	if(!setNumBytes(fsize))
		return printError("cannot set total number of bytes");

	printInfoS(0,"starting flash upload (chunk mode)", 0);

	numChunks = makeList((unsigned long*)buf, fsize);
	curr_chunk = chunklist;

	printProgress(0,"Flash",cnt,total);

	while(numChunks--)
	{
		if(!setToolAdr(curr_chunk->start))
		{
			freeList();
			return printError("failed to set address");
		}

		csize = curr_chunk->len;
		cnt = curr_chunk->start & 0x00FFFFFF;

		while(csize >= 512)
		{
			if(!uploadBuf(128, buf+cnt))
			{
				freeList();
				return printError("buffer upload failed");;
			}
			else
			{
				if(!writeFlash())
				{
					freeList();
					return printError("flash write failed");;
				}
			}
			csize -= 512;
			cnt += 512;
			printProgress(0,"Flash",cnt,total);
		}
		if(csize)
		{
			if(!uploadBuf(csize>>2, buf+cnt))
			{
				freeList();
				return printError("buffer upload failed");;
			}
			else
			{
				if(!writeFlash())
				{
					freeList();
					return printError("flash write failed");;
				}
			}
			csize = 0;
			printProgress(0,"Flash",cnt,total);
		}

		curr_chunk = curr_chunk->next;
	}

	printProgress(0,"Flash",total,total);

	freeList();

	if(!enableBulk(0))
		return printError("bulk write disable failed");;

	printInfoH(0,"flash upload (chunk mode) complete, total bytes:", total);
	return 1;
}

int uploadFlash(int fd, unsigned long loc, unsigned int erase)
{
	unsigned long fsize, cnt, total;
	unsigned char buf[MAXBUF];

	printInfoH(2,"setting address to", loc);
	if(!setToolAdr(loc))
		return printError("failed to set address");

	if(erase & 0x40)
	{
		printInfoS(2,"erasing flash sector", 0);
		if(!eraseFlashSector())
			return printError("failed to erase start sector");
		printInfoS(2,"flash sector erased", 0);
	}
	else if((erase & 0x20) || (erase & 0x10))
	{
		printInfoS(2,"erasing whole flash", 0);
		reconfSerial(TOOLBAUD, 0, 200);

		if(!eraseFlash())
			return printError("failed to erase whole flash");
		reconfSerial(TOOLBAUD, 0, 5);

		printInfoS(2,"whole flash erased", 0);
	}

	printInfoS(2,"enabling bulk write",0);
	if(!enableBulk(1))
		return printError("bulk write enable failed");

	fsize = read(fd, buf, MAXBUF);

	printInfoH(2,"setting total number of bytes to", fsize);
	if(!setNumBytes(fsize))
		return printError("cannot set total number of bytes");

	cnt = 0;
	total = fsize;

	printInfoS(0,"starting flash upload", 0);
	printProgress(0,"Flash",cnt,total);

	while(fsize >= 512)
	{
		if(!uploadBuf(128, buf+cnt))
		{
			return printError("buffer upload failed");;
		}
		else
		{
			if(!writeFlash())
				return printError("flash write failed");;
		}
		fsize -= 512;
		cnt += 512;
		printProgress(0,"Flash",cnt,total);
	}
	if(fsize)
	{
		if(!uploadBuf(fsize>>2, buf+cnt))
		{
			return printError("buffer upload failed");;
		}
		else
		{
			if(!writeFlash())
				return printError("flash write failed");;
		}
		fsize = 0;
		printProgress(0,"Flash",total,total);
	}

	if(!enableBulk(0))
		return printError("bulk write disable failed");;

	printInfoH(0,"flash upload complete, total bytes:", total);
	return 1;
}


