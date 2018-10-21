/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* Authored by B. Doty */
/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* This routine does shaded contour plots.   */

#include <stdio.h>
#include <math.h>
#include "gatypes.h"
#include "gx.h"

void chksid(void);
 
/* Common values for the shading routines.                           */

#define XYBMAX 5000
static gaint *flgh, *flgv;      /* Pointer to flags arrays                 */
static gadouble *xystk[XYBMAX]; /* Pointers to xy stack buffers            */
static gaint stkcnt;            /* Current number of stacked buffers       */
static gadouble *xypnt;         /* Pointer into a stack buffer             */
static gadouble *xybuf;         /* Pointer to xy coord buffer              */
static gaint xycnt;             /* Current count in xy coord buffer        */
static gaint imax, jmax;        /* grid size                               */
static gaint imn,imx,jmn,jmx;   /* Current grid bounds                     */
static gadouble *gr;            /* Pointer to grid                         */
static char *gru;               /* Pointer to grid undef mask              */
static gaint grsize;            /* Number of elements in grid              */
static gaint color;             /* Current color to use for shading        */
static gaint prvclr;            /* Color of one level lower                */
static gadouble val;            /* Current shading level value             */
static gaint bndflg;            /* Current coutour hit a boundry           */


/* The grid r is shaded.  Size is by js.  lvs indicates the number of 
   shaded levels.  vs contains the values bounding the shaded regions.
   clrs contains lvs+1 colors for the shaded regions.  u is the
   undefined grid data value.                                        */

