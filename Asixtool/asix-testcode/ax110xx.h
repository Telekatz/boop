#ifndef AX110XX_H
#define AX110XX_H

/* Byte Registers */
__sfr __at (0x80)	P0;	// Port 0
__sfr __at (0x81)	SP;	// Stack Pointer
__sfr __at (0x82)	DPL0;	// Data Pointer 0 Low Byte
__sfr __at (0x83)	DPH0;	// Data Pointer 0 High Byte
__sfr __at (0x82)	DPL;	// Data Pointer 0 Low Byte
__sfr __at (0x83)	DPH;	// Data Pointer 0 High Byte
__sfr __at (0x84)	DPL1;	// Data Pointer 1 Low Byte
__sfr __at (0x85)	DPH1;	// Data Pointer 1 High Byte
__sfr __at (0x86)	DPS;	// Data Pointers Select Register
__sfr __at (0x87)	PCON;	// Power Configuration Register
__sfr __at (0x88)	TCON;	// Timer 0,1 Configuration Register
__sfr __at (0x89)	TMOD;	// Timer 0,1 Control Mode Register
__sfr __at (0x8A)	TL0;	// Timer 0 Low Byte Counter
__sfr __at (0x8B)	TL1;	// Timer 1 Low Byte Counter
__sfr __at (0x8C)	TH0;	// Timer 0 High Byte Counter
__sfr __at (0x8D)	TH1;	// Timer 1 High Byte Counter
__sfr __at (0x8E)	CKCON;	// Clock Control Register
__sfr __at (0x8F)	CSREPR;	// Software Reset and External Program RAM Select Register

__sfr __at (0x90)	P1;	// Port 1
__sfr __at (0x91)	EIF;	// Extended Interrupt Flag Register
__sfr __at (0x92)	WTST;	// Program Wait States Register
__sfr __at (0x93)	DPX0;	// Data Pointer Extended Register 0
__sfr __at (0x93)	DPX;	// Data Pointer Extended Register 0
__sfr __at (0x94)	SDSTSR;	// Software Timer and Software DMA complete interrupt status
__sfr __at (0x95)	DPX1;	// Data Pointer Extended Register 1
__sfr __at (0x96)	I2CCIR;	// I2C Command Index Register
__sfr __at (0x97)	I2CDR;	// I2C Data Register
__sfr __at (0x98)	SCON0;	// UART 0 Configuration Register
__sfr __at (0x99)	SBUF0;	// UART 0 Buffer Register
__sfr __at (0x9A)	DBAR;	// DMA Bus Arbitration Register
__sfr __at (0x9B)	DCIR;	// DMA Command Index Register
__sfr __at (0x9C)	DDR;	// DMA Data Register
__sfr __at (0x9D)	ACON;	// Address Control Register
__sfr __at (0x9E)	PISSR1;	// Peripheral Interrupt Status Summary Register 1
__sfr __at (0x9F)	PISSR2;	// Peripheral Interrupt Status Summary Register 2

__sfr __at (0xA0)	P2;	// Port 2
/* Local Bus Master Mode */
__sfr __at (0xA1)	LMSR;	// Local Bus Mode Setup Register
__sfr __at (0xA2)	LCR;	// Local Bus Command Register
__sfr __at (0xA3)	LSR;	// Local Bus Status Register
__sfr __at (0xA4)	LDALR;	// Local Bus Device Address Low Register
__sfr __at (0xA5)	LDAHR;	// Local Bus Device Address High Register
__sfr __at (0xA6)	LDCSR;	// Local Bus Device Chip Select Register
/* Local Bus Slave Mode */
__sfr __at (0xA1)	LSAIER;	// Local Bus Slave mode Action and Interrupt Enable Register
__sfr __at (0xA2)	LSCR;	// Local Bus Slave mode Command Register
__sfr __at (0xA3)	LSSR;	// Local Bus Slave mode Status Register
__sfr __at (0xA4)	XMWLR;	// External Master Write-read Low Register
__sfr __at (0xA5)	XMWHR;	// External Master Write-read High Register
__sfr __at (0xA6)	XMRLR;	// External Master Read-only Low Register
__sfr __at (0xA7)	XMRHR;	// External Master Read-only High Register
__sfr __at (0xA8)	IE;	// Interrupt Enable register
__sfr __at (0xA9)	LDLR;	// Local Bus Data Low Register
__sfr __at (0xAA)	LDHR;	// Local Bus Data High Register
__sfr __at (0xAB)	DMALR;	// Local Bus DMA Address Low Register
__sfr __at (0xAC)	DMAMR;	// Local Bus DMA Address Medium Register
__sfr __at (0xAD)	DMAHR;	// Local Bus DMA Address High Register
__sfr __at (0xAE)	TCIR;	// TOE Command Index Register
__sfr __at (0xAF)	TDR;	// TOE Data Register

