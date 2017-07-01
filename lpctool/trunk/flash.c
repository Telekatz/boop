#include "lpc2220.h"
#include "flash.h"
#include "serial.h"
#include "lcd.h"

const unsigned long secaddr[19] = 
{ 0x00000,
  0x02000,
  0x03000,
  0x04000,
  0x08000,
  0x10000,
  0x18000,
  0x20000,
  0x28000,
  0x30000,
  0x38000,
  0x40000,
  0x48000,
  0x50000,
  0x58000,
  0x60000,
  0x68000,
  0x70000,
  0x78000 };

unsigned long flash_base;
unsigned short check1, check2;

void slow(void)
{
	BCFG0 = 0x10000420;
	BCFG2 = 0x10000420;
}

void fast(void)
{
	BCFG0 = 0x10000400;
	BCFG2 = 0x10000400;
}
int eraseSector(unsigned char chip, unsigned char secnum)
{

	if(chip == 0)
		flash_base = FLASH0_BASE;
	else
		flash_base = FLASH1_BASE;

	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
	*((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0x80;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
	*((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
	*((volatile unsigned short *)(flash_base + (secaddr[secnum]<<1))) = 0x30;
	retry:
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if((check1 & 0x44) == (check2 & 0x44))
				goto done;
			if(!(check2 & 0x24))
				goto retry;
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if((check1 & 0x44) == (check2 & 0x44))
				goto done;
			*((volatile unsigned short *)flash_base) = 0xF0;
			return -1;
			
	done:
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if(check1 != check2)
				goto retry;
	return 0;
}

int eraseSectorAt(unsigned long sadr)
{

	flash_base = sadr & 0xFF000000;

	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
	*((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0x80;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
	*((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
	*((volatile unsigned short *)(sadr)) = 0x30;

	retry:
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if((check1 & 0x44) == (check2 & 0x44))
				goto done;
			if(!(check2 & 0x24))
				goto retry;
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if((check1 & 0x44) == (check2 & 0x44))
				goto done;
			*((volatile unsigned short *)flash_base) = 0xF0;
			return -1;
			
	done:
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if(check1 != check2)
				goto retry;
	return 0;
}

int eraseFlash(unsigned char chip)
{
	if(chip == 0)
		flash_base = FLASH0_BASE;
	else
		flash_base = FLASH1_BASE;

	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
	*((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0x80;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
	*((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0x10;

	retry:
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if((check1 & 0x44) == (check2 & 0x44))
				goto done;
			if(!(check2 & 0x24))
				goto retry;
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if((check1 & 0x44) == (check2 & 0x44))
				goto done;
			*((volatile unsigned short *)flash_base) = 0xF0;
			return -1;
			
	done:
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if(check1 != check2)
				goto retry;
	return 0;
}

int writeWord(unsigned long addr, unsigned short data)
{
	flash_base = addr & 0xFF000000;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
	*((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0xA0;
	*((volatile unsigned short *)(addr)) = data;

	retry:
			check1 = *((volatile unsigned short *)addr);
			check2 = *((volatile unsigned short *)addr);
			if((check1 & 0x40) == (check2 & 0x40))
				goto done;
			if(!(check2 & 0x20))
				goto retry;
			check1 = *((volatile unsigned short *)addr);
			check2 = *((volatile unsigned short *)addr);
			if((check1 & 0x40) == (check2 & 0x40))
				goto done;
			*((volatile unsigned short *)addr) = 0xF0;
			return -1;
			
	done:
			check1 = *((volatile unsigned short *)addr);
			check2 = *((volatile unsigned short *)addr);
			if(check1 != check2)
				goto retry;

	return 0;
}

void prepareBulk(unsigned long dst)
{
	flash_base = dst & 0xFF000000;

	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
	*((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
	*((volatile unsigned short *)(flash_base | 0xAAA)) = 0x20;
}

void endBulk(unsigned long dst)
{
	flash_base = dst & 0xFF000000;

	*((volatile unsigned short *)(flash_base)) = 0x90;
	*((volatile unsigned short *)(flash_base)) = 0x00;
}

int writeBulk(unsigned long src, unsigned long dst, unsigned long cnt)
{
	flash_base = dst;
	while(cnt--)
	{
		if(*((volatile unsigned short *)src) != 0xFFFF)
		{
			*((volatile unsigned short *)flash_base) = 0xA0;
			*((volatile unsigned short *)flash_base) = *((volatile unsigned short *)src);
	
	retry:
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if((check1 & 0x40) == (check2 & 0x40))
				goto done;
			if(!(check2 & 0x20))
				goto retry;
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if((check1 & 0x40) == (check2 & 0x40))
				goto done;
			*((volatile unsigned short *)flash_base) = 0xF0;
			return -1;
			
	done:
			check1 = *((volatile unsigned short *)flash_base);
			check2 = *((volatile unsigned short *)flash_base);
			if(check1 != check2)
				goto retry;
		}
		src+=2;
		flash_base+=2;
	}

	return 0;
}
