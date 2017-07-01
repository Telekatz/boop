#include "lpctool_serial.h"

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

int serialDev = -1;
static struct termios term;

// change baudrate
// doesn't really do anything if it's the same
void setBaud(unsigned int baud)
{

	printInfoU(0, "Setting baudrate to", baud);
	switch(baud)
	{
		case 2400:
			cfsetispeed(&term,B2400);
			cfsetospeed(&term,B2400);
			break;
		case 4800:
			cfsetispeed(&term,B4800);
			cfsetospeed(&term,B4800);
			break;
		case 9600:
			cfsetispeed(&term,B9600);
			cfsetospeed(&term,B9600);
			break;
		case 19200:
			cfsetispeed(&term,B19200);
			cfsetospeed(&term,B19200);
			break;
		case 38400:
			cfsetispeed(&term,B38400);
			cfsetospeed(&term,B38400);
			break;
		case 57600:
			cfsetispeed(&term,B57600);
			cfsetospeed(&term,B57600);
			break;
		case 115200:
			cfsetispeed(&term,B115200);
			cfsetospeed(&term,B115200);
			break;
		case 230400:
			cfsetispeed(&term,B230400);
			cfsetospeed(&term,B230400);
			break;
		default:
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

int openSerial(char *dev)
{

	serialDev=open(dev,O_RDWR);
	if(serialDev<0) 
	{
		printf("cannot acces %s\n",dev);
		return serialDev;
	}

	tcflush(serialDev, TCIOFLUSH);

	setDTR(0);
	setRTS(0);

	tcgetattr(serialDev,&term);

	setBaud(TOOLBAUD);

	term.c_cflag = (term.c_cflag & ~CSIZE) | CS8;
	term.c_cflag |= CLOCAL | CREAD;

	term.c_cflag &= ~(PARENB | PARODD);
	term.c_cflag &= ~CSTOPB;
	term.c_iflag=IGNBRK;

	term.c_iflag |= IXON | IXOFF;

	term.c_lflag=0;

	term.c_oflag=0;


	setTimeout(5);

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
		//~ tcflush(serialDev, TCIOFLUSH);
		while (boff < len)
		{
			ret = write (serialDev, buf+boff, MIN (len - boff, XFERSIZE));
			
			#if 0
			printf("sent %i: %c\n", ret, *(buf+boff));
			#endif
			
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
		while (boff < len)
		{
			ret = read (serialDev, buf+boff, MIN (len - boff, XFERSIZE));
	if (ret < 0 && errno != EAGAIN)
	{
		perror ("error reading from serial port");
		return 0;
	}
			boff += ret;
		}
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