__sfr __at (0xB0)	P3;	// Port 3
__sfr __at (0xB1)	CCAPL0;	// PCA Module 0 Compare/Capture Low Byte Register
__sfr __at (0xB2)	CCAPL1;	// PCA Module 1 Compare/Capture Low Byte Register
__sfr __at (0xB3)	CCAPL2;	// PCA Module 2 Compare/Capture Low Byte Register
__sfr __at (0xB4)	CCAPL3;	// PCA Module 3 Compare/Capture Low Byte Register
__sfr __at (0xB5)	CCAPL4;	// PCA Module 4 Compare/Capture Low Byte Register
__sfr __at (0xB6)	MCIR;	// MAC Command Index Register
__sfr __at (0xB7)	MDR;	// MAC Data Register
__sfr __at (0xB8)	IP;	// Interrupt Priority Register
__sfr __at (0xB9)	CCAPH0;	// PCA Module 0 Compare/Capture High Byte Register
__sfr __at (0xBA)	CCAPH1;	// PCA Module 1 Compare/Capture High Byte Register
__sfr __at (0xBB)	CCAPH2;	// PCA Module 2 Compare/Capture High Byte Register
__sfr __at (0xBC)	CCAPH3;	// PCA Module 3 Compare/Capture High Byte Register
__sfr __at (0xBD)	CCAPH4;	// PCA Module 4 Compare/Capture High Byte Register
__sfr __at (0xBE)	EPCR;	// Ethernet PHY Command Index Register
__sfr __at (0xBF)	EPDR;	// Ethernet PHY Data Register

__sfr __at (0xC0)	SCON1;	// UART 1 Configuration Register
__sfr __at (0xC1)	SBUF1;	// UART 1 Buffer Register
__sfr __at (0xC2)	CMOD;	// PCA Timer/Counter Mode Register
__sfr __at (0xC3)	CCON;	// PCA Timer/Counter Control Register
__sfr __at (0xC4)	CL;	// PCA Timer/Counter Low
__sfr __at (0xC5)	CH;	// PCA Timer/Counter High
__sfr __at (0xC8)	T2CON;	// Timer 2 Configuration Register
__sfr __at (0xC9)	T2IF;	// Timer 2 Interrupt Flags
__sfr __at (0xCA)	RLDL;	// Timer 2 Reload Low Byte
__sfr __at (0xCB)	RLDH;	// Timer 2 Reload High Byte
__sfr __at (0xCC)	TL2;	// Timer 2 Low Byte Counter
__sfr __at (0xCD)	TH2;	// Timer 2 High Byte Counter
__sfr __at (0xCE)	SPICIR;	// SPI Command Index Register
__sfr __at (0xCF)	SPIDR;	// SPI Data Register

