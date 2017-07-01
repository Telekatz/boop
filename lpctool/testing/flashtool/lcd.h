#ifndef LCD_H
#define LCD_H

// the lcd data & command ports
#define LCD_CMD		(*((volatile unsigned char *) 0x81000000))
#define LCD_DATA	(*((volatile unsigned char *) 0x81000001))

// drawmodes 
#define DRAW_PUT	0
#define DRAW_XOR	1
#define DRAW_ERASE	2
#define DRAW_RAW	10
#define DRAW_RAW_INV	11

#define TIMEOUT 0x00400000

struct t_bitmap
{
	unsigned char width;
	unsigned char height;
	unsigned char mode;
	unsigned char *data;
} ;

extern unsigned long timer;

struct t_bitmap bitmap;

void lcd_init(unsigned char s);
void lcd_enable(unsigned char e);

void do_rcu(unsigned char x, unsigned char y, unsigned char l, unsigned char m);

void lcd_fill(unsigned char f);
void draw_block(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char c, unsigned char m);
void draw_hline(unsigned char x, unsigned char y, unsigned char l, unsigned char c, unsigned char m);
void draw_vline(unsigned char x, unsigned char y, unsigned char l, unsigned char c, unsigned char m);
void draw_rect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char lw, unsigned char c, unsigned char m);
void clearStatus(void);
void drawStatus(char *msg);

#endif
