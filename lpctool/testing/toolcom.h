#ifndef TOOLCOM_H
#define TOOLCOM_H

#define ACK	0x00
#define HELO	0x01
#define BACK	0x02

#define MAXBUF 0x00100000

int makeList(unsigned long *buf, unsigned long len);
void freeList(void);
int openRamTool();
int setToolAdr(unsigned long adr);
int eraseFlash(void);
int writeFlash(void);
int enableBulk(unsigned char b);
int setNumBytes(unsigned long numBytes);
int uploadFlash(int fd, unsigned long loc, unsigned int erase);
int uploadChunks(int fd, unsigned long loc, unsigned int erase);

#endif
