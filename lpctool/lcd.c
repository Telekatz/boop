#include "lpc2220.h"
#include "lcd.h"
#include "fonty.h"
//#include "b.h"

void pause(unsigned int ptime)
{
	unsigned long xx;
	xx = ptime * 750;
	while(xx--)
	{};
}

void clearStatus(void)
{
	draw_block(0, 151, 128, 10, 3, DRAW_ERASE);
	timer++;
}

void drawStatus(char *msg)
{
	draw_block(0, 152, 128, 10, 3, DRAW_PUT);
	draw_string(1,152,">>",3,DRAW_XOR);
	draw_string(16,152,msg,3,DRAW_XOR);
	timer = 0;
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
	pause(0x64);
	LCD_CMD = 0xAB;		// start internal oscillator
	LCD_CMD = 0x27;		// set internal regulator resistor ratio (8.1)
	LCD_CMD = 0x81;		// volume cmd
	LCD_CMD = 0x3B; 	// volume cmd value
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
	LCD_CMD = 0x55;		// set light gray mode frame  2nd and 1st value
	LCD_CMD = 0x8B;		// set light gray mode frame  4th and 3rd cmd
	LCD_CMD = 0x55;		// set light gray mode frame  4th and 3rd vakue
	LCD_CMD = 0x8C;		// set dark gray mode frame  2nd and 1st cmd
	LCD_CMD = 0x66;		// set dark gray mode frame  2nd and 1st value
	LCD_CMD = 0x8D;		// set dark gray mode frame  4th and 3rd cmd
	LCD_CMD = 0x66;		// set dark gray mode frame  4th and 3rd vakue
	LCD_CMD = 0x8E;		// set black mode frame  2nd and 1st cmd
	LCD_CMD = 0xAA;		// set black mode frame  2nd and 1st value
	LCD_CMD = 0x8F;		// set black mode frame  4th and 3rd cmd
	LCD_CMD = 0xAA;		// set black mode frame  4th and 3rd value

	lcd_set(s);

	LCD_CMD = 0x2E;		// set power control register (boost on, reg on, buffer off)

	pause(0x64);

	LCD_CMD = 0x2F;		// set power control register (boost on, reg on, buffer on)
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
