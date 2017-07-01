/*
	 testmenu.c - testmenu for menuing system
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

#include "menu.h"
#include "lcd.h"
#include "fonty.h"
#include "keyboard.h"
#include "backlight.h"
#include "adc.h"
#include "rf.h"
#include "flash.h"
#include "ir_raw.h"
#include "ir_capture.h"
#include "ir_selector.h"
#include "infrared.h"
#include "sid.h"
#include "timerfuncs.h"
#include "sound.h"
#include "lpc2220.h"
#include "controls.h"
#include "global.h"
#include "cc1100.h"
#include "rf.h"
#include "irq.h"
#include "serial.h"
#include "rtctimer.h"

#include "icon.h"
#include "iconLeo_gray_128.h"

#define TRANSPARENT_CROSS   1   // 0/1
#if TRANSPARENT_CROSS
#   include "iconCross_t.h"
#   define ICON_CROSS   iconCross_t
#else
#   include "iconCross.h"
#   define ICON_CROSS   iconCross
#endif

#include "iconHourglas.h"
#include "iconHourglas_t.h"

//#include "sidfiles.h"

extern volatile unsigned char bat_state;

unsigned int testdata1, testdata2;

#define MAX_SOFTMENU	16

void test_circle(void)
{
   lcd_fill (LCD_COLOR_W);
	draw_disc (20, 20, 18, LCD_COLOR_B, DRAW_PUT);
	draw_disc (20, 20, 14, LCD_COLOR_B, DRAW_XOR);

	draw_qdisc (60, 20, 18, 0, LCD_COLOR_B, DRAW_PUT);
	draw_qdisc (60, 40, 18, 1, LCD_COLOR_B, DRAW_PUT);
	draw_qdisc (60, 60, 18, 2, LCD_COLOR_B, DRAW_PUT);
	draw_qdisc (60, 80, 18, 3, LCD_COLOR_B, DRAW_PUT);

	draw_qcircle (100, 20, 18, 0, LCD_COLOR_B, DRAW_PUT);
	draw_qcircle (100, 40, 18, 1, LCD_COLOR_B, DRAW_PUT);
	draw_qcircle (100, 60, 18, 2, LCD_COLOR_B, DRAW_PUT);
	draw_qcircle (100, 80, 18, 3, LCD_COLOR_B, DRAW_PUT);


	draw_circle (30,120,20, LCD_COLOR_B, DRAW_PUT);

	waitKeyUpDown();
}

void test_movearound(void)
{
	int xpos=0;
	int ypos=0;
	int oldx=0;
	int oldy=0;

   lcd_fill (LCD_COLOR_W);
	draw_disc (xpos+5, ypos+5, 5, LCD_COLOR_B, DRAW_PUT);
	while (!KEY_Exit)
	{
		waitKeyUpDown();
		oldx=xpos;
		oldy=ypos;
		if (KEY_Up)
			ypos -= 3;
		if (KEY_Down)
			ypos += 3;
		if (KEY_Left)
			xpos -= 3;
		if (KEY_Right)
			xpos += 3;

		if (xpos > 117)
			xpos = 117;
		if (xpos < 0)
			xpos = 0;
		if (ypos > 149)
			ypos = 149;
		if (ypos < 0)
			ypos = 0;

		draw_disc (oldx+5, oldy+5, 5, LCD_COLOR_B, DRAW_ERASE);
		draw_disc (xpos+5, ypos+5, 5, LCD_COLOR_B, DRAW_PUT);
	}
}


void test_frame(void)
{
   lcd_fill (LCD_COLOR_W);
	
	draw_frame (0,0,40,40,4,0, LCD_COLOR_B, DRAW_PUT);
	draw_frame (42,0,40,40,3,16, LCD_COLOR_B, DRAW_PUT);
	draw_frame (84,0,40,40,7,3, LCD_COLOR_B, DRAW_PUT);


	draw_frame (0,42,40,40,4,7, LCD_COLOR_B, DRAW_PUT);
	draw_frame (42,42,40,40,4,8, LCD_COLOR_B, DRAW_PUT);
	draw_frame (84,42,40,40,1,7, LCD_COLOR_B, DRAW_PUT);

	waitKeyUpDown();
}

void test_frame2(void)
{

	unsigned char t, r;
	t = 1;
	r = 0;
again:
   lcd_fill (LCD_COLOR_W);

	draw_string(0, 152, "t:", LCD_COLOR_B, DRAW_PUT);
	draw_numU(12,152,t,0,LCD_COLOR_B,DRAW_PUT);
	draw_string(64, 152, "r:", LCD_COLOR_B, DRAW_PUT);
	draw_numU(76,152,r,0,LCD_COLOR_B,DRAW_PUT);
	
   draw_frame (0,0,LCD_SIZE_X,128,t,r, LCD_COLOR_B, DRAW_PUT);

	while(!KEY_Exit) 
	{
		waitKeyUpDown();
		if(KEY_Vplus)
		{
			t++;
			if(t>64)
				t = 1;
			goto again;
		}
		if(KEY_Vminus)
		{
			if(t==1)
				t = 64;
			else
				t--;
			goto again;
		}
		if(KEY_Pplus)
		{
			r++;
			if(r>63)
				r = 0;
			goto again;
		}
		if(KEY_Pminus)
		{
			if(r==0)
				r = 63;
			else
				r--;
			goto again;
		}
	}
}

#define DRAW_FONTS_LINESPACE	1
#define DRAW_FONTS_XSTART		15
void draw_fonts ()
{
	unsigned short y = 10;
	unsigned short h;

   lcd_fill (LCD_COLOR_W);
#if 0
	set_font (SMOOTHFONT);
	draw_string (0,	y,	"‹J", LCD_COLOR_B, DRAW_PUT);
	draw_string (15,  y+1, "‹J", LCD_COLOR_B, DRAW_PUT);
	draw_string (30,  y+2, "‹J", LCD_COLOR_B, DRAW_PUT);
	draw_string (45,  y+3, "‹J", LCD_COLOR_B, DRAW_PUT);
	draw_string (60,  y+5, "‹J", LCD_COLOR_B, DRAW_PUT);
	draw_string (75,  y+6, "‹J", LCD_COLOR_B, DRAW_PUT);
	draw_string (90,  y+7, "‹J", LCD_COLOR_B, DRAW_PUT);
	draw_string (105, y+8, "‹J", LCD_COLOR_B, DRAW_PUT);
	y += 8;
	y += SMOOTHFONT_HEIGHT + DRAW_FONTS_LINESPACE;
#endif
	
	set_font (SMALLFONT);
	h = get_stringheight ("g‹J");
	draw_block (0, y, DRAW_FONTS_XSTART-1, SMALLFONT_HEIGHT, LCD_COLOR_B, DRAW_PUT);
	set_font (SMALLFONT);
	draw_numU (1, y, h, 0, LCD_COLOR_B, DRAW_XOR);
	set_font(SMALLFONT);
	draw_string (DRAW_FONTS_XSTART, y, "Small Font", LCD_COLOR_B, DRAW_PUT);
	y += SMALLFONT_HEIGHT + DRAW_FONTS_LINESPACE;

	set_font (DOUBLEFONT);
	h = get_stringheight ("g‹J");
	draw_block (0, y, DRAW_FONTS_XSTART-1, DOUBLEFONT_HEIGHT, LCD_COLOR_B, DRAW_PUT);
	set_font (SMALLFONT);
	draw_numU (1, y+1, h, 0, LCD_COLOR_B, DRAW_XOR);
	set_font(DOUBLEFONT);
	draw_string (DRAW_FONTS_XSTART, y, "Double Font", LCD_COLOR_B, DRAW_PUT);
	y += DOUBLEFONT_HEIGHT + DRAW_FONTS_LINESPACE;
	
	set_font(BOLDFONT);
	h = get_stringheight ("g‹J");
	draw_block (0, y, DRAW_FONTS_XSTART-1, BOLDFONT_HEIGHT, LCD_COLOR_B, DRAW_PUT);
	set_font (SMALLFONT);
	draw_numU (1, y+1, h, 0, LCD_COLOR_B, DRAW_XOR);
	set_font (BOLDFONT);
	draw_string (DRAW_FONTS_XSTART, y, "Bold Font", LCD_COLOR_B, DRAW_PUT);
	y += BOLDFONT_HEIGHT + DRAW_FONTS_LINESPACE;

	set_font (BOLDDOUBLEFONT);
	h = get_stringheight ("g‹J");
	draw_block (0, y, DRAW_FONTS_XSTART-1, BOLDDOUBLEFONT_HEIGHT, LCD_COLOR_B, DRAW_PUT);
	set_font (SMALLFONT);
	draw_numU (1, y+1, h, 0, LCD_COLOR_B, DRAW_XOR);
	set_font(BOLDDOUBLEFONT);
	draw_string (DRAW_FONTS_XSTART, y, "B.Double", LCD_COLOR_B, DRAW_PUT);
	y += BOLDDOUBLEFONT_HEIGHT + DRAW_FONTS_LINESPACE;
	
	set_font (SMALLFONT);
	h = get_stringheight ("g‹J");
	draw_block (0, y+1, DRAW_FONTS_XSTART-1, SMALLFONT_HEIGHT, LCD_COLOR_B, DRAW_PUT);
	set_font (SMALLFONT);
	draw_numU  (1, y+1, h, 0, LCD_COLOR_B, DRAW_XOR);
	set_font (SMALLFONT);
	draw_string (DRAW_FONTS_XSTART+1, y + 1, "Invers Small Font", LCD_COLOR_B, DRAW_PUT);
   draw_block  (DRAW_FONTS_XSTART, y, LCD_SIZE_X, SMALLFONT_HEIGHT+2, LCD_COLOR_B, DRAW_XOR);
	y += SMALLFONT_HEIGHT + 2 + DRAW_FONTS_LINESPACE;
	
	set_font (SMOOTHFONT);
	h = get_stringheight ("g‹J");
	draw_block (0, y, DRAW_FONTS_XSTART-1, SMOOTHFONT_HEIGHT, LCD_COLOR_B, DRAW_PUT);
	set_font (SMALLFONT);
	draw_numU (1, y+1, h, 0, LCD_COLOR_B, DRAW_XOR);
	set_font(SMOOTHFONT);
	draw_string (DRAW_FONTS_XSTART, y, "Smooth Font", LCD_COLOR_B, DRAW_PUT);
	y += SMOOTHFONT_HEIGHT + DRAW_FONTS_LINESPACE;
	
	set_font (SMOOTHFONT);
	h = get_stringheight ("g‹J");
	draw_block (0, y+1, DRAW_FONTS_XSTART-1, SMOOTHFONT_HEIGHT, LCD_COLOR_B, DRAW_PUT);
	set_font (SMALLFONT);
	draw_numU (1, y+2, h, 0, LCD_COLOR_B, DRAW_XOR);
	set_font (SMOOTHFONT);
	draw_string (DRAW_FONTS_XSTART+1, y + 1, "Invers Smooth", LCD_COLOR_B, DRAW_PUT);
   draw_block  (DRAW_FONTS_XSTART, y, LCD_SIZE_X, SMOOTHFONT_HEIGHT+2, LCD_COLOR_B, DRAW_XOR);
	y += SMOOTHFONT_HEIGHT + 2 + DRAW_FONTS_LINESPACE;
	
	set_font(BOLDFONT);
	waitKeyUpDown();
}

void test_logo(void)
{
	lcd_fill (LCD_COLOR_W);
	draw_logo();
	waitKeyUpDown();
}



//-----------------------------------------------------------------------------
#undef  ICON_INFO
#define ICON_INFO(i)

#undef  ICON_INFO_ELEMENT
#define ICON_INFO_ELEMENT

#undef  ICON
#define ICON   iconElement_t

#include "iconFr.h"
#include "iconPlay.h"
#include "iconPause.h"
#include "iconStop.h"
#include "iconFf.h"

static const ICON* const iconList [] =
{
   &iconFr,
   &iconPlay,
   &iconPause,
   &iconStop,
   &iconFf
};

//-----------------------------------------------------------------------------
#define HOURGLAS_CENTER   120
void test_DrawIcon (int x, int y, int h, int w)
{
   int i;
   int ix;
   
   lcd_fill (LCD_COLOR_W);
   
   draw_block (LCD_SIZE_X / 2, LCD_SIZE_Y / 2,
      32, HOURGLAS_CENTER - LCD_SIZE_Y / 2 + 2,
      LCD_COLOR_LG, DRAW_NORCU);
   draw_block (LCD_SIZE_X / 2, HOURGLAS_CENTER,
      32, LCD_SIZE_Y - HOURGLAS_CENTER, LCD_COLOR_DG, DRAW_NORCU);
   
   drawIcon (0, 10, &iconLeo_gray_128, LCD_COLOR_B, DRAW_PUT);
   
   drawIcon (LCD_SIZE_X / 2 - iconHourglas.width / 2, HOURGLAS_CENTER - iconHourglas.height / 2,
      &iconHourglas, LCD_COLOR_B, DRAW_PUT);
   drawIcon (LCD_SIZE_X / 2 + 32 - iconHourglas_t.width / 2, HOURGLAS_CENTER - iconHourglas.height / 2,
      &iconHourglas_t, LCD_COLOR_B, DRAW_ALPHA);
   
#if TRANSPARENT_CROSS
   drawIcon (x, y, &ICON_CROSS, LCD_COLOR_B, DRAW_ALPHA);
#else
   drawIcon (x, y, &ICON_CROSS, LCD_COLOR_B, DRAW_PUT);
#endif

   //-------------------------------------------------------
   ix = w;
   for (i = 0; i < NUM_OF_ELEMENTS (iconList); i++)
   {
      drawIconExt (ix, LCD_SIZE_Y-1 - h,
         iconList [i]->data,
         iconList [i]->width, iconList [i]->height, ICON_GRAY,
         LCD_COLOR_B, DRAW_NORCU);
      ix += w + iconList [i]->width;
   }
   //-------------------------------------------------------
   draw_numU (0,  0,
      x, 0, LCD_COLOR_B, DRAW_NORCU);
   draw_numU (30, 0,
      y, 0, LCD_COLOR_B, DRAW_NORCU);
}   // test_DrawIcon
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void test_icon ()
{
   int i;
   int x, y;
   int w = 0;
   int h = 0;

   set_font (SMALLFONT);

   x = LCD_SIZE_X / 2 - ICON_CROSS.width / 2;
   y = LCD_SIZE_Y / 2 - ICON_CROSS.height / 2;

   for (i = 0; i < NUM_OF_ELEMENTS (iconList); i++)
   {
      w += iconList [i]->width;
      h = max (h, iconList [i]->height);
   }
   w = LCD_SIZE_X - w;
   w /= NUM_OF_ELEMENTS (iconList) + 1;

   test_DrawIcon (x, y, h, w);

   //waitKeyUpDown ();

   setBacklight (BL_AUTO);    
   while (KEY_Exit) {};
   waitKeyUp ();
   while (1)
   {
      if (isKeyPressed ())
      {
         setBacklight (BL_AUTO);    // pwm value
         if (KEY_Exit)
            return;
         //---------------------------------------

         if (KEY_Up)
            y--;
         else if (KEY_Down)
            y++;
         else if (KEY_Left)
            x--;
         else if (KEY_Right)
            x++;
         else if (KEY_OK)
         {
            x += 10;
            y -= 10;
         }
         else
         {
            unsigned char num = getKeynum ();
            if (num >= 16 && num <= 25)
            {
               if (num == 25)
                  num = 0;
               else
                  num -= 15;

               x += num;
               y -= num;
            }
         }

         if (x >= LCD_SIZE_X)
            x = 0;
         else if  (x < 0)
            x = LCD_SIZE_X-1 - ICON_CROSS.width;
         
         if (y >= LCD_SIZE_Y)
            y = 0;
         else if (y < 0)
            y = LCD_SIZE_Y-1 - ICON_CROSS.height;
         
         test_DrawIcon (x, y, h, w);

         //---------------------------------------
         waitKeyUp ();
      }   // if (isKeyPressed ())
   }   // while (1)
}   // test_icon
//-----------------------------------------------------------------------------



void fp1 ()
{
	unsigned char x,y;
   lcd_fill (LCD_COLOR_W);
	set_font(SMALLFONT);

	for(x=0;x<16;x++)
	{
		for(y=0;y<16;y++)
		{
			draw_char (x*8, (y*8)+16,x*16+y, LCD_COLOR_B, DRAW_PUT);
		}
	}
	waitKeyUpDown();
}

extern volatile unsigned char bl_val;

void set_bl(void)
{
   lcd_fill (LCD_COLOR_W);
	set_font(SMALLFONT);

	draw_string (0, 20, "Set Backlight:", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 30, "Prog/Vol+: brighter", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 40, "Prog/Vol-: darker", LCD_COLOR_B, DRAW_PUT);
	draw_numU (0, 64, bl_val, 0, LCD_COLOR_B, DRAW_PUT);
	
	while(!KEY_Exit)
	{
		waitKeyUpDown();
		bl_val &= 0x3F;
		if(KEY_Pplus || KEY_Pminus || KEY_Vplus || KEY_Vminus)
		{	
			draw_block(0,64,32,8,3,DRAW_ERASE);
			if(KEY_Pplus)
				bl_val++;
			if(KEY_Pminus)
				bl_val--;
			if(KEY_Vplus)
				bl_val+=10;
			if(KEY_Vminus)
				bl_val-=10;
			draw_numU (0, 64, bl_val, 0, LCD_COLOR_B, DRAW_PUT);
		}
	}

}

/*
void softfunc1(void)
{
	set_font(BOLDFONT);
draw_block (0, 80, LCD_SIZE_X, 20, LCD_COLOR_B, DRAW_ERASE);
draw_string (0, 80, "softfunc1 called", LCD_COLOR_B, DRAW_PUT);
while(!KEY_Exit) {};
}
*/

