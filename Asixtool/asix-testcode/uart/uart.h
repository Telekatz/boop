 /*
******************************************************************************
 *     Copyright (c) 2005	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*================================================================================
 * Module Name : uart.h
 * Purpose     : A header file of uart.c includes UART0 and UART1 module.
 * Author      : Robin Lee
 * Date        : 2006-01-10
 * Notes       :
 * $Log: uart.h,v $
 * Revision 1.1  2006/04/07 11:39:14  robin6633
 * no message
 *
 *================================================================================
 */

#ifndef __UART_H__
#define __UART_H__

#include "../types_ax110xx.h"

#define	UART0_ENABLE		1	// 1:enable UART0 ; 0:disable UART0
#define	UART1_ENABLE		0	// 1:enable UART1 ; 0:disable UART1
#define	HSUR_ENABLE		0	// 1:enable Hihg Speed UART ; 0:disable Hihg Speed UART
#define	UART_PMM_ENABLE		0	// 1:use UART in PMM mode ; 0:disable


/* NAMING CONSTANT DECLARATIONS */
#if UART0_ENABLE
#define MAX_TX_UART0_BUF_SIZE	512
#define MAX_TX_UART0_MASK	(MAX_TX_UART0_BUF_SIZE - 1)
#define MAX_RX_UART0_BUF_SIZE	512
#define MAX_RX_UART0_MASK	(MAX_RX_UART0_BUF_SIZE - 1)
#endif

#if UART1_ENABLE
#define MAX_TX_UART1_BUF_SIZE	512
#define MAX_TX_UART1_MASK	(MAX_TX_UART1_BUF_SIZE - 1)
#define MAX_RX_UART1_BUF_SIZE	512
#define MAX_RX_UART1_MASK	(MAX_RX_UART1_BUF_SIZE - 1)
#endif

/* GLOBAL VARIABLES */
#if UART0_ENABLE
extern unsigned int uart0_RxCount;
#endif

#if UART1_ENABLE
extern unsigned int uart1_RxCount;
#endif


/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void UART_Init(void);
signed char putchar(signed char);
signed char _getkey(void);
signed char NOBLK_getkey(void);
BOOL UART_ParityChk(signed char checkbyte);
void UART0_SetMode(unsigned char mode);
void UART1_SetMode(unsigned char mode);
BOOL UART_SetPort(unsigned char portNum);
void PMM_Uart0Init(void);


#endif /* End of __UART_H__ */

/* End of uart.h */
