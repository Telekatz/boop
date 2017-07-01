#ifndef FLASH_H
#define FLASH_H

#define FLASH0_BASE	0x80000000
#define FLASH1_BASE	0x82000000

void prepareBulk(unsigned long dst);
void endBulk(unsigned long dst);
int eraseSector(unsigned char chip, unsigned char secnum);
int eraseSectorAt(unsigned long sadr);
int eraseFlash(unsigned char chip);
int writeWord(unsigned long addr, unsigned short data);
int writeBulk(unsigned long src, unsigned long dst, unsigned long cnt);
void lcd_set(unsigned char s);

#endif
