/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* qqq resolve gxdraw vs gxsdrw issue for cterp off */
/* qqq clip labels and masking outside of parea */
/* qqq error(mem) handling */

/* Authored by B. Doty */
/* Add labeling with masking 10/2008  B. Doty */
/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "gatypes.h"
#include "gx.h"

/* function prototype */
void *galloc(size_t,char *);
void gree();
char *intprs (char *, gaint *);
char *getdbl (char *, gadouble *);
/* gaint gxclvert (FILE *); */
gaint dequal(gadouble, gadouble, gadouble);

/* For buffering contour lines, when label masking is in use */

struct gxclbuf {
  struct gxclbuf *fpclbuf;       /* Forward pointer */
  gaint len;                     /* Number of contour points */
  gaint color,style,width,sfit;  /* Output options for this line */
  gadouble *lxy;                 /* Line points, x,y number len */
  gadouble val;                  /* contour level value */
};

#if USESHP==1
#include "shapefil.h"
/* Structure that contains dBase field metadata */
struct dbfld {
  struct dbfld *next;           /* Address of next data base field */
  DBFFieldType type;            /* string, integer, double, or logical */
  char name[12];                /* library interface limits length to 11 charaters */
  gaint len;                    /* for type string: width of string
				   for type int and double: total number of digits */
  gaint prec; 			/* for type double: used with len for format string %len.prec */
  gaint index;                  /* index value (for identifying this field in a list of fields) */
  gaint flag;                   /* 0==static fiels (same for all shapes), 1==dynamic (varies w/ shape) */
  void *value;                  /* field value */
};
gaint gxshplin (SHPHandle, DBFHandle, struct dbfld *);
#endif

static struct gxclbuf *clbufanch=NULL;  /* Anchor for contour line buffering for masking */
static struct gxclbuf *clbuflast=NULL;  /* Last clbuf struct in chain */
static char pout[512];                  /* Build strings for KML here */

/* Contour labels get buffered here when not using label masking  */

#define LABMAX 200
static gadouble gxlabx[LABMAX];
static gadouble gxlaby[LABMAX];
static gadouble gxlabs[LABMAX];
static char gxlabv[LABMAX] [24];
static gaint gxlabc[LABMAX];
static gaint gxlabn=0;
static gadouble ldmin=2.5;    /* Minimum distance between labels */

/* Common values for the contouring routines.                        */

static short *lwk=NULL;                   /* Pntr to flag work area     */
static gaint lwksiz=0;                    /* Size of flag work area     */
static gadouble *fwk=NULL;                /* Pntr to X,Y coord buffer   */
static gaint fwksiz=0;                    /* Size of coord buffer       */
static gaint fwkmid;                      /* fwk midpoint               */
static gadouble *xystrt, *xyend;          /* Pntrs into the fwk buffer  */
static gaint iss,iww,jww;                 /* Grid row lengths           */
static gadouble vv;                       /* Value being contoured      */
static gadouble *rr;                      /* Start of grid              */

static char clabel [24];                  /* Label for current contour  */

void gxclmn (gadouble dis) {
  if (dis>0.1) ldmin = dis;
} 

/* GXCLEV draws contours at a specified contour level and locates
   labels for that contour level.  The labels are buffered into
   the label buffer for later output.

   Contours are drawn through the grid pointed at by r, row length is,
   number of rows js, row start and end ib and ie, column start
   and end jb and je.  The contour is drawn at value V, and undefined
   values u are ignored.  Note that min value for ib or jb is 1.     */

/* Label types:  0 - none, 1 - at slope=0, 2 - forced  */

