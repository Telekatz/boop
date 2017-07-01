/*
 *********************************************************************************
 *     Copyright (c) 2005	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : uart.c
 * Purpose     : The UART module driver. It manages the character
 *               buffer and handles the ISR. This driver includes UART0 and UART1.
 * Author      : Robin Lee
 * Date        : 2006-01-10
 * Notes       : None.
 * $Log: uart.c,v $
 * Revision 1.1  2006/04/07 11:39:14  robin6633
 * no message
 *
 *================================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include	"../ax110xx.h"
#include	"uart.h"



#if HSUR_ENABLE
#include	"hsuart.h"
#endif


/* STATIC VARIABLE DECLARATIONS */
#if UART0_ENABLE
static unsigned char	uart0_TxBuf[MAX_TX_UART0_BUF_SIZE];
static unsigned int	uart0_TxHead = 0;
static unsigned int	uart0_TxTail = 0;
static unsigned int	uart0_TxCount = 0;
static unsigned char	uart0_TxFlag = 0;
static unsigned char	uart0_RxBuf[MAX_RX_UART0_BUF_SIZE];
static unsigned int	uart0_RxHead = 0;
static unsigned int	uart0_RxTail = 0;
static unsigned char	uart0_Mode = 0;
unsigned int	uart0_RxCount = 0;
#endif

#if UART1_ENABLE
static unsigned char	uart1_TxBuf[MAX_TX_UART1_BUF_SIZE];
static unsigned int	uart1_TxHead = 0;
static unsigned int	uart1_TxTail = 0;
static unsigned int	uart1_TxCount = 0;
static unsigned char	uart1_TxFlag = 0;
static unsigned char	uart1_RxBuf[MAX_RX_UART1_BUF_SIZE];
static unsigned int	uart1_RxHead = 0;
static unsigned int	uart1_RxTail = 0;
static unsigned char	uart1_Mode = 0;
unsigned int	uart1_RxCount = 0;
#endif
static unsigned char	uartPort = 0;


/* LOCAL SUBPROGRAM DECLARATIONS */
#if UART0_ENABLE
//void			uart0_ISR(void);
static void		uart0_Init(void);
static signed char	uart0_PutChar(signed char c);
static signed char	uart0_GetKey(void);
static signed char	UART0_NoBlockGetkey(void);
#endif
#if UART1_ENABLE
//void			uart1_ISR(void);
static void		uart1_Init(void);
static signed char	uart1_PutChar(signed char c);
static signed char	uart1_GetKey(void);
static signed char	uart1_NoBlockGetkey(void);
#endif



/* LOCAL SUBPROGRAM BODIES */
#if UART0_ENABLE
/*
 * ----------------------------------------------------------------------------
 * static void UART0_ISR(void)
 * Purpose : UART0 interrupt service routine. For sending out, it puts data
 *           from software buffer into hardware serial buffer register (SBUF0).
 *           For receiving, it gets data from hardware serial buffer register
 *           (SBUF0) and stores into software buffer.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void uart0_ISR(void) __interrupt 4 __using 2
{
	unsigned char	parity = 0;

	if (RI0)
	{
		EA = 0;
		if (uart0_RxCount != MAX_RX_UART0_BUF_SIZE) 
		{
			uart0_RxBuf[uart0_RxHead] = SBUF0;
			
			if (uart0_Mode & BIT1)
			{
				parity = UART_ParityChk((unsigned char)uart0_RxBuf[uart0_RxHead]);
				if (RB08 != parity)
					P3 = 0xE7;
			}

			uart0_RxCount++;
			uart0_RxHead++;
			uart0_RxHead &= MAX_RX_UART0_MASK;
	    }
		RI0 = 0;
		EA = 1;
	} /* End of if(RI0) */

	if (TI0)
	{
		EA = 0;

		uart0_TxTail++;
		uart0_TxTail &= MAX_TX_UART0_MASK;
		uart0_TxCount--;
		if (uart0_TxCount > 0)
		{
			SBUF0 = uart0_TxBuf[uart0_TxTail];

			if (uart0_Mode & BIT1)
			{
				parity = UART_ParityChk((unsigned char)uart0_TxBuf[uart0_TxTail]);
				if (parity)
					TB08 = 1;
				else
					TB08 = 0;
			}
		}
		else
			uart0_TxFlag = 0;

		TI0 = 0;
		EA = 1;

	} /* End of if(TI0) */

} /* End of UART_Int */

