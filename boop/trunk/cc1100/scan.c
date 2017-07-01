//~ DN508 - Frequency Scanning using CC430Fx, CC110x, and CC111xFx 

void scanFreqBands_autocal(void) {
	UINT8 subBand;
	UINT8 i;
	UINT16 channel;
	// 1) Loop through all sub bands
	for (subBand = 0; subBand < NUMBER_OF_SUB_BANDS; subBand++) 
	{
		// 1.1) Set the base freq. for the current sub band. The values for FREQ2, FREQ1, and FREQ0 can be found in
		//		freqSettings[subBand][n], where n = 0, 1, or 2
		// 1.2) Set TEST0 register = 0x0B
		// 1.3) Loop through all channels
		for (channel = 0; channel <= lastChannel[subBand]; channel++ ) 
		{
			UINT8 pktStatus;
			// 1.3.1) Set CHANNR register = channel
			// 1.3.2) Change TEST0 register settings to 0x09 if freq is above 861 MHz
			if (channel == limitTest0Reg[subBand]) 
			{
			// 1.3.2.1) Set TEST0 register = 0x09
			}
			// 1.3.3) Enter RX mode by issuing an SRX strobe command
			// 1.3.4) Wait for radio to enter RX state (can be done by polling the MARCSTATE register)
			// 1.3.5) Wait for RSSI to be valid (See DN505 [7] on how long to wait)
			// 1.3.6) Read the PKTSTATUS register while the radio is in RX state (store it in pktStatus)
			// 1.3.7) Enter IDLE state by issuing an SIDLE strobe command
			// 1.3.8) Check if CS is asserted (use the value obtained in 1.3.6)
			if (pktStatus & 0x40) 
			{
				// CS is asserted
				// 1.3.8.1) Read RSSI value and store it in rssi_dec
				// 1.3.8.2) Calculate RSSI in dBm (rssi_dBm)(offset value found in rssi_offset[subBand])
				// 1.3.8.3) Store the RSSI value and the corresponding channel number
				rssiTable[carrierSenseCounter] = rssi_dBm;
				channelNumber[carrierSenseCounter] = channel;
				carrierSenseCounter++;
			}
		} // End Channel Loop
		// 1.4) Before moving on to the next sub band, scan through the rssiTable to find the highest RSSI value. Store
		//		the RSSI value in highRSSI[subBand] and the corresponding channel number in selectedChannel[subBand]
		for (i = 0; i < carrierSenseCounter; i++) 
		{
			if (rssiTable[i] > highRSSI[subBand]) 
			{
				highRSSI[subBand] = rssiTable[i];
				selectedChannel[subBand] = channelNumber[i];
			}
		}
		// 1.5) Reset carrierSenseCounter
		carrierSenseCounter = 0;
	} // End Band Loop
	// 2) When all sub bands have been scanned, find which sub band has the highest RSSI (Scan the highRSSI[subBand]
	//	table). Store the subBand (0, 1, or 2) and the corresponding channel in the global variables activeBand and
	//	activeChannel respectively
	//~ {
	INT16 tempRssi = -150;
	for (subBand = 0; subBand < NUMBER_OF_SUB_BANDS; subBand++) 
	{
		if (highRSSI[subBand] >= tempRssi) 
		{
			tempRssi = highRSSI[subBand];
			activeChannel = selectedChannel[subBand];
			activeBand = subBand;
		}
	}
}


void scanFreqBands_noautocal(void) {
	UINT8 subBand;
	UINT8 i;
	UINT16 channel;
	// 1) Loop through all sub bands
	for (subBand = 0; subBand < NUMBER_OF_SUB_BANDS; subBand++) 
	{
		// 1.1) Set the base freq. for the current sub band. The values for FREQ2, FREQ1, and FREQ0 can be found in
		//		freqSettings[subBand][n], where n = 0, 1, or 2
		// 1.2) Set TEST0 register = 0x0B
		// 1.3) Reset Calibration Counter (calibration performed when counter is 0)
		calCounter = 0;
		// 1.4) Loop through all channels
		for (channel = 0; channel <= lastChannel[subBand]; channel++ ) 
		{
			UINT8 pktStatus;
			// 1.4.1) Set CHANNR register = channel
			// 1.4.2) Change TEST0 register settings to 0x09 if freq is above 861 MHz. When TEST0 is changed to 0x09, it
			//		is important that FSCAL2 is set to 0x2A and that a new calibration is performed
			if (channel == limitTest0Reg[subBand]) 
			{
				// 1.4.2.1) Set TEST0 register = 0x09
				// 1.4.2.2) Set FSCAL2 register = 0x2A
				// 1.4.2.3) Calibration is needed when TEST0 is changed
				calCounter = 0;
			}
			// 1.4.3) Calibrate for every 5th ch. + at start of every sub band and every time the TEST0 reg. is changed
			if (calCounter++ == 0) 
			{
				// 1.4.3.1) Perform a manual calibration by issuing an SCAL strobe command
			}
			// 1.4.4)) Reset Calibration Counter (if calCounter = 5, we are 1 MHz away from the frequency where a
			//			calibration was performed)
			if (calCounter == 5) 
			{
				// 1.4.4.1) Calibration is performed if calCounter = 0
				calCounter = 0;
			}
			// 1.4.5) Enter RX mode by issuing an SRX strobe command
			// 1.4.6) Wait for radio to enter RX state (can be done by polling the MARCSTATE register)
			// 1.4.7) Wait for RSSI to be valid (See DN505 [7] on how long to wait)
			// 1.4.8) Read the PKTSTATUS register while the radio is in RX state (store it in pktStatus)
			// 1.4.9) Enter IDLE state by issuing an SIDLE strobe command
			// 1.4.10) Check if CS is asserted (use the value obtained in 1.4.8)
			if (pktStatus & 0x40) 
			{ 	// CS is asserted
				// 1.4.10.1) Read RSSI value and store it in rssi_dec
				// 1.4.10.2) Calculate RSSI in dBm (rssi_dBm)(offset value found in rssi_offset[subBand])
				// 1.4.10.3) Store the RSSI value and the corresponding channel number
				rssiTable[carrierSenseCounter] = rssi_dBm;
				channelNumber[carrierSenseCounter] = channel;
				carrierSenseCounter++;
			}
		} // End Channel Loop
		// 1.5) Before moving on to the next sub band, scan through the rssiTable to find the highest RSSI value. Store
		//		the RSSI value in highRSSI[subBand] and the corresponding channel number in selectedChannel[subBand]
		for (i = 0; i < carrierSenseCounter; i++) 
		{
			if (rssiTable[i] > highRSSI[subBand]) 
			{
				highRSSI[subBand] = rssiTable[i];
				selectedChannel[subBand] = channelNumber[i];
			}
		}
		// 1.6) Reset carrierSenseCounter
		carrierSenseCounter = 0;
	} // End Band Loop
	// 2) When all sub bands have been scanned, find which sub band has the highest RSSI (Scan the highRSSI[subBand]
	//		table). Store the subBand (0, 1, or 2) and the corresponding channel in the global variables activeBand and
	//		activeChannel respectively
	//~ {
	INT16 tempRssi = -150;
	for (subBand = 0; subBand < NUMBER_OF_SUB_BANDS; subBand++) 
	{
		if (highRSSI[subBand] >= tempRssi) 
		{
			tempRssi = highRSSI[subBand];
			activeChannel = selectedChannel[subBand];
			activeBand = subBand;
		}
	}
}
