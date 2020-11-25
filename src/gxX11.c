/* Copyright (C) 1988-2020 by George Mason University. See file COPYRIGHT for more information. */

/* Include ./configure's header file */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#include "gatypes.h"
#include "gx.h"
#include "bitmaps.h"

/* Following function prototype has to go here since it 
   depends on X include information, which shouldn't go in gx.h */

/*  gxbcol:  Assign best rgb to color number from standard colormap */
gaint gxbcol (XStandardColormap*, XColor *);
void gxdrbb (gaint, gaint, gadouble, gadouble, gadouble, gadouble, gaint);
void gxrswd (gaint);

/* Interface for X11 Release 3  */

/* Device interface level.  Following routines need to be
   interfaced to the hardware:

   gxdbgn - Initialize graphics output.  Set up any hardware
            scaling needed, clear the graphics display area,
            etc.
   gxdcol - Set hardware color.  The colors should be set up
            as follows:
            0 - black;   1 - white    2 - red     3 - green
            4 - blue     5 - cyan     6 - magenta 7 - yellow
            8 - orange   9 - purple  10 - yell-grn  11 - lt.blue
           12 - ora.yell 3 - blu-grn 14 - blu-purp  15 - grey
            If colors are not available then grey scales can
            be used, or the call can be a no op.
   gxdwid - Set hardware line weight.
   gxdfrm - New frame.  If in single buffer mode, clear the active
            display.  If in double buffer mode, clear the background
            buffer.
   gxdsgl - Initiate single buffer mode.  Normal mode of operation.
   gxddbl - Initiate double buffer mode, if available.  Both the
            foreground and background displays should be cleared.
   gxdswp - Swap buffers when in double buffer mode.  Should take
            no action if in single buffer mode.
   gxdrec - Draw a color filled rectangle.
   gxddrw - Draw a line using current attributes
   gxdmov - Move to a new point
   gxdend - Terminate graphics output.
                                                                  */

static gaint batch=0;                       /* Batch mode? */
static gaint lcolor,lwidth,owidth;          /* Current attributes */
static gaint devbck;                        /* Device background, query the graphics database to get this */
static gadouble xscl,yscl;                  /* Window Scaling */
static gaint xxx,yyy;                       /* Old position */
static gadouble xsize, ysize;               /* User specified size */
static unsigned long cvals[276];            /* Color pixel values */
static gaint cused[276];                    /* Color is assigned */
static gaint cmach[276];                    /* Color is matched */
static gaint dblmode;                       /* single or double buffering */
static gaint width,height,depth;            /* Window dimensions */
static gaint reds[16] =   {  0,255,250,  0, 30,  0,240,230,240,160,160,  0,230,  0,130,170};
static gaint greens[16] = {  0,255, 60,220, 60,200,  0,220,130,  0,230,160,175,210,  0,170};
static gaint blues[16]  = {  0,255, 60,  0,255,200,130, 50, 40,200, 50,255, 45,140,220,170};
static gaint greys[16] = {0,255,215,140,80,110,230,170,200,50,155,95,185,125,65,177};

/* Various arrays are kept for structures that describe displayed
   widgets.  This information is kept in static arrays for
   efficiency reasons -- so the arrays will tend to be in memory
   together and will be much faster to scan when paging is
   going on.  A linked list set up would be much easier to code
   and much cleaner, but at least for now I will stick with
   the pre-defined arrays.  */

static struct gobj obj[512];    /* Displayed objects */
static struct gobj obj2[512];   /* Background objects */
static gaint obnum,obnum2;        /* Current number of objects */
static struct gbtn btn[256];    /* Current buttons */
static struct gbtn btn2[256];   /* Background buttons */
static struct grbb rbb[32];     /* Current rubber-band regions */
static struct grbb rbb2[32];    /* Background rbb regions */
static struct gdmu dmu[200];    /* Current dropmenus */
static struct gdmu dmu2[200];   /* Background dropmenus */
/* static struct gdlg dlg[1];      /\* Current dialog *\/ */

static struct gevent *evbase;   /* Anchor for GrADS event queue */

/* All stuff passed to Xlib routines as args are put here in
   static areas since we are not invoking Xlib routines from main*/

static Screen *sptr;
Display *display=(Display *)NULL;
static gaint snum;
static GC gc;
static XGCValues values;
static XEvent report;
Window win=(Window) NULL;    /* used via extern in gagui */
static Pixmap pmap;
static Pixmap pmaps[200];
static XImage *image;
static Drawable drwbl;
static char *window_name = "GrADS " GRADS_VERSION "";
static char *display_name = NULL; 
static char *icon_name = "GrADS";
static Pixmap icon_pixmap;
static XSizeHints size_hints;
static gaint argc;
static char **argv;
static char *args[4];
static char *name = "grads";
static char *dgeom = "500x400+10+10";
static char *ugeom = NULL;
static Colormap cmap;
static XColor cell;
static XPoint *point;
static gaint dblmode;                     /* single or double buffering */
static XFontStruct *font1, *font2, *font3;
static XFontStruct *font1i, *font2i, *font3i;
static XSetWindowAttributes xsetw;
static gaint gfont,cfont;                       /* Font in use by grads */
static gaint pfilld[200];
static gaint rstate = 1;              /* Redraw state -- when zero,
                                      acceptance of X Events is
                                      blocked. */
static gaint bsflg;                   /* Backing store enabled or not */
static gaint excnt;                   /* Count of exposes to skip */


/* tell x interface that we are in batch mode */

void gxdbat (void) {
  batch = 1;
}

/* Query default color rgb values*/

void gxqdrgb (gaint clr, gaint *r, gaint *g, gaint *b) {
  if (clr>=0 && clr<16) {
    *r = reds[clr];
    *g = greens[clr];
    *b = blues[clr];
  } 
  return;
}

/* Routine to specify user-defined geometry string for X display window. 
   Must be called before gxdbgn to have any affect */

void gxdgeo (char *arg) {
  ugeom = arg;
}

void gxdbgn (gadouble xsz, gadouble ysz) {
struct gxdbquery dbq;
gaint dw, dh, flag, i, ipos, jpos, border;
char **flist,*xfnam;

  for (i=0; i<200; i++) pfilld[i] = 0;
  for (i=0; i<256; i++) cused[i] = 0; 
  for (i=0; i<256; i++) { btn[i].ch = NULL; btn2[i].ch = NULL; }
  for (i=0; i<256; i++) { btn[i].num = -1; btn2[i].num = -1; }
  for (i=0; i<32;  i++) { rbb[i].num = -1; rbb2[i].num = -1; }
  for (i=0; i<200; i++) { dmu[i].num = -1; dmu2[i].num = -1; }
  for (i=0; i<512; i++) { obj[i].type = -1; obj2[i].type = -1; }
  obnum = 0; obnum2 = 0;

  evbase = NULL;
  excnt = 0;

  args[0] = name;
  args[1] = NULL;
  argv = args;
  argc = 1;

  xsize = xsz;
  ysize = ysz;
  border = 4;

  /* Connect to X server */

  if ( (display=XOpenDisplay(display_name)) == NULL ) {
    printf("Error in GXSTRT: Unable to connect to X server\n");
    exit( -1 );
  }

  /* Get screen size from display structure macro, then figure out
     proper window size */

  snum = DefaultScreen(display);
  sptr = DefaultScreenOfDisplay(display);
  bsflg = 0;
  if (DoesBackingStore(sptr)) bsflg = 1;
  cmap = DefaultColormap(display, snum);
  dw = DisplayWidth(display, snum);
  dh = DisplayHeight(display, snum);
  depth = DefaultDepth(display, snum);
  ipos = 0;
  jpos = 0;

  /* window sizes are scaled according to the height of display */
  if ( xsize >= ysize ) {           /* landscape */
    if (xsize==11.0 && ysize==8.5) {    /* preseve old default */
      dh = (gaint)((gadouble)dh*0.6);        
      dw = (gaint)((gadouble)(dh)*xsz/ysz);  
    } else {
      dw = (gaint)((gadouble)(dw)*0.6);      
      dh = (gaint)((gadouble)dw*ysz/xsz);
    }
  } else {                          /* portrait */
    dh = (gaint)((gadouble)dh*0.9);
    dw = (gaint)((gadouble)(dh)*xsz/ysz);
  } 

  xscl = (gadouble)(dw)/xsize;
  yscl = (gadouble)(dh)/ysize;

  size_hints.flags = PPosition | PSize ;
  if (ugeom) {
    XGeometry (display, snum, ugeom, dgeom, 4, 1, 1, 0, 0,
        &ipos, &jpos, &dw, &dh);
    size_hints.flags = USPosition | USSize ;
  }
  size_hints.x = ipos;
  size_hints.y = jpos;
  size_hints.width = dw;
  size_hints.height = dh;
  width = dw ; /* hoop */
  height = dh ; /* hoop */
  xscl = (gadouble) (dw) / xsize ; /* hoop */
  yscl = (gadouble) (dh) / ysize ; /* hoop */

  /* Create window */

  win = XCreateSimpleWindow(display, RootWindow(display,snum),
        ipos, jpos, dw, dh, border,
        WhitePixel(display, snum), BlackPixel(display,snum));
  gxdbck(0);    /* set background to zero, and then update local variable devbck */
  devbck = gxdbkq();

  /* Set up icon pixmap */

  icon_pixmap = XCreateBitmapFromData(display, win, (char*)icon_bitmap_bits,
                icon_bitmap_width, icon_bitmap_height);

  /* Set standard properties */

  XSetStandardProperties(display, win, window_name, icon_name,
       icon_pixmap, argv, argc, &size_hints);

  /* Set colors */

  for (i=0; i<16; i++) {

    /* get rgb values for each color from the graphics database */
    gxdbqcol(i, &dbq);
    cell.red   = dbq.red*256;
    cell.blue  = dbq.blue*256;
    cell.green = dbq.green*256;
    if (XAllocColor(display, cmap, &cell)) {
      cvals[i] = cell.pixel;
    } else {
      cvals[i] = cvals[1];    /* Assume white and black got allocated */
    }
    cused[i] = 1;
  }
  for (i=0; i<16; i++) {
    cell.red = greys[i]*256;
    cell.blue = greys[i]*256;
    cell.green = greys[i]*256;
    if (XAllocColor(display, cmap, &cell)) {
      cvals[i+256] = cell.pixel;
    } else {
      cvals[i+256] = cvals[15];
    }
    cused[i+256] = 1;
  }

  /* Select event types */

  XSelectInput(display, win, ButtonReleaseMask | ButtonPressMask |
      ButtonMotionMask | ExposureMask | StructureNotifyMask);

  /* Get a Graphics Context */

  gc = XCreateGC(display, win, 0L, &values);
  XSetForeground(display, gc, cvals[1]);
  XSetLineAttributes(display, gc, 0L, LineSolid,
                     CapButt, JoinBevel);
  lwidth = 1;
  owidth = 0;
  lcolor = 1;

  /* Display Window */

  XMapWindow(display, win);

  /* We now have to wait for the expose event that indicates our
     window is up.  Also handle any resizes so we get the scaling
     right in case it gets changed right away.  We will check again
     for resizes during frame operations */

#ifndef __CYGWIN32__
  flag = 1;
  while (flag)  {
    XNextEvent(display, &report);
    switch  (report.type) {
    case Expose:
      if (report.xexpose.count != 0) break;
      else flag = 0;
      break;
    case ConfigureNotify:
      width = report.xconfigure.width;
      height = report.xconfigure.height;
      xscl = (gadouble)(width)/xsize;
      yscl = (gadouble)(height)/ysize;    
      break;
    }
  }
#endif /* __CYGWIN32__ */

  /* Now ready for drawing, so we can exit. */

  drwbl = win;   /* Initial drawable is the visible window */
  dblmode = 0;   /* Initially no double buffering mode     */

  xsetw.backing_store = Always;
  XChangeWindowAttributes (display, win, CWBackingStore, &xsetw);

  /* Set up a font */

  font1 = NULL;
  font2 = NULL;
  font3 = NULL;
  font1i = NULL;
  font2i = NULL;
  font3i = NULL;

  xfnam = gxgsym("GAXFS");
  if (xfnam) flist = XListFonts (display, xfnam, 1, &i);
  else flist = NULL;
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-r-normal-*-80*", 1, &i);
  }
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-r-normal-*-100*", 1, &i);
  }
  if (flist==NULL) {
    font1 = XLoadQueryFont (display, "fixed");
  } else {
    font1 = XLoadQueryFont (display, *flist);
    if (font1==NULL) printf ("ERROR: Unable to open a basic font!!!\n");
    XFreeFontNames (flist);
  }

  xfnam = gxgsym("GAXFSI");
  if (xfnam) flist = XListFonts (display, xfnam, 1, &i);
  else flist = NULL;
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-o-normal-*-80*", 1, &i);
  }
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-o-normal-*-100*", 1, &i);
  }
  if (flist==NULL) {
    font1i = XLoadQueryFont (display, "fixed");
  } else {
    font1i = XLoadQueryFont (display, *flist);
    if (font1i==NULL) font1i = font1;
    XFreeFontNames (flist);
  }

  xfnam = gxgsym("GAXFM");
  if (xfnam) flist = XListFonts (display, xfnam, 1, &i);
  else flist = NULL;
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-r-normal--~-100*", 1, &i);
  }
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-r-normal-*-120*", 1, &i);
  }
  if (flist==NULL) {
    font2 = font1;
  } else {
    font2 = XLoadQueryFont (display, *flist);
    if (font2==NULL) font2 = font1;
    XFreeFontNames (flist);
  }
  xfnam = gxgsym("GAXFMI");
  if (xfnam) flist = XListFonts (display, xfnam, 1, &i);
  else flist = NULL;
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-o-normal--~-100*", 1, &i);
  }
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-o-normal-*-120*", 1, &i);
  }
  if (flist==NULL) {
    font2i = font1i;
  } else {
    font2i = XLoadQueryFont (display, *flist);
    if (font2i==NULL) font2i = font1i;
    XFreeFontNames (flist);
  }

  xfnam = gxgsym("GAXFL");
  if (xfnam) flist = XListFonts (display, xfnam, 1, &i);
  else flist = NULL;
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-r-normal-*-140*", 1, &i);
  }
  if (flist==NULL) {
    font3 = font2;
  } else {
    font3 = XLoadQueryFont (display, *flist);
    if (font3==NULL) font3 = font2;
    XFreeFontNames (flist);
  }
  xfnam = gxgsym("GAXFLI");
  if (xfnam) flist = XListFonts (display, xfnam, 1, &i);
  else flist = NULL;
  if (flist==NULL) {
    flist = XListFonts (display, "-adobe-helvetica-bold-o-normal-*-140*", 1, &i);
  }
  if (flist==NULL) {
    font3i = font2i;
  } else {
    font3i = XLoadQueryFont (display, *flist);
    if (font3i==NULL) font3i = font2i;
    XFreeFontNames (flist);
  }
  cfont = 0;
  gxdsfn();

}

