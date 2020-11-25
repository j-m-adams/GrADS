/* Copyright (C) 1988-2020 by George Mason University. See file COPYRIGHT for more information. */

/* This file contains the primary Cairo-GrADS interface. 
   The interactive interface (X windows) is managed by routines in gxX.c and the routines here.  
   Hardcopy output is generated here, but managed in gxprint.c  */


/* Include ./configure's header file */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <cairo.h>
#include <cairo-ps.h>
#include <cairo-svg.h>
#include <cairo-pdf.h>
#include <cairo-xlib.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "gatypes.h"
#include "gx.h"
#include "gxC.h"

#define SX 800                              /* default X dimension of the image output */

void gxdXflush(void);                       /* function prototype (only used here and in gxX.c) */

/* local variables */
static gaint lcolor=-999,lwidth;            /* Current attributes */
static gaint actualcolor;                   /* current *actual* color */
static gaint lcolorsave,lwidthsave;         /* for save and restore */
static gaint rsav,gsav,bsav,psav;           /* Avoid excess color change calls */ 
static gaint aaflg=0;                       /* Anti-Aliasing flag */
static gaint filflg=0;                      /* Polygon-Filling flag */
static gaint brdrflg=0;                     /* Vector graphic border flag */
static gadouble brdrwid=0.0;                /* Vector graphic border size */
static gaint rotate=0;                      /* rotate landscape plots for hardcopy output */
static gaint width, height;                 /* Drawable size */
static gaint Xwidth, Xheight;               /* Window size */
static gaint Bwidth, Bheight;               /* Batch surface size */
static gaint force=0;                       /* force a color change */
static gadouble xsize, ysize;               /* GrADS page size (inches) */
static gadouble xscl,yscl;                  /* Window Scaling */
static gadouble xxx,yyy;                    /* Old position */
static FT_Library library=NULL;             /* for drawing fonts with FreeType with cairo */
static FT_Face face[100];                   /* for saving FreeType font faces */ 
static gaint faceinit=0;                    /* for knowing whether font faces have been initialized */
static cairo_surface_t *surface=NULL;       /* Surface being drawn to */   
static cairo_t *cr;                         /* graphics context */
static gaint surftype;                      /* Type of current surface.
                                               1=X, 2=Image, 3=PS/EPS, 4=PDF, 5=SVG */
static cairo_surface_t *Xsurface=NULL, *Hsurface=NULL, *Bsurface=NULL;
                                            /* X, Hardcopy, and Batch mode surfaces */
                                            /* X surface is passed to us by gxX. */
                                            /* Others are created as needed. */
static cairo_t *Xcr, *Hcr, *Bcr;            /* X, Hardcopy, and Batch mode contexts */  
static cairo_pattern_t *pattern[2048];      /* Save our patterns here */
static cairo_surface_t *pattsurf[2048];
static cairo_surface_t *surfmask, *surfsave;  /* Stuff for masking */
static cairo_t *crmask, *crsave;
static cairo_pattern_t *patternmask;
static gaint drawing;                         /* In the middle of line-to's? */
static gaint maskflag;                        /* Masking going on */
static gaint batch=0;                         /* Batch mode */
static gadouble clx,cly,clw,clh;              /* current clipping coordinates */
static gadouble clxsav,clysav,clwsav,clhsav;  /* saved clipping coordinates */


/* Initialize X interface, allocate cr */
/* If in batch mode, we don't get here */

void gxCbgn (cairo_surface_t *Xsfc, gadouble xsz, gadouble ysz, gaint dw, gaint dh) {
gaint i;

  batch = 0;
  for (i=0; i<2048; i++) pattern[i] = NULL;
  for (i=0; i<2048; i++) pattsurf[i] = NULL;
  if (!faceinit) {
    for (i=0; i<100; i++) face[i] = NULL; 
    faceinit=1;
    gxCftinit();         /* make sure FreeType library has been initialized */
  }

  drawing = 0;
  maskflag = 0;

  /* window dimensions */
  width = dw;   
  height = dh;  
  Xwidth = dw; 
  Xheight = dh;

  /* set page sizes  in inches */
  xsize = xsz; 
  ysize = ysz;

  /* initialize clipping parameters */
  clx = 0;     
  cly = 0;
  clw = width;
  clh = height;

  /* scale factors */
  xscl = (gadouble)(dw)/xsz;
  yscl = (gadouble)(dh)/ysz;


  /* make the X surface active for drawing */
  surface = Xsfc;
  surftype = 1;
  Xsurface = surface;
  cr = cairo_create(surface);
  Xcr = cr;
    
  /* default drawing settings */
  cairo_set_line_cap(cr,CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cr,CAIRO_LINE_JOIN_ROUND);
  gxCaa(1);                   /* initialize anti-aliasing to be ON */ 
  gxCcol(1);                  /* initial color is foreground */
}

/* Close down and clean up */

void gxCend (void) {
  if (batch) {
    /* batch mode */
    cairo_destroy(Bcr);
    cairo_surface_finish(Bsurface);
    cairo_surface_destroy(Bsurface);
    Bcr = NULL;
    Bsurface = NULL;
  }
  else {
    /* the interactive session */
    gxCflush(0);
    cairo_destroy(cr);
    cr = NULL;
  }
  /* close the FreeType library */
  gxCftend();     
}


