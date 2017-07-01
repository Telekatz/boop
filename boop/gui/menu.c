/*
	 menu.c - menuing system
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

#include "global.h"
#include "menu.h"
#include "lcd.h"
#include "fonty.h"
#include "keyboard.h"
#include "lpc2220.h"

//-----------------------------------------------------------------------------
#if FF_LARGE_MENU
#define MENU_FONT			 SMOOTHFONT
#define MENU_LINE_HEIGHT	(SMOOTHFONT_HEIGHT+2)
#else
#define MENU_FONT			 SMALLFONT
#define MENU_LINE_HEIGHT	SMALLFONT_HEIGHT+1
#endif

#define MENU_HEAD_LINE_HEIGHT	  (BOLDFONT_HEIGHT+2+2+2)
#define MENU_LINE_WIDTH			  (LCD_SIZE_X - 10)
#define MENU_BOTTOM_LINE_HEIGHT	(SMALLFONT_HEIGHT+5)

#define MENU_ARROW_SIZE  (min(MENU_LINE_HEIGHT-6, (LCD_SIZE_X-MENU_LINE_WIDTH-1)/2+1))

#define MENU_START_Y	MENU_HEAD_LINE_HEIGHT
#define MENU_LIST_LEN  ((LCD_SIZE_Y - MENU_HEAD_LINE_HEIGHT - MENU_BOTTOM_LINE_HEIGHT) / MENU_LINE_HEIGHT)

#define MENU_SCROLLBAR_HEIGHT (MENU_LINE_HEIGHT*MENU_LIST_LEN-10-10-3)	// [px]
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
int menu_execentry(struct MENU_ENTRY *entry)
{
	unsigned int *val;
	struct MENU *m;

	draw_block (0, 100, LCD_SIZE_X, 8, LCD_COLOR_B, DRAW_ERASE);
	switch(entry->type)
	{
		case INTENTRY:
			val = (unsigned int*)entry->data; 
			draw_block (0, 50, LCD_SIZE_X, 8, LCD_COLOR_B, DRAW_ERASE);
			draw_hexW  (0, 50, *val, LCD_COLOR_B, DRAW_PUT);
			break;
		case FUNCENTRY:
			entry->exec();
			return 1;
			break;
		case MENUENTRY:
			m = (MENU*)entry->data;
			if(m->num_entries != 0)
			{
				menu_exec((MENU*)entry->data);
				return 1;
			}
			break;
		default:
			break;
	}
	return 0;
}	// menu_execentry
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void menu_exec(struct MENU *menu)
{
	unsigned char total;	// entries in menu
	unsigned char csel;		// selected entry
		
	unsigned char moff;		// first entry displayed
	unsigned char moffold;	// only used to detect change
	
	unsigned char set;		// iterator for visible entries
	
	csel = 0;
	moff = 0;
	
again:
	lcd_fill(0);

	//-----------------------------------------
	// Menu head line
	set_font(BOLDFONT);
	draw_string (0,  0, (char*)menu->name, LCD_COLOR_B, DRAW_PUT);
	draw_block  (0, BOLDFONT_HEIGHT+2, LCD_SIZE_X, 2, LCD_COLOR_B, DRAW_PUT);
	// Menu bottom line
	draw_block  (0, LCD_SIZE_Y-SMALLFONT_HEIGHT-3, LCD_SIZE_X, 2, LCD_COLOR_B, DRAW_PUT);
	//-----------------------------------------

	total = menu->num_entries;
	moffold = total;
	
	while(KEY_Exit) {};
	while(!KEY_Exit)
	{
		// draw list: initially or when selection is on next 'page' and all entries 'move'
		if (moff!=moffold)
		{
			// menu list
			set_font (MENU_FONT);
			draw_block (0, MENU_START_Y, LCD_SIZE_X, MENU_LINE_HEIGHT*MENU_LIST_LEN, LCD_COLOR_B, DRAW_ERASE);
			for (set=moff; (set<total) && (set-moff<MENU_LIST_LEN);set++)
			{
				draw_string (1, MENU_START_Y+1 + ((set-moff) * MENU_LINE_HEIGHT),
								 (char*)menu->entry [set].name, LCD_COLOR_B, DRAW_PUT);
			}
			// scroll arrows
			if (moff)	// not the first page
			{
				draw_arrow (MENU_LINE_WIDTH+1, MENU_START_Y+3, 
								ARROW_UP, MENU_ARROW_SIZE, LCD_COLOR_B, DRAW_PUT);
			}
			if (total > moff+MENU_LIST_LEN) // not the last page
			{
				draw_arrow (MENU_LINE_WIDTH+1, MENU_LINE_HEIGHT*MENU_LIST_LEN+7, 
								ARROW_DOWN, MENU_ARROW_SIZE, LCD_COLOR_B, DRAW_PUT);
			}
			// scroll handle
			if (total > MENU_LIST_LEN)
			{
				draw_block (MENU_LINE_WIDTH+1, MENU_START_Y + 10 + (MENU_SCROLLBAR_HEIGHT/(total-1))*(csel),
											9, MENU_SCROLLBAR_HEIGHT/(total-1), 
											LCD_COLOR_DG, DRAW_PUT);
			}
			// select entry (invert)
			draw_block (0, MENU_START_Y + ((csel-moff) * MENU_LINE_HEIGHT),
							MENU_LINE_WIDTH, MENU_LINE_HEIGHT, LCD_COLOR_B, DRAW_XOR);
		}
		moffold = moff;
		
		//-----------------------------------------
		// Menu bottom line
		set_font (SMALLFONT);
		draw_block  ( 0, LCD_SIZE_Y-SMALLFONT_HEIGHT, LCD_SIZE_X, SMALLFONT_HEIGHT, LCD_COLOR_B, DRAW_ERASE);
		
		// entry number
		draw_char (0, LCD_SIZE_Y-SMALLFONT_HEIGHT, '0' + (csel+1)/10, LCD_COLOR_B, DRAW_PUT);
		draw_char (6, LCD_SIZE_Y-SMALLFONT_HEIGHT, '0' + (csel+1)%10, LCD_COLOR_B, DRAW_PUT);
		
		// entry help text
		draw_string (16, LCD_SIZE_Y-SMALLFONT_HEIGHT, (char*)menu->entry [csel].help, LCD_COLOR_B, DRAW_PUT);
		//-----------------------------------------

		// wait for key press
		waitKeyUpDown();
		
		// deselect current entry
		draw_block (0, MENU_START_Y + ((csel-moff) * MENU_LINE_HEIGHT),
						MENU_LINE_WIDTH, MENU_LINE_HEIGHT, LCD_COLOR_B, DRAW_XOR);
		
		// clear scroll handle
		if (total > MENU_LIST_LEN)
		draw_block (MENU_LINE_WIDTH+1, MENU_START_Y + 10 + (MENU_SCROLLBAR_HEIGHT/(total-1))*(csel),
									9, MENU_SCROLLBAR_HEIGHT/(total-1), 
									LCD_COLOR_DG, DRAW_XOR);
		
		if(KEY_OK)
		{
			if(menu_execentry((MENU_ENTRY*)&menu->entry[csel]))
				goto again;
		}
		if(KEY_Up)
		{
			csel--;
			if(csel<moff)
				moff--;
			if (csel >= total)
			{
				if (total > MENU_LIST_LEN)
					moff = total - MENU_LIST_LEN;
				csel = total-1;
			}
		}
		if(KEY_Down)
		{
			csel++;
			if (csel == MENU_LIST_LEN+moff)
				moff++;
			if (csel == total)
			{
				moff = 0;
				csel = 0;
			}
		}
		if(KEY_Left)
		{
			csel = 0;
			moff = 0;
		}
		if(KEY_Right)
		{
			csel = total -1;
			if (total > MENU_LIST_LEN)
				moff = total - MENU_LIST_LEN;
		}
		unsigned char num = getKeynum();
		if (num >= 16 && num <= 25)
		{
			csel = num - 16;
			if (csel >= total)
				csel = total-1;
			if(csel < moff)
				moff = csel;
			if (csel >= MENU_LIST_LEN-moff)
				moff = csel - MENU_LIST_LEN+1;
			if(menu_execentry((MENU_ENTRY*)&menu->entry[csel]))
				goto again;
		}
		
		// select new entry (invert)
		draw_block (0, MENU_START_Y + ((csel-moff) * MENU_LINE_HEIGHT),
						MENU_LINE_WIDTH, MENU_LINE_HEIGHT, LCD_COLOR_B, DRAW_XOR);
						
		// scroll handle moved
		if (total > MENU_LIST_LEN)
		draw_block (MENU_LINE_WIDTH+1, MENU_START_Y + 10 + (MENU_SCROLLBAR_HEIGHT/(total-1))*(csel),
							9, MENU_SCROLLBAR_HEIGHT/(total-1), 
							LCD_COLOR_DG, DRAW_PUT);
	}	// while (!KEY_Exit)
}	// menu_exec
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void init_menu(void)
{
}
//-----------------------------------------------------------------------------