void draw_itile(unsigned char x, unsigned char y, unsigned char type)
{
again:
	if(type < 0xF0)
	{
		if(type & 0x10)
			draw_block(x+1,	y+1,	2,	2,	3,	DRAW_XOR);
		if(type & 0x20)
			draw_block(x+5,	y+1,	2,	2,	3,	DRAW_XOR);
		if(type & 0x40)
			draw_block(x+1,	y+5,	2,	2,	3,	DRAW_XOR);
		if(type & 0x80)
			draw_block(x+5,	y+5,	2,	2,	3,	DRAW_XOR);
	}

	type <<= 4;

	if(type)
	{
		x += 8;
		goto again;
	}
}

const char itilemap[7][7] = 
{
	{ 0x00, 0x00, 0x00, 0x6C, 0x90, 0x00, 0x00 },
	{ 0x00, 0x00, 0x66, 0x6C, 0x99, 0x90, 0x00 },
	{ 0x00, 0x06, 0x66, 0x6C, 0x99, 0x99, 0x00 },
	{ 0x00, 0x66, 0x66, 0x6C, 0x99, 0x99, 0x90 },
	{ 0x00, 0x66, 0x66, 0x6C, 0x99, 0x99, 0x90 },
	{ 0x06, 0x66, 0x66, 0x6C, 0x99, 0x99, 0x99 },
	{ 0x0A, 0xAA, 0xAA, 0xA0, 0x55, 0x55, 0x55 }
}; 