/* Initialize an image surface for batch mode, created on startup.
   This will be used to get the lengths of strings. 
   Called by the printing layer.
   If in interactive mode, we don't get here. */

void gxCbatch (gadouble xsz, gadouble ysz) {
gaint dh,dw;               /* image size in pixels */
gaint i;

  batch=1;
  if (!faceinit) {
    for (i=0; i<100; i++) face[i] = NULL; 
    faceinit=1;
    gxCftinit();         /* make sure FreeType library has been initialized */
  }

  /* set page sizes  in inches */
  xsize = xsz; 
  ysize = ysz;

  /* set window dimensions; 100 times the page size or 1100 by 850 */
  if (xsize > ysize) { 
    dw = 1100;
    dh = 850;
  } 
  else {
    dw = 850;
    dh = 1100;
  }
  width = dw;
  height = dh; 
  Bwidth = dw; 
  Bheight = dh;

  /* scale factors (pixels per inch) */
  xscl = (gadouble)(dw)/xsize;
  yscl = (gadouble)(dh)/ysize;

  /* initialize clipping parameters */
  clx = 0;     
  cly = 0;
  clw = width;
  clh = height;

  /* create the image surface */
  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, dw, dh);
  surftype = 2;
  cr = cairo_create (surface);

  Bsurface = surface;
  Bcr = cr;
}


/* Initialize a hardcopy surface and make it the active surface for drawing. 
   Save settings for the interactive surface so they can be restored when we are done.  
   fmtflg == 1  for eps
   fmtflg == 2  for ps
   fmtflg == 3  for pdf
   fmtflg == 4  for svg
   fmtflg == 5  for png

   For vector graphic outputs, we need the output file name at initialization.
   For image output, we need the output file name at the end.

   We typically dump a single image to a file, and then close it. 
   Page breaks are not (yet?) in the new metabuffer structure
   Cairo docs: "An Encapsulated PostScript file should never contain more than one page"

*/

