/*
    asix-serial.c - serial port routines (host) to access the asix chip
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

#include "asix_serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#include "infohelper.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define MIN(x,y) ((x) < (y) ? (x) : (y))

static int serialDev = -1;
static struct termios term;

void setBaud(unsigned int baud)
{
	// change baudrate
	switch(baud)
	{
		case 921600:
			cfsetispeed(&term,B921600);
			cfsetospeed(&term,B921600);
			break;
		default:
			cfsetispeed(&term,B115200);
			cfsetospeed(&term,B115200);
			break;
	}
}

void setFlow(unsigned char flow)
{
	if(flow == 0)
	{
		term.c_iflag&=~(IXON|IXOFF|IXANY|INLCR|ICRNL);
	}
	else
	{
		term.c_iflag&=~(INLCR|ICRNL|IXANY);
		term.c_iflag|=(IXON|IXOFF);
	}
}

void setTimeout(unsigned int tout)
{
	term.c_cc[VMIN]=0;
	term.c_cc[VTIME]=tout;	// 10 seconds timeout
}

int openSerial(char *dev, unsigned int speed)
{

	serialDev=open(dev,O_RDWR);
	if(serialDev<0) {
		printf("cannot acces %s\n",dev);
		return serialDev;
	}

	setDTR(0);
	setRTS(0);

	tcgetattr(serialDev,&term);

	setBaud(speed);

	term.c_cflag = (term.c_cflag & ~CSIZE) | CS8 | PARENB;
	term.c_cflag |= CLOCAL | CREAD;

	term.c_cflag &= ~CSTOPB;
	term.c_iflag=IGNBRK;

	term.c_iflag |= IXON | IXOFF;

	term.c_lflag=0;

	term.c_oflag=0;


	setTimeout(2);

	setFlow(0);

	if (tcsetattr(serialDev, TCSANOW, &term)!=0)
		return printError("setattr stage 1 failed");


	if (tcgetattr(serialDev, &term)!=0)
		return printError("getattr failed");


	term.c_cflag &= ~CRTSCTS;

	if (tcsetattr(serialDev, TCSANOW, &term)!=0)
		return printError("setattr stage 2 failed");

	return serialDev;
}

int closeSerial(void)
{
	if(serialDev)
	{
		tcflush(serialDev, TCIOFLUSH);
		close(serialDev);
		return 0;
	}
	else
		return serialDev;
}

int reconfSerial(unsigned int baud, unsigned char flow, unsigned char tout)
{
	if(serialDev > 0)
	{

		setBaud(baud);
		setFlow(flow);
		setTimeout(tout);
		return tcsetattr(serialDev,TCSANOW,&term);
	}
	else
		return serialDev;
}

#define XFERSIZE 16

int sendBuf(unsigned int len, char *buf)
{
	unsigned int boff;
        int ret;

	if(serialDev > 0)
	{
		boff = 0;
		while (boff < len)
		{
			ret = write (serialDev, buf+boff, MIN (len - boff, XFERSIZE));
                        if (ret < 0 && errno != EAGAIN)
                        {
                                perror ("error writing to serial port");
				return 0;
                        }
                        boff += ret;
		}
		return boff;
	}
	return 0;
}

int sendBufE(unsigned int len, char *buf)
{
	return(getBuf(sendBuf(len,buf), buf));
}

int getBuf(unsigned int len, char *buf)
{
	unsigned int boff;
        int ret;

	if(serialDev > 0)
	{
		boff = 0;
			ret = read (serialDev, buf+boff, MIN (len - boff, XFERSIZE));
                        if (ret < 0 && errno != EAGAIN)
                        {
                                perror ("error reading from serial port");
				return 0;
                        }
			boff += ret;
		return boff;
	}
	return 0;
}

void setDTR(unsigned char val)
{
	int mcs;
	if(serialDev)
	{
		ioctl (serialDev, TIOCMGET, &mcs);
		if(val)
		{
			mcs |= TIOCM_DTR;
			ioctl (serialDev, TIOCMSET, &mcs);
		}
		else
		{
			mcs &= ~TIOCM_DTR;
			ioctl (serialDev, TIOCMSET, &mcs);
		}
	}
}

void setRTS(unsigned char val)
{
	int mcs;
	if(serialDev)
	{
		ioctl (serialDev, TIOCMGET, &mcs);
		if(val)
		{
			mcs |= TIOCM_RTS;
			ioctl (serialDev, TIOCMSET, &mcs);
		}
		else
		{
			mcs &= ~TIOCM_RTS;
			ioctl (serialDev, TIOCMSET, &mcs);
		}
	}
}

