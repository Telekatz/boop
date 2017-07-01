/*
    soundcore.c - sound core routines (output, ipol, mixing, ...)
    Copyright (C) 2007  Ch. Klippel <ck@mamalala.net>

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

#include "irq.h"
#include "soundcore.h"
#include "lpc2220.h"

#define AUTO_OFF_TIME	0xf000

static unsigned char bl_val, cmp_val;
unsigned char out1;
unsigned char timeout;
static unsigned int auto_timeout;

struct t_voice voices[MAXVOICES];

//***************************************************************************

typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef unsigned long uint32_t;


#define  FALSE	0
#define  TRUE	1

// CPU clock 16MHz
#define F_CPU	16000000

#define FINT (F_CPU/512)	// 512=256PWM steps / 0.5 step per PWM interrupt
// FINT = 31250Hz @F_CPU16MHz

#define FS (FINT/2)
// FS = 15625Hz @F_CPU16MHz

#define MAXTIME 6000
#define OSCILLATORS 3

struct IIR_filter* filter04_06;

uint16_t freq_coefficient[OSCILLATORS];

uint16_t mstimer=0;

uint16_t noise;

// SID registers

struct Voice
{
	uint16_t	Freq;			// Frequency: FreqLo/FreqHi
	uint16_t	PW;				// PulseWitdht: PW LO/HI only 12 bits used in SID
	uint8_t		ControlReg;		// NOISE,RECTANGLE,SAWTOOTH,TRIANGLE,TEST,RINGMOD,SYNC,GATE
	uint8_t		AttackDecay;	// bit0-3 decay, bit4-7 attack
	uint8_t		SustainRelease;	// bit0-3 release, bit4-7 sustain
};

struct Blocks
{
	struct Voice voice1;
	struct Voice voice2;
	struct Voice voice3;
	uint16_t FC;		// not implemented
	uint8_t RES_Filt;	// partly implemented
	uint8_t Mode_Vol;	// partly implemented
	uint8_t POTX;		// not implemented
	uint8_t POTY;		// not implemented
	uint8_t OSC3_Random;// not implemented
	uint8_t ENV3;		// not implemented
};

#define NUMREGISTERS 29

union Soundprocessor
{
	struct Blocks block;
	uint8_t sidregister[NUMREGISTERS];
} Sid;

// SID voice controll register bits
#define GATE (1<<0)
#define SYNC (1<<1)		// not implemented
#define RINGMOD (1<<2)	// implemented
#define TEST (1<<3)		// not implemented
#define TRIANGLE (1<<4)
#define SAWTOOTH (1<<5)
#define RECTANGLE (1<<6)
#define NOISE (1<<7)

// SID RES/FILT ( reg.23 )
#define FILT1 (1<<0)
#define FILT2 (1<<1)
#define FILT3 (1<<2)
// SID MODE/VOL ( reg.24 )  
#define VOICE3OFF (1<<7)

// attack, decay, release envelope timings
uint16_t AttackRate[16]={2,4,16,24,38,58,68,80,100,250,500,800,1000,3000,5000,8000};
uint16_t DecayReleaseRate[16]={6,24,48,72,114,168,204,240,300,750,1500,2400,3000,9000,15000,24000};

// oszillator waveform arrays 
int8_t wave0[256];
int8_t wave1[256];
int8_t wave2[256];

// envelope arrays

uint8_t envelope[OSCILLATORS];

uint16_t 	m_attack[OSCILLATORS];
uint16_t	m_decay[OSCILLATORS];
uint16_t	m_release[OSCILLATORS];

uint8_t		release_flag[OSCILLATORS];
uint8_t		attackdecay_flag[OSCILLATORS];


int16_t	level_sustain[OSCILLATORS];
//int test[9];
//struct IIR_filter filter04_06;
// = {0,0,0,0, B0, B1, B2, A1, A2};          //initialize filter with 0.4 cutoff frequency

//uint16_t	t_attack[OSCILLATORS];
//uint16_t	t_decay[OSCILLATORS];	
//uint16_t	t_release[OSCILLATORS];	

/************************************************************************

	interrupt routine 
	
	- calculate waverform phases
	- calculate waveforms
	- set PWM output
	- increase system timer  
	
************************************************************************/
uint8_t get_wavenum(struct Voice *voice)
{
	uint8_t n;

	if(voice==&Sid.block.voice1) n=0;
	if(voice==&Sid.block.voice2) n=1;
	if(voice==&Sid.block.voice3) n=2;
	
	return n;
}

