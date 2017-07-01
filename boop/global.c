/*
    global.c
    Copyright (C) 2007  Colibri <colibri_dvb@lycos.com>

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
#include "rf.h"
#include "global.h"

#define debugbuffersize   50
unsigned char debugoutbuf [debugbuffersize];

void *memcpy(void *dest,void *src,int count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
}

int memcmp(const void * cs,const void * ct,int count)
{
	const unsigned char *su1, *su2;
	signed char res = 0;

	for( su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
		if ((res = *su1 - *su2) != 0)
			break;
	return res;
}

unsigned char GetHWversion (void)
{
	if(((IOPIN2 >> 28) & 3) == 3)
		return(((((IOPIN0 >> 5) & 1) << 4) | (((IOPIN2 >> 30) & 1) << 3) | (((IOPIN2 >> 16) & 1) << 2) | 3));
	return(((IOPIN2 >> 27) & 2) + ((IOPIN2 >> 29) & 1) + (((IOPIN1 >> 23) & 7) << 2));
}

int puts (const char* string)
{
	struct RFendpoint_* cur_ep;
	volatile unsigned char i;
	
	cur_ep = openEP(debugoutbuf,0, packet_test);
	
   if (cur_ep)
   {        
      for (i=0; (i<debugbuffersize) && string[i];i++)
      {
			cur_ep->buffer[i] = string[i];
		}
		
		cur_ep->buffer[i] = 0;
		cur_ep->bufferlen = i+1;
		cur_ep->flags |= EPenabled | EPoutput | EPnewdata;
		
		
		RF_changestate(RFtx);
      // wait until all date are sent
		while(cur_ep->flags & EPnewdata);
		closeEP(cur_ep);
	}
	return 1;
}

/*
 * The width of the CRC calculation and result.
 * Modify the typedef for a 16 or 32-bit CRC standard.
 */

#define CRC_POLYNOMIAL 0x8005
#define CRC_WIDTH  (8 * sizeof(crc))
#define CRC_TOPBIT (1 << (CRC_WIDTH - 1))

crc CRCs(char* message)
{
    crc  remainder = 0;	
	int byte;
	unsigned char bit;
	
    /*
     * Perform modulo-2 division, a byte at a time.
     */
    for (byte = 0; message[byte]; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        remainder ^= (message[byte] << (CRC_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & CRC_TOPBIT)
            {
                remainder = (remainder << 1) ^ CRC_POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    /*
     * The final remainder is the CRC result.
     */
    return (remainder);

}   /* crcSlow() */

