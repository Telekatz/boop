//~ DN508 - Frequency Scanning using CC430Fx, CC110x, and CC111xFx


#define NUMBER_OF_SUB_BANDS   3

// Variables used to calculate RSSI
UINT8 rssi_dec;
INT16 rssi_dBm;
UINT8 rssi_offset[NUMBER_OF_SUB_BANDS] = {77, 77, 77};
// Freq. Band	 Range					   Channel
// 0			  779.009766 - 829.997314	 0 - 255	All 0x0B
// 1			  830.196869 - 881.184418	 0 - 255	<- 154 = 0x0B, 155 -> = 0x09
// 2			  881.384369 - 927.972992	 0 - 233	All 0x09
INT16 rssiTable[256];
UINT16 channelNumber[256];
UINT8 carrierSenseCounter = 0; // Counter used to keep track on how many time CS has been asserted in one sub band
							   //(i.e. how many RSSI values are stored for each band)
							   
// Stop Channel in each of the sub bands
UINT8 lastChannel[NUMBER_OF_SUB_BANDS]			= {  255,	 255,	 233};

// Channel number for each of the sub bands where one should change from TEST0 = 0x0B to TEST0 = 0x09
UINT16 limitTest0Reg[NUMBER_OF_SUB_BANDS]		= {  256,	 155,	   0 };

// Initialized to a value lower than the RSSI threshold
INT16 highRSSI[NUMBER_OF_SUB_BANDS]				= { -150,	-150,	-150};

// Initialized to a value greater than the highest channel number
UINT16 selectedChannel[NUMBER_OF_SUB_BANDS]		= {  300,	 300,	 300};
												// {FREQ2,  FREQ1, FREQ0}
UINT8 freqSettings[NUMBER_OF_SUB_BANDS][3]		= {	{0x1D,   0xF6,  0x40},
													{0x1F,   0xEE,  0x3F},
													{0x21,   0xE6,  0x3F}};

UINT8 activeBand;	 // After the scanFreqBands() function has run, this variable will contain the sub band where
					  // the strongest signal was found
UINT16 activeChannel; // After the scanFreqBands() function has run, this variable will contain the channel number
					  // where the strongest signal was found
UINT8 calCounter = 0; // This variable is only used when running the code shown in Figure 3