/*
 * ----------------------------------------------------------------------------
 * static void uart0_Init(void)
 * Purpose : Setting operation mode of UART0 and initiating the global values.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static void uart0_Init(void)
{
	unsigned char	sysClk = 0;
	unsigned int	i;

	uart0_TxHead = 0;
	uart0_TxTail = 0;
	uart0_TxCount = 0;
	uart0_TxFlag = 0;
	uart0_RxHead = 0;
	uart0_RxTail = 0;
	uart0_RxCount = 0;
	for (i=0 ; i<MAX_TX_UART0_BUF_SIZE ; i++)
		uart0_TxBuf[i] = 0;
	for (i=0 ; i<MAX_RX_UART0_BUF_SIZE ; i++)
		uart0_RxBuf[i] = 0;

	// Initialize TIMER1 for standard 8051 UART clock
	PCON  = 0x80;			// Disable BaudRate doubler.
	SM01  = 1;			// Use serial port 0 in mode 1 with 8-bits data.
	REN0  = 1;			// Enable UART0 receiver.
	TMOD  = 0x20;		// Use timer 1 in mode 2, 8-bit counter with auto-reload.
	uart0_Mode = 1;

	sysClk = CSREPR & 0xC0;
	switch (sysClk)
	{
		case SCS_100M :
			TH1 = 0xE4;		// Baud rate = 9600 @ 100MHz.
			break;
		case SCS_50M :
			TH1 = 0xF9;		// Baud rate = 9600 @ 50MHz.
			break;
		case SCS_25M :
			TH1 = 0xF9;		// Baud rate = 9600 @ 25MHz.
			break;
		default :
			TH1 = 0xF9;		// Baud rate = 9600 @ 25MHz.
			break;
	}

	ES0	= 1;				// Enable serial port Interrupt request
	TR1 = 1;				// Run Timer 1
	TI0 = 0;

} /* End of UART_Init */

/*
 * ----------------------------------------------------------------------------
 * static signed char uart0_PutChar(signed char c)
 * Purpose : UART0 output function. This function puts one byte data into the
 *           software character buffer.
 * Params  : c - one byte character.
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static signed char uart0_PutChar(signed char c)
{
	unsigned int	count = 0;

	if (c == 0xa)
	{
		do
		{
			EA = 0;
			count = uart0_TxCount; 
			EA = 1;
		} while (count == MAX_TX_UART0_BUF_SIZE);
		uart0_TxBuf[uart0_TxHead] = 0xd;
		EA = 0;
		uart0_TxCount++;
		EA = 1;
		uart0_TxHead++;
		uart0_TxHead &= MAX_TX_UART0_MASK;
	}
	do
	{
		EA = 0;
		count = uart0_TxCount; 
		EA = 1;
	} while (count == MAX_TX_UART0_BUF_SIZE);
	uart0_TxBuf[uart0_TxHead] = c;
	EA = 0;
	uart0_TxCount++;
	EA = 1;
	uart0_TxHead++;
	uart0_TxHead &= MAX_TX_UART0_MASK;

	if (!uart0_TxFlag)
	{
		uart0_TxFlag = 1;
		SBUF0 = uart0_TxBuf[uart0_TxTail];
	}

	
	return c;

}

/*
 * ----------------------------------------------------------------------------
 * static signed char uart0_GetKey(void)
 * Purpose : UART0 input function. This function replies one byte data from the
 *           software character buffer.
 * Params  : none
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static signed char uart0_GetKey (void)
{
    /* NAMING CONSTANT DECLARATIONS */

    /* LOCAL VARIABLE DECLARATIONS */
    signed char	c = 0;
    /* BODY */
	while(uart0_RxCount == 0);
	EA = 0;
	uart0_RxCount--;
	EA = 1;
	c = uart0_RxBuf[uart0_RxTail];
	uart0_RxTail++;
	uart0_RxTail &= MAX_RX_UART0_MASK;
	return c;
}