void setfreq(struct Voice *voice,uint16_t freq)
{
	uint32_t templong;
	uint8_t n;

	n=get_wavenum(voice);
	
	templong=freq;
	freq_coefficient[n]=templong*65536/FS;
}

void init_waveform(struct Voice *voice)
{
	uint16_t n;
	int16_t val;
	uint8_t wavetype;
	int8_t *wave_array;

	wavetype=voice->ControlReg;
	
	n=get_wavenum(voice);

	if(n==0) wave_array=wave0;
	if(n==1) wave_array=wave1;
	if(n==2) wave_array=wave2;

	val=-128;

	for(n=0;n<256;n++)
	{
		*wave_array=0xFF;

		if(wavetype&SAWTOOTH)  
		{
			*wave_array&=val;
		}

		if(wavetype&TRIANGLE) 
		{
				if(n&0x80) *wave_array&=((n^0xFF)<<1)-128;
				else *wave_array&=(n<<1)-128;
		}

		if(wavetype&RECTANGLE) 
		{
			if(n>(voice->PW >> 4)) // SID has 12Bit pwm, here we use only 8Bit
			{
				*wave_array&=127;
			}
			else *wave_array&=-127;
		}
		
		// if(wavetype&NOISE) *wave_array&=zufall();
		// noise has to be online calculated

		val++;
		wave_array++;
	}
}
//#define MAXLEVEL ( 0xFFFF / OSCILLATORS )
#define MAXLEVEL  19000
#define SUSTAINFAKTOR ( MAXLEVEL / 15 )

void setenvelope(struct Voice *voice)
{
	uint8_t n;
	
	n=get_wavenum(voice);
	attackdecay_flag[n]=TRUE;

//	t_attack[n]=AttackRate[voice->AttackDecay>>4];
//	t_decay[n]=DecayReleaseRate[voice->AttackDecay&0x0F];
//	t_release[n]=DecayReleaseRate[voice->SustainRelease&0x0F];
	level_sustain[n]=(voice->SustainRelease>>4)*SUSTAINFAKTOR;
	
//	m_attack[n]=MAXLEVEL/t_attack[n];
	m_attack[n]=MAXLEVEL/AttackRate[voice->AttackDecay>>4];
//	m_decay[n]=(MAXLEVEL-level_sustain[n]*SUSTAINFAKTOR)/t_decay[n];
	m_decay[n]=(MAXLEVEL-level_sustain[n]*SUSTAINFAKTOR)/DecayReleaseRate[voice->AttackDecay&0x0F];

//	m_release[n]=(level_sustain[n]*SUSTAINFAKTOR)/t_release[n];
	m_release[n]=(level_sustain[n])/DecayReleaseRate[voice->SustainRelease&0x0F];
}
/************************************************************************
	
	uint8_t set_sidregister(uint8_t regnum, uint8_t value)

	The registers of the virtual SID are set by this routine.
	For some registers it is necessary to transform the SID-register
	values to some internal settings of the emulator. 
	To select this registers and to start the calculation, the switch/
	case statement is used.
	For instance: If setting the SID status register, new waveforms in 
	the waveform array are calculated.

	4.2007 ch

************************************************************************/
uint8_t set_sidregister(uint8_t regnum, uint8_t value)
{
	//uint16_t temp;
	uint8_t regnum1;
	
	regnum1=regnum;
	
	if (regnum > 13) regnum++;
	if (regnum > 6 ) regnum++;
	
	if(regnum>NUMREGISTERS-1) return 1;
	Sid.sidregister[regnum]=value;

	switch(regnum1)
	{
		//voice1
		case 1:
		{
			//temp=(Sid.sidregister[0]+(Sid.sidregister[1]<<8))/17;
			//setfreq(&Sid.block.voice1,temp);

			freq_coefficient[0]=(Sid.sidregister[0]+(Sid.sidregister[1]<<8))/4;

		}break;
		case 3: init_waveform(&Sid.block.voice1);break;
		case 4: init_waveform(&Sid.block.voice1);break;
		case 5: setenvelope(&Sid.block.voice1);break;
		case 6: setenvelope(&Sid.block.voice1);break;
		
		//voice2
		case 8:
		{
			//temp=(Sid.sidregister[7]+(Sid.sidregister[8]<<8))/17;
			//setfreq(&Sid.block.voice2,temp);
			freq_coefficient[1]=(Sid.sidregister[7]+(Sid.sidregister[8]<<8))/4;
		}break;
		case 10: init_waveform(&Sid.block.voice2);break;
		case 11: init_waveform(&Sid.block.voice2);break;
		case 12: setenvelope(&Sid.block.voice2);break;
		case 13: setenvelope(&Sid.block.voice2);break;		
		
		//voice3
		case 15:
		{
			//temp=(Sid.sidregister[14]+(Sid.sidregister[15]<<8))/17;
			//setfreq(&Sid.block.voice3,temp);
			freq_coefficient[2]=(Sid.sidregister[14]+(Sid.sidregister[15]<<8))/4;
		}break;
		case 17: init_waveform(&Sid.block.voice3);break;
		case 18: init_waveform(&Sid.block.voice3);break;
		case 19: setenvelope(&Sid.block.voice3);break;
		case 20: setenvelope(&Sid.block.voice3);break;			
	}	
	return 0;
}