void gxdend (void) {
  XFreeGC(display, gc);
  XCloseDisplay(display);
}

/* Frame action.  Values for action are:
      0 -- new frame (clear display), wait before clearing.
      1 -- new frame, no wait.
      2 -- New frame in double buffer mode.
      7 -- new frame, but just clear graphics.  Do not clear
           event queue; redraw widgets.
      8 -- clear only the event queue.
      9 -- flush the X request buffer */

void gxdfrm (gaint iact) {
struct gevent *geve, *geve2;
gaint i;
  if (iact==9) {
    gxdeve(0);
    XFlush(display);
    return;
  }
  if (iact==0 || iact==1 || iact==7) {
    devbck = gxdbkq();
    XSetForeground(display, gc, cvals[devbck]);
    XFillRectangle (display, drwbl, gc, 0, 0, width, height);
    XSetForeground(display, gc, cvals[lcolor]);
    for (i=0; i<512; i++) obj[i].type = -1;
    obnum = 0;
  }

  /* Flush X event queue.  If iact is 7, keep the event info, otherwise discard it. */
  if (iact==7) gxdeve(0);
  else {
    while (XCheckMaskEvent(display, ButtonReleaseMask | ButtonPressMask |
             ButtonMotionMask | KeyPressMask |
             ExposureMask | StructureNotifyMask, &report)) {
      if (report.type==ConfigureNotify) {
        if(width!=report.xconfigure.width||height!=report.xconfigure.height){
          width = report.xconfigure.width;
          height = report.xconfigure.height;
          xscl = (gadouble)(width)/xsize;
          yscl = (gadouble)(height)/ysize;
          gxdsfn();
          if (iact==8) gxdrdw();
        }
      }
    }

    /* Flush GrADS event queue */
    geve = evbase;
    while (geve) {
      geve2 = geve->forw;
      free (geve);
      geve = geve2;
    }
    evbase = NULL;
  }

  /* Reset all widgets if appropriate. */

  if (iact<7 && iact!=2) gxrswd(0);

  /* Redraw all widgets if appropriate.*/

  if (iact==7) {
    gxrdrw(0);
    XFlush(display);
  }
}

/* Examine X event queue.  Flag tells us if we should wait for an
   event.  Any GrADS events (mouse-button presses) are queued.
   If flag is 2, wait for any event, not just a mouse event.  */

void gxdeve (gaint flag) {
struct gevent *geve, *geve2;
gaint i,j,ii,rc,wflg,button,eflg,idm,rdrflg;

  if (flag && evbase) flag = 0;   /* Don't wait if an event stacked */
  wflg = flag;
  eflg = 0;
  rdrflg = 0;
  while (1) {
    if (wflg && !rdrflg) {
      XMaskEvent(display, ButtonReleaseMask | ButtonPressMask |
         ButtonMotionMask | KeyPressMask | ExposureMask | StructureNotifyMask, &report);
      rc = 1;
    } else {
      rc = XCheckMaskEvent(display, ButtonReleaseMask | ButtonPressMask |
         ButtonMotionMask | KeyPressMask | ExposureMask | StructureNotifyMask, &report);
    }
    if (!rc && rdrflg) {
      gxdsfn();
      gxdrdw();
      rdrflg = 0;
      continue;
    }
    if (!rc) break;

    switch  (report.type) {
    case ButtonRelease:  
      break;
    case Expose:
      if (excnt>0) excnt--;
      else if (!bsflg) rdrflg = 1;
      break;
    case ButtonPress:
      geve = (struct gevent *)malloc(sizeof(struct gevent));
      if (geve==NULL) {
        printf ("Memory allocation error in event queue!!!!!!\n");
        eflg = 1;
        break;
      }
      if (evbase==NULL) evbase = geve;
      else {
        geve2 = evbase;
        while (geve2->forw) geve2 = geve2->forw;
        geve2->forw = geve;
      }
      geve->forw = NULL;
      i = report.xbutton.x;
      j = report.xbutton.y;
      button = report.xbutton.button;
      if (button==Button1) button=1;
      else if (button==Button2) button=2;
      else if (button==Button3) button=3;
      else if (button==Button4) button=4;
      else if (button==Button5) button=5;
      geve->mbtn = button;
      geve->x = xsize*((gadouble)i)/width;
      geve->y = ysize - ysize*((gadouble)j)/height;
      geve->type = 0;
      /* Scan to see if point-click event was on one of our
         widgets.  Handling depends on what is clicked on. */
      ii = 0;
      while (ii<512 && obj[ii].type>-1) {
        if (obj[ii].type!=0 && i>obj[ii].i1 && i<obj[ii].i2 &&
                               j>obj[ii].j1 && j<obj[ii].j2) {
          if (obj[ii].mb < 0 || obj[ii].mb == button) {
            if (obj[ii].type==1) gxevbn(geve,ii);
            else if (obj[ii].type==2) gxevrb(geve,ii,i,j);
            else if (obj[ii].type==3) {
              idm = ii;
              while (idm != -999) idm = gxevdm(geve,idm,i,j);
            }
            ii = 100000;           /* Exit loop */
          }
        }
        ii++;
      }
      wflg = 0;                  /* Check for more events, but don't
                                    wait if there aren't any. */
      break;
    case ConfigureNotify:
      if(width!=report.xconfigure.width||height!=report.xconfigure.height){
        width = report.xconfigure.width;
        height = report.xconfigure.height;
        xscl = (gadouble)(width)/xsize;
        yscl = (gadouble)(height)/ysize;
        rdrflg = 1;
        if (flag==2) wflg = 0;
      }
      break;
    }
    if (eflg) break;
  }
  if (rdrflg) {
    gxdsfn();
    gxdrdw();
  }
}

/* Return info on mouse button press event.  Wait if requested. */

void gxdbtn (gaint flag, gadouble *xpos, gadouble *ypos,
	     gaint *mbtn, gaint *type, gaint *info, gadouble *rinfo) {
struct gevent *geve;
gaint i;

  if (batch) {
    *xpos = -999.9;
    *ypos = -999.9;
    return;
  }
  gxdeve(flag);
  if (evbase==NULL) {
    *xpos = -999.9;
    *ypos = -999.9;
    *mbtn = -1;
    *type = -1;
  } else {
    geve = evbase;
    *xpos = geve->x;
    *ypos = geve->y;
    *mbtn = geve->mbtn;
    *type = geve->type;
    for (i=0; i<10; i++) *(info+i) = geve->info[i];
    for (i=0; i<4; i++) *(rinfo+i) = geve->rinfo[i];
    evbase = geve->forw;       /* Take even off queue */
    free(geve);
  }

}

/* sets color to new value */

void gxdcol (gaint clr) {
  if (clr<0) clr=0;
  if (clr>255) clr=255;
  devbck = gxdbkq();
  if (devbck) {
    if (clr==0) clr = 1;
    else if (clr==1) clr = 0;
  }
  if (!cused[clr] && !cmach[clr]) clr=15; 
  XSetForeground(display, gc, cvals[clr]);
  lcolor=clr;
}

/* define a new color */ 

gaint gxdacol(gaint clr, gaint red, gaint green, gaint blue, gaint alpha) {
struct gxdbquery dbq;
XStandardColormap best;
gaint screen_num = DefaultScreen(display);
gaint rc;

  if (clr<16 || clr>255) return(1);
  if (cused[clr]) {
    /* if this color was previously defined, release */
    XFreeColors(display, cmap, &(cvals[clr]),1,0);
    cused[clr]=0;
  }
  /* get rgb values for this color from the graphics database */
  gxdbqcol(clr, &dbq);
  cell.red   = dbq.red*256;
  cell.blue  = dbq.blue*256;
  cell.green = dbq.green*256;

  cmach[clr]=0;
  rc=2;
  if (XAllocColor(display, cmap, &cell)) {
    /* success! */
    cvals[clr] = cell.pixel;
    cused[clr] = 1;
    rc=0;
  } 
  else if (XGetStandardColormap(display,RootWindow(display,screen_num),&best,XA_RGB_BEST_MAP)) {
    /* try best match from standard colormap */
    if (gxbcol(&best, &cell)) {
      /* plan B success! */
      cvals[clr] = cell.pixel;
      cmach[clr] = 1;
      rc=1;
    } 
    else printf ("Color Matching Error.  Color number = %i\n",clr);
  } 
  else printf ("Color Map Allocation Error.  Color number = %i\n",clr);
  return(rc);
}


/* Assign best rgb to color number from standard colormap */
gaint gxbcol (XStandardColormap* best, XColor * cell) {
  XColor color, colors[256];
  unsigned long bestpixel=0;
  gaint d, i;
  gaint min;
 
  for (i=0; i<256; i++) {
    colors[i].pixel = i; 
  }
  XQueryColors(display, cmap, colors, 256);
  min = 65536;
  for (i=0; i<256; i++) {
    d = abs(cell->red - colors[i].red)
      + abs(cell->green - colors[i].green) 
      + abs(cell->blue - colors[i].blue);
    if (d<min) { min = d;  bestpixel = i; }
  }
  if (bestpixel < 256) {
    cell->pixel = bestpixel; 
    color.pixel = bestpixel; 
    XQueryColor(display, cmap, &color);
    return 1;
  } else {
    return 0;
  }	
}

void gxdwid (gaint wid){                 /* Set width     */
gauint lw;
  lwidth=wid;
  lw = 0;
  if (lwidth>5) lw=2;
  if (lwidth>11) lw=3;
  if (lw != owidth) {
    XSetLineAttributes(display, gc, lw, LineSolid,
                     CapButt, JoinBevel);
  }
  owidth = lw;
}

void gxdmov (gadouble x, gadouble y){        /* Move to x,y   */
  xxx = (gaint)(x*xscl+0.5);
  yyy = height - (gaint)(y*yscl+0.5);
}

void gxddrw (gadouble x, gadouble y){        /* Draw to x,y   */
gaint i, j;
  i = (gaint)(x*xscl+0.5);
  j = height - (gaint)(y*yscl+0.5);
  XDrawLine (display, drwbl, gc, xxx, yyy, i, j);
  xxx = i;
  yyy = j;
  if (QLength(display)&&rstate) gxdeve(0);
}

/* Draw a filled rectangle */
void gxdrec (gadouble x1, gadouble x2, gadouble y1, gadouble y2) {
gaint i1,i2,j1,j2;

  i1 = (gaint)(x1*xscl+0.5);
  j1 = height - (gaint)(y1*yscl+0.5);
  i2 = (gaint)(x2*xscl+0.5);
  j2 = height - (gaint)(y2*yscl+0.5);
  if (i1!=i2 && j1!=j2) {
    XFillRectangle (display, drwbl, gc, i1, j2, i2-i1, j1-j2);
  } else {
    XDrawLine (display, drwbl, gc, i1, j1, i2, j2);
  }
  if (QLength(display)&&rstate) gxdeve(0);
}


void gxdsgl (void) {
gaint i;
  if (dblmode) {
    gxrswd(1);
    for (i=0; i<512; i++) { obj[i].type=-1; obj2[i].type = -1;}
    obnum = 0;  obnum2 = 0;
    XFreePixmap (display, pmap);
    drwbl = win;
  }
  dblmode = 0;
  return;
}

void gxddbl (void) {
gaint i;
  pmap = XCreatePixmap (display, win, width, height, depth);
  XSync(display, 0) ; /* hoop */
  if (pmap==(Pixmap)NULL) {
    printf ("Error allocating pixmap for animation mode\n");
    printf ("Animation mode will not be enabled\n");
    return;
  }
  dblmode = 1;
  drwbl = pmap;
  devbck = gxdbkq();
  XSetForeground(display, gc, cvals[devbck]);
  XFillRectangle (display, drwbl, gc, 0, 0, width, height);
  XSetForeground(display, gc, cvals[lcolor]);
  gxrswd(1);  /* Reset all widgets */
  for (i=0; i<512; i++) { obj[i].type=-1; obj2[i].type = -1;}
  obnum = 0;  obnum2 = 0;
  return;
}


void gxdswp (void) {
  if (dblmode) {
    XCopyArea (display, pmap, win, gc, 0, 0, width, height, 0, 0);
  }
  devbck = gxdbkq();
  XSetForeground(display, gc, cvals[devbck]);
  XFillRectangle (display, drwbl, gc, 0, 0, width, height);
  XSetForeground(display, gc, cvals[lcolor]);
  gxrswd(0);
  gxcpwd();
  gxrswd(2);
  return;
}

void gxdfil (gadouble *xy, gaint n) {
gadouble *pt;
gaint i;
XPoint *pnt;

  point = (XPoint *)malloc(sizeof(XPoint)*n);
  if (point==NULL) {
    printf ("Error in polygon fill routine gxdfil: \n");
    printf ("   Unable to allocate enough memory for the request\n");
    return;
  }
  pnt = point;
  pt = xy;
  for (i=0; i<n; i++) {
    pnt->x = (gaint)(*pt*xscl+0.5);
    pnt->y = height - (gaint)(*(pt+1)*yscl+0.5);
    pt+=2;
    pnt++;
  }
  XFillPolygon (display, drwbl, gc, point, n, Nonconvex, CoordModeOrigin);
  free (point);
  if (QLength(display)&&rstate) gxdeve(0);
  return;
}

void gxdxsz (gaint xx, gaint yy) {
  if (batch) return;
  if (xx==width && yy==height) return;
  XResizeWindow (display, win, xx, yy);
  gxdeve(2);
}


/* Routine to display a button widget */
/* Flags are cumbersome, sigh....
      redraw -- indicates the button is being redrawn, probably
                due to a resize event.  When set, the assumption
                is that *pbn is NULL and is ignored
      btnrel -- indicates the button is being redrawn in a new state
                due to a buttonpress/buttonrelease event.
      nstat  -- forces the state to go to this new setting.
                Used for 'redraw button' command.  */

