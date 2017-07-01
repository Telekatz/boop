/*
 *********************************************************************************
 *     Copyright (c) 2005   ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 *********************************************************************************
 */
/*================================================================================
 * Module Name : types.h
 * Purpose     : Definition of type.
 * Author      : Robin Lee
 * Date        : 2005-12-28
 * Notes       : None.
 * $Log: types.h,v $
 * Revision 1.1  2006/04/07 11:36:17  robin6633
 * no message
 *
 *================================================================================
 */

#ifndef TYPES_H
#define TYPES_H

#ifndef NULL
 #define NULL ((void *) 0L)
#endif

typedef unsigned short int BOOL;

#define TRUE		1
#define FALSE		0

/* Bit Definitions for Bitwise Operation */
#define BIT0		0x00000001
#define BIT1		0x00000002
#define BIT2		0x00000004
#define BIT3		0x00000008
#define BIT4		0x00000010
#define BIT5		0x00000020
#define BIT6		0x00000040
#define BIT7		0x00000080
#define BIT8		0x00000100
#define BIT9		0x00000200
#define BIT10		0x00000400
#define BIT11		0x00000800
#define BIT12		0x00001000
#define BIT13		0x00002000
#define BIT14		0x00004000
#define BIT15		0x00008000
#define	BIT16		0x00010000
#define	BIT17		0x00020000
#define	BIT18		0x00040000
#define	BIT19		0x00080000
#define	BIT20		0x00100000
#define	BIT21		0x00200000
#define	BIT22		0x00400000
#define	BIT23		0x00800000
#define	BIT24		0x01000000
#define	BIT25		0x02000000
#define	BIT26		0x04000000
#define	BIT27		0x08000000
#define	BIT28		0x10000000
#define	BIT29		0x20000000
#define	BIT30		0x40000000
#define	BIT31		0x80000000

#define LOW_BYTE(word)	(unsigned char)(word & 0x00FF)
#define HIGH_BYTE(word)	(unsigned char)((word & 0xFF00) >> 8)

/* Keil compiler user define */
#define XDATA		xdata
#define IDATA		idata
#define BDATA		bdata
#define CODE		code
#define FAR		far

/* Serial interface command direction */
#define	SI_WR		BIT0
#define	SI_RD		BIT1

#define	FLASH_WR_ENB	(PCON |= PWE_)
#define	FLASH_WR_DISB	(PCON &= ~PWE_)


#endif 	/* end of TYPES_H */
