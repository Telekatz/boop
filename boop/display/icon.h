/**********************************************************************
@file       icon.c

@brief      Definition of icon structure and appropriate functions

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
#ifndef __ICON_H__
#define __ICON_H__


///////////////////////////////////////////////////////////////////
// ICON can be defined in the appropriate C file before
// including that header or the related icon files to define the 
// actual type of the icon structure:
//
// >> #define ICON   iconElement_t
//
///////////////////////////////////////////////////////////////////
#ifndef ICON
#define ICON   icon_t
#endif

///////////////////////////////////////////////////////////////////
// ICON_INFO can be defined in the appropriate C file before
// including that header to save one byte in the icon structure:
//
// >> #define ICON_INFO(i)
//
///////////////////////////////////////////////////////////////////
#ifndef ICON_INFO
#define ICON_INFO(i)        /* info    = */ i,
#define ICON_INFO_ELEMENT   unsigned char info;   // additional one byte (iconType_t might be larger)
#endif

#ifndef ICON_INFO_ELEMENT
#define ICON_INFO_ELEMENT
//#define ICON_IS_GRAY(i)   0
#define ICON_IS_GRAY(i) (sizeof(i.data) > (i.height/8 + (i.height%8 ? 1 : 0)) * i.width + 2);
#else
#define ICON_IS_GRAY(i)   ((i)->info >= ICON_GRAY)
#endif

typedef enum iconInfo_e
{
   ICON_BLACK_WHITE,               /// 1 bpp
   ICON_BLACK_WHITE_TRANSPARENT,   /// 2 bpp
   ICON_GRAY,                      /// 2 bpp
   ICON_GRAY_TRANSPARENT,          /// 3 bpp
   
   ICON_MAX
} iconInfo_t;

typedef struct icon_s
{
   unsigned char width;
   unsigned char height;
   ICON_INFO_ELEMENT
   unsigned char data [];
} icon_t, *icon_p;

typedef struct iconElement_s
{
   unsigned char width;
   unsigned char height;
   unsigned char data [];
} iconElement_t, *iconElement_p;

#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------

extern void drawIcon    (unsigned char x, unsigned char y,
                         const icon_t const *pIcon,
                         unsigned char c, unsigned char m);

extern void drawIconExt (unsigned char x, unsigned char y,
                         const unsigned char const *pData,
                         unsigned char width, unsigned char height, iconInfo_t iconInfo,
                         unsigned char c, unsigned char m);

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif   // #ifndef __ICON_H__