__sfr __at (0xD0)	PSW;	// Program Status Word
__sfr __at (0xD1)	CCAPM0;	// PCA Compare/Capture Module Mode Register 0
__sfr __at (0xD2)	CCAPM1;	// PCA Compare/Capture Module Mode Register 1
__sfr __at (0xD3)	CCAPM2;	// PCA Compare/Capture Module Mode Register 2
__sfr __at (0xD4)	CCAPM3;	// PCA Compare/Capture Module Mode Register 3
__sfr __at (0xD5)	CCAPM4;	// PCA Compare/Capture Module Mode Register 4
__sfr __at (0xD6)	OWCIR;	// OneWire Command Index Register
__sfr __at (0xD7)	OWDR;	// OneWire Data Register
__sfr __at (0xD8)	WDCON;	// Watchdog Configuration
__sfr __at (0xDE)	CANCIR;	// CAN Bus Command Index Register
__sfr __at (0xDF)	CANDR;	// CAN Bus Data Register

__sfr __at (0xE0)	ACC;	// Accumulator
/* UART2 registers */
__sfr __at (0xE1)	UR2_DLL;// High Speed UART2 Divisor Latch Low Register
__sfr __at (0xE1)	UR2_THR;// High Speed UART2 Transmit Holding Register for Writing
__sfr __at (0xE1)	UR2_RBR;// High Speed UART2 Receive Buffer Register for Reading
__sfr __at (0xE2)	UR2_DLH;// High Speed UART2 Divisor Latch High Register
__sfr __at (0xE2)	UR2_IER;// High Speed UART2 Interrupt Enable Register
__sfr __at (0xE3)	UR2_FCR;// High Speed UART2 FIFO Control Register for Writing
__sfr __at (0xE3)	UR2_IIR;// High Speed UART2 Interrupt Identification Register for Reading
__sfr __at (0xE4)	UR2_LCR;// High Speed UART2 Line Control Register
__sfr __at (0xE5)	UR2_MCR;// High Speed UART2 Modem Control Register
__sfr __at (0xE6)	UR2_LSR;// High Speed UART2 Line Status Register
__sfr __at (0xE7)	UR2_MSR;// High Speed UART2 Modem Status Register
__sfr __at (0xE8)	EIE;	// External Interrupt Enable
__sfr __at (0xE9)	STATUS;	// Status Register
__sfr __at (0xEA)	MXAX;	// MOVX @Ri Extended Register
__sfr __at (0xEB)	TA;	// Timed Access Protection

__sfr __at (0xF0)	B;	// B Working Register
__sfr __at (0xF8)	EIP;	// Extended Interrupt Priority Register
__sfr __at (0xF9)	MD0;	// MDU Data 0
__sfr __at (0xFA)	MD1;	// MDU Data 1
__sfr __at (0xFB)	MD2;	// MDU Data 2
__sfr __at (0xFC)	MD3;	// MDU Data 3
__sfr __at (0xFD)	MD4;	// MDU Data 4
__sfr __at (0xFE)	MD5;	// MDU Data 5
__sfr __at (0xFF)	ARCON	;// MDU Control Register

/* Bit Registers */

/* P0 (0x80) Bit Registers */
__sbit __at (0x80)	P0_0;
__sbit __at (0x81)	P0_1;
__sbit __at (0x82)	P0_2;
__sbit __at (0x83)	P0_3;
__sbit __at (0x84)	P0_4;
__sbit __at (0x85)	P0_5;
__sbit __at (0x86)	P0_6;
__sbit __at (0x87)	P0_7;

/* DPS (0x86) Bit Values */
#define SEL_		0x01
#define TSL_		0x20
#define ID0_		0x40
#define ID1_		0x80

/* PCON (0x87) Bit Values */
#define PMM_		0x01
#define STOP_		0x02
#define SWB_		0x04
#define PWE_		0x10
#define	SMOD1_		0x40
#define SMOD2_		0x80

/* TCON (0x88) Bit Registers */
__sbit __at (0x88)	IT0;
__sbit __at (0x89)	IE0;
__sbit __at (0x8A)	IT1;
__sbit __at (0x8B)	IE1;
__sbit __at (0x8C)	TR0;
__sbit __at (0x8D)	TF0;
__sbit __at (0x8E)	TR1;
__sbit __at (0x8F)	TF1;

