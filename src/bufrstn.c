/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* Authored by Jennifer Adams */

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
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "grads.h"
#include "gx.h"

/* This routine parses a BUFR file, then collects reports that match a data request */
 
gaint getbufr (struct gastn *stn) {
  char   ch1[16],ch2[16],pout[256];
  gaint    i,k,rc,mintim,maxtim,tim,oflg,dummy,keepreport,msgcnt;
  gaint    t,f,minf,maxf,minft,maxft,verb,toffneg;
  gadouble  minlon,maxlon,minlat,maxlat,minlev,maxlev;
  gadouble  hlon=0,hlat=0,hlev,htim;
  double rfold;
  struct garpt *rpt;
  struct bufrhdr subsethdr,rfhdr;
  gabufr_msg *msg;
  gabufr_val *val, *bval, *rfblockbeg;

  /* For informational messages, set verb=1. For debugging, set verb=2. */
  verb=1;

  /* Get dimension limits */
  mintim = stn->tmin;    maxtim = stn->tmax;
  minlon = stn->dmin[0]; maxlon = stn->dmax[0];
  minlat = stn->dmin[1]; maxlat = stn->dmax[1];
  minlev = stn->dmin[2]; maxlev = stn->dmax[2];
  if (minlev > maxlev) {
    minlev = stn->dmax[2];
    maxlev = stn->dmin[2];
  }
  if (stn->rflag) {
    minlon = minlon - stn->radius;
    maxlon = maxlon + stn->radius;
    minlat = minlat - stn->radius;
    maxlat = maxlat + stn->radius;
  }
  stn->rnum = 0;

  /* Loop through times -- How many data files are we going to have to open? */
  maxf=maxft=0;
  minf=minft=stn->pfi->dnum[3];
  for (tim=mintim; tim<=maxtim; tim++) {
    if (tim < 1) continue;
    if (tim > stn->pfi->dnum[3]) break;
    f = *(stn->pfi->fnums+tim-1);  /* f is filenumber for this time */
    if (f < minf) {
      minf = f;
      minft = tim;
    }
    if (f > maxf) {
      maxf = f;
      maxft = tim;
    }
  }

  /* Loop through files */
  for (f=minf; f<=maxf; f++) {

    /* Find a time axis index that will open file f */
    for (t=minft; t<=maxft; t++) {
      if (*(stn->pfi->fnums+t-1) == f) {
	tim=t;
	break;
      }
    }

    /* Call gaopfn to set the file name */
    rc = gaopfn(tim,1,&dummy,&oflg,stn->pfi);
    if (rc==-99999) {
      gaprnt(0,"getbufr error: gaopfn returned -99999\n");
      goto err;
    }
    if (rc==-88888) continue;
      
    /* Parse the BUFR data if it hasn't already been done */
    if (!stn->pfi->bufrdset) {
      gabufr_set_tbl_base_path(gxgnam("tables"));
      if (stn->pfi->tmplat) {
	if (verb) {
	  snprintf(pout,255,"Parsing BUFR file %s\n",stn->pfi->tempname);
	  gaprnt(2,pout);
	}
	stn->pfi->bufrdset = gabufr_open(stn->pfi->tempname);
      } 
      else { 
	if (verb) {
	  snprintf(pout,255,"Parsing BUFR file %s\n",stn->pfi->name);
	  gaprnt(2,pout);
	}
	stn->pfi->bufrdset = gabufr_open(stn->pfi->name);
      }
      if (!stn->pfi->bufrdset) {
	gaprnt(0,"Error from getbufr: gabufr_open failed\n");
	goto err; 

      } else {
	if (verb) gaprnt(2,"Finished parsing BUFR file\n");
      }
    }

    msgcnt = -1;
    /* Loop through bufr messages looking for valid reports */
    for (msg = stn->pfi->bufrdset->msgs; msg != NULL; msg = msg->next) { 
      msgcnt++;
      if (msg->is_new_tbl) continue;

      /* loop through msg subsets */
      for (i = 0; i < msg->subcnt; i++) {                     
  
	/* Copy time vals from the msg header, intialize others */
	subsethdr.tvals.yr = msg->year; 
	subsethdr.tvals.mo = msg->month;
	subsethdr.tvals.dy = msg->day;
	subsethdr.tvals.hr = msg->hour;
	subsethdr.tvals.mn = msg->min;
	toffneg = 0;
	subsethdr.sec = 0;
	subsethdr.toffvals.yr = 0;     /* offset times are initially zero */
	subsethdr.toffvals.mo = 0;
	subsethdr.toffvals.dy = 0;
	subsethdr.toffvals.hr = 0;
	subsethdr.toffvals.mn = 0;
	subsethdr.offsec = 0;
	subsethdr.lon      = -999;
	subsethdr.lat      = -999;
	subsethdr.lev      = -999;
	for (k=0;k<8;k++) *(subsethdr.stid+k)='?';

	/* Look for coordinate values in subset (First loop through subset vals) */
	getbufrhdr(msg->subs[i], NULL, stn->pfi->bufrinfo, &subsethdr, 0, &toffneg);

	/* Sort gabufr_vals into blocks according to their repetition factors (rf) */
	val = msg->subs[i];    /* first val in subset */
	rfblockbeg = val;      /* first val in initial rfblock */
	rfold = val->z;        /* initial rf */

	while (1) {  

	  /* (!val) occurs when all gabufr_vals in the subset have the same rf */
	  /* (val->z != rfold) marks the end of a block of gabufr_vals that have the same rf */
	  if ( (!val) || (val->z != rfold) ) {    

	    /* rfblock is a set of gabufr_vals that have the same repetition factor */
	    /* Copy bufrhdr values from the subset, look in the rfblock for more */
	    rfhdr.tvals = subsethdr.tvals;
	    rfhdr.toffvals = subsethdr.toffvals;
	    rfhdr = subsethdr;	  
	    getbufrhdr(rfblockbeg, val, stn->pfi->bufrinfo, &rfhdr, 1, &toffneg);

	    /* Determine if we want this report */
	    keepreport=1;
	    /* Convert seconds to minutes and add fields */ 
	    rfhdr.tvals.mn    += rfhdr.sec/60;   
	    rfhdr.toffvals.mn += rfhdr.offsec/60;  

	    /* Merge the time values and time offset values to get report time */
	    if (toffneg) {
 	      timsub(&rfhdr.tvals,&rfhdr.toffvals);  
	    } else {
	      timadd(&rfhdr.tvals,&rfhdr.toffvals);  
	    }

	    /* Get the report time in grid coordinates */
	    htim = t2gr(stn->tvals,&rfhdr.toffvals); 

	    /* Check if time is within range*/
	    if (stn->ftmin==stn->ftmax) {
	      if (fabs(htim-stn->ftmin)>0.5) {
		keepreport=0;
		if (verb==2) {
		  printf("report time (%4.1f) is outside range; dmin/dmax=%4.1f tim=%d\n",
			 htim,stn->ftmin,mintim);
		}
	      } 		
	    } else {
	      if (htim<stn->ftmin || htim>stn->ftmax) {
		keepreport=0;
		if (verb==2) {
		  printf("report time (%4.1f) is outside range; dmin=%4.1f dmax=%4.1f tmin=%d tmax=%d\n",
			 htim,stn->ftmin,stn->ftmax,mintim,maxtim);
		}
	      }
	    }

	    if (keepreport) {
	      if (stn->sflag) {
		/* check if stids match */
		for (k=0; k<8; k++) *(ch1+k) = tolower(*(rfhdr.stid+k));
		for (k=0; k<8; k++) *(ch2+k) = *(stn->stid+k);
		if (!cmpwrd(ch1,ch2)) {
		  keepreport=0;
		  if (verb==2) printf("report stid doesn't match\n");
		}

	      } else {
		/* check if stid is still the initialized value */
		for (k=0;k<8;k++) if (*(rfhdr.stid+k) == '?') {
		  keepreport=0;
		  if (verb==2) printf("report has no stid\n");
		}
		/* check if lat and lon are within range */
		hlon = rfhdr.lon;
		hlat = rfhdr.lat;
		if (hlon<minlon) hlon+=360.0;
		if (hlon>maxlon) hlon-=360.0;
		if (hlon<minlon || hlon>maxlon || hlat<minlat || hlat>maxlat) {
		  keepreport=0;
		  if (verb==2) printf("report not in lat/lon domain\n");
		}
		if (keepreport && stn->rflag &&
		    hypot(hlon-minlon,hlat-minlat)>stn->radius) {
		  keepreport=0;
		  if (verb==2) printf("report not within radius of lat/lon location\n");
		}
	      }
	    }
	    
	    /* loop through rfblock to get a data value */
	    if (keepreport) {
	      for (bval=rfblockbeg; bval != val; bval=bval->next) {
		if (bval->undef) continue;

		/* Non-replicated surface report */
		if ((stn->pvar->levels==0) && (bval->z == -1)) {           
		    
		  /* If variable x,y matches, chain report off stn block */
		  if ((dequal(bval->x,stn->pvar->units[0],1e-08)==0) && 
		      (dequal(bval->y,stn->pvar->units[1],1e-08)==0)) {
		    rpt = gaarpt(stn);
		    if (rpt==NULL) {
		      gaprnt(0,"getbufr error: gaarpt returned NULL\n");
		      goto err;
		    }
		    rpt->lat = hlat;
		    rpt->lon = hlon;
		    rpt->lev = stn->pfi->undef;
		    rpt->tim = htim;   
                    rpt->umask = 1;
		    rpt->val = bval->val;
		    for (k=0; k<8; k++) *(rpt->stid+k) = *(rfhdr.stid+k);
		    stn->rnum++;
		    break;   /* quit loop now that we've got non-replicated report */
		  }

		} 
		/* Replicated surface report */
		else if ((stn->pvar->levels==2) && (bval->z != -1)) {    
		  
		  /* If variable x,y matches, chain report off stn block */
		  if ((dequal(bval->x,stn->pvar->units[0],1e-08)==0) && 
		      (dequal(bval->y,stn->pvar->units[1],1e-08)==0)) {
		    rpt = gaarpt(stn);
		    if (rpt==NULL) {
		      gaprnt(0,"getbufr error: gaarpt returned NULL\n");
		      goto err;
		    }
		    rpt->lat = hlat;
		    rpt->lon = hlon;
		    rpt->lev = stn->pfi->undef;
		    rpt->tim = htim;   
                    rpt->umask = 1;
		    rpt->val = bval->val;
		    for (k=0; k<8; k++) *(rpt->stid+k) = *(rfhdr.stid+k);
		    stn->rnum++;
		  }
		  
		} 
		/* Replicated upper air report */
		else if ((stn->pvar->levels==1) && (bval->z != -1)) {

		  /* check if level is within range */
		  hlev = rfhdr.lev;
		  if (minlev==maxlev) {
		    if (fabs(hlev-minlev)>0.01) {
		      keepreport=0;
		      if (verb==2) printf("report level doesn't match\n");
		    }
		  } else {
		    if (hlev<minlev || hlev>maxlev) {
		      keepreport=0;
		      if (verb==2) printf("report level is out of range\n");
		    }
		  }
		  if (keepreport) {
		    /* If variable x,y matches, chain report off stn block */
		  if ((dequal(bval->x,stn->pvar->units[0],1e-08)==0) && 
		      (dequal(bval->y,stn->pvar->units[1],1e-08)==0)) {
		      rpt = gaarpt (stn);
		      if (rpt==NULL) {
			gaprnt(0,"getbufr error: gaarpt returned NULL\n");
			goto err;
		      }
		      rpt->lat = hlat;
		      rpt->lon = hlon;
		      rpt->lev = hlev;
		      rpt->tim = htim;
                      rpt->umask = 1;  
		      rpt->val = bval->val;
		      for (k=0; k<8; k++) *(rpt->stid+k) = *(rfhdr.stid+k);
		      stn->rnum++;
		    }
		  }
		}  /* Matches  if (stn->pvar->levels==0) { ... } else {   */
	      }  /* Matches  for (bval=rfblockbeg; bval != val; bval=bval->next) {  */
	    }  /* Matches  if (keepreport) {  */

	    /* If we've gotten here then we've reached the end of the subset */
	    if (!val) break; 

	    /* reset markers */
	    rfblockbeg = val;  
	    rfold = val->z;
	  }
	  val = val->next;
	} /* end of while loop */
      }   /* end of loop through message subsets */
    }     /* end of loop through messages */
  }  /* Matches  for (f=minf; f<=maxf; f++) {  */
  
  stn->rpt = sortrpt(stn->rpt);
  return(0);

err:
  for (i=0; i<BLKNUM; i++) {
    if (stn->blks[i] != NULL) free (stn->blks[i]);
  }
  return (1);
}

