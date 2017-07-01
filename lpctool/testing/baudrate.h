
// serial baudrate
// this is the baudrate at which lpctool communicates with flashtool

//~#define TOOLBAUD 115200UL
#define TOOLBAUD 38400UL	// use this if problems occur
							// USB-UART bridges like FT232 or PL2303
							// have glitches on the handshake lines
							// when the baudrate is changed.