/*
 * ----------------------------------------------------------------------------
 * static signed char uart0_NoBlockGetkey(void)
 * Purpose : UART0 input function. This function replies one byte data from the
 *           software character buffer. But it only check the buffer one time.
 *           If no data, it will reply a FALSE condition.
 * Params  : none
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static signed char UART0_NoBlockGetkey (void)
{
    char c = 0;

	if (uart0_RxCount !=0 )
	{
		EA = 0;
		uart0_RxCount--;
		EA = 1;
		c = uart0_RxBuf[uart0_RxTail];
		uart0_RxTail++;
		uart0_RxTail &= MAX_RX_UART0_MASK;
		return c;
	}
	else
	{
		return FALSE;
	}
}
#endif


#if UART1_ENABLE
////////////////////////////////////////////////////////////////////////////////
/*
 * ----------------------------------------------------------------------------
 * static void UART1_ISR(void)
 * Purpose : UART1 interrupt service routine. For sending out, it puts data
 *           from software buffer into hardware serial buffer register (SBUF1).
 *           For receiving, it gets data from hardware serial buffer register
 *           (SBUF1) and stores into software buffer.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
uart1_ISR(void) __interrupt 6 __using 2
{
	unsigned char	parity = 0;

	if (RI1)
	{
		EA = 0;
		if (uart1_RxCount != MAX_RX_UART1_BUF_SIZE) 
		{
			uart1_RxBuf[uart1_RxHead] = SBUF1;

			if (uart1_Mode & BIT1)
			{
				parity = UART_ParityChk((unsigned char)uart1_RxBuf[uart1_RxHead]);
				if (RB18 != parity)
					P3 = 0xE7;
			}

			uart1_RxCount++;
			uart1_RxHead++;
			uart1_RxHead &= MAX_RX_UART1_MASK;
	    }
		RI1 = 0;
		EA = 1;
	} /* End of if(RI0) */

	if (TI1)
	{
		EA = 0;

		uart1_TxTail++;
		uart1_TxTail &= MAX_TX_UART1_MASK;
		uart1_TxCount--;
		if (uart1_TxCount > 0)
		{
			SBUF1 = uart1_TxBuf[uart1_TxTail];
			
			if (uart1_Mode & BIT1)
			{
				parity = UART_ParityChk((unsigned char)uart1_TxBuf[uart1_TxTail]);
				if (parity)
					TB18 = 1;
				else
					TB18 = 0;
			}
		}
		else
			uart1_TxFlag = 0;

		TI1 = 0;
		EA = 1;

	} /* End of if(TI0) */

}

/*
 * ----------------------------------------------------------------------------
 * static void uart1_Init(void)
 * Purpose : Setting operation mode of UART1 and initiating the global values.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static void uart1_Init(void)
{
	unsigned char	sysClk = 0;
	unsigned int	i;

	uart1_TxHead = 0;
	uart1_TxTail = 0;
	uart1_TxCount = 0;
	uart1_TxFlag = 0;
	uart1_RxHead = 0;
	uart1_RxTail = 0;
	uart1_RxCount = 0;
	for (i=0 ; i<MAX_TX_UART1_BUF_SIZE ; i++)
		uart1_TxBuf[i] = 0;
	for (i=0 ; i<MAX_RX_UART1_BUF_SIZE ; i++)
		uart1_RxBuf[i] = 0;

	// Initialize TIMER1 for standard 8051 UART clock
	PCON  = 0;			// Disable BaudRate doubler.
	SM10  = 0;
	SM11  = 1;			// Use serial port 1 in mode 1 with 8-bits data.
	REN1  = 1;			// Enable UART1 receiver.
	TMOD  = 0x20;		// Use timer 1 in mode 2, 8-bit counter with auto-reload.
	uart1_Mode = 1;

	sysClk = CSREPR & 0xC0;
	switch (sysClk)
	{
		case SCS_100M :
			TH1 = 0xE4;		// Baud rate = 9600 @ 100MHz.
			break;
		case SCS_50M :
			TH1 = 0xF2;		// Baud rate = 9600 @ 50MHz.
			break;
		case SCS_25M :
			TH1 = 0xF9;		// Baud rate = 9600 @ 25MHz.
			break;
		default :
			TH1 = 0xF9;		// Baud rate = 9600 @ 25MHz.
			break;
	}

	ES1	= 1;				// Enable serial port Interrupt request
	TR1 = 1;				// Run Timer 1
	TI1 = 0;

}

/*
 * ----------------------------------------------------------------------------
 * static signed char uart1_PutChar(signed char c)
 * Purpose : UART1 output function. This function puts one byte data into the
 *           software character buffer.
 * Params  : c - one byte character.
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static signed char uart1_PutChar(signed char c)
{
	unsigned int	count = 0;

	if (c == 0xa)
	{
		do
		{
			EA = 0;
			count = uart1_TxCount; 
			EA = 1;
		} while (count == MAX_TX_UART1_BUF_SIZE);
		uart1_TxBuf[uart1_TxHead] = 0xd;
		EA = 0;
		uart1_TxCount++;
		EA = 1;
		uart1_TxHead++;
		uart1_TxHead &= MAX_TX_UART1_MASK;
	}
	do
	{
		EA = 0;
		count = uart1_TxCount; 
		EA = 1;
	} while (count == MAX_TX_UART1_BUF_SIZE);
	uart1_TxBuf[uart1_TxHead] = c;
	EA = 0;
	uart1_TxCount++;
	EA = 1;
	uart1_TxHead++;
	uart1_TxHead &= MAX_TX_UART1_MASK;

	if (!uart1_TxFlag)
	{
		uart1_TxFlag = 1;
		SBUF1 = uart1_TxBuf[uart1_TxTail];
	}
	
	return c;

}

/*
 * ----------------------------------------------------------------------------
 * static signed char uart1_GetKey(void)
 * Purpose : UART1 input function. This function replies one byte data from the
 *           software character buffer.
 * Params  : none
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static signed char uart1_GetKey (void)
{
    /* NAMING CONSTANT DECLARATIONS */

    /* LOCAL VARIABLE DECLARATIONS */
    signed char c;
    /* BODY */
	while(uart1_RxCount==0);
	EA = 0;
	uart1_RxCount--;
	EA = 1;
	c = uart1_RxBuf[uart1_RxTail];
	uart1_RxTail++;
	uart1_RxTail &= MAX_RX_UART1_MASK;
	return c;
}

