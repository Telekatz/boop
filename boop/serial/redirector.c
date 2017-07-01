/*
    redirector.h - serial port redirector
    Copyright (C) 2009  Telekatz <telekatz@gmx.de>

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
#include "redirector.h"
#include "serial.h"
#include "buffer.h"
#include "global.h"
#include "controls.h"
#include "fonty.h"
#include "rf.h"
#include "timerfuncs.h"

#define redirectorbuffersize	200

extern unsigned int is_drawing;

struct redirector_ {
	struct cBuffer_ incbuffer;
	struct cBuffer_ outcbuffer;
	struct RFendpoint_* dataout_ep;
	struct RFendpoint_* datain_ep;
	struct RFendpoint_* cmdout_ep;
	struct RFendpoint_* cmdin_ep;
	unsigned long lastlen;
	unsigned char timeoutcnt;
	unsigned char inbuffer[redirectorbuffersize];
	unsigned char outbuffer[redirectorbuffersize];
	unsigned char status;
};

const unsigned int listbrg[8] = {	1200,2400,4800,9600,14400,19200,28800,38400};

void redirector_start(FORM* form, struct redirector_* redirectorvars) {
	
	redirectorvars->dataout_ep->dest = ((NUMBOX*)(form->controls[3]))->value;
	redirectorvars->lastlen = 0;
	redirectorvars->timeoutcnt = 0;
	
	cBufferFlush(&redirectorvars->incbuffer);
	cBufferFlush(&redirectorvars->outcbuffer);
	uart0_incbuffer = &redirectorvars->incbuffer;
	uart0_outcbuffer = &redirectorvars->outcbuffer;
	
	redirectorvars->dataout_ep->flags |= EPenabled;
	redirectorvars->datain_ep->flags |=  EPenabled;
	
	U0FCR |= (1<<7);		// trigger level 1
	serial_setbrg (listbrg[((LISTBOX*)(form->controls[2]))->listindex]);
	sysInfo |= SYS_NOPDOWN;
	
	form->controls[2]->type |= BC_NoTabStop;
	form->controls[3]->type |= BC_NoTabStop;
	form->controls[5]->type |= BC_Disabled;
	form->controls[6]->type &= ~BC_Disabled;
	form->sel = 6;
	form->flags |= BC_Form_redraw;
	
}

void redirector_start_click(FORM* form, CONTROL* control) {
	
	volatile unsigned long timeout;
	
	struct redirector_* redirectorvars;
	redirectorvars = (struct redirector_*)form->tag;
	
	RF_changestate(RFrx);
	
	redirectorvars->cmdout_ep->dest = ((NUMBOX*)(form->controls[3]))->value;
	redirectorvars->cmdout_ep->data[0] = redir_SET;
	redirectorvars->cmdout_ep->data[1] = ((LISTBOX*)(form->controls[2]))->listindex;
	redirectorvars->cmdout_ep->bufferlen = 2;
	redirectorvars->cmdout_ep->flags |= EPnewdata | EPsendwor;
	redirectorvars->cmdin_ep->flags &= ~EPnewdata;
	
	RF_changestate(RFtx);
	
	addTimeout(&timeout, 200);
	while(timeout && !(redirectorvars->cmdin_ep->flags & EPnewdata));
	removeTimeout(&timeout);
	
	if(!timeout) {
		msgbox(30,BC_OKOnly | BC_DefaultButton1,"Fehler:\nZiel antwortet nicht");
		form_draw(form);
		RF_changestate(RFwor);
		return;
	}
	
	redirectorvars->cmdout_ep->data[0] = redir_START;
	redirectorvars->cmdout_ep->bufferlen = 1;
	redirectorvars->cmdout_ep->flags |= EPnewdata;
	redirectorvars->cmdin_ep->flags &= ~EPnewdata;
	
	RF_changestate(RFtx);
	
	addTimeout(&timeout, 200);
	while(timeout && !(redirectorvars->cmdin_ep->flags & EPnewdata));
	removeTimeout(&timeout);
	
	if(!timeout) {
		msgbox(30,BC_OKOnly | BC_DefaultButton1,"Fehler:\nZiel antwortet nicht");
		form->flags |= BC_Form_redraw;
		RF_changestate(RFwor);
		return;
	}
	
	redirector_start(form, redirectorvars);	
	
}


void redirector_stop_click(FORM* form, CONTROL* control) {
	
	struct redirector_* redirectorvars;
	redirectorvars = (struct redirector_*)form->tag;
	
	redirectorvars->cmdout_ep->dest = ((NUMBOX*)(form->controls[3]))->value;
	redirectorvars->cmdout_ep->data[0] = redir_STOP;
	redirectorvars->cmdout_ep->bufferlen = 1;
	redirectorvars->cmdout_ep->flags |= EPnewdata;
	
	RF_changestate(RFtx);
	
	redirectorvars->dataout_ep->flags &= ~EPenabled;
	redirectorvars->datain_ep->flags &=  ~EPenabled;
	
	U0FCR &= ~((1<<6) | (1<<7));		// trigger level 0
	uart0_incbuffer = 0;
	uart0_outcbuffer = 0;
	serial_setbrg (DEFAULTBRG);
	sysInfo &= ~SYS_NOPDOWN;
	RF_changestate(RFwor);
	
	form->controls[2]->type &= ~BC_NoTabStop;
	form->controls[3]->type &= ~BC_NoTabStop;
	form->controls[5]->type &= ~BC_Disabled;
	form->controls[6]->type |= BC_Disabled;
	form->sel = 5;
	form->flags |= BC_Form_redraw;

}

#define sendcount	59
void redirector_timer(FORM* form) {
	struct redirector_* redirectorvars;
	redirectorvars = (struct redirector_*)form->tag;
	
	//((NUMBOX*)(form->controls[4]))->value = redirectorvars->outcbuffer.len;
	//control_draw(form->controls[4],0);
	
	if(redirectorvars->incbuffer.len == redirectorvars->lastlen) {
		if(redirectorvars->timeoutcnt)
			redirectorvars->timeoutcnt--;
	}
	else {
		redirectorvars->timeoutcnt = 2;
	}
	redirectorvars->lastlen = redirectorvars->incbuffer.len;
	
	
	if(redirectorvars->dataout_ep && redirectorvars->incbuffer.len && !(redirectorvars->dataout_ep->flags & EPnewdata) && 
	((redirectorvars->incbuffer.len >= sendcount) | !(redirectorvars->timeoutcnt))) {
		
		if(redirectorvars->incbuffer.len >= sendcount)
			redirectorvars->dataout_ep->bufferlen = sendcount;
		else
			redirectorvars->dataout_ep->bufferlen = redirectorvars->incbuffer.len;
		
		redirectorvars->dataout_ep->flags |= EPnewdata;
		RF_changestate(RFtx);
	}
	
	if(redirectorvars->outcbuffer.len) {
		if(!(U0IER & IER_THRE)) {
			U0IER |= IER_THRE;
			U0THR = cBufferGet(uart0_outcbuffer);
			(((NUMBOX*)(form->controls[4]))->value)++;
			control_draw(form->controls[4],0);
		}
	}
	
	if(redirectorvars->cmdin_ep->flags & EPnewdata) {
		unsigned char ACK = 0;
		switch (redirectorvars->cmdin_ep->data[0]) {
			case redir_ACK:
				break;
			case redir_SET:
				if (redirectorvars->cmdin_ep->bufferlen > 1) {
					if(redirectorvars->cmdin_ep->data[1] <= baud_38400) {
						((LISTBOX*)(form->controls[2]))->listindex = redirectorvars->cmdin_ep->data[1];
						form->flags |= BC_Form_redraw;
					}
				ACK = 1;
				}
				break;
			case redir_START:
				((NUMBOX*)(form->controls[3]))->value = redirectorvars->cmdin_ep->dest;
				form->flags |= BC_Form_redraw;
				/*redir_dest = RXframe->srcAddr;
				setbaud(redir_baud);
				redirector = 1;
				ACK = 1;*/
				break;
			case redir_STOP:
				/*redirector = 0;
				setbaud(baud_default);
				ACK = 1;*/
				break;
			case redir_PING:
				ACK = 1;
				break;
		}
		
		redirectorvars->cmdin_ep->flags &= ~EPnewdata;

		if(ACK) {
			redirectorvars->cmdout_ep->data[0] = redir_ACK;
			redirectorvars->cmdout_ep->dest = redirectorvars->cmdin_ep->dest;
			redirectorvars->cmdout_ep->bufferlen = 1;
			redirectorvars->cmdout_ep->flags |= EPnewdata;
			RF_changestate(RFtx);
		}
	}
}


