void IRQ_Routine (void)   __attribute__ ((interrupt("IRQ")));
void FIQ_Routine (void)   __attribute__ ((interrupt("FIQ")));
void SWI_Routine (void)   __attribute__ ((interrupt("SWI")));
void UNDEF_Routine (void) __attribute__ ((interrupt("UNDEF")));

typedef volatile unsigned int *reg32_t;


#include "lpc2220.h"
#include "serial.h"
#include "flash.h"
#include "lcd.h"
#include "../toolcmd.h"
#include "fonty.h"

union
{
	unsigned long wbuf[128];
	unsigned short sbuf[256];
	unsigned char bbuf[512];
} buf;

unsigned long checksum;
unsigned short count;
unsigned long adr;
unsigned long numBytes;

void putHN(unsigned char pos, unsigned long hn)
{
	draw_string(33,pos,"0x",3,DRAW_PUT);
	draw_hexW(45,pos,hn,3,DRAW_PUT);
}

void putInfoVal(unsigned char pos, char *name, unsigned long val)
{
	draw_block(0, pos, 128, 8, 3, DRAW_ERASE);

	draw_string(0,pos,name,3,DRAW_PUT);
	putHN(pos, val);
}

void ACK(void)
{
	serial_puts(ACKTOKEN);
}

void BACK(void)
{
	serial_puts(BACKTOKEN);
}

void NACK(void)
{
	serial_puts(NACKTOKEN);
}

int receiveBuf(void)
{
	unsigned long ccheck;
	unsigned short cnt, cnt2, cnt3;

	cnt = count<<2;

	if(cnt > 512)
		return 0;

	checksum = 0;
	ccheck = 0;
	cnt2 = 0;

#define XFERSIZE 16

	while(cnt >= XFERSIZE)
	{
		cnt3=XFERSIZE;
		while(cnt3--)
		{
			buf.bbuf[cnt2] = serial_getc();
			checksum += buf.bbuf[cnt2++];
		}
		BACK();
		cnt -= XFERSIZE;
	}

	if(cnt)
	{
		while(cnt--)
		{
			buf.bbuf[cnt2] = serial_getc();
			checksum += buf.bbuf[cnt2++];
		}
		BACK();
	}

	ccheck = serial_getc() << 24;
	ccheck += serial_getc() << 16;
	ccheck += serial_getc() << 8;
	ccheck += serial_getc();

	if(ccheck == checksum)
	{
		return 1;
	}
	drawStatus("Wrong Checksum");

	return 0;
}

