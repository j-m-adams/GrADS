/* Copyright (C) 1988-2020 by George Mason University. See file COPYRIGHT for more information. */

/* Routines related to hardcopy (metafile) output. */

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
#include <math.h>
#include "gatypes.h"
#include "gx.h"

void *galloc(size_t,char *);

/* Struct to form linked list for the meta buffer */
/* The buffer area is allocated as float, to insure at least four bytes
   per element.  In some cases, ints and chars will get stuffed into a 
   float (via pointer casting).  */

/* Don't use gafloat or gaint for meta buffer stuffing.  */

struct gxmbuf {
  struct gxmbuf *fpmbuf;         /* Forward pointer */
  float *buff;                   /* Buffer area */
  gaint len;                     /* Length of Buffer area */
  gaint used;                    /* Amount of buffer used */
};

/* Buffer chain anchor here; also a convenience pointer to the last buffer
   in the chain.  Times 2, for double buffering.  mbufanch and mbuflast 
   always point to the buffer currently being added to.  In double buffering
   mode, that will be the background buffer (and mbufanch2 points to the 
   buffer representing the currently display image). */

static struct gxmbuf *mbufanch=NULL;  /* Buffer Anchor */
static struct gxmbuf *mbuflast=NULL;  /* Last struct in chain */

static struct gxmbuf *mbufanch2=NULL;  /* Buffer Anchor */
static struct gxmbuf *mbuflast2=NULL;  /* Last struct in chain */

static gaint dbmode;                   /* double buffering flag */
static struct gxpsubs *psubs=NULL;     /* function pointers for printing */
static struct gxdsubs *dsubs=NULL;     /* function pointers for display */

#define BWORKSZ 250000

static gaint mbuferror = 0;    /* Indicate an error state; suspends buffering */

/* Initialize any buffering, etc. when GrADS starts up */

void gxhnew (gadouble xsiz, gadouble ysiz, gaint hbufsz) {
gaint rc;
  mbufanch = NULL;
  mbuflast = NULL;
  mbuferror = 0;
  if (sizeof(int) > sizeof(float)) {
    printf ("Error in gx initialization: Incompatable int and float sizes\n");
    mbuferror = 99;
    return;
  }
  rc = mbufget();
  if (rc) {
    printf ("Error in gx initialization: Unable to allocate meta buffer\n");
    mbuferror = 99;
  }
}


/* Add command with 0 args to metafile buffer */

void hout0 (gaint cmd) {
gaint rc;
signed char *ch;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <3) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ch = (signed char *)(mbuflast->buff+mbuflast->used);
  *ch = (signed char)99;
  *(ch+1) = (signed char)cmd;
  mbuflast->used++;
}

/* Add a command with one small integer argument to the metafile buffer.
   The argument is assumed to fit into a signed char (-127 to 128).  */

void hout1c (gaint cmd, gaint opt) {
gaint rc;
signed char *ch;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <4) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ch = (signed char *)(mbuflast->buff+mbuflast->used);
  *ch = (signed char)99;
  *(ch+1) = (signed char)cmd;
  mbuflast->used++;
  *(ch+2) = (signed char)opt;
  mbuflast->used++;
}

/* Add command with one integer argument to metafile buffer */

void hout1 (gaint cmd, gaint opt) {
gaint rc;
signed char *ch;
int *iii;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <4) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ch = (signed char *)(mbuflast->buff+mbuflast->used);
  *ch = (signed char)99;
  *(ch+1) = (signed char)cmd;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)opt;
  mbuflast->used++;
}

/* Metafile buffer, command plus two double args */

void hout2 (gaint cmd, gadouble x, gadouble y) {
gaint rc;
signed char *ch;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <5) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ch = (signed char *)(mbuflast->buff+mbuflast->used);
  *ch = (signed char)99;
  *(ch+1) = (signed char)cmd;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)x;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)y;
  mbuflast->used++;
}

/* Metafile buffer, command plus two integer args */

void hout2i (gaint cmd, gaint i1, gaint i2) {
gaint rc;
signed char *ch;
int *iii;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <5) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ch = (signed char *)(mbuflast->buff+mbuflast->used);
  *ch = (signed char)99;
  *(ch+1) = (signed char)cmd;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i1;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i2;
  mbuflast->used++;
}

/* Metafile buffer, command plus three integer args */

void hout3i (gaint cmd, gaint i1, gaint i2, gaint i3) {
gaint rc;
signed char *ch;
int *iii;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <6) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ch = (signed char *)(mbuflast->buff+mbuflast->used);
  *ch = (signed char)99;
  *(ch+1) = (signed char)cmd;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i1;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i2;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i3;
  mbuflast->used++;
}