void startPWMIRQ(void)
{
		timeout = 0;
	auto_timeout = 0;
	out1 = 0;

	PWMTC = 0;
//	PWMPR = 0;
	PWMPR = 0;
	PWMMR0 = 0x1FF;	// pwm rate
	PWMMR2 = 0x00;	// pwm value
	PWMLER = 0x05;
	PWMMCR = 0x03;
	PWMPCR = (1<<10);
	PWMTCR = 0x03;
	PWMTCR = 0x09;

	VICVectAddr0 = (unsigned long)&(soundIRQ);
	VICVectCntl0 = VIC_SLOT_EN | INT_SRC_PWM;
	//VICIntSelect |= INT_PWM;
	VICIntEnable = INT_PWM;
	
}


void setBL(unsigned char level)
{
	bl_val = level;
}

unsigned char getBL(void)
{
	return bl_val;
}



void  __attribute__ ((interrupt("IRQ")))  soundIRQ (void)
{

	PWMIR = 0x01;
	
	static uint16_t phase0,phase1,phase2;
	static int16_t temp,temp1;
	static uint8_t k,flag=0;
	static uint8_t timer;
	static uint16_t noise8;
	static uint16_t sig0,sig1,sig2;
	static uint16_t tempphase;



	//DEBUGPIN_ON; // Pin to measure processing time

	flag^=1;
	if(flag)
	{
		//DEBUGPIN_ON; // Pin to measure processing time
		// Voice1
		tempphase=phase0+freq_coefficient[0]; //0.88us
		if(Sid.block.voice1.ControlReg&NOISE)
		{				
			if((tempphase^phase0)&0x4000) sig0=noise8*envelope[0];			
		}
		else
		{
			if(Sid.block.voice1.ControlReg&RINGMOD)
			{				
				if(phase2&0x8000) sig0=envelope[0]*-wave0[phase0>>8];
				else sig0=envelope[0]*wave0[phase0>>8];
			}
			else sig0=envelope[0]*wave0[phase0>>8]; //2.07us
		}
		phase0=tempphase;

		// Voice2
		tempphase=phase1+freq_coefficient[1]; //0.88us
		if(Sid.block.voice2.ControlReg&NOISE)
		{				
			if((tempphase^phase1)&0x4000) sig1=noise8*envelope[1];
		}
		else
		{
			if(Sid.block.voice2.ControlReg&RINGMOD)
			{				
				if(phase0&0x8000) sig1=envelope[1]*-wave1[phase1>>8];
				else sig1=envelope[1]*wave1[phase1>>8];
			}
			else sig1=0xff*wave1[phase1>>8]; //2.07us
		}
		phase1=tempphase;

		// Voice3
		tempphase=phase2+freq_coefficient[2]; //0.88us
		if(Sid.block.voice3.ControlReg&NOISE)
		{				
			if((tempphase^phase2)&0x4000) sig2=noise8*envelope[2];
		}
		else
		{
			if(Sid.block.voice3.ControlReg&RINGMOD)
			{				
				if(phase1&0x8000) sig2=envelope[2]*-wave2[phase2>>8];
				else sig2=envelope[2]*wave2[phase2>>8];
			}
			else sig2=envelope[2]*wave2[phase2>>8]; //2.07us
		}
		phase2=tempphase;
	}
	else
	{		

		//		temp=sig0+sig1+sig2;
		
		// voice filter selection
		temp=0; // direct output variable
		temp1=0; // filter output variable
		if(Sid.block.RES_Filt&FILT1) temp1+=sig0;
		else temp+=sig0;
		if(Sid.block.RES_Filt&FILT2) temp1+=sig1;
		else temp+=sig1;
		if(Sid.block.RES_Filt&FILT3) temp1+=sig2;
		else if(!(Sid.block.Mode_Vol&VOICE3OFF))temp+=sig2; // voice 3 with specal turn off bit
	
		//filterOutput = IIR2((struct IIR_filter*)&filter04_06, filterInput);
		//IIR2(filter04_06, temp1);
		k=(temp>>8)+128;
		k+=temp1>>10; // not real filter implemeted yet
		
		//OCR1A=k; // Output to PWM
		PWMMR2 = k; 
		PWMLER = 0x04;
		
		//DEBUGPIN_ON; // Pin to measure processing time
		
		// noise generator
		for(k=1;k<2;k++)
		{
			temp1 = noise;
			noise=noise << 1;
	
			temp1 ^= noise;
			if ( ( temp1 & 0x4000 ) == 0x4000 ) 
			{ 
				noise |= 1;
			}
		}
		noise8=noise>>6;
#ifdef NOISEOFF
		noise8=0;
#endif
	}

	timer--;
	if(timer==0) 
	{
		mstimer++; // increase millisecond timer
		// mstimerrate=0.992 milli seconds @FCPU16MHz 
		timer=31;
	}


	VICVectAddr = 0;
}



