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
#include "testmenu.h"
#include "gamesmenu.h"
#include "settingsmenu.h"
#include "toolsmenu.h"
#include "lcd.h"
#include "fonty.h"
#include "keyboard.h"
#include "global.h"
#include "flash.h"
#ifndef DEBUGMODE
	#include "sounds.h"
	#include "sound3.h"
	#include "sound.h"
#endif
#include "version.h"

unsigned int testdata1, testdata2;


void show_info(void) {
	unsigned short flashID;
	unsigned short hwID;
	flashID = getFlashID();
    draw_hexC (0, 30, flashID, LCD_COLOR_B, DRAW_PUT);
	
	hwID = GetHWversion();
    draw_hexS (0, 38, hwID, LCD_COLOR_B, DRAW_PUT);
	
	lcd_fill(0);
	set_font(BOLDFONT);
    draw_string (0,0,"Info", LCD_COLOR_B, DRAW_PUT);
	set_font(SMALLFONT);
	draw_string(0,14,"Flash \n"
	                 "Manuf. ID\n"
	                 "Device ID\n"
	                 "\n"
	                 "Betty\n"
                     "HW Version\n", LCD_COLOR_B, DRAW_PUT);
    draw_hexC (65, 23, (flashID>>8), LCD_COLOR_B, DRAW_PUT);
    draw_hexC (65, 32, (flashID&0x00ff), LCD_COLOR_B, DRAW_PUT);
    draw_hexC (65, 59, (unsigned char)hwID, LCD_COLOR_B, DRAW_PUT);
    draw_string (0,77,"Boop", LCD_COLOR_B, DRAW_PUT);
#ifdef RELEASE
    draw_numU (65,86,SVNVERSION+1,0, LCD_COLOR_B, DRAW_PUT);
    draw_string (0,86,"SVN Release", LCD_COLOR_B, DRAW_PUT);
#else
    draw_numU (65,86,SVNVERSION,0, LCD_COLOR_B, DRAW_PUT);
    draw_string (0,86,"SVN Revision", LCD_COLOR_B, DRAW_PUT);
#endif
    draw_string (0,95,__DATE__, LCD_COLOR_B, DRAW_PUT);
    draw_string (0,104,__TIME__, LCD_COLOR_B, DRAW_PUT);
	
	set_font(SMALLFONT);

	waitKeyUpDown();
}   // show_info
	

void show_credits(void)
{
	lcd_fill(0);
	set_font(BOLDFONT);
    draw_string (0,0,"Credits", LCD_COLOR_B, DRAW_PUT);
	set_font(SMALLFONT);
    draw_string (0,14,"Netguy\tBasics\n"
                      "Telekatz\tBasics, IR/RF\n"
                      "Theborg\tFirst Steps, HW\n"
                      "Colibri\tFlash SW\n"
                      "Damaltor\tSupport\n"
                      "Pr0nbug\tFirst Steps\n"
                      "Asgart\tForum/Wiki\n"
                      "Spessi\tSurport, HW", LCD_COLOR_B, DRAW_PUT);

	draw_string(0,95,"Und alle Anderen\n"
                     "die vergessen wurden.", LCD_COLOR_B, DRAW_PUT);

	draw_string(0,120,"Thanks to Max Fleischer\n"
                      "for a great Cartoon!", LCD_COLOR_B, DRAW_PUT);
	set_font(BOLDFONT);
    draw_string (8,150,"www.bettyhacks.com", LCD_COLOR_B, DRAW_PUT);
	set_font(SMALLFONT);
#ifndef DEBUGMODE
	playSound((unsigned char*)sound1_data, sound1_len);
	waitSound();
	playSound((unsigned char*)sound2_data, sound2_len);
	waitSound();
#endif
	waitKeyUpDown();
}   // show_credits

const struct MENU_ENTRY mainMenuEntries[] = 
{
	{ 0, 			(void*)&gamesMenu,	MENUENTRY,	"Games",	"Play some games"	},
	{ 0,	 		(void*)&testMenu,	MENUENTRY,	"TestStuff",	"Various tests"		},
	{ enter_settings_menu,	0,			FUNCENTRY,	"Settings",	"Change settings"	},
	{ 0,	 		(void*)&toolsMenu,	MENUENTRY,	"Tools",	"Tools"			},
	{ show_credits,		0,			FUNCENTRY,	"Credits",	"Show Credits"		},
	{ show_info,		0,			FUNCENTRY,	"Info",	"Show Info"		}
};


const struct MENU mainMenu =
{
   NUM_OF_ELEMENTS (mainMenuEntries), (MENU_ENTRY*)&mainMenuEntries, "Main Menu"
};
