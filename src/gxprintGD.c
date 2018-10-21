/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* Routines to print the graphics with calls to the GD library, needs gxGD.c */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include "gatypes.h"
#include "gxGD.h"
#include "gx.h"

/* local variables */
static gadouble xrsize,yrsize;   
static gaint rc=0;

/* Report on configuration */
void gxpcfg (void) {
  gxGDcfg();
}

/* Check to see if this printing backend supports fonts */
gaint gxpckfont (void) {
  return (0);
}

/* Keep a local copy of real page size. 
   Called from gxstrt() in gxsubs.c */
void gxpbgn (gadouble xsz, gadouble ysz) {
  xrsize = xsz;
  yrsize = ysz;
}


/* render the image output with the GD library */
/* Render the hardcopy output. Called from gacmd() in gauser.c 
     fnout -- output filename
     xin,yin -- image sizes (-999 for non-image formats)
     bwin -- background color
     fmtflg -- output format 
     bgImage, fgImage -- background/foreground image filenames
     tcolor -- transparent color 
     border -- a no-op arg for the GD build
*/
gaint gxprint (char *fnout, gaint xin, gaint yin, gaint bwin, gaint fmtflg, 
	       char *bgImage, char *fgImage, gaint tcolor, gadouble border) {

  /* Make sure we don't try to print an unsupported format */
  if (fmtflg!=5 && fmtflg!=6 && fmtflg!=7) return(9);

  /* initialize the image output */
  rc = gxGDinit(xrsize, yrsize, xin, yin, bwin, bgImage);
  if (rc) return (rc);

  /* draw the contents of the metabuffer */
  gxhdrw (0,1);
  if (rc) return (rc);

  /* Add the foreground image */
  if (*fgImage) rc = gxGDfgimg (fgImage);
  if (rc) return (rc);

  /* finish up */
  rc = gxGDend (fnout, bgImage, fmtflg, tcolor);
  return (rc);
}

void gxpcol (gaint col) {  /* new color */
  gxGDcol (col);
}
void gxpacol (gaint col) {  /* new color definition */
  gxGDacol (col);
}
void gxpwid (gaint wid) {  /* new line thickness */
  gxGDwid (wid);
}
void gxprec (gadouble x1, gadouble x2, gadouble y1, gadouble y2) {  /* filled rectangle */ 
  gxGDrec (x1, x2, y1, y2);
}
void gxpbpoly (void) {  /* start a polygon fill */
  gxGDbpoly ();
}
gaint gxpepoly (gadouble *xybuf, gaint xyc) {  /* terminate a polygon fill */
  rc = gxGDepoly(xybuf,xyc);
  return (rc);
}
void gxpmov (gadouble xpos, gadouble ypos) {  /* move to */ 
  gxGDmov (xpos, ypos);
}
void gxpdrw (gadouble xpos, gadouble ypos) {  /* draw to */  
  gxGDdrw (xpos, ypos);
}
void gxpflush (void) { /* finish drawing */
  gxGDflush();
}
void gxpsignal (gaint sig) { /* finish drawing */
  if (sig==1) gxGDflush();
}

/* these are no-ops in this build */
void gxpinit (gadouble xmx, gadouble ymx) { 
}
void gxpend (void) { 
}
gadouble gxpch (char ch, gaint fn, gadouble x, gadouble y, gadouble w, gadouble h, gadouble rot) {
  return 0;
}
gadouble gxpqchl (char ch, gaint fn, gadouble w) {
  return -999;
}
void gxpclip (gadouble x1, gadouble x2, gadouble y1, gadouble y2) { 
}