void gxclev (gadouble *r, gaint is, gaint js, gaint ib, gaint ie, gaint jb,
   gaint je, gadouble v, char *u, struct gxcntr *pcntr) {

gadouble *p1,*p2,*p4;
gaint i,j,iw,jw,rc;
short *l1,*l2;
char *p1u,*p2u,*p4u;
size_t sz;


/* Figure out in advance what grid sides the contour passes
   through for this grid box.  We will actually draw the contours
   later.  Doing the tests in advance saves some calculations.
   Note that a border of flags is left unset all around the grid
   boundry.  This avoids having to test for the grid boundries
   when we are following a contour.                                  */

strcpy (clabel,pcntr->label);          /* Label for this contour     */

iw=is+2; jw=js+2;                      /* Dimensions of flag grid    */
iww=iw; jww=jw; iss=is;                /* Set shared values          */
vv=v; rr=r;                            /* Set more shared values     */

/* Obtain storage for flag work area and coord buffer -- if we don't
   already have enough storage allocated for these.                  */

if (lwksiz<iw*jw*2) {                  /* If storage inadaquate then */
  if (lwk!=NULL) {                     /* If storage is allocated... */
    gree (lwk,"c1");                        /* Release it.                */
    gree (fwk,"c2");
    lwk = NULL; fwk = NULL;
  }
  lwksiz = iw*jw*2;                    /* Size of lwk flag area      */
  sz = lwksiz*sizeof(short);
  lwk = (short *)galloc(sz,"lwk");  /* Allocate flag work area */
  if (lwk==NULL) {
    printf ("Error in GXCLEV: Unable to allocate storage \n");
    return;
  }
  fwksiz = (iw+1)*(jw+1);              /* Size of coord pair buffer */
  if (fwksiz<500) fwksiz=500;          /* Insure big enough for small grids */
  sz = fwksiz*sizeof(gadouble);
  fwk = (gadouble *)galloc(sz,"fwk");    /* Allocate fwk   */
  if (fwk==NULL) {
    printf ("Error in GXCLEV: Unable to allocate storage \n");
    gree (lwk,"c3");
    lwk = NULL;
    return;
  }
  fwkmid = fwksiz/2;                   /* fwk midpoint              */
}

/* Set up the flags */

for (l1=lwk; l1<lwk+iw*jw*2; l1++) *l1=0;  /* Clear flags            */

for (j=jb;j<je;j++){                   /* Loop through the rows      */
  p1=r+is*(j-1)+ib-1;                  /* Set grid pointers to corner ...  */
  p2=p1+1; p4=p1+is;                   /*   values at start of row         */
  p1u=u+is*(j-1)+ib-1;                 /* Set undef pointers to corner ... */
  p2u=p1u+1; p4u=p1u+is;               /*   values at start of row         */
  l1=lwk+iw*j+ib; l2=l1+iw*jw;         /* Pointers to flags          */
  for (i=ib;i<ie;i++){                 /* Loop through a row         */
                                       /* Cntr pass through bottom?  */
    if (((*p1<=v&&*p2>v)||(*p1>v&&*p2<=v))&&*p1u!=0&&*p2u!=0) *l1=1;
                                       /* Cntr pass through left?    */
    if (((*p1<=v&&*p4>v)||(*p1>v&&*p4<=v))&&*p1u!=0&&*p4u!=0) *l2=1;
    p1++;p2++;p4++;l1++;l2++;          /* Bump pntrs through the row */
    p1u++;p2u++;p4u++;                 /* Bump undef pntrs through the row */
  }
  if (((*p1<=v&&*p4>v)||(*p1>v&&*p4<=v))&&*p1u!=0&&*p4u!=0) *l2=1;
}

p1=r+is*(je-1)+ib-1;                   /* Set grid pntrs to corner values  */
p2=p1+1; p4=p1+is;                     /*   at start of last row           */
p1u=u+is*(je-1)+ib-1;                   /* Set undef pntrs to corner values */
p2u=p1u+1; p4u=p1u+is;                     /*   at start of last row           */
l1=lwk+iw*je+ib;                       /* Flag pointers              */
for (i=ib;i<ie;i++) {                  /* Loop through the last row  */
                                       /* Check top of grid          */
  if (((*p1<=v&&*p2>v)||(*p1>v&&*p2<=v))&&*p1u!=0&&*p2u!=0) *l1=1;
  p1++;p2++;p4++;l1++;                 /* Incr pntrs through the row */
  p1u++;p2u++;p4u++;                   /* Incr undef pntrs through the row */
}

/* Look for a grid side that has a contour passing through it that
   has not yet been drawn.  Follow it in both directions.
   The X,Y coord pairs will be put in the floating point buffer,
   starting from the middle of the buffer.                           */

for (j=jb;j<je;j++){                   /* Loop through the rows      */
  l1=lwk+iw*j+ib; l2=l1+iw*jw;         /* Pointers to flags          */
  for (i=ib;i<ie;i++){                 /* Loop through a row         */
    if (*l1) {                         /* Do we got one?             */
      gxcflw (i,j,1,2);                /* Follow it                  */
      gxcflw (i,j-1,3,-2);             /* Follow it the other way    */
      rc = gxcspl(0,pcntr);            /* Output it                  */
      if (rc) goto merr;
    }
    if (*l2) {                         /* Do we got one?             */
      gxcflw (i,j,4,2);                /* Follow it                  */
      gxcflw (i-1,j,2,-2);             /* Follow it the other way    */
      rc = gxcspl(0,pcntr);            /* Output it                  */
      if (rc) goto merr;
    }
    l1++; l2++;
  }
}

/* Short (two point) lines in  upper right corner may hae been missed */

l1 = lwk+iw*je+ie-1;                   /* Upper right corner flags   */
l2 = lwk+iw*jw+iw*(je-1)+ie;
if (*l1 && *l2) {
  gxcflw (ie,je-1,4,2);                /* Follow it one way */
  xystrt = fwk + fwkmid;               /* Terminate other direction */
  rc = gxcspl(0,pcntr);                /* Output it */
  if (rc) goto merr;
}

return;

merr:
  printf ("Error in line contouring: Memory allocation for Label Buffering\n");
  return;
}