int main(void)
{
	unsigned char i;
	unsigned char counter;

	count = 0;
	adr = 0x00000000;
	timer = TIMEOUT+1;
	numBytes = 0;

	lcd_init(0);

	IODIR0|=0x10;
	IOCLR0=0x10;

	serial_init();

	set_font(1);
#ifdef RESIDENT
	draw_string(0, 0,">> FlashTool V0.2-r",3,DRAW_PUT);
#else
	draw_string(0, 0,">> FlashTool V0.2",3,DRAW_PUT);
#endif
	set_font(0);
	draw_string(0,14,"This tool is licensed under",3,DRAW_PUT);
	draw_string(0,22,"the GNU GPL 3 or later",3,DRAW_PUT);
	draw_string(0,34,"(c) 2007 Ch. Klippel",3,DRAW_PUT);
	draw_string(0,42,"<ck@mamalala.net>",3,DRAW_PUT);

	drawStatus("FlashTool ready!");

	adr = 0x00000000;
  	while (1)
  	{
		i = serial_getc();
		
		if (i)	// misses 0x00 !
		{
			counter += 10;
			if ( counter > 100)
				counter = 50;
				
			putInfoVal(counter, "i:", i); 
		}
		
		if(i==HELO_CMD)
		{
			serial_puts(HELOTOKEN);
			drawStatus("Sent HELO");
		}
		else if(i==BULKENABLE_CMD)
		{
			putInfoVal(80, "Bulk:", 1); 
			if((adr & 0xFF000000) == 0x80000000 || (adr & 0xFF000000) == 0x82000000)
			{  
				prepareBulk(adr);
				drawStatus("Enabled bulk write");
				draw_string(100,80,ACKTOKEN,3,DRAW_PUT);
				ACK();
			}
			else
			{
				draw_string(100,80,NACKTOKEN,3,DRAW_PUT);
				NACK();
			}
		}
		else if(i==SKIP_CMD)
		{
			adr += (serial_getc() << 16);
			adr += serial_getc();
			ACK();
		}
		else if(i==BULKDISABLE_CMD)
		{
			putInfoVal(80, "Bulk:", 0); 
			if((adr & 0xFF000000) == 0x80000000 || (adr & 0xFF000000) == 0x82000000)
			{  
				endBulk(adr);
				drawStatus("Disabled bulk write");
				draw_string(100,80,ACKTOKEN,3,DRAW_PUT);
				ACK();
			}
			else
			{
				draw_string(100,80,NACKTOKEN,3,DRAW_PUT);
				NACK();
			}
		}
		else if(i==SETADR_CMD)
		{
			adr = serial_getc() << 24;
			adr += serial_getc() << 16;
			adr += serial_getc() << 8;
			adr += serial_getc();

			putInfoVal(72, "Addr:", adr); 

			if((adr & 0xFF000000) == 0x40000000 || (adr & 0xFF000000) == 0x80000000 || (adr & 0xFF000000) == 0x82000000)
			{  
				drawStatus("Address changed");
				draw_string(100,72,ACKTOKEN,3,DRAW_PUT);
				ACK();
			}
			else
			{
				draw_string(100,72,NACKTOKEN,3,DRAW_PUT);
				adr = 0x00000000;
				NACK();
			}
		}
		else if(i==FILLBUF_CMD)
		{
			count = serial_getc();
			putInfoVal(88, "Buf:", count<<2); 
			if(count > 0 && count <= 128)
			{
				if(receiveBuf())
				{
					ACK();
				}
				else
				{
					goto fail_f;
				}
			}
			else
			{
fail_f:
				draw_string(100,88,NACKTOKEN,3,DRAW_PUT);
				count = 0;
				NACK();
			}
		}
		else if(i==WRITE_CMD)
		{
			putInfoVal(96, "Write:", adr); 
			i = serial_getc();
			if(i==WRITEGO_CMD && count > 0)
			{
				if(writeBulk((unsigned long) &buf.bbuf, adr, count<<1) == 0)
				{
					adr += (count<<2);
					ACK();
				}
				else
				{
					draw_string(100,96,NACKTOKEN,3,DRAW_PUT);
					NACK();
				}
			}
		}
		else if(i==SECTERASE_CMD)
		{
			
			drawStatus("Erasing Flash Sector");
			if(eraseSectorAt(adr) == 0)
			{
				drawStatus("Erased Flash Sector");
				ACK();
			}
			else
			{
				drawStatus("Erase Sector FAILED!!");
				NACK();
			}
		}
		else if(i==FULLERASE_CMD)
		{
			unsigned char w;
			w = 0xFF;
			if((adr & 0xFF000000) == 0x82000000)
			{
				drawStatus("Erasing Flash #1");
				w = 1;
			}
			else
			{
				drawStatus("Erasing Flash #0");
				w = 0;
			}
			if(eraseFlash(w) == 0)
			{
				if(w==1)
				{
					drawStatus("Erased Flash #1");
				}
				else if(w==0)
				{
					drawStatus("Erased Flash #0");
				}
				ACK();
			}
			else
			{
				if(w)
				{
					drawStatus("Flash #1 Erase FAILED!!");
				}
				else
				{
					drawStatus("Flash #0 Erase FAILED!!");
				}
				NACK();
			}
		}
		else if(i==SETNUMBYTES_CMD)
		{
			numBytes = serial_getc() << 24;
			numBytes += serial_getc() << 16;
			numBytes += serial_getc() << 8;
			numBytes += serial_getc();
			putInfoVal(104, "NumB:", numBytes); 
			if(numBytes <= 0x00100000)
			{
				draw_string(100,104,ACKTOKEN,3,DRAW_PUT);
				ACK();
			}
			else
			{
				draw_string(100,104,NACKTOKEN,3,DRAW_PUT);
				NACK();
			}
		}
		else if(i==READ_CMD)
		{
			for(adr = 0x40000200; adr < 0x40000210; adr+=4)
			{
				putHexW((unsigned long)(*(volatile unsigned long *)(adr)));
				serial_putc('\n');
			}
			for(adr = 0x80000000; adr < 0x80000010; adr+=4)
			{
				putHexW((unsigned long)(*(volatile unsigned long *)(adr)));
				serial_putc('\n');
			}
		}
	}
	return 0;
}

/*  Stubs for various interrupts (may be replaced later)  */
/*  ----------------------------------------------------  */

void IRQ_Routine (void) {
	while (1) ;	
}

void FIQ_Routine (void)  {
	while (1) ;	
}


void SWI_Routine (void)  {
	while (1) ;	
}


void UNDEF_Routine (void) {
	while (1) ;	
}
