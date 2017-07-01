/**********************************************************************
@file       drawDigit.c

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
#include "global.h"
#include "lcd.h"
#include "drawDigit.h"


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

#define DIGIT_SEG_A   _BV (0)
#define DIGIT_SEG_B   _BV (1)
#define DIGIT_SEG_C   _BV (2)
#define DIGIT_SEG_D   _BV (3)
#define DIGIT_SEG_E   _BV (4)
#define DIGIT_SEG_F   _BV (5)
#define DIGIT_SEG_G   _BV (6)
#define DIGIT_SEG_H   _BV (7)

const unsigned char digitSegPattern [] =
{
/* 0 */  DIGIT_SEG_A               | DIGIT_SEG_C | DIGIT_SEG_D | DIGIT_SEG_E | DIGIT_SEG_F | DIGIT_SEG_G,
/* 1 */                                                                        DIGIT_SEG_F | DIGIT_SEG_G,
/* 2 */  DIGIT_SEG_A | DIGIT_SEG_B | DIGIT_SEG_C               | DIGIT_SEG_E | DIGIT_SEG_F              ,
/* 3 */  DIGIT_SEG_A | DIGIT_SEG_B | DIGIT_SEG_C                             | DIGIT_SEG_F | DIGIT_SEG_G,
/* 4 */                DIGIT_SEG_B               | DIGIT_SEG_D               | DIGIT_SEG_F | DIGIT_SEG_G,
/* 5 */  DIGIT_SEG_A | DIGIT_SEG_B | DIGIT_SEG_C | DIGIT_SEG_D                             | DIGIT_SEG_G,
/* 6 */  DIGIT_SEG_A | DIGIT_SEG_B | DIGIT_SEG_C | DIGIT_SEG_D | DIGIT_SEG_E               | DIGIT_SEG_G,
/* 7 */  DIGIT_SEG_A                                                         | DIGIT_SEG_F | DIGIT_SEG_G,
/* 8 */  DIGIT_SEG_A | DIGIT_SEG_B | DIGIT_SEG_C | DIGIT_SEG_D | DIGIT_SEG_E | DIGIT_SEG_F | DIGIT_SEG_G,
/* 9 */  DIGIT_SEG_A | DIGIT_SEG_B | DIGIT_SEG_C | DIGIT_SEG_D               | DIGIT_SEG_F | DIGIT_SEG_G
};
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void drawDigit (unsigned char x, unsigned char y,
                unsigned char w, unsigned char h, 
                unsigned char digit, unsigned char d, unsigned char c, unsigned char m)
{
   char w1 = w - 2*d - 2;
   char h1 = (h - 5) / 2;
   unsigned char i, mode, pattern;

   pattern = 0;
   if (digit <= 9)
      pattern = digitSegPattern [digit];

   w1 = max (w1, 3);
   h1 = max (h1, 3);
   if (!d)
      d = DIGIT_BAR_WIDTH;

   for (i = 0; i < 3; i++)
   {
      mode = DRAW_ERASE;
      if (pattern & _BV (i+0))
         mode = m;
      draw_block (x + d+1, y + i*(h1+1),
                  w1, d,
                  c, mode);
   }
   for (i = 0; i < 2; i++)
   {
      mode = DRAW_ERASE;
      if (pattern & _BV (i+3))
         mode = m;
      draw_block (x, y + d-1 + i*(h1+1),
                  d, h1,
                  c, mode);
   }
   for (i = 0; i < 2; i++)
   {
      mode = DRAW_ERASE;
      if (pattern & _BV (i+5))
         mode = m;
      draw_block (x + d + 1 + w1 + 1, y + d-1 + i*(h1+1),
                  d, h1,
                  c, mode);
   }
}   // drawDigit
//-----------------------------------------------------------------------------