void gxcflw (gaint i, gaint j, gaint iside, gaint dr) {

/* Follow a contour to the end.  i and j are the grid location
   of the start of the contour.  is is the side on which we are
   entering the grid box.  dr is the direction in which we should
   fill the output buffer.  Other needed values are external.        */

/* The grid box:

     p4      side 3     p3
        x ------------ x
        |              |
 side 4 |              | side 2
        |              |
        |              |
        x ------------ x
      p1    side 1      p2
                                                                     */

short *l1,*l2,*l3,*l4;                 /* Flags for each box side    */
gadouble *p1,*p2,*p3,*p4;              /* Grid points for a box      */
gaint isave,jsave;                     /* Save initial grid postn    */
gadouble *xy,*xyllim, *xyulim;         /* Buffer position and limits */ 


isave=i; jsave=j;
p1=rr+iss*(j-1)+i-1;                   /* Set pntrs to corner values */
p2=p1+1; p4=p1+iss; p3=p4+1;
l1=lwk+iww*j+i; l4=l1+iww*jww;         /* Pointers to flags          */
l2=l4+1; l3=l1+iww;
xy = fwk + fwkmid;                     /* Start in middle of buffer  */
xyllim = fwk+6;                        /* Buffer limits              */
xyulim = fwk+(fwksiz-6);

if (iside==1) goto side1;              /* Jump in based on side      */
if (iside==2) goto side2;
if (iside==3) goto side3;
goto side4;

/* Calculate exit point in the current grid box, then move to the
   next grid box based on the exit side.                             */

side1:                                 /* Exit side 1; Enter side 3  */
*xy = i + (vv-*p1)/(*p2-*p1);          /* Calculate exit point       */
*(xy+1) = j;
*l1 = 0;                               /* Indicate we were here      */
xy+=dr;                                /* Move buffer pntr           */
if ((xy<xyllim)||(xy>xyulim)) goto done;   /* Don't exceed buffer    */
l3=l1; l1-=iww; l4-=iww; l2=l4+1;      /* Move pntrs to lower box    */
p4=p1; p3=p2; p1-=iss; p2-=iss;
j--;
if (*l1 && *l2 && *l4) {               /* Handle col point           */
  if (pathln(*p1, *p2, *p3, *p4)) goto side4;
  else goto side2;
}
if (*l4) goto side4;                   /* Find exit point            */
if (*l1) goto side1;
if (*l2) goto side2;
goto done;                             /* If no exit point, then done*/

side2:                                 /* Exit side 2; Enter side 4  */
*xy = i + 1.0;                         /* Calculate exit point       */
*(xy+1) = j + (vv-*p2)/(*p3-*p2);
*l2 = 0;                               /* Indicate we were here      */
xy+=dr;                                /* Move buffer pntr           */
if ((xy<xyllim)||(xy>xyulim)) goto done;   /* Don't exceed buffer    */
l4=l2; l1++; l2++; l3++;               /* Move pntrs to right box    */
p1=p2; p4=p3; p2++; p3++;
i++;
if (*l1 && *l2 && *l3) {               /* Handle col point           */
  if (pathln(*p1, *p2, *p3, *p4)) goto side3;
  else goto side1;
}
if (*l1) goto side1;                   /* Find exit point            */
if (*l2) goto side2;
if (*l3) goto side3;
goto done;                             /* If no exit point, then done*/

side3:                                 /* Exit side 3; Enter side 1  */
*xy = i + (vv-*p4)/(*p3-*p4);          /* Calculate exit point       */
*(xy+1) = j + 1.0;
*l3 = 0;                               /* Indicate we were here      */
xy+=dr;                                /* Move buffer pntr           */
if ((xy<xyllim)||(xy>xyulim)) goto done;   /* Don't exceed buffer    */
l1=l3; l4+=iww; l3+=iww; l2=l4+1;      /* Move pntrs to upper box    */
p1=p4; p2=p3; p3+=iss; p4+=iss;
j++;
if (*l2 && *l3 && *l4) {               /* Handle col point           */
  if (pathln(*p1, *p2, *p3, *p4)) goto side2;
  else goto side4;
}
if (*l2) goto side2;                   /* Find exit point            */
if (*l3) goto side3;
if (*l4) goto side4;
goto done;                             /* If no exit point, then done*/

side4:                                 /* Exit side 4; Enter side 2  */
*xy = i;                               /* Calculate exit point       */
*(xy+1) = j + (vv-*p1)/(*p4-*p1);
*l4 = 0;                               /* Indicate we were here      */
xy+=dr;                                /* Move buffer pntr           */
if ((xy<xyllim)||(xy>xyulim)) goto done;   /* Don't exceed buffer    */
l2=l4; l1--; l3--; l4--;               /* Move pntrs to upper box    */
p2=p1; p3=p4; p1--; p4--;
i--;
if (*l3 && *l4 && *l1) {               /* Handle col point           */
  if (pathln(*p1, *p2, *p3, *p4)) goto side1;
  else goto side3;
}
if (*l3) goto side3;                   /* Find exit point            */
if (*l4) goto side4;
if (*l1) goto side1;

done:

if ((i==isave)&&(j==jsave)) {          /* Closed contour?            */
  *xy = *(fwk+fwkmid);                 /* Close it off               */
  *(xy+1) = *(fwk+fwkmid+1);
  xy+=dr;
}

if (dr<0) 
  xystrt=xy+2; 
else 
  xyend=xy-2;  /* Set final buffer pntrs   */
return;
}

/* Calculate shortest combined path length through a col point.
   Return true if shortest path is side 1/2,3/4, else false.         */

gaint pathln (gadouble p1, gadouble p2, gadouble p3, gadouble p4) {
gadouble v1,v2,v3,v4,d1,d2;

  v1 = (vv-p1)/(p2-p1);
  v2 = (vv-p2)/(p3-p2);
  v3 = (vv-p4)/(p3-p4);
  v4 = (vv-p1)/(p4-p1);
  d1 = hypot(1.0-v1, v2) + hypot(1.0-v4, v3);
  d2 = hypot(v1, v4) + hypot(1.0-v2, 1.0-v3);
  if (d2<d1) return (0);
  return (1);
}

