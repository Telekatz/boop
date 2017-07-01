/* bmp.c -- bmp write/read api
 *
 * author: hackbard@hackdaworld.dyndns.org
 *
 */

#include "bmp.h"

int bmp_init(t_bmp *bmp,int outfd) {

  dprintf(outfd,"[bmp] initializing bmp api ...\n");

  memset(bmp,0,sizeof(t_bmp));
  bmp->outfd=outfd;

  return B_SUCCESS;
}

int bmp_shutdown(t_bmp *bmp) {

  if(bmp->map!=NULL) {
    dprintf(bmp->outfd,"[bmp] free pixmap memory\n");
    free(bmp->map);
  }

  dprintf(bmp->outfd,"[bmp] shutdown\n");

  return B_SUCCESS;
}

int bmp_check_header_and_info(t_bmp *bmp) {

  dprintf(bmp->outfd,"[bmp] magic identifier: %c%c\n",
          bmp->hdr.identifier&0xff,bmp->hdr.identifier>>8);

  if(bmp->info.compression!=0) {
    dprintf(bmp->outfd,"[bmp] compression not supported\n");
    return B_NO_SUPPORT;
  }
  
  if(bmp->info.bpp!=24) {
    dprintf(bmp->outfd,"[bmp] only true color (24bpp) supported\n");
    return B_NO_SUPPORT;
  }
    
  if(bmp->hdr.offset!=BMP_H_SIZE+BMP_I_SIZE) {
    dprintf(bmp->outfd,"[bmp] files with %d bytes offset not supported\n",
            bmp->hdr.offset);
    return B_NO_SUPPORT;
  }

  if(bmp->info.size!=BMP_I_SIZE) {
    dprintf(bmp->outfd,"[bmp] files with %d bytes info size not supported\n",
            bmp->info.size);
    return B_NO_SUPPORT;
  }
    
  return B_SUCCESS;
}

int bmp_alloc_map(t_bmp *bmp) {

  int size;

  size=bmp->width*bmp->height*3;

  dprintf(bmp->outfd,"[bmp] alloc map memory (%d bytes)\n",size);

  if((bmp->map=(t_pixel *)malloc(size))==NULL) {
    dprintf(bmp->outfd,"[bmp] memory map alloc failed\n");
    return B_E_MEM;
  }

  return B_SUCCESS;
}

int bmp_write_file(t_bmp *bmp) {
 
  int fill,xsize,size;
  int y;
  unsigned char buf[3];

  memset(buf,0,3);

  if(!(bmp->mode&WRITE)) {
    dprintf(bmp->outfd,"[bmp] write mode not specified\n");
    return B_WRONG_MODE;
  }

  xsize=bmp->width*3;
  fill=(4-(xsize%4))%4;
  size=(xsize+fill)*bmp->height;

  /* construct it */
  bmp->hdr.identifier='B'|('M'<<8);
  bmp->hdr.size=size+BMP_H_SIZE+BMP_I_SIZE;
  bmp->hdr.offset=BMP_H_SIZE+BMP_I_SIZE;
  bmp->info.size=BMP_I_SIZE;
  bmp->info.width=bmp->width;
  bmp->info.height=bmp->height;
  bmp->info.planes=1;
  bmp->info.bpp=24;
  bmp->info.imagesize=size;
  if(bmp->info.xres==0) bmp->info.xres=2048;
  if(bmp->info.yres==0) bmp->info.yres=2048;
  bmp->info.noc=0;
  bmp->info.ic=0;

  /* write it */
  if((bmp->fd=open(bmp->file,O_WRONLY|O_CREAT))<0) {
    dprintf(bmp->outfd,"[bmp] unable to open file %s\n",bmp->file);
    return B_NO_FILE;
  }

  if(write(bmp->fd,&(bmp->hdr),BMP_H_SIZE)<BMP_H_SIZE) {
    dprintf(bmp->outfd,"[bmp] unable to write bmp header\n");
    return B_E_WRITE_DATA;
  }

  if(write(bmp->fd,&(bmp->info),BMP_I_SIZE)<BMP_I_SIZE) {
    dprintf(bmp->outfd,"[bmp] unable to write bmp info\n");
    return B_E_WRITE_DATA;
  }

  for(y=0;y<bmp->height;y++) {
    if(write(bmp->fd,bmp->map+y*bmp->width,xsize)<xsize) {
      dprintf(bmp->outfd,"[bmp] unable to write image data line %d\n",y);
      return B_E_WRITE_DATA;
    }
    if(write(bmp->fd,buf,fill)<fill) {
      dprintf(bmp->outfd,"[bmp] unable to write fill bytes\n");
      return B_E_WRITE_DATA;
    }
  }

  close(bmp->fd);

  return B_SUCCESS;
}

int bmp_cut_grab_bottom(t_bmp *dst,t_bmp *src,int dz,unsigned char m) {

  int off;

  dst->width=src->width;
  dst->height=dz;

  if(dz>src->height) {
    dprintf(src->outfd,"[bmp] cut region greater than image height\n");
    return B_E_GEOMETRY;
  }

  if(bmp_alloc_map(dst)!=B_SUCCESS) {
    dprintf(dst->outfd,"[bmp] no map memory\n");
    return B_E_MEM;
  }

  off=(m==GRAB)?0:(src->height-dz)*src->width;
  memcpy(dst->map,src->map+off,dz*src->width*sizeof(t_pixel));

  return B_SUCCESS;
}

int bmp_read_file(t_bmp *bmp) {

  unsigned char buf[BMP_H_SIZE+BMP_I_SIZE];
  int y,xsize;
  int crop;

  if(!(bmp->mode&READ)) {
    dprintf(bmp->outfd,"[bmp] read mode not specified\n");
    return B_WRONG_MODE;
  }

  if((bmp->fd=open(bmp->file,O_RDONLY))<0) {
    dprintf(bmp->outfd,"[bmp] unable to open file %s\n",bmp->file);
    return B_NO_FILE;
  }

  if(read(bmp->fd,buf,BMP_H_SIZE+BMP_I_SIZE)<BMP_H_SIZE+BMP_I_SIZE) {
    dprintf(bmp->outfd,"[bmp] error reading bmp header & info\n");
    return B_NO_HI;
  }

  memcpy(&(bmp->hdr),buf,BMP_H_SIZE);
  memcpy(&(bmp->info),buf+BMP_H_SIZE,BMP_I_SIZE);

  if(bmp_check_header_and_info(bmp)!=B_SUCCESS) {
    dprintf(bmp->outfd,"[bmp] header/info check failed\n");
    return B_HI_FAIL;
  }

  bmp->width=bmp->info.width;
  bmp->height=bmp->info.height;

  bmp->map=(t_pixel *)malloc(bmp->width*bmp->height*sizeof(t_pixel));
  if(bmp->map==NULL) {
    dprintf(bmp->outfd,"[bmp] malloc of map memory failed\n");
    return B_E_MEM;
  }

  crop=bmp->info.imagesize/bmp->height-bmp->width*(bmp->info.bpp/8);
  xsize=(bmp->info.bpp/8)*bmp->width;

  for(y=0;y<bmp->height;y++) {
    if(read(bmp->fd,bmp->map+y*bmp->width,xsize)<xsize) {
      dprintf(bmp->outfd,"[bmp] reading image data of line %d failed\n",y);
      return B_E_READ_DATA;
    }
    if(read(bmp->fd,buf,crop)<crop) {
      dprintf(bmp->outfd,"[bmp] failed reading rest of line\n");
      return B_E_READ_DATA;
    }
  }
    
  close(bmp->fd);

  return B_SUCCESS;
}
