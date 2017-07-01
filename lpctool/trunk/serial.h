#ifndef BOOP_SERIAL_H
#define BOOP_SERIAL_H

#define	USRRxData      	(1 << 0)
#define	USRTxHoldEmpty 	(1 << 6)
#define GET_CHAR(p)	p
#define PUT_CHAR(p,c)  	(p= (unsigned )(c))
#define TX_READY(s)    	((s) & USRTxHoldEmpty)
#define RX_DATA(s)     	((s) & USRRxData)

int serial_flush_input(void);
int serial_flush_output(void);
void serial_setbrg (unsigned int baudrate);
int serial_init (void);
void serial_putc (const char c);
int serial_tstc (void);
int serial_getc (void);
void serial_puts (const char *s);

void putHexC(const unsigned char v);
void putHexS(const unsigned short v);
void putHexW(const unsigned long v);

#endif /* BOOP_SERIAL_H */
