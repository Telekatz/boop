/*
	cc1100.h - 
	Copyright (C) 2007

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef cc1100_H
#define cc1100_H

#define MISO1			(1<<18)
#define MOSI1			(1<<19)
#define SCK1			(1<<17)
#define CS1				(1<<23)
#define GDO0			(1<<16)
#define GDO1			(1<<18)
#define GDO2			(1<<24)


#define WRITE			0x00
#define BURST			0x40
#define READ			0x80
#define TX_fifo			0x7F
#define RX_fifo			0xff
#define PATABLE			0x3E

//Command Strobes
#define SRES			0x30
#define SFSTXON			0x31
#define SXOFF			0x32
#define SCAL			0x33
#define SRX				0x34
#define STX 			0x35
#define SIDLE			0x36
#define SWOR			0x38
#define SPWD			0x39
#define SFRX			0x3A
#define SFTX			0x3B
#define SWORRST			0x3C
#define SNOP			0x3D

//Status Registers
#define PARTNUM			(0x30 | READ | BURST)
#define VERSION			(0x31 | READ | BURST)
#define FREQEST			(0x32 | READ | BURST)
#define LQI				(0x33 | READ | BURST)
#define RSSI			(0x34 | READ | BURST)
#define MARCSTATE		(0x35 | READ | BURST)
#define WORTIME1		(0x36 | READ | BURST)
#define WORTIME0		(0x37 | READ | BURST)
#define PKTSTATUS		(0x38 | READ | BURST)
#define VCO_VC_DAC		(0x39 | READ | BURST)
#define TXBYTES			(0x3A | READ | BURST)
#define RXBYTES			(0x3B | READ | BURST)
#define RCCTRL1_STATUS	(0x3C | READ | BURST)
#define RCCTRL0_STATUS	(0x3D | READ | BURST)

//Configuration Registers
#define IOCFG2			0x00		// GDO2 output pin configuration
#define IOCFG1			0x01		// GDO1 output pin configuration
#define IOCFG0			0x02		// GDO0 output pin configuration
#define FIFOTHR			0x03		// RX FIFO and TX FIFO thresholds
#define SYNC1			0x04		// Sync word, high byte
#define SYNC0			0x05		// Sync word, low byte
#define PKTLEN			0x06		// Packet length
#define PKTCTRL1		0x07		// Packet automation control
#define PKTCTRL0		0x08		// Packet automation control
#define ADDR_			0x09		// Device address
#define CHANNR			0x0A		// Channel number
#define FSCTRL1			0x0B		// Frequency synthesizer control
#define FSCTRL0			0x0C		// Frequency synthesizer control
#define FREQ2			0x0D		// Frequency control word, high byte
#define FREQ1			0x0E		// Frequency control word, middle byte
#define FREQ0			0x0F		// Frequency control word, low byte
#define MDMCFG4			0x10		// Modem configuration
#define MDMCFG3			0x11		// Modem configuration
#define MDMCFG2			0x12		// Modem configuration
#define MDMCFG1			0x13		// Modem configuration
#define MDMCFG0			0x14		// Modem configuration
#define DEVIATN			0x15		// Modem deviation setting
#define MCSM2			0x16		// Main Radio Cntrl State Machine config
#define MCSM1			0x17		// Main Radio Cntrl State Machine config
#define MCSM0			0x18		// Main Radio Cntrl State Machine config
#define FOCCFG			0x19		// Frequency Offset Compensation config
#define BSCFG			0x1A		// Bit Synchronization configuration
#define AGCCTRL2		0x1B		// AGC control
#define AGCCTRL1		0x1C		// AGC control
#define AGCCTRL0		0x1D		// AGC control
#define WOREVT1			0x1E		// High byte Event 0 timeout
#define WOREVT0			0x1F		// Low byte Event 0 timeout
#define WORCTRL			0x20		// Wake On Radio control
#define FREND1			0x21		// Front end RX configuration
#define FREND0			0x22		// Front end TX configuration
#define FSCAL3			0x23		// Frequency synthesizer calibration
#define FSCAL2			0x24		// Frequency synthesizer calibration
#define FSCAL1			0x25		// Frequency synthesizer calibration
#define FSCAL0			0x26		// Frequency synthesizer calibration
#define RCCTRL1			0x27		// RC oscillator configuration
#define RCCTRL0			0x28		// RC oscillator configuration
#define FSTEST			0x29		// Frequency synthesizer cal control
#define PTEST			0x2A		// Production test
#define AGCTEST			0x2B		// AGC test
#define TEST2			0x2C		// Various test settings
#define TEST1			0x2D		// Various test settings
#define TEST0			0x2E		// Various test settings

#define MARCSTATE_IDLE	0x01
#define MARCSTATE_RX	0x0d

extern const unsigned char conf[]; //__attribute__((aligned(0x4)));
extern const unsigned char confasync[];// __attribute__((aligned(0x4)));

void cc1100_init(void);
unsigned char cc1100_write(unsigned char addr, unsigned char* data, unsigned char length);
unsigned char cc1100_write1(unsigned char addr,unsigned char data);
unsigned char cc1100_read(unsigned char addr, unsigned char* data, unsigned char length);
unsigned char cc1100_read1(unsigned char addr);
unsigned char cc1100_strobe(unsigned char cmd);

#endif