void illusion(void)
{
	int x,y,z;

   lcd_fill (LCD_COLOR_W);
	z = 1;
	for(x=0;x<15;x++)
	{
		for(y=0;y<15;y++)
		{
			if(z++ & 0x01)
			{
				draw_block (2+ (x*8), 16+ (y*8), 8, 8, LCD_COLOR_B, DRAW_PUT);
			}
		}
	}

	for(y=0;y<7;y++)
	{
		for(x=0;x<7;x++)
		{
			draw_itile(2+(x*16), 24+(y*8), itilemap[y][x]);
		}
	}

	for(y=0;y<6;y++)
	{
		for(x=0;x<7;x++)
		{
			draw_itile(2+(x*16), 80+(y*8), ~itilemap[5-y][x]);
		}
	}


	waitKeyUpDown();
}

void test_bl(void)
{
	setBacklight(BL_AUTO);
}

void bat_dbg(void)
{
	bat_state ^= BAT_DEBUG;
}

void tkey(void)
{
	while(1)
	{
		waitKeyUpDown();
		setBacklight(BL_AUTO);
      lcd_fill (LCD_COLOR_W);
		set_font(SMALLFONT);
		draw_hexW (0, 16, keys[0], LCD_COLOR_B, DRAW_PUT);
		draw_hexW (0, 24, keys[1], LCD_COLOR_B, DRAW_PUT);
	}
}