void gxdpbn (gaint bnum, struct gbtn *pbn, gaint redraw, gaint btnrel, gaint nstat) {
gaint i, j, w, h, ilo, ihi, jlo, jhi, ccc, len;
struct gbtn *gbb;
struct gobj *pob=NULL;
  if (bnum<0 || bnum>255) return;
  if (dblmode) {
    gbb = &(btn2[bnum]);
    if (btnrel) {
      drwbl = win;
      gbb = &(btn[bnum]);
    }
  } else gbb = &(btn[bnum]);
  if (!redraw) {
    *gbb = *pbn;
    gbb->num = bnum;
  }
  if (!redraw || rstate==0) {
    if (dblmode) {
      if (obnum2>511) {
        printf ("Error: Too many widgets on screen\n");
        return;
      }
    } else {
      if (obnum>511) {
        printf ("Error: Too many widgets on screen\n");
        return;
      }
    }
    if (dblmode) {pob = &(obj2[obnum2]); obnum2++;}
    else {pob = &(obj[obnum]); obnum++;}
  }
  if (gbb->num<0) return;
  if (nstat>-1) gbb->state = nstat;
  if (redraw>1) {
    if (pbn->ch) {
      if (gbb->ch) gree(gbb->ch,"f500");
      gbb->ch = pbn->ch;
      gbb->len = pbn->len;
    }
    if (redraw==3) {
      gbb->fc = pbn->fc; gbb->bc = pbn->bc;
      gbb->oc1 = pbn->oc1; gbb->oc2 = pbn->oc2;
      gbb->ftc = pbn->ftc; gbb->btc = pbn->btc;
      gbb->otc1 = pbn->otc1; gbb->otc2 = pbn->otc2;
    }
  }
  i = (gaint)(gbb->x*xscl+0.5);
  j = height - (gaint)(gbb->y*yscl+0.5);
  w = (gaint)(gbb->w*xscl+0.5);
  h = (gaint)(gbb->h*yscl+0.5);
  w = w - 2;
  h = h - 2;
  gbb->ilo = 1 + i - w/2;
  gbb->jlo = 1 + j - h/2;
  gbb->ihi = gbb->ilo + w;
  gbb->jhi = gbb->jlo + h;
  ilo = gbb->ilo;  ihi = gbb->ihi;
  jlo = gbb->jlo;  jhi = gbb->jhi;
  if (gbb->state) ccc = gbb->btc;
  else ccc = gbb->bc;
  if (ccc>-1) {
    gxdcol(ccc);
    XFillRectangle (display, drwbl, gc, gbb->ilo, gbb->jlo, w, h);
  }
  gxdwid(gbb->thk);
  if (gbb->state) ccc = gbb->otc1;
  else ccc = gbb->oc1;
  if (ccc>-1) {
    gxdcol(ccc);
    XDrawLine (display, drwbl, gc, ilo, jhi, ihi, jhi);
    XDrawLine (display, drwbl, gc, ihi, jhi, ihi, jlo);
  }
  if (gbb->state) ccc = gbb->otc2;
  else ccc = gbb->oc2;
  if (ccc>-1) {
    gxdcol(ccc);
    XDrawLine (display, drwbl, gc, ihi, jlo, ilo, jlo);
    XDrawLine (display, drwbl, gc, ilo, jlo, ilo, jhi);
  }
  if (gbb->state) ccc = gbb->ftc;
  else ccc = gbb->fc;
  if (ccc>-1) {
    len = 0;
    while (*(gbb->ch+len)) len++;
/*    len++;*/
    gxdcol(ccc);
    if (gfont==1 && font1) {
      XSetFont (display, gc, font1->fid);
      w = XTextWidth(font1, gbb->ch, len);
      i = i - w/2;
      j = j + 5*font1->ascent/9;
    }
    if (gfont==2 && font2) {
      XSetFont (display, gc, font2->fid);
      w = XTextWidth(font2, gbb->ch, len);
      i = i - w/2;
      j = j + 5*font2->ascent/9;
    }
    if (gfont==3 && font3) {
      XSetFont (display, gc, font3->fid);
      w = XTextWidth(font3, gbb->ch, len);
      i = i - w/2;
      j = j + 5*font3->ascent/9;
    }
    XDrawString(display, drwbl, gc, i, j, gbb->ch, len);
  }
  gxdcol(lcolor);
  if (dblmode && btnrel) drwbl = pmap;
  if (!redraw || rstate==0) {
    pob->type = 1;
    pob->mb = -1;
    pob->i1 = ilo;  pob->i2 = ihi;
    pob->j1 = jlo;  pob->j2 = jhi;
    pob->iob.btn = gbb;
  }
  XFlush(display);
}

/* Routine to display a drop menu widget:
      redraw -- indicates the button is being redrawn, probably
                due to a resize event.  When set, the assumption
                is that *dmu is NULL and is ignored; info for
                redrawing is obtained from the existing
                structure list.
      nstat  -- re-defines the dropmenu.
                Used for 'redraw dropmenu' command.  */

void gxdrmu (gaint mnum, struct gdmu *pmu, gaint redraw, gaint nstat) {
gaint i, j, w, h, ilo, ihi, jlo, jhi, len, lw;
struct gdmu *gmu;
struct gobj *pob=NULL;

  if (mnum<0 || mnum>199) return;
  if (dblmode) gmu = &(dmu2[mnum]);
  else gmu = &(dmu[mnum]);
  if (!redraw) {
    *gmu = *pmu;
    gmu->num = mnum;
  }
  if (gmu->num<0) return;
  if (gmu->casc) return;
  if (!redraw || rstate==0) {
    if (dblmode) {
      if (obnum2>511) {
        printf ("Error: Too many widgets on screen\n");
        return;
      }
    } else {
      if (obnum>511) {
        printf ("Error: Too many widgets on screen\n");
        return;
      }
    }
    if (dblmode) {pob = &(obj2[obnum2]); obnum2++;}
    else {pob = &(obj[obnum]); obnum++;}
  }
  i = (gaint)(gmu->x*xscl+0.5);
  j = height - (gaint)(gmu->y*yscl+0.5);
  w = (gaint)(gmu->w*xscl+0.5);
  h = (gaint)(gmu->h*yscl+0.5);
  w = w - 2;
  h = h - 2;
  gmu->ilo = 1 + i - w/2;
  gmu->jlo = 1 + j - h/2;
  gmu->ihi = gmu->ilo + w;
  gmu->jhi = gmu->jlo + h;
  ilo = gmu->ilo;  ihi = gmu->ihi;
  jlo = gmu->jlo;  jhi = gmu->jhi;
  if (gmu->bc>-1) {
    gxdcol(gmu->bc);
    XFillRectangle (display, drwbl, gc, ilo, jlo, w+1, h+1);
  }
  lw = 1;
  if (gmu->thk>5) lw = 2;
  if (gmu->thk>12) lw = 3;
  gxdwid(1);
  if (gmu->oc1>-1) {
    gxdcol(gmu->oc1);
    XDrawLine (display, drwbl, gc, ilo, jhi, ihi, jhi);
    if (lw>1) XDrawLine (display, drwbl, gc, ilo+1, jhi-1, ihi-1, jhi-1);
    if (lw>2) XDrawLine (display, drwbl, gc, ilo+2, jhi-2, ihi-2, jhi-2);
    XDrawLine (display, drwbl, gc, ihi, jhi, ihi, jlo);
    if (lw>1) XDrawLine (display, drwbl, gc, ihi-1, jhi-1, ihi-1, jlo+1);
    if (lw>2) XDrawLine (display, drwbl, gc, ihi-2, jhi-2, ihi-2, jlo+2);
  }
  if (gmu->oc2>-1) {
    gxdcol(gmu->oc2);
    XDrawLine (display, drwbl, gc, ihi, jlo, ilo, jlo);
    if (lw>1) XDrawLine (display, drwbl, gc, ihi-1, jlo+1, ilo+1, jlo+1);
    if (lw>2) XDrawLine (display, drwbl, gc, ihi-2, jlo+2, ilo+2, jlo+2);
    XDrawLine (display, drwbl, gc, ilo, jlo, ilo, jhi);
    if (lw>1) XDrawLine (display, drwbl, gc, ilo+1, jlo+1, ilo+1, jhi-1);
    if (lw>2) XDrawLine (display, drwbl, gc, ilo+2, jlo+2, ilo+2, jhi-2);
  }
  if (gmu->fc>-1) {
    len = 0;
    while (*(gmu->ch+len)) len++;
/*    len++;*/
    gxdcol(gmu->fc);
    if (gfont==1 && font1i) {
      XSetFont (display, gc, font1i->fid);
      w = XTextWidth(font1i, gmu->ch, len);
      i = ilo + font1i->ascent/2;
      j = j + 5*font1i->ascent/9;
    }
    if (gfont==2 && font2i) {
      XSetFont (display, gc, font2i->fid);
      w = XTextWidth(font2i, gmu->ch, len);
      i = ilo + font2i->ascent/2;
      j = j + 5*font2i->ascent/9;
    }
    if (gfont==3 && font3i) {
      XSetFont (display, gc, font3i->fid);
      w = XTextWidth(font3i, gmu->ch, len);
      i = ilo + font3i->ascent/2;
      j = j + 5*font3i->ascent/9;
    }
    XDrawString(display, drwbl, gc, i, j, gmu->ch, len);
    if (gfont==1 && font1) XSetFont (display, gc, font1->fid);
    if (gfont==2 && font2) XSetFont (display, gc, font2->fid);
    if (gfont==3 && font3) XSetFont (display, gc, font3->fid);
  }
  gxdcol(lcolor);
  if (!redraw || rstate==0) {
    pob->type = 3;
    pob->mb = -1;
    pob->i1 = ilo;  pob->i2 = ihi;
    pob->j1 = jlo;  pob->j2 = jhi;
    pob->iob.dmu = gmu;
  }
  XFlush(display);
}

/* Select font based on screen size */

void gxdsfn(void) {

  if (width<601 || height<421) {
    if (gfont!=1) {
      if (font1) XSetFont (display, gc, font1->fid);
      gfont = 1;
    }
  } else if (width<1001 || height<651) {
    if (gfont!=2) {
      if (font2) XSetFont (display, gc, font2->fid);
      gfont = 2;
    }
  } else {
    if (gfont!=3) {
      if (font3) XSetFont (display, gc, font3->fid);
      gfont = 3;
    }
  }
}


/* Attempt to redraw when user resizes window */

void gxdrdw (void) {
int i;
  rstate = 0;
  devbck = gxdbkq();
  XSetForeground(display, gc, cvals[devbck]);
  XFillRectangle (display, drwbl, gc, 0, 0, width, height);
  XSetForeground(display, gc, cvals[lcolor]);
  for (i=0; i<512; i++) obj[i].type = -1;
  obnum = 0;
  if (dblmode) {
    dblmode = 0;
    XFreePixmap (display, pmap);
    pmap = XCreatePixmap (display, win, width, height, depth);
    if (pmap==(Pixmap)NULL) {
      printf ("Error allocating pixmap for resize operation\n");
      printf ("Animation mode will be disabled\n");
      dblmode = 0;
      drwbl = win;
      rstate = 1;
      return;
    }
    drwbl = win;
    devbck = gxdbkq();
    XSetForeground(display, gc, cvals[devbck]);
    XFillRectangle (display, drwbl, gc, 0, 0, width, height);
    XFillRectangle (display, pmap, gc, 0, 0, width, height);
    XSetForeground(display, gc, cvals[lcolor]);
    for (i=0; i<512; i++) obj2[i].type = -1;
    obnum2 = 0;
    gxhdrw(1,0);
    gxrdrw(1);
    dblmode = 1;
    drwbl = pmap;
  }
  gxhdrw(0,0);
  gxrdrw(0);
  rstate = 1;
}

/* Redraw all widgets.  Flag indicates whether to redraw
   foreground or background widgets. */

void gxrdrw (int flag) {
int i;
  if (flag) {
    for (i=0; i<256; i++) {
      if (btn2[i].num>-1) gxdpbn(i, NULL, 1, 0, -1);
    }
    for (i=0; i<32; i++) {
      if (rbb2[i].num>-1) gxdrbb(i,rbb2[i].type,rbb2[i].xlo,rbb2[i].ylo,rbb2[i].xhi,rbb2[i].yhi,rbb2[i].mb);
    }
    for (i=0; i<200; i++) {
      if (dmu2[i].num>-1) gxdrmu(i, NULL, 1, -1);
    }
  } else {
    for (i=0; i<256; i++) {
      if (btn[i].num>-1) gxdpbn(i, NULL, 1, 0, -1);
    }
    for (i=0; i<32; i++) {
      if (rbb[i].num>-1) gxdrbb(i,rbb[i].type,rbb[i].xlo,rbb[i].ylo,rbb[i].xhi,rbb[i].yhi,rbb[i].mb);
    }
    for (i=0; i<200; i++) {
      if (dmu[i].num>-1) gxdrmu(i, NULL, 1, -1);
    }
  }
}

/* Reset all widgets; release memory as appropriate. */
/* flag = 0 resets foreground, flag = 1 resets both,
   flag = 2 resets background only; for after swapping */

void gxrswd(gaint flag) {
int i;

  if (flag!=2) {
    for (i=0; i<256; i++) {
      if (btn[i].num>-1 && btn[i].ch!=NULL) gree(btn[i].ch,"f501");
      btn[i].num = -1;
      btn[i].ch = NULL;
    }
    for (i=0; i<200; i++) {
      if (dmu[i].num>-1 && dmu[i].ch!=NULL) gree(dmu[i].ch,"f502");
      dmu[i].num = -1;
      dmu[i].ch = NULL;
    }
    for (i=0; i<32; i++) rbb[i].num = -1;
  }

  if (flag) {
    for (i=0; i<256; i++) {
      if (flag!=2) {
        if (btn2[i].num>-1 && btn2[i].ch!=NULL) gree(btn2[i].ch,"f503");
      }
      btn2[i].num = -1;
      btn2[i].ch = NULL;
    }
    for (i=0; i<200; i++) {
      if (flag!=2) {
        if (dmu2[i].num>-1 && dmu2[i].ch!=NULL) gree(dmu2[i].ch,"f504");
      }
      dmu2[i].num = -1;
      dmu2[i].ch = NULL;
    }
    for (i=0; i<32; i++) rbb2[i].num = -1;
  }
}


/* Copy all widgets during swap in double buffer mode */

void gxcpwd(void) {
struct grbb *grb;
struct gbtn *gbn;
struct gdmu *gmu;
int i;

  for (i=0; i<256; i++) {
    if (btn2[i].num>-1) btn[i] = btn2[i];
  }

  for (i=0; i<200; i++) {
    if (dmu2[i].num>-1) dmu[i] = dmu2[i];
  }

  for (i=0; i<32; i++) {
    if (rbb2[i].num>-1) rbb[i] = rbb2[i];
  }

  /* Rebuild list of currently displayed items */

  for (i=0; i<512; i++) obj[i].type = -1;
  obnum = obnum2;
  for (i=0; i<obnum; i++) {
    obj[i] = obj2[i];
    if (obj[i].type==1) {
      gbn = obj[i].iob.btn;
      obj[i].iob.btn = &(btn[gbn->num]);
    } else if (obj[i].type==2) {
      grb = obj[i].iob.rbb;
      obj[i].iob.rbb = &(rbb[grb->num]);
    } else if (obj[i].type==3) {
      gmu = obj[i].iob.dmu;
      obj[i].iob.dmu = &(dmu[gmu->num]);
    }
  }
  for (i=0; i<512; i++) obj2[i].type = -1;
  obnum2 = 0;
}

/* Reset a particular widget, given widget type and number */
/* Assumes arrays are used for holding all the widget info */

