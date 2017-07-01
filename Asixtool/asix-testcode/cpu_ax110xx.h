#ifndef AX11000_H
#define AX11000_H

#define	GET_SYS_VALUE		1 // 1:get the system globe value information
#define	AX_SWRST_ENB		0 // 1:use software reset function ; 0:not use
#define	AX_SWRBT_ENB		0 // 1:use software reboot function ; 0:not use
#define	STOE_ENABLE		0
#define	AX_WATCHDOG_ENB		0 // 1:involve watchdog function ; 0:not use

/*-------------------------------------------------------------*/
#define	RUNTIME_CODE_START_AT_0H	0	/*	0: runtime code start at 0h */
#define	RUNTIME_CODE_START_AT_24kH	1	/*	1: runtime code start at 24kh. */
#define	RUNTIME_CODE_START_ADDRESS	(RUNTIME_CODE_START_AT_0H)

/* MACRO DECLARATIONS */
#define	AX_DBG_LED(value)	{P1 = value;} // debug LED port, default is P1

/* NAMING CONSTANT DECLARATIONS */
#define	PROG_WTST_0		0
#define	PROG_WTST_1		1
#define	PROG_WTST_2		2
#define	PROG_WTST_3		3
#define	PROG_WTST_4		4
#define	PROG_WTST_5		5
#define	PROG_WTST_6		6
#define	PROG_WTST_7		7
#define	DATA_STRETCH_0		0
#define	DATA_STRETCH_1		1
#define	DATA_STRETCH_2		2
#define	DATA_STRETCH_3		3
#define	DATA_STRETCH_4		4
#define	DATA_STRETCH_5		5
#define	DATA_STRETCH_6		6
#define	DATA_STRETCH_7		7

#define	SHADOW_MEM_ENB		0x10
#define	SYS_CLK_100M		0xC0
#define	SYS_CLK_50M		0x40
#define	SYS_CLK_25M		0x00

#define	WD_INTR_ENABLE		1
#define	WD_INTR_DISABLE		0
#define	WD_RESET_ENABLE		1
#define	WD_RESET_DISABLE	0
#define	WD_HIGH_PRIORITY	1
#define	WD_LOW_PRIORITY		0
#define	WD_INTERVAL_131K	0x00
#define	WD_INTERVAL_1M		WD0_
#define	WD_INTERVAL_8M		WD1_
#define	WD_INTERVAL_67M		WD1_|WD0_

void		AX11000_Init(void);
unsigned char	AX11000_GetSysClk(void);
unsigned char	AX11000_GetMemSdw(void);
unsigned char	AX11000_GetProgWst(void);
unsigned char	AX11000_GetDataSth(void);
void		AX11000_SoftReset(void);
void		AX11000_SoftReboot(void);
void		AX11000_WatchDogSetting(unsigned char wdIntrEnb, unsigned char wdRstEnb, unsigned char wdPrty, unsigned char wdTime);

#endif /* End of AX11000_H */