/* TMOD (0x89) Bit Values */
#define	T0_M0_		0x01
#define	T0_M1_		0x02
#define	T0_CT_		0x04
#define	T0_GATE_	0x08
#define	T1_M0_		0x10
#define	T1_M1_		0x20
#define	T1_CT_		0x40
#define	T1_GATE_	0x80

#define T1_MASK_	0xF0
#define T0_MASK_	0x0F

/* CKCON (0x8E) Bit Values */
#define MD0_		0x01
#define MD1_		0x02
#define MD2_		0x04
#define	T0M_		0x08
#define	T1M_		0x10
#define	T2M_		0x20
#define WD0_		0x40
#define WD1_		0x80

/* CSREPR (0x8F) Bit Values */
#define	SW_RST		0x01
#define	SW_RBT		0x02
#define	FARM		0x04
#define	FAES		0x08
#define	PMS		0x10
#define	SCS_100M	0xC0
#define	SCS_75M		0x80
#define	SCS_50M		0x40
#define	SCS_25M		0x00

/* P1 (0x90) Bit Registers */
__sbit __at (0x90)	P1_0;
__sbit __at (0x91)	P1_1;
__sbit __at (0x92)	P1_2;
__sbit __at (0x93)	P1_3;
__sbit __at (0x94)	P1_4;
__sbit __at (0x95)	P1_5;
__sbit __at (0x96)	P1_6;
__sbit __at (0x97)	P1_7;

/* EIF (0x91) Bit Values */
#define INT2F		0x01
#define INT3F		0x02
#define INT4F		0x04
#define INT5F		0x08
#define INT6F		0x10

/* SCON0 (0x98) Bit Registers */
__sbit __at (0x98)	RI0;
__sbit __at (0x99)	TI0;
__sbit __at (0x9A)	RB08;
__sbit __at (0x9B)	TB08;
__sbit __at (0x9C)	REN0;
__sbit __at (0x9D)	SM02;
__sbit __at (0x9E)	SM01;
__sbit __at (0x9F)	SM00;

/* DBAR (0x9A) Bit Values */
#define	BUS_REQ		0x01
#define	BUS_GRANT	0x80

/* ACON (0x9D) Bit Values */
#define	FLAT_MODE	0x02
#define	LARGE_MODE	0x00

/* PISSR1 (0x9E) Bit Values */
#define	LB_INT_STU	0x01
#define	LB_EXT_INT_STU	0x02
#define	ETH_INT_STU	0x04
#define	TOE_INT_STU	0x08
#define	OW_INT_STU	0x10
#define	SPI_INT_STU	0x20
#define	I2C_INT_STU	0x40
#define	CAN_INT_STU	0x80

/* PISSR2 (0x9F) Bit Values */
#define	UART2_INT_STU	0x01

/* P2 (0xA0) Bit Registers */
__sbit __at (0xA0)	P2_0;
__sbit __at (0xA1)	P2_1;
__sbit __at (0xA2)	P2_2;
__sbit __at (0xA3)	P2_3;
__sbit __at (0xA4)	P2_4;
__sbit __at (0xA5)	P2_5;
__sbit __at (0xA6)	P2_6;
__sbit __at (0xA7)	P2_7;

/* IE (0xA8) Bit Registers */
__sbit __at (0xA8)	EX0;	/* 1=Enable External interrupt 0 */
__sbit __at (0xA9)	ET0;	/* 1=Enable Timer 0 interrupt */
__sbit __at (0xAA)	EX1;	/* 1=Enable External interrupt 1 */
__sbit __at (0xAB)	ET1;	/* 1=Enable Timer 1 interrupt */
__sbit __at (0xAC)	ES0;	/* 1=Enable Serial port 0 interrupt */
__sbit __at (0xAD)	ET2;	/* 1=Enable Timer 2 interrupt */
__sbit __at (0xAE)	ES1;	/* 1=Enable Serial port 1 interrupt */
__sbit __at (0xAF)	EA;	/* 0=Disable all interrupts */