void getbufrhdr (gabufr_val *first, gabufr_val *last, struct bufrinfo *info, 
		 struct bufrhdr *hdr, gaint flag, gaint *toffneg) {
  gaint k,toffhr;
  char bigstr[256];
  gadouble pval;
  gadouble tofffrac;
  
  gabufr_val *val;

  for (val = first; val != last; val = val->next) {    
    if (!val) break;
    if (val->undef) continue; 
    /* flag should be 0 for subsets, 1 for rfblocks */
    if ((!flag) && (val->z != -1)) continue; 
    
    /* YEAR */
    if (val->x == info->base.yrxy[0] && val->y == info->base.yrxy[1]) {
      if (val->sval == NULL) hdr->tvals.yr = (gaint)val->val;
    }
    if (val->x == info->offset.yrxy[0] && val->y == info->offset.yrxy[1]) {
      if (val->sval == NULL) hdr->toffvals.yr = (gaint)val->val;
    }
    /* MONTH */
    if (val->x == info->base.moxy[0] && val->y == info->base.moxy[1]) {
      if (val->sval == NULL) hdr->tvals.mo = (gaint)val->val;
    }
    if (val->x == info->offset.moxy[0] && val->y == info->offset.moxy[1]) {
      if (val->sval == NULL) hdr->toffvals.mo = (gaint)val->val;
    }
    /* DAY */
    if (val->x == info->base.dyxy[0] && val->y == info->base.dyxy[1]) {
      if (val->sval == NULL) hdr->tvals.dy = (gaint)val->val;
    }
    if (val->x == info->offset.dyxy[0] && val->y == info->offset.dyxy[1]) {
      if (val->sval == NULL) hdr->toffvals.dy = (gaint)val->val;
    }
    /* HOUR */
    if (val->x == info->base.hrxy[0] && val->y == info->base.hrxy[1]) {
      if (val->sval == NULL) hdr->tvals.hr = (gaint)val->val;
    }
    if (val->x == info->offset.hrxy[0] && val->y == info->offset.hrxy[1]) {
      if (val->sval == NULL) {
	/* If offset is negative, trip flag and then use absolute value */
	if (val->val < 0) *toffneg = 1; 
	pval = fabs(val->val); 
	/* If offset contains fractional hours, update minutes too */
	toffhr   = (gaint)pval;
	tofffrac = pval - toffhr;
	hdr->toffvals.hr = toffhr;
	hdr->toffvals.mn = (gaint)(0.5+(tofffrac*60.0));
      }
    }
    /* MINUTE */
    if (val->x == info->base.mnxy[0] && val->y == info->base.mnxy[1]) {
      if (val->sval == NULL) hdr->tvals.mn = (gaint)val->val;
    }
    if (val->x == info->offset.mnxy[0] && val->y == info->offset.mnxy[1]) {
      if (val->sval == NULL) hdr->toffvals.mn = (gaint)val->val;   
    }
    /* SECONDS */
    if (val->x == info->base.scxy[0] && val->y == info->base.scxy[1]) {
      if (val->sval == NULL) hdr->sec = (gaint)val->val;
    }
    if (val->x == info->offset.scxy[0] && val->y == info->offset.scxy[1]) {
      if (val->sval == NULL) hdr->offsec = (gaint)val->val;
    }
    /* STATION ID */
    if (val->x == info->stidxy[0] && val->y == info->stidxy[1]) {
      if (val->sval != NULL) {  
	/* copy string */
	for (k=0; k<8; k++) {
	  if (*(val->sval+k) == '\0') break;
	  *(hdr->stid+k) = *(val->sval+k);
	}
	/* pad with spaces */
	while (k<8) {
	  *(hdr->stid+k) = ' ';
	  k++;
	}
      } else {                  
	snprintf(bigstr,255,"%-10d",(gaint)val->val);
	for (k=0; k<8; k++) *(hdr->stid+k) = *(bigstr+k);
      }
    }
    /* LATITUDE */
    if (val->x == info->latxy[0] && val->y == info->latxy[1]) {
      if (val->sval == NULL) hdr->lat = val->val;
    }
    /* LONGITUDE */
    if (val->x == info->lonxy[0] && val->y == info->lonxy[1]) {
      if (val->sval == NULL) hdr->lon = val->val;
    }
    /* LEVEL */
    if (val->x == info->levxy[0] && val->y == info->levxy[1]) {
      if (val->sval == NULL) hdr->lev = val->val;
    }
  }
}