gaint gxcspl (gaint frombuf, struct gxcntr *pcntr) {

/* This subroutine does the curve fitting for the GX contouring
   package.  The X,Y point pairs have been placed in a floating
   point buffer.  *xystrt points to the start of the points in
   the buffer (namely, the first X,Y pair in the buffer).  *xyend
   points to end of the buffer -- the last X in the line (ie, the
   start of the last X,Y pair).

   To handle the end point conditions more easily, there must be
   enough room left in the buffer to hold one X,Y pair before
   the start of the line, and one X,Y pair after the end.

   The points are fitted with a psuedo cubic spline curve.
   (The slopes are arbitrarily chosen).  An intermediate point is
   output every del grid units.  */


struct gxclbuf *pclbuf; 
gadouble *x0,*x1,*x2,*x3,*y0,*y1,*y2,*y3;
gadouble sx1,sx2,sy1,sy2,d0,d1,d2,xt1,xt2,yt1,yt2;
gadouble t,t2,t3,tint,x,y,ax,bx,ay,by;
gadouble del,kurv,cmax,dacum,dcmin,c1,c2;
gadouble mslope,tslope,xlb=0.0,ylb=0.0;
gaint i,icls,nump=0,labflg;
gadouble *xy;
size_t sz;

if (pcntr->ltype==2) dacum=2.0;
else dacum=1.0;             /* Accumulated length between labels */
dcmin=ldmin;         /* Minimum distance between labels                */
labflg=0;          /* Contour not labelled yet   */
mslope=1000.0;     /* Minimum slope of contour line */

del=0.05;                              /* Iteration distance         */
if (frombuf) del=0.02;
kurv=0.5;                              /* Curviness (0 to 1)         */
cmax=0.7;                              /* Limit curviness            */

icls=0;                                /* Is it a closed contour?    */
if ( (*xystrt==*xyend) && (*(xystrt+1)==*(xyend+1)) ) icls=1;

/* Convert contour coordinates to plotting inches.  We will do
   our spline fit and assign labels in this lower level coordinate
   space to insure readability.    */

if (!frombuf) {
  nump=xyend-xystrt;
  nump=(nump+2)/2;
  gxcord (xystrt, nump, 3);
}



/* If using label masking, buffer the lines, and output the labels. */

if (pcntr->mask && !frombuf) {

  /* Allocate and chain a clbuf */

  sz = sizeof(struct gxclbuf);
  pclbuf = (struct gxclbuf *) galloc(sz,"pclbuf"); 
  if (pclbuf==NULL) return (1);
  if (clbufanch==NULL) clbufanch = pclbuf;
  else clbuflast->fpclbuf = pclbuf;
  clbuflast = pclbuf;
  pclbuf->fpclbuf = NULL;

  /* Allocate space for the line points */

  pclbuf->len = nump; 
  sz = sizeof(gadouble)*nump*2;
  pclbuf->lxy = (gadouble *) galloc(sz,"pclbufxy");
  if (pclbuf->lxy==NULL) return(1);

  /* Copy the line points and line info */

  for (i=0;i<nump*2;i++) *(pclbuf->lxy+i) = *(xystrt+i); 
  pclbuf->color = gxqclr();
  pclbuf->style = gxqstl();
  pclbuf->width = gxqwid();
  pclbuf->sfit = pcntr->spline;
  pclbuf->val = pcntr->val;

  /* Plot labels and set mask */

  if (pcntr->ltype && clabel[0]) {
    for (xy=xystrt+2; xy<xyend-1; xy+=2) {
      dacum += hypot(*xy-*(xy-2),*(xy+1)-*(xy-1));
      /* c1 = (thisY - prevY) * (nextY - thisY) */
      if (dequal(*(xy+1),*(xy-1),1e-12)==0 || dequal(*(xy+3),*(xy+1),1e-12)==0) 
	c1 = 0.0; 
      else 
	c1 = (*(xy+1)-*(xy-1))*(*(xy+3)-*(xy+1));
      /* c2 = abs(thisY - prevY) + abs(nextY - thisY) */
      if ( dequal(*(xy+1),*(xy-1),1e-12)==0 ) {
	if ( dequal(*(xy+3),*(xy+1),1e-12)==0 ) {
	  /* thisY = prevY = nextY. Check if true for X coords too */
	  if (dequal(*(xy),*(xy-2),1e-12)==0 && dequal(*(xy+2),*(xy),1e-12)==0 ) {
	    /* Duplicate point. Set c2 artificially high so label is not drawn */
	    c2 = 99;  
	  }
	  else
	    c2 = 0.0;
	}
	else 
	  c2 = fabs(*(xy+3)-*(xy+1));
      }
      else {
	if ( dequal(*(xy+3),*(xy+1),1e-12)==0 )
	  c2 = fabs(*(xy+1)-*(xy-1));
	else 
	  c2 = fabs(*(xy+1)-*(xy-1)) + fabs(*(xy+3)-*(xy+1));
      }
      /* Plot the label... 
	 if slope is zero or has changed sign, 
	 if contour doesn't bend too much,
	 and if not too close to another label */
      if ( c1<=0.0 && c2<0.02 && dacum>dcmin ){
        if (!gxqclab (*xy,*(xy+1),pcntr->labsiz)) {
          if (pcntr->shpflg==0) gxpclab (*xy,*(xy+1),0.0,gxqclr(),pcntr);
          dacum=0.0;
        }
      }
    }
    dacum += hypot(*xyend-*(xyend-2),*(xyend+1)-*(xyend-1));
    /* for closed contours, check the joining point */
    if (icls) {
      /* c1 = (endY - secondY) * (prevY - endY) */
      if (dequal(*(xyend+1),*(xystrt+3),1e-12)==0 || 
	  dequal(*(xyend-1),*(xyend+1),1e-12)==0 ) 
	c1=0.0;
      else 
	c1 = (*(xyend+1)-*(xystrt+3))*(*(xyend-1)-*(xyend+1));
      /* c2 = abs(endY - prevY) + abs(secondY - endY) */
      if (dequal(*(xyend+1),*(xyend-1),1e-12)==0) {
	if (dequal(*(xystrt+3),*(xyend+1),1e-12)==0) {
	  /* thisY = prevY = nextY  
	     Duplicate point. Set c2 artificially high so label is not drawn */
	  c2 = 99;  
	}
	else 
	  c2 = fabs(*(xystrt+3)-*(xyend+1));
      }
      else {
	if (dequal(*(xystrt+3),*(xyend+1),1e-12)==0)
	  c2 = fabs(*(xyend+1)-*(xyend-1));
	else 
	  c2 = fabs(*(xyend+1)-*(xyend-1)) + fabs(*(xystrt+3)-*(xyend+1));
      }
      /* same criteria apply as for non-closed contours */
      if (c1<=0.0 && c2<0.02 && dacum>dcmin){
        if (!gxqclab (*xy,*(xy+1),pcntr->labsiz)) {
          if (pcntr->shpflg==0) gxpclab (*xyend,*(xyend+1),0.0,gxqclr(),pcntr);
        }
      }
    }
  }

  return(0);
}

/* If specified, do not do the cubic spline fit, just output the
   contour sides, determine label locations, and return.        */

if (pcntr->spline==0) {
  if (pcntr->mask==1) gxplot (*xystrt, *(xystrt+1), 3);
  else gxplot (*xystrt, *(xystrt+1), 3);
  for (xy=xystrt+2; xy<xyend-1; xy+=2) {
    gxplot (*xy, *(xy+1), 2);
    if (!frombuf) {
      dacum += hypot(*xy-*(xy-2),*(xy+1)-*(xy-1));
      if ((*(xy+1)-*(xy-1))*(*(xy+3)-*(xy+1))<0.0 &&
                                  gxlabn<LABMAX && dacum>dcmin ){
        if (clabel[0]) {
          gxlabx[gxlabn]=(*xy);
          gxlaby[gxlabn]=(*(xy+1));
          gxlabs[gxlabn]=0.0;
          strcpy (gxlabv[gxlabn],clabel);
          gxlabc[gxlabn] = gxqclr();
          gxlabn++;
        }
        dacum=0.0;
      }
    }
  }
  gxplot (*xyend, *(xyend+1), 2);
  if (!frombuf) {
    dacum += hypot(*xyend-*(xyend-2),*(xyend+1)-*(xyend-1));
    if (icls) {
      if ((*(xyend+1)-*(xystrt+3))*(*(xyend-1)-*(xyend+1))<0.0 &&
                                    gxlabn<LABMAX && dacum>dcmin ){
        if (clabel[0]) {
          gxlabx[gxlabn]=(*xyend);
          gxlaby[gxlabn]=(*(xyend+1));
          gxlabs[gxlabn]=0.0;
          strcpy (gxlabv[gxlabn],clabel);
          gxlabc[gxlabn] = gxqclr();
          gxlabn++;
        }
      }
    }
  }
  return (0);
}

/*  We handle end points by assigning a shadow point just beyond
    the start and end of the line.  This is a bit tricky since we
    have to make sure we ignore any points that are too close
    together.  If the contour is open, we extend the line straigth
    out one more increment.  If the contour is closed, we extend the
    line by wrapping it to the other end.  This ensures that a
    closed contour will have a smooth curve fit at our artificial
    boundry.                                                         */

x3=xyend; y3=xyend+1;                  /* Point to last X,Y          */
x2=x3; y2=y3;
do {                                   /* Loop to find prior point   */
  x2-=2; y2-=2;                        /* Check next prior point     */
  if (x2<xystrt) goto exit;            /* No valid line to draw      */
  d2 = hypot ((*x3-*x2), (*y3-*y2));   /* Get distance               */
} while (d2<0.01);                     /* Loop til distance is big   */

x0=xystrt; y0=xystrt+1;                /* Point to first X,Y         */
x1=x0; y1=y0;
do {                                   /* Loop to find next point    */
  x1+=2; y1+=2;                        /* Point to next X,Y          */
  if (x1>xyend) goto exit;             /* Exit if no valid line      */
  d1 = hypot ((*x1-*x0), (*y1-*y0));   /* Distance to next point     */
} while (d1<0.01);                     /* Keep looping til d1 is big */

if (icls) {                            /* Select shadow points       */
  *(xystrt-2) = *x2;                   /* Wrap for closed contour    */
  *(xystrt-1) = *y2;
  *(xyend+2)  = *x1;
  *(xyend+3)  = *y1;
}
else {
  *(xystrt-2) = *x0 + (*x0 - *x1);     /* Linear for open contour    */
  *(xystrt-1) = *y0 + (*y0 - *y1);
  *(xyend+2)  = *x3 + (*x3 - *x2);
  *(xyend+3)  = *y3 + (*y3 - *y2);
}
/* We have extended the line on either end.  We can now loop through
   the points in the line.  First set up the loop.                   */

x2=x1; x1=x0; x0=xystrt-2; x3=x2;      /* Init pointers to coords    */
y2=y1; y1=y0; y0=xystrt-1; y3=y2;

d0 = hypot ((*x1-*x0), (*y1-*y0));     /* Init distances             */
d1 = hypot ((*x2-*x1), (*y2-*y1));

xt1 = (*x1-*x0)/d0 + (*x2-*x1)/d1;     /* Partial slope calculation  */
yt1 = (*y1-*y0)/d0 + (*y2-*y1)/d1;
xt1 *= kurv;                           /* Curviness factor           */
yt1 *= kurv;

gxplot (*x1,*y1,3);                    /* Start output with pen up   */

/* Loop through the various points in the line                       */

x3+=2; y3+=2; 
while (x3 < xyend+3) {                 /* Loop to end of the line    */

  d2 = hypot ((*x3-*x2),(*y3-*y2));    /* Distance to next point     */
  while (d2<0.01 && x3<xyend+3) {      /* Skip points too close      */
    x3+=2; y3+=2;                      /* Check next point           */
    d2 = hypot ((*x3-*x2),(*y3-*y2));  /* Distance to next point     */
  }
  if (x3 >= xyend+3) break;            /* Went too far?              */

  if (!frombuf) {

    dacum+=d1;                         /* Total dist. from last labl */

    if (pcntr->ltype && ((*y2-*y1)*(*y3-*y2)<0.0) && gxlabn<LABMAX && dacum>dcmin){
      if (clabel[0]) {
        gxlabx[gxlabn]=(*x2);
        gxlaby[gxlabn]=(*y2);
        gxlabs[gxlabn]=0.0;
        strcpy (gxlabv[gxlabn],clabel);
        gxlabc[gxlabn] = gxqclr();
        gxlabn++;
        labflg=1;
      }
      dacum=0.0;
    }
    if (pcntr->ltype==2 && !labflg && gxlabn<LABMAX && x2!=xyend) {
      if (*x1 < *x2 && *x2 < *x3) {
        tslope = atan2(*y3-*y1, *x3-*x1);
        if (fabs(mslope) > fabs(tslope)) {
          mslope = tslope;
          xlb = *x2;
          ylb = *y2;
        }
      } else if (*x1 > *x2 && *x2 > *x3 ) {
        tslope = atan2(*y1-*y3, *x1-*x3);
        if (fabs(mslope) > fabs(tslope)) {
          mslope = tslope;
          xlb = *x2;
          ylb = *y2;
        }
      }
    }
  }

  xt2 = (*x2-*x1)/d1 + (*x3-*x2)/d2;   /* Partial slope calculation  */
  yt2 = (*y2-*y1)/d1 + (*y3-*y2)/d2;
  xt2 *= kurv;                         /* Curviness factor           */
  yt2 *= kurv;

  if (d1>cmax) t=cmax; else t=d1;      /* Limit curviness            */
  sx1 = xt1*t;                         /* Calculate slopes           */
  sx2 = xt2*t;
  sy1 = yt1*t;
  sy2 = yt2*t;

                                       /* Calculate Cubic Coeffic.   */
  ax = sx1 + sx2 + 2.0*(*x1) - 2.0*(*x2);
  bx = 3.0*(*x2) - sx2 - 2.0*sx1 - 3.0*(*x1);
  ay = sy1 + sy2 + 2.0*(*y1) - 2.0*(*y2);
  by = 3.0*(*y2) - sy2 - 2.0*sy1 - 3.0*(*y1);

  tint=del/d1;                         /* How much to increment      */

  for (t=0.0; t<1.0; t+=tint) {        /* Increment this segment     */
    t2=t*t; t3=t2*t;                   /* Get square and cube        */
    x = ax*t3 + bx*t2 + sx1*t + *x1;   /* Get x value on curve       */
    y = ay*t3 + by*t2 + sy1*t + *y1;   /* Get y value on curve       */
    gxplot (x,y,2);                    /* Output the point           */
  }
  d0=d1; d1=d2; xt1=xt2; yt1=yt2;      /* Carry calcs forward        */
  x0=x1; x1=x2; x2=x3;                 /* Update pointers            */
  y0=y1; y1=y2; y2=y3;
  x3+=2; y3+=2;  
}
gxplot (*xyend,*(xyend+1),2);          /* Last point                 */

if (!frombuf && pcntr->ltype==2 && !labflg && gxlabn<LABMAX && fabs(mslope)<2.0) {
  if (clabel[0]) {
    gxlabx[gxlabn]=xlb;
    gxlaby[gxlabn]=ylb;
    gxlabs[gxlabn]=mslope;
    strcpy (gxlabv[gxlabn],clabel);
    gxlabc[gxlabn] = gxqclr();
    gxlabn++;
  }
}

exit:                                  /* No line here, just exit    */
return(0);
}

