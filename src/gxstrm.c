/* Copyright (C) 1988-2020 by George Mason University. See file COPYRIGHT for more information. */

/* Authored by B. Doty */

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
#include <math.h>
#include "gatypes.h"
#include "gx.h"

void gxstrm (gadouble *u, gadouble *v, gadouble *c, gaint is, gaint js,
   char *umask, char *vmask, char *cmask, gaint flag, gadouble *shdlvs,
   gaint *shdcls, gaint shdcnt, gaint den, gadouble strmarrd, gadouble strmarrsz,
   gaint strmarrt) {
gadouble x,y,xx,yy;
gadouble *up, *vp, *cp, cv1,cv2,cv;
gadouble uv1,uv2,uv,vv1,vv2,vv,auv,avv,xsav,ysav,xold=0.0,yold=0.0;
gadouble fact,rscl,xxsv,yysv,xstrt,ystrt;
gadouble xx1,yy1,xx2,yy2,adj,dacum,tacum;
gaint i,ii,jj,ii1,ij1,i2,j2,ipt,acnt,icol,scol,dis;
gaint *it,siz,iacc,iisav,iscl,imn,imx,jmn,jmx,iz,jz,iss,jss,bflg;
char *upmask,*vpmask,*cpmask;

  scol = -9;
  icol = 1;

  /* Figure out the interval for the flag grid */

  i = is;
  if (js>i) i = js;
  iscl = 200/i;
  iscl = iscl + den - 5;
  if (iscl<1) iscl=1;
  if (iscl>10) iscl=10;
  ii = 1;
  if (den<0) {           /* Support very high resolution grids */
    ii = -1 * (den-1);
    if (ii>10) ii = 10;
    if ((is<200 || js<100) && ii>2) ii=2;    /* Limit downscaling to only high res */
    if ((is<500 || js<250) && ii>5) ii=5;
    if ((is<1000 || js<500) && ii>10) ii=10;
    if ((is<1500 || js<750) && ii>15) ii=15;
    if (ii>20) ii=20;
  }
  rscl = (gadouble)iscl/(gadouble)ii;
  fact = 0.5/rscl;
  /* if (fact<0.3) fact = 0.3; */

  /* Allocate memory for the flag grid */

  iss = is*iscl/ii; jss = js*iscl/ii;
  siz = iss*jss;
  it = (gaint *)malloc(sizeof(gaint) * siz);
  if (it==NULL) {
    printf ("Cannot allocate memory for streamline function\n");
    return;
  }
  for (i=0; i<siz; i++) *(it+i) = 0;

  /* Loop through flag grid to look for start of streamlines.  
     To start requires no streams drawn within surrounding 
     flag boxes.  */

  i2 = 0;
  j2 = 0;
  for (i=0; i<siz; i++) {
    dis = 2;
    if (den<5) dis = 3;
    if (den>5) dis = 1;
    if (den<0) dis = 1;
    if (den<-5) dis = 2;
    imn = i2-dis; imx = i2+dis+1; 
    jmn = j2-dis; jmx = j2+dis+1;
    if (imn<0) imn = 0;
    if (imx>iss) imx = iss;
    if (jmn<0) jmn = 0;
    if (jmx>jss) jmx = jss;
    iacc = 0;
    for (jz=jmn; jz<jmx; jz++) {
      ipt = jz*iss+imn;
      for (iz=imn; iz<imx; iz++) {
        iacc = iacc + *(it+ipt);
        ipt++;
      }
    }
    if (iacc==0){
      x = ((gadouble)i2)/rscl;
      y = ((gadouble)j2)/rscl;
      xsav = x;
      ysav = y;
      xstrt = x; ystrt = y;
      gxconv (x+1.0,y+1.0,&xx,&yy,3);
      gxplot (xx,yy,3);
      xxsv = xx; yysv = yy;
      iisav = -999;
      iacc = 0;
      acnt = 0;
      dacum = 0.0;
      tacum = 0.0;
      bflg = 0;
      while (x>=0.0 && x<(gadouble)(is-1) && y>=0.0 && y<(gadouble)(js-1)) {
        ii = (gaint)x;
        jj = (gaint)y;
        xx = x - (gadouble)ii;
        yy = y - (gadouble)jj;
        up = u + jj*is+ii; upmask = umask + jj*is+ii;  
        vp = v + jj*is+ii; vpmask = vmask + jj*is+ii; 
        if (*upmask==0 || 
	    *(upmask+1)==0 ||
            *(upmask+is)==0 || 
	    *(upmask+is+1)==0) break;
        if (*vpmask==0 || 
	    *(vpmask+1)==0 ||
            *(vpmask+is)==0 || 
	    *(vpmask+is+1)==0) break;
        if (flag) {
          cp = c + jj*is+ii; cpmask = cmask + jj*is+ii;
          if (*cpmask==0 || 
	      *(cpmask+1)==0 ||
              *(cpmask+is)==0 || 
	      *(cpmask+is+1)==0) icol = 15;
          else {
            cv1 = *cp + (*(cp+1)-*cp)*xx;
            cv2 = *(cp+is) + (*(cp+is+1)-*(cp+is))*xx;
            cv = cv1 + (cv2-cv1)*yy;
            icol = gxshdc(shdlvs,shdcls,shdcnt,cv);
          }
          if (icol!=scol && icol>-1) gxcolr(icol);
          scol = icol;
        }
        uv1 = *up + (*(up+1)-*up)*xx;
        uv2 = *(up+is) + (*(up+is+1)-*(up+is))*xx;
        uv = uv1 + (uv2-uv1)*yy;
        vv1 = *vp + (*(vp+1)-*vp)*xx;
        vv2 = *(vp+is) + (*(vp+is+1)-*(vp+is))*xx;
        vv = vv1 + (vv2-vv1)*yy;
        auv = fabs(uv); avv=fabs(vv);
        if (auv<0.1 && avv<0.1) break;
        if (auv>avv) {
          vv = vv*fact/auv;
          uv = uv*fact/auv;
        } else {
          uv = uv*fact/avv;
          vv = vv*fact/avv;
        }
        gxconv (x+1.0,y+1.0,&xx,&yy,3);     /* account for localized grid distortions */
        gxconv (x+1.1,y+1.0,&xx1,&yy1,3);
        gxconv (x+1.0,y+1.1,&xx2,&yy2,3);
        adj = hypot(xx-xx1,yy-yy1)/hypot(xx-xx2,yy-yy2);
        if (adj>1.0) uv=uv/adj;
        else vv = vv*adj;
        if (fabs(uv)<1e-6 && fabs(vv)<1e-6) break;
        x = x + uv;
        y = y + vv;
        ii1 = (gaint)(x*rscl);
        ij1 = (gaint)(y*rscl);
        ii1 = ij1*iss + ii1;
        if (ii1<0 || ii1>=siz) break;
        if (*(it+ii1)==1) break;
        if (ii1!=iisav && iisav>-1) *(it+iisav) = 1;
        if (ii1==iisav) iacc++;
        else {iacc = 0; tacum = 0; }
        if (iacc>10 && tacum<0.1) break;
        if (iacc>100) break;
        iisav = ii1;
        gxconv (x+1.0,y+1.0,&xx,&yy,3);
        if (icol>-1) {
          if (bflg) {gxplot(xold,yold,3); bflg=0;}
          gxplot (xx,yy,2);
        } else bflg = 1;
        dacum += hypot(xx-xold,yy-yold);
        tacum += hypot(xx-xold,yy-yold);
        acnt++;
        if (dacum>strmarrd) {
          if (icol>-1) strmar (xxsv,yysv,xx,yy,strmarrsz,strmarrt);
          acnt = 0; dacum = 0.0;
        }
        xold = xx;
        yold = yy;
        xxsv = xx; yysv = yy;
      }
      bflg = 0;
      x = xsav; y = ysav;
      gxconv (x+1.0,y+1.0,&xx,&yy,3);
      gxplot (xx,yy,3);
      xxsv = xx;
      yysv = yy;
      iisav = -999;
      iacc = 0;
      acnt = 19;
      dacum = 0.0;
      tacum = 0.0;
      while (x>=0.0 && x<(gadouble)(is-1) && y>=0.0 && y<(gadouble)(js-1)) {
        ii = (gaint)x;
        jj = (gaint)y;
        xx = x - (gadouble)ii;
        yy = y - (gadouble)jj;
        up = u + jj*is+ii; upmask = umask + jj*is+ii;  
        vp = v + jj*is+ii; vpmask = vmask + jj*is+ii; 
        if (*upmask==0 || 
	    *(upmask+1)==0 ||
            *(upmask+is)==0 || 
	    *(upmask+is+1)==0) break;
        if (*vpmask==0 || 
	    *(vpmask+1)==0 ||
            *(vpmask+is)==0 || 
	    *(vpmask+is+1)==0) break;
        if (flag) {
          cp = c + jj*is+ii; cpmask = cmask + jj*is+ii; 
          if (*cpmask==0 || 
	      *(cpmask+1)==0 ||
              *(cpmask+is)==0 || 
	      *(cpmask+is+1)==0) icol = 15;
          else {
            cv1 = *cp + (*(cp+1)-*cp)*xx;
            cv2 = *(cp+is) + (*(cp+is+1)-*(cp+is))*xx;
            cv = cv1 + (cv2-cv1)*yy;
            icol = gxshdc(shdlvs,shdcls,shdcnt,cv);
          }
          if (icol!=scol && icol>-1) gxcolr(icol);
          scol = icol;
        }
        uv1 = *up + (*(up+1)-*up)*xx;
        uv2 = *(up+is) + (*(up+is+1)-*(up+is))*xx;
        uv = uv1 + (uv2-uv1)*yy;
        vv1 = *vp + (*(vp+1)-*vp)*xx;
        vv2 = *(vp+is) + (*(vp+is+1)-*(vp+is))*xx;
        vv = vv1 + (vv2-vv1)*yy;
        auv = fabs(uv); avv=fabs(vv);
        if (auv<0.1 && avv<0.1) break;
        if (auv>avv) {
          vv = vv*fact/auv;
          uv = uv*fact/auv;
        } else {
          uv = uv*fact/avv;
          vv = vv*fact/avv;
        }
        gxconv (x+1.0,y+1.0,&xx,&yy,3);     /* account for localized grid distortions */
        gxconv (x+1.1,y+1.0,&xx1,&yy1,3);
        gxconv (x+1.0,y+1.1,&xx2,&yy2,3);
        adj = hypot(xx-xx1,yy-yy1)/hypot(xx-xx2,yy-yy2);
        if (adj>1.0) uv=uv/adj;
        else vv = vv*adj;
        if (fabs(uv)<1e-6 && fabs(vv)<1e-6) break;
        x = x - uv;
        y = y - vv;
        ii1 = (gaint)(x*rscl);
        ij1 = (gaint)(y*rscl);
        ii1 = ij1*iss + ii1;
        if (ii1<0 || ii1>=siz) break;
        if (*(it+ii1)==1) break;
        if (ii1!=iisav && iisav>-1) *(it+iisav) = 1;
        if (ii1==iisav) iacc++;
        else iacc = 0;
        if (iacc>10 && tacum<0.1) break;
        if (iacc>100) break;
        iisav = ii1;
        gxconv (x+1.0,y+1.0,&xx,&yy,3);
        if (icol>-1) {
          if (bflg) {gxplot(xold,yold,3); bflg=0;}
          gxplot (xx,yy,2);
        } else bflg = 1;
        dacum += hypot(xx-xold,yy-yold);
        tacum += hypot(xx-xold,yy-yold);
        xold = xx;
        yold = yy;
        acnt++;
        if (dacum>strmarrd) {
          if (icol>-1) strmar(xx,yy,xxsv,yysv,strmarrsz,strmarrt);
          acnt = 0; dacum=0.0;
        }
        xxsv = xx; yysv = yy;
      }
      ii1 = (gaint)(xstrt*rscl);
      ij1 = (gaint)(ystrt*rscl);
      ii1 = ij1*iss + ii1;
      if (ii1>=0 || ii1<siz) *(it+ii1) = 1;
    }
    i2++;
    if (i2==iss) { i2 = 0; j2++; }
  }
  free (it);
}

