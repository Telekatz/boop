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

// drawmodes 
#define DRAW_PUT	0
#define DRAW_XOR	1
#define DRAW_ERASE	2

void lcd_init(unsigned char s);
void lcd_enable(unsigned char e);
void do_rcu(unsigned char x, unsigned char y, unsigned char l, unsigned char m);
void do_rcuS(unsigned char x, unsigned char y, unsigned char m);
void pause(unsigned int ptime);

void lcd_fill(unsigned char f);
void draw_logo(void);
void draw_block(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char c, unsigned char m);
void draw_hline(unsigned char x, unsigned char y, unsigned char l, unsigned char c, unsigned char m);
void draw_vline(unsigned char x, unsigned char y, unsigned char l, unsigned char c, unsigned char m);
void draw_rect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char lw, unsigned char c, unsigned char m);
void draw_pixel(unsigned char x, unsigned char y, unsigned char c, unsigned char m);
void draw_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char c, unsigned char m);
void draw_circle(unsigned char cx, unsigned char cy, unsigned char radius, unsigned char c, unsigned char m);

#endif