/* P3 (0xB0) Bit Registers (Mnemonics & Ports) */
__sbit __at (0xB0)	P3_0;
__sbit __at (0xB1)	P3_1;
__sbit __at (0xB2)	P3_2;
__sbit __at (0xB3)	P3_3;
__sbit __at (0xB4)	P3_4;
__sbit __at (0xB5)	P3_5;
__sbit __at (0xB6)	P3_6;
__sbit __at (0xB7)	P3_7;

__sbit __at (0xB0)	RXD;	/* Serial data input */
__sbit __at (0xB1)	TXD;	/* Serial data output */
__sbit __at (0xB2)	INT0;	/* External interrupt 0 */
__sbit __at (0xB3)	INT1;	/* External interrupt 1 */
__sbit __at (0xB4)	T0;	/* Timer 0 external input */
__sbit __at (0xB5)	T1;	/* Timer 1 external input */
__sbit __at (0xB6)	WR;	/* External data memory write strobe */
__sbit __at (0xB7)	RD;	/* External data memory read strobe */

/* IP (0xB8) Bit Registers */
__sbit __at (0xB8)	PX0;
__sbit __at (0xB9)	PT0;
__sbit __at (0xBA)	PX1;
__sbit __at (0xBB)	PT1;
__sbit __at (0xBC)	PS0;
__sbit __at (0xBD)	PT2;
__sbit __at (0xBE)	PS1;

/* SCON1 (0xC0) Bit Registers */
__sbit __at (0xC0)	RI1;
__sbit __at (0xC1)	TI1;
__sbit __at (0xC2)	RB18;
__sbit __at (0xC3)	TB18;
__sbit __at (0xC4)	REN1;
__sbit __at (0xC5)	SM12;
__sbit __at (0xC6)	SM11;
__sbit __at (0xC7)	SM10;

/* CMOM (0xC2) Bit Values */
#define PCA_ECF		0x01
#define PCA_T0_OF	0x08
#define	PCA_EXT_CLK	0x0E
#define	PCA_GATE_OFF	0x80

/* CCON (0xC3) Bit Values */
#define PCA_CCF0	0x01
#define PCA_CCF1	0x02
#define	PCA_CCF2	0x04
#define	PCA_CCF3	0x08
#define	PCA_CCF4	0x10
#define	PCA_CR		0x40
#define	PCA_CF		0x80

/* T2CON (0xC8) Bit Registers */
__sbit __at (0xC8)	CPRL2;
__sbit __at (0xC9)	CT2;
__sbit __at (0xCA)	TR2;
__sbit __at (0xCB)	EXEN2;
__sbit __at (0xCC)	TCLK;
__sbit __at (0xCD)	RCLK;
__sbit __at (0xCE)	EXF2;
__sbit __at (0xCF)	TF2;

/* PSW (0xD0) Bit Registers */
__sbit __at (0xD0)	P;
__sbit __at (0xD1)	F1;
__sbit __at (0xD2)	OV;
__sbit __at (0xD3)	RS0;
__sbit __at (0xD4)	RS1;
__sbit __at (0xD5)	F0;
__sbit __at (0xD6)	AC;
__sbit __at (0xD7)	CY;

/* CCAPM0-4 (0xD1-0xD5) Bit Values */
#define ECCFs		0x01
#define PWMs		0x02
#define	TOGs		0x04
#define	MATs		0x08
#define	CAPNs		0x10
#define	CAPPs		0x20
#define	ECOMs		0x40
#define	CEXs		0x80

/* PSW (0xD8) Bit Registers */
__sbit __at (0xD8)	RWT;
__sbit __at (0xD9)	EWT;
__sbit __at (0xDA)	WTRF;
__sbit __at (0xDB)	WDIF;