/* Metafile buffer, command plus four integer args */

void hout5i (gaint cmd, gaint i1, gaint i2, gaint i3, gaint i4, gaint i5) {
gaint rc;
signed char *ch;
int *iii;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <8) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ch = (signed char *)(mbuflast->buff+mbuflast->used);
  *ch = (signed char)99;
  *(ch+1) = (signed char)cmd;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i1;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i2;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i3;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i4;
  mbuflast->used++;
  iii = (int *)(mbuflast->buff+mbuflast->used);
  *iii = (int)i5;
  mbuflast->used++;
}

/* Metafile buffer, command plus four double args */

void hout4 (gaint cmd, gadouble xl, gadouble xh, gadouble yl, gadouble yh) {
gaint rc;
signed char *ch;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <7) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ch = (signed char *)(mbuflast->buff+mbuflast->used);
  *ch = (signed char)99;
  *(ch+1) = (signed char)cmd;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)xl;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)xh;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)yl;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)yh;
  mbuflast->used++;
}

/* Metafile buffer, command plus three double args */

void hout3 (gaint cmd, gadouble x, gadouble y, gadouble r) {
gaint rc;
signed char *ch;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <7) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ch = (signed char *)(mbuflast->buff+mbuflast->used);
  *ch = (signed char)99;
  *(ch+1) = (signed char)cmd;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)x;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)y;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)r;
  mbuflast->used++;
}

/* Add a single character to the metafile buffer, along with the font number (less
   than 100), location (x,y), and size/rotation specs (4 floats).  Uses -21 as a
   cmd value.   */

void houtch (char ch, gaint fn, gadouble x, gadouble y,
         gadouble w, gadouble h, gadouble ang) {
gaint rc;
signed char *ccc;
char *ucc;
  if (mbuferror) return;
  if (mbuflast->len - mbuflast->used <8) {
    rc = mbufget();
    if (rc) {
      gxmbuferr();
      return;
    }
  }
  ccc = (signed char *)(mbuflast->buff+mbuflast->used);
  ucc = (char *)(ccc+2);
  *ccc = (signed char)99;
  *(ccc+1) = (signed char)(-21);
  *ucc = ch; 
  *(ccc+3) = (signed char)fn;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)x;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)y;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)w;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)h;
  mbuflast->used++;
  *(mbuflast->buff+mbuflast->used) = (float)ang;
  mbuflast->used++;
}

/* User has issued a clear.  
   This may also indicate the start or end of double buffering.  
   If we are not double buffering, just free up the memory buffer and return.  
   If we are starting up double buffering, we need another buffer chain.  
   If we are ending double buffering, all memory needs to be released.  
   If we are in the midst of double buffering, do a "swap" and free the foreground buffer. 

   Values for action are:
      0 -- new frame (clear display), wait before clearing.
      1 -- new frame, no wait.
      2 -- New frame in double buffer mode.  If not supported
           has same result as action=1.  Usage involves multiple
           calls with action=2 to obtain an animation effect.  
      7 -- new frame, but just clear graphics.  Do not clear  
           event queue; redraw buttons. 
      8 -- clear only the event queue.
      9 -- clear only the X request buffer
*/ 

void gxhfrm (gaint iact) {
struct gxmbuf *pmbuf, *pmbufl;

  /* Start up double buffering */
  if (iact==2 && dbmode==0) { 
    mbufrel(1); 
    if (mbufanch==NULL) mbufget();
    mbufanch2 = mbufanch;
    mbuflast2 = mbuflast;
    mbufanch = NULL;
    mbuflast = NULL;
    mbufget();
    dbmode = 1; 
  }

  /* End of double buffering */
  if (iact!=2 && dbmode==1) {
    mbufrel(0);
    mbufanch = mbufanch2;
    mbufrel(1);
    dbmode = 0;
    mbuferror = 0;
    return;
  }

  /* If double buffering, swap buffers */
  if (dbmode) {
    pmbuf = mbufanch;     /* Save pointer to background buffer */
    pmbufl = mbuflast;
    mbufanch = mbufanch2;
    mbufrel(1);           /* Get rid of former foreground buffer */
    mbufanch2 = pmbuf;    /* Set foreground to former background */
    mbuflast2 = pmbufl; 
  } 
  else {
    /* Not double buffering, so just free buffers */
    mbufrel(1);
  }
  if (!dbmode) mbuferror = 0;        /* Reset error state on clear command */
}