// plot a graph RSSI/t
void RSSI_monitor(void) {
	unsigned char row = 0;
	int rssi_dBm;
	unsigned char rssi;
	volatile unsigned long timer = 0;
	unsigned char x;
   lcd_fill (LCD_COLOR_W);
	
	// title
	set_font(BOLDFONT);
	draw_string (0, 0, "RF test", LCD_COLOR_B, DRAW_PUT);
   draw_block (0,10,LCD_SIZE_X,2, LCD_COLOR_B, DRAW_PUT);
	set_font(SMALLFONT);
	
	RF_changestate(RFrx);
	
	// coordinate system
	// x (time)
	draw_block (19,20,102,1, LCD_COLOR_B, DRAW_PUT);
	
	// y (RSSI)
	draw_block (19,20,1,129, LCD_COLOR_B, DRAW_PUT);   // axis
	for(x=20;x<150;x+=10)   // ticks
		draw_pixel (18,x, LCD_COLOR_B, DRAW_PUT);
	for(x=20;x<160;x+=20)   // value
		draw_numU (0, x-3, x-20, 0, LCD_COLOR_B, DRAW_PUT);
	draw_string (0,152, "-dBm", LCD_COLOR_B, DRAW_PUT);   // label
	
	
	while(KEY_Exit) {};
	while (!KEY_Exit)
	{
		// wait
		while (timer)
		{
			if(KEY_Exit)
				timer = 0;
		}
		addTimeout(&timer,20);	// next sample in n*5 ms (100 ms)
		
		// read RSSI
		VICIntEnClr = INT_EINT0;
		rssi = cc1100_read1(RSSI);
		VICIntEnable = INT_EINT0;
		
		// scale
		if(rssi & 0x80)
			rssi_dBm = (rssi-256)/2-75;
		else
			rssi_dBm = rssi/2-75;
		
		if(rssi_dBm < 0)
			rssi_dBm = ~rssi_dBm + 1;
		else
			rssi_dBm = 0;
		
		// plot
      draw_block (row+20,21,1,128, LCD_COLOR_B, DRAW_ERASE);
		draw_pixel(row+20, rssi_dBm+30, LCD_COLOR_B, DRAW_PUT);
      draw_pixel (row+20,19, LCD_COLOR_B,DRAW_ERASE);
		
		// next time
		row++;
		if(row == 100)
			row=0;
		draw_pixel (row+20, 19, LCD_COLOR_B, DRAW_PUT);
		//draw_block (row+20,30,1,100, LCD_COLOR_B, DRAW_PUT);
	}
	removeTimeout(&timer);
	RF_changestate(RFwor);
}


