#include <8052.h>

#include "ax110xx.h"
#include "interrupt_ax110xx.h"
#include "cpu_ax110xx.h"
#include "types_ax110xx.h"
#include "uart/uart.h"

#define ERASE_EVENT	1
#define WRITE_EVENT	2
#define READ_EVENT	3

extern void ax11000_PeripherialISR(void) __interrupt 9;
#if UART0_ENABLE
extern void			uart0_ISR(void) __interrupt 4 __using 2;
#endif
#if UART1_ENABLE
extern void			uart1_ISR(void) __interrupt 6 __using 2;
#endif


volatile unsigned char hi_flag = 1;
volatile unsigned char timer = 0;
volatile unsigned char tstate = 0;


void timer0_irq_proc(void) __interrupt 1 using 2
{
	timer++;

	if (timer == 0)
	{
		tstate = 1;
	}
	
	switch(tstate)
	{
		case 1:
		case 16:
			P1_0 = 0;
			break;
		case 8:
		case 24:
			P1_0 = 1;
			break;
		default: break;
	}

	if(tstate)
		tstate++;
	
	TR0 = 0; /* Stop Timer 0 counting */
	TH0 = 0x00;
	TL0 = 0x00;
	TR0 = 1; /* Start counting again */
}

void pause(unsigned char len)
{
	unsigned char a;
	unsigned int b;
	a=len;
	b=0xFFFF;
	while(a--)
	{
		while(b--)
		{
			__asm
			nop
			__endasm;
		}
	}
}

unsigned char _sdcc_external_startup()
{
	return 0;
}

void putstring(char *string)
{
	unsigned char sc;
	sc = 0;
	while(string[sc] != 0)
		putchar(string[sc++]);
}

const char hval[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

void putHexB(unsigned char b)
{
	putchar(hval[b>>4]);
	putchar(hval[b&0x0F]);
}

void putHexW(unsigned int w)
{
	putHexB(w>>8);
	putHexB(w&0xFF);
}

void putHexD(unsigned long d)
{
	putHexW(d>>16);
	putHexW(d&0xFFFF);
}

void main(void)
{
	AX11000_Init();

	switch (CSREPR & (BIT6|BIT7))
	{
		case SCS_100M :
			goto three;
			break;
		case SCS_50M :
			goto two;
			break;
		case SCS_25M :
			goto one;
			break;
	}

	goto none;

three:
	P1 = 0x00;
	pause(0x07);
	P1 = 0xFF;
	pause(0x07);
two:
	P1 = 0x00;
	pause(0x07);
	P1 = 0xFF;
	pause(0x07);
one:
	P1 = 0x00;
	pause(0x07);
	P1 = 0xFF;
none:

	TCON = 0;     /* timer control register, byte operation */
	TH0 = 0x00;
	TL0 = 0x00;
	TR0 = 1;      /* start timer0 */
	ET0 = 1;      /* Enable Timer 0 overflow interrupt IE.1 */
	TI = 0;       /* clear this out */

	UART_SetPort(0);

	TMOD = (TMOD & 0xF0) | 0x01;  /* timer control mode, byte operation */

	EA = 1;       /* Enable Interrupts */

	while (1) 
	{
		pause(0x0F);

		putstring("Hello World!\n\r");
	}
}
