/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT for more information. */

/* Routines to rasterize the graphics using the GD library. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <gd.h>
#include "gatypes.h"
#include "gx.h"
#include "gxGD.h"


/* default size of the graph */
#define SX 800
#define SY 600

/* local variables */
static gdImagePtr im=NULL;
static gdPoint xxyy[256];
static gdPoint *xybuf=NULL;

static gaint cdef[256],cnum[256];
static gaint ximg,yimg;
static gaint ixrs,iyrs;
static gaint backbw,gdthck,fflag,xyflag,ccol,xyc=0;
static gaint xsav,ysav;
static gaint xcur,ycur,xycnt=0;


gaint gxGDinit (gadouble xrsize, gadouble yrsize, gaint xin, gaint yin, gaint bwin, char *bgImage) {
gaint i,len;
FILE *bgfile;

  for (i=0; i<256; i++) cdef[i]=0; 
  /* keep track of background color */
  if (bwin<-900) 
    backbw = gxdbkq();   /* default is hardware background color */
  else 
    backbw = bwin;       /* user-specified, 0==black, 1==white */

  /*  Allocate the GD image and set up the scaling for it */
  if (xin<0 || yin<0) {  
    /* user has not specified image size */
    if (xrsize > yrsize) {
      ximg = SX; 
      yimg = (gaint)((yrsize/xrsize)*SX);
    }
    else {
      ximg = (gaint)((xrsize/yrsize)*SX); 
      yimg = SX;
    }
  } 
  else {
    ximg = xin; 
    yimg = yin;
  }
  ixrs = (gaint)(xrsize*1000.0+0.5);
  iyrs = (gaint)(yrsize*1000.0+0.5);


  im = NULL;
  /* handle background PNG picture */
  if (*bgImage) {
    /* Make sure bgImage is a .png -- otherwise return error */
    len = 0;
    while (*(bgImage+len)) len++;
    len = len-4;
    if (len>0) {
      if (*(bgImage+len+1)!='p' || 
	  *(bgImage+len+2)!='n' || 
	  *(bgImage+len+3)!='g' ) {
	if (*(bgImage+len+1)!='P' || 
	    *(bgImage+len+2)!='N' || 
	    *(bgImage+len+3)!='G' ) {
	  return(5);
	} 
      }
    }
    /* open bgImage */
    if ((bgfile=fopen(bgImage,"rb"))) {
      if ((im=gdImageCreateFromPng(bgfile)) != NULL) {
	if (im->sx < ximg || im->sy < yimg) {
	  gdImageDestroy(im);
	  im=NULL;
	}
      } 
      else {
	fclose(bgfile);
	return(7);
      }
      fclose(bgfile);
    } 
    else return(3);
  }

  if (!im) {
    /* im = gdImageCreateTrueColor(ximg,yimg);  For anti-aliasing */
    im = gdImageCreate(ximg,yimg);
  }

  /*  Set up background and foreground colors */
  if (backbw) {
    /* background is white ... */
    cnum[0] = gdImageColorAllocate(im, 255, 255, 255);  /* ... so color 0 is white */
    cnum[1] = gdImageColorAllocate(im, 0, 0, 0);        /*  and color 1 is black */
  } else {
    /* the other way around */
    cnum[0] = gdImageColorAllocate(im, 0, 0, 0);
    cnum[1] = gdImageColorAllocate(im, 255, 255, 255);
  }
  cdef[0] = 1; 
  cdef[1] = 1;

  /* initialize flags and other variables */
  fflag = 0;
  xyflag = 0;
  gdthck = 1;
  ccol = 1;  

  return(0);
}

void gxGDcol (gaint clr) {  /* new color */
struct gxdbquery dbq;
  gxGDflush();
  ccol = clr; 
  if (ccol<0)   ccol=0;   
  if (ccol>255) ccol=15;   /* limit of 256 colors in GD */
  gxdbqcol(clr, &dbq);     /* query the data base for info about this color */
  /* Allocate this color if not done already */
  if (cdef[ccol]==0) {
    cnum[ccol] = gdImageColorAllocate(im, dbq.red, dbq.green, dbq.blue);
    cdef[ccol] = 1;
  }
}

