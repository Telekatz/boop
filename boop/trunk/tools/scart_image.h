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

/* bootstat register */
const unsigned char scart_bootstat = 0x01;

/* bootvec register, high byte of the reset vector */
const unsigned char scart_bootvec  = 0x00;

/* address of the first byte of scart_image to program to */
const unsigned short scart_image_startaddr = 0x0000;

/* from betty_scart.bin SVN #14 */
/*  truncated to a size of 0x1C00 = 7168 to preserve the content of the last flash sector */
#include "scart_image_data.h"