// plot incoming radio data in single shot mode
// CC1100 datasheet section 31.1 Asynchronous Operation, p. 57
// 1 px is 50 ys
void RF_viewer(void) {
	unsigned char row = 0;
	unsigned char col = 16;
	
   lcd_fill (LCD_COLOR_W);
	set_font(BOLDFONT);
	draw_string (0, 0, "RF viewer", LCD_COLOR_B, DRAW_PUT);
   draw_block (0,10,LCD_SIZE_X,2, LCD_COLOR_B, DRAW_PUT);
	set_font(SMALLFONT);
	
	RFasyncmode(true);

	//cc1100_write1(FREQ2,0x20);
	//cc1100_write1(FREQ1,0x29);
	//cc1100_write1(FREQ0,0x3E);
	
	//cc1100_write1(AGCCTRL2,0x07);
	//cc1100_write1(AGCCTRL1,0x00);
	//cc1100_write1(AGCCTRL0,0x91);
	//~ cc1100_write1(AGCCTRL2,0x03);
	//~ cc1100_write1(AGCCTRL1,0x00);
	//~ cc1100_write1(AGCCTRL0,0x92);
	
//~ #define RF2UART
	
#ifdef RF2UART
	unsigned char bits=0;
	unsigned char bitno=7;

	// set uart baudrate
	serial_setbrg(230400);	// fastest
	
	// set frequency
	//~ cc1100_write1(FREQ2,0x10);
	//~ cc1100_write1(FREQ1,0x12);
	//~ cc1100_write1(FREQ0,0xF6);
	
	// set gain							// OOK/ASK optima. see TI DN022
	//~ cc1100_write1(AGCCTRL2,0x07);	// A
	cc1100_write1(AGCCTRL2,0x03);	// B
	cc1100_write1(AGCCTRL1,0x00);
	//~ cc1100_write1(AGCCTRL0,0x92);	// A
	cc1100_write1(AGCCTRL0,0x91);	// B
#endif
	
	cc1100_strobe(SFRX);
	cc1100_strobe(SRX);
	
	while(KEY_Exit) {};
	while(!KEY_Exit)
	{
#ifdef RF2UART
		//~ waitus(100);	// 1 byte -> 10 bits / 115200 baud = 86,8 ys 
		waitus(50);	// 1 byte -> 10 bits / 230400 baud = 43,4 ys 
		
		if (!(FIOPIN0 & GDO2))					// get rf state as bit
			bits |= (1<<bitno);
		
		if(bitno == 0)							// byte complete? -> uart
		{
			bitno = 7;
			serial_putbyte(bits);
			bits = 0;
		}
		else
			bitno--;							// advance
		
#else
		waitus(50);

		draw_block(row,col,2,8,3,DRAW_ERASE);
		
		// plot: get data from CC1100's GDO2 on P0.24 (fast IO)
		if((FIOPIN0 & GDO2))
			draw_pixel (row,col+2, LCD_COLOR_B, DRAW_PUT);
		else
			draw_pixel (row,col, LCD_COLOR_B, DRAW_PUT);	 
		
		// advance plot position
		row++;
		if(row == 127)
		{
			col +=8;
			row=0;
			
			// screen is full, wait for keypress
			if(col >140) 
			{
				wait5ms();
				col = 16;
				while(!(keys[0] || keys[1]));// && !(FIOPIN0 & GDO2));	// this seems strange, alternatives: while(!ANYKEY) or isKeyPressed
			}
		}
#endif
	}
	RFasyncmode(false);
}