/* When label masking is not in use, this routine gets called after the 
   contour lines are drawn to plot the labels.  A rectangle with the background
   color is drawn before the label to blank beneath the label.  If label
   masking is in use, this routine should not be called.  */

void gxclab (gadouble csize, gaint flag, gaint colflg) {
gadouble x,y,xy[10],xd1,xd2,yd1,yd2,w,h,buff;
gaint i,lablen,colr,bcol,fcol;

 if (!flag) { gxlabn=0; return; }
 colr = gxqclr();
 for (i=0; i<gxlabn; i++ ) {
   lablen=strlen(gxlabv[i]);
   bcol = gxdbkq();
   if (bcol<2) bcol=0;  /* If bcol is neither black nor white, leave it alone. Otherwise, set to 0 for 'background' */
   h = csize*1.2;                       /* set label height */
   w = 0.2;
   gxchln (gxlabv[i],lablen,csize,&w);  /* get label width */
   if (gxlabs[i]==0.0) {                /* contour label is not rotated */
     x = gxlabx[i] - (w/2.0);           /* adjust reference point */
     y = gxlaby[i] - (h/2.0);
     gxcolr (bcol);
     buff=h*0.2;                        /* add a buffer above and below the string, already padded in X */
     gxrecf (x, x+w, y-buff, y+h+buff); /* draw the background rectangle,  */
     if (colflg>-1) fcol = colflg;
     else fcol = gxlabc[i];
     if (fcol==bcol) gxcolr(1);         /* if label color is same as background, use foreground */
     else gxcolr (fcol);
     gxchpl (gxlabv[i],lablen,x,y,h,csize,0.0);  /* draw the label */
   } else {                             /* contour label is rotated */
     xd1 = (h/2.0)*sin(gxlabs[i]);
     xd2 = (w/2.0)*cos(gxlabs[i]);
     yd1 = (h/2.0)*cos(gxlabs[i]);
     yd2 = (w/2.0)*sin(gxlabs[i]);
     x = gxlabx[i] - xd2 + xd1;         /* adjust reference point */
     y = gxlaby[i] - yd2 - yd1;
     xd1 = (h/2.0*1.6)*sin(gxlabs[i]);
     xd2 = 1.1*(w/2.0)*cos(gxlabs[i]);
     yd1 = (h/2.0*1.6)*cos(gxlabs[i]);
     yd2 = 1.1*(w/2.0)*sin(gxlabs[i]);
     xy[0] = gxlabx[i] - xd2 + xd1;     /* rotated background rectangle => polygon */
     xy[1] = gxlaby[i] - yd2 - yd1;
     xy[2] = gxlabx[i] - xd2 - xd1;
     xy[3] = gxlaby[i] - yd2 + yd1;
     xy[4] = gxlabx[i] + xd2 - xd1;
     xy[5] = gxlaby[i] + yd2 + yd1;
     xy[6] = gxlabx[i] + xd2 + xd1;
     xy[7] = gxlaby[i] + yd2 - yd1;
     xy[8] = xy[0];
     xy[9] = xy[1];
     gxcolr (bcol);
     gxfill (xy,5);
     if (colflg>-1) fcol = colflg;
     else fcol = gxlabc[i];
     if (fcol==bcol) gxcolr(1);         /* if label color is same as background, use foreground */
     else gxcolr (fcol);
     gxchpl (gxlabv[i],lablen,x,y,h,csize,gxlabs[i]*180/M_PI); /* draw the label */
   }
 }
 gxcolr (colr);
 gxlabn=0;
}

