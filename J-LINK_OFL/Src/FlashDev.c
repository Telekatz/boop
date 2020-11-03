/*********************************************************************
*               (c) SEGGER Microcontroller GmbH & Co. KG             *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
----------------------------------------------------------------------
File    : FlashDev.c
Purpose : Flash device description Template
--------  END-OF-HEADER  ---------------------------------------------
*/

#include "FlashOS.H"

struct FlashDevice const FlashDevice __attribute__ ((section ("DevDscr"))) =  {
//struct FlashDevice const FlashDevice =  {
  ALGO_VERSION,              // Algo version
  "EN29LV800BB",             // Flash device name
  ONCHIP,                    // Flash device type
  0x80000000,                // Flash base address
  0x00100000,                // Total flash device size in Bytes (1024 KB)
  2,                         // Page Size (number of bytes that will be passed to ProgramPage(). May be multiple of min alignment in order to reduce overhead for calling ProgramPage multiple times
  0,                         // Reserved, should be 0
  0xFF,                      // Flash erased value
  1000,                       // Program page timeout in ms
  6000,                      // Erase sector timeout in ms
  //
  // Flash sector layout definition
  //
  0x00004000, 0x00000000,   // 1 *  16 KB =  16 KB
  0x00002000, 0x00004000,   // 2 *   8 KB =  16 KB
  0x00008000, 0x00008000,   // 1 *  32 KB =  32 KB
  0x00010000, 0x00010000,   // 15*  64 KB = 960 KB
  0xFFFFFFFF, 0xFFFFFFFF    // Indicates the end of the flash sector layout. Must be present.
};