void gxrs1wd (int wdtyp, int wdnum) {
struct grbb *grb;
struct gbtn *gbn;
struct gdmu *gmu;
int ii,jj=0;

  if (wdtyp<1 || wdtyp>3) return;
  if (wdtyp==1 && (wdnum<0 || wdnum>255)) return;
  if (wdtyp==2 && (wdnum<0 || wdnum>31)) return;
  if (wdtyp==3 && (wdnum<0 || wdnum>199)) return;

  /* Remove this widget from the list of displayed items */

  ii = 0;
  while (ii<512 && obj[ii].type>-1) {
    if (obj[ii].type!=0 && obj[ii].type==wdtyp) {
      if (obj[ii].type==1) {
        gbn = obj[ii].iob.btn;
        jj = gbn->num;
      } else if (obj[ii].type==2) {
        grb = obj[ii].iob.rbb;
        jj = grb->num;
      } else if (obj[ii].type==3) {
        gmu = obj[ii].iob.dmu;
        jj = gmu->num;
      }
      if (jj==wdnum) {
        obj[ii].type = 0;   /* This should be enough to cause this */
                            /* widget to be ignored. */
        ii = 100000;   /* Exit loop */
      }
    }
    ii++;
  }

  /* Remove this widget from the widget array */

  if (wdtyp==1) {
    if (btn[wdnum].num>-1 && btn[wdnum].num != wdnum)  {
       printf ("Logic Error 64 in gxrs1wd\n");
    }
    if (btn[wdnum].num>-1 && btn[wdnum].ch!=NULL) gree(btn[wdnum].ch,"f505");
    btn[wdnum].num = -1;
    btn[wdnum].ch = NULL;
  } else if (wdtyp==2) {
    if (rbb[wdnum].num>-1 && rbb[wdnum].num != wdnum) {
       printf ("Logic Error 65 in gxrs1wd\n");
    }
    rbb[wdnum].num = -1;
  } else if (wdtyp==3) {
    if (dmu[wdnum].num>-1 && dmu[wdnum].num != wdnum) {
       printf ("Logic Error 65 in gxrs1wd\n");
    }
    if (dmu[wdnum].num>-1 && dmu[wdnum].ch!=NULL) gree(dmu[wdnum].ch,"f506");
    dmu[wdnum].num = -1;
    dmu[wdnum].ch = NULL;
  }
}


/* Click occurred over a button object. */

void gxevbn(struct gevent *geve, int iobj) {
struct gbtn *gbn;
int jj,c1,c2,i1,i2,j1,j2;

  /* Fill in button specific event info */

  geve->type = 1;
  gbn = obj[iobj].iob.btn;
  geve->info[0] = gbn->num;

  jj = gbn->num;
  if (btn[jj].state) {
    c1=btn[jj].otc1; c2=btn[jj].otc2;
    geve->info[1] = 0;
  } else {
    c1=btn[jj].oc1; c2=btn[jj].oc2;
    geve->info[1] = 1;
  }

  /* Redraw button outline as pressed, if appropriate
     (ie, if the outline colors are different) */

  i1=btn[jj].ilo; i2=btn[jj].ihi;
  j1=btn[jj].jlo; j2=btn[jj].jhi;
  if ( !(btn[jj].state && btn[jj].oc1==btn[jj].otc2 &&
         btn[jj].oc2==btn[jj].otc1)) {
    if (c2>-1 && c1>-1) {
      gxdwid(btn[jj].thk);
      gxdcol(c2);
      XDrawLine (display, win, gc, i1, j2, i2, j2);
      XDrawLine (display, win, gc, i2, j2, i2, j1);
      gxdcol(c1);
      XDrawLine (display, win, gc, i2, j1, i1, j1);
      XDrawLine (display, win, gc, i1, j1, i1, j2);
      XFlush(display);
    }
  }

  /* Wait for button release, and do final redraw
     of button with new state. */

  while (1) {
    XMaskEvent(display, ButtonReleaseMask | ButtonMotionMask, &report);
    if (report.type == ButtonRelease) break;
  }

  if (btn[jj].state) btn[jj].state=0;
  else btn[jj].state=1;
  gxdpbn (jj, NULL, 1, 1, -1);
  XFlush(display);
}

/* Click occurred in a rubber-banded region.  */

void gxevrb(struct gevent *geve, int iobj, int i, int j) {
struct grbb *grb;
int i1,i2,j1,j2,i1o,j1o,i2o,j2o,xoflg,typ;
int ilo,ihi,jlo,jhi;

  i1o = i2o = j1o = j2o = 0;
  /* Get rest of event info */
  geve->type = 2;
  grb = obj[iobj].iob.rbb;
  geve->info[0] = grb->num;
  typ = grb->type;
  ilo = obj[iobj].i1;  ihi = obj[iobj].i2;
  jlo = obj[iobj].j1;  jhi = obj[iobj].j2;

  /* Set foreground color to something that will show up when Xor'd */

  XSetForeground(display, gc, cvals[0]^cvals[1]);
  XSetFunction(display, gc, GXxor);

  /* Loop on button motion, waiting for button release */

  xoflg = 0;
  while (1) {
    XMaskEvent(display, ButtonReleaseMask|ButtonMotionMask, &report);
    if (report.type==MotionNotify) {
      if (xoflg) {
        if (typ==1)
           XDrawRectangle(display, win, gc, i1o, j1o, i2o-i1o, j2o-j1o);
        else XDrawLine (display, win, gc, i1o, j1o, i2o, j2o);
      }
      if (i<report.xmotion.x) { i1 = i; i2 = report.xmotion.x; }
      else { i2 = i; i1 = report.xmotion.x; }
      if (j<report.xmotion.y) { j1 = j; j2 = report.xmotion.y; }
      else { j2 = j; j1 = report.xmotion.y; }
      if (i1<ilo) i1 = ilo; if (i2>ihi) i2 = ihi;
      if (j1<jlo) j1 = jlo; if (j2>jhi) j2 = jhi;
      if (typ==1) XDrawRectangle (display, win, gc, i1, j1, i2-i1, j2-j1);
      else XDrawLine (display, win, gc, i1, j1, i2, j2);
      i1o=i1; i2o=i2; j1o=j1; j2o=j2; xoflg = 1;
    } else break;
  }
  if (xoflg) {
    if (typ==1)
       XDrawRectangle(display, win, gc, i1o, j1o, i2o-i1o, j2o-j1o);
    else XDrawLine (display, win, gc, i1o, j1o, i2o, j2o);
  }
  XSetForeground(display, gc, cvals[lcolor]);
  XSetFunction(display, gc, GXcopy);
  XFlush(display);
  i1 = report.xbutton.x; j1 = report.xbutton.y;
  if (i1<ilo) i1 = ilo; if (i1>ihi) i1 = ihi;
  if (j1<jlo) j1 = jlo; if (j1>jhi) j1 = jhi;
  geve->rinfo[0] = xsize*((gadouble)i1)/width;
  geve->rinfo[1] = ysize - ysize*((gadouble)j1)/height;
}

/* Set up a rubber-band region.  */

void gxdrbb (gaint num, gaint type, gadouble xlo, gadouble ylo, gadouble xhi, gadouble yhi, gaint mbc) {
  struct grbb *prb;
  struct gobj *pob;

  if (num<0 || num>31) return;
  if (xlo>=xhi) return;
  if (ylo>=yhi) return;

  if (dblmode) {
    if (obnum2>511) return;
    pob = &(obj2[obnum2]); obnum2++;
    prb = &(rbb2[num]);
  } else {
    if (obnum>511) return;
    pob = &(obj[obnum]); obnum++;
    prb = &(rbb[num]);
  }

  pob->type = 2;
  pob->mb = mbc;
  pob->i1 = (gaint)(xlo*xscl+0.5);
  pob->i2 = (gaint)(xhi*xscl+0.5);
  pob->j1 = height - (gaint)(yhi*yscl+0.5);
  pob->j2 = height - (gaint)(ylo*yscl+0.5);
  pob->iob.rbb = prb;
  prb->num = num;
  prb->xlo = xlo; prb->xhi = xhi;
  prb->ylo = ylo; prb->yhi = yhi;
  prb->type = type;
  prb->mb = mbc;
}

/* Handle drop menu.  Create new (temporary) window and
   put the text in it.  Window is destroyed when the mouse
   button is released.  */

static int dmrecu,dmi1[4],dmi2[4],dmj1[4],dmj2[4],dmnum[4],dmsel[4],dmcur[4];

int gxevdm(struct gevent *geve, int iobj, int ipos, int jpos) {
struct gdmu *gmu;
int i,j,iorig,jorig,ival,ilo,ihi,jlo,jhi,w,h,len,lw;

  geve->type = 3;
  gmu = obj[iobj].iob.dmu;

  /* Redraw base using toggled colors */
  i=0;
  ilo = gmu->ilo; ihi = gmu->ihi;
  jlo = gmu->jlo; jhi = gmu->jhi;
  w = ihi - ilo;
  h = jhi - jlo;
  j = jlo + h/2 - 1;
  if (gmu->tbc>-1 && gmu->tfc>-1) {
    if (gmu->tbc>-1) {
      gxdcol(gmu->tbc);
      XFillRectangle (display, drwbl, gc, ilo, jlo, w+1, h+1);
    }
    if (gmu->tfc>-1) {
      len = 0;
      while (*(gmu->ch+len)) len++;
/*    len++;*/
      gxdcol(gmu->tfc);
      if (gfont==1 && font1i) {
        XSetFont (display, gc, font1i->fid);
        w = XTextWidth(font1i, gmu->ch, len);
        i = ilo + font1i->ascent/2;
        j = j + 5*font1i->ascent/9;
      }
      if (gfont==2 && font2i) {
        XSetFont (display, gc, font2i->fid);
        w = XTextWidth(font2i, gmu->ch, len);
        i = ilo + font2i->ascent/2;
        j = j + 5*font2i->ascent/9;
      }
      if (gfont==3 && font3i) {
        XSetFont (display, gc, font3i->fid);
        w = XTextWidth(font3i, gmu->ch, len);
        i = ilo + font3i->ascent/2;
        j = j + 5*font3i->ascent/9;
      }
      XDrawString(display, drwbl, gc, i, j, gmu->ch, len);
    }
  }
  gxdwid(1);
  lw = 1;
  if (gmu->thk>5) lw = 2;
  if (gmu->thk>11) lw = 3;
  if (gmu->toc1>-1) {
    gxdcol(gmu->toc1);
    XDrawLine (display, drwbl, gc, ilo, jhi, ihi, jhi);
    if (lw>1) XDrawLine (display, drwbl, gc, ilo+1, jhi-1, ihi-1, jhi-1);
    if (lw>2) XDrawLine (display, drwbl, gc, ilo+2, jhi-2, ihi-2, jhi-2);
    XDrawLine (display, drwbl, gc, ihi, jhi, ihi, jlo);
    if (lw>1) XDrawLine (display, drwbl, gc, ihi-1, jhi-1, ihi-1, jlo+1);
    if (lw>2) XDrawLine (display, drwbl, gc, ihi-2, jhi-2, ihi-2, jlo+2);
  }
  if (gmu->toc2>-1) {
    gxdcol(gmu->toc2);
    XDrawLine (display, drwbl, gc, ihi, jlo, ilo, jlo);
    if (lw>1) XDrawLine (display, drwbl, gc, ihi-1, jlo+1, ilo+1, jlo+1);
    if (lw>2) XDrawLine (display, drwbl, gc, ihi-2, jlo+2, ilo+2, jlo+2);
    XDrawLine (display, drwbl, gc, ilo, jlo, ilo, jhi);
    if (lw>1) XDrawLine (display, drwbl, gc, ilo+1, jlo+1, ilo+1, jhi-1);
    if (lw>2) XDrawLine (display, drwbl, gc, ilo+2, jlo+2, ilo+2, jhi-2);
  }

  /* Set up and display first menu */

  iorig = obj[iobj].i1;
  jorig = obj[iobj].j2;

  dmrecu = -1;
  for (i=0; i<4; i++) {
    dmi1[i] = -1; dmi2[i] = -1; dmj1[i] = -1; dmj2[i] = -1;
    dmnum[i] = -1; dmsel[i] = -1;
  }

  ival = gxpopdm (gmu, iobj, iorig, iorig, jorig);

  /* Display base in standard colors */

  gxdsfn();
  w = ihi - ilo;
  h = jhi - jlo;
  j = jlo + h/2 - 1;
  if (gmu->bc>-1 && gmu->fc>-1) {
    if (gmu->bc>-1) {
      gxdcol(gmu->bc);
      XFillRectangle (display, drwbl, gc, ilo, jlo, w+1, h+1);
    }
    if (gmu->fc>-1) {
      len = 0;
      while (*(gmu->ch+len)) len++;
/*    len++;*/
      gxdcol(gmu->fc);
      if (gfont==1 && font1i) {
        XSetFont (display, gc, font1i->fid);
        w = XTextWidth(font1i, gmu->ch, len);
        i = ilo + font1i->ascent/2;
        j = j + 5*font1i->ascent/9;
      }
      if (gfont==2 && font2i) {
        XSetFont (display, gc, font2i->fid);
        w = XTextWidth(font2i, gmu->ch, len);
        i = ilo + font2i->ascent/2;
        j = j + 5*font2i->ascent/9;
      }
      if (gfont==3 && font3i) {
        XSetFont (display, gc, font3i->fid);
        w = XTextWidth(font3i, gmu->ch, len);
        i = ilo + font3i->ascent/2;
        j = j + 5*font3i->ascent/9;
      }
      XDrawString(display, drwbl, gc, i, j, gmu->ch, len);
    }
  }
  gxdwid(gmu->thk);
  if (gmu->oc1>-1) {
    gxdcol(gmu->oc1);
    XDrawLine (display, drwbl, gc, ilo, jhi, ihi, jhi);
    if (lw>1) XDrawLine (display, drwbl, gc, ilo+1, jhi-1, ihi-1, jhi-1);
    if (lw>2) XDrawLine (display, drwbl, gc, ilo+2, jhi-2, ihi-2, jhi-2);
    XDrawLine (display, drwbl, gc, ihi, jhi, ihi, jlo);
    if (lw>1) XDrawLine (display, drwbl, gc, ihi-1, jhi-1, ihi-1, jlo+1);
    if (lw>2) XDrawLine (display, drwbl, gc, ihi-2, jhi-2, ihi-2, jlo+2);
  }
  if (gmu->oc2>-1) {
    gxdcol(gmu->oc2);
    XDrawLine (display, drwbl, gc, ihi, jlo, ilo, jlo);
    if (lw>1) XDrawLine (display, drwbl, gc, ihi-1, jlo+1, ilo+1, jlo+1);
    if (lw>2) XDrawLine (display, drwbl, gc, ihi-2, jlo+2, ilo+2, jlo+2);
    XDrawLine (display, drwbl, gc, ilo, jlo, ilo, jhi);
    if (lw>1) XDrawLine (display, drwbl, gc, ilo+1, jlo+1, ilo+1, jhi-1);
    if (lw>2) XDrawLine (display, drwbl, gc, ilo+2, jlo+2, ilo+2, jhi-2);
  }
  XFlush(display);
  for (i=0; i<4; i++) if (dmsel[i]<0) dmnum[i] = -1;
  geve->info[0] = dmnum[0];
  geve->info[1] = dmsel[0];
  geve->info[2] = dmnum[1];
  geve->info[3] = dmsel[1];
  geve->info[4] = dmnum[2];
  geve->info[5] = dmsel[2];
  geve->info[6] = dmnum[3];
  geve->info[7] = dmsel[3];
  return (ival);
}