/* When label masking is in use, this routine is called to plot all the 
   contour lines after the contour labels have been plotted and their masking
   regions set.  Thus the contour lines drawn will not overlay the labels.  */

void gxpclin (void) {
gaint i,rc;
struct gxclbuf *pclbuf, *p2;
struct gxcntr lcntr;

 /* Set up gxcntr struct appropriately -- most values are dummy */
 lcntr.labsiz = 0.5;
 lcntr.ltype = 0;
 lcntr.mask = 1;
 lcntr.labcol = 1;
 lcntr.ccol = 1;
 lcntr.label = NULL;
 
 /* Copy the lines into fwk, dump the lines,
    release storage, return.  fwk should be guaranteed big enough for the 
    largest line we have, and shouldn't have been release via gxcrel at
    this point. */
 
 pclbuf = clbufanch;
 while (pclbuf) { 
   if (pclbuf->lxy) {
     xystrt = fwk+2;  
     xyend = xystrt + 2*(pclbuf->len-1);
     for (i=0; i<2*pclbuf->len; i++) *(xystrt+i) = *(pclbuf->lxy+i); 
     gxcolr (pclbuf->color);
     gxstyl (pclbuf->style);
     gxwide (pclbuf->width); 
     lcntr.spline = pclbuf->sfit;
     rc = gxcspl(1,&lcntr);
   }
   pclbuf = pclbuf->fpclbuf;
 }
 pclbuf = clbufanch;
 while (pclbuf) {
   p2 = pclbuf->fpclbuf;
   if (pclbuf->lxy) gree (pclbuf->lxy,"c5");
   gree (pclbuf,"c6");
   pclbuf = p2;
 }
 clbufanch = NULL;
 clbuflast = NULL;
 return;
}

