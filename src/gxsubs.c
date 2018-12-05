/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* Authored by B. Doty */

/*  Low level graphics interface, providing scaling, line styles,
    clipping, character drawing, metafile output, etc.         */

#ifdef HAVE_CONFIG_H
#include "config.h"

/* If autoconfed, only include malloc.h when it's presen */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#else /* undef HAVE_CONFIG_H */

#include <malloc.h>

#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dlfcn.h>
#include "gatypes.h"
#include "gx.h"

char *gaqupb (char *, gaint);
void gree ();

/* The following variables are local to this file, and are used by
   all the routines in the file.    */

static char *datad = "/usr/local/lib/grads";
static gadouble xsize, ysize;                /* Virtual page size  */
static gadouble rxsize, rysize;              /* Real page size     */
static gaint lwflg;                          /* Reduce lw due vpage*/
static gadouble clminx,clmaxx,clminy,clmaxy; /* Clipping region    */
static gaint cflag;                          /* Clipping flag      */
static gaint mflag;                          /* mask flag          */
static gadouble dash[8];                     /* Linestyle pattern  */
static gaint dnum,lstyle;                    /* Current linestyle  */
static gaint lcolor;                         /* Current color      */
static gaint lwide;                          /* Current linewidth  */
static gadouble oldx,oldy;                   /* Previous position  */
static gaint bufmod;                         /* Buffering mode     */
static gadouble xsave,ysave,alen,slen;       /* Linestyle constants*/
static gaint jpen,dpnt;
static gaint intflg;                         /* Batch/Interactive flag    */
static void (*fconv) (gadouble, gadouble, gadouble *, gadouble *); /* for proj rnt */
static void (*gconv) (gadouble, gadouble, gadouble *, gadouble *); /* for grid rnt */
static void (*bconv) (gadouble, gadouble, gadouble *, gadouble *); /* for back transform rnt */
static gaint bcol;                           /* background color */
static gaint savcol;                         /* for color save/restore */
static char *mask;                           /* pointer to mask array */
static gaint maskx;                          /* Size of a row in the array */
static gaint masksize;                       /* Size of mask array */
static gaint maskflg;                        /* mask flag; -999 no mask yet,
                                                0 no mask used, 1 mask values set, -888 error  */
static struct gxpsubs psubs;                 /* Holds function pointers to printing subroutines */
static struct gxdsubs dsubs;                 /* Holds function pointers to display subroutines */

/* For STNDALN, routines included are gxgnam and gxgsym */
#ifndef STNDALN

/* Initialize graphics output  */
/* If batch flag is 1, batch mode only (no graphics output) */

gaint gxstrt (gadouble xmx, gadouble ymx, gaint batch, gaint hbufsz, char *gxdopt, char *gxpopt, char *xgeom) {
  gaint rc;

  printf ("GX Package Initialization: Size = %g %g \n",xmx,ymx);
  if (batch) printf ("Running in Batch mode\n");
  intflg = !batch;                            /* Set batch/interactive flag */
  gxdbinit();                                 /* Initialize the graphics data base */
  rc = gxload(gxdopt,gxpopt);                 /* Load the graphics routines from a shared library */
  if (rc) {
    printf("GX Package Terminated \n"); 
    return (rc);
  }
  if (intflg) {
    if (xgeom[0]!='\0') dsubs.gxdgeo(xgeom);  /* tell display software about geometry override */
    dsubs.gxdbgn(xmx, ymx);                   /* Initialize graphics output */
    dsubs.gxdwid(3);                          /* Initial line width */
  } else {				      		       		    
    dsubs.gxdbat();                           /* Tell display hardware layer we're in batch mode */
    psubs.gxpinit(xmx, ymx);                  /* printing layer initializes batch mode surface */
  }					  					    		  
  psubs.gxpbgn (xmx, ymx);                    /* Tell printing layer about page size */
  rxsize = xmx;                               /* Set local variables with real page size  */
  rysize = ymx;				  				   
  clminx=0; clmaxx=xmx;                       /* Set clipping area       */
  clminy=0; clmaxy=ymx;
  xsave=0.0; ysave=0.0;
  lstyle=0; lwide=3;
  oldx=0.0; oldy=0.0;
  fconv=NULL;                                 /* No projection set up    */
  gconv=NULL;                                 /* No grid scaling set up  */
  bconv=NULL;                                 /* No back transform       */
  gxchii();                                   /* Init character plotting */
  bufmod=0;                                   /* double buffering is OFF */
  gxhnew(rxsize,rysize,hbufsz);               /* Init hardcopy buffering */
  gxscal (0.0,xmx,0.0,ymx,0.0,xmx,0.0,ymx);   /* Linear scaling=inches   */
  gxvpag (xmx,ymx,0.0,xmx,0.0,ymx);           /* Virtual page scaling    */
  mask = NULL; maskflg = -999;                /* Don't allocate mask until first use */
  gxcolr(1);                                  /* Initial color is 1 (foreground) */
  return(0);
}

