/**********************************************************************
@file		 drawIcon.c

@brief		Draw Black/White and Gray scale icons with or without
			Alpha channel (Transparency)

-----------------------------------------------------------------------
@author  2010 Roman Steiger
			LeoTheLoewe [at] gmx [dot] de
-----------------------------------------------------------------------
Following compile flags are usable:


-----------------------------------------------------------------------
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
-----------------------------------------------------------------------
@History

1.00 20012010 RSt  Creation

@end
***********************************************************************/
#include "global.h"
#include "lcd.h"
#include "icon.h"


//-----------------------------------------------------------------------------
// Internally used functions

static void _drawGray			(unsigned char x, unsigned char y,
								const unsigned char const *pData,
								unsigned char width, unsigned char height, iconInfo_t iconInfo,
								unsigned char m);

static void _drawBlackWhite		(unsigned char x, unsigned char y,
								const unsigned char const *pData,
								unsigned char width, unsigned char height, iconInfo_t iconInfo,
								unsigned char c, unsigned char m);
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void drawIcon (unsigned char x, unsigned char y,
				const icon_t const *pIcon, unsigned char c, unsigned char m)
{
	if (!pIcon)
		return;

	drawIconExt (x, y, pIcon->data,
		pIcon->width, pIcon->height, pIcon->info,
		c, m);
}	// drawIcon
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void drawIconExt (unsigned char x, unsigned char y,
				const unsigned char const *pData,
				unsigned char width, unsigned char height, iconInfo_t iconInfo,
				unsigned char c, unsigned char m)
{
	switch (iconInfo)
	{
		case ICON_BLACK_WHITE_TRANSPARENT:	/// 2 bpp
		case ICON_BLACK_WHITE:	/// 1 bpp
			_drawBlackWhite (x, y, pData,
				width, height, iconInfo,
				c, m);
		break;

		case ICON_GRAY_TRANSPARENT:	/// 3 bpp
		case ICON_GRAY:	/// 2 bpp
			_drawGray (x, y, pData,
				width, height, iconInfo,
				m);
		break;

		default:
			if (x + width > LCD_SIZE_X)
				width = LCD_SIZE_X - x;

			if (y + height > LCD_SIZE_Y)
				height = LCD_SIZE_Y - y;

			draw_rect (x, y,
						  width, height,
						  1, LCD_COLOR_B, DRAW_XOR);

		break;
	}	// switch (iconInfo)
}	// drawIconExt
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
static void _drawGray 	(unsigned char x, unsigned char y,
						 const unsigned char const *pData,
						 unsigned char width, unsigned char height, iconInfo_t iconInfo,
						 unsigned char m)
{
	unsigned char alphaBuf [LCD_SIZE_X];
	unsigned short ix;
	unsigned short iy;
	unsigned char  iconWidth = width;
	unsigned char  bpp;
	unsigned char* alphaBuf_p;

	if(iconInfo==ICON_GRAY_TRANSPARENT)
	{
		alphaBuf_p = alphaBuf;
		bpp=3;
	}
	else
	{
		alphaBuf_p = NULL;
		bpp=2;
	}


	if (x + width > LCD_SIZE_X)
		width = LCD_SIZE_X - x;

	if (y + height > LCD_SIZE_Y)
		height = LCD_SIZE_Y - y;

	unsigned short idx = (width * bpp);	// 3 byte per pixel
	unsigned char s = y & 0x07;
	is_drawing++;
	if (s)
	{
		for (ix = 0; ix < width; ix++)
		{
			alphaBuf	[ix] = 0;
			drawbuf [1][ix] = 0;
			drawbuf [0][ix] = 0;
		}	// for (ix = 0; ix < width; ix++)

		for (iy = 0; iy < height; iy += 8)
		{
			// Need to process two icon lines for one drawbuf line
			if (iy)
			{
				for (ix = 0; ix < width; ix++)
				{
					idx -= bpp;
					alphaBuf	[ix] = pData [idx+2] >> (8-s);
					drawbuf [1][ix] = pData [idx+1] >> (8-s);
					drawbuf [0][ix] = pData [idx] >> (8-s);
				}	// for (ix = 0; ix < width; ix++)

				// go back to end of this page
				idx += (width * bpp);	// 3bpp
				// goto next page
				idx += (iconWidth * bpp);	// 3bpp
			}
			for (ix = 0; ix < width; ix++)
			{
		  	idx -= bpp;
				alphaBuf	[ix] |= pData [idx+2] << s;
				drawbuf [1][ix] |= pData [idx+1] << s;
				drawbuf [0][ix] |= pData [idx] << s;
			}
			do_rcuAlpha (x, y+iy, width, m, alphaBuf_p);

			// go back to end of this page
			idx += (width * bpp);	// 3bpp
		}	// for (iy = 0; iy < height; iy += 8)

		/* Pixel in the last page: height & 0x07
			(i.e. 13 hight => 5 pixel in last page) */
		if (((height & 0x07) == 0) ||
			 ((height & 0x07) > (8 - s)))
		{
			for (ix = 0; ix < width; ix++)
			{
		  	idx -= bpp;
				alphaBuf	[ix] = pData [idx+2] >> (8-s);
				drawbuf [1][ix] = pData [idx+1] >> (8-s);
				drawbuf [0][ix] = pData [idx] >> (8-s);
			}	// for (ix = 0; ix < width; ix++)
			do_rcuAlpha (x, y+iy, width, m, alphaBuf_p);

		}
	}
	else
	{
		for (iy = 0; iy < height; iy += 8)
		{
			for (ix = 0; ix < width; ix++)
			{
				idx -= bpp;
				alphaBuf	[ix] = pData [idx+2];
				drawbuf [1][ix] = pData [idx+1];
				drawbuf [0][ix] = pData [idx];
			}
			do_rcuAlpha (x, y+iy, width, m, alphaBuf_p);

			// go back to end of this page
			idx += (width * bpp);	// 3bpp
			// goto next page
			idx += (iconWidth * bpp);	// 3bpp
		}	// for (iy = 0; iy < height; iy += 8)
	}	// else if (s)
	is_drawing--;
}	// _drawGrayAlpha
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
static void _drawBlackWhite	(unsigned char x, unsigned char y,
							const unsigned char const *pData,
							unsigned char width, unsigned char height, iconInfo_t iconInfo,
							unsigned char c, unsigned char m)
{
	unsigned char alphaBuf [LCD_SIZE_X];
	unsigned short ix;
	unsigned short iy;
	unsigned char  iconWidth = width;
	unsigned char  bpp;
	unsigned char* alphaBuf_p;

	if(iconInfo==ICON_BLACK_WHITE_TRANSPARENT)
	{
		alphaBuf_p = alphaBuf;
		bpp=2;
	}
	else
	{
		alphaBuf_p = NULL;
		bpp=1;
	}

	if (x + width > LCD_SIZE_X)
		width = LCD_SIZE_X - x;

	if (y + height > LCD_SIZE_Y)
		height = LCD_SIZE_Y - y;

	unsigned short idx = width * bpp;	// 1 byte per pixel
	unsigned char s = y & 0x07;
	is_drawing++;
	if (s)
	{
		for (ix = 0; ix < width; ix++)
		{
			alphaBuf	[ix] = 0;
			drawbuf [1][ix] = 0;
			drawbuf [0][ix] = 0;
		}	// for (ix = 0; ix < width; ix++)

		for (iy = 0; iy < height; iy += 8)
		{
			// Need to process two icon lines for one drawbuf line
			if (iy)
			{
				for (ix = 0; ix < width; ix++)
				{
					idx -= bpp;
					alphaBuf		[ix] = pData [idx+1] >> (8-s);
					if (c & 0x02)
						drawbuf [0][ix] = pData [idx] >> (8-s);
					else
						drawbuf [0][ix] = 0;

					if (c & 0x01)
						drawbuf [1][ix] = pData [idx] >> (8-s);
					else
						drawbuf [1][ix] = 0;
				}	// for (ix = 0; ix < width; ix++)
				do_rcuAlpha (x, y+iy, width, m, alphaBuf_p);

				// go back to end of this page
				idx += width * bpp;	// 1bpp
				// goto next page
				idx += iconWidth * bpp;	// 1bpp
			}
			for (ix = 0; ix < width; ix++)
			{
				idx -=bpp;
				alphaBuf		[ix] = pData [idx+1] << s;
				if (c & 0x02)
					drawbuf [0][ix] |= pData [idx] << s;

				if (c & 0x01)
					drawbuf [1][ix] |= pData [idx] << s;
			}
			do_rcuAlpha (x, y+iy, width, m, alphaBuf_p);

			// go back to end of this page
			idx += width * bpp;	// 1bpp
		}	// for (iy = 0; iy < height; iy += 8)

		/* Pixel in the last page: height & 0x07
			(i.e. 13 hight => 5 pixel in last page) */
		if (((height & 0x07) == 0) ||
			((height & 0x07) > (8 - s)))
		{
			for (ix = 0; ix < width; ix++)
			{
				idx -= bpp;
				alphaBuf		[ix] = pData [idx+1] >> (8-s);
				if (c & 0x02)
					drawbuf [0][ix] = pData [idx] >> (8-s);
				else
					drawbuf [0][ix] = 0;

				if (c & 0x01)
					drawbuf [1][ix] = pData [idx] >> (8-s);
				else
					drawbuf [1][ix] = 0;
			}	// for (ix = 0; ix < width; ix++)

			do_rcuAlpha (x, y+iy, width, m, alphaBuf_p);
		}
	}
	else
	{
		for (iy = 0; iy < height; iy += 8)
		{
			for (ix = 0; ix < width; ix++)
			{
				idx -= bpp;
				alphaBuf		[ix] = pData [idx+1];
				if (c & 0x02)
					drawbuf [0][ix] = pData [idx];
				else
					drawbuf [0][ix] = 0;

				if (c & 0x01)
					drawbuf [1][ix] = pData [idx];
				else
					drawbuf [1][ix] = 0;
			}
			do_rcuAlpha (x, y+iy, width, m, alphaBuf);

			// go back to end of this page
			idx += width * bpp;	// 1bpp
			// goto next page
			idx += iconWidth * bpp;	// 1bpp
		}	// for (iy = 0; iy < height; iy += 8)
	}	// else if (s)
	is_drawing--;
}	// _drawBlackWhiteAlpha
//-----------------------------------------------------------------------------