void test_RF(void) {
	
	unsigned char destAddr;
	
   lcd_fill (LCD_COLOR_W);
	destAddr = 0x01;
	
	set_font(BOLDFONT);
	draw_string (0, 0, "RF test", LCD_COLOR_B, DRAW_PUT);
   draw_block (0,10,LCD_SIZE_X,2, LCD_COLOR_B, DRAW_PUT);
   draw_block (0,149,LCD_SIZE_X,2, LCD_COLOR_B, DRAW_PUT);
	draw_string (1, 15, "Destination address", LCD_COLOR_B, DRAW_PUT);
	set_font(SMALLFONT);
	
	draw_string (0, 60, "TV/Power Key:", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 69, "send something", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 86, "OK Key: Ping", LCD_COLOR_B, DRAW_PUT);

	draw_numU (1, 25, destAddr, 0, LCD_COLOR_B, DRAW_PUT);
	
	while(KEY_Exit) {};
	while(!KEY_Exit)
	{
		waitKeyUpDown();
		
		
		draw_numU (1, 25, destAddr, 0, LCD_COLOR_B, DRAW_XOR);
		if(KEY_Left) {
			destAddr--;
		}
		if(KEY_Right) {
			destAddr++;
		}
		draw_numU (1, 25, destAddr, 0, LCD_COLOR_B, DRAW_XOR);
		
		
		if(KEY_OK)
		{
			unsigned char ping;
         draw_block (0,40,LCD_SIZE_X,10,3,DRAW_ERASE);
			draw_string (0, 40, "send ping", LCD_COLOR_B, DRAW_PUT);
			
			ping = RF_sendping(destAddr);
			
         draw_block (0,40,LCD_SIZE_X,10,3,DRAW_ERASE);
			if (ping) {
				draw_string (0, 40, "ping received", LCD_COLOR_B, DRAW_PUT);
			}
			else {
				draw_string (0, 40, "ping timeout", LCD_COLOR_B, DRAW_PUT);
			}
			
		}
		else if(KEY_Power)
		{
			struct RFendpoint_* cur_ep;
			
			cur_ep = openEP(0,0, packet_test);
			if(cur_ep) {
				cur_ep->dest = destAddr;
				cur_ep->data[0] = 'P';
				cur_ep->data[1] = 'W';
				cur_ep->data[2] = 0x00;
				cur_ep->bufferlen = 3;
				cur_ep->flags |= EPenabled | EPoutput | EPnewdata | EPonce | EPsendwor;
				
				RF_changestate(RFtx);
			}
		}
		else if(KEY_TV)
		{	
			struct RFendpoint_* cur_ep;
			
			cur_ep = openEP(0,0, packet_test);
			cur_ep->dest = destAddr;
			cur_ep->data[0] = 'T';
			cur_ep->data[1] = 'V';
			cur_ep->data[2] = 0x00;
			cur_ep->bufferlen = 3;
			cur_ep->flags |= EPenabled | EPoutput | EPnewdata | EPonce | EPsendwor;
			
			RF_changestate(RFtx);
		}
	}
}


void test_irrec(void) {
	
	struct RAWcmd_ test_cmd;
	
	test_cmd.tval = 0;
	
   lcd_fill (LCD_COLOR_W);

	set_font(BOLDFONT);
	draw_string (0, 0, "IR receive test", LCD_COLOR_B, DRAW_PUT);
   draw_block (0,10,LCD_SIZE_X,2, LCD_COLOR_B, DRAW_PUT);
   draw_block (0,149,LCD_SIZE_X,2, LCD_COLOR_B, DRAW_PUT);
	set_font(SMALLFONT);
	
	draw_string (0, 20, "OK Key:", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 29, "receive IR command", LCD_COLOR_B, DRAW_PUT);
	
	draw_string (0, 40, "Mute Key:", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 49, "send received command", LCD_COLOR_B, DRAW_PUT);
	
	while(KEY_Exit) {};
	while(!KEY_Exit)
	{
		waitKeyUpDown();
		if(KEY_OK)
		{
			getIR_test(&test_cmd);
		}
		if(KEY_Mute)
		{
			RAW_Send((unsigned long)&test_cmd);
			while(KEY_Mute)
				RAW_Repeat();
			RAW_Stop();
		}
	}
	setEncoder(irDevTab.device[irDevTab.active].encoder, irDevTab.device[irDevTab.active].set);
}

void erase_flash1(void)
{	int x;

	x = eraseFlash(1);
	
}