/* When gxout shape is in use, this routine is called to dump all the 
   contour vertices to the shapefile
   For each contour in the buffer: 
     get the vertex x/y coordinates, 
     convert them to lon/lat, 
     write out the coordinates to the shapefile,
     release storage and return. 
   Returns -1 on error, otherwise returns number of shapes written to file.
*/
#if USESHP==1
gaint gxshplin (SHPHandle sfid, DBFHandle dbfid, struct dbfld *dbanch) {
gaint i,rc,ival;
struct dbfld *fld;
struct gxclbuf *pclbuf=NULL, *p2;
gaint shpid,*pstart=NULL,nParts,nFields;
SHPObject *shp;
gadouble x,y,*lons=NULL,*lats=NULL,*vals=NULL,lon,lat,val,dval;
 
 nParts = 1;
 nFields = 1;
 pstart = (gaint*)galloc(nParts*sizeof(gaint),"pstart");
 *pstart = 0;
 shpid=0;
 pclbuf = clbufanch;
 if (pclbuf==NULL) {
   printf("Error in gxshplin: contour buffer is empty\n");
   rc = -1; 
   goto cleanup;
 }
 while (pclbuf) { 
   if (pclbuf->lxy) {
     /* allocate memory for lons and lats of the vertices in contour line */
     if ((lons = (gadouble*)galloc (pclbuf->len*sizeof(gadouble),"shplons"))==NULL) {
       printf("Error in gxshplin: unable to allocate memory for lon array\n");
       rc = -1;
       goto cleanup;
     }
     if ((lats = (gadouble*)galloc (pclbuf->len*sizeof(gadouble),"shplats"))==NULL) {
       printf("Error in gxshplin: unable to allocate memory for lat array\n");
       rc = -1;
       goto cleanup;
     }
     if ((vals = (gadouble*)galloc (pclbuf->len*sizeof(gadouble),"shpvals"))==NULL) {
       printf("Error in gxshplin: unable to allocate memory for val array\n");
       rc = -1;
       goto cleanup;
     }
     /* get x,y values and convert them to lon,lat */
     for (i=0; i<pclbuf->len; i++) {
       x = *(pclbuf->lxy+(2*i)); 
       y = *(pclbuf->lxy+(2*i+1)); 
       gxxy2w (x,y,&lon,&lat);
       *(lons+i) = lon;
       *(lats+i) = lat;
       *(vals+i) = pclbuf->val;
     }
     /* create the shape, write it out, then release it */
     shp = SHPCreateObject (SHPT_ARCM,shpid,nParts,pstart,NULL,pclbuf->len,lons,lats,NULL,vals);
     i = SHPWriteObject(sfid,-1,shp);
     SHPDestroyObject(shp);
     if (i!=shpid) {
       printf("Error in gxshplin: SHPWriteObject returned %d, shpid=%d\n",i,shpid);
       rc = -1;
       goto cleanup;
     }
     gree(lons,"c10"); lons=NULL;
     gree(lats,"c11"); lats=NULL;
     gree(vals,"c12"); vals=NULL;
     /* write out the attribute fields for this shape */
     fld = dbanch;           /* point to the first one */
     while (fld != NULL) {
       if (fld->flag==0) {   /* static fields */
	 if (fld->type==FTString) {
	   DBFWriteStringAttribute (dbfid,shpid,fld->index,(const char *)fld->value);
	 } else if (fld->type==FTInteger) {
	   intprs(fld->value,&ival);
	   DBFWriteIntegerAttribute (dbfid,shpid,fld->index,ival);
	 } else if (fld->type==FTDouble) {
	   getdbl(fld->value,&dval);
	   DBFWriteDoubleAttribute (dbfid,shpid,fld->index,dval);
	 }
       }
       else {                /* dynamic fields */
	 if (strcmp(fld->name,"CNTR_VALUE")==0) {
	   val = pclbuf->val;
	   DBFWriteDoubleAttribute (dbfid,shpid,fld->index,val);
	 }
       }
       fld = fld->next;      /* advance to next field */
     }
     shpid++;
   }
   pclbuf = pclbuf->fpclbuf;
 }
 /* if no errors, return the number of contour lines written to the file */
 rc = shpid;
 
 cleanup:
 if (lons) gree (lons,"c7");
 if (lats) gree (lats,"c8");
 if (vals) gree (vals,"c8");
 if (pstart) gree (pstart,"c9");
 /* release the memory in the contour buffer */
 pclbuf = clbufanch;
 while (pclbuf) {
   p2 = pclbuf->fpclbuf;
   if (pclbuf->lxy) gree (pclbuf->lxy,"c5");
   gree (pclbuf,"c6");
   pclbuf = p2;
 }
 clbufanch = NULL;
 clbuflast = NULL;
 return (rc);
}
#endif

