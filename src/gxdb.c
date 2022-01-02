/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT for more information. */

/* Keep track of persistent settings for "device backend" attributes, such as 
   settings for custom colors, line widths, fonts, patterns, etc.  This interface
   provides routines to set the values and to query the values. 
   If support is ever added for writing the meta buffer out to a file, 
   the information here should be written to the beginning of the file.  */


#ifdef HAVE_CONFIG_H
#include "config.h"

/* If autoconfed, only include malloc.h when it's present */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#else /* undef HAVE_CONFIG_H */

#include <malloc.h>

#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include "gatypes.h"
#include "gx.h"


static gaint pdcred[16] = {  0,255,250,  0, 30,  0,240,230,240,160,160,  0,230,  0,130,170};
static gaint pdcgre[16] = {  0,255, 60,220, 60,200,  0,220,130,  0,230,160,175,210,  0,170};
static gaint pdcblu[16] = {  0,255, 60,  0,255,200,130, 50, 40,200, 50,255, 45,140,220,170};

static gadouble pdcwid[12] = {0.6, 0.8, 1.0, 1.25, 1.5, 1.75, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0};

/* basic colors.  legacy backends will only support the first 256 */
static gaint reds[COLORMAX], greens[COLORMAX], blues[COLORMAX], alphas[COLORMAX];
static gaint tilenum[COLORMAX];

/* tile patterns.  these can include solid colors with transparency.  */
static gaint ptypes[COLORMAX], pxsz[COLORMAX], pysz[COLORMAX];
static gaint pfthick[COLORMAX],pfcols[COLORMAX],pbcols[COLORMAX];
static char *pnames[COLORMAX];

/* custom line widths.  */

static gadouble widths[256];

/* custom font info */

static char *fontname[100];
static char *fn_serif = "serif";
static char *fn_sans = "sans-serif";
static gaint fnbold[100];
static gaint fnitalic[100];
static gaint hershflag;                /* For fn 1 to 6, use Hershey fonts or not */
static gaint dbdevbck;                 /* Device background color */
static gaint dboutbck;                 /* Ouput (image or hardcopy) background color */
static gaint dbtransclr;               /* transparent color number (for hardcopy) */
static struct gxdsubs *dsubs=NULL;     /* function pointers for display */


/* Initialize all the device backend persitent info on startup.
   The "basic colors" (0 to 15) are pre-defined
   Line widths (0 to 11) are also pre-defined. */

void gxdbinit () {
gaint i;

  /* Initialize colors (default and user-defined) */
  for (i=0; i<COLORMAX; i++) {
    reds[i]   = 150; 
    greens[i] = 150;
    blues[i]  = 150;
    alphas[i] = 255;
    tilenum[i] = -999;
    ptypes[i] = -999;
    pnames[i] = NULL;
    pxsz[i] = 10; 
    pysz[i] = 10;
    pfthick[i] = 3; 
    pfcols[i] = -999; 
    pbcols[i] = -999;
  }
  for (i=0; i<16; i++) {
    reds[i]   = pdcred[i];
    greens[i] = pdcgre[i];
    blues[i]  = pdcblu[i];
    alphas[i] = 255;
  }

  /* initialize line widths (default and user-defined) */
  for (i=0; i<256; i++) widths[i] = 1.0; 
  for (i=0; i<12; i++) widths[i] = pdcwid[i]; 

  /* Initialize font settings */
  for (i=0; i<100; i++) {
    fontname[i] = NULL;
    fnbold[i] = 0;
    fnitalic[i] = 0;
  }

  /* these will be for emulations of hershey fonts 0-5, but not 3 */
  fontname[0] = fn_sans;   fnbold[0] = 0;  fnitalic[0] = 0; 
  fontname[1] = fn_serif;  fnbold[1] = 0;  fnitalic[1] = 0;
  fontname[2] = fn_sans;  fnbold[2] = 0;  fnitalic[2] = 1;
  fontname[4] = fn_sans;   fnbold[4] = 1;  fnitalic[4] = 0;
  fontname[5] = fn_serif;  fnbold[5] = 1;  fnitalic[5] = 0;

  /* other flags for this and that */
  hershflag  = 0;     /* zero, use hershey fonts.  1, use emulation. */  
  dbdevbck   = 0;     /* initial device background color is black */
  dboutbck   = -1;    /* initial output background color is 'undefined' */
  dbtransclr = -1;    /* initial transparent color is 'undefined' */
   
}

/* Set a font name for a font from 10 to 99.  
   Use of a font number that has no defined font family name will be handled
   by the device backend, usually by using a default sans-serif font. */

void gxdbsetfn (gaint fn, char *str) {
gaint len,i;
char *newname;

  if (fontname[fn]) {
    /* this font number has been previously assigned. Reset and free memory */
    free (fontname[fn]);
    fontname[fn] = NULL;
    fnbold[fn] = 0;
    fnitalic[fn] = 0;
  }
  if (str) {
    len = 0;
    while (*(str+len)) len++;
    newname = (char *)malloc(len+1);
    if (newname==NULL) return;
    for (i=0; i<len; i++) *(newname+i) = *(str+i);
    *(newname+len) = '\0';
    fontname[fn] = newname;
  }
  /* No 'else' statement here ... we can allow a font name to be NULL */
}

/* Query font settings */