gaint gxChinit (gadouble xsz, gadouble ysz, gaint xin, gaint yin, gaint bwin, 
		char *fname, gaint fmtflg, char *bgImage, gadouble border) {
gaint status;
gaint dh=0,dw=0;          /* image size in pixels */
gadouble psx=0,psy=0;     /* page size in points */
gaint bcol,i;
gaint bgwidth,bgheight,len;

  if (!faceinit) {
    for (i=0; i<100; i++) face[i] = NULL;  
    faceinit=1;
    gxCftinit();         /* make sure FreeType library has been initialized */
  }

  gxCflush (1);
  drawing = 0;
  maskflag = 0;

  /* save these settings to restore when hardcopy drawing is finished */
  lcolorsave = lcolor; 
  lwidthsave = lwidth;
  clxsav = clx; 
  clysav = cly;
  clwsav = clw;
  clhsav = clh;

  /* set page sizes in inches */
  xsize = xsz;
  ysize = ysz;

  if (fmtflg==5) {  /* Image output */
    /*  Set the image width and height */
    if (xin<0 || yin<0) {     /* user has not specified image size */
      if (xsize > ysize) {    /* landscape */
	dw = SX; 
	dh = (gaint)((ysize/xsize)*SX);
      }
      else {
	dw = (gaint)((xsize/ysize)*SX); 
	dh = SX;
      }
    } 
    else {
      dw = xin; 
      dh = yin;
    }
    /* image dimensions */
    width = dw; 
    height = dh;
    /* scale factors (pixels per inch) */
    xscl = (gadouble)(dw)/xsize;
    yscl = (gadouble)(dh)/ysize;
  }

  if (fmtflg==1 || fmtflg==2 || fmtflg==3 || fmtflg==4) {  /* vector graphic output */
    /* set page size in points */
    if (ysize > xsize) {      
      /* portrait: no rotation needed for any format */
      psx = xsize * 72.0;
      psy = ysize * 72.0;
    }
    else {
      /* landscape: ps, eps, and pdf must be rotated to fit on page */
      if (fmtflg==1 || fmtflg==2 || fmtflg==3) {
	rotate = 1;         
	psx = ysize * 72.0;
	psy = xsize * 72.0;
      }
      else {
	/* landscape: svg doesn't need to be rotated */
	psx = xsize * 72.0;
	psy = ysize * 72.0;
      }
    }
    /* page dimensions */
    width = (gaint)psx;
    height = (gaint)psy;
    /* scale factors (points per inch) */
    xscl = 72.0;
    yscl = 72.0;
    /* set a border for ps, eps, and pdf formats */
    if (fmtflg<=3) {
      brdrwid = border * xscl;
      brdrflg = 1; 
    }
  }

  /* initialize clipping parameters */
  clx = 0;     
  cly = 0;
  clw = width;
  clh = height;

  /* Create the hardcopy surface */
  if (fmtflg==1 || fmtflg==2) {            /* PS or EPS */
    surftype = 3;
    surface = cairo_ps_surface_create (fname,psx,psy);
    if (fmtflg==1) cairo_ps_surface_set_eps(surface,1);
  }
  else if (fmtflg==3) {                    /* PDF */
    surftype = 4;
    surface = cairo_pdf_surface_create(fname,psx,psy);
  }
  else if (fmtflg==4) {                    /* SVG */
    surftype = 5;
    surface = cairo_svg_surface_create(fname,psx,psy);
  }
  else if (fmtflg==5) {                    /* PNG */
    surftype = 2;

    if (*bgImage) {
      /* Make sure bgImage is a .png  */
      len = 0;
      while (*(bgImage+len)) len++;
      len = len-4;
      if (len>0) {
	if (*(bgImage+len+1)!='p' || *(bgImage+len+2)!='n' || *(bgImage+len+3)!='g' ) {
	  if (*(bgImage+len+1)!='P' || *(bgImage+len+2)!='N' || *(bgImage+len+3)!='G' ) {
	    return(5);
	  }
	}
      }
      /* create a new surface from the background image */
      surface = cairo_image_surface_create_from_png (bgImage);
      status = cairo_surface_status (surface);
      if (status) {
	printf("Error in gxChinit: unable to import background image %s\n",bgImage); 
        printf("Cairo status: %s\n",cairo_status_to_string(status));
	cairo_surface_finish(surface);
	cairo_surface_destroy(surface);
	return(7); 
      }
      /* make sure background image size matches output image size */
      bgwidth = cairo_image_surface_get_width (surface);
      bgheight = cairo_image_surface_get_height (surface);
      if (bgwidth != width || bgheight != height) {
	printf(" background image size is %d x %d \n",bgwidth,bgheight);
	printf("     output image size is %d x %d \n",width,height);
	cairo_surface_finish(surface);
	cairo_surface_destroy(surface);
	return(10);
      }
    }
    else {
      /* no background image specified by user */
      surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, dw, dh);
    }
  }

  /* Make sure surface set up without any problems */
  status = cairo_surface_status (surface);
  if (status) {
    printf("Error in gxChinit: failed to initialize hardcopy surface \n");
    printf("Cairo status: %s\n",cairo_status_to_string(status));
    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);
    return(1);
  }
 
  /* Set the Cairo context */
  cr = cairo_create (surface);
  Hsurface = surface; 
  Hcr = cr;

  /* default drawing settings */
  cairo_set_line_cap(cr,CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cr,CAIRO_LINE_JOIN_ROUND);
  gxCaa(1);                   /* initialize anti-aliasing to be ON */ 

  force = 1;             /* force a color change */
  bcol = gxdbkq();
  if (bcol>1) {
    /* User has set the background to be a color other than black/white.
       The background rectangle is therefore in the metabuffer and will
       cover whatever we draw here to initalize the output */
    if (bwin>-900) {
      printf("Warning: Background color cannot be changed at print time    \n");
      printf(" if it has been set to a color other than black or white. \n");
      printf(" The current background color is %d. \n",bcol);
      if (bwin==1) 
      printf(" The option \"white\" will be ignored. \n");
      else
      printf(" The option \"black\" will be ignored. \n");
    }
  }
  else {
    if (bwin>-900) 
      gxdboutbck(bwin);  /* change the background color if user provided an override */
    gxCcol(0);           /* 0 here means 'background' */
    cairo_paint(cr);     /* paint it */
  }

  gxCcol(1);             /* set the initial color to 'foreground' */
  force = 0;             /* back to unforced color changes */
  return (0);
}


/* End the hardcopy drawing and reset drawing to the interactive/batch surface */

