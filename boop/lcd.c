/*
    lcd.c - lcd control & graphics primitives
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

#include "lpc2220.h"
#include "lcd.h"
#include "fonty.h"
//#include "b.h"

void pause(unsigned int ptime)
{
	unsigned long xx;
	xx = ptime * 750;
	while(xx)
	{
		xx--;
	};
}

void lcd_set(unsigned char s)
{
	if(s == 0)
	{
		LCD_CMD = 0xA0;		// set segment remap (00H mapped to seg0)
		LCD_CMD = 0xC8;		// set com output scan direction (remapped mode)
	}
	else
	{
		LCD_CMD = 0xA1;		// set segment remap (7FH mapped to seg0)
		LCD_CMD = 0xC0;		// set com output scan direction (normal mode)
	}
}

void lcd_init(unsigned char s)
{

	LCD_CMD = 0xE1;		// exit power save mode
	LCD_CMD = 0xE2;		// software reset
	pause(0x32);
	LCD_CMD = 0xAB;		// start internal oscillator
	LCD_CMD = 0x27;		// set internal regulator resistor ratio (8.1)
	LCD_CMD = 0x81;		// volume cmd
	LCD_CMD = 0x3A; 	// volume cmd value
	LCD_CMD = 0x65;		// set DC-DC converter factor (4x)
	LCD_CMD = 0x60;		// set upper window corner ax cmd
	LCD_CMD = 0x1C;		// set upper window corner ax value
	LCD_CMD = 0x61;		// set upper window corner ay cmd
	LCD_CMD = 0x0A;		// set upper window corner ay value
	LCD_CMD = 0x62;		// set lower window corner bx cmd
	LCD_CMD = 0x75;		// set lower window corner bx value
	LCD_CMD = 0x63;		// set lower window corner by cmd
	LCD_CMD = 0x81;		// set lower window corner by value
	LCD_CMD = 0x90;		// set PWM and FRC (4-frames)
	LCD_CMD = 0x88;		// set white mode frame  2nd and 1st cmd
	LCD_CMD = 0x00;		// set white mode frame  2nd and 1st value
	LCD_CMD = 0x89;		// set white mode frame  4th and 3rd cmd
	LCD_CMD = 0x00;		// set white mode frame  4th and 3rd value
	LCD_CMD = 0x8A;		// set light gray mode frame  2nd and 1st cmd
	LCD_CMD = 0x36;		// set light gray mode frame  2nd and 1st value
	LCD_CMD = 0x8B;		// set light gray mode frame  4th and 3rd cmd
	LCD_CMD = 0x63;		// set light gray mode frame  4th and 3rd vakue
	LCD_CMD = 0x8C;		// set dark gray mode frame  2nd and 1st cmd
	LCD_CMD = 0x67;		// set dark gray mode frame  2nd and 1st value
	LCD_CMD = 0x8D;		// set dark gray mode frame  4th and 3rd cmd
	LCD_CMD = 0x76;		// set dark gray mode frame  4th and 3rd vakue
	LCD_CMD = 0x8E;		// set black mode frame  2nd and 1st cmd
	LCD_CMD = 0xAC;		// set black mode frame  2nd and 1st value
	LCD_CMD = 0x8F;		// set black mode frame  4th and 3rd cmd
	LCD_CMD = 0xCA;		// set black mode frame  4th and 3rd value
	LCD_CMD = 0x66;		// set DC-DC converter factor (5x)
	lcd_set(s);

//	LCD_CMD = 0x2E;		// set power control register (boost on, reg on, buffer off)

//	pause(0x64);

	LCD_CMD = 0x2f;		// set power control register (boost on, reg on, buffer on)   0x2F;
	LCD_CMD = 0xA4;		// set entire display on, normal mode)
	LCD_CMD = 0xA6;		// set normal display mode, inverse = 0xA7


	lcd_fill(0x00);

	LCD_CMD = 0xAF;

	set_font(0);
}

void lcd_enable(unsigned char e)
{
	if(e)
	{
		LCD_CMD = 0xAF;		// set display on
	}
	else
	{
		LCD_CMD = 0xAE;		// set display off
	}
}

void lcd_fill(unsigned char f)
{
	unsigned char x,y;
	for(y=0;y<20;y++)
	{
		LCD_CMD = 0xB0;
		LCD_CMD = y;
		LCD_CMD = 0x10;
		LCD_CMD = 0x00;
		for(x=0;x<128;x++)
		{
			LCD_DATA = f;
			LCD_DATA = f;
		}
	}
}

void draw_logo(void)
{/*
	unsigned short f;
	unsigned char x,y;
	f=0;
	for(y=0;y<b_h>>3;y++)
	{
		LCD_CMD = 0xB0;
		LCD_CMD = y;
		LCD_CMD = 0x10;
		LCD_CMD = 0x00;
		for(x=0;x<b_w;x++)
		{
			LCD_DATA = b_data[f++];
			LCD_DATA = b_data[f++];
		}
	}*/
}

