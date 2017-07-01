/*
	controls.c - GUI controls
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

#include "rtc.h"
#include "controls.h"
#include "lcd.h"
#include "fonty.h"
#include "global.h"
#include "lpc2220.h"
#include "keyboard.h"
#include "backlight.h"
#include "timerfuncs.h"

#define Cursor_timeout	150

FORM* currentform = 0;

const char string_yes[] = "JA";
const char string_no[] = "NEIN";
const char string_ok[] = "OK";

const char TXTKey1[] = ".,-+_1@*;:'&";
const char TXTKey2[] = "abc2ABC";
const char TXTKey3[] = "def3DEF";
const char TXTKey4[] = "ghi4GHI";
const char TXTKey5[] = "jkl5JKL";
const char TXTKey6[] = "mno6MNO";
const char TXTKey7[] = "pqrs7PQRS";
const char TXTKey8[] = "tuv8TUV";
const char TXTKey9[] = "wxyz9WXYZ";
const char TXTKey0[] = " 0";

const char* const TXTKeys[10] = {TXTKey1,TXTKey2,TXTKey3,TXTKey4,TXTKey5,TXTKey6,TXTKey7,TXTKey8,TXTKey9,TXTKey0};

unsigned char msgbox (unsigned char y,unsigned short flags, char *st)
{
	unsigned short textwidth;
	unsigned short textheight;
	unsigned char width;
	unsigned char height;
	unsigned char left;
	unsigned char x;
	unsigned char sel;
	
	struct bt_
	{
		unsigned char cnt;
		unsigned char left[3];
		unsigned char top;
		unsigned char ret[3];
		char* text[3];
	  unsigned char textHeight;
	} button;
	
	textwidth = get_stringwidth(st);
	textheight = get_stringheight(st);
	
	if(textwidth > 120)
		width = 124;
	else if(textwidth < 54)
		width = 60;
	else
		width = (textwidth | 0x01) + 6;
		
	button.textHeight = get_stringheight (string_yes);
	if ((flags & 0x03) == BC_YesNo)
	{
		button.cnt = 2;
		button.left[0] = 38;
		button.ret[0]=BC_Yes;
		button.text[0] = (char*)string_yes;
		button.left[1] = 65;
		button.ret[1]=BC_No;
		button.text[1] = (char*)string_no;
		button.top = y + textheight +5;
	}
	else if ((flags & 0x03) == BC_nobuttons)
	{
		button.cnt = 0;
		button.top = 0;
	}
	else
	{
		button.cnt = 1;
		button.left[0] = 51;
		button.ret[0]=BC_OK;
		button.text[0] = (char*)string_ok;
		button.top = y + textheight +5;
		
	}
	
	if(button.cnt)
	  height = textheight + button.textHeight + 12;
	else
		height = textheight + 6;
	
	left = (128-width)>>1;
	
	draw_block (left,y,width,height,	LCD_COLOR_B, DRAW_ERASE);
	draw_rect  (left,y,width,height,1, LCD_COLOR_B, DRAW_PUT);
	draw_rect (left+1,y+1,width-2,height-2,1, LCD_COLOR_DG, DRAW_PUT);
	draw_string (left+3,y+3,st, LCD_COLOR_B, DRAW_PUT);
	
	for (x=0;x<button.cnt;x++)
	{
	  draw_rect (button.left[x],button.top, 25, button.textHeight+2, 1, LCD_COLOR_B, DRAW_PUT);
	  draw_string (button.left[x]+12- (get_stringwidth (button.text[x])>>1), button.top+2,
					button.text[x], LCD_COLOR_B, DRAW_PUT);
	}
	
	sel = 0;
	if ((flags & 0x0C) == BC_DefaultButton2)
	{
		if(button.cnt>1)
			sel=1;
	}
	else if ((flags & 0x0C) == BC_DefaultButton3)
	{
		if(button.cnt>2)
			sel=2;
	}
	
	setBacklight(BL_AUTO);	
	while(KEY_OK) {};
	while(!KEY_OK && button.cnt)
	{
	  draw_block (button.left[sel]+1, button.top+1, 23, button.textHeight, LCD_COLOR_B, DRAW_XOR);
		waitKeyUpDown();
	  draw_block (button.left[sel]+1, button.top+1, 23, button.textHeight, LCD_COLOR_B, DRAW_XOR);
		if(KEY_Left)
		{
			if(sel>0)
				sel--;
		}
		if(KEY_Right)
		{
			if(sel<button.cnt-1)
				sel++;
		}
	}
	
	if((flags & 0x03) != BC_nobuttons)
		draw_block(left,y,width,height,3,DRAW_ERASE);
	return button.ret[sel];
}	// msgbox
	
void label_draw (LABEL* label, unsigned char sel)
{
	unsigned char c;
	unsigned char m;
	
	set_font (label->font);
	if(label->type & BC_Disabled)
		c=2;
	else
		c=3;
	
	if(sel) {
		draw_block (label->left,label->top,label->width,label->height, LCD_COLOR_B, DRAW_PUT);
		m=DRAW_ERASE;
	}
	else
		m=DRAW_PUT;
	draw_string(label->left+1,label->top+1,label->caption, c, m);
	
}

void button_draw (BUTTON* button, unsigned char sel)
{
	unsigned char c;
	unsigned char cleft;
	unsigned char ctop;
	
	set_font (button->font);
	if(button->type & BC_Disabled)
		c=2;
	else
		c=3;
	
	draw_block(button->left,button->top,button->width,button->height,3,DRAW_ERASE);
	draw_rect(button->left,button->top,button->width,button->height,1,c,DRAW_PUT);
	draw_rect(button->left+1,button->top+1,button->width-2,button->height-2,1,c-1,DRAW_PUT);
		
	cleft = ((button->width - get_stringwidth(button->caption))>>1) +button->left;
	ctop = ((button->height - get_stringheight(button->caption)+1)>>1) +button->top;
		
	if (sel == 2)
	{
	  draw_block (button->left+2,button->top+2,button->width-4,button->height-4, LCD_COLOR_B, DRAW_PUT);
		draw_string(cleft,ctop,button->caption, c, DRAW_ERASE);
	}
	else if (sel)
	{
		draw_block(button->left+2,button->top+2,button->width-4,button->height-4,1,DRAW_PUT);
		draw_string(cleft,ctop,button->caption, c, DRAW_PUT);
	}
	else
		draw_string(cleft,ctop,button->caption, c, DRAW_PUT);
}	// button_draw

void numbox_draw(NUMBOX* numbox, unsigned char sel) {
	
	unsigned char c;
	unsigned char m;
	
	set_font (numbox->font);
	if(numbox->type & BC_Disabled)
		c=2;
	else
		c=3;
	
	if(sel) {
		draw_block (numbox->left,numbox->top,numbox->width,numbox->height, LCD_COLOR_B, DRAW_PUT);
		m=DRAW_ERASE;
	}
	else
		m=DRAW_PUT;
	draw_numU(numbox->left+1,numbox->top+1,numbox->value, 0, c, m);
	
	
}

void txtbox_draw(TXTBOX* txtbox, unsigned char sel) {
	
	unsigned char c;
	unsigned char m;
	
	set_font (txtbox->font);
	if(txtbox->type & BC_Disabled)
		c=2;
	else
		c=3;
		
	if(sel) {
		draw_block (txtbox->left,txtbox->top,txtbox->width,txtbox->height, LCD_COLOR_B, DRAW_PUT);
		m=DRAW_ERASE;
	}
	else
		m=DRAW_PUT;
	draw_string(txtbox->left+1,txtbox->top+1,txtbox->text, c, m);
	
}

void datebox_draw(DATEBOX* datebox, unsigned char sel) {
	
	unsigned char c;
	unsigned char m;
	unsigned char w1,w2;
	
	set_font (datebox->font);
	if(datebox->type & BC_Disabled)
		c=2;
	else
		c=3;
		
	if(sel) {
		draw_block (datebox->left,datebox->top,datebox->width,datebox->height, LCD_COLOR_B, DRAW_PUT);
		m=DRAW_ERASE;
	}
	else
		m=DRAW_PUT;
	
	w1 = get_stringwidth("0")+1;
	w2 = get_stringwidth(".")+1;
	
	draw_char(1+datebox->left 			,datebox->top+1,days[datebox->date.weekday * 2], c, m);
	draw_char(1+datebox->left+1*w1 	,datebox->top+1,days[(datebox->date.weekday * 2) + 1], c, m);
	draw_hexC(1+datebox->left+2*w1+1*w2,datebox->top+1,datebox->date.day, c, m);
	draw_char(1+datebox->left+4*w1+1*w2,datebox->top+1,'.', c, m);
	draw_hexC(1+datebox->left+4*w1+2*w2,datebox->top+1,datebox->date.month, c, m);
	draw_char(1+datebox->left+6*w1+2*w2,datebox->top+1,'.', c, m);
	draw_hexC(1+datebox->left+6*w1+3*w2,datebox->top+1,datebox->date.year, c, m);
	
}

void timebox_draw(TIMEBOX* timebox, unsigned char sel) {
	
	unsigned char c;
	unsigned char m;
	unsigned char w1,w2;
	
	set_font (timebox->font);
	if(timebox->type & BC_Disabled)
		c=2;
	else
		c=3;
	
	if(sel) {
		draw_block (timebox->left,timebox->top,timebox->width,timebox->height, LCD_COLOR_B, DRAW_PUT);
		m=DRAW_ERASE;
	}
	else
		m=DRAW_PUT;
	
	w1 = get_stringwidth("0")+1;
	w2 = get_stringwidth(":")+1;
	
	draw_hexC(1+timebox->left			,timebox->top+1,timebox->time.hour, c, m);
	draw_char(1+timebox->left+2*w1		,timebox->top+1,':', c, m);
	draw_hexC(1+timebox->left+2*w1+1*w2,timebox->top+1,timebox->time.minute, c, m);
	draw_char(1+timebox->left+4*w1+1*w2,timebox->top+1,':', c, m);
	draw_hexC(1+timebox->left+4*w1+2*w2,timebox->top+1,timebox->time.second, c, m);

}

void listbox_draw (LISTBOX* listbox, unsigned char sel)
{
	unsigned char c;
	unsigned char m;
	unsigned char arrow = (listbox->height - 6) / 2 + 1;
	
	set_font (listbox->font);
	if(listbox->type & BC_Disabled)
		c=2;
	else
		c=3;
	
	if (sel)
	{
		draw_block (listbox->left,listbox->top,listbox->width,listbox->height,
				  LCD_COLOR_B, DRAW_PUT);
		m=DRAW_ERASE;
	}
	else
		m=DRAW_PUT;
	
	if(listbox->listindex < listbox->listcount)
		draw_string(listbox->left+1,listbox->top+1,listbox->list[listbox->listindex], c, m);
	if (sel)
	{
		draw_block (listbox->left + listbox->width-((arrow+1)*2+1),listbox->top,
					(arrow+1)*2+1,listbox->height,
					LCD_COLOR_B, DRAW_PUT);
		draw_arrow (listbox->left + listbox->width-(arrow+1)*2, listbox->top + (listbox->height/2)-arrow,
					ARROW_LEFT,  arrow, LCD_COLOR_B, DRAW_ERASE);
		draw_arrow (listbox->left + listbox->width-arrow-1,	 listbox->top + (listbox->height/2)-arrow,
					ARROW_RIGHT, arrow, LCD_COLOR_B, DRAW_ERASE);
	}
}

void control_draw (CONTROL* control, unsigned char select)
{
	draw_block (control->left,control->top,control->width,control->height,LCD_COLOR_B,DRAW_ERASE);
	switch ((control->type) & 0x1F)
	{
		case BC_Label:
			label_draw((LABEL*)control,select);
			break;
		case BC_Button:
			button_draw((BUTTON*)control,select);
			break;
		case BC_Numbox:
			numbox_draw((NUMBOX*)control,select);
			break;
		case BC_Txtbox:
			txtbox_draw((TXTBOX*)control,select);
			break;
		case BC_Datebox:
			datebox_draw((DATEBOX*)control,select);
			break;
		case BC_Timebox:
			timebox_draw((TIMEBOX*)control,select);
			break;
		case BC_Listbox:
			listbox_draw((LISTBOX*)control,select);
			break;
	}
}	// control_draw
	
void form_draw (FORM* form)
{
	unsigned char x;
	
	lcd_fill(0);
	set_font(BOLDFONT);
	draw_string (0, 0, form->caption, LCD_COLOR_B, DRAW_PUT);
	draw_block (0,10,128,2, LCD_COLOR_B, DRAW_PUT);
	
	if (form->footer)
	{
		set_font(SMALLFONT);
		draw_block (0,149,128,2, LCD_COLOR_B, DRAW_PUT);
		draw_string (0,152, form->footer, LCD_COLOR_B, DRAW_PUT);
	}
	
	for (x=0;x<form->controlcount;x++)
	{
		control_draw(form->controls[x],(form->sel == x));
	}
}	// form_draw

unsigned char get_prev_control(FORM* form) {
	
	signed short x;
	unsigned char type;
	
	for(x=form->sel-1;x>-1;x--) {
		type = (form->controls[x])->type;
		if(!(type & 0xe0))
			return x;
	}
	for(x=form->controlcount-1;x>form->sel;x--) {
		type = (form->controls[x])->type;
		if(!(type & 0xe0))
			return x;
	}
	return form->sel;
}

unsigned char get_next_control(FORM* form) {
	
	unsigned char x;
	unsigned char type;
	
	for(x=form->sel+1;x<form->controlcount;x++) {
		type = (form->controls[x])->type;
		if(!(type & 0xe0))
			return x;
	}
	for(x=0;x<form->sel;x++) {
		type = (form->controls[x])->type;
		if(!(type & 0xe0))
			return x;
	}
	return form->sel;
}

extern void cpu_idle();

void waitFormKeyUp(void)
{
	while((keys[0] != 0) || (keys[1] != 0)) {
		if(currentform->flags & BC_Timer_expired) {
			currentform->flags &= ~BC_Timer_expired;
			if(currentform->timer)
				currentform->timer(currentform);
		}
		if(currentform->flags & BC_Form_redraw) {
			form_draw(currentform);
			currentform->flags &= ~BC_Form_redraw;
		}
	}
}

void waitFormKeyDown(void)
{
	while((keys[0] == 0) && (keys[1] == 0))	{
		if(currentform->flags & BC_Timer_expired) {
			currentform->flags &= ~BC_Timer_expired;
			if(currentform->timer)
				currentform->timer(currentform);
		}
		if(currentform->flags & BC_Form_redraw) {
			form_draw(currentform);
			currentform->flags &= ~BC_Form_redraw;
		}
		cpu_idle();
	};
	setBacklight(BL_AUTO);	// pwm value
}

#define waitFormKeyUpDown() { waitFormKeyUp(); waitFormKeyDown(); }
	
void numbox_entervalue(NUMBOX* numbox) {
	
	unsigned char digit[5];
	unsigned char len;
	unsigned char maxlen;
	unsigned char cpos;
	unsigned char width;
	unsigned char space;
	unsigned char k;
	unsigned short value;
	unsigned short x;
	unsigned char h;

	if((KEY_Left) || (KEY_Right)) {
		if((KEY_Left) && (numbox->value > numbox->min))
				numbox->value--;
		if((KEY_Right) && (numbox->value < numbox->max))
				numbox->value++;
		return;
	}
	set_font(numbox->font);
	width = get_stringwidth("0");
	space = get_stringwidth("00") - 2 * width;
	h=get_stringheight("0");
	
	cpos = 0;
	len = 0;
	x=numbox->max;
	for(maxlen = 1;x > 9;maxlen++)
		x /= 10;
	
	while(!(KEY_Exit || KEY_OK)) {
		k=getKeynum();
		if((k > 15) && (k < 26)) {
			digit[cpos] = k-15;
			if(digit[cpos] > 9)
				digit[cpos] = 0;
			if(len < maxlen)
				len++;
			cpos++;
			if(cpos == maxlen)
				cpos--;
		}
		if(KEY_Left) {
			if(len)
				len--;
			cpos = len;
		}
		draw_block(numbox->left,numbox->top,numbox->width,numbox->height,3,DRAW_ERASE);
		for(x=0; x<len; x++) 
			draw_char((numbox->left+1)+(width+space)*x ,numbox->top+1, digit[x] + 0x30,3, DRAW_PUT);
		draw_hline ((numbox->left+1)+ (width+space)*cpos,numbox->top+h,width, LCD_COLOR_B, DRAW_PUT);
		waitFormKeyUpDown();
	}
	if((KEY_OK) && len) {
		x=1;
		value = 0;
		for(k=len;k;k--) {
			value += x*digit[k-1];
			x *=10;
		}
		if(value > numbox->max)
			numbox->value = numbox->max;
		else if(value < numbox->min)
			numbox->value = numbox->min;
		else
			numbox->value = value;
	}
	waitFormKeyUp();
}

void txtbox_entervalue(TXTBOX* txtbox) {

	unsigned char k;
	unsigned char kpos;
	unsigned char kold;
	unsigned char cpos;
	unsigned char x;
	unsigned char y;
	unsigned char h;
	char buffer[txtbox->maxlen];
	
	memcpy(buffer,txtbox->text,txtbox->maxlen);
	
	if(KEY_Right) {
		for(cpos=0;buffer[cpos] !=0;cpos++);
	}
	else {
		cpos = 0;
		buffer[cpos] = 0;
	}
	if(cpos == txtbox->maxlen-1)
		cpos--;
	kpos = 0;
	
	h=get_stringheight("0");
	kold = 10;
	while(!(KEY_Exit || KEY_OK)) {
		k=getKeynum();
		if((k > 15) && (k < 26)) {
			k = k-16;
			if(k != kold) {
				kpos = 0;
				if((cpos < txtbox->maxlen-2) && (buffer[cpos]))
					cpos++;
			}
			else {
				kpos++;
				if(TXTKeys[k][kpos] == 0)
					kpos = 0;
			}
			buffer[cpos] = TXTKeys[k][kpos];
			buffer[cpos+1] = 0;
			kold = k;
		}
		if(KEY_Right) {
			if((cpos < txtbox->maxlen-2) && (buffer[cpos])) {
				cpos++;
				kold = 10;
			}
		}
		if(KEY_Left) {
			if((cpos > 0) && (!buffer[cpos]))
				cpos--;
			buffer[cpos] = 0;
		}
		draw_block(txtbox->left,txtbox->top,txtbox->width,txtbox->height,3,DRAW_ERASE);
		draw_string (txtbox->left+1,txtbox->top+1,buffer, LCD_COLOR_B, DRAW_PUT);
		x=get_stringwidth(buffer);
		y=get_stringwidth(&buffer[cpos]);
		if(!y){
			y=get_stringwidth("A");
			x+=y+1;
		}
		
		draw_block(txtbox->left+x-y,txtbox->top,y+2,h+2,3,DRAW_XOR);
		
		waitKeyUp();
		if((buffer[cpos]) && (cpos < txtbox->maxlen-2)) {
			unsigned short cnt;
			cnt = Cursor_timeout;
			while(((keys[0] == 0) && (keys[1] == 0)) && --cnt)	{
				wait5ms();
			}
			setBacklight(BL_AUTO);
			if(!cnt) {
				if((cpos < txtbox->maxlen-2) && (buffer[cpos]))
					cpos++;
				buffer[cpos] = 0;
				kold = 10;
			}
		}
		else
			waitFormKeyDown();
	
	}
	if(KEY_OK) {
		memcpy(txtbox->text,buffer,txtbox->maxlen);
	}
	waitFormKeyUp();
}

void datebox_entervalue(DATEBOX* datebox) {
	unsigned char cursor;
	signed char x;
	signed char v;
	char cpos[3];
	unsigned char w1, w2;
	struct date_ dateold;
	unsigned char exit = 0;
	
	if(KEY_Right)
		cursor = 2;
	else
		cursor = 0;
	
	dateold = datebox->date;
	
	w1 = get_stringwidth("0")+1;
	w2 = get_stringwidth(".")+1;
	
	cpos[0] = 2 * w1 + 1 * w2;
	cpos[1] = 4 * w1 + 2 * w2;
	cpos[2] = 6 * w1 + 3 * w2;
	
	draw_block(datebox->left,datebox->top,datebox->width,datebox->height,3,DRAW_ERASE);
	datebox_draw(datebox,0);
	draw_block(datebox->left+ cpos[cursor],datebox->top,2*w1+1,datebox->height,3,DRAW_XOR);
		
	while(!(KEY_Exit | KEY_OK)) {
		
		if(KEY_Left || KEY_Right) {
			cursor += 3;
			if (KEY_Left)
				cursor--;
			else
				cursor++;
			cursor %= 3;
		}
		
		x = getNumKeyValue();
		if (x >= 0) {
			v= x<<4;
			draw_block(datebox->left+cpos[cursor],datebox->top,2*w1+1,datebox->height,3,DRAW_ERASE);
			draw_char(datebox->left+1+cpos[cursor],datebox->top+1, x + 0x30,3, DRAW_PUT);
			draw_char(datebox->left+w1+1+cpos[cursor],datebox->top+1, '_',3, DRAW_PUT);
			waitKeyUpDown();
			x = getNumKeyValue();
			if (x >= 0) {
				v |= x;
				switch (cursor++) {
					case 0:
						datebox->date.day = checkBCD(v,31,1);
						break;
					case 1:
						datebox->date.month = checkBCD(v,12,1);
						break;
					case 2:
						datebox->date.year = checkBCD(v,99,0);
						exit++;
						break;
				}
				cursor %= 3;
			}
		}
		
		if(KEY_Down || KEY_Up) {	
			if (KEY_Up)
				x=1;
			else
				x=-1;
			switch (cursor) {
				case 0:
					calcBCD((unsigned char*)&datebox->date.day, x, 31, 1);
					break;
				case 1:
					calcBCD((unsigned char*)&datebox->date.month, x, 12, 1);
					break;
				case 2:
					calcBCD((unsigned char*)&datebox->date.year, x, 99, 0);
					break;
			}
		}
		calcweekday(&datebox->date);
		
		draw_block(datebox->left,datebox->top,datebox->width,datebox->height,3,DRAW_ERASE);
		datebox_draw(datebox,0);
		draw_block(datebox->left+ cpos[cursor],datebox->top,2*w1+1,datebox->height,3,DRAW_XOR);
		
		if(exit)
			break;
		
		waitFormKeyUpDown();
	}
	
	if (KEY_OK | exit) {
		switch (datebox->date.month) {
			case 0x02:
				x = (((datebox->date.year & 0xf0)>>4) * 10 + (datebox->date.year & 0x0f)) & 0x03; 
				if (x && (datebox->date.day > 0x28))
					datebox->date.day = 0x28;
				else if (!x && (datebox->date.day > 0x29))
					datebox->date.day = 0x29;
				break;
			case 0x04:
			case 0x06:
			case 0x09:
			case 0x11:
				if (datebox->date.day > 0x30)
					datebox->date.day = 0x30;
		}
		calcweekday(&datebox->date);
	}
	else
		datebox->date = dateold;
	
	waitFormKeyUp();
}

void timebox_entervalue(TIMEBOX* timebox) {
	unsigned char cursor;
	signed char x;
	signed char v;
	char cpos[3];
	unsigned char w1, w2;
	struct time_ timeold;
	unsigned char exit = 0;
	
	if(KEY_Right)
		cursor = 2;
	else
		cursor = 0;
	
	timeold = timebox->time;
	
	w1 = get_stringwidth("0")+1;
	w2 = get_stringwidth(":")+1;
	
	cpos[0] = 0;
	cpos[1] = 2 * w1 + 1 * w2;
	cpos[2] = 4 * w1 + 2 * w2;
	
	draw_block(timebox->left,timebox->top,timebox->width,timebox->height,3,DRAW_ERASE);
	timebox_draw(timebox,0);
	draw_block(timebox->left+ cpos[cursor],timebox->top,2*w1+1,timebox->height,3,DRAW_XOR);
		
	while(!(KEY_Exit | KEY_OK)) {
		
		if(KEY_Left || KEY_Right) {
			cursor += 3;
			if (KEY_Left)
				cursor--;
			else
				cursor++;
			cursor %= 3;
		}
		
		x = getNumKeyValue();
		if (x >= 0) {
			v= x<<4;
			draw_block(timebox->left+cpos[cursor],timebox->top,2*w1+1,timebox->height,3,DRAW_ERASE);
			draw_char(timebox->left+1+cpos[cursor],timebox->top+1, x + 0x30,3, DRAW_PUT);
			draw_char(timebox->left+w1+1+cpos[cursor],timebox->top+1, '_',3, DRAW_PUT);
			waitKeyUpDown();
			x = getNumKeyValue();
			if (x >= 0) {
				v |= x;
				switch (cursor++) {
					case 0:
						timebox->time.hour = checkBCD(v,23,0);
						break;
					case 1:
						timebox->time.minute = checkBCD(v,59,0);
						break;
					case 2:
						timebox->time.second = checkBCD(v,59,0);
						exit++;
						break;
				}
				cursor %= 3;
			}
		}
		
		if(KEY_Down || KEY_Up) {	
			if (KEY_Up)
				x=1;
			else
				x=-1;
			switch (cursor) {
				case 0:
					calcBCD((unsigned char*)&timebox->time.hour, x, 23, 0);
					break;
				case 1:
					calcBCD((unsigned char*)&timebox->time.minute, x, 59, 0);
					break;
				case 2:
					calcBCD((unsigned char*)&timebox->time.second, x, 59, 0);
					break;
			}
		}
		
		draw_block(timebox->left,timebox->top,timebox->width,timebox->height,3,DRAW_ERASE);
		timebox_draw(timebox,0);
		draw_block(timebox->left+ cpos[cursor],timebox->top,2*w1+1,timebox->height,3,DRAW_XOR);
		
			
		if(exit)
			break;
		
		waitFormKeyUpDown();
	}
	
	if (KEY_Exit)
		timebox->time = timeold;
	
	waitFormKeyUp();
}

void listbox_changevalue (LISTBOX* listbox)
{
	if((KEY_Left) && listbox->listindex)
		listbox->listindex--;
	if((KEY_Right) && listbox->listindex < listbox->listcount - 1)
		listbox->listindex++;
}

void control_onkey(FORM* form, CONTROL* control) {
	unsigned short x;
	
	if (KEY_OK)
	{
		switch ((control->type) & 0x1F) {
			case BC_Label:
			case BC_Txtbox:
			case BC_Numbox:	
			case BC_Datebox:
			case BC_Timebox:
			case BC_Listbox:
				if (control->ONclick)
					control->ONclick(form,control);
				waitFormKeyUp();
				break;
			case BC_Button:
				x=form->sel;
				button_draw((BUTTON*)control,2);
				if (control->ONclick)
					control->ONclick(form,control);
				waitFormKeyUp();
				if(x==form->sel)
					x=1;
				else
					x=0;
				button_draw((BUTTON*)control,x);
				break;
		}
		return;
	}
	switch ((control->type) & 0x1F) {
		case BC_Label:
		case BC_Button:
			break;
		case BC_Numbox:
			{
				NUMBOX* numbox = (NUMBOX*)control;				
				x=numbox->value;
				if(((getKeynum() > 15) && (getKeynum() < 26)) ||  (KEY_Left) || (KEY_Right))
					numbox_entervalue(numbox);
				if((x!=numbox->value) && (numbox->ONchange))
					numbox->ONchange(form,control);
			}
			break;
		case BC_Txtbox:
			{
				TXTBOX* txtbox = (TXTBOX*)control;	
				crc oldtext = CRCs(txtbox->text);
				if(((getKeynum() > 15) && (getKeynum() < 26)) | KEY_Right)
					txtbox_entervalue(txtbox);
				if((oldtext != CRCs(txtbox->text)) && (txtbox->ONchange))
					txtbox->ONchange(form,control);
			}
			break;
		case BC_Datebox:
			{
				DATEBOX* datebox = (DATEBOX*)control;
				struct date_ olddate = datebox->date; 
				if(((getKeynum() > 15) && (getKeynum() < 26)) | KEY_Right)
					datebox_entervalue(datebox);
				if(memcmp(&olddate,&datebox->date,4) && (datebox->ONchange))
					datebox->ONchange(form,control);	
			}
			break;
		case BC_Timebox:
			{
				TIMEBOX* timebox = (TIMEBOX*)control;
				struct time_ oldtime = timebox->time; 
				if(((getKeynum() > 15) && (getKeynum() < 26)) | KEY_Right)
					timebox_entervalue(timebox);
				if(memcmp(&oldtime,&timebox->time,4) && (timebox->ONchange))
					timebox->ONchange(form,control);
			}
			break;
		case BC_Listbox:
			{
				LISTBOX* listbox = (LISTBOX*)control;
				unsigned char oldvalue = listbox->listindex;
				if(KEY_Left | KEY_Right)
					listbox_changevalue((LISTBOX*) control);
				if((oldvalue!=listbox->listindex) && (listbox->ONchange))
					listbox->ONchange(form,control);
			}
			break;
	}
	control_draw(control,1);
}

void form_ontimer(unsigned int cb) {
	currentform->flags |= BC_Timer_expired;
	//currentform->timer(currentform);
}



// displays a form and passes input to form element handlers
void form_exec(FORM* form) {
	
	unsigned char x;
	FORM* oldform;
	unsigned int form_cb = 0xff;
	
	oldform = currentform;
	currentform = form;
	
	if(form->load)
		form->load(form);
		
	if(form->controls[form->sel]->type & BC_NoTabStop) {
		x = get_next_control(form);
		form->sel = x;
	}

	form_draw(form);
	
	if(form->timer) {
		form_cb = addTimerCB(form_ontimer, form->intervall);
		startCB(form_cb);
		sysInfo |= SYS_NOPDOWN;
	}
	
	while(KEY_Exit) {};
	while((!KEY_Exit) && (!(form->flags & BC_Form_exit)))
	{
		waitFormKeyUpDown();
		
		if(KEY_Up | KEY_Down) {
			if(KEY_Up)
				x = get_prev_control(form);
			else
				x = get_next_control(form);
			control_draw(form->controls[form->sel],0);
			form->sel = x;
			control_draw(form->controls[form->sel],1);
		}
		else
			control_onkey(form,form->controls[form->sel]);
	}
	
	if(form_cb != 0xff) {
		sysInfo &= ~SYS_NOPDOWN;
		stopCB(form_cb);
		removeTimerCB(form_cb);
	}
	
	if(form->close)
		form->close(form);
	
	currentform = oldform;
	
	while(KEY_Exit);
}