void gxshad (gadouble *r, gaint is, gaint js, gadouble *vs, gaint *clrs, gaint lvs, char *u) {
gadouble *p1,*p2,*p3,*p4;
gadouble x,y,rmin,rmax;
gaint i, j, k, rc; 
gaint *f1,*f2,*f3,*f4;
char *p1u,*p2u,*p3u,*p4u;

  /* Make some stuff global within this file     */
  imax = is;
  jmax = js;
  gr = r; 
  gru = u;

  /* Initialize xy coord buffer and stack buffer setup.             */
  stkcnt = 0;
  for (i=0; i<XYBMAX; i++) xystk[i] = NULL;

  grsize = is*js;
  xybuf = (gadouble *)malloc(sizeof(gadouble)*grsize*2);
  if (xybuf==NULL) {
    printf ("Error in gxshad: Unable to allocate xy coord buffer\n");
    return;
  }
  xycnt = 0;

  /* Alocate flag work area.                                         */
  flgh = (gaint *)malloc(grsize*sizeof(gaint));
  flgv = (gaint *)malloc(grsize*sizeof(gaint));
  if (flgh==NULL || flgv==NULL) {
    printf ("Error in gxshad:  Unable to allocate flags buffer\n");
    return;
  }

  /* Loop through bands of the grid.  Each band is three grid boxes
     wide, which avoids any problems with imbedded max's within min's 
     or with imbedded complex undefined regions.                     */
  imn = 1;
  imx = imax;
  for (jmn=1; jmn<jmax; jmn+=3) {
    jmx = jmn+3;
    if (jmx>jmax) jmx=jmax;
    
    rmin = 9.99e33;
    rmax = -9.99e33;
    for (j=jmn; j<=jmx; j++) {
      p1 = gr + ((j-1)*imax+imn-1);
      p1u = gru + ((j-1)*imax+imn-1);
      for (i=imn; i<=imx; i++) {
        if (*p1u!=0) {
          if (*p1>rmax) rmax = *p1;
          if (*p1<rmin) rmin = *p1;
        }
        p1++; p1u++;      
      }
    }

    /* Loop through shade levels.  */
    prvclr = 0;
    for (k=0; k<lvs; k++) {
      val = vs[k];
      color = clrs[k];
      if (k<lvs-1 && vs[k+1]<rmin) {
        prvclr = color;
        continue;
      }
      if (val>rmax) continue;

      /* Set up flags to indicate which grid boxes contain missing data values
         and where the grid boundries are.  Flag values are:
             0 - nothing yet
             1 - contour has been drawn through this side
             7 - contour drawn through missing data box side
             8 - boundry between missing data value box and non-missing data value box
             9 - missing data value box side                                             */
      
      f1 = flgh + ((jmn-1)*imax);
      f4 = flgv + ((jmn-1)*imax);
      for (j=jmn; j<=jmx; j++) {
        for (i=1; i<=imax; i++) {
         *f1 = 0; *f4 = 0;
         f1++; f4++;
        }
      }
  
      for (j=jmn; j<jmx; j++) {
        p1 = gr + ((j-1)*imax+imn-1);
        p2 = p1+1;
        p3 = p2+imax;
        p4 = p1+imax;

        p1u = gru + ((j-1)*imax+imn-1);
        p2u = p1u+1;
        p3u = p2u+imax;
        p4u = p1u+imax;

        f1 = flgh + ((j-1)*imax+imn-1);
        f2 = flgv + ((j-1)*imax+imn);
        f3 = f1 + imax;
        f4 = f2 - 1;
        for (i=imn; i<imx; i++) {
          if (*p1u==0 || *p2u==0 || *p3u==0 || *p4u==0) {
            *f1=9; *f2=9; *f3=9; *f4=9;
          }
          p1++; p2++; p3++; p4++; 
          p1u++; p2u++; p3u++; p4u++; 
          f1++; f2++; f3++; f4++;  
        }
      }

      for (j=jmn; j<jmx; j++) {
        p1 = gr + ((j-1)*imax+imn-1);
        p2 = p1+1;
        p3 = p2+imax;
        p4 = p1+imax;

        p1u = gru + ((j-1)*imax+imn-1);
        p2u = p1u+1;
        p3u = p2u+imax;
        p4u = p1u+imax;

        f1 = flgh + ((j-1)*imax+imn-1);
        f2 = flgv + ((j-1)*imax+imn);
        f3 = f1 + imax;
        f4 = f2 - 1;
        for (i=imn; i<imx; i++) {
          if (*p1u!=0 && *p2u!=0 && *p3u!=0 && *p4u!=0) {
            if (*f1==9) *f1=8;
            if (*f2==9) *f2=8;
            if (*f3==9) *f3=8;
            if (*f4==9) *f4=8;
          }
          p1++; p2++; p3++; p4++; 
          p1u++; p2u++; p3u++; p4u++; 
          f1++; f2++; f3++; f4++;  
        }
      }

      /* Loop through grid, finding starting locations for a contour 
         line.  Once found, call gxsflw to follow the contour until 
         it is closed.  The contour is closed by following the grid 
         boundry (and missing-data-value boundries) if necessary.      */

      for (j=jmn; j<=jmx; j++) {
        p1 = r + ((j-1)*imax+imn-1);
        p2 = p1+1;
        p4 = p1+imax;
        f1 = flgh + ((j-1)*imax+imn-1);
        f4 = flgv + ((j-1)*imax+imn-1);
        for (i=imn; i<=imx; i++) {
          if (i<imx && (*f1==0 || *f1==8) &&
              ( (*p1<=val && *p2>val) || (*p1>val && *p2<=val) ) ) {
            if (j==jmx) rc = gxsflw(i,j-1,3);
            else rc = gxsflw(i,j,1);
            if (rc) goto err;
          }
          if (j<jmx && (*f4==0 || *f4==8) &&
              ( (*p1<=val && *p4>val) || (*p1>val && *p4<=val) ) ) {
            if (i==imx) rc = gxsflw(i-1,j,2);
            else rc = gxsflw(i,j,4);
            if (rc) goto err;
          }
          p1++; p2++; p4++; 
          f1++; f4++; 
        }
      }


      /* Check for any unfilled regions by looking for any unfollowed
         boundry or missing-data-value sides that have point values
         that are both greater than the current shade value.  This
         indicates a possible closed region (closed by missing data
         value boundries) that we have not yet picked up.  We will  
         bound that region and fill it.                               */

      for (j=jmn; j<=jmx; j++) {
        p1 = r + ((j-1)*imax+imn-1);
        p2 = p1+1;
        p4 = p1+imax;
        f1 = flgh + ((j-1)*imax+imn-1);
        f4 = flgv + ((j-1)*imax+imn-1);
        for (i=imn; i<=imx; i++) {
          rc = 0;
          if (i<imx) {
            if (j==jmn && *f1==0 && *p1>val && *p2>val) rc = gxsflw(i,j,5);
            if (j==jmx && *f1==0 && *p1>val && *p2>val) rc = gxsflw(i,j,6);
            if (*f1==8 && *p1>val && *p2>val) rc = gxsflw(i+1,j,9);
          }
          if (j<jmx) {
            if (i==imn && *f4==0 && *p1>val && *p4>val) rc = gxsflw(i,j,7);
            if (i==imx && *f4==0 && *p1>val && *p4>val) rc = gxsflw(i,j,8);
            if (*f4==8 && *p1>val && *p4>val) rc = gxsflw(i,j+1,10);
          }
          if (rc) goto err;
          f1++; f4++; p1++; p2++; p4++;
        }
      }
      prvclr = color;
    }
    
    /* All closed maximas have been filled, and all closed minimas 
       have been stacked.  Fill minimas in reverse order.           */

    /* Note: to insure the various bands 'fit' together properly, 
       the boundry points are adjusted outward slightly.  This due to
       the Xserver not filling out to the boundry in poly fills.    */

    for (i=stkcnt-1; i>=0; i--) {
      xypnt = xystk[i];
      xycnt = (gaint)(*xypnt);
      color = (gaint)(*(xypnt+1));
      xypnt+=2;
      for (j=0; j<xycnt; j++) {
        gxconv(*(xypnt+(j*2)),*(xypnt+(j*2+1)),&x,&y,3);
        *(xypnt+(j*2)) = x;
        *(xypnt+(j*2+1)) = y;
      }
      gxcolr (color);
      gxfill (xypnt, xycnt);
      free(xystk[i]);
    }
    stkcnt = 0;
    xycnt = 0;
  }

  /* Free memory areas  */

  free(xybuf);
  free(flgh);
  free(flgv);
  return;

err:
  printf ("Error in gxshad\n");
  for (i=0; i<stkcnt; i++) free(xystk[i]);
  free(xybuf);
  free(flgh);
  free(flgv);
  return;
}
 