unsigned char rcubuf[2][128];
unsigned char drawbuf[2][128];

void do_rcu(unsigned char x, unsigned char y, unsigned char l, unsigned char m)
{
	unsigned char q;

	LCD_CMD = 0xB0;
	LCD_CMD = (y >> 3);
	LCD_CMD = (0x10 + (x >> 4));
	LCD_CMD = (x & 0x0F);

	q = LCD_DATA;
	q = l;
	
	while(q--)
	{
		rcubuf[0][q] = LCD_DATA;
		rcubuf[1][q] = LCD_DATA;
	}

	q = LCD_DATA;

	LCD_CMD = 0xB0;
	LCD_CMD = (y >> 3);
	LCD_CMD = (0x10 + (x >> 4));
	LCD_CMD = (x & 0x0F);

	q = l;

	switch(m)
	{
		case DRAW_PUT:		// draw put
			while(q--)
			{
				LCD_DATA = rcubuf[0][q] | drawbuf[0][q];
				LCD_DATA = rcubuf[1][q] | drawbuf[1][q];
			}
			break;
		case DRAW_XOR:		// draw xor
			while(q--)
			{
				LCD_DATA = rcubuf[0][q] ^ drawbuf[0][q];
				LCD_DATA = rcubuf[1][q] ^ drawbuf[1][q];
			}
			break;
		case DRAW_ERASE:		// draw erase
			while(q--)
			{
				LCD_DATA = rcubuf[0][q] & ~drawbuf[0][q];
				LCD_DATA = rcubuf[1][q] & ~drawbuf[1][q];
			}
			break;
	}
}

void do_rcuS(unsigned char x, unsigned char y, unsigned char m)
{
	unsigned char q;

	LCD_CMD = 0xB0;
	LCD_CMD = (y >> 3);
	LCD_CMD = (0x10 + (x >> 4));
	LCD_CMD = (x & 0x0F);

	q = LCD_DATA;
	rcubuf[0][0] = LCD_DATA;
	rcubuf[1][0] = LCD_DATA;

	q = LCD_DATA;

	LCD_CMD = 0xB0;
	LCD_CMD = (y >> 3);
	LCD_CMD = (0x10 + (x >> 4));
	LCD_CMD = (x & 0x0F);

	switch(m)
	{
		case 0:		// draw put
			LCD_DATA = rcubuf[0][0] | drawbuf[0][0];
			LCD_DATA = rcubuf[1][0] | drawbuf[1][0];
			break;
		case 1:		// draw xor
			LCD_DATA = rcubuf[0][0] ^ drawbuf[0][0];
			LCD_DATA = rcubuf[1][0] ^ drawbuf[1][0];
			break;
		case 2:		// draw erase
			LCD_DATA = rcubuf[0][0] & ~drawbuf[0][0];
			LCD_DATA = rcubuf[1][0] & ~drawbuf[1][0];
			break;
	}
}

