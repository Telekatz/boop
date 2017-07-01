#ifndef INTERRUPT_AX110XX_H
#define INTERRUPT_AX110XX_H

#define LBI_IRQ_ENABLE			0 // Local Bus IRQ; 1:On 2:Off
#define ETH_IRQ_ENABLE			0 // Ethernet IRQ; 1:On 2:Off
#define I2C_IRQ_ENABLE			0 // I2C IRQ; 1:On 2:Off
#define SPI_IRQ_ENABLE			0 // SPI IRQ; 1:On 2:Off
#define ONEWIRE_IRQ_ENABLE		0 // OneWire IRQ; 1:On 2:Off
#define CAN_IRQ_ENABLE			0 // CAN IRQ; 1:On 2:Off
#define HSUART_IRQ_ENABLE		0 // HS-Uart IRQ; 1:On 2:Off
#define BOOTLDR_IRQ			0 // Bootloader Source Code; 1:On 2:Off
#define POWER_MANAGEMENT_IRQ_ENABLE	0 // Power Management Wake-Up IRQ; 1:On 2:Off
#define WATCHDOG_IRQ_ENABLE		0 // Watchdog IRQ; 1:On 2:Off

#define ON		1
#define OFF		0
#define EXTINT0(val)	{EX0 = val;}
#define EXTINT1(val)	{EX1 = val;}
#define EXTINT2(val)	{EINT2 = val;}
#define EXTINT3(val)	{EINT3 = val;}
#define EXTINT4(val)	{EINT4 = val;}
#define EXTINT5(val)	{EINT5 = val;}
#define EXTINT6(val)	{EINT6 = val;}

#endif
