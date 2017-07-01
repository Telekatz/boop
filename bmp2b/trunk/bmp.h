/* bmp.h -- bmp headers */

#ifndef BMP_H
#define BMP_H

/* includes */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* defines */
#define B_SUCCESS 1
#define B_ERROR -1
#define B_WRONG_MODE -2
#define B_NO_FILE -3
#define B_NO_HI -4
#define B_NO_SUPPORT -5
#define B_HI_FAIL -6
#define B_E_MEM -7
#define B_E_READ_DATA -8
#define B_E_WRITE_DATA -9
#define B_E_GEOMETRY -10
#define MAX_CHARS_FILE 128
#define BMP_H_SIZE 14
#define BMP_I_SIZE 40
#define GRAB 'g'

/* bmp specific variables */
typedef struct s_bmp_hdr {
  unsigned short int identifier;
  unsigned int size;
  unsigned short int reserved1;
  unsigned short int reserved2;
  unsigned int offset; /* <- 14 + 40 bytes = 0x36 */
} __attribute__ ((packed)) t_bmp_hdr; /* 14 bytes */

typedef struct s_bmp_info {
  unsigned int size; /* 40 bytes = 0x28 */
  int width;
  int height;
  unsigned short int planes;
  unsigned short int bpp;
  unsigned int compression;
  unsigned int imagesize;
  unsigned int xres;
  unsigned int yres;
  unsigned int noc;
  unsigned int ic;
} __attribute__ ((packed)) t_bmp_info; /* 40 bytes */

typedef struct s_pixel {
  unsigned char b;
  unsigned char g;
  unsigned char r;
} __attribute__ ((packed)) t_pixel;

typedef struct s_bmp {
  int outfd;
  int width;
  int height;
  unsigned char mode;
#define READ (1<<0)
#define WRITE (1<<1)
  char file[MAX_CHARS_FILE];
  int fd;
  t_bmp_hdr hdr;
  t_bmp_info info;
  t_pixel *map;
} t_bmp;

/* function prototypes */
int bmp_init(t_bmp *bmp,int outfd);
int bmp_shutdown(t_bmp *bmp);
int bmp_check_header_and_info(t_bmp *bmp);
int bmp_alloc_map(t_bmp *bmp);
int bmp_write_file(t_bmp *bmp);
int bmp_cut_grab_bottom(t_bmp *dst,t_bmp *src,int dz,unsigned char m);
int bmp_read_file(t_bmp *bmp);

#endif