gaint gxChend (char *fname, gaint fmtflg, char *fgImage) {
gaint fgwidth,fgheight,len,status;
cairo_surface_t *sfc2=NULL;     /* Surface for fgImage */   

  gxCflush(1);         /* finish drawing */
  
  /* bgImage and fgImage only work when output format is PNG */
  if (fmtflg==5) {              
    if (*fgImage) {
      /* Make sure fgImage is a .png  */
      len = 0;
      while (*(fgImage+len)) len++;
      len = len-4;
      if (len>0) {
	if (*(fgImage+len+1)!='p' || *(fgImage+len+2)!='n' || *(fgImage+len+3)!='g' ) {
	  if (*(fgImage+len+1)!='P' || *(fgImage+len+2)!='N' || *(fgImage+len+3)!='G' ) {
	    return(6);
	  }
	}
      }
      /* create a new surface from the foreground image */
      sfc2 = cairo_image_surface_create_from_png (fgImage);
      status = cairo_surface_status (sfc2);
      if (status) {
	printf("Error in gxChend: unable to import foreground image %s\n",fgImage);
	printf("Cairo status: %s\n",cairo_status_to_string(status));
	cairo_surface_finish(sfc2);
	cairo_surface_destroy(sfc2); 
	return(8); 
      }
      /* check to make sure foreground and output images are the same size */
      fgwidth = cairo_image_surface_get_width (sfc2);
      fgheight = cairo_image_surface_get_height (sfc2);
      if (fgwidth != width || fgheight != height) {
	printf(" foreground image size is %d x %d \n",fgwidth,fgheight);
	printf("     output image size is %d x %d \n",width,height);
	cairo_surface_finish(sfc2);
	cairo_surface_destroy(sfc2);
	return(11);
      }
      /* draw the foreground image OVER the hardcopy surface */
      cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
      cairo_set_source_surface(cr, sfc2, 0, 0);
      cairo_paint(cr);
      /* clean up */
      cairo_surface_finish(sfc2);
      cairo_surface_destroy(sfc2);
    }
  }

  /* Make sure surface was rendered without any problems */
  status = cairo_surface_status (surface);
  if (status) {
    printf("Error in gxChend: an error occured during rendering of hardcopy surface \n");
    printf("Cairo status: %s\n",cairo_status_to_string(status));
    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);
    return(1);
  }

  /* dump everything to hardcopy output, finish and destroy the surface */
  cairo_surface_show_page (surface);
  if (fmtflg==5) {
    cairo_surface_write_to_png (surface, fname);  /* for image (png) output only */
  }
  cairo_destroy(cr);
  cairo_surface_finish(surface);
  cairo_surface_destroy(surface);
  rotate=0;             /* unset rotation for landscape hardcopy */
  brdrflg=0;            /* reset border flag for vector graphic output */
  brdrwid=0.0;          /* reset border width for vector graphic output */
  gxdboutbck(-1);       /* unset output background color */

  /* Reset everything back to the interactive/batch surface for drawing */
  Hsurface=NULL;
  if (batch) {
    surface = Bsurface;
    cr = Bcr;
    width  = Bwidth; 
    height = Bheight;
  }
  else {
    surface = Xsurface;
    cr = Xcr;
    width  = Xwidth; 
    height = Xheight;
  }
  lcolor = lcolorsave; 
  lwidth = lwidthsave; 
  clx = clxsav; 
  cly = clysav;
  clw = clwsav;
  clh = clhsav;
  xscl = (gadouble)(width)/xsize;
  yscl = (gadouble)(height)/ysize;

  return (0);
}


/* Make this X surface active for drawing */

void gxCsfc (cairo_surface_t *Xsfc) {
  surface = Xsfc;
  surftype = 1;
  Xsurface = surface;
  cr = cairo_create(surface);
  Xcr = cr;
}

/* Swap action for double buffering */

void gxCswp (cairo_surface_t *Xsfc, cairo_surface_t *Xsfc2) {
  /* draw the background (sfc2) onto the foreground (sfc) */
  cr = cairo_create(Xsfc);
  cairo_set_source_surface(cr, Xsfc2, 0, 0);
  cairo_paint(cr);
  /* reset the cairo context to the background surface,  */
  cr = cairo_create(Xsfc2);
}

/* Frame action.  Clear the frame. */ 

void gxCfrm (void) {
gaint bcolr,savcol;

  gxCflush(0);
  savcol = lcolor;    /* Ccol will change lcolor value */
  bcolr = gxdbkq();   /* get background color from data base */
  force = 1;          /* force the color change */
  if (bcolr>1) 
    gxCcol(bcolr);    /* the background color is neither black nor white */
  else {
    gxCcol(0);        /* 0 here means 'background' */
  }
  cairo_paint(cr);    /* paint the source */
  gxCcol(savcol);     /* restore the current color */
  force = 0;          /* back to unforced color changes */
}

/* Our interactive surface got resized. */

void gxCrsiz (gaint dw, gaint dh) {
  width = dw;  
  height = dh; 
  Xwidth = width; 
  Xheight = height;
  xscl = (gadouble)(dw)/xsize;
  yscl = (gadouble)(dh)/ysize;
  gxCfrm();
}

/*  Finish up any pending actions, like drawing, or masking */
/*  opt=0 discard everything. opt=1 finish rendering. */

void gxCflush (gaint opt) {
  if (drawing) cairo_stroke(cr);
  drawing = 0;
  if (maskflag) {
    cr = crsave;  
    surface = surfsave;
    if (opt) {
      cairo_surface_show_page(surfmask);
      patternmask = cairo_pattern_create_for_surface(surfmask);
      cairo_set_source_rgba(cr, (double)(rsav)/255.0, (double)(gsav)/255.0, 
			    (double)(bsav)/255.0, (double)(-1*psav)/255.0);
      cairo_mask(cr, patternmask);
    }
    cairo_destroy(crmask);
    cairo_surface_finish (surfmask);
    cairo_surface_destroy (surfmask);
    maskflag = 0;
  }
  if (surftype==1 && opt) gxdXflush();  /* force completed graphics to display */
}

/* New color value */

