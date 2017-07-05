/*
 * bmp2b.c - convert colored 24 bit bmp to the betty display ram format
 *
 * author: hackbard@hackdaworld.org
 * changed: damaltor@gmail.com
 * changelog: edited the first lines of output to fit boop's requirements
 * 		the old line is still there, but commented
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bmp.h"

#define DX	128
#define DY	160
#define PM	(DY/8)

#define BINARY	1
#define CHAR	2
#define SHOW	3

int main(int argc,char **argv) {

	int i,fd;
	t_bmp src,dst;
	char in[128];
	char out[128];
	char blook[128+8];
	unsigned char stat;
	unsigned char buf[2];
	int page,col;
	int b,pix;

	stat=0;
	for(i=1;i<argc;i++) {
		if(argv[i][0]!='-')
			continue;
		switch(argv[i][1]) {
			case 'i':
				strncpy(in,argv[++i],128);
				break;
			case 'o':
				strncpy(out,argv[++i],128);
				break;
			case 'b':
				stat=BINARY;
				break;
			case 'c':
				stat=CHAR;
				break;
			default:
				printf("usage:\n\n");
				printf("%s -i <bitmap> -o <out file> [-b/c]\n",
				       argv[0]);
				printf("\n");
				printf("  -b: binary out\n");
				printf("  -c: char array\n\n");
				return -1;
		}
	}

	/* the bitmap infile */
	bmp_init(&src,1);
	src.mode=READ;
	strncpy(src.file,in,128);
	bmp_read_file(&src);

	/* the bitmap outfile */
	sprintf(blook,"blook_%s",in);
	bmp_init(&dst,1);
	dst.mode=WRITE;
	strncpy(dst.file,blook,128+8);
	dst.width=src.width;
	dst.height=src.height;
	bmp_alloc_map(&dst);

	if((src.width!=DX)|(src.height=!DY)) {
		printf("wrong dimensions: %d %d (need: %d %d)\n",
		       src.width,src.height,DX,DY);
		return -1;
	}

	/* out file */
	fd=open(out,O_WRONLY|O_CREAT);
	if(fd<0) {
		perror("open outfile");
		return fd;
	}

	if(stat==CHAR)
		dprintf(fd,"const unsigned char b_w = %d;\nconst unsigned char b_h = %d;\nconst char b_data[] = {\n",DX,DY);

//		dprintf(fd,"const unsigned char default_logo[%d]={\n",DX*PM*2);
//		was the old function

	for(page=0;page<PM;page++) {
		for(col=0;col<DX;col++) {
			buf[0]=0;
			buf[1]=0;
			for(i=0;i<8;i++) {
				// bmp: bottom rows first ... (i forgot that!)
				pix=((DY-1-(page*8+i))*DX)+col;
				b=src.map[pix].r+src.map[pix].g+src.map[pix].b;
				b/=3;
				if(b<=(0.25*255)) {
					buf[0]|=(1<<i);
					buf[1]|=(1<<i); // 1 1
					dst.map[pix].r=0;
					dst.map[pix].g=0;
					dst.map[pix].b=0;
					continue;
				}
				if(b<=(0.5*255)) {
					buf[0]|=(1<<i); // 1 0
					dst.map[pix].r=0.25*255;
					dst.map[pix].g=0.25*255;
					dst.map[pix].b=0.25*255;
					continue;
				}
				if(b<=(0.75*255)) {
					buf[1]|=(1<<i); // 0 1
					dst.map[pix].r=0.75*255;
					dst.map[pix].g=0.75*255;
					dst.map[pix].b=0.75*255;
					continue;
				}
				// 0 0 .. do nothing!
				dst.map[pix].r=255;
				dst.map[pix].g=255;
				dst.map[pix].b=255;
			}
			if(stat==BINARY) {
				i=write(fd,buf,2);
				if(i<0) {
					perror("bin write");
					return i;
				}
				if(i!=2) {
					printf("write failure\n");
					return -1;
				}
			}
			else if(stat==CHAR) {
				dprintf(fd,"\t0x%02x,0x%02x%c\n",buf[0],buf[1],
				        ((page+1==PM)&(col+1==DX))?' ':',');
			}
		}
	}

	if(stat==CHAR)
		dprintf(fd,"};\n");

	bmp_write_file(&dst);

	close(fd);
	bmp_shutdown(&src);
	bmp_shutdown(&dst);

	return 0;
}