/* Interrupt Vectors: Address = (N * 8) + 3 */
#define IE0_VECTOR	 0	/* 0x03 External Interrupt 0 */
#define TF0_VECTOR	 1	/* 0x0B Timer 0 */
#define IE1_VECTOR	 2	/* 0x13 External Interrupt 1 */
#define TF1_VECTOR	 3	/* 0x1B Timer 1 */
#define SIO0_VECTOR	 4	/* 0x23 Serial port */
#define TF2_VECTOR	 5	/* 0x2B Timer 2 */
#define SIO1_VECTOR	 6	/* 0x33 Serial port */
#define DMA_VECTOR	 7	/* 0x3B Internal DMA Transfer */
#define PCA_VECTOR	 8	/* 0x43 Internal Programmable Counter Array */
#define PIR_VECTOR	 9	/* 0x4B Internal Peripherial IRQ */
#define SDMA_VECTOR	10	/* 0x53 Internal Software DMA */
#define WU_VECTOR	11	/* 0x5B Wake-Up */
#define WDT_VECTOR	12	/* 0x63 Internal Watchdog */

/* UR2_IER (0xE2) Bit Values */
#define	UR2_RDI_ENB	0x01
#define	UR2_THRI_ENB	0x02
#define	UR2_RLSI_ENB	0x04
#define	UR2_MSI_ENB	0x08

/* UR2_IIR (0xE3) Bit Values */
#define	UR2_NONE_INTR		0x01
#define	UR2_RLS_INTR		0x06
#define	UR2_RD_TRIG_INTR	0x04
#define	UR2_RD_TI_INTR		0x0C
#define	UR2_THRE_INTR		0x02
#define	UR2_MS_INTR		0x00

/* UR2_FCR (0xE3) Bit Values */
#define	UR2_FIFO_MODE	0x01
#define	UR2_RXFIFO_RST	0x02
#define	UR2_TXFIFO_RST	0x04
#define	UR2_TRIG_01	0x00
#define	UR2_TRIG_04	0x40
#define	UR2_TRIG_08	0x80
#define	UR2_TRIG_14	0xC0

/* UR2_LCR (0xE4) Bit Values */
#define	UR2_CHAR_5		0x00
#define	UR2_CHAR_6		0x01
#define	UR2_CHAR_7		0x02
#define	UR2_CHAR_8		0x03
#define	UR2_STOP_10		0x00
#define	UR2_STOP_15		0x04
#define	UR2_PARITY_ENB		0x08
#define	UR2_EVEN_PARITY		0x10
#define	UR2_STICK_PARITY	0x20
#define	UR2_BREAK_ENB		0x40
#define	UR2_DLAB_ENB		0x80

/* UR2_MCR (0xE5) Bit Values */
#define	UR2_DTR			0x01
#define	UR2_RTS			0x02
#define	UR2_OUT1		0x04
#define	UR2_OUT2		0x08
#define	UR2_LOOPBACK		0x10
#define	UR2_RS485_RECEIVE	0x40
#define	UR2_RS485_DRIVE		0x80

/* UR2_LSR (0xE6) Bit Values */
#define	UR2_DR		0x01
#define	UR2_OE		0x02
#define	UR2_PE		0x04
#define	UR2_FE		0x08
#define	UR2_BI		0x10
#define	UR2_THRE	0x20
#define	UR2_TEMT	0x40
#define	UR2_FRAME_ERR	0x80

/* UR2_MSR (0xE7) Bit Values */
#define	UR2_DCTS	0x01
#define	UR2_DDSR	0x02
#define	UR2_TERI	0x04
#define	UR2_DDCD	0x08
#define	UR2_CTS		0x10
#define	UR2_DSR		0x20
#define	UR2_RI		0x40
#define	UR2_DCD		0x80

/* EIE (0xE8) Bit Registers */
__sbit __at (0xE8)	EINT2;
__sbit __at (0xE9)	EINT3;
__sbit __at (0xEA)	EINT4;
__sbit __at (0xEB)	EINT5;
__sbit __at (0xEC)	EINT6;
__sbit __at (0xED)	EWDI;

/* EIP (0xF8) Bit Registers */
__sbit __at (0xF8)	PINT2;
__sbit __at (0xF9)	PINT3;
__sbit __at (0xFA)	PINT4;
__sbit __at (0xFB)	PINT5;
__sbit __at (0xFC)	PINT6;
__sbit __at (0xFD)	PWDI;

#endif