void gxCcol (gaint clr) {
struct gxdbquery dbq;
gaint bcol;

  if (drawing) cairo_stroke(cr);
  drawing = 0;
  lcolor = clr;                  /* outside this routine lcolor 0/1 still means background/foreground */
  bcol = gxdbkq();               /* get background color */
  if (bcol==1) {                 /* if background is white ...  */
    if (clr==0)      clr = 1;    /*  ...change color 0 to white (1) */
    else if (clr==1) clr = 0;    /*  ...change color 1 to black (0) */
  }
  gxdbqcol(clr, &dbq);           /* query the data base for color values */
  if (clr == gxdbqtransclr()) {  /* If this is the transparent color, override values  */
    dbq.red   = 0;
    dbq.green = 0;
    dbq.blue  = 0;
    dbq.alpha = 0;
    dbq.tile  = -999;
  }
  if (force || clr!=actualcolor || dbq.red!=rsav || dbq.green!=gsav || dbq.blue!=bsav || dbq.alpha!=psav) {
    /* change to new color */
    if (maskflag) gxCflush(1);
    actualcolor = clr;           /* inside this routine actualcolor 0/1 means black or white */
    if (dbq.tile > -900) {
      /* new color is a pattern */
      if (pattern[dbq.tile]==NULL)  gxCpattc (dbq.tile);
      cairo_set_source(cr, pattern[dbq.tile]);
      cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REPEAT);
    } 
    else {
      if (dbq.alpha==255) {
        cairo_set_source_rgb(cr, (double)(dbq.red)/255.0, (double)(dbq.green)/255.0, (double)(dbq.blue)/255.0);
      } 
      else {
	/* set up color masking if alpha value is negative */
        if (dbq.alpha < 0) {
          maskflag = 1;
          surfmask = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width , height);
          crmask = cairo_create(surfmask);                    /* create a new graphics context */
          cairo_set_source_rgba(crmask, 1.0, 1.0, 1.0, 1.0);  /* set mask color */  
	  gxdbqwid(lwidth-1,&dbq);                            /* set current line width */
	  cairo_set_line_width(crmask,dbq.wid);  
	  cairo_set_line_cap(crmask,CAIRO_LINE_CAP_ROUND);    /* set line drawing specs */
	  cairo_set_line_join(crmask,CAIRO_LINE_JOIN_ROUND);
	  /* save the current surface/cr, set the new surfmask/crmask to be the current one */
          crsave = cr;
          surfsave = surface;
          cr = crmask;
          surface = surfmask;
        }
	else {
          cairo_set_source_rgba(cr, (double)(dbq.red)/255.0, (double)(dbq.green)/255.0,
			 	    (double)(dbq.blue)/255.0, (double)(dbq.alpha)/255.0);
        }
      }
    }
    /* save these color values */
    rsav = dbq.red; 
    gsav = dbq.green; 
    bsav = dbq.blue; 
    psav = dbq.alpha; 
  }
}

/* Set the line width */

void gxCwid (gaint wid) {                 
struct gxdbquery dbq;
  if (drawing) cairo_stroke(cr);
  drawing = 0;
  lwidth = wid;
  gxdbqwid(wid-1,&dbq);  /* at this point wid still starts at 1, so subtract to get index right */
  cairo_set_line_width(cr,dbq.wid);  
}


/* Convert x,y coordinates from inches on the page into coordinates for current drawing surface */

void gxCxycnv (gadouble x, gadouble y, gadouble *nx, gadouble *ny) {
  if (surftype>2 && rotate) {
    *ny = (gadouble)height - x*xscl;
    *nx = (gadouble)width - (y*yscl);
  }
  else {
    *nx = x*xscl;
    *ny = (gadouble)height - (y*yscl);
  }
  if (brdrflg) {
    *nx = brdrwid + *nx*(((gadouble)width -brdrwid*2.0)/(gadouble)width);
    *ny = brdrwid + *ny*(((gadouble)height-brdrwid*2.0)/(gadouble)height);
  }
}


/* Move to x,y */

void gxCmov (gadouble x, gadouble y) {
  if (drawing) cairo_stroke(cr);
  drawing = 0;
  gxCxycnv (x,y,&xxx,&yyy);
}


/* Draw to x,y */

void gxCdrw (gadouble x, gadouble y) {
gadouble di, dj;
  gxCxycnv (x,y,&di,&dj);
  if (!filflg) {           /* we're not filling a polygon */
    if (!drawing) {        /* this is the start of a line */
      drawing = 1;
      cairo_move_to(cr,xxx,yyy);
      cairo_line_to(cr,di,dj);
    }
    else {
      cairo_line_to(cr,di,dj);
    }
  }
  xxx = di;
  yyy = dj;
}


/* Set the clipping coordinates */

void gxCclip (gadouble x1, gadouble x2, gadouble y1, gadouble y2) {
gadouble di1,di2,dj1,dj2;
  gxCxycnv (x1,y1,&di1,&dj1);
  gxCxycnv (x2,y2,&di2,&dj2);
  /* set the clipping variables -- these get used when drawing characters */
  clx = di1;
  cly = dj2;
  clw = di2-di1;
  clh = dj1-dj2;
}


/* Draw a filled rectangle */

void gxCrec (gadouble x1, gadouble x2, gadouble y1, gadouble y2) {
gadouble di1,di2,dj1,dj2;
  if (drawing) cairo_stroke(cr);
  drawing = 0;
  gxCxycnv (x1,y1,&di1,&dj1);
  gxCxycnv (x2,y2,&di2,&dj2);
  if (di1!=di2 && dj1!=dj2) { 
    cairo_rectangle(cr,di1,dj2,di2-di1,dj1-dj2);  
    /* disable antialiasing, otherwise faint lines appear around the edges */
    if (aaflg) {
      cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE); 
      cairo_fill(cr);
      cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT); 
    }
    else {
      cairo_fill(cr);
    }
  } else {
    cairo_move_to(cr,di1,dj1);
    cairo_line_to(cr,di2,dj2);
    cairo_stroke(cr);
  }
}