// SID Registers
#define VOICE1	0
#define VOICE2	7
#define VOICE3	14
#define CONTROLREG 4
#define ATTACKDECAY 5
#define SUSTAINRELEASE 6

// SID Control Register Bits
#define GATE (1<<0)
#define GATEOFF 0
#define SYNC (1<<1)		// not implemented
#define RINGMOD (1<<2)	
#define TEST (1<<3)		// not implemented
#define TRIANGLE (1<<4)
#define SAWTOOTH (1<<5)
#define RECTANGLE (1<<6)
#define NOISE (1<<7)

// notes VOICE1
#define C1_1	1+VOICE1,0x02,0+VOICE1,0x25,
#define C2_1	1+VOICE1,0x04,0+VOICE1,0x49,
#define C3_1	1+VOICE1,0x08,0+VOICE1,0x93,
#define C4_1	1,0x11,0,0x25,
#define	D4_1	1,0x13,0,0x3F,			
#define	E4_1	1,0x15,0,0x9A,			
#define F4_1	1,0x16,0,0xE3,			
#define	G4_1	1,0x19,0,0x81,			
#define	A4_1	1,0x1C,0,0xC6,			
#define	B4_1	1,0x20,0,0x5E,			
#define	C5_1	1,0x22,0,0x4B,			

// notes VOICE3
#define C2_2	1+VOICE2,0x04,0+VOICE2,0x49,
#define C4_2	1+VOICE2,0x11,0+VOICE2,0x25,
#define	D4_2	1+VOICE2,0x13,0+VOICE2,0x3F,
#define	E4_2	1+VOICE2,0x15,0+VOICE2,0x9A,

// notes VOICE3
#define C2_3	1+VOICE3,0x04,0+VOICE3,0x49,
#define C4_3	1+VOICE3,0x11,0+VOICE3,0x25,
#define	D4_3	1+VOICE3,0x13,0+VOICE3,0x3F,			
#define	E4_3	1+VOICE3,0x15,0+VOICE3,0x9A,			
#define F4_3	1+VOICE3,0x16,0+VOICE3,0xE3,			
#define	G4_3	1+VOICE3,0x19,0+VOICE3,0x81,			
#define	A4_3	1+VOICE3,0x1C,0+VOICE3,0xC6,			
#define	B4_3	1+VOICE3,0x20,0+VOICE3,0x5E,			
#define	C5_3	1+VOICE3,0x22,0+VOICE3,0x4B,