static gadouble a150 = 150.0*M_PI/180;

void strmar (gadouble xx1, gadouble yy1, gadouble xx2, gadouble yy2, gadouble sz, gaint type) {
gadouble dir,xy[8];

  if (sz<0.0001) return;
  dir = atan2(yy2-yy1,xx2-xx1);
  xy[0] = xx2; xy[1] = yy2;
  xy[2] = xx2+sz*cos(dir+a150); xy[3] = yy2+sz*sin(dir+a150);
  xy[4] = xx2+sz*cos(dir-a150); xy[5] = yy2+sz*sin(dir-a150);
  xy[6] = xx2; xy[7] = yy2;
  if (type==1) {
    gxplot (xx2,yy2,3);
    gxplot (xy[2],xy[3],2);
    gxplot (xx2,yy2,3);
    gxplot (xy[4],xy[5],2);
    gxplot (xx2,yy2,3);
  }
  if (type==2) {
    gxfill(xy,4);
  }
}

/* Given a shade value, return the relevent color */

gaint gxshdc (gadouble *shdlvs, gaint *shdcls, gaint shdcnt, gadouble val) {
gaint i;

  if (shdcnt==0) return(1);
  if (shdcnt==1) return(shdcls[0]);
  if (val<=shdlvs[1]) return(shdcls[0]);
  for (i=1; i<shdcnt-1; i++) {
    if (val>shdlvs[i] && val<=shdlvs[i+1]) return(shdcls[i]); 
  }
  return(shdcls[shdcnt-1]);
}
