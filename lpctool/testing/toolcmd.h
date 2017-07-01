#ifndef TOOLCMD_H
#define TOOLCMD_H

#define HELO_CMD	'.'
#define BULKENABLE_CMD	'B'
#define BULKDISABLE_CMD	'X'
#define SETADR_CMD	'A'
#define FILLBUF_CMD	'F'
#define WRITE_CMD	'W'
#define WRITEGO_CMD	'!'
#define SECTERASE_CMD	'P'
#define FULLERASE_CMD	'E'
#define SETNUMBYTES_CMD	'S'
#define READ_CMD	'R'
#define SKIP_CMD	'>'
#define HELOTOKEN	"HELO"
#define ACKTOKEN	"ACK!"
#define NACKTOKEN	"NACK"
#define BACKTOKEN	"!"

#define MIN_GAP		0x10

typedef struct t_chunk
{
	unsigned long start;
	unsigned long len;
	struct t_chunk *prev;
	struct t_chunk *next;
} t_chunk;

#endif