/* pull-down menu */

int gxpopdm(struct gdmu *gmu, int iobj, int porig, int iorig, int jorig) {
int flag,i,j,cnt,len,maxw,w=0,h=0,absx,absy,enter;
int item=0,itemold,jmin,jmax,imin,imax,isiz,jsiz,j1,j2,dmflag,ii;
int pimin=0,pjmin=0,pisiz=0,pjsiz=0,ptrs[200],eflag,lln,cascf,ecnt,itt,itt2;
unsigned long lw;
char *ch,ich[5];
Pixmap tpmap=0;
Window pop, dummy;
GC gcp;

  dmrecu++;
  cnt = 0;
  ch = gmu->ch;
  len = 0;
  i = 0;
  if (dmrecu==0) {
    while (*(ch+len)) len++;
    ch = ch+len+1;
    i = len+1;
  }
  maxw = 0;
  cascf = 0;
  while (i<gmu->len) {
    len = 0;
    lln = 0;
    ptrs[cnt] = -1;
    eflag = 1;
    while (*(ch+len)) {
      if (i+len+4<gmu->len && *(ch+len)=='>' &&
           (*(ch+len+1)>='0' && *(ch+len+1)<='9') &&
           (*(ch+len+2)>='0' && *(ch+len+2)<='9') &&
            *(ch+len+3)=='>') {
        ich[0] = *(ch+len+1);
        ich[1] = *(ch+len+2);
        ich[2] = '\0';
        ptrs[cnt] =  atoi(ich);
        eflag = 0;
        lln--;
        cascf = 1;
      }
      if (i+len+5<gmu->len && *(ch+len)=='>' &&
           (*(ch+len+1)>='0' && *(ch+len+1)<='1') &&
           (*(ch+len+2)>='0' && *(ch+len+2)<='9') &&
           (*(ch+len+3)>='0' && *(ch+len+3)<='9') &&
            *(ch+len+4)=='>') {
        ich[0] = *(ch+len+1);
        ich[1] = *(ch+len+2);
        ich[2] = *(ch+len+3);
        ich[3] = '\0';
        ptrs[cnt] =  atoi(ich);
        eflag = 0;
        lln--;
        cascf = 1;
      }
      len++;
      if (eflag) lln++;
    }
    if (gfont==1 && font1i) {
      w = XTextWidth(font1i, ch, lln);
      h = font1i->ascent;
    }
    if (gfont==2 && font2i) {
      w = XTextWidth(font2i, ch, lln);
      h = font2i->ascent;
    }
    if (gfont==3 && font3i) {
      w = XTextWidth(font3i, ch, lln);
      h = font3i->ascent;
    }
    w = w + h;
    if (w>maxw) maxw = w;
    ch = ch+len+1;
    i = i+len+1;
    cnt++;
  }
  if (cascf) {
    ich[0] = 'x'; ich[1] = 'x'; ich[2] = 'x'; ich[3] = 'x';
    if (gfont==1 && font1i) w = XTextWidth(font1i, ich, 3);
    if (gfont==2 && font2i) w = XTextWidth(font2i, ich, 3);
    if (gfont==3 && font3i) w = XTextWidth(font3i, ich, 3);
    maxw = maxw + w;
  }
  h = h*2;
  jsiz = h * cnt;
  isiz = maxw;
  imin = iorig; imax = imin + isiz;
  jmin = jorig; jmax = jmin + jsiz;
  if (imax > width) {
    if (cascf) imax = iorig - isiz;
/*    else imax = width;*/
    else imax = porig;
    imin = imax - isiz;
  }
  if (jmax > height) {
    jmax = height;
    jmin = jmax - jsiz;
  }

  dmi1[dmrecu] = imin; dmi2[dmrecu] = imax;
  dmj1[dmrecu] = jmin; dmj2[dmrecu] = jmax;
  dmnum[dmrecu] = gmu->num;

  xsetw.save_under = 1;
  if (!bsflg) {
    tpmap = XCreatePixmap (display, win, isiz, jsiz, depth);
    XSync(display, 0);
    if (tpmap) {
       pimin = imin; pjmin = jmin; pisiz = isiz; pjsiz = jsiz;
       XCopyArea (display, win, tpmap, gc, pimin, pjmin, pisiz, pjsiz, 0, 0);
    }
  }
  pop = XCreateWindow(display, win, imin, jmin, isiz, jsiz, 0,
        CopyFromParent, CopyFromParent, CopyFromParent,
        CWSaveUnder, &xsetw);
  XSelectInput(display, pop, ButtonReleaseMask | ButtonPressMask |
      ButtonMotionMask | ExposureMask | StructureNotifyMask);
  gcp = XCreateGC(display, pop, 0L, &values);
  XSetForeground(display, gcp, cvals[1]);
  lw = 1;
  if (gmu->thk>5) lw = 2;
  if (gmu->thk>11) lw = 3;
  XSetLineAttributes(display, gcp, lw, LineSolid,
                     CapButt, JoinBevel);
  XMapWindow(display, pop);
  flag = 1;
  while (flag)  {
    XMaskEvent(display, ExposureMask | StructureNotifyMask, &report);
    switch  (report.type) {
    case Expose:
      if (report.xexpose.count != 0) break;
      else flag = 0;
      break;
    case ConfigureNotify:
      break;
    }
  }
  if (gmu->bbc>-1) {
    XSetForeground(display, gcp, cvals[gmu->bbc]);
    XFillRectangle (display, pop, gcp, 0, 0, isiz, jsiz);
  }
  if (gmu->boc1 > -1) {
    XSetForeground(display, gcp, cvals[gmu->boc1]);
    XDrawLine (display, pop, gcp, 0, jsiz-1, isiz-1, jsiz-1);
    XDrawLine (display, pop, gcp, isiz-1, jsiz-1, isiz-1, 0);
  }
  if (gmu->boc2 > -1) {
    XSetForeground(display, gcp, cvals[gmu->boc2]);
    XDrawLine (display, pop, gcp, isiz-1, 0, 0, 0);
    XDrawLine (display, pop, gcp, 0, 0, 0, jsiz-1);
  }
  if (gfont==1 && font1i) XSetFont (display, gcp, font1i->fid);
  if (gfont==2 && font2i) XSetFont (display, gcp, font2i->fid);
  if (gfont==3 && font3i) XSetFont (display, gcp, font3i->fid);
  cnt = 0;
  ch = gmu->ch;
  len = 0;
  i=0;
  if (dmrecu==0) {
    while (*(ch+len)) len++;
    ch = ch+len+1;
    i = len+1;
  }
  while (i<gmu->len) {
    len = 0;
    lln = 0;
    eflag = 1;
    while (*(ch+len)) {
      if (i+len+4<gmu->len && *(ch+len)=='>' &&
           (*(ch+len+1)>='0' && *(ch+len+1)<='9') &&
           (*(ch+len+2)>='0' && *(ch+len+2)<='9') &&
            *(ch+len+3)=='>') {
        eflag = 0;
      }
      if (i+len+5<gmu->len && *(ch+len)=='>' &&
           (*(ch+len+1)>='0' && *(ch+len+1)<='1') &&
           (*(ch+len+2)>='0' && *(ch+len+2)<='9') &&
           (*(ch+len+3)>='0' && *(ch+len+3)<='9') &&
            *(ch+len+4)=='>') {
        eflag = 0;
      }
      len++;
      if (eflag) lln++;
    }
    if (gfont==1 && font1i) {
      w = XTextWidth(font1i, ch, lln);
    }
    if (gfont==2 && font2i) {
      w = XTextWidth(font2i, ch, lln);
    }
    if (gfont==3 && font3i) {
      w = XTextWidth(font3i, ch, lln);
    }
    j1 = h/4;
    j2 = (cnt+1)*h - h/3;
    XSetForeground(display, gcp, cvals[gmu->bfc]);
    XDrawString(display, pop, gcp, j1, j2, ch, lln);
    if (ptrs[cnt]>-1) {
      j1 = h/5;
      j2 = (cnt+1)*h - h/2;
      if (gmu->soc2 > -1) {
        XSetForeground(display, gcp, cvals[gmu->soc2]);
        XDrawLine (display, pop, gcp, isiz-j1*3, j2+j1, isiz-j1*3, j2-j1);
        XDrawLine (display, pop, gcp, isiz-j1*3, j2-j1, isiz-j1, j2);
      }
      if (gmu->soc1 > -1) {
        XSetForeground(display, gcp, cvals[gmu->soc1]);
        XDrawLine (display, pop, gcp, isiz-j1, j2, isiz-j1*3, j2+j1);
      }
    }
    ch = ch+len+1;
    i = i+len+1;
    cnt++;
  }
  itemold = -1;
  enter = 1;

  XTranslateCoordinates (display, win, RootWindow (display, snum), 0, 0,
			      &absx, &absy, &dummy);

  /* Follow pointer around.  If it lands on another drop-menu
     header, exit to draw that one instead.  */

  dmflag = -999;
  ecnt = 0;
  while (1) {
    XMaskEvent(display, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, &report);
    if (report.type == ButtonPress || enter == 1) {
      i = report.xbutton.x_root;
      j = report.xbutton.y_root;
      i -= absx;
      j -= absy;
      item = (j-jmin)/h;
      if (i>imax || i<imin || j>jmax || j<jmin) item = -1;
      if (item > -1 && ptrs[item] > -1) {
        j1 = item*h+1;
        dmcur[dmrecu] = item;
        itt = ptrs[item];
        if (dmu[itt].num == itt) {
          dmflag = gxpopdm( &(dmu[ptrs[item]]), iobj, imin, imax, j1+jmin);
          if (dmflag != -99) {
            if (dmsel[dmrecu+1]<1) {
              item = -2;
              itemold = -2;
            }
          }
        }
      }
      dmflag = -999;
      itemold = item;
      if(enter == 1) {
        enter = 0;
      } else {
        break;
      }
    }
    if (report.type == ButtonRelease && item > -1) break;
    if (report.type == MotionNotify) {
      i = report.xmotion.x;
      j = report.xmotion.y;
      i = report.xmotion.x_root;
      j = report.xmotion.y_root;
      i -= absx;
      j -= absy;
      if (i>imax || i<imin || j>jmax || j<jmin ) {
        item = -1;
        if (dmrecu>0) {
          for (itt=0; itt<dmrecu; itt++) {
            if (i>=dmi1[itt] && i<=dmi2[itt] &&
                j>=dmj1[itt] && j<=dmj2[itt]  ) {
              itt2 = (j-dmj1[itt])/h;
              if (itt2 != dmcur[itt]) {
                dmflag = -99;
                itt = 999;
              }
            }
          }
        }
      }
      else item = (j-jmin)/h;
      if (dmflag == -99) {itemold = -1; break; }
      if (item!=itemold) {
        if (item>-1) {
          j1 = item*h+lw; j2 = (item+1)*h-lw-1;
          if (gmu->soc2 > -1) {
            XSetForeground(display, gcp, cvals[gmu->soc2]);
            XDrawLine (display, pop, gcp, lw, j1, lw, j2);
            XDrawLine (display, pop, gcp, lw, j1, isiz-lw-1, j1);
          }
          if (gmu->soc1 > -1) {
            XSetForeground(display, gcp, cvals[gmu->soc1]);
            XDrawLine (display, pop, gcp, isiz-lw-1, j1, isiz-lw-1, j2);
            XDrawLine (display, pop, gcp, lw, j2, isiz-lw-1, j2);
          }
          if (ptrs[item]>-1) {
            j1 = h/5;
            j2 = (item+1)*h - h/2;
            if (gmu->soc1 > -1) {
              XSetForeground(display, gcp, cvals[gmu->soc1]);
              XDrawLine (display, pop, gcp, isiz-j1*3, j2+j1, isiz-j1*3, j2-j1);
              XDrawLine (display, pop, gcp, isiz-j1*3, j2-j1, isiz-j1, j2);
            }
            if (gmu->soc2 > -1) {
              XSetForeground(display, gcp, cvals[gmu->soc2]);
              XDrawLine (display, pop, gcp, isiz-j1, j2, isiz-j1*3, j2+j1);
            }
          }
          ecnt = 0;
        }
        if (itemold>-1) {
          j1 = itemold*h+lw; j2 = (itemold+1)*h-lw-1;
          if (gmu->bbc > -1) {
            XSetForeground(display, gcp, cvals[gmu->bbc]);
            XDrawLine (display, pop, gcp, lw, j1, lw, j2);
            XDrawLine (display, pop, gcp, lw, j1, isiz-lw-1, j1);
            XDrawLine (display, pop, gcp, isiz-lw-1, j1, isiz-lw-1, j2);
            XDrawLine (display, pop, gcp, lw, j2, isiz-lw-1, j2);
          }
          if (ptrs[itemold]>-1) {
            j1 = h/5;
            j2 = (itemold+1)*h - h/2;
            if (gmu->soc2 > -1) {
              XSetForeground(display, gcp, cvals[gmu->soc2]);
              XDrawLine (display, pop, gcp, isiz-j1*3, j2+j1, isiz-j1*3, j2-j1);
              XDrawLine (display, pop, gcp, isiz-j1*3, j2-j1, isiz-j1, j2);
            }
            if (gmu->soc1 > -1) {
              XSetForeground(display, gcp, cvals[gmu->soc1]);
              XDrawLine (display, pop, gcp, isiz-j1, j2, isiz-j1*3, j2+j1);
            }
          }
          ecnt = 0;
        }
      }
      if (ecnt > 3 && item > -1 && item==itemold && ptrs[item] > -1) {
        j1 = itemold*h+1;
        dmcur[dmrecu] = item;
        itt = ptrs[item];
        if (dmu[itt].num == itt) {
          dmflag = gxpopdm( &(dmu[ptrs[item]]), iobj, imin, imax, j1+jmin);
          if (dmflag != -99) {
            if (dmsel[dmrecu+1]<1) {
              item = -2;
              itemold = -2;
            }
            break;
          }
          dmflag = -999;
        }
      }
      if (item==itemold) ecnt++;
      itemold = item;
      if (item==-1) {
        ii = 0;
        while (ii<512 && obj[ii].type>-1) {
          if (ii != iobj && obj[ii].type==3 &&
              i>obj[ii].i1 && i<obj[ii].i2 && j>obj[ii].j1 && j<obj[ii].j2) {
            dmflag = ii;
            ii = 100000;           /* Exit loop */
          }
          ii++;
        }
      }
      if (dmflag > -1) break;
    }
  }
  dmsel[dmrecu] = itemold + 1;
  if (ptrs[itemold]>-1 && dmsel[dmrecu+1]<0) dmsel[dmrecu] = -1;
  XUnmapWindow(display,pop);
  XDestroyWindow(display,pop);
  if (!bsflg) {
    if (tpmap) {
      XCopyArea (display, tpmap, win, gc, 0, 0, pisiz, pjsiz, pimin, pjmin);
      XFreePixmap (display, tpmap);
    }
    excnt++;
  }
  XFlush(display);
  dmrecu--;
  return (dmflag);
}