void gxGDwid (gaint wid) {  /* new line thickness */
gaint thck;
  gxGDflush();
  thck = wid;
  gdthck = 1;
  if (thck>5) gdthck = 2;
  if (thck>11) gdthck = 3;
}

void gxGDacol (gaint clr) {  /* new color definition */
  gxGDflush();
  cdef[clr] = 0;   /* Indicate that this new color must be allocated */
}

void gxGDrec (gadouble x1, gadouble x2, gadouble y1, gadouble y2) {  /* filled rectangle */ 
gaint i,xp,yp,xp2,yp2;
gaint ix1,ix2,iy1,iy2;
  gxGDflush();

  ix1  = (gaint)(x1*1000.0+0.5);
  ix2  = (gaint)(x2*1000.0+0.5);
  iy1  = (gaint)(y1*1000.0+0.5);
  iy2  = (gaint)(y2*1000.0+0.5);

  xp = (ximg*ix1)/ixrs;
  yp = yimg-(yimg*iy1)/iyrs;
  xp2 = (ximg*ix2)/ixrs;
  yp2 = yimg-(yimg*iy2)/iyrs;

  if (xp>xp2) { 
    i = xp;
    xp = xp2;
    xp2 = i;
  }
  if (yp>yp2) { 
    i = yp;
    yp = yp2;
    yp2 = i;
  }
  gdImageFilledRectangle(im, xp, yp, xp2, yp2, cnum[ccol]);
}

void gxGDbpoly (void) {  /* start a polygon fill */
  gxGDflush();
  fflag = 1;
  xyc = 0;
}

gaint gxGDepoly (gadouble *xybuffer, gaint xyc) {  /* terminate a polygon fill */
gaint thisx,thisy,i,ptr,ix1,iy1;
gadouble x1,y1;

  /* allocate memory for array of gdPoints */
  xybuf = (gdPoint *)malloc(sizeof(gdPoint)*(xyc+2));
  if (xybuf==NULL) {
    printf("Memory allocation error in gxGDepoly\n");
    return(99);
  }
  /* populate xybuf with gdPoint values */
  ptr=0;
  for (i=0; i<xyc; i++) {
    x1 = *(xybuffer+ptr);
    y1 = *(xybuffer+ptr+1);
    ix1  = (gaint)(x1*1000.0+0.5);
    iy1  = (gaint)(y1*1000.0+0.5);
    thisx = (ximg*ix1)/ixrs;
    thisy = yimg-(yimg*iy1)/iyrs;
    (xybuf+i)->x = thisx;
    (xybuf+i)->y = thisy;
    ptr+=2;
  }
  /* close the polygon if necessary */
  if (xybuf->x != (xybuf+xyc-1)->x ||
      xybuf->y != (xybuf+xyc-1)->y) {
    (xybuf+xyc)->x = xybuf->x;
    (xybuf+xyc)->y = xybuf->y;
    xyc++;
  }
  /* render it */
  gdImageFilledPolygon(im, xybuf, xyc, cnum[ccol]); 
  /* clean up */
  free (xybuf);
  xybuf = NULL;
  fflag = 0;
  return(0);
}

void gxGDmov (gadouble x1, gadouble y1) {  /* move to */ 
gaint ix1,iy1;

  gxGDflush();

  ix1  = (gaint)(x1*1000.0+0.5);
  iy1  = (gaint)(y1*1000.0+0.5);
  xsav = (ximg*ix1)/ixrs;
  ysav = yimg-(yimg*iy1)/iyrs;

  if (!fflag) {
    xxyy[0].x = xsav;
    xxyy[0].y = ysav;
    xycnt = 1;
    xyflag = 0;
  }
}

void gxGDdrw (gadouble x1, gadouble y1) {  /* draw to */  
gaint ix1,iy1;

  ix1  = (gaint)(x1*1000.0+0.5);
  iy1  = (gaint)(y1*1000.0+0.5);
  xcur = (ximg*ix1)/ixrs;
  ycur = yimg-(yimg*iy1)/iyrs;

  if (!fflag) { 
    xxyy[xycnt].x = xcur;
    xxyy[xycnt].y = ycur;
    if (xycnt<255) xycnt++;
    else {
      gxGDpoly(xycnt+1,cnum[ccol],gdthck);
      xxyy[0].x = xcur;
      xxyy[0].y = ycur;
      xycnt = 1; 
    }
    xyflag = 1;
  }
  xsav = xcur; 
  ysav = ycur;
}