void redirector_test(FORM* form, CONTROL* control) {

	if(uart0_incbuffer) {
		cBufferAdd(uart0_incbuffer,'a');
		cBufferAdd(uart0_incbuffer,'b');
		cBufferAdd(uart0_incbuffer,'c');
	}
}


// menu (Tool>Redirector)
void uarto_redirector(void) {
	
	struct redirector_ redirectorvars;
	
	cBufferInit(&redirectorvars.incbuffer, redirectorvars.inbuffer, redirectorbuffersize);
	cBufferInit(&redirectorvars.outcbuffer, redirectorvars.outbuffer, redirectorbuffersize);
		
	const char* listbox1items[8] = {"1200","2400","4800","9600","14400","19200","28800","38400"};
	
	const LABEL label1		= {BC_Labelnotab,0,20,70,9,BOLDFONT,0,"Baud",0};
	const LABEL label2		= {BC_Labelnotab,0,32,70,9,BOLDFONT,0,"Zieladresse",0};
	LISTBOX listbox1		= {BC_Listbox,78,20,40,10,SMALLFONT,5,8,(char**)listbox1items,NULL,NULL};
	NUMBOX numbox1			= {BC_Numbox,78,32,20,9,SMALLFONT,1,0,255,NULL,NULL};
	NUMBOX numbox2			= {BC_Numbox | BC_NoTabStop,0,100,20,9,SMALLFONT,redirectorvars.outcbuffer.len,0,redirectorbuffersize,NULL,NULL};
	BUTTON button1			= {BC_Button,10,120,48,15,SMALLFONT,0,"Start",redirector_start_click};
	BUTTON button2			= {BC_Button | BC_Disabled,70,120,48,15,SMALLFONT,0,"Stop",redirector_stop_click};
	const BUTTON button3	= {BC_Button, 0,50,48,15,SMALLFONT,0,"test",redirector_test};
	
	CONTROL* controls[8] = {	(CONTROL*)&label1,
								(CONTROL*)&label2,
								(CONTROL*)&listbox1,
								(CONTROL*)&numbox1,
								(CONTROL*)&numbox2,
								(CONTROL*)&button1,
								(CONTROL*)&button2,
								(CONTROL*)&button3};
	
	FORM form  = {"Com redirector","",NULL,NULL,redirector_timer,(unsigned long)&redirectorvars,controls,2,8,0,2};
	
	redirectorvars.dataout_ep = openEP(&redirectorvars.incbuffer, 0, packet_redirDAT);
	redirectorvars.datain_ep = openEP(&redirectorvars.outcbuffer, 0, packet_redirDAT);
	redirectorvars.cmdout_ep = openEP(0, 0, packet_redirCMD);
	redirectorvars.cmdin_ep = openEP(0, 0, packet_redirCMD);
	
	if(redirectorvars.dataout_ep && redirectorvars.datain_ep && redirectorvars.cmdout_ep && redirectorvars.cmdin_ep) {
		redirectorvars.dataout_ep->flags |= EPoutput | EPtypecbuffer;
		redirectorvars.datain_ep->flags |= EPinput | EPtypecbuffer;
		redirectorvars.cmdout_ep->flags |= EPoutput | EPtypedata | EPenabled;
		redirectorvars.cmdin_ep->flags |= EPinput | EPtypedata | EPenabled;
		form_exec(&form);
	}
	else {
		msgbox(30,BC_OKOnly | BC_DefaultButton1,"Fehler:\nKein freier RF Endpoint\nverfuegbar.\n");
	}
	
	redirector_stop_click(&form,form.controls[6]); 
	
	closeEP(redirectorvars.datain_ep);
	closeEP(redirectorvars.dataout_ep);
	closeEP(redirectorvars.cmdin_ep);
	closeEP(redirectorvars.cmdout_ep);
	
}