void test_sid(void) {
	unsigned char temp;
	
   lcd_fill (LCD_COLOR_W);

	set_font(BOLDFONT);
	draw_string (0, 0, "SID", LCD_COLOR_B, DRAW_PUT);
   draw_block (0,10,LCD_SIZE_X,2, LCD_COLOR_B, DRAW_PUT);
   draw_block (0,149,LCD_SIZE_X,2, LCD_COLOR_B, DRAW_PUT);
	set_font(SMALLFONT);
	
	draw_string (0, 20, "A B C D", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 29, "set envelope", LCD_COLOR_B, DRAW_PUT);
	
	draw_string (0, 45, "OK", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 54, "play a song", LCD_COLOR_B, DRAW_PUT);
	
	draw_string (0, 70, "1-7", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 79, "play a note", LCD_COLOR_B, DRAW_PUT);
	
	draw_string (0, 95, "color keys", LCD_COLOR_B, DRAW_PUT);
	draw_string (0, 104, "set waveform", LCD_COLOR_B, DRAW_PUT);
	
	sysInfo |= SYS_IR;
	SID.noise = 0xaa;
	playstate = 0x00;
	playtone[0] = 0;
	playtone[1] = 0;
	playtone[2] = 0;
	temp = 0;
	PWMPR = 0;
	switchSound(SOUND_ON);
	SID.flags |= SIDenable;
	SIDsetwave(0,TRIANGLE);
	SIDsetadsr(0,8,8,10,8);
	
	while(KEY_Exit) {};
	do 	{
		waitKeyUp();
		if (temp)
		{
			SID.reg[REG_Control] &= ~GATE;
			temp = 0;
		}
		waitKeyDown();
		if (KEY_1)
		{
			SIDsetfrq(0,4455);
			SID.reg[REG_Control] |= GATE;
			temp =1;
		}
		else if (KEY_2)
		{
			SIDsetfrq(0,5001);
			SID.reg[REG_Control] |= GATE;
			temp =1;
		}
		else if (KEY_3)
		{
			SIDsetfrq(0,5613);
			SID.reg[REG_Control] |= GATE;
			temp =1;
		}
		else if (KEY_4)
		{
			SIDsetfrq(0,5947);
			SID.reg[REG_Control] |= GATE;
			temp =1;
		}
		else if (KEY_5)
		{
			SIDsetfrq(0,6676);
			SID.reg[REG_Control] |= GATE;
			temp =1;
		}
		else if (KEY_6)
		{
			SIDsetfrq(0,7493);
			SID.reg[REG_Control] |= GATE;
			temp =1;
		}
		else if (KEY_7)
		{
			SIDsetfrq(0,8411);
			SID.reg[REG_Control] |= GATE;
			temp =1;
		}
		else if (KEY_A)
		{
			SIDsetadsr(0,0,10,7,5);
		}
		else if (KEY_B)
		{
			SIDsetadsr(0,0,10,0,10);
		}
		else if (KEY_C)
		{
			SIDsetadsr(0,8,8,10,6);
		}
		else if (KEY_D)
		{
			SIDsetadsr(0,0,0,15,1);
		}
		else if (KEY_Red)
		{
			SIDsetwave(0,TRIANGLE);
		}
		else if (KEY_Green)
		{
			SIDsetwave(0,SAWTOOTH);
		}
		else if (KEY_Yellow)
		{
			SID.reg[REG_PWhi] = 0x08;
			SIDsetwave(0,RECTANGLE);
		}
		else if (KEY_Blue)
		{
			SIDsetwave(0,NOISE);
		}
		else if (KEY_OK)
		{
			if (playstate == 0)
			{
				playstate = 1;
				
				SIDsetadsr(1,0,5,0,5);
				SIDsetwave(1,SAWTOOTH);
				SID.reg[7+REG_PWhi] = 0x06;
				tonelen[1] = 0;
				playtone[1] = (unsigned char*)&song3[0];
				
				SIDsetadsr(2,0,9,0,9);
				SIDsetwave(2,RECTANGLE);
				SID.reg[14+REG_PWhi] = 0x08;
				tonelen[2] = 0;
				playtone[2] = (unsigned char*)&song2[0];
				
				playtone_cb = addTimerCB(SIDplaytone, 4);
				startCB(playtone_cb);
			}
		}
	} while (!KEY_Exit);
	sysInfo &= ~SYS_IR;
	switchSound(SOUND_OFF);
	SID.flags &= ~SIDenable;
}

void test_click(FORM* form, CONTROL* control) {
	unsigned char x;
	x=msgbox(45,BC_OKOnly | BC_DefaultButton1,control->caption);
	form_draw(form);
}

void test2_click(FORM* form, CONTROL* control) {
	beep(0);
}

void test3_click(FORM* form, CONTROL* control) {
	beep(1);
}

void test_numbox(FORM* form, CONTROL* control) {
	unsigned char x;
	x=msgbox(30,BC_OKOnly | BC_DefaultButton1,"numbox click");
	form_draw(form);
}

void test_numchange(FORM* form, CONTROL* control) {
	unsigned short x;
	x=((NUMBOX*)control)->value;
	if(x==50) {
		x=msgbox(30,BC_OKOnly | BC_DefaultButton1,"numbox =50");
		form_draw(form);
	}
}

void test_change(FORM* form, CONTROL* control) {
	unsigned short x;
	x=msgbox(30,BC_OKOnly | BC_DefaultButton1,"geaendert");
	form_draw(form);
}

void formload (FORM* form)
{
	unsigned char x;
	struct RFendpoint_* cur_ep;
	
	cur_ep = openEP(&(form->controls[7])->tag1, 1, packet_RFenc);
	form->tag = (unsigned long) cur_ep;
	if (cur_ep)
	{
		cur_ep->flags |= EPenabled;
		RF.flags &= ~WORenabled;
		RF_changestate(RFrx);
	}
	
	x=msgbox(50,BC_OKOnly | BC_DefaultButton1,"Form open");
}

void formclose (FORM* form)
{
	unsigned char x;
	
	if (form->tag)
	{
		closeEP((struct RFendpoint_*)form->tag);
		RF_changestate(RFwor);
	}

	x=msgbox(50,BC_OKOnly | BC_DefaultButton1,"Form close");
	
}

void test_timer (FORM* form)
{
	struct RFendpoint_* cur_ep;
	
	cur_ep = (struct RFendpoint_*)form->tag;
	if (cur_ep->flags & EPnewdata)
	{
		cur_ep->flags &= ~EPnewdata;
		control_draw(form->controls[7], (form->sel == 7));		
	}
}