gaint gxsflw (gaint i, gaint j, gaint iside) {
 
/* Follow a shaded outline to the end.  Close it if necessary by 
   following around undef areas and around the grid border.         */ 
 
/* The grid box:

              (f3)
     p4      side 3     p3
        x ------------ x
        |              |
 side 4 |              | side 2
  (f4)  |              |  (f2)
        |              |
        x ------------ x
      p1    side 1      p2
             (f1)
                                                                     */
 
gaint *f1,*f2,*f3,*f4,*ff,*fu,*fd,*fl,*fr; 
gaint cnt,rc,isave,jsave,uflag,ucflg,bflag,k; 
gadouble *p1,*p2,*p3,*p4;
gadouble x,y;
 
  isave = i; jsave = j;
  uflag = 0;
  bndflg = 0;

  bflag = 0;
  if (iside==1) goto side1;              /* Jump in based on side    */
  if (iside==2) goto side2;
  if (iside==3) goto side3;
  if (iside==4) goto side4;
  bflag = 1;
  if (iside==5) goto br;
  if (iside==6) goto tr;
  if (iside==7) goto lu;
  if (iside==8) goto ru;
  if (iside==9) goto ur;
  if (iside==10) goto uu;
  printf ("Logic error 40 in gxshad\n");
  return (1);
 
  /* Calculate entry point in the current grid box, then move to the
     next grid box based on the exit side.                           */
 
  side1:                                 /* Enter side 1             */

    if (i<imn || i>(imx-1) || j<jmn || j>jmx) {
      printf ("logic error 12 in gxshad\n");
      printf ("  side1, %i %i \n",i,j);
      return(1);
    }

    p1 = gr + (imax*(j-1)+i-1);
    p2 = p1+1;
    x = (gadouble)i + (val-*p1)/(*p2-*p1);  /* Calculate entry point    */
    y = (gadouble)j;
    rc = putxy(x,y);                     /* Put points in buffer     */
    if (rc) return(rc);
    f1 = flgh + (imax*(j-1)+i-1);
    if (*f1==1 || *f1==7) goto done;     /* We may be done           */
    if (*f1>5 && !uflag) {               /* Entered an undef box?    */
      if (*f1==9) {
        printf ("Logic error 4 in gxshad: %i %i\n",i,j);
        return(1);
      }
      *f1 = 7;                           /* Indicate we were here    */
      if (*p1>val) {
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto uleft;                      
      } else {
        i++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto uright;
      }
    }              
    if (*f1==8) *f1 = 7;                 /* Indicate we were here    */
    else *f1 = 1;
    uflag = 0;
    if (j+1>jmx) {                       /* At top boundry?          */
      if (*p1>val) {
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto tleft;                      
      } else {
        i++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto tright;
      }
    }

    /* Check for exit side.  Also check for col.                    */

    p3 = p2+imax;
    p4 = p3-1;
    if ( (*p2<=val && *p3>val) || (*p2>val && *p3<=val) ) {
      if ( (*p3<=val && *p4>val) || (*p3>val && *p4<=val) ) { 
        if (!spathl(*p1, *p2, *p3, *p4)) {
          i--;
          goto side2;                    /* Exiting 4, go enter 2  */
        }
      }
      i++;
      goto side4;                        /* Exiting 2, go enter 4  */
    }
    if ( (*p3<=val && *p4>val) || (*p3>val && *p4<=val) ) {
      j++;
      goto side1;                        /* Exiting 3, go enter 1  */
    }
    if ( (*p4<=val && *p1>val) || (*p4>val && *p1<=val) ) {
      i--;
      goto side2;                        /* Exiting 4, go enter 2  */
    }
    printf ("Logic error 8 in gxshad\n");
    return(1);
 
  side2:                                 /* Enter side 2           */

    if (i<(imn-1) || i>(imx-1) || j<jmn || j>(jmx-1)) {
      printf ("logic error 12 in gxshad\n");
      printf ("  side2, %i %i \n",i,j);
      return(1);
    }

    p2 = gr + (imax*(j-1)+i);
    p3 = p2+imax;
    x = (gadouble)(i+1);
    y = (gadouble)j + (val-*p2)/(*p3-*p2);  /* Calculate entry point    */
    rc = putxy(x,y);                     /* Put points in buffer     */
    if (rc) return(rc);
    f2 = flgv + (imax*(j-1)+i);
    if (*f2==1 || *f2==7) goto done;     /* We may be done           */
    if (*f2>5 && !uflag) {               /* Entered an undef box?    */
      if (*f2==9) {
        printf ("Logic error 4 in gxshad: %i %i\n",i,j);
        printf ("Side 2, entered %i \n",iside);
        return(1);
      }
      *f2 = 7;                           /* Indicate we were here    */
      if (*p2>val) {
        i++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto udown;                      
      } else {
        i++; j++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto uup;
      }
    }
    if (*f2==8) *f2 = 7;                 /* Indicate we were here    */
    else *f2 = 1;
    uflag = 0;
    if (i<imn) {                         /* At left boundry?         */
      if (*p2>val) {
        i++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto ldown;                      
      } else {
        i++; j++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto lup;
      }
    }

    /* Check for exit side.  Also check for col.                    */

    p1 = p2-1;
    p4 = p3-1;
    if ( (*p3<=val && *p4>val) || (*p3>val && *p4<=val) ) {
      if ( (*p4<=val && *p1>val) || (*p4>val && *p1<=val) ) { 
        if (spathl(*p1, *p2, *p3, *p4)) {
          j--;
          goto side3;                    /* Exiting 1, go enter 3  */
        }
      }
      j++;
      goto side1;                        /* Exiting 3, go enter 1  */
    }
    if ( (*p4<=val && *p1>val) || (*p4>val && *p1<=val) ) {
      i--;
      goto side2;                        /* Exiting 4, go enter 2  */
    }
    if ( (*p1<=val && *p2>val) || (*p1>val && *p2<=val) ) {
      j--;
      goto side3;                        /* Exiting 1, go enter 3  */
    }
    printf ("Logic error 8 in gxshad\n");
    return(1);
 
  side3:                                 /* Enter side 3             */

    if (i<imn || i>(imx-1) || j<(jmn-1) || j>(jmx-1)) {
      printf ("logic error 12 in gxshad\n");
      printf ("  side3, %i %i \n",i,j);
      return(1);
    }

    p3 = gr + (imax*(j)+i);
    p4 = p3-1;
    x = (gadouble)i + (val-*p4)/(*p3-*p4);  /* Calculate entry point    */
    y = (gadouble)(j+1);
    rc = putxy(x,y);                     /* Put points in buffer     */
    if (rc) return(rc);
    f3 = flgh + (imax*(j)+i-1);
    if (*f3==1 || *f3==7) goto done;     /* We may be done           */
    if (*f3>5 && !uflag) {               /* Entered an undef box?    */
      if (*f3==9) {
        printf ("Logic error 4 in gxshad: %i %i\n",i,j);
        printf ("Side 3, entered %i \n",iside);
        return(1);
      }
      *f3 = 7;                           /* Indicate we were here    */
      if (*p3>val) {
        i++; j++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto uright;                     
      } else {
        j++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto uleft;
      }
    }
    if (*f3==8) *f3 = 7;                 /* Indicate we were here    */
    else *f3 = 1;
    uflag = 0;
    if (j<jmn) {                         /* At bottom boundry?       */
      if (*p3>val) {
        i++; j++; 
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto bright;                     
      } else {
        j++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto bleft; 
      }
    }

    /* Check for exit side.  Also check for col.                    */

    p1 = p4-imax;
    p2 = p1+1;
    if ( (*p1<=val && *p4>val) || (*p1>val && *p4<=val) ) {
      if ( (*p1<=val && *p2>val) || (*p1>val && *p2<=val) ) { 
        if (!spathl(*p1, *p2, *p3, *p4)) {
          i++;
          goto side4;                    /* Exiting 2, go enter 4  */
        }
      }
      i--;
      goto side2;                        /* Exiting 4, go enter 2  */
    }
    if ( (*p1<=val && *p2>val) || (*p1>val && *p2<=val) ) {
      j--;
      goto side3;                        /* Exiting 1, go enter 3  */
    }
    if ( (*p2<=val && *p3>val) || (*p2>val && *p3<=val) ) {
      i++;
      goto side4;                        /* Exiting 2, go enter 4  */
    }
    printf ("Logic error 8 in gxshad\n");
    return(1);
 
  side4:                                 /* Enter side 4           */

    if (i<1 || i>imax || j<1 || j>(jmax-1)) {
      printf ("logic error 12 in gxshad\n");
      printf ("  side4, %i %i \n",i,j);
      printf (" imax, jmax = %i %i \n",imax,jmax);
      return(1);
    }

    p1 = gr + (imax*(j-1)+i-1);
    p4 = p1+imax;
    x = (gadouble)i;
    y = (gadouble)j + (val-*p1)/(*p4-*p1);  /* Calculate entry point    */
    rc = putxy(x,y);                     /* Put points in buffer     */
    if (rc) return(rc);
    f4 = flgv + ((j-1)*imax+i-1);
    if (*f4==1 || *f4==7) goto done;     /* We may be done           */
    if (*f4>5 && !uflag) {               /* Entered an undef box?    */
      if (*f4==9) {
        printf ("Logic error 4 in gxshad: %i %i\n",i,j);
        printf ("Side 4, entered %i \n",iside);
        return(1);
      }
      *f4 = 7;                           /* Indicate we were here    */
      if (*p1>val) {
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto udown;                      
      } else {
        j++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto uup;
      }
    }
    if (*f4==8) *f4 = 7;                 /* Indicate we were here    */
    else *f4 = 1;
    uflag = 0;
    if (i+1>imx) {                       /* At right boundry?        */
      if (*p1>val) {
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto rdown;                      
      } else {
        j++;
        rc = putxy((gadouble)i,(gadouble)j);
        if (rc) return(rc);
        goto rup;
      }
    }

    /* Check for exit side.  Also check for col.                    */

    p2 = p1+1;
    p3 = p4+1;
    if ( (*p1<=val && *p2>val) || (*p1>val && *p2<=val) ) {
      if ( (*p2<=val && *p3>val) || (*p2>val && *p3<=val) ) { 
        if (spathl(*p1, *p2, *p3, *p4)) {
          j++;
          goto side1;                    /* Exiting 3, go enter 1  */
        }
      }
      j--;
      goto side3;                        /* Exiting 1, go enter 3  */
    }
    if ( (*p2<=val && *p3>val) || (*p2>val && *p3<=val) ) {
      i++;
      goto side4;                        /* Exiting 2, go enter 4  */
    }
    if ( (*p3<=val && *p4>val) || (*p3>val && *p4<=val) ) {
      j++;
      goto side1;                        /* Exiting 3, go enter 1  */
    }
    printf ("Logic error 8 in gxshad\n");
    return(1);

  /* At an undefined boundry and last moved towards the left.  */

  uleft:

    bndflg = 1;
    if (bflag && i==isave && j==jsave) goto done;
    if (j<(jmn+1)||j>jmx-1) {
      printf ("Logic error 16 in gxshad\n");
      return (1);
    }
    fu = flgv + ((j-1)*imax+i-1);
    fd = fu-imax;
    if (i==imn) {
      if ((*fu>5 && *fd>5) || (*fu<5 && *fd<5)) {
        printf ("Logic error 20 in gxshad\n");
        return (1);
      }
      if (*fu>5) goto ldown;
      else goto lup;
    }
    ff = flgh + ((j-1)*imax+i-2);
    cnt=0;
    if (*ff==7 || *ff==8) cnt++;
    if (*fu==7 || *fu==8) cnt++;
    if (*fd==7 || *fd==8) cnt++;
    if (cnt==2 || cnt==0) {
      printf ("Logic error 24 in gxshad\n");
      return (1);
    }
    ucflg = 0;
    if (cnt==3) ucflg = undcol(i,j);
    if (ucflg==9) return(1);
    if (!ucflg && (*ff==7 || *ff==8)) {
      i--;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        if (*fu>5) {
          j--;
          goto side3;
        } else goto side1;
      }
      *ff = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto uleft;
    }
    if (ucflg!=2 && (*fd==7 || *fd==8)) {
      j--;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        if (ucflg || *fu==9) {
          goto side4;
        } else {
          i--;
          goto side2;
        }
      }
      *fd = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto udown;
    }
    if (ucflg!=1 && (*fu==7 || *fu==8)) {
      j++;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        if (ucflg || *fd==9) {
          j--;
          goto side4;
        } else {
          i--; j--; 
          goto side2;
        }
      }
      *fu = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto uup;
    }
    printf ("Logic error 28 in gxshad\n");
    return(1);
     
  /* At an undefined boundry and last moved towards the right. */

  uright:

    if (bflag && i==isave && j==jsave) goto done;

  ur:

    bndflg = 1;
    if (j<(jmn+1)||j>jmx-1) {
      printf ("Logic error 16 in gxshad\n");
      return (1);
    }
    fu = flgv + ((j-1)*imax+i-1);
    fd = fu-imax;
    if (i==imx) {
      if ((*fu>5 && *fd>5) || (*fu<5 && *fd<5)) {
        printf ("Logic error 20 in gxshad\n");
        return (1);
      }
      if (*fu>5) goto rdown;
      else goto rup;
    }
    ff = flgh + ((j-1)*imax+i-1);
    cnt=0;
    if (*ff==7 || *ff==8) cnt++;
    if (*fd==7 || *fd==8) cnt++;
    if (*fu==7 || *fu==8) cnt++;
    if (cnt==2 || cnt==0) {
      printf ("Logic error 24 in gxshad\n");
      return (1);
    }
    ucflg = 0;
    if (cnt==3) ucflg = undcol(i,j);
    if (ucflg==9) return(1);
    if (!ucflg && (*ff==7 || *ff==8)) {
      i++;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        i--;
        if (*fu>5) {
          j--;
          goto side3;
        } else goto side1;
      }
      *ff = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto uright;
    }
    if (ucflg!=1 && (*fd==7 || *fd==8)) {
      j--;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1; 
        if (ucflg || *fu==9) {
          i--;
          goto side2;
        } else {
          goto side4;
        }
      }
      *fd = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto udown;
    }
    if (ucflg!=2 && (*fu==7 || *fu==8)) {
      j++;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        if (ucflg || *fd==9) {
          i--; j--;
          goto side2;
        } else {
          j--; 
          goto side4;
        }
      }
      *fu = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto uup;
    }
    printf ("Logic error 28 in gxshad\n");
    return(1);
     
  /* At an undefined boundry and last moved towards the top.   */

  uup:

    if (bflag && i==isave && j==jsave) goto done;

  uu:

    bndflg = 1;
    if (i<(imn+1)||i>imx-1) {
      printf ("Logic error 16 in gxshad\n");
      return (1);
    }
    fr = flgh + ((j-1)*imax+i-1);
    fl = fr-1;
    if (j==jmx) {
      if ((*fr>5 && *fl>5) || (*fr<5 && *fl<5)) {
        printf ("Logic error 20 in gxshad\n");
        return (1);
      }
      if (*fr>5) goto tleft;
      else goto tright;
    }
    ff = flgv + ((j-1)*imax+i-1);
    cnt=0;
    if (*ff==7 || *ff==8) cnt++;
    if (*fr==7 || *fr==8) cnt++;
    if (*fl==7 || *fl==8) cnt++;
    if (cnt==2 || cnt==0) {
      printf ("Logic error 24 in gxshad\n");
      return (1);
    }
    ucflg = 0;
    if (cnt==3) ucflg = undcol(i,j);
    if (ucflg==9) return(1);
    if (!ucflg && (*ff==7 || *ff==8)) {
      j++;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        j--;
        uflag = 1;
        if (*fr>5) {
          i--;
          goto side2;
        }
        else goto side4;
      }
      *ff = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto uup;   
    }
    if (ucflg!=2 && (*fr==7 || *fr==8)) {
      i++;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        if (ucflg || *fl==9) {
          i--; j--;
          goto side3;
        } else {
          i--;
          goto side1;
        }
      }
      *fr = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto uright;
    }
    if (ucflg!=1 && (*fl==7 || *fl==8)) {
      i--;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        if (ucflg || *fr==9) {
          j--;
          goto side3;
        } else {
          goto side1;
        }
      }
      *fl = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto uleft;
    }
    printf ("Logic error 28 in gxshad\n");
    return(1);
     
  /* At an undefined boundry and last moved towards the bottom.  */

  udown:

    bndflg = 1;
    if (bflag && i==isave && j==jsave) goto done;
    if (i<(imn+1)||i>imx-1) {
      printf ("Logic error 16 in gxshad\n");
      return (1);
    }
    fr = flgh + ((j-1)*imax+i-1);
    fl = fr-1;
    if (j==jmn) {
      if ((*fr>5 && *fl>5) || (*fr<5 && *fl<5)) {
        printf ("Logic error 20 in gxshad\n");
        return (1);
      }
      if (*fr>5) goto bleft;
      else goto bright;
    }
    ff = flgv + ((j-2)*imax+i-1);
    cnt=0;
    if (*ff==7 || *ff==8) cnt++;
    if (*fr==7 || *fr==8) cnt++;
    if (*fl==7 || *fl==8) cnt++;
    if (cnt==2 || cnt==0) {
      printf ("Logic error 24 in gxshad\n");
      return (1);
    }
    ucflg = 0;
    if (cnt==3) ucflg = undcol(i,j);
    if (ucflg==9) return(1);
    if (!ucflg && (*ff==7 || *ff==8)) {
      j--;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        if (*fr>5) {
          i--;
          goto side2;
        }
        else goto side4;
      }
      *ff = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto udown;   
    }
    if (ucflg!=1 && (*fr==7 || *fr==8)) {
      i++;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        if (ucflg || *fl==9) {
          i--;
          goto side1;
        } else {
          i--; j--;
          goto side3;
        }
      }
      *fr = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto uright;
    }
    if (ucflg!=2 && (*fl==7 || *fl==8)) {
      i--;
      p1 = gr + ((j-1)*imax+i-1);
      if (*p1<=val) {
        uflag = 1;
        if (ucflg || *fr==9) {
          goto side1;
        } else {
          j--;
          goto side3;
        }
      }
      *fl = 7;
      rc = putxy((gadouble)i,(gadouble)j);
      if (rc) return(rc);
      goto uleft;
    }
    printf ("Logic error 28 in gxshad\n");
    return(1);

  /* Follow grid boundry until we hit a missing data area, or until
     we hit the restart of the contour line.                         */

  tright:

    if (bflag && i==isave && j==jsave) goto done;

  tr:

    bndflg = 1; 
    if (i==imx) goto rdown;
    ff = flgh + ((j-1)*imax+i-1);
    if (*ff>5) goto udown;
    i++;
    p1 = gr + ((j-1)*imax+i-1);
    if (*p1<=val) {
      j--; i--;
      goto side3;
    }
    *ff = 1;
    rc = putxy((gadouble)i,(gadouble)j);
    if (rc) return(rc);
    goto tright;

  tleft:

    bndflg = 1;
    if (bflag && i==isave && j==jsave) goto done;
    if (i==imn) goto ldown;
    ff = flgh + ((j-1)*imax+i-2);
    if (*ff>5) goto udown;
    i--;
    p1 = gr + ((j-1)*imax+i-1);
    if (*p1<=val) {
      j--;
      goto side3;
    }
    *ff = 1;
    rc = putxy((gadouble)i,(gadouble)j);
    if (rc) return(rc);
    goto tleft; 

  bright:

    if (bflag && i==isave && j==jsave) goto done;

  br:

    bndflg = 1;
    if (i==imx) goto rup;
    ff = flgh + ((j-1)*imax+i-1);
    if (*ff>5) goto uup;
    i++;
    p1 = gr + ((j-1)*imax+i-1);
    if (*p1<=val) {
      i--;
      goto side1;
    }
    *ff = 1;
    rc = putxy((gadouble)i,(gadouble)j);
    if (rc) return(rc);
    goto bright;

  bleft:

    bndflg = 1;
    if (bflag && i==isave && j==jsave) goto done;
    if (i==imn) goto lup;
    ff = flgh + ((j-1)*imax+i-2);
    if (*ff>5) goto uup;
    i--;
    p1 = gr + ((j-1)*imax+i-1);
    if (*p1<=val) {
      goto side1;
    }
    *ff = 1;
    rc = putxy((gadouble)i,(gadouble)j);
    if (rc) return(rc);
    goto bleft; 
     
  rup:

    if (bflag && i==isave && j==jsave) goto done;

  ru:

    bndflg = 1;
    if (j==jmx) goto tleft;
    ff = flgv + ((j-1)*imax+i-1);
    if (*ff>5) goto uleft;
    j++;
    p1 = gr + ((j-1)*imax+i-1);
    if (*p1<=val) {
      j--; i--;
      goto side2;
    }
    *ff = 1;
    rc = putxy((gadouble)i,(gadouble)j);
    if (rc) return(rc);
    goto rup;   
     
  rdown:

    bndflg = 1;
    if (bflag && i==isave && j==jsave) goto done;
    if (j==jmn) goto bleft;
    ff = flgv + ((j-2)*imax+i-1);
    if (*ff>5) goto uleft;
    j--;
    p1 = gr + ((j-1)*imax+i-1);
    if (*p1<=val) {
      i--;
      goto side2;
    }
    *ff = 1;
    rc = putxy((gadouble)i,(gadouble)j);
    if (rc) return(rc);
    goto rdown; 
     
  lup:

    if (bflag && i==isave && j==jsave) goto done;

  lu:

    bndflg = 1;
    if (j==jmx) goto tright;
    ff = flgv + ((j-1)*imax+i-1);
    if (*ff>5) goto uright;
    j++;
    p1 = gr + ((j-1)*imax+i-1);
    if (*p1<=val) {
      j--; 
      goto side4;
    }
    *ff = 1;
    rc = putxy((gadouble)i,(gadouble)j);
    if (rc) return(rc);
    goto lup;   
     
  ldown:

    bndflg = 1;
    if (bflag && i==isave && j==jsave) goto done;
    if (j==jmn) goto bright;
    ff = flgv + ((j-2)*imax+i-1);
    if (*ff>5) goto uright;
    j--;
    p1 = gr + ((j-1)*imax+i-1);
    if (*p1<=val) {
      goto side4;
    }
    *ff = 1;
    rc = putxy((gadouble)i,(gadouble)j);
    if (rc) return(rc);
    goto ldown; 
     
  done:

  shdcmp();
  if (xycnt<4) goto cont;
  if (shdmax()) {
    for (k=0; k<xycnt; k++) {
      gxconv(*(xybuf+(k*2)),*(xybuf+(k*2+1)),&x,&y,3);
      *(xybuf+(k*2)) = x;
      *(xybuf+(k*2+1)) = y;
    }
    gxcolr (color);
    gxfill (xybuf, xycnt);
  } else {
    xystk[stkcnt] = (gadouble *)malloc(sizeof(gadouble)*(xycnt+1)*2);
    if (xystk[stkcnt]==NULL) {
      printf ("Memory allocation error in gxshad: stack buffer\n");
      return (1);
    }
    xypnt = xystk[stkcnt];
    *xypnt = (gadouble)(xycnt)+0.1;
    *(xypnt+1) = (gadouble)(prvclr)+0.1;
    xypnt+=2;
    for (k=0; k<xycnt; k++) {
      *(xypnt+(k*2)) = *(xybuf+(k*2));
      *(xypnt+(k*2+1)) = *(xybuf+(k*2+1));
    }
    stkcnt++;
    if (stkcnt>=XYBMAX) {
      printf ("Buffer stack limit exceeded in gxshad\n");
      return(1);
    }
  }