/* Handle dialog box.  A new, temporary window is created,
   and text input is allowed.  Window is destroyed as soon as
   the enter key is detected.  */

/* modified 102400 by love to support user settings for window with editing. */

char *gxdlg (struct gdlg *qry) {
int flag,i,j,cnt,len,w,h=0,rlen,plen,tlen;
int dflag,cn,c0,c1,c2,i1,i2=0,ii,w1=0,w2=0,cflag,eflag,pflag;
int jmin,imin,isiz,jsiz,j1,j2;
int n0,n1,n2,n3,n4,n5,n6,n7,p0,p1,p2,p3,p4,p5,p6,pl=0;
Time lastBtnDown;
char *tch,buff[80],*rch,ch[1],*pch;
KeySym keysym;
int pimin,pjmin,pisiz,pjsiz;
Pixmap tpmap=0, wmap=0;
Window pop;
GC gcp;

  pch = (char *)malloc(512);
  tch = (char *)malloc(512);
  rch = (char *)malloc(512);
  if (pch==NULL) {
    printf ("Memory Allocation Error: Dialog Box\n");
    *(rch+0)='\0';
    return (rch);
  }
  if (tch==NULL) {
    printf ("Memory Allocation Error: Dialog Box\n");
    *(rch+0)='\0';
    return (rch);
  }
  if (rch==NULL) {
    printf ("Memory Allocation Error: Dialog Box\n");
    *(rch+0)='\0';
    return (rch);
  }
  plen = 0;
  rlen = 0;

  XSelectInput(display, win, ButtonReleaseMask | ButtonPressMask |
      ButtonMotionMask | ExposureMask | StructureNotifyMask);

/* Check for a '|' and split the input string into a prompt and
   an initial string.  Check for a '\' or '/' to split the line */
  len = 0;
  if (qry->ch) while (*(qry->ch+len)) len++;
  flag = 0;
  pflag = 0;
  c0 = 0;
  c1 = 0;
  c2 = 0;
  for (i = 0; i<len; i++) {
    *(tch-c1+i) = *(qry->ch+i);
    if ((*(qry->ch+i) == '|' || *(qry->ch+i) == '/') && pflag == 0) {
      c1 = i + 1; 
      strcpy(pch,tch);
      pflag = i;
    }
    if (*(qry->ch+i) == '/' && flag == 0) flag = i;
  }
  plen = c1;
  if (*(pch) == '|' || *(pch) == '/') {*(pch) = '\0'; plen = 0;}
  if (*(pch+pflag) == '|' || *(pch+pflag) == '/') *(pch+pflag) = ' ';
  *(pch+c1) = '\0';
  
/* Define input string */
  for (i = 0; i<len-c1; i++) { /* find leading blanks and new line indicators */
    if (*(tch+i) == '/' || *(tch+i) == ' ' || *(tch+i) == '|') continue;
    else {c2 = i; break;}
  }
  for (i = 0; i<len-c2; i++) 	/* remove leading blanks and new line indicators */
    *(rch+i) =  *(tch+c2+i); 
  strcpy(tch,rch);
  tlen = len - c2 - c1;
  *(tch+tlen) = '\0';
    
  if (qry->x<0 &&qry->y<0 &&qry->h<0 &&qry->w<0)
    dflag = 1;
  else if (flag)
    dflag = 2; 
  else 
    dflag = 0;
 
  if (pflag && !flag) c0 = c1;
  if (dflag == 1 ) c0 = 0;
  
/* Set width and height of dialog box */
  if (gfont==1 && font1i) {
    if (pch) w1 = XTextWidth(font1i, pch, plen);
    else w1 = 0;
    h = font1i->ascent;
  }
  if (gfont==2 && font2i) {
    if (pch) w1 = XTextWidth(font2i, pch, plen);
    else w1 = 0;
    h = font2i->ascent;
  }
  if (gfont==3 && font3i) {
    if (pch) w1 = XTextWidth(font3i, pch, plen);
    else w1 = 0;
    h = font3i->ascent;
  }
  if (gfont==1 && font1) {
    if (tch) w2 = XTextWidth(font1, tch, tlen);
    else w2 = 0;
    h = font1->ascent;
  }
  if (gfont==2 && font2) {
    if (tch) w2 = XTextWidth(font2, tch, tlen);
    else w2 = 0;
    h = font2->ascent;
   }
   if (gfont==3 && font3) {
    if (tch) w1 = XTextWidth(font3, tch, tlen);
    else w2 = 0;
    h = font3->ascent;
  }
  if (flag) w = ((w1)>(w2))?(w1):(w2);
  else w = w1 + w2;
  if (flag) h = h*1.8;


/* Convert box size to pixels */
  if (qry->h == -1 ) {
    jsiz = h*5;
    if (jsiz>height) jsiz = height*3/4;
  }
  else jsiz = (int)(qry->h*yscl+0.5);
  if (qry->h == 0 ) jsiz = h*1.8;
  
  if (qry->w == -1 ) isiz = width*2/3;
  else isiz = (int)(qry->w*xscl+0.5);
  if (qry->w == 0 ) isiz = w+h*1.5;
  
  if (qry->x == -1 ) imin = (width-isiz)/2;
  else imin = qry->x*xscl+0.5-isiz/2;
  
  if (qry->y == -1 ) jmin = (height-jsiz)/2;
  else jmin = height - (qry->y*yscl+0.5+jsiz/2);
  pimin = imin; pjmin = jmin; pisiz = isiz; pjsiz = jsiz;
  
  xsetw.save_under = 1;
  if (!bsflg) {
    wmap = XCreatePixmap (display, win, width, height, depth);
    XSync(display, 0);
    if (wmap) {
       XCopyArea (display, win, wmap, gc, 0, 0, width, height, 0, 0);
    }
  }
  pop = XCreateWindow(display, win, pimin, pjmin, pisiz, pjsiz, 0,
        CopyFromParent, CopyFromParent, CopyFromParent,
        CWSaveUnder, &xsetw);
  XSelectInput(display, pop, ExposureMask | StructureNotifyMask | KeyPressMask);
  gcp = XCreateGC(display, pop, 0L, &values);
  if (qry->oc > -1) cn = qry->oc;
  else cn = 1;
  XSetForeground(display, gcp, cvals[cn]);
  XSetLineAttributes(display, gcp, 0L, LineSolid,
                     CapButt, JoinBevel);
  XMapWindow(display, pop);
  flag = 1;
  while (flag)  {
    XMaskEvent(display, ExposureMask | StructureNotifyMask, &report);
    switch  (report.type) {
    case Expose:
      if (report.xexpose.count != 0) break;
      else flag = 0;
      break;
    case ConfigureNotify:
      break;
    }
  }

/* Draw dialog box and text */
  if (dflag == 1 && plen == 0) {
    plen = tlen;
    strcpy(pch,tch);
    tlen = 0;
    *(tch) = '\0';
  }
  if (qry->bc > -1) cn = qry->bc;
  else cn = 15;
  XSetForeground(display, gcp, cvals[cn]);
  XFillRectangle (display, pop, gcp, 0, 0, pisiz, pjsiz);
  if (qry->oc > -1) cn = qry->oc;
  else cn = 1;
  XSetForeground(display, gcp, cvals[cn]);
  XDrawRectangle (display, pop, gcp, 0, 0, pisiz-1, pjsiz-1);
  if (qry->th > 5) XDrawRectangle (display, pop, gcp, 1, 1, pisiz-3, pjsiz-3);
  if (qry->fc > -1) cn = qry->fc;
  else cn = 0;
  XSetForeground(display, gcp, cvals[cn]);
  if (dflag) {
    if (gfont==1 && font1i) XSetFont (display, gcp, font1i->fid);
    if (gfont==2 && font2i) XSetFont (display, gcp, font2i->fid);
    if (gfont==3 && font3i) XSetFont (display, gcp, font3i->fid);
    i = (isiz-w)/2;
    if (dflag == 1) {
      j = h*2;
    } else {
      j = jsiz*3/7;
    }
    XSetForeground(display, gcp, cvals[0]);
    if (plen>0) XDrawString(display, pop, gcp, i, j, pch, plen);
    if (qry->pc > -1) cn = qry->pc;
    else cn = 1;
    XSetForeground(display, gcp, cvals[cn]);
    if (plen>0) XDrawString(display, pop, gcp, i-1, j-1, pch, plen);
    if (gfont==1 && font1) XSetFont (display, gcp, font1->fid);
    if (gfont==2 && font2) XSetFont (display, gcp, font2->fid);
    if (gfont==3 && font3) XSetFont (display, gcp, font3->fid);
    if (dflag == 1) {
      j = h*4;
      j1 = h*2+1;
      j2 = jsiz-j1-1;
    } else {
      j = jsiz*6/7;
      j1 = jsiz*1/2;
      j2 = jsiz -j1 -1;
    }
    if (qry->fc > -1) cn = qry->fc;
    else cn = 0;
    XSetForeground(display, gcp, cvals[cn]);
    if (gfont==1 && font1) XSetFont (display, gcp, font1->fid);
    if (gfont==2 && font2) XSetFont (display, gcp, font2->fid);
    if (gfont==3 && font3) XSetFont (display, gcp, font3->fid);
    if (tlen>0) XDrawString(display, pop, gcp, i, j, tch, tlen);
    strcpy(rch,tch);
    rlen = tlen;
  } else {
    if (gfont==1 && font1) XSetFont (display, gcp, font1->fid);
    if (gfont==2 && font2) XSetFont (display, gcp, font2->fid);
    if (gfont==3 && font3) XSetFont (display, gcp, font3->fid);
    j1 = 1;
    j2 = jsiz -j1 -1;
    j = (j1+j2+h-1)/2;
    if (pflag) {
      i = h/2;
      strcpy(rch,pch);
      strcat(rch,tch);
      rlen = plen + tlen;
      if (rlen>0) XDrawString(display, pop, gcp, i, j, rch, rlen);
    } else {
      i = (isiz-w)/2;
      if (tlen>0) XDrawString(display, pop, gcp, i, j, tch, tlen);
      strcpy(rch,tch);
      rlen = tlen;
    }    
  }

  if (!bsflg) {
    tpmap = XCreatePixmap (display, pop, pisiz, pjsiz, depth);
    XSync(display, 0);
    if (tpmap) {
       XCopyArea (display, pop, tpmap, gc, 0, 0, pisiz, pjsiz, 0, 0);
    }
  }

/* Loop on edit session and exit on Enter */
  c1 = rlen;
  c2 = rlen;
  cflag = 0;
  eflag = 0;
  for (i=0; i<512; i++) *(tch+i) = '\0';
  lastBtnDown = 0;
  while (1) {
    XMaskEvent(display, ButtonReleaseMask | ButtonPressMask |
         ButtonMotionMask | KeyPressMask | ExposureMask | StructureNotifyMask, &report);
   
/*mf 980112
  explicit cast of report to make it work on the NERSC j90
  this is NOT correct for X11R6.3
mf*/

    if (report.type == NoExpose) continue;

    if (report.type == Expose || report.type == GraphicsExpose) {
      if (excnt>0) excnt--;
      if (!bsflg) {
        if (wmap && report.type == Expose) {
          XCopyArea (display, wmap, win, gc, 0, 0, width, height, 0, 0);
        }
        if (tpmap) {
          XCopyArea (display, tpmap, pop, gc, 0, 0, pisiz, pjsiz, 0, 0);
        }
      }
      XFlush(display);
    }
    if (!bsflg && report.type != Expose && report.type != GraphicsExpose && tpmap) {
      XFreePixmap (display, tpmap); 
      tpmap = (Pixmap) NULL;
    }
    if (report.type == ButtonPress && report.xbutton.time < lastBtnDown + 300) {
      c1 = c0;
      c2 = rlen;
      i1 = (isiz-w)/2;
      i2 = (isiz+w)/2;
      cflag = 1;
    } else if (report.type == ButtonPress && rlen>0) {
      i1 = report.xbutton.x - imin;
      if (2*i1 > isiz-w && 2*i1 < isiz+w) c1 = (i1 - (isiz-w)/2 + 4)*rlen/w;
      if (2*i1 <= isiz-w) c1 = 0;
      if (2*i1 >= isiz+w) c1 = rlen; 
      if (2*i1 <= isiz-w) i1 = (isiz-w)/2; 
      if (2*i1 >= isiz+w) i1 = (isiz+w)/2; 
      if (c1 < c0) c1 = c0;
      c2 = c1;
      lastBtnDown = report.xbutton.time;
    } else if ((report.type == ButtonRelease || report.type == MotionNotify) && rlen>0 && cflag==0) {
      i2 = report.xbutton.x - imin;
      if (2*i2 > isiz-w && 2*i2 < isiz+w) c2 = (i2 - (isiz-w)/2 + 8)*rlen/w;
      if (2*i2 <= isiz-w) c2 = c0;
      if (2*i2 >= isiz+w) c2 = rlen; 
      if (2*i2 <= isiz-w) i2 = (isiz-w)/2; 
      if (2*i2 >= isiz+w) i2 = (isiz+w)/2; 
      if (c2 < c0) c2 = c0;
      cflag = 0;
    }
    if(rlen == 0) {
      i2 = isiz/2;
      i1 = isiz/2;
      c2 = c0;
      c1 = c0;
    }
    if (report.type ==  KeyPress) {
      cnt = XLookupString((XKeyEvent *)&report,buff,80,&keysym,NULL);
      if (cnt>0) {
        if (keysym==XK_Return || keysym==XK_Linefeed) {
          if (qry->nu == 1) {
            for (i=0; i<rlen-c0; i++) *(tch+i) = *(rch+c0+i);
            tlen = rlen-c0;
            ii = 0;
            n0=0; n1=0; n2=0; n3=0; n4=0; n5=0; n6=0; n7=0;
            p0=0; p1=0; p2=0; p3=0; p4=0; p5=0; p6=0;
            for (i = 0; i<tlen; i++) {
              if (*(tch+i) == '.') {n1++; p1=i+1;}
              if (*(tch+i) == '+') {n2++; p2=i+1;}
              if (*(tch+i) == '-') {n3++; p3=i+1;}
              if (*(tch+i) == 'e') {n4++; p4=i+1;}
              if (*(tch+i) == 'E') {n5++; p5=i+1;}
              if (*(tch+i) >= 'a' && *(tch+i) <= 'z' && *(tch+i) != 'e') n0=1;
              if (*(tch+i) >= 'A' && *(tch+i) <= 'Z' && *(tch+i) != 'E') n0=1;
              if (n0==1 && p0==0) p0 = i;
              if (n0==1) pl =i;
              if (*(tch+i) == '#') {n6++; p6=i+1;}
              if (*(tch+i) >= '0' && *(tch+i) <= '9') n7=1;
            }     
            if (n1==1 && p1==1) ii = 1; /* require digit before decimal  */    
            if (n1==1 && p1==2 && p2==1 && n2==1) ii = 1;
            if (n1==1 && p1==2 && p3==1 && n3==1) ii = 1;  
            if (n1==1 && p1==2 && p2==p4+1 && n2==2) ii = 1;
            if (n1==1 && p1==2 && p3==p4+1 && n3==2) ii = 1;  
            if (n1==1 && p1==2 && p2==p5+1 && n2==2) ii = 1;
            if (n1==1 && p1==2 && p3==p5+1 && n3==2) ii = 1;  
            if (n1>1) ii = 2; /* too many decimal points */
            if (n2>=1 && p2>p4+1 && n4==1) ii = 3; /* misplaced + sign in exponent */
            if (n2>=1 && p2>p5+1 && n5==1) ii = 3;
            if (n3>=1 && p3>p4+1 && n4==1) ii = 4; /* misplaced - sign in exponent */
            if (n3>=1 && p3>p5+1 && n5==1) ii = 4;           
            if (n2>=1 && p2>1 && n4+n5==0) ii = 5; /* misplaced + sign in number */
            if (n2>=1 && p2>1 && p2<p4 && n4==1) ii = 5;
            if (n2>=1 && p2>1 && p2<p5 && n5==1) ii = 5;            
            if (n3>=1 && p3>1 && n4+n5==0) ii = 6; /* misplaced - sign in number */
            if (n3>=1 && p3>1 && p3<p4 && n4==1) ii = 6;
            if (n3>=1 && p3>1 && p3<p5 && n5==1) ii = 6;
            if (n1==1 && p1>p4 && n4==1) ii = 7; /* decimal in exponent */
            if (n1==1 && p1>p5 && n5==1) ii = 7;            
            if (n4>0 && p4==tlen) ii = 8; /* missing exponent value */
            if (n5>0 && p5==tlen) ii = 8;
            if (n4>0 && p4==tlen-1 && p2==tlen) ii = 8;
            if (n5>0 && p5==tlen-1 && p2==tlen) ii = 8;
            if (n4>0 && p4==tlen-1 && p3==tlen) ii = 8;
            if (n5>0 && p5==tlen-1 && p3==tlen) ii = 8;
            if (n7==0) ii = 9; /* missing number value */
            if (n4>0 && p4==1) ii = 9;
            if (n5>0 && p5==1) ii = 9;
            if (n4>0 && p4==2 && p1==1) ii = 9;
            if (n5>0 && p5==2 && p1==1) ii = 9;
            if (n4>0 && p4==2 && p2==1) ii = 9;
            if (n5>0 && p5==2 && p2==1) ii = 9;
            if (n4>0 && p4==2 && p3==1) ii = 9;
            if (n5>0 && p5==2 && p3==1) ii = 9;
            if (n2>2) ii = 10; /* too many '+' signs */
            if (n3>2) ii = 11; /* too many '-' signs */
            if (n4+n5>1) ii = 12; /* too many exponent symbols */
            if (n6>0) ii = 13; /* # sign still present, number not entered */
            if (n0>0) ii = 14; /* number includes an alpha char */
            if (ii>1) { /* calculate error indices to highlight */
              eflag = 1;
              if (ii==2) {c1 = p1-1; c2 = p1;}
              if (ii==3) {c1 = p2-1; c2 = p2;}
              if (ii==4) {c1 = p3-1; c2 = p3;}
              if (ii==5) {c1 = p2-1; c2 = p2;}
              if (ii==6) {c1 = p3-1; c2 = p3;}
              if (ii==7) {c1 = p1-1; c2 = p1;}
              if (ii==8) {c1 = tlen; c2 = tlen+1; 
                 *(tch+tlen)='#'; *(tch+tlen+1)='\0'; tlen++;}
              if (ii==9) {c1 = 0; c2 = 1; 
                  strcpy(rch,tch); *(tch)='#'; 
                 for (i=1; i<tlen+1; i++) *(tch+i) = *(rch+i-1); 
                 *(tch+tlen+1) = '\0';}
              if (ii==10) {c1 = p2-1; c2 = p2;}
              if (ii==11) {c1 = p3-1; c2 = p3;}
              if (ii==12) {
                if (p5>p4) {c1 = p5-1; c2 = p5;}
                else {c1 = p4-1; c2 = p4;}
              }
              if (ii==13) {c1 = p6-1; c2 = p6;}
              if (ii==14) {c1 = p0-1; c2 = pl+1;}
              rlen = tlen+c0;
              if (pflag && !dflag) {
                c1 += plen; c2 += plen;
                strcpy(rch,tch);
                strcpy(tch,pch);
                strcat(tch,rch);
                rlen = strlen(tch);
              } 
              strcpy(rch,tch);
            
            } else {
              eflag = 0;
               if (ii==1) { /* insert zero digit before decimal  */ 
                strcpy(rch,tch);
                tlen++;
                ii = 0;
                for (i=0; i<tlen; i++) {
                  if (i==p1-1) {
                    *(tch+i) = '0';
                    i++;
                  }
                  *(tch+i) = *(rch+ii);
                  ii++;
                }
                *(tch+tlen) = '\0';
              }
              rlen = tlen+c0;
              if (pflag && !dflag) {
                strcpy(rch,tch);
                strcpy(tch,pch);
                strcat(tch,rch);
              }
              strcpy(rch,tch);
            
              break;
            }           
          } else break;
        }
        if (keysym==XK_BackSpace && c1>c0) {
          strcpy(tch,rch);
          ii = 0;
          for (i=c0; i<rlen; i++) {
            if (i == c1-1 && c1 == c2) continue;
            if (i >= c1 && i < c2) continue;
            *(rch+ii) = *(tch+i);
            ii++;
          }
          rlen = ii;
          if (pflag && !dflag) {strcpy(tch,pch); strcat(tch,rch); strcpy(rch,tch); rlen+=plen;}
          *(rch+rlen) = '\0';
          c1--;
          c2 = c1;
        }
        if (keysym==XK_Delete) {
          strcpy(tch,rch);
          ii = 0;
          for (i=c0; i<rlen; i++) {
            if (i == c1 && c1 == c2) continue;
            if (i >= c1 && i < c2) continue;
            *(rch+ii) = *(tch+i);
            ii++;
          }
          rlen = ii;
          if (pflag && !dflag) {strcpy(tch,pch); strcat(tch,rch); strcpy(rch,tch); rlen+=plen;}
          if (c1==rlen &&c1>0) c1--;
          *(rch+rlen) = '\0';
          c2 = c1;
        }
        if (keysym>=XK_space && keysym<=XK_asciitilde) {
          if (qry->nu == 1) {
            *(ch) = buff[0];
            if ((*ch>='0' && *ch<='9') || *ch=='+' || *ch=='-' || *ch=='.' || *ch=='e' || *ch=='E') ;
            else continue;
          }
          strcpy(tch,rch);
          rlen += c1-c2+1;
          ii = 0;
          for (i=0; i<rlen; i++) {
            if (i == c1) {
              *(rch+c1) = buff[0];
              i++;
              ii += c2-c1;
            }
            *(rch+i) = *(tch+ii);
            ii++;
          }
          c1++;
          *(rch+rlen) = '\0';
          c2 = c1;
        }
        for (i=0; i<512; i++) *(tch+i) = '\0';
      }
      if ((keysym==XK_KP_Left  ||keysym==XK_Left) && c1>c0) {if (c1==c2) c2--; c1--;}
      if ((keysym==XK_KP_Right ||keysym==XK_Right) && c1<rlen) {if (c1==c2) c2++; c1++;}
      if ((keysym==XK_KP_Down  ||keysym==XK_Down) && c2>c1) c2--; 
      if ((keysym==XK_KP_Up    ||keysym==XK_Up) && c2<rlen) c2++;
    }
    if (rlen>=0 && c1>=c2) {
      if (qry->bc > -1) cn = qry->bc;
      else cn = 15;
      XSetForeground(display, gcp, cvals[cn]);
      XFillRectangle (display, pop, gcp, 2, j1+1, isiz-4, j2-2);
      if (qry->fc > -1) cn = qry->fc;
      else cn = 0;
      XSetForeground(display, gcp, cvals[cn]);
      if (gfont==1 && font1) w = XTextWidth(font1, rch, rlen);
      if (gfont==2 && font2) w = XTextWidth(font2, rch, rlen);
      if (gfont==3 && font3) w = XTextWidth(font3, rch, rlen);
      if (w+h < isiz) i = (isiz-w)/2;
      else i = isiz-w-h/2;
      if (pflag && !dflag && w+h < isiz) i = h/2;
      if (c1>0) strncpy(tch,rch, (size_t) c1);
      *(tch+c1) = '\0';
      if (gfont==1 && font1) w1 = XTextWidth(font1, tch, c1);
      if (gfont==2 && font2) w1 = XTextWidth(font2, tch, c1);
      if (gfont==3 && font3) w1 = XTextWidth(font3, tch, c1);
      ii = i+w1;
      if (ii<=0) {
        i=-w1;
        XDrawString(display, pop, gcp, i, j, rch, rlen);
        XDrawString(display, pop, gcp, i, j, "|", 1);
      } else {
        XDrawString(display, pop, gcp, i, j, rch, rlen);
        XDrawString(display, pop, gcp, ii, j, "|", 1);
      }
    }
    if(c1<c2) {
      if (gfont==1 && font1) w = XTextWidth(font1, rch, rlen);
      if (gfont==2 && font2) w = XTextWidth(font2, rch, rlen);
      if (gfont==3 && font3) w = XTextWidth(font3, rch, rlen);
      if (w+h < isiz) i = (isiz-w)/2;
      else i = isiz-w-h/2;
      if (pflag && !dflag && w+h < isiz) i = h/2;
      
      if (c1>0) strncpy(tch,rch, (size_t) c1);
      *(tch+c1) = '\0';
      if (gfont==1 && font1) w1 = XTextWidth(font1, tch, c1);
      if (gfont==2 && font2) w1 = XTextWidth(font2, tch, c1);
      if (gfont==3 && font3) w1 = XTextWidth(font3, tch, c1);
      if (qry->bc > -1) cn = qry->bc;
      else cn = 15;
      XSetForeground(display, gcp, cvals[cn]);
      XFillRectangle (display, pop, gcp, 2, j1+1, isiz-4, j2-2);
      if (qry->fc > -1) cn = qry->fc;
      else cn = 0;
      XSetForeground(display, gcp, cvals[cn]);
      XDrawString(display, pop, gcp, i, j, tch, c1);
      
      for (ii=0; ii<c2-c1; ii++) *(tch+ii) = *(rch+ii+c1);
      *(tch+c2-c1) = '\0';
      if (gfont==1 && font1) w2 = XTextWidth(font1, tch, c2-c1);
      if (gfont==2 && font2) w2 = XTextWidth(font2, tch, c2-c1);
      if (gfont==3 && font3) w2 = XTextWidth(font3, tch, c2-c1);
      i += w1;     
      if (qry->fc > -1) cn = qry->fc;
      else cn = 0;
      if (eflag) cn = 2;
      XSetForeground(display, gcp, cvals[cn]);
      XFillRectangle (display, pop, gcp, i, j1+1, w2, j2-2);
      if (qry->bc > -1) cn = qry->bc;
      else cn = 15;
      XSetForeground(display, gcp, cvals[cn]);
      XDrawString(display, pop, gcp, i, j, tch, c2-c1);
      
      for (ii=0; ii<rlen-c2; ii++) *(tch+ii) = *(rch+ii+c2);
      *(tch+rlen-c2) = '\0';
      i += w2;
      if (qry->bc > -1) cn = qry->bc;
      else cn = 15;
      XSetForeground(display, gcp, cvals[cn]);
      XFillRectangle (display, pop, gcp, i, j1+1, isiz-i2, j2-2);
      if (qry->fc > -1) cn = qry->fc;
      else cn = 0;
      XSetForeground(display, gcp, cvals[cn]);
      XDrawString(display, pop, gcp, i, j, tch, rlen-c2);
      
      if (qry->oc > -1) cn = qry->oc;
      else cn = 1;
      XSetForeground(display, gcp, cvals[cn]);
      XDrawRectangle (display, pop, gcp, 0, 0, isiz-1, jsiz-1);
      if (qry->th > 5) XDrawRectangle (display, pop, gcp, 1, 1, isiz-3, jsiz-3);
    }
    for (i=0; i<512; i++) *(tch+i) = '\0';

    if (!bsflg && report.type != Expose && report.type != GraphicsExpose && !tpmap) {
      tpmap = XCreatePixmap (display, pop, pisiz, pjsiz, depth);
      XSync(display, 0);
      if (tpmap) {
         XCopyArea (display, pop, tpmap, gc, 0, 0, pisiz, pjsiz, 0, 0);
      }
    }
  }

/* Restore original screen */
  XUnmapWindow(display,pop);
  XDestroyWindow(display,pop);
  gxdsfn();
  XSelectInput(display, win, ButtonReleaseMask | ButtonPressMask |
      ButtonMotionMask | ExposureMask | StructureNotifyMask);
  if (!bsflg) {
    if (wmap) {
      XCopyArea (display, wmap, win, gc, 0, 0, width, height, 0, 0);
      XFreePixmap (display, wmap);
    }
    if (tpmap) {
      XFreePixmap (display, tpmap);
    }
    excnt++;
  }
  XFlush(display);
  
/* Remove '+' signs */
  if (qry->nu == 1) {
    strcpy(tch,rch);
    ii = 0;
    for (i=0; i<rlen; i++) {
      if (*(rch+i)  == '+') continue;
      *(rch+ii) = *(tch+i);
      ii++;
    }
    rlen = ii;
  } 
/* Trim prompt string */
  if (pflag && !dflag) {
    strcpy(tch,rch);
    ii = 0;
    for (i=c0; i<rlen; i++) {
      *(rch+ii) = *(tch+i);
      ii++;
    }
    rlen = ii;
  }
  *(rch+rlen) = '\n';
  *(rch+rlen+1) = '\0';
  return (rch);
}

