#ifndef FONTY_H
#define FONTY_H

extern unsigned char drawbuf[2][128];

void set_font(unsigned char f);
void draw_char(unsigned char x, unsigned char y, unsigned char ch, unsigned char c, unsigned char m);
void draw_string(unsigned char x, unsigned char y, char *st, unsigned char c, unsigned char m);
void draw_stringc(unsigned char x, unsigned char y, char *st, unsigned char cnt, unsigned char c, unsigned char m);
void draw_hexC(unsigned char x, unsigned char y, const unsigned char v, unsigned char c, unsigned char m);
void draw_hexS(unsigned char x, unsigned char y, const unsigned short v, unsigned char c, unsigned char m);
void draw_hexW(unsigned char x, unsigned char y, const unsigned long v, unsigned char c, unsigned char m);

#endif