// waveforms
#define SETNOISE_1 		4,0x81,5,0xBB,6,0xAD,
#define SETNOISE_2 		11,0x81,12,0xBB,13,0xAD,
#define SETNOISE_3 		18,0x81,19,0xBB,20,0xAD,
#define SETTRIANGLE_1	4,0x11,5,0xBB,6,0xAA,
#define SETTRIANGLE_2	4+VOICE2,0x11,5+VOICE2,0xBB,6+VOICE2,0xAA,
#define SETTRIANGLE_3	4+VOICE3,0x11,5+VOICE3,0xBB,6+VOICE3,0xAA,
#define SETRECTANGLE_1	VOICE1+CONTROLREG,RECTANGLE+GATE,5+VOICE1,0xBB,6+VOICE1,0xAA,

#define SLOWATTACK 0xB0
#define FASTATTACK 0x00
#define SLOWDECAY 0x0A
#define FASTDECAY 0x00
#define SUSTAINQUITE 0x00
#define SUSTAINNORM	0xA0
#define SUSTAINLOUD 0xF0
#define SLOWRELEASE 0x09
#define FASTRELEASE 0x00


// pause controll, no SID registers, values will be interpreted by the EMULATOR 
#define QUARTER	0xF0,0x00,0xF1,0x01, // pause quarter tone
#define HALF	0xF0,0x00,0xF1,0x02, // pause half tone
#define ONESEC	0xF0,0x00,0xF1,0x04, // pause 2 seconds
#define TWOSEC	0xF0,0x00,0xF1,0x08, // pause 2 seconds
#define FIVESEC	0xF0,0x00,0xF1,40, // pause 2 seconds
#define STOP	0xFF,0xFF, // stops sound generation

