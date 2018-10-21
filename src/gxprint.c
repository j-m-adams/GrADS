/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* Routines to print the graphics with calls to the Cairo library, needs gxC.c */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "gatypes.h"
#include "gx.h"
#include "gxC.h"

/* local variables */
static gadouble xsize,ysize;   
static gaint rc=0;

/* Report on configuration */
void gxpcfg (void) {
  gxCcfg();
}

/* Check to see if this printing backend supports fonts */
gaint gxpckfont (void) {
  return (1);
}

/* Keep a local copy of real page size */
void gxpbgn (gadouble xsz, gadouble ysz) {
  xsize = xsz;
  ysize = ysz;
}

/* Tell Cairo to initialize a surface for checking string lengths.
   This is only called when in batch mode */

void gxpinit (gadouble xsz, gadouble ysz) {
  gxCbatch (xsz,ysz);
}

/* Tell Cairo to destroy the surface for checking string lengths. */

void gxpend (void) {
  gxCend ();
}


/* Render the hardcopy output. 

    fnout -- output filename
    xin,yin -- image sizes (-999 for non-image formats)
    bwin -- background color
    fmtflg -- output format 
    bgImage, fgImage -- background/foreground image filenames
    tcolor -- transparent color 
*/

gaint gxprint (char *fnout, gaint xin, gaint yin, gaint bwin, gaint fmtflg, 
	       char *bgImage, char *fgImage, gaint tcolor, gadouble border) {

  /* Make sure we don't try to print an unsupported format */
  if (fmtflg!=1 && fmtflg!=2 && fmtflg!=3 && fmtflg!=4 && fmtflg!=5) return (9);

  if (tcolor != -1) gxdbsettransclr (tcolor);   /* tell graphics database about transparent color override */

  /* initialize the output for vector graphics or image */
  rc = gxChinit (xsize,ysize,xin,yin,bwin,fnout,fmtflg,bgImage,border);
  if (rc) return (rc); 

  /* draw the contents of the metabuffer */
  gxhdrw (0,1);
  if (rc) return (rc); 

  /* finish up */
  rc = gxChend (fnout,fmtflg,fgImage);

  gxdbsettransclr (-1);    /* unset transparent color override */
  return (rc);
}

void gxpcol (gaint col) {  /* new color */
  gxCcol (col);
}
void gxpacol (gaint col) {  /* new color definition */
  /* this is a no-op for cairo */
}
void gxpwid (gaint wid) {  /* new line thickness */
  gxCwid (wid);
}
void gxprec (gadouble x1, gadouble x2, gadouble y1, gadouble y2) {  /* filled rectangle */ 
  gxCrec (x1,x2,y1,y2);
}
void gxpbpoly (void) {  /* start a polygon fill */
  gxCbfil(); 
}
gaint gxpepoly (gadouble *xybuf, gaint xyc) {  /* terminate a polygon fill */
  gxCfil (xybuf,xyc);
  return(0);
}
void gxpmov (gadouble xpos, gadouble ypos) {  /* move to */ 
  gxCmov (xpos,ypos);
}
void gxpdrw (gadouble xpos, gadouble ypos) {  /* draw to */  
  gxCdrw (xpos,ypos);
}
void gxpflush (void) { /* finish drawing */
  gxCflush(1);
}
void gxpsignal (gaint sig) { 
  if (sig==1) gxCflush(1);   /* finish drawing */
  if (sig==2) gxCaa(0);      /* disable anti-aliasing */
  if (sig==3) gxCaa(1);      /* enable anti-aliasing */
}
gadouble gxpch (char ch, gaint fn, gadouble x, gadouble y, gadouble w, gadouble h, gadouble rot) { /* draw character */
  return (gxCch (ch, fn, x, y, w, h, rot));
}
gadouble gxpqchl (char ch, gaint fn, gadouble w) {  /* query character length */
 return (gxCqchl (ch, fn, w)); 
}
void gxpclip (gadouble x1, gadouble x2, gadouble y1, gadouble y2) {  /* set clipping area */ 
  gxCclip (x1,x2,y1,y2);
}