/* Routine to write out contour line vertices to a KML file. 
   For each contour in the buffer: 
     get the vertex x/y coordinates, 
     convert them to lon/lat, 
     write out the coordinates to the kmlfile,
     release storage and return. 
   Returns -1 on error, otherwise the number of contours written. 
*/
gaint gxclvert (FILE *kmlfp)  {
struct gxclbuf *pclbuf,*p2;
 gadouble lon,lat,x,y;  
 gaint i,j,c,err;
 err=0;
 c=0;
 pclbuf = clbufanch;
 while (pclbuf) { 
   if (pclbuf->lxy) {
     /* write out headers for each contour */
     snprintf(pout,511,"    <Placemark>\n");
     if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
     snprintf(pout,511,"      <styleUrl>#%d</styleUrl>\n",pclbuf->color);
     if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=2; goto cleanup;}
     snprintf(pout,511,"      <name>%g</name>\n",pclbuf->val);
     if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=3; goto cleanup;}
     snprintf(pout,511,"      <LineString>\n");
     if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=4; goto cleanup;}
     snprintf(pout,511,"        <altitudeMode>clampToGround</altitudeMode>\n");
     if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=5; goto cleanup;}
     snprintf(pout,511,"        <tessellate>1</tessellate>\n");
     if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=6; goto cleanup;}
     snprintf(pout,511,"        <coordinates>\n          ");
     if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=7; goto cleanup;}
     /* get x,y values and convert them to lon,lat */
     j=1;
     for (i=0; i<pclbuf->len; i++) {
       x = *(pclbuf->lxy+(2*i)); 
       y = *(pclbuf->lxy+(2*i+1)); 
       gxxy2w (x,y,&lon,&lat);
       if (lat>90)  lat = 90;
       if (lat<-90) lat = -90;
       snprintf(pout,511,"%g,%g,0 ",lon,lat); 
       if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=8; goto cleanup;}
       if (j==6 || i==(pclbuf->len-1)) { 
	 if (j==6) snprintf(pout,511,"\n          "); 
	 else snprintf(pout,511,"\n"); 
	 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=9; goto cleanup;}
	 j=0;
       }
       j++;
     }
     /* write out footers for each contour */
     snprintf(pout,511,"        </coordinates>\n      </LineString>\n    </Placemark>\n");
     if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=10; goto cleanup;}
     c++;
   }
   pclbuf = pclbuf->fpclbuf;
 }
 cleanup:
 /* release the memory in the contour buffer */
 pclbuf = clbufanch;
 while (pclbuf) {
   p2 = pclbuf->fpclbuf;
   if (pclbuf->lxy) gree (pclbuf->lxy,"c5");
   gree (pclbuf,"c6");
   pclbuf = p2;
 }
 clbufanch = NULL;
 clbuflast = NULL;
 if (err) return (-1);
 else return (c);
}



/* Plot contour labels when label masking is in use. 
   Currently, rot is assumed to be zero.  */

void gxpclab (gadouble xpos, gadouble ypos, gadouble rot, gaint ccol, struct gxcntr *pcntr) {
  gadouble x,y,w,h,csize,buff;
  gaint lablen,bcol,fcol,scol,swid;

  csize = pcntr->labsiz;
  lablen = strlen(clabel);
  bcol = gxdbkq();
  if (bcol<2) bcol=0;  /* If bcol is neither black nor white, leave it alone. Otherwise, set to 0 for 'background' */
  h = csize*1.2;                          /* set label height */
  w = 0.2;
  gxchln (clabel,lablen,csize,&w);        /* get label width */
  buff=h*0.05;                            /* set a small buffer around the label */
  x = xpos-(w/2.0);                       /* adjust reference point */
  y = ypos-(h/2.0);
  scol = gxqclr();
  if (pcntr->labcol > -1) fcol = pcntr->labcol;
  else fcol = ccol;
  gxcolr (fcol);
  swid = gxqwid();
  /* if contour label thickness is set to -999, then we draw a fat version of the label
     in the background color and then overlay a thin version of the label in desired color. 
     This will only work with hershey fonts, since the boldness of cairo fonts is not
     controlled by the thickness setting for contour labels. */
  if (pcntr->labwid > -1) gxwide(pcntr->labwid);
  if (pcntr->labwid == -999) {
    /* invoke settings for fat background label */
     gxwide(12); 
     gxcolr(bcol);
  }
  /* draw the label */
  gxchpl (clabel,lablen,x,y,h,csize,0.0);
  if (pcntr->labwid == -999) {
    /* overlay a thin label in foreground color */
    gxwide(1); gxcolr(fcol);
    gxchpl (clabel,lablen,x,y,h,csize,0.0);
  }
  /* update the mask where this label is positioned */
  gxmaskrec (x-buff, x+w+buff, y-buff, y+h+buff);

  gxcolr (scol);
  gxwide (swid);
}

/* query if the contour label will overlay another, if using masking */

int gxqclab (gadouble xpos, gadouble ypos, gadouble csize) {
gadouble lablen,x,y,w,h,buff;
gaint rc;
  lablen=strlen(clabel);
  w = 0.2;
  h = csize*1.2;            /* height scaled by 1.2 for consistency with other labels */
  gxchln (clabel,lablen,csize,&w);
  x = xpos-(w/2.0);
  y = ypos-(h/2.0);
  buff=h*0.2;                            /* set a buffer around the label */
  /* area to check is a bit (0.02) smaller than the actual mask */
  rc = gxmaskrq (x-buff+0.02, x+w+buff-0.02, y-buff+0.02, y+h+buff-0.02);
  return (rc);
}

/*  Release storage used by the contouring package  */

void gxcrel (void) {
  if (lwk!=NULL) {
    gree (lwk,"c7"); lwk = NULL; 
  }
  if (fwk!=NULL) {
    gree (fwk,"c8"); fwk = NULL;
  }
  lwksiz = 0; fwksiz = 0;
}