/* 
 * Code for sorting a linked list of station reports so they are 
 * in increasing time order. The algorithm used is Mergesort.
 * The sort function returns the new head of the list. 
 * 
 * This code is copyright 2001 Simon Tatham.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL SIMON TATHAM BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
struct garpt * sortrpt(struct garpt *list) {
  struct garpt *p, *q, *e, *tail;
  gaint insize, nmerges, psize, qsize, i;

  if (!list) return NULL;
  insize = 1;
  while (1) {
    p = list;
    list = NULL;
    tail = NULL;
    nmerges = 0;  /* count number of merges we do in this pass */
    while (p) {
      nmerges++;  /* there exists a merge to be done */
      /* step `insize' places along from p */
      q = p;
      psize = 0;
      for (i = 0; i < insize; i++) {
	psize++;
	q = q->rpt;
	if (!q) break;
      }

      /* if q hasn't fallen off end, we have two lists to merge */
      qsize = insize;

      /* now we have two lists; merge them */
      while (psize > 0 || (qsize > 0 && q)) {

	/* decide whether next rpt of merge comes from p or q */
	if (psize == 0) {   
	  /* p is empty; e must come from q. */
	  e = q; 
	  q = q->rpt; 
	  qsize--;
	} else if (qsize == 0 || !q) {	
	  /* q is empty; e must come from p. */
	  e = p; 
	  p = p->rpt; 
	  psize--;
	} else if ((p->tim - q->tim) <= 0) {
	  /* First rpt of p is lower (or same); e must come from p. */
	  e = p; 
	  p = p->rpt; 
	  psize--;
	} else {
	  /* First garpt of q is lower; e must come from q. */
	  e = q; 
	  q = q->rpt; 
	  qsize--;
	}

	/* add the next rpt to the merged list */
	if (tail) {
	  tail->rpt = e;
	} else {
	  list = e;
	}
	tail = e;
      }

      /* now p has stepped `insize' places along, and q has too */
      p = q;
    }
    tail->rpt = NULL;

    /* If we have done only one merge, we're finished. */
    if (nmerges <= 1)   /* allow for nmerges==0, the empty list case */
      return list;

    /* Otherwise repeat, merging lists twice the size */
    insize *= 2;
  }
}