/* Screen save and show operation */
/* Save from displayed window.  Do show operation to current
   window display, sometimes background */

void gxdssv (int frame) {
  if (batch) {
    printf("Error: the screen command does not work in batch mode\n");
    return;
  }
  if (frame<0 || frame>199) return;
  if (!pfilld[frame]) {
    pmaps[frame] = XCreatePixmap (display, win, width, height, depth);
    if (pmaps[frame]==(Pixmap)NULL) {
      printf ("Error allocating pixmap for screen save operation\n");
      printf ("Screen will not be saved\n");
      return;
    }
    pfilld[frame] = 1;
  }
  XCopyArea (display, win, pmaps[frame], gc, 0, 0, width, height, 0, 0);
}

void gxdssh (int cnt) {
  if (batch) {
    printf("Error: the screen command does not work in batch mode\n");
    return;
  }
  if (cnt<0 || cnt>199) return;
  if (pfilld[cnt]) XCopyArea (display, pmaps[cnt], drwbl, gc, 0, 0, width, height, 0, 0);
}

void gxdsfr (int frame) {
  if (batch) {
    printf("Error: the screen command does not work in batch mode\n");
    return;
  }
  if (frame<0 || frame>199) return;
  if (pfilld[frame]) {
    XFreePixmap (display, pmaps[frame]);
    pfilld[frame] = 0;
  }
}