/****************************************************************************
	
	static uint8_t sound[]

	Sound Data Array

	The main programm parses the sound data array. 
	It reads the register address and data from the array
	and writes it to the "virtual SID registers".
	0xF0,0xF1 and 0xFF are no SID registers and interpreted as
	controll code for the parser. F0+256*F1 gives the value in milliseconds 
	to pause parsing and let the SID playing it's sound.
	After the pause the next register values are read and written to the SID.
	At the end of the array ther should be the values 0xFF,0xFF which stop 
	the parser and stop sound generation.	 
	
	syntax:

static uint8_t sound[] PROGMEM ={
	sidregister,value,
	sidregister,value,
	....
	0xF0,pauselowvalue,
	0xF1,pausehighvalue,
	....
	sidregister,value,
	sidregister,value,
	0xF0,pauselowvalue,
	0xF1,pausehighvalue,
	0xFF,0xFF			// stop command, turn sound off
};

	April 2007 (c) Christoph Haberer, christoph(at)roboterclub-freiburg.de

****************************************************************************/
static uint8_t sound[] = {

/*************************************************************
			attack,decay,sustain,release example
*************************************************************/


			SETTRIANGLE_1	// set triangle waveform
			
			ATTACKDECAY+VOICE1,SLOWATTACK+SLOWDECAY, 
			SUSTAINRELEASE+VOICE1,SUSTAINNORM+SLOWRELEASE,

			
			//ATTACKDECAY+VOICE1,0xAA, 
			//SUSTAINRELEASE+VOICE1,0x1A,

			//ATTACKDECAY+VOICE1,FASTATTACK+5, 
			//ATTACKDECAY+VOICE1,0xA, 
			//SUSTAINRELEASE+VOICE1,0x8A,
			//SUSTAINRELEASE+VOICE1,SUSTAINQUITE+FASTRELEASE,

			C4_1	// note C4 one channel1 ( voice1 )
			TWOSEC	// wait 2 seconds
			//FIVESEC

			VOICE1+CONTROLREG,TRIANGLE+GATEOFF, // gate off to start release cycle

			TWOSEC	// wait 2 seconds
			
			//STOP

/*************************************************************
			tone latter
*************************************************************/

			SETTRIANGLE_1
			C4_1
			HALF
			D4_1
			HALF
			E4_1
			HALF
			F4_1
			HALF
			G4_1
			HALF
			A4_1
			HALF
			B4_1
			HALF
			C5_1
			HALF

			TWOSEC

/*************************************************************
			3 tone example
*************************************************************/

			SETTRIANGLE_1
			C4_1
			HALF
			SETTRIANGLE_2
			E4_2
			HALF
			SETTRIANGLE_3
			G4_3
			HALF

			TWOSEC
			
			// start release cycle
			VOICE1+CONTROLREG,TRIANGLE+GATEOFF,
			VOICE2+CONTROLREG,TRIANGLE+GATEOFF,
			VOICE3+CONTROLREG,TRIANGLE+GATEOFF,

			TWOSEC
			TWOSEC

/*************************************************************
			PWM example
*************************************************************/
			
			SETRECTANGLE_1
			C4_1

			2,0x00,	// set PWM value
			3,0x04,	// set PWM value
			ONESEC
			
			2,0x00,	// set PWM value
			3,0x06,	// set PWM value
			ONESEC

			2,0x00,	// set PWM value
			3,0x08,	// set PWM value
			TWOSEC

/*************************************************************
			ring modulation example
*************************************************************/

			SETTRIANGLE_1
			F4_1

			6,0x8B, // sustain/release

			SETTRIANGLE_3
			C4_3
			6+VOICE3,0x08, // sustain/release

			VOICE1+CONTROLREG,0x14,	// set ringmod, gate off for decay
			VOICE3+CONTROLREG,0x00,	// gate off
			
			TWOSEC
			TWOSEC

/*************************************************************
			noise example
*************************************************************/

			// shotnoise
			1,0x40,
			0,0x00,
			SETNOISE_1
			ATTACKDECAY+VOICE1,FASTATTACK+5, 
			SUSTAINRELEASE+VOICE1,SUSTAINQUITE+FASTRELEASE,
			QUARTER

			// shotnoise
			1,0x40,
			0,0x00,
			SETNOISE_1
			ATTACKDECAY+VOICE1,FASTATTACK+5, 
			SUSTAINRELEASE+VOICE1,SUSTAINQUITE+FASTRELEASE,
			QUARTER

			// shotnoise
			1,0x40,
			0,0x00,
			SETNOISE_1
			ATTACKDECAY+VOICE1,FASTATTACK+5, 
			SUSTAINRELEASE+VOICE1,SUSTAINQUITE+FASTRELEASE,
			QUARTER
			
			VOICE1+CONTROLREG,NOISE+GATEOFF,

			TWOSEC

			// light noise
			SETNOISE_1
			1,0x40,
			0,0x00,	
			
			ATTACKDECAY+VOICE1,SLOWATTACK+SLOWDECAY, 
			SUSTAINRELEASE+VOICE1,SUSTAINNORM+0x0B,
			TWOSEC

			VOICE1+CONTROLREG,NOISE+GATEOFF,
			TWOSEC

			// dark noise
			SETNOISE_1
			1,0x05,
			0,0x00,		
			TWOSEC

/*************************************************************
			3 tone example
*************************************************************/

			SETTRIANGLE_1
			C4_1
			HALF
			SETTRIANGLE_2
			E4_2
			HALF
			SETTRIANGLE_3
			G4_3
			HALF

			TWOSEC
			
			// start release cycle
			VOICE1+CONTROLREG,TRIANGLE+GATEOFF,
			VOICE2+CONTROLREG,TRIANGLE+GATEOFF,
			VOICE3+CONTROLREG,TRIANGLE+GATEOFF,

			TWOSEC
			TWOSEC


/*************************************************************
			3 tone gong
*************************************************************/
/*
			SETTRIANGLE_1
			ATTACKDECAY+VOICE1,FASTATTACK+SLOWDECAY, 
			SUSTAINRELEASE+VOICE1,1+SLOWRELEASE,
			C4_1
			HALF

			SETTRIANGLE_2
			ATTACKDECAY+VOICE2,FASTATTACK+SLOWDECAY, 
			SUSTAINRELEASE+VOICE2,1+SLOWRELEASE,
			D4_2
			HALF

			SETTRIANGLE_3
			ATTACKDECAY+VOICE3,FASTATTACK+SLOWDECAY, 
			SUSTAINRELEASE+VOICE3,1+SLOWRELEASE,
			E4_3
			HALF

			TWOSEC

			// start release cycle
			VOICE1+CONTROLREG,TRIANGLE+GATEOFF,
			VOICE2+CONTROLREG,TRIANGLE+GATEOFF,
			VOICE3+CONTROLREG,TRIANGLE+GATEOFF,
*/			
/*************************************************************/
			STOP
	};