void draw_block(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char c, unsigned char m)
{
	unsigned char q,s;
	unsigned char yy,ss,vv,ww,xx;
	unsigned char segm;

	s = y;

	if(h)
	{
		
		
		
		yy = h;
		ss = y >> 3;
		vv = y & 0x07;
	
		ww = yy & 0x07;

		segm = 0xFF;

		xx = 8;
		if(yy < 8)
		{
			segm >>= (8-ww);
			xx -= (8-ww);
			if(vv > (8-xx))
			{
				xx = yy -(vv-(8-xx));
			}
			else
			{
				xx = yy;
			}
		}
		else
		{
			xx -= vv;
		}

		segm <<= vv;

		q = w;
		while(q--)
		{
			if(c & 0x02)
				drawbuf[0][q] = segm;
			else
				drawbuf[0][q] = 0;
	
			if(c & 0x01)
				drawbuf[1][q] = segm;
			else
				drawbuf[1][q] = 0;
		}
		do_rcu(x,s,w,m);

		yy -= xx;

		while(yy > 8)
		{
			s += 8;
			yy -= 8;
			q = w;
			while(q--)
			{
				if(c & 0x02)
					drawbuf[0][q] = 0xFF;
				else
					drawbuf[0][q] = 0;
		
				if(c & 0x01)
					drawbuf[1][q] = 0xFF;
				else
					drawbuf[1][q] = 0;
			}
			do_rcu(x,s,w,m);
		}

		if(yy)
		{
			s += 8;
			q = w;
			while(q--)
			{
				if(c & 0x02)
					drawbuf[0][q] = 0xFF >> (8-yy);
				else
					drawbuf[0][q] = 0;
		
				if(c & 0x01)
					drawbuf[1][q] = 0xFF >> (8-yy);
				else
					drawbuf[1][q] = 0;
			}
			do_rcu(x,s,w,m);
		}
	}
}

void draw_hline(unsigned char x, unsigned char y, unsigned char l, unsigned char c, unsigned char m)
{
	unsigned char p,q;

	q = l;
	p = 0x01 << (y & 0x07);

	while(q--)
	{
		if(c & 0x02)
			drawbuf[0][q] = p;
		else
			drawbuf[0][q] = 0;

		if(c & 0x01)
			drawbuf[1][q] = p;
		else
			drawbuf[1][q] = 0;
	}
	do_rcu(x,y,l,m);
}

void draw_vline(unsigned char x, unsigned char y, unsigned char l, unsigned char c, unsigned char m)
{
	unsigned char s;
	unsigned char yy,ss,vv,ww,xx;
	unsigned char segm;

	s = y;

	if(l)
	{
		
		
		
		yy = l;
		ss = y >> 3;
		vv = y & 0x07;
	
		ww = yy & 0x07;

		segm = 0xFF;

		xx = 8;
		if(yy < 8)
		{
			segm >>= (8-ww);
			xx -= (8-ww);
			if(vv > (8-xx))
			{
				xx = yy -(vv-(8-xx));
			}
			else
			{
				xx = yy;
			}
		}
		else
		{
			xx -= vv;
		}

		segm <<= vv;

		if(c & 0x02)
			drawbuf[0][0] = segm;
		else
			drawbuf[0][0] = 0;

		if(c & 0x01)
			drawbuf[1][0] = segm;
		else
			drawbuf[1][0] = 0;

		do_rcuS(x,s,m);

		yy -= xx;

		while(yy > 8)
		{
			s += 8;
			yy -= 8;
			if(c & 0x02)
				drawbuf[0][0] = 0xFF;
			else
				drawbuf[0][0] = 0;
		
			if(c & 0x01)
				drawbuf[1][0] = 0xFF;
			else
				drawbuf[1][0] = 0;
			do_rcuS(x,s,m);
		}

		if(yy)
		{
			s += 8;
			if(c & 0x02)
				drawbuf[0][0] = 0xFF >> (8-yy);
			else
				drawbuf[0][0] = 0;
	
			if(c & 0x01)
				drawbuf[1][0] = 0xFF >> (8-yy);
			else
				drawbuf[1][0] = 0;
			do_rcuS(x,s,m);
		}
	}
}