/* Routine to install stipple pixmaps to provide pattern fills for
   recf and polyf.  Choices include solid, dot, line and open.
   Check and line density can be controlled as well as line angle. */

void gxdptn (gaint typ, gaint den, gaint ang) {
unsigned char *bitmap_bits;
int bitmap_width, bitmap_height;
Pixmap stipple_pixmap;

  if (typ==0) {
    bitmap_bits = open_bitmap_bits;
    bitmap_width = open_bitmap_width;
    bitmap_height = open_bitmap_height;
  }
  else if (typ==1) {
    XSetFillStyle (display, gc, FillSolid);
    return;
  }
  else if (typ==2) {
    if (den==6) {
      bitmap_bits = dot_6_bitmap_bits;
      bitmap_width = dot_6_bitmap_width;
      bitmap_height = dot_6_bitmap_height;
    }
    else if (den==5) {
      bitmap_bits = dot_5_bitmap_bits;
      bitmap_width = dot_5_bitmap_width;
      bitmap_height = dot_5_bitmap_height;
    }
    else if (den==4) {
      bitmap_bits = dot_4_bitmap_bits;
      bitmap_width = dot_4_bitmap_width;
      bitmap_height = dot_4_bitmap_height;
    }
    else if (den==3) {
      bitmap_bits = dot_3_bitmap_bits;
      bitmap_width = dot_3_bitmap_width;
      bitmap_height = dot_3_bitmap_height;
    }
    else if (den==2) {
      bitmap_bits = dot_2_bitmap_bits;
      bitmap_width = dot_2_bitmap_width;
      bitmap_height = dot_2_bitmap_height;
    }
    else if (den==1) {
      bitmap_bits = dot_1_bitmap_bits;
      bitmap_width = dot_1_bitmap_width;
      bitmap_height = dot_1_bitmap_height;
    }
    else {
      printf ("Error in density specification: density = %d\n",den);
      return;
    }
  }
  else if (typ==3) {
    if (ang==0) {
      if (den==5) {
        bitmap_bits = line_0_5_bitmap_bits;
        bitmap_width = line_0_5_bitmap_width;
        bitmap_height = line_0_5_bitmap_height;
      }
      else if (den==4) {
        bitmap_bits = line_0_4_bitmap_bits;
        bitmap_width = line_0_4_bitmap_width;
        bitmap_height = line_0_4_bitmap_height;
      }
      else if (den==3) {
        bitmap_bits = line_0_3_bitmap_bits;
        bitmap_width = line_0_3_bitmap_width;
        bitmap_height = line_0_3_bitmap_height;
      }
      else if (den==2) {
        bitmap_bits = line_0_2_bitmap_bits;
        bitmap_width = line_0_2_bitmap_width;
        bitmap_height = line_0_2_bitmap_height;
      }
      else if (den==1) {
        bitmap_bits = line_0_1_bitmap_bits;
        bitmap_width = line_0_1_bitmap_width;
        bitmap_height = line_0_1_bitmap_height;
      }
      else {
        printf ("Error in density specification: density = %d\n",den);
        return;
      }
    }
    else if (ang==30) {
      if (den==5) {
        bitmap_bits = line_30_5_bitmap_bits;
        bitmap_width = line_30_5_bitmap_width;
        bitmap_height = line_30_5_bitmap_height;
      }
      else if (den==4) {
        bitmap_bits = line_30_4_bitmap_bits;
        bitmap_width = line_30_4_bitmap_width;
        bitmap_height = line_30_4_bitmap_height;
      }
      else if (den==3) {
        bitmap_bits = line_30_3_bitmap_bits;
        bitmap_width = line_30_3_bitmap_width;
        bitmap_height = line_30_3_bitmap_height;
      }
      else if (den==2) {
        bitmap_bits = line_30_2_bitmap_bits;
        bitmap_width = line_30_2_bitmap_width;
        bitmap_height = line_30_2_bitmap_height;
      }
      else if (den==1) {
        bitmap_bits = line_30_1_bitmap_bits;
        bitmap_width = line_30_1_bitmap_width;
        bitmap_height = line_30_1_bitmap_height;
      }
      else {
        printf ("Error in density specification: density = %d\n",den);
        return;
      }
    }
    else if (ang==45) {
      if (den==5) {
        bitmap_bits = line_45_5_bitmap_bits;
        bitmap_width = line_45_5_bitmap_width;
        bitmap_height = line_45_5_bitmap_height;
      }
      else if (den==4) {
        bitmap_bits = line_45_4_bitmap_bits;
        bitmap_width = line_45_4_bitmap_width;
        bitmap_height = line_45_4_bitmap_height;
      }
      else if (den==3) {
        bitmap_bits = line_45_3_bitmap_bits;
        bitmap_width = line_45_3_bitmap_width;
        bitmap_height = line_45_3_bitmap_height;
      }
      else if (den==2) {
        bitmap_bits = line_45_2_bitmap_bits;
        bitmap_width = line_45_2_bitmap_width;
        bitmap_height = line_45_2_bitmap_height;
      }
      else if (den==1) {
        bitmap_bits = line_45_1_bitmap_bits;
        bitmap_width = line_45_1_bitmap_width;
        bitmap_height = line_45_1_bitmap_height;
      }
      else {
        printf ("Error in density specification: density = %d\n",den);
        return;
      }
    }
    else if (ang==60) {
      if (den==5) {
        bitmap_bits = line_60_5_bitmap_bits;
        bitmap_width = line_60_5_bitmap_width;
        bitmap_height = line_60_5_bitmap_height;
      }
      else if (den==4) {
        bitmap_bits = line_60_4_bitmap_bits;
        bitmap_width = line_60_4_bitmap_width;
        bitmap_height = line_60_4_bitmap_height;
      }
      else if (den==3) {
        bitmap_bits = line_60_3_bitmap_bits;
        bitmap_width = line_60_3_bitmap_width;
        bitmap_height = line_60_3_bitmap_height;
      }
      else if (den==2) {
        bitmap_bits = line_60_2_bitmap_bits;
        bitmap_width = line_60_2_bitmap_width;
        bitmap_height = line_60_2_bitmap_height;
      }
      else if (den==1) {
        bitmap_bits = line_60_1_bitmap_bits;
        bitmap_width = line_60_1_bitmap_width;
        bitmap_height = line_60_1_bitmap_height;
      }
      else {
        printf ("Error in density specification: density = %d\n",den);
        return;
      }
    }
    else if (ang==-30) {
      if (den==5) {
        bitmap_bits = line_120_5_bitmap_bits;
        bitmap_width = line_120_5_bitmap_width;
        bitmap_height = line_120_5_bitmap_height;
      }
      else if (den==4) {
        bitmap_bits = line_120_4_bitmap_bits;
        bitmap_width = line_120_4_bitmap_width;
        bitmap_height = line_120_4_bitmap_height;
      }
      else if (den==3) {
        bitmap_bits = line_120_3_bitmap_bits;
        bitmap_width = line_120_3_bitmap_width;
        bitmap_height = line_120_3_bitmap_height;
      }
      else if (den==2) {
        bitmap_bits = line_120_2_bitmap_bits;
        bitmap_width = line_120_2_bitmap_width;
        bitmap_height = line_120_2_bitmap_height;
      }
      else if (den==1) {
        bitmap_bits = line_120_1_bitmap_bits;
        bitmap_width = line_120_1_bitmap_width;
        bitmap_height = line_120_1_bitmap_height;
      }
      else {
        printf ("Error in density specification: density = %d\n",den);
        return;
      }
    }
    else if (ang==-45) {
      if (den==5) {
        bitmap_bits = line_135_5_bitmap_bits;
        bitmap_width = line_135_5_bitmap_width;
        bitmap_height = line_135_5_bitmap_height;
      }
      else if (den==4) {
        bitmap_bits = line_135_4_bitmap_bits;
        bitmap_width = line_135_4_bitmap_width;
        bitmap_height = line_135_4_bitmap_height;
      }
      else if (den==3) {
        bitmap_bits = line_135_3_bitmap_bits;
        bitmap_width = line_135_3_bitmap_width;
        bitmap_height = line_135_3_bitmap_height;
      }
      else if (den==2) {
        bitmap_bits = line_135_2_bitmap_bits;
        bitmap_width = line_135_2_bitmap_width;
        bitmap_height = line_135_2_bitmap_height;
      }
      else if (den==1) {
        bitmap_bits = line_135_1_bitmap_bits;
        bitmap_width = line_135_1_bitmap_width;
        bitmap_height = line_135_1_bitmap_height;
      }
      else {
        printf ("Error in density specification: density = %d\n",den);
        return;
      }
    }
    else if (ang==-60) {
      if (den==5) {
        bitmap_bits = line_150_5_bitmap_bits;
        bitmap_width = line_150_5_bitmap_width;
        bitmap_height = line_150_5_bitmap_height;
      }
      else if (den==4) {
        bitmap_bits = line_150_4_bitmap_bits;
        bitmap_width = line_150_4_bitmap_width;
        bitmap_height = line_150_4_bitmap_height;
      }
      else if (den==3) {
        bitmap_bits = line_150_3_bitmap_bits;
        bitmap_width = line_150_3_bitmap_width;
        bitmap_height = line_150_3_bitmap_height;
      }
      else if (den==2) {
        bitmap_bits = line_150_2_bitmap_bits;
        bitmap_width = line_150_2_bitmap_width;
        bitmap_height = line_150_2_bitmap_height;
      }
      else if (den==1) {
        bitmap_bits = line_150_1_bitmap_bits;
        bitmap_width = line_150_1_bitmap_width;
        bitmap_height = line_150_1_bitmap_height;
      }
      else {
        printf ("Error in density specification: density = %d\n",den);
        return;
      }
    }
    else if (ang==90||ang==-90) {
      if (den==5) {
        bitmap_bits = line_90_5_bitmap_bits;
        bitmap_width = line_90_5_bitmap_width;
        bitmap_height = line_90_5_bitmap_height;
      }
      else if (den==4) {
        bitmap_bits = line_90_4_bitmap_bits;
        bitmap_width = line_90_4_bitmap_width;
        bitmap_height = line_90_4_bitmap_height;
      }
      else if (den==3) {
        bitmap_bits = line_90_3_bitmap_bits;
        bitmap_width = line_90_3_bitmap_width;
        bitmap_height = line_90_3_bitmap_height;
      }
      else if (den==2) {
        bitmap_bits = line_90_2_bitmap_bits;
        bitmap_width = line_90_2_bitmap_width;
        bitmap_height = line_90_2_bitmap_height;
      }
      else if (den==1) {
        bitmap_bits = line_90_1_bitmap_bits;
        bitmap_width = line_90_1_bitmap_width;
        bitmap_height = line_90_1_bitmap_height;
      }
      else {
        printf ("Error in density specification: density = %d\n",den);
        return;
      }
    }
    else {
      printf ("Error in angle specification: angle = %d\n",ang);
      return;
    }
  }
  else {
    printf ("Error in fill specification: type = %d\n",typ);
    return;
  }

  if (typ>1) {
    stipple_pixmap = XCreateBitmapFromData(display, win, (char*)bitmap_bits,
		     bitmap_width, bitmap_height);
    XSetStipple(display, gc, stipple_pixmap);
    XSetFillStyle(display, gc, FillStippled);
  }
}

/* Gives current window information */

gaint win_data (struct xinfo *xinf) {
  int absx, absy;
  Window dummy;
  XWindowAttributes result;

  if (display == (Display *) NULL || win == (Window) NULL) return 0;
  if (!XGetWindowAttributes (display, win, &result) ) return 0;
  if (!XTranslateCoordinates (display, win, RootWindow (display, snum), 0, 0,
			      &absx, &absy, &dummy) ) return 0;
  xinf->winid=(int)win;
  xinf->winw=result.width;  
  xinf->winh=result.height;  
  xinf->winb=result.border_width;
  xinf->winx=absx;  
  xinf->winy=absy;  
  return 1;
}
 
/* Given x,y page location, return screen pixel location */

void gxdgcoord (gadouble x, gadouble y, gaint *i, gaint *j) {
  if (batch) {
    *i = -999;
    *j = -999;
    return;
  }
  *i = (gaint)(x*xscl+0.5);
  *j = height - (gaint)(y*yscl+0.5);
}

void gxdimg(gaint *im, gaint imin, gaint jmin, gaint isiz, gaint jsiz) {
int i,j,col;

  if (batch) return;

  image = XGetImage(display, drwbl, imin, jmin, isiz, jsiz, AllPlanes, XYPixmap);
  if (image==NULL) {
    printf ("Unable to allocate image file for gxout imap. \n");
    return;
  }

  for (j=0; j<jsiz; j++) {
    for (i=0; i<isiz; i++) {
       col = *(im+j*isiz+i);
       if (col<255) XPutPixel(image,i,j,cvals[col]);
    }
  }
  XPutImage(display, drwbl, gc, image, 0, 0, imin, jmin, isiz, jsiz);
  XDestroyImage(image);
}

gadouble gxdch (char ch, gaint fn, gadouble x, gadouble y, gadouble w, gadouble h, gadouble rot) {
  return (-999); 
}
gadouble gxdqchl (char ch, gaint fn, gadouble w) {
  return (-999); 
}
void gxdopt (gaint opt) {
}
void gxsetpatt(gaint pnum) {
}
void gxdsignal (gaint sig) {
}
void gxdXflush (void) {
}
void gxdclip (gadouble xlo, gadouble xhi, gadouble ylo, gadouble yhi) {
}
void gxdcirc (gadouble x, gadouble y, gadouble r, gaint flg) {
}
void gxdcfg (void) {
  printf("X%d.%d ",X_PROTOCOL,X_PROTOCOL_REVISION);
}
/* Check to see if this display backend supports fonts */
gaint gxdckfont (void) {
  return (0);
}
