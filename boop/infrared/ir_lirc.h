/*
    ir_lirc.h - lirc ir encoder
    Copyright (C) 2009  <telekatz@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IR_LIRC_H
#define IR_LIRC_H


#define LIRC_RC5				0x0002    /* IR data follows RC5 protocol */
#define LIRC_SHIFT_ENC			LIRC_RC5  /* IR data is shift encoded (name obsolete) */
#define LIRC_RC6				0x0004    /* IR data follows RC6 protocol */
#define LIRC_RCMM				0x0008    /* IR data follows RC-MM protocol */
#define LIRC_SPACE_ENC			0x0010	 /* IR data is space encoded */
//#define LIRC_SPACE_FIRST		0x0020    /* bits are encoded as space+pulse */
//#define LIRC_GOLDSTAR			0x0040    /* encoding found on Goldstar remote */
//#define LIRC_GRUNDIG			0x0080    /* encoding found on Grundig remote */
#define LIRC_BO				  0x0100    /* encoding found on Bang & Olufsen remote */
#define LIRC_RF					0x0200    /* RF ASK/OOK modulator */
//#define LIRC_SERIAL			0x0400    /* serial protocol */
//#define LIRC_XMP				0x0800    /* XMP protocol */

/* additinal flags: can be orred together with protocol flag */
//#define REVERSE		0x0800
#define LIRC_NO_HEAD_REP		0x1000	  /* no header for key repeats */
#define LIRC_NO_FOOT_REP		0x2000	  /* no foot for key repeats */
#define LIRC_CONST_LENGTH		0x4000    /* signal length+gap is always constant */
#define LIRC_REPEAT_HEADER		0x8000    /* header is also sent before repeat code */

#define is_rc6		(lirctable->flags & LIRC_RC6)
#define is_biphase	((lirctable->flags & LIRC_RC5) || is_rc6)
#define is_rcmm		(lirctable->flags & LIRC_RCMM)
#define is_const	(lirctable->flags & LIRC_CONST_LENGTH)
#define is_RF		(lirctable->flags & LIRC_RF)
#define is_BO   (lirctable->flags & LIRC_BO)     /* flag for b&o specific protocol MN2017325 */
#define has_header	(lirctable->phead && lirctable->shead)
#define has_foot	(lirctable->pfoot && lirctable->sfoot)
#define has_repeat	(lirctable->prepeat && lirctable->srepeat)

//FS20 Protocol description see http://fhz4linux.info/tiki-index.php?page=FS20%20Protocol
#define calcFS20pre(HC1,par1,HC2,par2)						((1<<18) | (HC1<<10) | (par1<<9) | (HC2<<1) | par2)
#define calcFS20code(Adr,parAdr,Cmd,parCmd,Chk,parChk)		((Adr<<19) | (parAdr<<18) | (Cmd<<10) | (parCmd<<9) | (Chk<<1) | parChk)

void LIRC_Encode (void) __attribute__ ((section(".text.fastcode")));

void LIRC_Init(unsigned char map);
void LIRC_Send(unsigned long cmd);
void LIRC_Repeat(void);
void LIRC_Stop(void);

#endif