/* Draw an open or filled circle */

void gxCcirc (gadouble x, gadouble y, gadouble r, gaint flg) {
gadouble di,dj,rad;
  if (drawing) cairo_stroke(cr);
  drawing = 0;
  gxCxycnv (x,y,&di,&dj);
  rad = r*yscl;
  cairo_move_to(cr,di+rad,dj);
  cairo_arc (cr, di, dj, rad, 0, 2*M_PI);
  if (flg)
    cairo_fill (cr);    /* filled */
  else
    cairo_stroke (cr);  /* open */
}


/* initialize a polygon fill by turning on a flag */

void gxCbfil (void) {
  filflg = 1;
}

/* Draw a filled polygon */

void gxCfil (gadouble *xy, gaint n) {
gadouble *pt,x,y;
gaint i;

  if (drawing) cairo_stroke(cr);
  drawing = 0;
  pt = xy;
  for (i=0; i<n; i++) {
    gxCxycnv (*pt,*(pt+1),&x,&y);
    if (i==0) cairo_move_to(cr,x,y);
    else cairo_line_to(cr,x,y);
    pt+=2;
  }
  /* disable antialiasing, otherwise faint lines appear around the edges */
  if (aaflg) {
    cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);
    cairo_fill(cr);                                  
    cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
  }
  else {
    cairo_fill(cr);                                  
  }
  /* turn off polygon-filling flag */
  filflg = 0;
  return;
}


/* Turn anti-aliasing on (flag=1) or off (flag=0) 
   Anti-aliasing is also automatically disabled in gxCrec and gxCfil so that
   faint lines do not appear around the edges of filled rectangles and polygons.
*/

void gxCaa (gaint flag) {
  if (cr!=NULL) {
    if (flag) {
      if (aaflg==0) {
	cairo_set_antialias(cr,CAIRO_ANTIALIAS_DEFAULT);
	aaflg = 1;
      }
    }
    else {
      if (aaflg) {
	cairo_set_antialias(cr,CAIRO_ANTIALIAS_NONE);
	aaflg = 0; 
      }
    }
  }
}


/* Get the x bearing (width) used to plot a character in the specific font and size */

gadouble gxCqchl (char ch, gaint fn, gadouble w) {
cairo_text_extents_t te;
gadouble usize,awidth,swidth,fontsize=100.0;
char str[2];
char *astr = "A";

  gxCselfont (fn);

  /* get the scale factor (for width only) based on the size of "A" */
  cairo_set_font_size (cr, fontsize);
  cairo_text_extents (cr, astr, &te);
  awidth = fontsize/te.width; 
  usize = w * awidth * xscl;
  if (brdrflg) usize = usize*(((gadouble)width-brdrwid*2.0)/(gadouble)width);

  /* get the text extents of the character */
  str[0] = ch;
  str[1] = '\0';
  cairo_text_extents (cr, str, &te);

  /* return the scaled width of the character */
  swidth = te.x_advance*usize/(fontsize*xscl);
  return(swidth);
}

/* Draw a character. 
   Specify a large font size and then scale it down, to prevent 
   the pixelation from interfereing with the exact positioning.
   Not much code, but it took a lot of work to get this right.  */

gadouble gxCch (char ch, gaint fn, gadouble x, gadouble y, gadouble w, gadouble h, gadouble rot) {
cairo_text_extents_t te;
gadouble xpage,ypage,usize,vsize;
gadouble aheight,awidth,swidth,fontsize=100.0;
char str[2];
char *astr = "A";

  if (drawing) cairo_stroke(cr);
  drawing = 0;
  gxCselfont (fn);

  /* get the scale factor based on size of "A" */
  cairo_set_font_size (cr, fontsize);
  cairo_text_extents (cr, astr, &te);
  awidth  = fontsize/te.width;
  aheight = fontsize/te.height;
  usize = w * xscl * awidth;
  vsize = h * yscl * aheight;
  if (brdrflg) {
    usize = usize*(((gadouble)width-brdrwid*2.0)/(gadouble)width);
    vsize = vsize*(((gadouble)height-brdrwid*2.0)/(gadouble)height);
  }
  /* Convert the position coordinates and adjust the rotation if necessary */
  gxCxycnv (x,y,&xpage,&ypage);
  if (rotate) rot = rot + M_PI/2;

  /* get the text extents of the character we want to draw */
  str[0] = ch;
  str[1] = '\0';
  cairo_text_extents (cr, str, &te);

  /* draw a character */
  cairo_save(cr);                                    /* save the untranslated, unrotated, unclipped context */
  cairo_rectangle(cr,clx,cly,clw,clh);               /* set the clipping area */
  cairo_clip (cr);                                   /* clip it */
  cairo_translate(cr,xpage,ypage);                   /* translate to location of character */
  cairo_rotate(cr,-1.0*rot);                         /* rotate if necessary */
  cairo_move_to (cr, 0.0, 0.0);                      /* move to (translated) origin */
  cairo_scale (cr, usize/fontsize, vsize/fontsize);  /* apply the scale factor right before drawing */
  cairo_show_text (cr, str);                         /* finally, draw the darned thing */
  cairo_restore(cr);                                 /* restore the saved graphics context */

  /* return the scaled width of the character */
  swidth = te.x_advance*usize/(fontsize*xscl);
  return(swidth);
}