cont:
  xycnt = 0;
  return (0);

}
 
/* Calculate shortest combined path length through a col point.
   Return true if shortest path is side 1/2,3/4, else false.         */
 
gaint spathl (gadouble p1, gadouble p2, gadouble p3, gadouble p4) {
gadouble v1,v2,v3,v4,d1,d2;
 
  v1 = (val-p1)/(p2-p1);
  v2 = (val-p2)/(p3-p2);
  v3 = (val-p4)/(p3-p4);
  v4 = (val-p1)/(p4-p1);
  d1 = hypot(1.0-v1, v2) + hypot(1.0-v4, v3);
  d2 = hypot(v1, v4) + hypot(1.0-v2, 1.0-v3);
  if (d2<d1) return (0);
  return (1);
}

/* Determine characteristics of an undefined path col */

gaint undcol (gaint i, gaint j) {
gadouble *p1, *p2, *p3, *p4;
char *p1u, *p2u, *p3u, *p4u;

  if (i<2 || i>imax-1 || j<2 || j>jmax-1) {
    printf ("Logic error 32 in gxshad\n");
    return (9);
  }
  p1 = gr + ((j-2)*imax+i-2);
  p2 = p1 + 2;
  p3 = p2 + imax*2;
  p4 = p3 - 2;

  p1u = gru + ((j-2)*imax+i-2);
  p2u = p1u + 2;
  p3u = p2u + imax*2;
  p4u = p3u - 2;


  if (*p1u==0 && *p3u==0 && *p2u!=0 && *p4u!=0) return(1);
  if (*p1u!=0 && *p3u!=0 && *p2u==0 && *p4u==0) return(2);
  printf ("Logic error 36 in gxshad\n");
  return (9);
}