/* Loads the graphics back end shared libraries and define the required subroutines */
gaint gxload(char *gxdopt, char *gxpopt) {
  void *phandle=NULL,*dhandle=NULL; 
  const char *err=NULL,*dname=NULL,*pname=NULL;
  char *cname=NULL;
  FILE *cfile;
  
  /* Printing Hardcopy */
  pname=(const char *)gaqupb(gxpopt,4);
  if (pname==NULL) {
    printf("GX Package Error: Could not find a record for the printing plug-in named \"%s\" \n",gxpopt);
    /* Tell user where we looked based on $GAUDPT */
    cname = getenv("GAUDPT");
    if (cname==NULL) {
      printf("  * The environment variable GAUDPT has not been set\n");
    } 
    else {
      cfile = fopen(cname,"r");
      if (cfile==NULL) {
        printf("  * Unable to open the file named by the GAUDPT environment variable: %s\n",cname);
      }
      else {
        printf("  * No entry with \"gxprint %s\" in the file named by the GAUDPT environment variable: %s\n",gxpopt,cname);
        fclose(cfile);
      }
    }
    /* Tell user where we looked based on $GADDIR/udpt */
    cname = gxgnam("udpt");
    cfile = fopen(cname,"r");
    if (cfile==NULL) {
      printf("  * Unable to open the default User Defined Plug-in Table: %s\n",cname);
    }
    else {
      printf("  * No entry with \"gxprint %s\" in the default User Defined Plug-in Table: %s\n",gxpopt,cname);
      fclose(cfile);
    }
    printf("  Please read the documentation at http://cola.gmu.edu/grads/gadoc/plugins.html\n");
    return(1);
  }
  dlerror();
  phandle = dlopen (pname, RTLD_LAZY);
  if (!phandle) {
    printf("GX Package Error: dlopen failed to get a handle on gxprint plug-in named \"%s\" \n",gxpopt); 
    if ((err=dlerror())!=NULL) printf("   %s\n",err); 
    return(1);
  }

  /* Display */
  dname=(const char *)gaqupb(gxdopt,3);
  if (dname==NULL) {
    printf("GX Package Error: Could not find a record for the display plug-in named \"%s\" \n",gxpopt);
    /* Tell user where we looked based on $GAUDPT */
    cname = getenv("GAUDPT");
    if (cname==NULL) {
      printf("  * The environment variable GAUDPT has not been set\n");
    } 
    else {
      cfile = fopen(cname,"r");
      if (cfile==NULL) {
        printf("  * Unable to open the file named by the GAUDPT environment variable: %s\n",cname);
      }
      else {
        printf("  * No entry with \"gxdisplay %s\" in the file named by the GAUDPT environment variable: %s\n",gxdopt,cname);
        fclose(cfile);
      }
    }
    /* Tell user where we looked based on $GADDIR/udpt */
    cname = gxgnam("udpt");
    cfile = fopen(cname,"r");
    if (cfile==NULL) {
      printf("  * Unable to open the default User Defined Plug-in Table: %s\n",cname);
    }
    else {
      printf("  * No entry with \"gxdisplay %s\" in the default User Defined Plug-in Table: %s\n",gxdopt,cname);
      fclose(cfile);
    }
    printf("  Please read the documentation at http://cola.gmu.edu/grads/gadoc/plugins.html\n");
    return(1);
  }
  dlerror();
  dhandle = dlopen (dname, RTLD_LAZY);
  if (!dhandle) {
    printf("GX Package Error: dlopen failed to get a a handle on gxdisplay plug-in named \"%s\" \n",gxdopt); 
    if ((err=dlerror())!=NULL) printf("   %s\n",err); 
    return(2);
  }
  
  /* Get pointers to the printing subroutines */
  dlerror();
  psubs.gxpcfg   = dlsym(phandle,"gxpcfg");
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpckfont= dlsym(phandle,"gxpckfont");
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpbgn   = dlsym(phandle,"gxpbgn");   
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpinit  = dlsym(phandle,"gxpinit");  
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpend   = dlsym(phandle,"gxpend");   
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxprint  = dlsym(phandle,"gxprint");  
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpcol   = dlsym(phandle,"gxpcol");   
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpacol  = dlsym(phandle,"gxpacol");  
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpwid   = dlsym(phandle,"gxpwid");   
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxprec   = dlsym(phandle,"gxprec");   
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpcirc  = dlsym(phandle,"gxpcirc");   
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpbpoly = dlsym(phandle,"gxpbpoly"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpepoly = dlsym(phandle,"gxpepoly"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpmov   = dlsym(phandle,"gxpmov");   
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpdrw   = dlsym(phandle,"gxpdrw");   
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpflush = dlsym(phandle,"gxpflush"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpsignal= dlsym(phandle,"gxpsignal");
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpclip  = dlsym(phandle,"gxpclip");  
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpch    = dlsym(phandle,"gxpch");    
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  psubs.gxpqchl  = dlsym(phandle,"gxpqchl");  
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  
  /* get pointers to the display (hardware) subroutines, some are needed even in batch mode */
  dsubs.gxdcfg   = dlsym(dhandle,"gxdcfg");
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  dsubs.gxdckfont= dlsym(dhandle,"gxdckfont");
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(1);}
  dsubs.gxdbb    = dlsym(dhandle,"gxdbb"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdfb    = dlsym(dhandle,"gxdfb"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdacol  = dlsym(dhandle,"gxdacol"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdbat   = dlsym(dhandle,"gxdbat"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdbgn   = dlsym(dhandle,"gxdbgn"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdbtn   = dlsym(dhandle,"gxdbtn"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdch    = dlsym(dhandle,"gxdch"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdclip  = dlsym(dhandle,"gxdclip"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdcol   = dlsym(dhandle,"gxdcol"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxddbl   = dlsym(dhandle,"gxddbl"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxddrw   = dlsym(dhandle,"gxddrw"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdend   = dlsym(dhandle,"gxdend"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdfil   = dlsym(dhandle,"gxdfil"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdfrm   = dlsym(dhandle,"gxdfrm"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdgcoord= dlsym(dhandle,"gxdgcoord"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdgeo   = dlsym(dhandle,"gxdgeo"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdimg   = dlsym(dhandle,"gxdimg"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdlg    = dlsym(dhandle,"gxdlg"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdmov   = dlsym(dhandle,"gxdmov"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdopt   = dlsym(dhandle,"gxdopt"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdpbn   = dlsym(dhandle,"gxdpbn"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdptn   = dlsym(dhandle,"gxdptn"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdptn   = dlsym(dhandle,"gxdptn"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdqchl  = dlsym(dhandle,"gxdqchl"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdrbb   = dlsym(dhandle,"gxdrbb"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdrec   = dlsym(dhandle,"gxdrec"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdcirc  = dlsym(dhandle,"gxdcirc"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdrmu   = dlsym(dhandle,"gxdrmu"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdsfr   = dlsym(dhandle,"gxdsfr"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdsgl   = dlsym(dhandle,"gxdsgl"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdsignal= dlsym(dhandle,"gxdsignal"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdssh   = dlsym(dhandle,"gxdssh"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdssv   = dlsym(dhandle,"gxdssv"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdswp   = dlsym(dhandle,"gxdswp"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdwid   = dlsym(dhandle,"gxdwid"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdXflush= dlsym(dhandle,"gxdXflush"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxdxsz   = dlsym(dhandle,"gxdxsz"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxrs1wd  = dlsym(dhandle,"gxrs1wd"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.gxsetpatt= dlsym(dhandle,"gxsetpatt"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  dsubs.win_data = dlsym(dhandle,"win_data"); 
  if ((err=dlerror())!=NULL) {printf("Error in gxload: %s\n",err); return(2);}
  return(0);
}

/* pass the pointer to the graphcis printing subroutines to gxmeta.c and gauser.c */
struct gxpsubs* getpsubs() {
  return &psubs; 
}

/* pass the pointer to the graphcis display subroutines to gxmeta.c and gauser.c */
struct gxdsubs* getdsubs() {
  return &dsubs; 
}

/* get configuration information from display and printing plug-ins */
void gxcfg (char *gxdopt, char *gxpopt) {
  const char *dname,*pname;
  dname=(const char *)gaqupb(gxdopt,3);
  printf(" -+- GX Display \"%s\"  %s  ",gxdopt,dname); dsubs.gxdcfg(); printf("\n");
  pname=(const char *)gaqupb(gxpopt,4);
  printf(" -+- GX Print   \"%s\"  %s  ",gxpopt,pname); psubs.gxpcfg(); printf("\n");
}

/* Terminate graphics output */
void gxend (void) {
  if (mask) free(mask);
  if (intflg) 
    dsubs.gxdend();                              /* Close X11 window */
  else
    psubs.gxpend();                              /* Tell printing layer to destroy batch mode surface */
  printf ("GX Package Terminated \n");
}

/* Send a signal to the rendering engine and the metabuffer. 
   Signal values are: 
     1 == Done with draw/display so finish rendering
     2 == Disable anti-aliasing 
     3 == Enable anti-aliasing 
     4 == Cairo push
     5 == Cairo pop and then paint
*/
void gxsignal (gaint sig) {
  if (intflg) dsubs.gxdsignal(sig);  /* tell the rendering layer about the signal */
  hout1c(-22,sig);                   /* put the signal in the metafile buffer */
}


/* Query the width of a character */
gadouble gxqchl (char ch, gaint fn, gadouble w) {
gadouble wid;

  /* cases where we want to use Hershey fonts */
  if (fn==3) return (-999.9);                  /* symbol font */
  if (fn>5 && fn<10) return (-999.9);          /* user-defined font file */
  if (fn<6 && !gxdbqhersh()) return (-999.9);  /* font 0-5 and hershflag=0 in gxmeta.c */

  if (intflg) 
    wid = dsubs.gxdqchl (ch, fn, w);           /* get the character width (interactive mode) */
  else 
    wid = psubs.gxpqchl (ch, fn, w);           /* get the character width (batch mode) */
  return (wid);
}

/* Draw a character */
gadouble gxdrawch (char ch, gaint fn, gadouble x, gadouble y, gadouble w, gadouble h, gadouble rot) {
gadouble wid;

  /* cases where we want to use Hershey fonts */ 
  if (fn==3) return (-999.9);                  /* symbol font */
  if (fn>5 && fn<10) return (-999.9);          /* user-defined font file */
  if (fn<6 && !gxdbqhersh()) return (-999.9);  /* font 0-5 and hershflag=0 in gxmeta.c */

  /* from here on we're using Cairo fonts */
  gxvcon(x,y,&x,&y);                           /* scale the position and size for the virual page */
  gxvcon2(w,h,&w,&h);
  if (w>0 && h>0) {                            /* make sure the width and height are non-zero */
    if (intflg) 
      wid = dsubs.gxdch (ch, fn, x, y, w, h, rot);   /* plot a character */
    else 
      wid = psubs.gxpqchl (ch, fn, w);         /* get the character width (batch mode) */
    houtch (ch, fn, x, y, w, h, rot);          /* put the character in the metabuffer */
    gxppvp2(wid,&wid);                         /* rescale the character width back to real page size */
    return (wid);                              /* return character width */
  }
  else return(0);
}


/* Frame action.  Values for action are:
      0 -- new frame (clear display), wait before clearing.
      1 -- new frame, no wait.
      2 -- New frame in double buffer mode.  If not supported
           has same result as action=1.  Usage involves multiple
           calls with action=2 to obtain an animation effect.  
      7 -- new frame, but just clear graphics.  Do not clear  
           event queue; redraw buttons. 
      8 -- clear only the event queue.
      9 -- clear only the X request buffer */

void gxfrme (gaint action) {

  if (action>7) { 
    if (intflg) dsubs.gxdfrm(action);
    return;
  }
  gxmaskclear();
  if (intflg) {
    if (action==0) getchar();        /* Wait if requested */
    if (action!=2 && bufmod) {
      dsubs.gxdsgl ();               /* tell hardware to turn off double buffer mode */
      bufmod=0;
    }
    if (action==2 && (!bufmod)) {
      dsubs.gxddbl ();               /* tell hardware to turn on double buffer mode */
      bufmod=1;
    }
    if (bufmod) dsubs.gxdswp ();     /* swap */
    dsubs.gxdfrm (action);           /* tell hardware layer about frame action */
    dsubs.gxdfrm (9);                /* clear the X request buffer */
  }
  gxhfrm (action);                   /* reset metabuffer */
  bcol = gxdbkq();
  if (bcol>1) {
    /* If background is not black/white, draw a full page rectangle and populate the metabuffer */
    savcol = lcolor;
    gxcolr(bcol);
    gxrecf(0.0, rxsize, 0.0, rysize);
    gxcolr(savcol);
  }
}


/* Set color.  Colors are: 0 - black;    1 - white
                           2 - red;      3 - green     4 - blue
                           5 - cyan;     6 - magenta   7 - yellow
                           8 - orange;   9 - purple   10 - lt. green
                          11 - m.blue   12 - yellow   13 - aqua
			  14 - d.purple 15 - gray
   Other colors may be available but are defined by the device driver */

void gxcolr (gaint clr){                 /* Set color     */
  if (clr<0) clr=0;
  if (clr>=COLORMAX) clr=COLORMAX-1; 
  hout1(-3,clr);
  if (intflg) dsubs.gxdcol (clr);
  lcolor = clr;
}

/* define a new color */

gaint gxacol (gaint clr, gaint red, gaint green, gaint blue, gaint alpha ) {
  gaint rc=0;
  gxdbacol (clr, red, green, blue, alpha);                       /* update the database */
  hout5i(-5,clr,red,green,blue,alpha);                           /* tell the metabuffer */
  if (intflg) rc = dsubs.gxdacol(clr, red, green, blue, alpha);  /* tell hardware */ 
  return(rc);
}


/* Set line weight */

void gxwide (gaint wid) {                 /* Set width     */
gaint hwid;
  hwid = wid;
  hout2i(-4,hwid,wid);
  if (intflg) dsubs.gxdwid (hwid);
  lwide = hwid;
}

/* Move to x, y with 'clipping'.  Clipping is implmented
   coarsely, where any move or draw point that is outside the
   clip region is not plotted.                          */

void gxmove (gadouble x, gadouble y) {        /* Move to x,y   */
  mflag = 0;
  oldx = x;
  oldy = y;
  if ( x<clminx || x>clmaxx || y<clminy || y>clmaxy ) {
    cflag=1;
    return;
  }
  cflag=0;
  gxvcon(x,y,&x,&y);
  hout2(-10,x,y);
  if (intflg) dsubs.gxdmov (x,y);
}

/* Draw to x, y with clipping */

void gxdraw (gadouble x, gadouble y) {        /* Draw to x,y   */
gadouble xnew,ynew;
gaint pos=0;
  if ( x<clminx || x>clmaxx || y<clminy || y>clmaxy ) {
    if (!cflag) {
      bdterp (oldx,oldy,x,y,&xnew,&ynew);
      gxvcon(xnew,ynew,&xnew,&ynew);
      hout2(-11,xnew,ynew);
      if (intflg) dsubs.gxddrw (xnew,ynew);
      cflag=1;
    }
    oldx = x; oldy = y;
    return;
  }
  if (cflag) {
    bdterp (oldx,oldy,x,y,&xnew,&ynew);
    cflag=0;
    gxvcon(xnew,ynew,&xnew,&ynew);
    hout2(-10,xnew,ynew);
    if (intflg) dsubs.gxdmov (xnew,ynew);
  }
  oldx = x; oldy = y;
  gxvcon(x,y,&x,&y);
  if (maskflg>0) pos = ((gaint)(y*100.0))*maskx + (gaint)(x*100.0);
  if (maskflg>0 && pos>0 && pos<masksize && *(mask+pos)=='1') {
    hout2(-10,x,y);
    if (intflg) dsubs.gxdmov (x,y);
    mflag = 1;
    return;
  }
  if (mflag) {
    hout2(-10,x,y);
    if (intflg) dsubs.gxdmov (x,y);
    mflag = 0;
    return;
  }
  hout2(-11,x,y);
  if (intflg) dsubs.gxddrw (x, y);
}

/* Draw lines in small segments, sometimes needed when masking is in use 
   (eg, grid lines)  */

void gxsdrw (gadouble x, gadouble y){ 
gadouble xdif,ydif,xx,yy,slope,incr;
gaint xnum,ynum,i;

  if (maskflg > 0) {
    ydif = fabs(oldy-y);
    xdif = fabs(oldx-x);
    if (ydif<0.03 && xdif<0.03) gxdraw(x,y);
    else {
      if (xdif>ydif) {
        incr = 0.03;
        if (ydif/xdif<0.3) incr = 0.02;
        xnum = (gaint)(xdif/incr);
        slope = (y-oldy)/(x-oldx);
        xx = oldx; yy = oldy;
        if (x < oldx) incr = -1.0 * incr;
        for (i=0; i<xnum; i++) {
          xx = xx + incr;
          yy = yy + incr*slope;
          gxdraw(xx,yy);
        }
        gxdraw(x,y);
      } else {
        incr = 0.03;
        if (xdif/ydif<0.3) incr = 0.02;
        ynum = (gaint)(ydif/incr);
        slope = (x-oldx)/(y-oldy);
        xx = oldx; yy = oldy;
        if (y < oldy) incr = -1.0 * incr;
        for (i=0; i<ynum; i++) {
          xx = xx + incr*slope;
          yy = yy + incr;
          gxdraw(xx,yy);
        }
        gxdraw(x,y);
      }
    } 
  } else {
    gxdraw (x,y);
  }
}

/* Set software linestyle */

void gxstyl (gaint style) {              /* Set line style  */
  if (style==-9) style=1;
  lstyle=style;
  if (style==2) {
    dnum=1;
    dash[0]=0.25;
    dash[1]=0.1;  }
  else if (style==3) {
    dnum=1;
    dash[0]=0.03;
    dash[1]=0.03;   }
  else if (style==4) {
    dnum=3;
    dash[0]=0.25;
    dash[1]=0.1;
    dash[2]=0.1;
    dash[3]=0.1;   }
  else if (style==5) {
    dnum=1;
    dash[0]=0.01;
    dash[1]=0.08;  }
  else if (style==6) {
    dnum=3;
    dash[0]=0.15;
    dash[1]=0.08;
    dash[2]=0.01; ;
    dash[3]=0.08;   }
  else if (style==7) {
    dnum=5;
    dash[0]=0.15;
    dash[1]=0.08;
    dash[2]=0.01;
    dash[3]=0.08;
    dash[4]=0.01;
    dash[5]=0.08;  }
  else lstyle=0;
  slen=dash[0]; jpen=2; dpnt=0;
}


/* Move and draw with linestyles and clipping */

void gxplot (gadouble x, gadouble y, gaint ipen ) {    /* Move or draw  */
gadouble x1,y1;

  if (lstyle<2) {
     if (ipen==2) gxdraw (x,y);
     else gxmove (x,y);
     xsave=x; ysave=y;
     return;
  }
  if (ipen==3) {
    slen=dash[0];
    dpnt=0;
    jpen=2;
    xsave=x;
    ysave=y;
    gxmove (x,y);
    return;
  }
  alen=hypot ((x-xsave),(y-ysave));
  if (alen<0.001) return;
  while (alen>slen) {
    x1=xsave+(x-xsave)*(slen/alen);
    y1=ysave+(y-ysave)*(slen/alen);
    if (jpen==2) gxdraw (x1,y1);
            else gxmove (x1,y1);
    dpnt+=1;
    if (dpnt>dnum) dpnt=0;
    slen=slen+dash[dpnt];
    jpen+=1;
    if (jpen>3) jpen=2;
  }
  slen=slen-alen;
  xsave=x;
  ysave=y;
  if (jpen==2) gxdraw (x,y);
          else gxmove (x,y);
  if (slen<0.001) {
    dpnt+=1;
    if (dpnt>dnum) dpnt=0;
    slen=dash[dpnt];
    jpen+=1;
    if (jpen>3) jpen=2;
  }
}

/* Specify software clip region.  */

void gxclip (gadouble xmin, gadouble xmax, gadouble ymin, gadouble ymax) {
gadouble clxmin,clxmax,clymin,clymax;

  /* for software clipping */
  clminx = xmin;
  clmaxx = xmax;
  clminy = ymin;
  clmaxy = ymax;
  if (clminx<0.0) clminx = 0.0;
  if (clmaxx>xsize) clmaxx = xsize;
  if (clminy<0.0) clminy = 0.0;
  if (clmaxy>ysize) clmaxy = ysize;

  /* specify the hardware clip region, and put it in the metabuffer as well */
  gxvcon(clminx,clminy,&clxmin,&clymin);
  gxvcon(clmaxx,clmaxy,&clxmax,&clymax);
  if (intflg) dsubs.gxdclip(clxmin,clxmax,clymin,clymax);
  hout4(-23,clxmin,clxmax,clymin,clymax); 
}

/* Constants for linear scaling */

static gadouble xm,xb,ym,yb;

/* Specify low level linear scaling (scaling level 1) */

void gxscal (gadouble xmin, gadouble xmax, gadouble ymin, gadouble ymax,
             gadouble smin, gadouble smax, gadouble tmin, gadouble tmax){
  xm=(xmax-xmin)/(smax-smin);
  xb=xmin-(xm*smin);
  ym=(ymax-ymin)/(tmax-tmin);
  yb=ymin-(ym*tmin);
}

/* Constants for virtual page scaling */

static gadouble vxm,vxb,vym,vyb;

/* Specify virtual page scaling. 
   Input args are as follows:
     xmax,ymax == virtual page sizes
     smin,smax == real page X-coordinates of virtual page
     tmin,tmax == real page Y-coordinates of virtual page
*/

void gxvpag (gadouble xmax, gadouble ymax,
	     gadouble smin, gadouble smax, gadouble tmin, gadouble tmax){
gadouble xmin, ymin;
  /* set virtual page size */
  xmin = 0.0;
  ymin = 0.0;
  xsize = xmax;  
  ysize = ymax;  
  /* check if virtual page coordinates extend beyond the real page size */
  if (smin<0.0) smin=0.0;
  if (smax>rxsize) smax = rxsize;
  if (tmin<0.0) tmin=0.0;
  if (tmax>rysize) tmax = rysize;
  /* set clipping area to virtual page */
  clminx = 0.0;
  clmaxx = xmax;
  clminy = 0.0;
  clmaxy = ymax;
  /* if virtual page is small, set a flag to reduce line thickness */
  if ((smax-smin)/rxsize < 0.49 || (tmax-tmin)/rysize < 0.49) lwflg = 1;
  else lwflg = 0;
  /* set up constants for virtual page scaling */
  vxm=(smax-smin)/(xmax-xmin);
  vxb=smin-(vxm*xmin);
  vym=(tmax-tmin)/(ymax-ymin);
  vyb=tmin-(vym*ymin);
  /* For non-software clipping ... put coordinates in the metabuffer and tell the hardware */
  gxclip(clminx,clmaxx,clminy,clmaxy); 
}

/* Do virtual page scaling conversion */

void gxvcon (gadouble s, gadouble t, gadouble *x, gadouble *y) {  /* positions, real->virtual */
  *x = s*vxm+vxb;
  *y = t*vym+vyb;
}

void gxvcon2 (gadouble s, gadouble t, gadouble *x, gadouble *y) {  /* characters, real->virtual */
  *x = s*vxm;
  *y = t*vym;
}
 
void gxppvp (gadouble x, gadouble y, gadouble *s, gadouble *t) {   /* positions, virtual->real */
  *s = (x-vxb)/vxm;
  *t = (y-vyb)/vym;
}

void gxppvp2 (gadouble x, gadouble *s) {  /* character width, virtual->real */
  *s = (x)/vxm;
}


/* Specify projection-level scaling, typically used for map
   projections.  The address of the routine to perform the scaling
   is provided.  This is scaling level 2, and is the level that
   mapping is done. */

void gxproj ( void (*fproj) (gadouble s, gadouble t, gadouble *x, gadouble *y)){

  fconv=fproj;
}

/* Specify grid level scaling, typically used to convert a grid
   to lat-lon values that can be input to the projection or linear
   level scaling.  The address of a routine is provided to perform
   the possibly non-linear scaling.  This is scaling level 3, and
   is the level that contouring is done.  */

void gxgrid ( void (*fproj) (gadouble s, gadouble t, gadouble *x, gadouble *y)){

  gconv=fproj;
}

/* Convert coordinates at a particular level to level 0 coordinates
   (hardware coords, 'inches').  The level of the input coordinates
   is provided.  User projection and grid scaling routines are called
   as needed.  */

void gxconv (gadouble s, gadouble t, gadouble *x, gadouble *y, gaint level) { 

  if (level>2 && gconv!=NULL) (*gconv)(s,t,&s,&t);
  if (level>1 && fconv!=0) (*fconv)(s,t,&s,&t);
  if (level>0) {
    s=s*xm+xb;
    t=t*ym+yb;
  }
  *x=s;
  *y=t;
}

/* Convert from level 0 coordinates (inches) to level 2 world
   coordinates.  The back transform is done via conversion
   linearly from level 0 to level 1, then calling the back
   transform map routine, if available, to do level 1 to level
   2 transform.  */

void gxxy2w (gadouble x, gadouble y, gadouble *s, gadouble *t) { 

  /* Do level 0 to level 1 */
  if (xm==0.0 || ym==0.0) {
    *s = -999.9;
    *t = -999.9;
    return;
  }
  *s = (x-xb)/xm;
  *t = (y-yb)/ym;

  /* Do level 1 to level 2 */
  if (bconv!=NULL) (*bconv)(*s,*t,s,t);
}

/* Allow caller to specify a routine to do the back transform from
   level 1 to level 2 coordinates. */
void gxback ( void (*fproj) (gadouble s, gadouble t, gadouble *x, gadouble *y)){

  bconv=fproj;
}


/* Convert from grid coordinates to map coordinates (level 3 to level 2) */
void gxgrmp (gadouble s, gadouble t, gadouble *x, gadouble *y) {

  if (gconv!=NULL) (*gconv)(s,t,&s,&t);
  *x = s;
  *y = t;
}

/* Convert an array of higher level coordinates to level 0 coordinates.
   The conversion is done 'in place' and the input coordinates are
   lost.  This routine performs the same function as coord except is
   somewhat more efficient for many coordinate transforms.         */

void gxcord (gadouble *coords, gaint num, gaint level) {
gaint i;
gadouble *xy;

  if (level>2 && gconv!=NULL) {
    xy=coords;
    for (i=0; i<num; i++) {
      (*gconv) (*xy,*(xy+1),xy,xy+1);
      xy+=2;
    }
  }

  if (level>1 && fconv!=NULL) {
    xy=coords;
    for (i=0; i<num; i++) {
      (*fconv) (*xy,*(xy+1),xy,xy+1);
      xy+=2;
    }
  }

  if (level>0) {
    xy=coords;
    for (i=0; i<num; i++) {
      *xy = *xy*xm+xb;
      xy++;
      *xy = *xy*ym+yb;
      xy++;
    }
  }
}

/* Delete level 3 or level 2 and level 3 scaling.  
   Level 1 scaling cannot be deleted.  */

void gxrset (gaint level) {

  if (level > 2) gconv=NULL;
  if (level > 1) { fconv=NULL; bconv=NULL; }
}

/* Plot a circle. flg=0 for open, flg=1 for filled. */

void gxcirc (gadouble x, gadouble y, gadouble r, gaint flg) {
gadouble xr,xr2,yr,rad;
  if (x<=clminx || x>=clmaxx || y<=clminy || y>=clmaxy) return;
  gxvcon (x,y,&xr,&yr);
  gxvcon (x+r,y,&xr2,&y);
  if (xr2>=xr)
    rad = xr2-xr;
  else
    rad = xr-xr2;
  if (flg)
    hout3(-24,xr,yr,rad);
  else
    hout3(-25,xr,yr,rad);
  if (intflg) {
    dsubs.gxdcirc (xr, yr, rad, flg);
  }
}


/* Plot a color filled rectangle.  */

void gxrecf (gadouble xlo, gadouble xhi, gadouble ylo, gadouble yhi) {
gadouble x;

  if (xlo>xhi) {
    x = xlo;
    xlo = xhi;
    xhi = x;
  }
  if (ylo>yhi) {
    x = ylo;
    ylo = yhi;
    yhi = x;
  }
  if (xhi<=clminx || xlo>=clmaxx || yhi<=clminy || ylo>=clmaxy) return;
  if (xlo<clminx) xlo = clminx;
  if (xhi>clmaxx) xhi = clmaxx;
  if (ylo<clminy) ylo = clminy;
  if (yhi>clmaxy) yhi = clmaxy;
  gxvcon (xlo,ylo,&xlo,&ylo);
  gxvcon (xhi,yhi,&xhi,&yhi);
  hout4(-6,xlo,xhi,ylo,yhi);
  if (intflg) {
    dsubs.gxdrec (xlo, xhi, ylo, yhi);
  }
}

/* Define fill pattern for rectangles and polygons. */

void gxptrn (gaint typ, gaint den, gaint ang) {
  hout3i(-12,typ,den,ang);
  if (intflg) dsubs.gxdptn (typ, den, ang);
}

/* query line width */

gaint gxqwid (void) {
  return (lwide);
}

/* query color */

gaint gxqclr (void) {
  return (lcolor);
}

/* query style */

gaint gxqstl (void) {
  return (lstyle);
}

/* Draw markers 1-5. */

void gxmark (gaint mtype, gadouble x, gadouble y, gadouble siz ) {
gadouble xy[80],siz2;
gaint i,ii,cnt;

  siz2 = siz/2.0;
  if (mtype==1) {                      /* cross hair */
    gxmove (x,y-siz2);
    gxdraw (x,y+siz2);
    gxmove (x-siz2,y);
    gxdraw (x+siz2,y);
    return;
  }
  if (mtype==2 || mtype==3 || mtype==10 || mtype==11) { /* circles */
    if (siz<0.1) ii = 30;
    else if (siz<0.3) ii = 15;
    else ii = 10;
    if (mtype>3) ii = 15;
    cnt = 0;
    for (i=60; i<415; i+=ii) {
      xy[cnt*2]   = x + siz2*cos((gadouble)(i)*pi/180.0);
      xy[cnt*2+1] = y + siz2*sin((gadouble)(i)*pi/180.0);
      cnt++;
    }
    xy[cnt*2]   = xy[0];
    xy[cnt*2+1] = xy[1];
    cnt++;
    if (mtype==2) {                  /* Open circle */
      gxmove(xy[0],xy[1]);
      for (i=1; i<cnt; i++) gxdraw (xy[i*2],xy[i*2+1]);
    } else if (mtype==3) {           /* Filled circle */
      gxfill (xy,cnt);
    } else if (mtype==10) {          /* Scattered fill */
      gxmove(xy[6],xy[7]);
      for (i=4; i<14; i++) gxdraw (xy[i*2],xy[i*2+1]);
      gxmove(xy[30],xy[31]);
      for (i=16; i<25; i++) gxdraw (xy[i*2],xy[i*2+1]);
      gxdraw (xy[0],xy[1]);
      for (i=8; i<14; i++) xy[i] = xy[i+18];
      xy[14] = xy[2]; xy[15] = xy[3];
      gxfill (xy+2,7);
    } else if (mtype==11) {          /* Broken fill */
      xy[0]  = x + siz2*cos(68.0*pi/180.0);
      xy[1]  = y + siz2*sin(68.0*pi/180.0);
      xy[8]  = x + siz2*cos(112.0*pi/180.0);
      xy[9]  = y + siz2*sin(112.0*pi/180.0);
      xy[24] = x + siz2*cos(248.0*pi/180.0);
      xy[25] = y + siz2*sin(248.0*pi/180.0);
      xy[32] = x + siz2*cos(292.0*pi/180.0);
      xy[33] = y + siz2*sin(292.0*pi/180.0);
      gxmove(xy[0],xy[1]);
      for (i=1; i<5; i++) gxdraw (xy[i*2],xy[i*2+1]);
      gxmove(xy[24],xy[25]);
      for (i=13; i<17; i++) gxdraw (xy[i*2],xy[i*2+1]);
      xy[26] = xy[8]; xy[27] = xy[9];
      gxfill (xy+8,10);
      xy[50] = xy[0]; xy[51] = xy[1];
      gxfill (xy+32,10);
    }
    return;
  }
  if (mtype==4 || mtype==5) {          /* Draw sqaures */
    xy[0] = x-siz2; xy[1] = y+siz2;
    xy[2] = x+siz2; xy[3] = y+siz2;
    xy[4] = x+siz2; xy[5] = y-siz2;
    xy[6] = x-siz2; xy[7] = y-siz2;
    xy[8] = xy[0]; xy[9] = xy[1];
    if (mtype==4) {
      gxmove (xy[0],xy[1]);
      for (i=1; i<5; i++) gxdraw (xy[i*2],xy[i*2+1]);
    } else {
      gxfill (xy,5);
    }
    return;
  }
  if (mtype==6) {                      /* ex marks the spot */
    gxmove (x-siz2*0.71,y-siz2*0.71);
    gxdraw (x+siz2*0.71,y+siz2*0.71);
    gxmove (x-siz2*0.71,y+siz2*0.71);
    gxdraw (x+siz2*0.71,y-siz2*0.71);
    return;
  }
  if (mtype==7 || mtype==12) {   /* Open or closed diamond */
    gxmove (x-siz2*0.75,y);
    gxdraw (x,y+siz2*1.1);
    gxdraw (x+siz2*0.75,y);
    gxdraw (x,y-siz2*1.1);
    gxdraw (x-siz2*0.75,y);
    if (mtype==12) {
      xy[0] = x-siz2*0.75; xy[1]=y;
      xy[2] = x; xy[3] = y+siz2*1.1;
      xy[4] = x+siz2*0.75; xy[5] = y;
      xy[6] = x; xy[7] = y-siz2*1.1;
      xy[8] = x-siz2*0.75; xy[9] = y;
      gxfill(xy,4);
    }
    return;
  }
  if (mtype==8 || mtype==9) {          /* Triangles */
    xy[0] = x; xy[1] = y+siz2;
    xy[2] = x+siz2*0.88; xy[3] = y-siz2*0.6;
    xy[4] = x-siz2*0.88; xy[5] = y-siz2*0.6;
    xy[6] = x; xy[7] = y+siz2;
    if (mtype==8) {
      gxmove (xy[0],xy[1]);
      for (i=1; i<4; i++) gxdraw (xy[i*2],xy[i*2+1]);
    } else {
      gxfill (xy,4);
    }
    return;
  }
}

/* Plot centered title.  Only supports angle of 0 and 90 */

void gxtitl (char *chrs, gadouble x, gadouble y, gadouble height,
             gadouble width, gadouble angle) {
gadouble xx,yy;
gaint len,i;

  i = 0;
  len = 0;
  while (*(chrs+i)) {
    if (*(chrs+i)!=' ') len=i+1;
    i++;
  }
  if (len==0) return;

  xx = x; yy = y;
  if (angle > 45.0) {
    yy = y - 0.5*width*(gadouble)len;
  } else {
    xx = x - 0.5*width*(gadouble)len;
  }
  gxchpl (chrs, len, xx, yy, height, width, angle);
}

/* Do polygon fill.  It is assumed the bulk of the work will be done
   in hardware.  We do perform clipping at this level, and
   actually do the work to clip at the clipping boundry.       */

void gxfill (gadouble *xy, gaint num) {
gadouble *r, *out, *buff, x, y, xybuff[40];
gaint i,flag,onum,aflag;

  if (num<3) return;
  /* Do clipping.    */

  aflag = 0;
  if (num<10) buff = xybuff;
  else {
    buff = (gadouble *)malloc(sizeof(gadouble)*num*4);
    if (buff==NULL) {
      printf("Memory allocation error in gxfill.  Can't fill contour\n");
      return;
    }
    aflag = 1;
  }

  r = xy;
  out = buff;
  onum = 0;
  flag = 0;
  if (*r<clminx || *r>clmaxx || *(r+1)<clminy || *(r+1)>clmaxy) flag=1;
  for (i=0; i<num; i++) {
    if (*r<clminx || *r>clmaxx || *(r+1)<clminy || *(r+1)>clmaxy) {
      if (!flag) {
        bdterp (*(r-2), *(r-1), *r, *(r+1), &x, &y);
        *out = x;
        *(out+1) = y;
        onum++;
        out+=2;
      }
      *out = *r;
      *(out+1) = *(r+1);
      if (*r<clminx) *out = clminx;
      if (*r>clmaxx) *out = clmaxx;
      if (*(r+1)<clminy) *(out+1) = clminy;
      if (*(r+1)>clmaxy) *(out+1) = clmaxy;
      onum++;
      out+=2;
      flag = 1;
    } else {
      if (flag) {
        bdterp (*(r-2), *(r-1), *r, *(r+1), &x, &y);
        *out = x;
        *(out+1) = y;
        onum++;
        out+=2;
      }
      *out = *r;
      *(out+1) = *(r+1);
      onum++;
      out+=2;
      flag = 0;
    }
    r+=2;
  }

  r = buff;
  for (i=0; i<onum; i++) {
    gxvcon (*r,*(r+1),r,r+1);
    r+=2;
  }

  /* Output to metabuffer */

  hout1(-7,onum);             /* start a polygon fill */
  r = buff;
  hout2(-10,*r,*(r+1));       /* move to first point in polygon */
  r+=2;
  for (i=1; i<onum; i++) {
    hout2(-11,*r,*(r+1));     /* draw to next point in polygon */
    r+=2;
  }
  hout0(-8);                  /* terminate polygon */

  /* Output to hardware */

  if (intflg) dsubs.gxdfil (buff, onum);
  if (aflag) free(buff);
}

/* Perform edge interpolation for clipping  */

void bdterp (gadouble x1, gadouble y1, gadouble x2, gadouble y2,
             gadouble *x, gadouble *y) {

  if (x1<clminx || x2<clminx || x1>clmaxx || x2>clmaxx) {
    *x = clminx;
    if (x1>clmaxx || x2>clmaxx) *x = clmaxx;
    *y = y1 - ((y1-y2)*(x1-*x)/(x1-x2));
    if (*y<clminy || *y>clmaxy) goto sideh;
    return;
  }

  sideh:

  if (y1<clminy || y2<clminy || y1>clmaxy || y2>clmaxy) {
    *y = clminy;
    if (y1>clmaxy || y2>clmaxy) *y = clmaxy;
    *x = x1 - ((x1-x2)*(y1-*y)/(y1-y2));
    return;
  }
}

void gxbutn (gaint bnum, struct gbtn *pbn) {
  hout1(-20,bnum);
  dsubs.gxdpbn(bnum, pbn, 0, 0, -1);
}

/* Set mask for a rectangular area */

void gxmaskrec (gadouble xlo, gadouble xhi, gadouble ylo, gadouble yhi) {
gaint siz,i,j,pos,ilo,ihi,jlo,jhi,jj;

  if (maskflg == -888) return;

  if (mask==NULL) {                     /* If not allocated yet, now's the time */
    siz = (gaint)(rxsize*rysize*10000.0); 
    mask = (char *)malloc(siz);
    if (mask==NULL) {
      printf ("Error allocating mask array memory\n");
      printf ("Execution continues with no mask\n");
      maskflg = -888;
      return;
    }
    masksize = siz;
    maskx = (gaint)(rxsize*100.0);
    gxmaskclear();
  } 
  maskflg = 1;
  
  /* do clipping for the mask */
  if (xlo<clminx && xhi<clminx) return;
  if (xlo>clmaxx && xhi>clmaxx) return;
  if (ylo<clminy && yhi<clminy) return;
  if (ylo>clmaxy && yhi>clmaxy) return;

  if (xlo<clminx) xlo=clminx;
  if (xhi>clmaxx) xhi=clmaxx;
  if (ylo<clminy) ylo=clminy;
  if (yhi>clmaxy) yhi=clmaxy;

  /* convert to virtual page coordinates */
  gxvcon(xlo,ylo,&xlo,&ylo);
  gxvcon(xhi,yhi,&xhi,&yhi);
  
  ilo = (gaint)(xlo*100.0);
  ihi = (gaint)(xhi*100.0);
  jlo = (gaint)(ylo*100.0);
  jhi = (gaint)(yhi*100.0);
  if (ilo<0) ilo = 0;
  if (ihi<0) ihi = 0;
  if (ilo>=maskx) ilo = maskx-1;
  if (ihi>=maskx) ihi = maskx-1;
  for (j=jlo; j<=jhi; j++) {
    jj = j*maskx;
    for (i=ilo; i<=ihi; i++) {
      pos = jj+i;
      if (pos>=0 && pos<masksize) *(mask+pos) = '1';
    }
  }
}

/* Given a rectangular area, check to see if it overlaps with any existing
   mask.  This is used to avoid overlaying contour labels. */

gaint gxmaskrq (gadouble xlo, gadouble xhi, gadouble ylo, gadouble yhi) {
gaint i,j,ilo,ihi,jlo,jhi,jj,pos;

  if (maskflg == -888) return(0);
  if (mask==NULL) return (0);
  if (maskflg==0) return (0);

  /* If query region is partially or completely outside of clip area, indicate an overlap */

  if (xlo<clminx || xhi>clmaxx || ylo<clminy || yhi>clmaxy) return(1);

  /* convert to virtual page coordinates */
  gxvcon(xlo,ylo,&xlo,&ylo);
  gxvcon(xhi,yhi,&xhi,&yhi);
  
  ilo = (gaint)(xlo*100.0);
  ihi = (gaint)(xhi*100.0);
  jlo = (gaint)(ylo*100.0);
  jhi = (gaint)(yhi*100.0);
  if (ilo<0) ilo = 0;
  if (ihi<0) ihi = 0;
  if (ilo>maskx) ilo = maskx;
  if (ihi>maskx) ihi = maskx;
  for (j=jlo; j<=jhi; j++) {
    jj = j*maskx;
    for (i=ilo; i<=ihi; i++) {
      pos = jj+i;
      if (pos>=0 && pos<masksize) {
        if (*(mask+pos) == '1') return(1);
      }
    }
  }
  return (0);
}

/* Set mask to unset state */

void gxmaskclear(void) {
gaint i;   
  if (maskflg > 0)  {
    for (i=0; i<masksize; i++) *(mask+i) = '0';
    maskflg = 0;
  }
}

#endif  /* matches #ifndef STNDALN */


/* Query env symbol */

char *gxgsym(char *ch) {
  return (getenv(ch));
}

/* Construct full file path name from env symbol or default */

char *gxgnam(char *ch) {
char *fname=NULL, *ddir;
gaint len,i,j;
size_t sz;

  /* calc partial length of output string */
  len = 0;
  i = 0;
  while (*(ch+i)) { i++; len++;}

  /* Query the env symbol */
  ddir = gxgsym("GADDIR");

  /* calc the total length of the output string */
  if (ddir==NULL) {
    i = 0;
    while (*(datad+i)) { i++; len++;}
  } else {
    i = 0;
    while (*(ddir+i)) { i++; len++;}
  }

  /* Allocate memory for the output */
  sz = len+15;
  fname = (char *)malloc(sz);
  if (fname==NULL) {
    printf ("Memory allocation error in data set open\n");
    return (NULL);
  }

  /* fill in the directory depending on the value of the env var */
  if (ddir==NULL) {
    i = 0;
    while (*(datad+i)) {
      *(fname+i) = *(datad+i);
      i++;
    }
  } else if (*ddir=='.') {
    i = 0;
  } else {
    i = 0;
    while (*(ddir+i)) {
      *(fname+i) = *(ddir+i);
      i++;
    }
  }

  /* Insure a slash between dir name and file name */
  if (i!=0 && *(fname+i-1)!='/') {
    *(fname+i) = '/';
    i++;
  }

  /* fill in the file name */
  j = 0;
  while (*(ch+j)) {
    *(fname+i) = *(ch+j);
    i++; j++;
  }
  *(fname+i) = '\0';

  return (fname);
}

