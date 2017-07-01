#include "ax110xx.h"
#include "interrupt_ax110xx.h"
#include "cpu_ax110xx.h"
#include "types_ax110xx.h"

static unsigned char globeInt		= 0;
static unsigned char cpuSysClk		= 0;
static unsigned char memShadow		= 0;
static unsigned char progWtst		= 0;
static unsigned char dataStretch	= 0;

void AX11000_Init(void)
{
	EA = 0; // turn off globe interrupt

	P0 = 0xFF;
	P1 = 0xFF;
	P2 = 0xFF;
	P3 = 0xFF;

	/* Check program wait-state and data memory wait-state */
	if (CSREPR & PMS)
	{
		memShadow = 1;

		/* for real chip */
		switch (CSREPR & (BIT6|BIT7))
		{
			case SCS_100M :
				cpuSysClk = SCS_100M;
				WTST = 0x01;
				CKCON = 0x01;
				break;
			case SCS_50M :
				cpuSysClk = SCS_50M;
				WTST = 0x00;
				CKCON = 0x01;
				break;
			case SCS_25M :
				cpuSysClk = SCS_25M;
				WTST = 0x00;
				CKCON = 0x01;
				break;
		}
	}
	else
	{
		memShadow = 0;

		/* for real chip */
		switch (CSREPR & (BIT6|BIT7))
		{
			case SCS_100M :
				cpuSysClk = SCS_100M;
				WTST = 0x07;
				CKCON = 0x01;
				break;
			case SCS_50M :
				cpuSysClk = SCS_50M;
				WTST = 0x03;
				CKCON = 0x01;
				break;
			case SCS_25M :
				cpuSysClk = SCS_25M;
				WTST = 0x01;
				CKCON = 0x01;
				break;
		}
	}

	progWtst = WTST & 0x07;
	dataStretch = CKCON & 0x07;

	PCON	&= ~SWB_;	// Turn off switchback function.

	EIF		&= ~INT5F;	// Clear the flag of interrupt 5.
	EIF		&= ~INT6F;	// Clear the flag of interrupt 6.
	if (WTRF)
	{
		CSREPR |= SW_RST;
		WTRF	= 0;		// Clear Watch Dog Time Out Reset Flag.
	}

	PINT2	= 1;		// INT2 priority is at high level for DMA request.

	EXTINT0(OFF);		// EINT0 interrupt.
	EXTINT1(OFF);		// EINT1 interrupt.
	EXTINT2(OFF);		// EINT2 interrupt for DMA request.
	EXTINT3(OFF);		// EINT3 interrupt for PCA.
	EXTINT4(OFF);		// EINT4 interrupt for peripheral device.
	EXTINT5(OFF);		// EINT5 interrupt for software timer.
	EXTINT6(OFF);		// EINT6 interrupt for wake-up of power management mode and stop mode.

	if (globeInt & BIT7)
		EA = globeInt & BIT0;
	else
		EA = 1;		// Enable the globe interrupt.
}

unsigned char AX11000_GetSysClk(void)
{
	return cpuSysClk;
}

unsigned char AX11000_GetMemSdw(void)
{
	return memShadow;
}

unsigned char AX11000_GetProgWst(void)
{
	return progWtst;
}

unsigned char AX11000_GetDataSth(void)
{
	return dataStretch;
}

void AX11000_SoftReset(void)
{
	if (EA)
		globeInt = BIT0 | BIT7; // BIT0:EA bit record ; BIT7:Software reset record.
	else
		globeInt = BIT7;

	EA = 0;

	CSREPR |= SW_RST;
}

void AX11000_SoftReboot(void)
{
	CSREPR |= SW_RBT;
}

void AX11000_WatchDogSetting(unsigned char wdIntrEnb, unsigned char wdRstEnb, unsigned char wdPrty, unsigned char wdTime)
{
	CKCON |= wdTime;

	if (wdIntrEnb & WD_INTR_ENABLE)
	{
		EWDI = 1;
	}
	else
	{
		EWDI = 0;
	}

	if (wdRstEnb & WD_RESET_ENABLE)
	{
		TA = 0xAA;
		TA = 0x55;
		EWT = 1;
	}
	else
	{
		TA = 0xAA;
		TA = 0x55;
		EWT = 0;
	}

	if (wdPrty & WD_HIGH_PRIORITY)
	{
		PWDI = 1;
	}
	else
	{
		PWDI = 0;
	}
	
	TA = 0xAA;
	TA = 0x55;
	RWT = 1;
	WTRF = 0;
	WDIF = 0;
}
