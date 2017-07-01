/* identify the Entry Point  */

ENTRY(_startup)

/* specify the LPC2106 memory areas  */

MEMORY 
{
	flash	: ORIGIN = 0x80000000, LENGTH = 1M		/* FLASH ROM		*/	
	ram  	: ORIGIN = 0x40000200, LENGTH = 64K - 0x220	/* free RAM area	*/
}

/* define a global symbol _stack_end  */

_stack_end = 0x4000FEDC;

/* now define the output sections  */

SECTIONS 
{
	. = 0;			/* set location counter to address zero  */
	
	startup :
	{
		*(.startup)
	} >flash		/* the startup code goes into FLASH */

	.text :			/* collect all sections that should go into FLASH after startup  */ 
	{
		*(.text)	/* all .text sections (code)  */
		*(.rodata)	/* all .rodata sections (constants, strings, etc.)  */
		*(.rodata*)	/* all .rodata* sections (constants, strings, etc.)  */
		*(.glue_7)	/* all .glue_7 sections  (no idea what these are) */
		*(.glue_7t)	/* all .glue_7t sections (no idea what these are) */
		_etext = .;	/* define a global symbol _etext just after the last code byte */
	} >flash		/* put all the above into FLASH */

	.data :			/* collect all initialized .data sections that go into RAM  */ 
	{
		_data = .;	/* create a global symbol marking the start of the .data section  */
		*(.data)	/* all .data sections  */
		_edata = .;	/* define a global symbol marking the end of the .data section  */
	} >ram AT >flash	/* put all the above into RAM (but load the LMA copy into FLASH) */

	.bss :			/* collect all uninitialized .bss sections that go into RAM  */
	{
		_bss_start = .;	/* define a global symbol marking the start of the .bss section */
		*(.bss)		/* all .bss sections  */
	} >ram			/* put all the above in RAM (it will be cleared in the startup code */

	. = ALIGN(4);		/* advance location counter to the next 32-bit boundary */
	_bss_end = . ;		/* define a global symbol marking the end of the .bss section */
}
	_end = .;		/* define a global symbol marking the end of application RAM */

PROVIDE( BCFG0 = 0xFFE00000);
PROVIDE( BCFG1 = 0xFFE00004);
PROVIDE( BCFG2 = 0xFFE00008);
PROVIDE( PINSEL0 = 0xE002C000);
PROVIDE( PINSEL2 = 0xE002C014);
PROVIDE( IO0PIN = 0xE0028000);