void gxGDflush (void) {  /* finish up any drawing operation */
  if (xyflag) { 
    gxGDpoly(xycnt,cnum[ccol],gdthck); 
    xyflag=0; 
    xycnt=0; 
  }
}

/* Turns out that the anti-aliasing doesn't work with
   the line thickness, in gd-v2.  Decided to not use
   the anti-aliasing this version.  But kept the 
   function calls here, commented out, for possible
   future use.  */

void gxGDpoly (gaint xycnt, gaint col, gaint thck) {

  /*     gdImageSetAntiAliased(im,col); */
  gdImageSetThickness(im,thck);

  if (xycnt==2) {
    /*     gdImageLine(im, xxyy[0].x, xxyy[0].y, xxyy[1].x, xxyy[1].y, gdAntiAliased); */
    gdImageLine(im, xxyy[0].x, xxyy[0].y, xxyy[1].x, xxyy[1].y, col);
  }
  else if (xycnt>2) {
    /*    gdImageOpenPolygon(im, xxyy, xycnt, gdAntiAliased); */
    gdImageOpenPolygon(im, xxyy, xycnt, col);
  }
}


gaint gxGDfgimg (char *fgImage) {  /* handle foreground PNG picture */
gaint len;
FILE *fgfile;
gdImagePtr imfg;

  /* Make sure foreground image file is a .png -- otherwise return error */
  len = 0;
  while (*(fgImage+len)) len++;
  len = len-4;
  if (len>0) {
    if (*(fgImage+len+1)!='p' || 
	*(fgImage+len+2)!='n' || 
	*(fgImage+len+3)!='g' ) {
      if (*(fgImage+len+1)!='P' || 
	  *(fgImage+len+2)!='N' || 
	  *(fgImage+len+3)!='G' ) {
	return(6);
      } 
    }
  }
  
  if ((fgfile=fopen(fgImage,"rb"))) {
    if ((imfg=gdImageCreateFromPng(fgfile)) !=NULL) {
      gdImageCopy(im,imfg,0,0,0,0,imfg->sx,imfg->sy);
    }
    else {
      fclose(fgfile);
      return(8);
    }
  } else {
    return(4);
  }
  fclose(fgfile);
  gdImageDestroy(imfg);
  return (0);
}


gaint gxGDend (char *fnout, char *bgImage, gaint fmtflg, gaint tcolor) {
FILE *ofile, *bgfile;
gdImagePtr imbg=NULL;
gaint retcod;

  retcod = 0;

  /* optionally convert a color to transparent */
  if (tcolor != -1 ) {
    if (cdef[tcolor]){
      gdImageColorTransparent(im,cnum[tcolor]);
      printf("Transparent color: #%d\n",tcolor);
    }
  }

  if (*bgImage) {
    if ((bgfile=fopen(bgImage,"rb"))) {
      if ((imbg=gdImageCreateFromPng(bgfile)) !=NULL) {
	gdImageCopy(imbg,im,0,0,0,0,im->sx,im->sy);
      }
    }
    fclose(bgfile);
    gdImageDestroy(im);
    im=imbg;
  }

  ofile = fopen(fnout, "wb");
  if (ofile==NULL) { 
    printf("Open error on %s\n",fnout);
    retcod = 1; 
  } else {
    if (fmtflg==6) {                    /* image output in gif format */
      gdImageGif (im, ofile);
    }
    else if (fmtflg==7) {     	        /* image output in jpg format */
      gdImageJpeg(im, ofile, -1);
    }
    else {                       	/* image output in png format */
      gdImagePng(im, ofile);
    }
    fclose(ofile);
  }

  gdImageDestroy(im);
  im = NULL;
  return (retcod);
}


/* report on configuration */
void gxGDcfg (void) {
  printf("gd-%s ",GD_VERSION_STRING);
}

  
