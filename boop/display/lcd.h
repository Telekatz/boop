/*
	lcd.h - lcd control & graphics primitives
	Copyright (C) 2007  Ch. Klippel <ck@mamalala.net>

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

#ifndef LCD_H
#define LCD_H

// the lcd data & command ports
#define LCD_CMD		(*((volatile unsigned char *) 0x81000000))
#define LCD_DATA	(*((volatile unsigned char *) 0x81000001))

// LCD commands
#define LCD_CONTRAST 	0x3A	// original 0x3F (0..63)

#define LCD_GRAYMODE_88	0x00	// original 0x3F
#define LCD_GRAYMODE_89	0x00	// original 0x3F
#define LCD_GRAYMODE_8A	0x36	// original 0x33
#define LCD_GRAYMODE_8B	0x63	// original 0x33
#define LCD_GRAYMODE_8C	0x67	// original 0x66
#define LCD_GRAYMODE_8D	0x76	// original 0x66
#define LCD_GRAYMODE_8E	0xAC	// original 0x99
#define LCD_GRAYMODE_8F	0xCA	// original 0x99

#define LCD_DCDCx3		0x64	// 
#define LCD_DCDCx4		0x65	// 
#define LCD_DCDCx5		0x66	// 
//~ #define LCD_DCDCx?		0x67	// ? datasheet p. 33

#define LCD_IRRR_1		0x20	//
#define LCD_IRRR_2		0x21	//
#define LCD_IRRR_3		0x22	//
#define LCD_IRRR_4		0x23	//
#define LCD_IRRR_5		0x24	//
#define LCD_IRRR_6		0x25	//
#define LCD_IRRR_7		0x26	//
#define LCD_IRRR_8		0x27	//

#define LCD_ALLON		0xA4	// black
#define LCD_ALLOFF		0xA5	// white
#define LCD_NORMAL		0xA6	// 
#define LCD_REVERSE		0xA7	// 

#define LCD_SLEEP		0xA9	// enter Sleep Mode. internal oscillator and LCD powersupply off 

#define LCD_OSC_START	0xAB	// 

#define LCD_OFF			0xAE	// 
#define LCD_ON			0xAF	// 

#define LCD_WAKEUP		0xE1	// 
#define LCD_RESET		0xE2	// 


// drawmodes 
#define DRAW_PUT		0
#define DRAW_XOR		1
#define DRAW_ERASE		2
#define DRAW_ALPHA		3
#define DRAW_NORCU		4

#define LCD_COLOR_W		0
#define LCD_COLOR_LG	1
#define LCD_COLOR_DG	2
#define LCD_COLOR_B		3

#define LCD_SIZE_X		128
#define LCD_SIZE_Y		160

#define ARROW_UP		1
#define ARROW_DOWN		2
#define ARROW_LEFT		3
#define ARROW_RIGHT		4
#define ARROW_LEN		4

#define TOP_LEFT		0
#define TOP_RIGHT		1
#define	BOTTOM_LEFT		2
#define BOTTOM_RIGHT	3

#define BORDER_LEFT		0
#define BORDER_RIGHT	127

extern unsigned char drawbuf[2][LCD_SIZE_X];
extern unsigned int is_drawing;

extern volatile unsigned char symbols;
extern volatile unsigned char oldsymbols;
#define symbolRF	(1<<0)
extern const unsigned char symIR[];

typedef void(*rcu_fn)(unsigned char x, unsigned char y, unsigned char l, unsigned char m);
typedef void (*rcuAlpha_fn) (unsigned char x, unsigned char y, unsigned char l, unsigned char m,
                             unsigned char* alphaBuf);
typedef void(*rcus_fn)(unsigned char x, unsigned char y, unsigned char m);
typedef void(*bl_fn)(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char c, unsigned char m);
typedef void(*ln_fn)(unsigned char x, unsigned char y, unsigned char l, unsigned char c, unsigned char m);

// Function pointer to section (".text.fastcode")
extern rcu_fn do_rcu;
extern rcuAlpha_fn   do_rcuAlpha;
extern rcus_fn do_rcuS;
extern bl_fn draw_block;
extern ln_fn draw_hline;
extern ln_fn draw_vline;


extern void lcd_set (unsigned char s);

extern void lcd_init (unsigned char s);
extern void lcd_enable (unsigned char e);
//extern void do_rcu(unsigned char x, unsigned char y, unsigned char l, unsigned char m);
//extern void do_rcuAlpha (unsigned char x, unsigned char y, unsigned char l, unsigned char m,
//                         unsigned char* alphaBuf);
//extern void do_rcuS(unsigned char x, unsigned char y, unsigned char m);
extern void lcd_set_contrast(unsigned char c);

extern void lcd_fill (unsigned char f);
extern void draw_logo (void);
//extern void draw_block (unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char c, unsigned char m);
//extern void draw_hline (unsigned char x, unsigned char y, unsigned char l, unsigned char c, unsigned char m);
//extern void draw_vline (unsigned char x, unsigned char y, unsigned char l, unsigned char c, unsigned char m);
extern void draw_rect (unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char lw, unsigned char c, unsigned char m);
extern void draw_pixel (unsigned char x, unsigned char y, unsigned char c, unsigned char m);
extern void draw_line (unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char c, unsigned char m);
extern void draw_circle (unsigned char cx, unsigned char cy, unsigned char radius, unsigned char c, unsigned char m);
extern void draw_qcircle (unsigned char cx, unsigned char cy, unsigned char radius, unsigned char quarter, unsigned char c, unsigned char m);
extern void draw_disc (unsigned char cx, unsigned char cy, unsigned char radius, unsigned char c, unsigned char m);
extern void draw_qdisc (unsigned char cx, unsigned char cy, unsigned char radius, unsigned char quarter, unsigned char c, unsigned char m);

extern unsigned char draw_arrow (int x, int y, unsigned char ArrowDir, int ArrowLen, unsigned char Color, unsigned char Mode);

extern void draw_frame(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char t, unsigned char r, unsigned char c, unsigned char m);
extern void draw_symbol(unsigned char x, unsigned char y, unsigned char l,unsigned char* data, unsigned char c, unsigned char m);
extern void refresh_symbols(unsigned int cb);
extern void redraw_symbols(void);

extern void update_active_ind (void);

#endif
