#include "lcd.h"
#include "fonty.h"

#include "charset.info"
#include "charset.bits"

static unsigned char tx, ty;
static unsigned short coff;
unsigned char *font_bits, *font_info;
 
void set_font(unsigned char f)
{
	font_bits = (unsigned char*) charset_bits;
	font_info = (unsigned char*) charset_info;
	switch(f)
	{
		case 0:
				coff = 0;
				break;
		case 1:
				coff = 96;
				break;
		case 2:
				coff = 224;
				break;
		case 3:
		case 4:
				coff = 272;
				break;
	}
}

void _draw_char(unsigned int ch, unsigned char c, unsigned char m)
{
	unsigned char p,s;
	unsigned int cpos, cnt;

	cpos = 0;

	for(cnt=0; cnt<ch; cnt++)
	{
		if(cnt & 0x01)
		{
			cpos += (font_info[cnt>>1] & 0x0F);
		}
		else
		{
			cpos += (font_info[cnt>>1] >> 4);
		}
	}

	p = font_info[ch>>1];
	if(ch & 0x01)
	{
		p &= 0x0F;
	}
	else
	{
		p >>= 4;
	}

	if(ty & 0x07)
	{
		s = ty & 0x07;

		for(cnt=0;cnt<p;cnt++)
		{
			if(c & 0x02)
				drawbuf[0][p-cnt-1] = font_bits[cpos+cnt] << s;
			else
				drawbuf[0][p-cnt-1] = 0x00;

			if(c & 0x01)
				drawbuf[1][p-cnt-1] = font_bits[cpos+cnt] << s;
			else
				drawbuf[1][p-cnt-1] = 0x00;
		}
		do_rcu(tx,ty,p,m);

		for(cnt=0;cnt<p;cnt++)
		{
			if(c & 0x02)
				drawbuf[0][p-cnt-1] = font_bits[cpos+cnt] >> (8-s);
			else
				drawbuf[0][p-cnt-1] = 0x00;

			if(c & 0x01)
				drawbuf[1][p-cnt-1] = font_bits[cpos+cnt] >> (8-s);
			else
				drawbuf[1][p-cnt-1] = 0x00;
		}
		do_rcu(tx,ty+8,p,m);
	}
	else
	{
		for(cnt=0;cnt<p;cnt++)
		{
			if(c & 0x02)
				drawbuf[0][p-cnt-1] = font_bits[cpos+cnt];
			else
				drawbuf[0][p-cnt-1] = 0x00;

			if(c & 0x01)
				drawbuf[1][p-cnt-1] = font_bits[cpos+cnt];
			else
				drawbuf[1][p-cnt-1] = 0x00;
		}
		do_rcu(tx,ty,p,m);
	}
	tx += (p+1);
}

void draw_char(unsigned char x, unsigned char y, unsigned char ch, unsigned char c, unsigned char m)
{
	tx = x;
	ty = y;
	_draw_char(ch+coff, c, m);
}

void draw_string(unsigned char x, unsigned char y, char *st, unsigned char c, unsigned char m)
{
	unsigned char cp;
	tx = x;
	ty = y;

	cp = 0;
	while(st[cp] != 0)
	{
		_draw_char(st[cp]+coff, c, m);
		cp++;
	}
}

void draw_stringc(unsigned char x, unsigned char y, char *st, unsigned char cnt, unsigned char c, unsigned char m)
{
	unsigned char cp,cx;
	cx = cnt;
	tx = x;
	ty = y;

	cp = 0;
	while(cx--)
	{
		_draw_char(st[cp]+coff, c, m);
		cp++;
	}
}

extern char hval[16];

void draw_hexC(unsigned char x, unsigned char y, const unsigned char v, unsigned char c, unsigned char m)
{
	tx = x;
	ty = y;
	_draw_char(hval[v>>4]+coff, c, m);
	_draw_char(hval[v & 0x0F]+coff, c, m);
}

void draw_hexS(unsigned char x, unsigned char y, const unsigned short v, unsigned char c, unsigned char m)
{
	tx = x;
	ty = y;
	draw_hexC(x, y, v >> 8, c, m);
	draw_hexC(tx, ty, v & 0xFF, c, m);
}

void draw_hexW(unsigned char x, unsigned char y, const unsigned long v, unsigned char c, unsigned char m)
{
	draw_hexS(x, y, v >> 16, c, m);
	draw_hexS(tx, ty, v & 0xFFFF, c, m);
}