void test_controls(void) {
	
	const char* listbox1items[5] = {	"Listitem1",
										"Listitem2",
										"Listitem3",
										"Listitem4",
										"Listitem555"};
	
	char txtbuffer[20];
	const LABEL label1		= {BC_Label,0,15,40,10,SMALLFONT,0,"label1",test_click};
	const LABEL label2		= {BC_Labelnotab,50,15,40,9,BOLDFONT,0,"label2",0};
	const LABEL label3		= {BC_Labelnotab,0,110,40,9,BOLDFONT,0,"Numbox:",0};
	const BUTTON button1	= {BC_Button,0,30,40,15,SMALLFONT,0,"Button1",test_click};
	const BUTTON button2	= {BC_Button,0,50,40,16,SMALLFONT,0,"Button2",test2_click};
	const BUTTON button3	= {BC_Button,0,70,40,16,SMALLFONT,0,"Button3",test3_click};
	const BUTTON button4	= {BC_Button | BC_Disabled,0,90,40,16,SMALLFONT,0,"Button4",test_click};
	NUMBOX numbox1			= {BC_Numbox,0,120,20,9,SMALLFONT,1,1,500,test_numbox,test_numchange};
	TXTBOX txtbox1			= {BC_Txtbox,0,135,120,10,SMALLFONT,20,0,txtbuffer,test_click,test_change};
	DATEBOX datebox1		= {BC_Datebox,50,30,58,9,SMALLFONT,0,{0x01, 0x02, 0x01, 0x08},NULL,test_change};
	TIMEBOX timebox1		= {BC_Timebox,50,50,58,9,SMALLFONT,0,{0x00, 0x00, 0x12, 0x00},NULL,test_change};
	LISTBOX listbox1		= {BC_Listbox,50,70,58,10,SMALLFONT,0,5,(char**)listbox1items,NULL,test_change};
	
	CONTROL* controls[12] = {	(CONTROL*)&label1,
								(CONTROL*)&label2,
								(CONTROL*)&label3,
								(CONTROL*)&button1,
								(CONTROL*)&button2,
								(CONTROL*)&button3,
								(CONTROL*)&button4,
								(CONTROL*)&numbox1,
								(CONTROL*)&txtbox1,
								(CONTROL*)&datebox1,
								(CONTROL*)&timebox1,
								(CONTROL*)&listbox1};
	
	FORM form  = {"Controls","footer\tTab\tTab",formload,formclose,test_timer,0,controls,3,12,0,10};
	memcpy(txtbuffer,"Textbox",8);
	
	form_exec(&form);
	
}

/*
struct MENU_ENTRY softMenuEntries[16];
struct MENU softMenu;
*/

const struct MENU_ENTRY testMenuEntries[] =
{
	{RTCtimer_edit,	0,	FUNCENTRY,	"Timer",		"Timer"	},
	{test_icon,		0, FUNCENTRY, "Icon Demo",	  "Draw icons"          },
	{test_irrec,		0, FUNCENTRY, "Receive IR",	  "Receive IR"			 },
	{test_RF,			0, FUNCENTRY, "RF test",		  "Test RF"				 },
	{RSSI_monitor,	 0, FUNCENTRY, "RSSI Monitor",	"RSSI Monitor"	},
	{RF_viewer,		 0, FUNCENTRY, "RF viewer",		"RF viewer"	},
	{test_circle,	  0, FUNCENTRY, "Circle",			"Draw circles"		  },
	{test_controls,	0, FUNCENTRY, "Controls",		 "Control elements"	 },
	{test_frame,		0, FUNCENTRY, "Frame",			 "Draw frame"			 },
	{test_frame2,	  0, FUNCENTRY, "Frame2",			"Draw frame2"			},
	{draw_fonts,		0, FUNCENTRY, "Fonts",			 "Draw fonts"			 },
	{test_logo,		 0, FUNCENTRY, "Logo",			  "Draw logo"			  },
	{fp1,				 0, FUNCENTRY, "Charset",		  "Draw charset"		  },
	{illusion,		  0, FUNCENTRY, "Illusion",		 "Optical illusion"	 },
	{test_bl,			0, FUNCENTRY, "Auto backlight", "Test auto backlight" },
	{bat_dbg,			0, FUNCENTRY, "Bat debug",		"Show/hide bat values"},
	{tkey,				0, FUNCENTRY, "Keytest",		  "Test keypad"			},
	{erase_flash1,	 0, FUNCENTRY, "Erase Flash1",	"Erase Flash1"	},
	{test_sid,		  0, FUNCENTRY, "Sound",			 "Play music"			 },
	{test_movearound, 0, FUNCENTRY, "Move",			  "Slowly move a cursor"},
//	{ 0,	 	(void*)&softMenu,	MENUENTRY,	"Soft-Test",	"access soft"	}
};

const struct MENU testMenu =
{
	NUM_OF_ELEMENTS (testMenuEntries), (MENU_ENTRY*)&testMenuEntries, "Test Menu" 
};

/*
void testmenu_init(void)
{
	testdata1 = 0;
	testdata2 = 0;
	
	for(i=0;i<16;i++)
	{
		softMenuEntries[i].exec = 0;
		softMenuEntries[i].data = 0;
		softMenuEntries[i].type = 0;
		softMenuEntries[i].name = 0;
		softMenuEntries[i].help = 0;
	}

	softMenuEntries[0].exec = softfunc1;
	softMenuEntries[0].data = 0;
	softMenuEntries[0].type = FUNCENTRY;
	softMenuEntries[0].name = "softfunc test";
	softMenuEntries[0].help = "run softfunc test";


	softMenu.name		= "Soft Menu";
	softMenu.num_entries	= 1;
	softMenu.entry		= softMenuEntries;

}
*/

