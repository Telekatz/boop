/**********************************************************************
@file       drawDigit.h

@brief      Draw a scalable 7-segment digit

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

1.00 10012010 RSt  Creation

@end
***********************************************************************/
#ifndef __DRAW_DIGIT_H__
#define __DRAW_DIGIT_H__

#include "global.h"
#include "lcd.h"

#ifndef DIGIT_BAR_WIDTH
#define DIGIT_BAR_WIDTH   3
#endif

//      |<-   w   ->|
//      |           |
//      |d|1| w1|1|d|
//           ___      ______
//       _  |_a_|  _  _2_  ^
//      | |       | |      |
//      d |       | f  h1
//      |_|  ___  |_| ___  h
//       _  |_b_|  _   1
//      | |       | |
//      e |       | g      |
//      |_|  ___  |_|      v
//          |_c_|     _2____
//

//-----------------------------------------------------------------------------
void drawDigit (unsigned char x, unsigned char y,
                unsigned char w, unsigned char h, 
                unsigned char digit, unsigned char d, unsigned char c, unsigned char m);
//-----------------------------------------------------------------------------


#endif   // #ifndef __DRAW_DIGIT_H__



