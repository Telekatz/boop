/*
    settingsmenue.c
    Copyright (C) 2008  <telekatz@gmx.de>

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
#include "settingsmenu.h"
#include "lcd.h"
#include "fonty.h"
#include "keyboard.h"
#include "rtc.h"
#include "ir_selector.h"
#include "backlight.h"
#include "bfs.h"
#include "rf.h"
#include "ir_capture.h"
#include "controls.h"
#include "global.h"

extern volatile unsigned char bl_val;
extern unsigned int bl_timeout;
extern unsigned char bl_max;
extern unsigned char bl_speed;
unsigned char lcd_contrast;

const struct MENU_ENTRY settingsMenuEntries[] =
{
	{ change_clock,		0,		FUNCENTRY,	"Clock",	"set time/date"	},
	{ RC_setting,		0,		FUNCENTRY,	"RC keys",	"assign RC keys"	},
	{ display_setting,	0,		FUNCENTRY,	"Display",	"backlight, contrast"	},
	{ RF_setting,		0,		FUNCENTRY,	"RF",		"ID, channel"	},
	{ RAW_learnmenue,	0,		FUNCENTRY,	"IR learn",	"Learn IR commands"	},
};

const struct MENU settingsMenu =
{
	5, (MENU_ENTRY*)&settingsMenuEntries, "Settings" 
};

const struct settings_ default_settings =
{
	0x02,	// version      change this when modifying the struct
	63,		// bl_max       hardware limit of LCD controller
	LCD_CONTRAST,
	2,		// bl_speed;
	1000	// bl_timeout
};

// change handlers

void brightness_onchange(FORM* form, CONTROL* control) {
	bl_max=((NUMBOX*)control)->value;
	setBacklight(bl_max);
}

void timeout_onchange(FORM* form, CONTROL* control) {
	bl_timeout=((NUMBOX*)control)->value * 200;
	setBacklight(bl_max);
}

void contrast_onchange(FORM* form, CONTROL* control) 
{
	lcd_contrast = ((NUMBOX*)control)->value;
	//~ lcd_set_contrast(0x3A);
	lcd_set_contrast(lcd_contrast);
}

void speed_onchange(FORM* form, CONTROL* control) 
{
	bl_speed=((NUMBOX*)control)->value;
	//~ setBLSpeed(bl_speed);
}

// the submenus

void display_setting(void) {
	
	const LABEL label1		= {BC_Labelnotab,	0,15,100,10,BOLDFONT,0,"Brightness",0};
	NUMBOX numbox1			= {BC_Numbox,		0,25, 13, 9,SMALLFONT,bl_val,0,63,brightness_onchange,NULL};
	const LABEL label2		= {BC_Labelnotab,	0,37,100,10,BOLDFONT,0,"Contrast",0};
	NUMBOX numbox2			= {BC_Numbox,		0,47, 19, 9,SMALLFONT,lcd_contrast,0,63,contrast_onchange,NULL};
    const LABEL label3		= {BC_Labelnotab,	0,59,100,10,BOLDFONT,0,"Backlight timeout",0};
	NUMBOX numbox3			= {BC_Numbox,		0,69, 19, 9,SMALLFONT,bl_timeout/200,1,200,timeout_onchange,NULL};
	const LABEL label4		= {BC_Labelnotab,	0,81,100,10,BOLDFONT,0,"Backlight fade time",0};
	NUMBOX numbox4			= {BC_Numbox,		0,91, 19, 9,SMALLFONT,bl_speed,1,6,speed_onchange,NULL};
	
	CONTROL* controls[8] = {	(CONTROL*)&label1,
								(CONTROL*)&label2,
								(CONTROL*)&label3,
								(CONTROL*)&label4,
								(CONTROL*)&numbox1,
								(CONTROL*)&numbox2,
								(CONTROL*)&numbox3,
								(CONTROL*)&numbox4};
	
	FORM form = {"Display settings"," ",0,0,0,0,controls,4,8,0,0};
	form_exec(&form);	// controls.c
	
	// grayscale staircase test pattern for calibration
	// form_exec->form_draw clears the screen
	//   either add this as a control or
	//   clear screen manually before form_exec
	//~ draw_block ( 0, 80, 20, 20, LCD_COLOR_B, DRAW_PUT);
	//~ draw_block (21, 80, 20, 20, LCD_COLOR_DG, DRAW_PUT);
	//~ draw_block (42, 80, 20, 20, LCD_COLOR_LG, DRAW_PUT);
}

void RF_setting(void) {
	struct RFsettings_ RFsettings;
	
	BFS_LoadFile(BFS_ID_RFsettings, sizeof(struct RFsettings_), (unsigned char*) &RFsettings);
	
	const LABEL label1		= {BC_Labelnotab,	0,15,100,10,BOLDFONT,0,"Device address",0};
	NUMBOX numbox1			= {BC_Numbox,		0,25, 19, 9,SMALLFONT,RFsettings.address,1,127,NULL,NULL};
	const LABEL label2		= {BC_Labelnotab,	0,37,100,10,BOLDFONT,0,"Channel",0};
	NUMBOX numbox2			= {BC_Numbox,		0,47, 19, 9,SMALLFONT,RFsettings.channel,0,255,NULL,NULL};
	
	CONTROL* controls[4] = {	(CONTROL*)&label1,
								(CONTROL*)&label2,
								(CONTROL*)&numbox1,
								(CONTROL*)&numbox2};
	
	FORM form  = {"RF settings"," ",0,0,0,0,controls,2,4,0,0};
	
	form_exec(&form);
	
	RFsettings.address = numbox1.value;
	RFsettings.channel = numbox2.value;
	
	BFS_SaveFile(BFS_ID_RFsettings, sizeof(struct RFsettings_), (unsigned char*) &RFsettings);
	load_RF_setting();
}

// learned remotes
void load_RC_setting(void) {
	
	if(BFS_LoadFile(BFS_ID_remotes, sizeof(struct irDeviceTable_), (unsigned char*) &irDevTab) != sizeof(struct irDeviceTable_)) {
		irDevTab = default_irDevTab;
		BFS_SaveFile(BFS_ID_remotes, sizeof(struct irDeviceTable_), (unsigned char*) &irDevTab);
		draw_string(0, 30, "default RC load", 3, DRAW_PUT);
	}
}

// load values stored in flash to ram
// called by main() for init
void load_setting(void) {
	struct settings_ settings;
	
	BFS_LoadFile(BFS_ID_settings, sizeof(struct settings_), (unsigned char*) &settings);
	
	// if neccessary, use hard-coded defaults
	if(settings.version != default_settings.version) 
	{
		settings = default_settings;
		BFS_SaveFile(BFS_ID_settings, sizeof(struct settings_), (unsigned char*) &settings);
		draw_string(0, 40, "default settings load", 3, DRAW_PUT);
	}
	// backlight        // TODO: see void initBacklight(void) in backlight.c)
	bl_timeout = settings.bl_timeout;
	bl_max = settings.bl_max;
	setBacklight(bl_max);
	bl_speed = settings.bl_speed;
	
	// LCD contrast
	lcd_contrast = settings.lcd_contrast ;
	lcd_set_contrast(lcd_contrast);
}

// called by the parent menu
void enter_settings_menu(void) {
	struct settings_ settings;
	
	// run menu
	menu_exec((struct MENU *)&settingsMenu);
	
	// save changes
	settings = default_settings;
	settings.bl_timeout = bl_timeout;
	settings.bl_max = bl_max;
	settings.lcd_contrast = lcd_contrast;
	settings.bl_speed = bl_speed;
	BFS_SaveFile(BFS_ID_settings, sizeof(struct settings_), (unsigned char*) &settings);
}
