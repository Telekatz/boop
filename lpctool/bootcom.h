#ifndef BOOTCOM_H
#define BOOTCOM_H

#define OK_LIT	0x00
#define OK_0	0x01

unsigned int uuencode(unsigned char *ibuf, unsigned char *obuf, int len);
int openBootLoader(unsigned int cfreq);
int uploadData(unsigned long addr, unsigned long len, unsigned char *ibuf);
int uploadRAM(int fd, int loc);

#endif