/* Set the Cairo font based on the font number and the settings 
   obtained from the backend database settings */

void gxCselfont (gaint fn) {
  struct gxdbquery dbq;
  cairo_font_face_t *cf_face=NULL;           
  FT_Face newface;
  gaint dflt,rc;
  gaint cbold,citalic;
  
  /* get font info from graphics database */
  gxdbqfont (fn, &dbq);

  /* font 0-5 (but not 3) and hershflag=1 in gxmeta.c, so we use cairo to draw something hershey-like */
  if (fn<6) {
    cbold = CAIRO_FONT_WEIGHT_NORMAL;
    if (dbq.fbold == 1) cbold = CAIRO_FONT_WEIGHT_BOLD;
    citalic = CAIRO_FONT_SLANT_NORMAL;
    if (dbq.fitalic == 1) citalic = CAIRO_FONT_SLANT_ITALIC;
    if (dbq.fitalic == 2) citalic = CAIRO_FONT_SLANT_OBLIQUE;
  
    if (dbq.fname == NULL) {
      /* we should never have fn<6 and fname==NULL, but just in case... */
      cairo_select_font_face (cr, "sans-serif", citalic, cbold); 
    } else {
      cairo_select_font_face (cr, dbq.fname, citalic, cbold);
    }
  }
  else {
    /* font>=10 */
    dflt=0;
    if (library == NULL) dflt=1;      /* use default fonts */  
    if (dbq.fname == NULL) dflt=1;    /* make sure we have a font filename */

    if (!dflt) {
      if (face[fn]==NULL) {
        /* try to open user-provided font file */
        rc = FT_New_Face(library, dbq.fname, 0, &newface);
        if (rc) { 
  	  printf("Error: Unable to open font file \"%s\"\n",dbq.fname);
          printf(" Will use a default \"sans-serif\" font instead\n");
          dflt=1;
	  /* update the data base so this error message only appears once */
	  gxdbsetfn(fn, NULL);
        } 
        else {
	  /* we succeeded, so save the face and update the font status */
          face[fn] = newface; 
	}          
      }
      else {
        /* this font has already been opened, so we use the saved face */
        newface = face[fn];
      }
    }

    if (!dflt) {
	/* create a new font face  */
	cf_face = cairo_ft_font_face_create_for_ft_face (newface, 0);
	cairo_set_font_face(cr,cf_face);
    }
    else {
      /* set up a default font with the Cairo "Toy" interface */
      cairo_select_font_face (cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    }
  }
}

/* Create a tile pattern on the fly.  Get all the info on the 
   pattern from the backend db.   */

void gxCpattc (gaint pnum) {
struct gxdbquery dbq;
cairo_surface_t *surfacep1;
cairo_t *crpatt;
gadouble xx,yy,x1,y1;
gaint pt,alph,status;

  if (pnum<0 || pnum>=COLORMAX) return;

  gxdbqpatt (pnum, &dbq);
  gxdbqwid (dbq.pthick, &dbq);

  if (dbq.ptype==0) {
    /* create tile from user-provided filename */
    surfacep1 = cairo_image_surface_create_from_png (dbq.fname);
    status = cairo_surface_status (surfacep1);
    if (status) {
      printf("Error in gxCpattc: unable to import tile image %s \n",dbq.fname);
      printf("Cairo status: %s\n",cairo_status_to_string(status));
      cairo_surface_finish(surfacep1);
      cairo_surface_destroy(surfacep1); 
      return; 
    }
    crpatt = cairo_create(surfacep1);
  }
  else {
    /* create the tile on-the-fly */
    surfacep1 = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, dbq.pxs, dbq.pys);
    crpatt = cairo_create(surfacep1);
    
    if (dbq.ptype==1) {
      /* Type is a solid -- paint entire surface with foreground color */
      if (dbq.pfcol < 0 ) {
	/* use default foreground color, opaque white */
	cairo_set_source_rgba(crpatt, 1.0, 1.0, 1.0, 1.0);
      } else {
	gxdbqcol (dbq.pfcol, &dbq);
	if (dbq.red < 0) {                
	  /* this color cannot be assigned to a tile, use default */
	  cairo_set_source_rgba(crpatt, 1.0, 1.0, 1.0, 1.0);
	} else {
	  alph = dbq.alpha;
	  if (alph<0) alph = -1*alph;
	  cairo_set_source_rgba(crpatt, (double)(dbq.red)/255.0, (double)(dbq.green)/255.0,
				(double)(dbq.blue)/255.0, (double)(alph)/255.0);
	}
      }
    } else {
      /* Type is not a solid -- paint entire surface with background color */
      if (dbq.pbcol < 0 ) {
	/* use default background color, which is completely transparent */
	cairo_set_source_rgba(crpatt, 0.0, 0.0, 0.0, 0.0);
      } else {
	gxdbqcol (dbq.pbcol, &dbq);
	if (dbq.red < 0) {
	  /* this color cannot be assigned to a tile, use default */
	  cairo_set_source_rgba(crpatt, 0.0, 0.0, 0.0, 0.0);
	} else {
	  alph = dbq.alpha;
	  if (alph<0) alph = -1*alph;
	  cairo_set_source_rgba(crpatt, (double)(dbq.red)/255.0, (double)(dbq.green)/255.0,
				(double)(dbq.blue)/255.0, (double)(alph)/255.0);
	}
      }
    }
    cairo_paint(crpatt);

    /* now paint the dots or lines */
    if (dbq.ptype>1) {     /* Not solid */
      if (dbq.pfcol < 0 ) {
	/* use default foreground color, opaque white */
	cairo_set_source_rgba(crpatt, 1.0, 1.0, 1.0, 1.0);
      } else {
	gxdbqcol (dbq.pfcol, &dbq);
	if (dbq.red < 0) {
	  /* this color cannot be assigned to a tile, use default */
	  cairo_set_source_rgba(crpatt, 1.0, 1.0, 1.0, 1.0);
	} else {
	  alph = dbq.alpha;
	  if (alph<0) alph = -1*alph;
	  cairo_set_source_rgba(crpatt, (double)(dbq.red)/255.0, (double)(dbq.green)/255.0,
				(double)(dbq.blue)/255.0, (double)(alph)/255.0);
	}
      }
      cairo_set_line_width(crpatt,dbq.wid);  
      xx = (gadouble)dbq.pxs;
      yy = (gadouble)dbq.pys;
      pt = dbq.ptype;
      if (pt==2) {         /* dot */
	x1 = (xx/2.0)-dbq.wid/2.0; y1 = (yy/2.0);
	cairo_arc (crpatt, x1, y1, dbq.wid, 0.0, 2.0*3.1416);
	cairo_fill (crpatt);
      } else if (pt>=3 && pt<=5) {   /* diagonals */
	if (pt==3 || pt==5) {
	  cairo_move_to (crpatt, 0.0, 0.0);
	  cairo_line_to (crpatt, xx, yy);
	  cairo_stroke (crpatt);
	  cairo_move_to (crpatt, 0.0, -1.0*yy);
	  cairo_line_to (crpatt, 2.0*xx, yy);
	  cairo_stroke (crpatt);
	  cairo_move_to (crpatt, -1.0*xx, 0.0);
	  cairo_line_to (crpatt, xx, 2.0*yy);
	  cairo_stroke (crpatt);
	}
	if (pt==4 || pt==5) {
	  cairo_move_to (crpatt, 0.0, yy);
	  cairo_line_to (crpatt, xx, 0.0);
	  cairo_stroke (crpatt);
	  cairo_move_to (crpatt, 0.0, 2.0*yy);
	  cairo_line_to (crpatt, 2.0*xx, 0.0);
	  cairo_stroke (crpatt);
	  cairo_move_to (crpatt, -1.0*xx, yy);
	  cairo_line_to (crpatt, xx, -1.0*yy);
	  cairo_stroke (crpatt);
	}
      } else if (pt>=6 && pt<=8) {   /* up/down accross */
	x1 = xx/2.0; y1 = yy/2.0;
	if (pt==6 || pt==8) {
	  cairo_move_to (crpatt, x1, 0.0);
	  cairo_line_to (crpatt, x1, yy);
	  cairo_stroke (crpatt);
	}
	if (pt==7 || pt==8) {
	  cairo_move_to (crpatt, 0.0, y1);
	  cairo_line_to (crpatt, xx, y1);
	  cairo_stroke (crpatt);
	}
      } 
    }
  }

  cairo_surface_show_page (surfacep1);
  pattern[pnum] = cairo_pattern_create_for_surface(surfacep1);
  pattsurf[pnum] = surfacep1; 
  cairo_destroy(crpatt);
}

/* Here we get notified that a pattern has been reset/changed.  
   This will trigger re-creating the pattern if it is used. */

void gxCpattrset (gaint pnum) {
  if (pnum<0 || pnum>=2048) return;
  cairo_pattern_destroy (pattern[pnum]);
  cairo_surface_finish (pattsurf[pnum]);
  cairo_surface_destroy (pattsurf[pnum]);
  pattern[pnum] = NULL;
  pattsurf[pnum] = NULL;
}

/* initialize the FreeType library */
void gxCftinit (void) {
  gaint rc=1;
  if (library == NULL) {
    rc = FT_Init_FreeType(&library);
    if (rc) library=NULL;
  }
}

/* close the FreeType library */
void gxCftend (void) {
  FT_Done_FreeType(library);
}

void gxCpush (void) {
  cairo_push_group(cr);
}

void gxCpop (void) {
  cairo_pop_group_to_source(cr);
  cairo_paint(cr);
}

/* report on configuration */
void gxCcfg (void) {
  printf("cairo-%d.%d.%d ",CAIRO_VERSION_MAJOR,CAIRO_VERSION_MINOR,CAIRO_VERSION_MICRO);
}

