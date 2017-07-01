/*
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
#include "scart_icp.h"
#include "redirector.h"
#include "fs20.h"

const struct MENU_ENTRY toolsMenuEntries[] = 
{
	{ scart_icp,		0,	FUNCENTRY,	"Scart ICP",	"Program the SCART unit"	},		// tools/scarticp.c
#if FF_UART_REDIRECT
	{ uarto_redirector,	0,	FUNCENTRY,	"Redirector",	"Serial redirector"	},				// serial/redirector.c
#endif
	{ fs20_decoder,		0,	FUNCENTRY,	"FS20 decoder",	"FS20 decoder"	}					// cc1100/fs20.c
	
};

const struct MENU toolsMenu =
{
    NUM_OF_ELEMENTS (toolsMenuEntries), (MENU_ENTRY*)&toolsMenuEntries, "Tools"
};