/*
 * ----------------------------------------------------------------------------
 * static signed char uart1_NoBlockGetkey(void)
 * Purpose : UART1 input function. This function replies one byte data from the
 *           software character buffer. But it only check the buffer one time.
 *           If no data, it will reply a FALSE condition.
 * Params  : none
 * Returns : c - one byte character.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
static signed char uart1_NoBlockGetkey (void)
{
    char c = 0;

	if (uart1_RxCount !=0 )
	{
		EA = 0;
		uart1_RxCount--;
		EA = 1;
		c = uart1_RxBuf[uart1_RxTail];
		uart1_RxTail++;
		uart1_RxTail &= MAX_RX_UART1_MASK;
		return c;
	}
	else
	{
		return FALSE;
	}
}
#endif


/* EXPORTED SUBPROGRAM BODIES */

/*
 * ----------------------------------------------------------------------------
 * signed char _getkey (void)
 * Purpose : UART getkey function. This function is the entry of getting
 *           characters from software buffer of system's UART ports,
 *           UART0, UART1 and HSUR. 
 * Params  : none
 * Returns : ch - one byte character.
 * Note    : The default UART port is UART0.
 * ----------------------------------------------------------------------------
 */
signed char _getkey (void)
{
	signed char	ch = 0;

	switch (uartPort)
	{
		#if UART0_ENABLE
		case 0 :
			ch = uart0_GetKey();
			break;
		#endif
		#if UART1_ENABLE
		case 1 :
			ch = uart1_GetKey();
			break;
		#endif
		#if HSUR_ENABLE
		case 2 :
			ch = HSUR_GetChar();
			break;
		#endif
	}
	
	return ch;
}

/*
 * ----------------------------------------------------------------------------
 * signed char putchar(signed char c)
 * Purpose : UART putchar function. This function is the entry of putting
 *           characters into software buffer of system's UART ports,
 *           UART0, UART1 and HSUR. 
 * Params  : c - one byte character to be put.
 * Returns : ch - the same character to be replied.
 * Note    : The default UART port is UART0.
 * ----------------------------------------------------------------------------
 */
signed char putchar(signed char c)
{
	signed char	ch = 0;

	switch (uartPort)
	{
		#if UART0_ENABLE
		case 0 :
			ch = uart0_PutChar(c);
			break;
		#endif
		#if UART1_ENABLE
		case 1 :
			ch = uart1_PutChar(c);
			break;
		#endif
		#if HSUR_ENABLE
		case 2 :
			ch = HSUR_PutChar(c);
			break;
		#endif
	}

	return ch;
}

