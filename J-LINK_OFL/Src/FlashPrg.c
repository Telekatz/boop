/*********************************************************************
*               (c) SEGGER Microcontroller GmbH & Co. KG             *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
----------------------------------------------------------------------
File    : FlashPrg.c
Purpose : Implementation of RAMCode template
--------  END-OF-HEADER  ---------------------------------------------
*/
#include "FlashOS.H"
#include "lpc2220.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define PAGE_SIZE_SHIFT (1)      // The smallest program unit (one page) is 8 byte in size

/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
//
// We use this dummy variable to make sure that the PrgData
// section is present in the output elf-file as this section
// is mandatory in current versions of the J-Link DLL 
//
static volatile int _Dummy;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _FeedWatchdog
*
*  Function description
*    Feeds the watchdog. Needs to be called during RAMCode execution
*    in case of an watchdog is active.
*/
static void _FeedWatchdog(void) {
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Init
*
*  Function description
*    Handles the initialization of the flash module.
*
*  Parameters
*    Addr: Flash base address
*    Freq: Clock frequency in Hz
*    Func: Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int Init(U32 Addr, U32 Freq, U32 Func) {
  (void)Addr;
  (void)Freq;
  (void)Func;
  //
  // Init code
  //

  BCFG0 = 0x100005EF;
  PLLCON = 0x00000003;
  VPBDIV = 0x02;
  PLLCFG  = 0x42;   // 0100.0010  = PSEL=10=4 MSEL=00010=3
  PLLFEED = 0xAA;
  PLLFEED = 0x55;
  BCFG0 = 0x10001CA0;
  BCFG2 = 0x10001CA0;
  BCFG1 = 0x00000400;
  PINSEL0 = 0x00008005;
  PINSEL1 = 0x00000000;
  PINSEL2 = 0x0de049d4;
  IOSET2 = 0x01FC0000;
  IODIR2 = 0x01FC0000;
  IODIR0 = 0x002018D0;

  return 0;
}

/*********************************************************************
*
*       UnInit
*
*  Function description
*    Handles the de-initialization of the flash module.
*
*  Parameters
*    Func: Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int UnInit(U32 Func) {
  (void)Func;
  //
  // Uninit code
  //
  (void)Func; // Dummy

  return 0;
}

/*********************************************************************
*
*       EraseSector
*
*  Function description
*    Erases one flash sector.
*
*  Parameters
*    Addr: Address of the sector to be erased
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int EraseSector(U32 SectorAddr) {
  int r;
  unsigned long flash_base;

  r = 0;
  //
  // Erase sector code
  //

  flash_base = SectorAddr & 0xFF000000;

    *((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
    *((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
    *((volatile unsigned short *)(flash_base | 0xAAA)) = 0x80;
    *((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
    *((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
    *((volatile unsigned short *)(SectorAddr)) = 0x30;

    if((*((volatile unsigned short *)(SectorAddr)) & 0x44) == (*((volatile unsigned short *)(SectorAddr)) & 0x44))
    {
      *((volatile unsigned short *)(flash_base)) = 0xF0;
      return 1;
    }

  _FeedWatchdog();

  while ((*((volatile unsigned short *)(flash_base)) & 0x44) != (*((volatile unsigned short *)(flash_base)) & 0x44))
    {}

  return r;
}

/*********************************************************************
*
*       ProgramPage
*
*  Function description
*    Programs one flash page.
*
*  Parameters
*    DestAddr: Destination address
*    NumBytes: Number of bytes to be programmed (always a multiple of program page size, defined in FlashDev.c)
*    pSrcBuff: Point to the source buffer
*
*  Return value 
*    0 O.K.
*    1 Error
*/
int ProgramPage(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff) {
  volatile U16 * pSrc;
  volatile U16 * pDest;
  //U8 AccessWidth;
  //U32 Status;
  U32 NumPages;
  U32 NumBytesAtOnce;
  int r;
  unsigned long flash_base;

  r           = -1;
  pSrc        = (volatile U16*)pSrcBuff;
  pDest       = (volatile U16*)DestAddr;
  flash_base = DestAddr & 0xFF000000;
  //
  // RAMCode is able to program multiple pages
  //
  NumPages    = NumBytes >> PAGE_SIZE_SHIFT;
  //
  // Program page-wise
  //
  if (NumPages) {
    r = 0;
    do {
      NumBytesAtOnce = (1 << PAGE_SIZE_SHIFT);
      _FeedWatchdog();
      //
      // Program one page
      //

      *((volatile unsigned short *)(flash_base | 0xAAA)) = 0xAA;
      *((volatile unsigned short *)(flash_base | 0x554)) = 0x55;
      *((volatile unsigned short *)(flash_base | 0xAAA)) = 0xA0;
      *pDest = *pSrc;

      if(*pDest == *pSrc)
      {
        *((volatile unsigned short *)(flash_base)) = 0xF0;
        return 1;
      }

      while(*pDest != *pSrc)
      {}

      pDest++;
      pSrc++;
    } while (--NumPages);
  }
  return r;
}

/*********************************************************************
*
*       Verify
*
*  Function description
*    Compares a specified number of bytes of a provided data
*    buffer with the content of the device
*
*  Parameters
*    Addr: Start address in memory which should be compared
*    NumBytes: Number of bytes to be compared
*    pBuff: Pointer to the data to be compared
*
*  Return value
*    == (Addr + NumBytes): O.K.
*    != (Addr + NumBytes): *not* O.K. (ideally the fail address is returned)
*
*/
U32 Verify(U32 Addr, U32 NumBytes, U8 *pBuff) {
  unsigned char *pFlash;
  unsigned long r;

  pFlash = (unsigned char *)Addr;
  r = Addr + NumBytes;
  do {
      if (*pFlash != *pBuff) {
        r = (unsigned long)pFlash;
        break;
      }
      pFlash++;
      pBuff++;
  } while (--NumBytes);
  return r;
}

/*********************************************************************
*
*       BlankCheck
*
*  Function description
*    Checks if a memory region is blank
*
*  Parameters
*    Addr: Blank check start address
*    NumBytes: Number of bytes to be checked
*    BlankData: Pointer to the destination data
*
*  Return value
*    0: O.K., blank
*    1: O.K., *not* blank
*    < 0: Error
*
*/
int BlankCheck(U32 Addr, U32 NumBytes, U8 BlankData) {
  U8* pData;

  pData = (U8*)Addr;
  do {
    if (*pData++ != BlankData) {
      return 1;
    }
  } while (--NumBytes);
  return 0;
}

/*********************************************************************
*
*       SEGGER_OPEN_Program
*
*  Function description
*    Programs a specified number of bytes into the target flash.
*    NumBytes is either FlashDevice.PageSize or a multiple of it.
*
*  Notes
*    (1) This function can rely on that at least FlashDevice.PageSize will be passed
*    (2) This function must be able to handle multiple of FlashDevice.PageSize
*
*  Parameters
*    Addr: Start read address
*    NumBytes: Number of bytes to be read
*    pBuff: Pointer to the destination data
*
*  Return value
*    0 O.K.
*    1 Error
*
*/
int SEGGER_OPEN_Program(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff) {
  U32 NumPages;
  int r;

  NumPages = (NumBytes >> PAGE_SIZE_SHIFT);
  r = 0;
  do {
    r = ProgramPage(DestAddr, (1uL << PAGE_SIZE_SHIFT), pSrcBuff);
    if (r < 0) {
      return r;
    }
    DestAddr += (1uL << PAGE_SIZE_SHIFT);
    pSrcBuff += (1uL << PAGE_SIZE_SHIFT);
  } while (--NumPages);
  return r;
}

/*********************************************************************
*
*       SEGGER_OPEN_Erase
*
*  Function description
*    Erases one or more flash sectors
*
*  Notes
*    (1) This function can rely on that at least one sector will be passed
*    (2) This function must be able to handle multiple sectors at once
*    (3) This function can rely on that only multiple sectors of the same sector
*        size will be passed. (e.g. if the device has two sectors with different
*        sizes, the DLL will call this function two times with NumSectors = 1)
*
*  Parameters
*    SectorAddr: Address of the start sector to be erased
*    SectorIndex: Index of the start sector to be erased
*    NumSectors: Number of sectors to be erased. At least 1 sector is passed.
*
*  Return value
*    0 O.K.
*    1 Error
*
*/
/*
int SEGGER_OPEN_Erase(U32 SectorAddr, U32 SectorIndex, U32 NumSectors) {
  (void)SectorAddr;
  U32 Status;
  int r;

  (void)SectorIndex;
  _FeedWatchdog();
  r = 0;
  do {
    EraseSector(SectorAddr);
    SectorAddr += (1 << PAGE_SIZE_SHIFT);
  } while (--NumSectors);
  return r;
}
*/