void playSID(void) {

	int16_t amp[OSCILLATORS];
	uint16_t schedule_timer,temp;
	uint8_t n,flag;
	uint8_t controll_regadr[3];
	uint16_t soundindex;
	
	soundindex = 0;
	schedule_timer=0;
	noise = 0xaa;
	
	controll_regadr[0]  = 4; 
	controll_regadr[1]  = 12;
	controll_regadr[2]  = 20;

//	struct Voice voice;

	attackdecay_flag[0]=TRUE;
	attackdecay_flag[1]=TRUE;
	attackdecay_flag[2]=TRUE;
		
	//initialize SID-registers
	
	Sid.sidregister[6]=0xF0;
	Sid.sidregister[14]=0xF0;
	Sid.sidregister[22]=0xF0;
	setenvelope(&Sid.block.voice1);
	setenvelope(&Sid.block.voice2);
	setenvelope(&Sid.block.voice3);
	
	// set all amplitudes to zero
	for(n=0;n<OSCILLATORS;n++) amp[n]=0;

	while(1)
	{
		//DEBUGPIN_ON; // Pin to measure processing time
		
		// calculate envelopes
		// if gate is ONE then the attack,decay,sustain cycle begins
		// if gate switches to zero the sound decays
		for(n=0;n<OSCILLATORS;n++)
		{
			if(Sid.sidregister[controll_regadr[n]]&GATE) // if gate set then attack,decay,sustain
			{
				if(attackdecay_flag[n]) 
				{	// if attack cycle
					amp[n]+=m_attack[n];
					if(amp[n]>MAXLEVEL)
					{
						amp[n]=MAXLEVEL;
						attackdecay_flag[n]=FALSE; // if level reached, then switch to decay
					}
				}
				else // decay cycle
				{
					if(amp[n]>level_sustain[n])
					{
						amp[n]-=m_decay[n];
						if(amp[n]<level_sustain[n]) amp[n]=level_sustain[n];
					}

				} 
			}
			else // if gate flag is not set then release
			{
				attackdecay_flag[n]=TRUE; // at next attack/decay cycle start wiht attack
				if(amp[n]>0)
				{
					amp[n]-=m_release[n];
					if(amp[n]<0) amp[n]=0;
				}			
			}
			envelope[n]=amp[n]>>8;
		}
		//DEBUGPIN_OFF; // Pin to measure processing time
		
		//DEBUGPIN_ON; // Pin to measure processing time
		if(schedule_timer==0)
		{
//			schedule_timer=MAXTIME;
			
			flag=TRUE;
			while(flag)
			{
				n=sound[soundindex];
				soundindex++;
				
				if(n<NUMREGISTERS)
				{
					set_sidregister(n,sound[soundindex]);
					soundindex++;
				}
				else 
				{
					if(n==0xF0) // get lower part of pause to next tone
					{
						schedule_timer=sound[soundindex];
						soundindex++;
						flag=TRUE;	
					}
					if(n==0xF1) // get higher part of pause to next tone
					{
						schedule_timer|=sound[soundindex]<<8;
						soundindex++;
						flag=FALSE;	
					}
					if(n==0xF2) // replay command
					{
						soundindex=0;
					}
					if(n==0xF3) // one byte delay comand
					{
						schedule_timer=sound[soundindex];
						soundindex++;
						flag=FALSE;	
					}
					flag=FALSE;
					if(n==0xFF) 
					{
							return; // Stop Programm execution
					}
				}
			}
			

		}
		else schedule_timer--;		
//		DEBUGPIN_OFF; // Pin to measure processing time

		//DEBUGPIN_ON; // Pin to measure remaining processing time
		while(temp==mstimer); // wait for millisecond multiple
		temp=mstimer;
		//DEBUGPIN_OFF;
	}
}