gaint putxy (gadouble x, gadouble y) {

  if (xycnt>=grsize) return(1);
  *(xybuf+(xycnt*2)) = x;
  *(xybuf+(xycnt*2+1)) = y;
  xycnt++;
  return(0);
}

/* Remove duplicate consecutive points from the closed contour   */

void shdcmp (void) {
gaint i,j;

  i=0;
  for (j=1; j<xycnt; j++) {
    if ( *(xybuf+(i*2)) != *(xybuf+(j*2)) || 
         *(xybuf+(i*2+1)) != *(xybuf+(j*2+1)) ) {
      i++;
      if (i!=j) {
        *(xybuf+(i*2)) = *(xybuf+(j*2)); 
        *(xybuf+(i*2+1)) = *(xybuf+(j*2+1));
      }
    }
  }
  xycnt = i+1;
}
      
/* Determine if the current closed contour (contained in xybuf) 
   is a max or a min.                                            */

gaint shdmax(void) {
gadouble x, y, xsave, ysave=0.0, *p1;
gaint i,j;

  /* If we hit some boundry during our travels, then this one has
     to be a max (since we are doing strips 3 grid boxes wide, and
     this makes it impossible to have a "floating" undef region)  */

  if (bndflg) return(1);

  /* Find the minimum x value in the contour line.  Check the   
     right hand point to see if this is a max or a min.          */

  xsave = 9.9e33;
  for (i=0; i<xycnt; i++) {
    x = *(xybuf+(i*2));
    y = *(xybuf+(i*2+1));
    if ( y == floor(y) && x < xsave) {
      xsave = x;
      ysave = y;
    }
  }
  i = (gaint)xsave;
  j = (gaint)ysave;
  p1 = gr + ((j-1)*imax+i);
  if (*p1 > val) return (1);
  return (0);
}