void draw_rect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char lw, unsigned char c, unsigned char m)
{
	if(lw)
	{
		if(lw == 1)
		{
			draw_hline(x,     y,     w,   c, m);
			draw_hline(x,     y+h-1, w,   c, m);

			draw_vline(x,     y+1,   h-2, c, m);
			draw_vline(x+w-1, y+1,   h-2, c, m);
		}
		else
		{
			draw_block(x,      y,      w,  lw,       c, m);
			draw_block(x,      y+h-lw, w,  lw,       c, m);
			draw_block(x,      y+lw,   lw, h-(2*lw), c, m);
			draw_block(x+w-lw, y+lw,   lw, h-(2*lw), c, m);
		}
	}
}

void draw_pixel(unsigned char x, unsigned char y, unsigned char c, unsigned char m)
{
	unsigned char p;

	p = 0x01 << (y & 0x07);

	if(c & 0x02)
		drawbuf[0][0] = p;
	else
		drawbuf[0][0] = 0;

	if(c & 0x01)
		drawbuf[1][0] = p;
	else
		drawbuf[1][0] = 0;

	do_rcuS(x,y,m);
}

void draw_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char c, unsigned char m)
{
	unsigned char i,dx,dy,px,py;
	unsigned int x,y;

	if (y2 > y1)                    // incl. POSITIVE
	{
		dx=x2-x1;
		dy=y2-y1;
		px=x1;
		py=y1;
		draw_pixel(x1, y1, c, m);           // first pixel
		if (dx>=dy)                        // incl. is more horizontal than vertical
		{
			y = y1;
			for(i=0;i<dx;i++)
			{
				y+=dy;
				if (y>=dx)
				{
					y-=dx;
					py+=1;
				}
				px+=1;
				draw_pixel(px, py, c, m);
			}
		}
		else                               // incl. is more vertical than horizontal
		{
			x = x1;
			for(i=0;i<dy;i++)
			{
				x+=dx;
				if (x>=dy)
				{
					x-=dy;
					px+=1;
				}
				py+=1;
				draw_pixel(px, py, c, m);        //??????
			}
		};
	}
	else if (y2 < y1)                    // incl. NEGATIVE
	{
		dx=x2-x1;
		dy=y1-y2;                          // inverted to produce a positive value
		dx=dx;
		dy=dy;
		px=x1;
		py=y2;                             // Note: due to inverted "dy",
		draw_pixel(x2, y2, c, m);           // the first pixel became the last one
		if (dx>=dy)                        // incl. is more horizontal than vertical
		{
			y = y1;
			py=y1;
			for(i=0;i<dx;i++)
			{
				y+=dy;
				if (y>=dx)
				{
					y-=dx;
					py-=1;
				}
				px+=1;
				draw_pixel(px, py, c, m);
			}
		}
		else                               // incl. is more vertical than horizontal
		{
			x = x1;
			px=x2;
			for(i=0;i<dy;i++)
			{
				x+=dx;
				if (x>=dy)
				{
					x-=dy;
					px-=1;
				}
				py+=1;
				draw_pixel(px, py, c, m);
			}
		};
	}
}

void draw_circle(unsigned char cx, unsigned char cy, unsigned char radius, unsigned char c, unsigned char m)
{
	signed int tswitch;
	unsigned char y, x = 0;
	unsigned char d;
	
	d = cy - cx;
	y = radius;
	tswitch = 3 - 2 * radius;
	while (x <= y)
	{
		draw_pixel(cx + x,     cy + y, c, m);
		draw_pixel(cx + x,     cy - y, c, m);
		draw_pixel(cx - x,     cy + y, c, m);
		draw_pixel(cx - x,     cy - y, c, m);

		draw_pixel(cy + y - d, cy + x, c, m);
		draw_pixel(cy + y - d, cy - x, c, m);
		draw_pixel(cy - y - d, cy + x, c, m);
		draw_pixel(cy - y - d, cy - x, c, m);

		if (tswitch < 0)
		{ 
			tswitch += (4 * x + 6);
		}
		else
		{
			tswitch += (4 * (x - y) + 10);
			y--;
		}
		x++;
	}
}