void gxdbqfont (gaint fn, struct gxdbquery *pdbq) {
  if (fn<0 || fn>99) {
    pdbq->fname   = fontname[0];
    pdbq->fbold   = fnbold[0];
    pdbq->fitalic = fnitalic[0];
  } else {
    pdbq->fname   = fontname[fn];
    pdbq->fbold   = fnbold[fn];
    pdbq->fitalic = fnitalic[fn];
  }
}

/* Query a color */

void gxdbqcol (gaint colr, struct gxdbquery *pdbq) {
  if (colr<0 || colr>=COLORMAX) {
    pdbq->red = 150;
    pdbq->green = 150;
    pdbq->blue = 150;
    pdbq->alpha = 255;
    pdbq->tile = -999;
  } else {
    pdbq->red = reds[colr];
    pdbq->green = greens[colr];
    pdbq->blue = blues[colr];
    pdbq->alpha = alphas[colr];
    pdbq->tile = tilenum[colr];
  }
}

/* Define a new color */

gaint gxdbacol (gaint clr, gaint red, gaint green, gaint blue, gaint alpha) {
  if (clr<0 || clr>=COLORMAX ) return(1); 
  if (red == -9) {
    /* this is a pattern */
    reds[clr] = red; 
    greens[clr] = green; 
    blues[clr] = blue;  
    alphas[clr] = 255;
    tilenum[clr] = green;
  } else {
    /* this is a color */
    reds[clr] = red;
    greens[clr] = green;
    blues[clr] = blue;
    alphas[clr] = alpha;
    tilenum[clr] = -999;
  }
  return 0;
}

/* Query line width */

void gxdbqwid (gaint lwid, struct gxdbquery *pdbq) {    
  if (lwid<0 || lwid>255) pdbq->wid = 1.0; 
  else pdbq->wid = widths[lwid];
}

/* Set line width */

void gxdbsetwid (gaint lwid, gadouble val) {    
  if (lwid>=1 && lwid<=256) {
    if (val>0) widths[lwid-1] = val;
  }
}

/* Set pattern settings */

void gxdbsetpatt (gaint *itt, char *str) {    
gaint pnum,len,i;
char *newname;

  pnum = *itt;
  if (pnum<0 || pnum>=COLORMAX) return;

  /* trigger a pattern reset in the hardware layer */
  if (dsubs==NULL) dsubs = getdsubs();  /* get ptrs to the graphics display functions */
  dsubs->gxsetpatt(pnum);                        

  if (pnames[pnum]) {
    /* reset tile filename and free memory */
    free (pnames[pnum]);
    pnames[pnum] = NULL;
  }

  /* reset all elements */
  ptypes[pnum]  = -999;
  pxsz[pnum]    = -999; 
  pysz[pnum]    = -999;
  pfthick[pnum] = -999; 
  pfcols[pnum]  = -999; 
  pbcols[pnum]  = -999;

  /* now populate data base entries with new values */
  if (str) {
    len = 0;
    while (*(str+len)) len++;
    newname = (char *)malloc(len+1);
    if (newname==NULL) return;
    for (i=0; i<len; i++) *(newname+i) = *(str+i);
    *(newname+len) = '\0';
    pnames[pnum] = newname;
  }
  if (*(itt+1) < -900) return;  ptypes[pnum]  = *(itt+1);
  if (*(itt+2) < -900) return;  pxsz[pnum]    = *(itt+2);
  if (*(itt+3) < -900) return;  pysz[pnum]    = *(itt+3);
  if (*(itt+4) < -900) return;  pfthick[pnum] = *(itt+4);
  if (*(itt+5) < -900) return;  pfcols[pnum]  = *(itt+5);
  if (*(itt+6) < -900) return;  pbcols[pnum]  = *(itt+6);
}

/* Query pattern settings */

void gxdbqpatt (gaint pnum, struct gxdbquery *pdbq) {    
  if (pnum<0 || pnum>=COLORMAX) return;
  pdbq->ptype = ptypes[pnum];
  pdbq->pxs = pxsz[pnum];
  if (pdbq->pxs < -900) pdbq->pxs = 9;
  pdbq->pys = pysz[pnum];
  if (pdbq->pys < -900) pdbq->pys = 9;
  pdbq->pthick = pfthick[pnum];
  if (pdbq->pthick < -900) pdbq->pthick = 3;
  pdbq->pfcol = pfcols[pnum];
  pdbq->pbcol = pbcols[pnum];
  pdbq->fname = pnames[pnum];
};

/* Query hershey flag */

gaint gxdbqhersh (void) {    
  return (hershflag);
}

/* Set hershey flag */

void gxdbsethersh (gaint flag) {    
  if (flag==0 || flag==1) 
    hershflag = flag;
}

/* Query transparent color (for image output) */

gaint gxdbqtransclr (void) {   
  return (dbtransclr);
}

/* Set transparent color (for image output) */

void gxdbsettransclr (gaint clr) {   
  dbtransclr = clr; 
}

/* Set device background color. */

void gxdbck (gaint clr) {    
  dbdevbck = clr;
}

/* Set output background color */

void gxdboutbck (gaint clr) {    
  dboutbck = clr;
}

/* Query background color */

gaint gxdbkq  (void) {    
  /* If the output background color is not set, return device background color */
  if (dboutbck != -1) 
    return (dboutbck);
  else 
    return (dbdevbck);
}