void chksid (void) {
gaint *f1,*f4;
gaint i,j;
gadouble x,y;
    f1 = flgh;
    f4 = flgv;
    for (j=1; j<=jmax; j++) {
      for (i=1; i<=imax; i++) {
        if (i<imax) {              
          if (*f1==1) gxcolr(1);
          else if (*f1==7) gxcolr(3);
          else if (*f1==8) gxcolr(8);
          else if (*f1==9) gxcolr(4);
          else if (*f1==0) gxcolr(15);
          else gxcolr(2);
          gxconv((gadouble)i,(gadouble)j,&x,&y,3);
          gxplot(x,y,3);
          gxconv((gadouble)(i+1),(gadouble)j,&x,&y,3);
          gxplot(x,y,2);
        }
        if (j<jmax) {
          if (*f4==1) gxcolr(1);
          else if (*f4==7) gxcolr(3);
          else if (*f4==8) gxcolr(8);
          else if (*f4==9) gxcolr(4);
          else if (*f4==0) gxcolr(15);
          else gxcolr(2);
          gxconv((gadouble)i,(gadouble)j,&x,&y,3);
          gxplot(x,y,3);
          gxconv((gadouble)i,(gadouble)(j+1),&x,&y,3);
          gxplot(x,y,2);
        }
        f1++; f4++;
      }
    }
    gxfrme (0);
}
