/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT for more information. */

/* Simplified X interface for Cairo -- no widgets, buttons, X-based pattern fill, etc. */

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

#include <cairo.h>
#include <cairo-xlib.h>

#include "gatypes.h"
#include "gx.h"
#include "gxC.h"
#include "bitmaps2.h"

/* function prototypes */
void gxdXflush (void);

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
static gadouble xscl,yscl;                  /* Window Scaling */
static gadouble xsize, ysize;               /* User specified size */
static gaint dblmode;                       /* single or double buffering */
static gaint width,height,depth;            /* Window dimensions */
static struct gevent *evbase;               /* Anchor for GrADS event queue */
static cairo_surface_t *surface=NULL,*surface2=NULL; /* Cairo surfaces */

/* All stuff passed to Xlib routines as args are put here in
   static areas since we are not invoking Xlib routines from main*/

static Visual *xvis;
static Screen *sptr;
Display *display=(Display *)NULL;
static gaint snum;
static XEvent report;
Window win=(Window) NULL;   /* used via extern in gagui */
static Drawable drwbl,drwbl2;
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
static gaint dblmode;                     /* single or double buffering */
static XSetWindowAttributes xsetw;
static gaint rstate = 1;              /* Redraw state -- when zero,
                                      acceptance of X Events is
                                      blocked. */
static gaint bsflg;                   /* Backing store enabled or not */
static gaint excnt;                   /* Count of exposes to skip */


/* tell x interface that we are in batch mode */

void gxdbat (void) {
  batch = 1;
}

/* Routine to specify user-defined geometry string for X display window. 
   Must be called before gxdbgn to have any affect */

void gxdgeo (char *arg) {
  ugeom = arg;
}

