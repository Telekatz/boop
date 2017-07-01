#include "ax110xx.h"
#include "cpu_ax110xx.h"
#include "interrupt_ax110xx.h"
#include "types_ax110xx.h"

#if LBI_IRQ_ENABLE
#include	"lbi.h"
#endif

#if ETH_IRQ_ENABLE
#include	"ethernet/mac.h"
#include	"ethernet/stoe.h"
#endif

#if I2C_IRQ_ENABLE
#include	"i2c.h"
#endif

#if SPI_IRQ_ENABLE
#include	"spi.h"
#endif

#if ONEWIRE_IRQ_ENABLE
#include	"onewire.h"
#endif

#if CAN_IRQ_ENABLE
#include	"can.h"
#endif

#if HSUART_IRQ_ENABLE
#include	"hsuart.h"
#endif

void	ax11000_PeripherialISR(void) __interrupt 9;

#if POWER_MANAGEMENT_IRQ_ENABLE
void	ax11000_PmmISR(void);
#endif

#if WATCHDOG_IRQ_ENB
void	ax11000_WatchDogISR(void);
#endif

void ax11000_PeripherialISR(void) __interrupt 9
{
	unsigned int	intrStt1 = 0;
	unsigned int	intrStt2 = 0;

	/* Interrupt type check */
	while (1)
	{
		EA = 0;
		intrStt1 = PISSR1;
		intrStt2 = PISSR2;
		EA = 1;

		if ((intrStt1 == 0) && (intrStt2 == 0))
			break;

#if LBI_IRQ_ENABLE
		if (intrStt1 & LB_INT_STU)
		{
			LocalBusFunc();
		}

		if (intrStt1 & LB_EXT_INT_STU)
		{
			LocalBusAccessCompleteFunc();
		}
#endif

#if ETH_IRQ_ENABLE
#if (MAC_GET_INTSTATUS_MODE == MAC_INTERRUPT_MODE)
		if (intrStt1 & ETH_INT_STU)
		{
			MAC_SetInterruptFlag();
		}
#endif
#endif

#if ETH_IRQ_ENABLE
#if (STOE_GET_INTSTATUS_MODE == STOE_INTERRUPT_MODE)
		if (intrStt1 & TOE_INT_STU)
		{
			STOE_SetInterruptFlag();
		}
#endif
#endif

#if CAN_IRQ_ENABLE
		if (intrStt1 & CAN_INT_STU)
		{
			CAN_Func();
		}
#endif

#if I2C_IRQ_ENABLE
		if (intrStt1 & I2C_INT_STU)
		{
			I2C_Func();
		}
#endif

#if SPI_IRQ_ENABLE
		if (intrStt1 & SPI_INT_STU)
		{
			SPI_Func();
		}
#endif

#if ONEWIRE_IRQ_ENABLE
		if (intrStt1 & OW_INT_STU)
		{
			ONEWIRE_Func();
		}
#endif

#if HSUART_IRQ_ENABLE
		if (intrStt2 & UART2_INT_STU)
		{
			HSUR_Func();
		}
#endif
	}
}

#if POWER_MANAGEMENT_IRQ_ENABLE
void ax11000_PmmISR(void) __interrupt 11
{
	unsigned int	wakeStatus = 0;
	unsigned int	mode = 0;
	unsigned int	sysClk = 0;
	unsigned int	canReg = 0;
	unsigned int	intEnb = 0;

	EA = 0;
	EIF &= ~INT6F;	// Clear the flag of interrupt 6.
	EA = 1;

	

	wakeStatus = PCON;
	if (!(wakeStatus & SWB_))
	{
		PCON &= ~ (PMM_ | STOP_);
	}

#if ETH_IRQ_ENABLE
	/* Ethernet wake up function */
	/* disable wake up interrupt */
	MDR  = 0;
	MCIR = MAC_STOP_PMM_INT_ENABLE_REG;
	/* clear MAC status register */
	CLI_MacRd((ULONG)MAC_STOP_PMM_INT_STATUS_REG, &WakeStatus, 1);
	STOE_Standby = 0;
#endif

#if CAN_IRQ_ENABLE
	/* CAN wake up function */
	CAN_WakeUpEvent(0); // 0 for exit power management mode
#endif
}
#endif

#if WATCHDOG_IRQ_ENB
ax11000_WatchDogISR(void) __interrupt 12
{
	if (EWDI)
	{
		if (EWT)
		{
			TA = 0xAA;
			TA = 0x55;
			WDIF = 0; // clear WatchDog Interrupt Flag.
			WTRF = 0; // clear the Watchdog Timer Reset Flag.
			RWT = 1; // reset watchdog timer.
		}
		else
		{
			TA = 0xAA;
			TA = 0x55;
			WDIF = 0; //clear WatchDog Interrupt Flag.
			RWT = 1; // reset watchdog timer.
		}
	}
}
#endif
