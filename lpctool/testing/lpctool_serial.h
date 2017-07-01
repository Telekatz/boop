#ifndef LPCTOOL_SERIAL_H
#define LPCTOOL_SERIAL_H

#include "baudrate.h"

int openSerial(char *dev);
int closeSerial(void);
int reconfSerial(unsigned int baud, unsigned char flow, unsigned char tout);
int sendBuf(unsigned int len, char *buf);
int getBuf(unsigned int len, char *buf);
void setDTR(unsigned char val);
void setRTS(unsigned char val);

extern int serialDev;

#endif