/*
 * ----------------------------------------------------------------------------
 * void UART_Init(void)
 * Purpose : UART initial function. It will call a real initial function
 *           corresponding to the used UART port.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
void UART_Init(void)
{
	switch (uartPort)
	{
		#if UART0_ENABLE
		case 0 :
			uart0_Init();
			break;
		#endif
		#if UART1_ENABLE
		case 1 :
			uart1_Init();
			break;
		#endif
		#if HSUR_ENABLE
		case 2 :
			HSUR_Init();
			HSUR_Setup(0x0060, (UR2_CHAR_8|UR2_STOP_10), (UR2_RDI_ENB|UR2_RLSI_ENB),
			(UR2_FIFO_MODE|UR2_RXFIFO_RST|UR2_TXFIFO_RST|UR2_TRIG_08), UR2_RTS);
			break;
		#endif
	}
}

/*
 * ----------------------------------------------------------------------------
 * signed char NOBLK_getkey (void)
 * Purpose : UART no blocking getkey function with one checked. This function
 *           is the entry of getting characters from software buffer of
 *           system's UART ports, UART0, UART1 and HSUR.
 * Params  : none
 * Returns : ch - one byte character.
 * Note    : The default UART port is UART0.
 * ----------------------------------------------------------------------------
 */
signed char NOBLK_getkey (void)
{
    signed char	c = 0;

	switch (uartPort)
	{
		#if UART0_ENABLE
		case 0 :
			c = UART0_NoBlockGetkey();
			break;
		#endif
		#if UART1_ENABLE
		case 1 :
			c = uart1_NoBlockGetkey();
			break;
		#endif
	}

	return c;
}

/*
 * ----------------------------------------------------------------------------
 * BOOL UART_ParityChk(signed char checkByte)
 * Purpose : UART parity checked function in one byte transfer.
 * Params  : checkByte - one byte character.
 * Returns : TRUE - odd parity ; FALSE - even parity.
 * Note    : none
 * ----------------------------------------------------------------------------
 */
BOOL UART_ParityChk(signed char checkByte)
{
	unsigned char	oneNum = 0;
	unsigned int	i;

	oneNum = 0;
	for (i=0 ; i<=7 ; i++)
	{
		if (checkByte & (BIT0<<i))
		{
			oneNum ++;
		}
	}
	if ((oneNum % 2) == 0)
		return FALSE;		// if '1' number is even, return 0
	else
		return TRUE;		// if '1' number is odd, return 1
}

/*
 * ----------------------------------------------------------------------------
 * void UART0_SetMode(unsigned char mode)
 * Purpose : Setting operation mode of UART0.
 * Params  : mode - operation mode (0~3).
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
#if UART0_ENABLE
void UART0_SetMode(unsigned char mode)
{
	uart0_Mode = mode;
}
#endif

/*
 * ----------------------------------------------------------------------------
 * void UART1_SetMode(unsigned char mode)
 * Purpose : Setting operation mode of UART0.
 * Params  : mode - operation mode (0~3).
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
#if UART1_ENABLE
void UART1_SetMode(unsigned char mode)
{
	uart1_Mode = mode;
}
#endif

/*
 * ----------------------------------------------------------------------------
 * BOOL UART_SetPort(unsigned char portNum)
 * Purpose : Setting which UART port will be used.
 * Params  : portNum - uart port number (0~2).
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
BOOL UART_SetPort(unsigned char portNum)
{
	uartPort = portNum & 0x03;
	UART_Init();
	return TRUE;
}

/*
 * ----------------------------------------------------------------------------
 * void PMM_Uart0Init(void)
 * Purpose : Initiating the UART0 to work in power management mode.
 * Params  : none
 * Returns : none
 * Note    : none
 * ----------------------------------------------------------------------------
 */
#if UART_PMM_ENABLE
void PMM_Uart0Init(void)
{
	unsigned int	i;

	uart0_TxHead = 0;
	uart0_TxTail = 0;
	uart0_TxCount = 0;
	uart0_TxFlag = 0;
	uart0_RxHead = 0;
	uart0_RxTail = 0;
	uart0_RxCount = 0;
	for (i=0 ; i<MAX_TX_UART0_BUF_SIZE ; i++)
		uart0_TxBuf[i] = 0;
	for (i=0 ; i<MAX_RX_UART0_BUF_SIZE ; i++)
		uart0_RxBuf[i] = 0;

	// Initialize TIMER1 for standard 8051 UART clock
	PCON  = BIT7;		// Enable BaudRate doubler.
	SM01  = 1;			// Use serial port 0 in mode 1 with 8-bits data.
	REN0  = 1;			// Enable UART0 receiver.
	TMOD  = 0x20;		// Use timer 1 in mode 2, 8-bit counter with auto-reload.

	TH1 = 0xFE;			// Baud rate = 1200 @ 25MHz.

	ES0	= 1;				// Enable serial port Interrupt request
	TR1 = 1;				// Run Timer 1
	TI0 = 0;

}
#endif


/* End of uart.c */