void gxdbgn (gadouble xsz, gadouble ysz) {
gaint dw,dh,flag,ipos,jpos,border;

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
  if ((display=XOpenDisplay(display_name)) == NULL) {
    printf("Error in GXSTRT: Unable to connect to X server\n");
    exit(-1);
  }

  /* Get screen size from display structure macro, then figure out proper window size */
  snum = DefaultScreen(display);
  sptr = DefaultScreenOfDisplay(display);
  xvis = DefaultVisual(display,snum);
  bsflg = 0;
  if (DoesBackingStore(sptr)) bsflg = 1;
  dw = DisplayWidth(display, snum);
  dh = DisplayHeight(display, snum);
  depth = DefaultDepth(display, snum);
  ipos = 0;
  jpos = 0;

  /* window sizes are scaled according to the height of display */
  if ( xsize >= ysize ) {           /* landscape */
    if (xsize==11.0 && ysize==8.5) {    /* preserve old default */
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
    XGeometry (display,snum,ugeom,dgeom,4,1,1,0,0,&ipos,&jpos,&dw,&dh);
    size_hints.flags = USPosition | USSize ;
  }
  size_hints.x = ipos;
  size_hints.y = jpos;
  size_hints.width = dw;
  size_hints.height = dh;
  width = dw ; 
  height = dh ;
  xscl = (gadouble) (dw) / xsize ;
  yscl = (gadouble) (dh) / ysize ;

  /* Create window */
  win = XCreateSimpleWindow(display,RootWindow(display,snum),ipos,jpos,dw,dh,border,
			    WhitePixel(display,snum),BlackPixel(display,snum));

  /* Set up icon pixmap */
  icon_pixmap = XCreateBitmapFromData(display,win,(char*)icon_bitmap_bits,
				      icon_bitmap_width,icon_bitmap_height);

  /* Set standard properties */
  XSetStandardProperties(display,win,window_name,icon_name,icon_pixmap,argv,argc,&size_hints);

  /* Select event types */
  XSelectInput(display, win, ButtonReleaseMask | ButtonPressMask | ExposureMask | StructureNotifyMask);

  /* Display Window */
  XMapWindow(display,win);

  /* We now have to wait for the expose event that indicates our
     window is up.  Also handle any resizes so we get the scaling
     right in case it gets changed right away.  We will check again
     for resizes during frame operations */
  flag = 1;
  while (flag)  {
    XNextEvent(display,&report);
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

  /* Now ready for drawing */
  drwbl = win;   /* Initial drawable is the visible window */
  dblmode = 0;   /* Initially no double buffering mode     */

  xsetw.backing_store = Always;
  XChangeWindowAttributes (display,win,CWBackingStore,&xsetw);

  /* Set up the cairo Xlib surface and initialize gxC */
  surface = cairo_xlib_surface_create (display,drwbl,xvis,dw,dh);
  gxCbgn(surface,xsz,ysz,dw,dh);

}

void gxdend (void) {
  gxCend();
  cairo_surface_finish (surface);
  cairo_surface_destroy (surface);
  XCloseDisplay(display);
}

/* Frame action.  Values for action are:
      0 -- new frame (clear display), wait before clearing.
      1 -- new frame, no wait.
      2 -- New frame in double buffer mode.
      7 -- new frame, but just clear graphics.  Do not clear
           event queue; redraw widgets.
      8 -- clear only the event queue.
      9 -- flush the X request buffer 
*/

void gxdfrm (gaint iact) {
struct gevent *geve,*geve2;
  if (iact==9) {
    gxdeve(0);
    gxdXflush();
    return;
  }
  if (iact==0 || iact==1 || iact==7) {
    gxCfrm();
  }

  /* Flush X event queue. If iact is 7, keep the event info, otherwise discard it. */
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

  if (iact==7) gxdXflush();
}

/* Examine X event queue.  Flag tells us if we should wait for an
   event.  Any GrADS events (mouse-button presses) are queued.
   If flag is 2, wait for any event, not just a mouse event.  */

void gxdeve (gaint flag) {
struct gevent *geve, *geve2;
gaint i,j,rc,wflg,button,eflg,rdrflg;

  if (flag && evbase) flag = 0;   /* Don't wait if an event stacked */
  wflg = flag;
  eflg = 0;
  rdrflg = 0;
  while (1) {
    if (wflg && !rdrflg) {
      XMaskEvent(display, ButtonReleaseMask | ButtonPressMask |
          ExposureMask | StructureNotifyMask, &report);
      rc = 1;
    } else {
      rc = XCheckMaskEvent(display, ButtonReleaseMask | ButtonPressMask |
          ExposureMask | StructureNotifyMask, &report);
    }
    if (!rc && rdrflg) {
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
    evbase = geve->forw;       /* Take event off queue */
    free(geve);
  }

}

gaint gxdacol (gaint clr, gaint red, gaint green, gaint blue, gaint alpha) { 
  /* define a new color -- this is a no-op for cairo */
  return(0);
}

void gxdcol (gaint clr) {                    /* set color to new value */
  gxCcol(clr);
}

void gxdwid (gaint wid){                     /* Set line width */
  gxCwid(wid);
}

void gxdmov (gadouble x, gadouble y){        /* Move to x,y   */
  gxCmov(x,y);
}

void gxddrw (gadouble x, gadouble y) {       /* Draw to x,y */
  gxCdrw (x,y);
  if (QLength(display)&&rstate) gxdeve(0);
}

void gxdrec (gadouble x1, gadouble x2, gadouble y1, gadouble y2) {   /* draw a filled rectangle */
  gxCrec(x1,x2,y1,y2);
  if (QLength(display)&&rstate) gxdeve(0);
}
 
void gxdcirc (gadouble x, gadouble y, gadouble r, gaint flg) {
  gxCcirc(x,y,r,flg);
  if (QLength(display)&&rstate) gxdeve(0);
}

void gxddbl (void) {                         /* turn on double buffer mode */
  XSync(display, 0);
  gxCfrm();  /* clear the foreground */
  /* create a new background surface, set it to be the current drawable */
  surface2 = cairo_surface_create_similar (surface, CAIRO_CONTENT_COLOR_ALPHA, width, height);
  cairo_xlib_surface_set_drawable(surface2,drwbl,width,height);
  gxCsfc(surface2);   /* set the cairo graphics context to the new background surface */
  drwbl2 = cairo_xlib_surface_get_drawable(surface2); 
  drwbl = drwbl2;
  gxCfrm();  /* clear the new background surface */
  dblmode = 1; 
  return;
}

void gxdswp (void) {                        /* swap command -- copy backgroud to the foreground */
  if (dblmode) {
     /* this will draw the background (surface2) onto the foreground (surface) and
	reset the cairo context to the background surface  */
    gxCswp(surface,surface2);
    gxCfrm();     /* clear the background */
   }
  return;
}

void gxdsgl (void) {                         /* turn off double buffer mode */
  if (dblmode) {
    /* destroy the background surface */
     cairo_surface_destroy(surface2);
     /* set the window as the current drawable */
     drwbl = win;
     cairo_xlib_surface_set_drawable(surface,drwbl,width,height);
     gxCsfc(surface);   /* this will set the cairo graphics context to the foreground surface */
  }
  dblmode = 0;
  return;
}

void gxdfil (gadouble *xy, gaint n) {        /* draw a filled polygon */
  gxCfil (xy,n);
  if (QLength(display)&&rstate) gxdeve(0);
}

void gxdxsz (gaint xx, gaint yy) {           /* resize the X window */
  if (batch) return;
  if (xx==width && yy==height) return;
  XResizeWindow (display, win, xx, yy);
  gxdeve(2);
}

void gxdrdw (void) {                        /* Redraw when user resizes window */
  rstate = 0;
  cairo_xlib_surface_set_size(surface, width, height);
  gxCrsiz (width, height);
  gxhdrw(0,0);
  rstate = 1;
}

/* no widgets -- all routines are no-ops (with warning messages) */


/* button widget */
void gxdpbn (gaint bnum, struct gbtn *pbn, gaint redraw, gaint btnrel, gaint nstat) { 
  printf("Warning: The Cairo graphics display interface does not support buttons\n");
}

/* drop menu */
void gxdrmu (gaint mnum, struct gdmu *pmu, gaint redraw, gaint nstat) { 
  printf("Warning: The Cairo graphics display interface does not support drop menus\n");
}

/* rubber-band region */
void gxdrbb (gaint num, gaint type, gadouble xlo, gadouble ylo, gadouble xhi, gadouble yhi, gaint mbc) {
  printf("Warning: The Cairo graphics display interface does not support rubber band widgets\n");
}

/* Dialog box */
char *gxdlg (struct gdlg *qry) {
  printf("Warning: The Cairo graphics display interface does not support dialog boxes\n");
  return (NULL);
}

/* Reset a widget */
void gxrs1wd (int wdtyp, int wdnum) {
}

/* Screen save and show operations, also no-ops */
void gxdssv (int frame) {
  printf("Warning: The Cairo graphics display interface does not support the screen command\n");
}
void gxdssh (int cnt) {
  printf("Warning: The Cairo graphics display interface does not support the screen command\n");
}
void gxdsfr (int frame) {
  printf("Warning: The Cairo graphics display interface does not support the screen command\n");
}

/* Routine to install stipple pixmaps, a no-op */

void gxdptn (int typ, int den, int ang) {
}

/* Gives current window information */

gaint win_data (struct xinfo *xinf) {
  int absx, absy;
  Window dummy;
  XWindowAttributes result;

  if (batch) return (0);
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
  printf("Warning: The Cairo graphics display interface does not support 'gxout imap'\n");
}

/* Get the x bearing (width) used to plot a character */

gadouble gxdqchl (char ch, gaint fn, gadouble w) {
  return (gxCqchl(ch,fn,w)); 
}

/* Plot a character */
gadouble gxdch (char ch, gaint fn, gadouble x, gadouble y, gadouble w, gadouble h, gadouble rot) {
gadouble xadv;

  xadv = gxCch(ch,fn,x,y,w,h,rot);   /* draw character with Cairo, width of character returned */
  if (QLength(display)&&rstate) gxdeve(0);
  return (xadv);
}

/* Flush pending graphics; called from gxC and gxX */
void gxdXflush (void) {
  XFlush(display);
}

/* Allows higher levels to indicate various options, conditions, etc. to the
   hardware rendering layer.  */
/* opt = 4:  major finish point; complete any ongoing graphics rendering */

void gxdopt (gaint opt) {
  if (opt==4) gxCflush(1);
}

void gxsetpatt (gaint pnum) {
  gxCpattrset(pnum);   
}

void gxdsignal (gaint sig) {
  if (sig==1) gxCflush(1);   /* finish rendering */
  if (sig==2) gxCaa(0);      /* disable anti-aliasing */
  if (sig==3) gxCaa(1);      /* enable anti-aliasing */
  if (sig==4) gxCpush();     /* push */
  if (sig==5) gxCpop();      /* pop */
}

void gxdclip (gadouble xlo, gadouble xhi, gadouble ylo, gadouble yhi) {
  gxCclip (xlo,xhi,ylo,yhi);
}

void gxdcfg (void) {
  printf("X%d.%d ",X_PROTOCOL,X_PROTOCOL_REVISION);
  gxCcfg();
}

/* Check to see if this display backend supports fonts */
gaint gxdckfont (void) {
  return (1);
}