/* Redraw based on contents of current buffers.  Items that persist from plot
   to plot ARE NOT IN THE META BUFFER; these items are set in the hardware attribute
   database and are queried by the backend. 

   This routine is called from gxX (ie, a lower level of the backend rendering), 
   and this routine calls back into gxX.  This is not, however, implemented as
   true recursion -- events are disabled in gxX during this redraw, so addtional
   levels of recursion are not allowed.  

   If dbflg, draw from the background buffer.  Otherwise draw from the 
   foreground buffer. */

void gxhdrw (gaint dbflg, gaint pflg) {
struct gxmbuf *pmbuf;
float *buff;
int *iii;
gadouble r,s,x,y,w,h,ang;
gadouble *xybuf;
gaint ppp,cmd,op1,op2,op3,op4,op5,fflag,xyc=0,fn,sig;
signed char *ch;
char ccc,*uch;

  if (dbflg && !dbmode) {
    printf ("Logic error 0 in Redraw.  Contact Developer.\n");
    return;
  }

  if (psubs==NULL) psubs = getpsubs();  /* get ptrs to the graphics printing functions */
  if (dsubs==NULL) dsubs = getdsubs();  /* get ptrs to the graphics display functions */
 
  if (dbflg) pmbuf = mbufanch2;
  else pmbuf = mbufanch; 

  fflag = 0;
  xybuf = NULL;

  while (pmbuf) {
    ppp = 0;
    while (ppp < pmbuf->used) {

      /* Get message type */
 
      ch = (signed char *)(pmbuf->buff + ppp);
      cmd = (gaint)(*ch);
      if (cmd != 99) {
        printf ("Metafile buffer is corrupted\n");
        printf ("Unable to complete redraw and/or print operation\n");
        return;
      }
      cmd = (gaint)(*(ch+1));
      ppp++;


      /* Handle various message types */
      /* -9 is end of file.  Should not happen. */

      if (cmd==-9) {
        printf ("Logic Error 4 in Redraw.  Notify Developer\n");
        return;
      }

      /*  -1 indicates start of file.  Should not occur. */

      else if (cmd==-1) {
        printf ("Logic Error 8 in Redraw.  Notify Developer\n");
        return;
      }
  
      /* -2 indicates new frame.  Also should not occur */

      else if (cmd==-2) {
        printf ("Logic Error 12 in Redraw.  Notify Developer\n");
        return;
      }

      /* -3 indicates new color.  One arg; color number.  */
  
      else if (cmd==-3) {
        iii = (int *)(pmbuf->buff + ppp);
        op1 = (gaint)(*iii);
	if (pflg) 
	  psubs->gxpcol (op1);          /* for printing */
	else 
	  dsubs->gxdcol (op1);          /* for hardware */
        ppp++;
      }

      /* -4 indicates new line thickness.  It has two arguments */
 
      else if (cmd==-4) {
        iii = (int *)(pmbuf->buff + ppp);
        op1 = (gaint)(*iii);
	if (pflg)
	  psubs->gxpwid (op1);          /* for printing */
	else
	  dsubs->gxdwid (op1);          /* for hardware */
        ppp += 2;
      }

      /*  -5 defines a new color, in rgb.  It has five int args */

      else if (cmd==-5){
        iii = (int *)(pmbuf->buff + ppp);
        op1 = (gaint)(*iii);
        iii = (int *)(pmbuf->buff + ppp + 1);
        op2 = (gaint)(*iii);
        iii = (int *)(pmbuf->buff + ppp + 2);
        op3 = (gaint)(*iii);
        iii = (int *)(pmbuf->buff + ppp + 3);
        op4 = (gaint)(*iii);
        iii = (int *)(pmbuf->buff + ppp + 4);
        op5 = (gaint)(*iii);
        gxdbacol (op1,op2,op3,op4,op5);   /* update the data base */
	if (pflg) 
	  psubs->gxpacol (op1);                 /* for printing (no-op for cairo) */
	else 
	  dsubs->gxdacol (op1,op2,op3,op4,op5); /* for hardware (no-op for cairo) */
        ppp += 5;
      }

      /* -6 is for a filled rectangle.  It has four args. */ 
 
      else if (cmd==-6){
        buff = pmbuf->buff + ppp;
        r = (gadouble)(*buff);
        s = (gadouble)(*(buff+1));
        x = (gadouble)(*(buff+2));
        y = (gadouble)(*(buff+3));
	if (pflg) 
	  psubs->gxprec(r,s,x,y);          /* for printing */
	else
	  dsubs->gxdrec(r,s,x,y);          /* for hardware */
        ppp += 4;
      }

      /* -7 indicates the start of a polygon fill.  It has one arg, 
         the length of the polygon.  We allocate an array for the entire
         polygon, so we can present it to the hardware backend in 
         on piece. */

      else if (cmd==-7) {
        iii = (int *)(pmbuf->buff + ppp);
        op1 = (gaint)(*iii);
        xybuf = (gadouble *)galloc(sizeof(gadouble)*op1*2,"gxybuf");
        if (xybuf==NULL) {
          printf ("Memory allocation error: Redraw\n");
          return;
        }
        xyc = 0;
        fflag = 1;
        ppp += 1;
	/* tell printing layer about new polygon. */
	if (pflg) psubs->gxpbpoly();  
      }

      /* -8 is to terminate polygon fill.  It has no args */

      else if (cmd==-8) {
        if (xybuf==NULL) {
          printf ("Logic Error 16 in Redraw.  Notify Developer\n");
          return;
        }
	if (pflg) 
	  psubs->gxpepoly (xybuf,xyc);  /* for printing */
	else
	  dsubs->gxdfil (xybuf,xyc);    /* for hardware */
        gree (xybuf,"gxybuf");
        xybuf = NULL;
        fflag = 0;
      }

      /* -10 is a move to instruction.  It has two double args */ 

      else if (cmd==-10) {
        buff = pmbuf->buff + ppp;
        x = (gadouble)(*buff);
        y = (gadouble)(*(buff+1));
	if (fflag) {
	  xybuf[xyc*2] = x;
	  xybuf[xyc*2+1] = y;
	  xyc++;
	}
	if (pflg) 
	  psubs->gxpmov(x,y);            /* for printing */
	else         
	  dsubs->gxdmov(x,y);            /* for hardware */
        ppp += 2;
      }

      /*  -11 is draw to.  It has two double args. */  
        
      else if (cmd==-11) {
        buff = pmbuf->buff + ppp;
        x = (gadouble)(*buff);
        y = (gadouble)(*(buff+1));
	if (fflag) {
	  xybuf[xyc*2] = x;
	  xybuf[xyc*2+1] = y;
	  xyc++;
	}
	if (pflg) 
	  psubs->gxpdrw(x,y);            /* for printing */
	else 
	  dsubs->gxddrw(x,y);            /* for hardware */
        ppp += 2;
      }
      
      /* -12 indicates new fill pattern.  It has three arguments. */
 
      else if (cmd==-12) {
	/* This is a no-op for cairo; X-based pattern drawing */
        buff = pmbuf->buff + ppp;
	dsubs->gxdptn ((gaint)*(buff+0),(gaint)*(buff+1),(gaint)*(buff+2));
	if (pflg) 
	  psubs->gxpflush(); 
        ppp += 3;
      }

      /* -20 is a draw widget.  We will redraw it in current state. */

      else if (cmd==-20) {
	/* This is a no-op for cairo; X-based buttonwidget drawing */
        buff = pmbuf->buff + ppp;
	dsubs->gxdpbn ((gaint)*(buff+0),NULL,1,0,-1);
	if (pflg) 
	  psubs->gxpflush(); 
        ppp += 1;
      }

      /* -21 is for drawing a single character in the indicated font and size */

      else if (cmd==-21) {
        ch = (signed char *)(pmbuf->buff + ppp - 1);
        fn = (gaint)(*(ch+3));
        uch = (char *)(pmbuf->buff + ppp - 1);
        ccc = *(uch+2);
        buff = pmbuf->buff + ppp;
        x = (gadouble)(*buff);
        y = (gadouble)(*(buff+1));
        w = (gadouble)(*(buff+2));
        h = (gadouble)(*(buff+3));
        ang = (gadouble)(*(buff+4));
	if (pflg) 
	  r = psubs->gxpch (ccc,fn,x,y,w,h,ang);     /* print a character */
	else 
	  r = dsubs->gxdch (ccc,fn,x,y,w,h,ang);     /* draw a character */
        ppp += 5;
      }

      /* -22 is for a signal. It has one signed character argument */

      else if (cmd==-22) {
	ch = (signed char *)(pmbuf->buff + ppp - 1);
	sig = (gaint)(*(ch+2));
	if (pflg) 
	  psubs->gxpsignal(sig); 
	else 
	  dsubs->gxdsignal(sig);
	ppp++;
      }

      /* -23 is for the clipping area. It has four args. */ 
 
      else if (cmd==-23){
        buff = pmbuf->buff + ppp;
        r = (gadouble)(*buff);
        s = (gadouble)(*(buff+1));
        x = (gadouble)(*(buff+2));
        y = (gadouble)(*(buff+3));
	if (pflg) 
	  psubs->gxpclip(r,s,x,y);          /* for printing */
	else
	  dsubs->gxdclip(r,s,x,y);          /* for hardware */
        ppp += 4;
      }

      /* -24 is for a filled circle. It has three args. */ 
 
      else if (cmd==-24){
        buff = pmbuf->buff + ppp;
        x = (gadouble)(*buff);
        y = (gadouble)(*(buff+1));
        r = (gadouble)(*(buff+2));
	if (pflg) 
	  psubs->gxpcirc(x,y,r,1);          /* for printing */
	else
	  dsubs->gxdcirc(x,y,r,1);          /* for hardware */
        ppp += 3;
      }
      
      /* -25 is for an open circle. It has three args. */ 
 
      else if (cmd==-25){
        buff = pmbuf->buff + ppp;
        x = (gadouble)(*buff);
        y = (gadouble)(*(buff+1));
        r = (gadouble)(*(buff+2));
	if (pflg) 
	  psubs->gxpcirc(x,y,r,0);          /* for printing */
	else
	  dsubs->gxdcirc(x,y,r,0);          /* for hardware */
        ppp += 3;
      }

      /* Any other command would be invalid */

      else {
         printf ("Logic Error 20 in Redraw.  Notify Developer\n");
        return;
      }
    } 
    if (pmbuf == mbuflast) break;
    pmbuf = pmbuf->fpmbuf;
  }
  /* tell hardware and printing layer we are finished */
  if (pflg) psubs->gxpflush();  
  dsubs->gxdopt(4);
}


/* Allocate and chain another buffer area */

gaint mbufget (void) {
struct gxmbuf *pmbuf;

  if (mbufanch==NULL) {
    pmbuf = (struct gxmbuf *)galloc(sizeof(struct gxmbuf),"mbufanch");  
    if (pmbuf==NULL) return (1);
    mbufanch = pmbuf;                  /* set the new buffer structure as the anchor */
    mbuflast = pmbuf;                  /* ... and also as the last one */
    pmbuf->buff = (float *)galloc(sizeof(float)*BWORKSZ,"anchbuff");  /* allocate a buffer */
    if (pmbuf->buff==NULL) return(1);
    pmbuf->len = BWORKSZ;              /* set the buffer length */
    pmbuf->used = 0;                   /* initialize the buffer as unused */
    pmbuf->fpmbuf = NULL;              /* terminate the chain */
  }
  else {
    if (mbuflast->fpmbuf==NULL) {      /* no more buffers in the chain */
      pmbuf = (struct gxmbuf *)galloc(sizeof(struct gxmbuf),"mbufnew");  
      if (pmbuf==NULL) return (1);
      mbuflast->fpmbuf = pmbuf;        /* add the new buffer structure to the chain */
      mbuflast = pmbuf;                /* reset mbuflast to the newest buffer structure in the chain */
      pmbuf->buff = (float *)galloc(sizeof(float)*BWORKSZ,"newbuff");  /* allocate a buffer */
      if (pmbuf->buff==NULL) return(1);
      pmbuf->len = BWORKSZ;            /* set the buffer length */
      pmbuf->used = 0;                 /* initialize the buffer as unused */
      pmbuf->fpmbuf = NULL;            /* terminate the chain */
    }
    else {                             /* we'll just re-use what's already been chained up */
      pmbuf = mbuflast->fpmbuf;        /* get the next buffer in the chain */
      pmbuf->used = 0;                 /* reset this buffer to unused */
      mbuflast = pmbuf;                /* set mbuflast to point to this buffer */
    }
  }
  return (0);
}

/* Free buffer chain.  
   If flag is 1, leave allocated buffers alone and mark the anchor as unused 
   If flag is 0, free all buffers, including the anchor
*/

void mbufrel (gaint flag) {
struct gxmbuf *pmbuf,*pmbuf2;
gaint i;

  i = flag;
  pmbuf = mbufanch;                /* point at the anchor */
  while (pmbuf) {
    pmbuf2 = pmbuf->fpmbuf;        /* get next link in chain */
    if (!i) {                      /* this part only gets executed when flag is 0 */
      if (pmbuf->buff) gree (pmbuf->buff,"gxmbuf");
      gree (pmbuf,"mbufbuff");     /* free the pmbuf link */
      i = 0; 
    }
    pmbuf = pmbuf2;                /* move up the chain */
  }
  if (!flag) {
    mbufanch = NULL;               /* no more metabuffer */
  } 
  else {
    if (mbufanch) mbufanch->used = 0;
  }
  mbuflast = mbufanch;
}

void gxmbuferr() {
  printf ("Error in gxmeta: Unable to allocate meta buffer\n");
  printf ("                 Buffering for the current plot is disabled\n");
  mbuferror = 1;
  mbufrel(0);
}

