/* Copyright (C) 1988-2020 by George Mason University. See file COPYRIGHT for more information. */

/*  Values output into the GRIB1 map file:
     Header:
     hipnt info: 0 - version number
                 1 - size of time axis
                 2 - number of records per time
                 3 - Grid type
                     255: user defined grid. 1 record per grid.
                     29: predefined grid set 29 and 30. 2 records per grid.
		 4 - size of off_t index array  (added for version 4)
		 5 - size of ensemble axis  (added for version 5)
     hfpnt info:  None
     Indices:
     intpnt info (for each mapped grib record) :
                 0 - position of start of data in file
                 1 - position of start of bit map in file
                 2 - number of bits per data element
     fltpnt info :
                 0 - decimal scale factor for this record
                 1 - binary scale factor
                 2 - reference value

*/

#ifdef HAVE_CONFIG_H
#include "config.h"

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#else /* undef HAVE_CONFIG_H */
#include <malloc.h>
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "grads.h"
#include "gagmap.h"
#if GRIB2
#include "grib2.h"
void gaseekgb(FILE *, off_t, g2int, off_t *, g2int *);
#endif


/* global variables */
extern struct gamfcmn mfcmn;
struct dt rtime;   /* time based on input map */
struct dt ftime;   /* time based on dd file */
static off_t flen;
gaint ng1elems=3;
gaint ng2elems=2;

/*  Routine to scan a grib1 or grib2 file and output an index (map) file. */

gaint gribmap (void) {

#if GRIB2
 unsigned char *cgrib=NULL;
 g2int  listsec0[3],listsec1[13],numlocal,numfields,n;
 g2int  unpack=0,expand=0,lgrib;
 off_t iseek,lskip;
 gribfield  *gfld;
 size_t  lengrib;
 struct gag2indx *g2indx;
#endif
 char *ch=NULL;
 gaint ret,ierr,flag,rcgr,record;
 gaint rc,i,t,e,ee,r,tmin=0,tmax=0,told,tcur,fnum,didmatch=0;
 gaint sp,sp2,ioff,eoff,tstart,estart,it;
 gaint oldver,newver,oldtrecs=0,oldtsz=0,oldesz=0,oldbigflg,oldeoff,oldioff,oldi;
 struct gafile *pfi;
 struct dt dtim,dtimi;
 struct gaens *ens;
 struct gaindxb indxbb;

 /* initialize a few things */
 mfile=NULL;
 pindxb = &indxbb;

 /* Get the descriptor file name */
 if (ifile==NULL) {
   printf ("\n");
   cnt = nxtcmd (cmd,"Enter name of Data Descriptor file: ");
   if (cnt==0) return(1);
   getwrd(crec,cmd,250);
   ifile = crec;
 }
 
 /* Allocate memory for gafile structure */
 pfi = getpfi();
 if (pfi==NULL) {
   printf ("gribmap: ERROR! unable to allocate memory for gafile structure\n"); return(1);
 }

 /* Parse the descriptor file */
 if (update | upgrade | downgrade) {
   /* if updating, upgrading, or downgrading ... read the index file too */
   rc = gaddes (ifile, pfi, 1);
   if (rc) printf("gribmap: ERROR! unable to parse the data descriptor file or the existing index file\n");
 }
 else {
   rc = gaddes (ifile, pfi, 0);
   if (rc) printf("gribmap: ERROR! unable to parse the data descriptor file\n");
 }
 if (rc) return(1);

 /* Make sure this is a GRIB dataset */
 if (pfi->idxflg!=1 && pfi->idxflg!=2) {
   printf ("gribmap: ERROR! data descriptor file is not for GRIB data\n"); return(1);
 } 

 /* Upgrade or Downgrade the version of the index file */
 if (upgrade || downgrade) {

   /* * * GRIB1 index file upgrade/downgrade * * */
   if (pfi->idxflg==1) {
     if (upgrade && pfi->pindx->type == g1ver) {
       printf("gribmap: GRIB1 index file version is already up to date\n"); return (0);
     }
     if (downgrade)
       if (pfi->pindx->type==4 || (pfi->pindx->type==5 && *(pfi->pindx->hipnt+4)>0)) {
       printf("gribmap: GRIB1 index file was created with the \"-big\" option and cannot be downgraded \n"); 
       return (0);
     }
     if (upgrade)
       printf("gribmap: upgrading GRIB1 index file from version %d to %d\n",pfi->pindx->type,g1ver);
     else
       printf("gribmap: downgrading GRIB1 index file from version %d to 1\n",pfi->pindx->type);
     
     /* Set up a new gaindx structure and copy info from pfi->pindx */
     if ((pindx = (struct gaindx *)galloc(sizeof(struct gaindx),"pindxgm"))==NULL) {
       printf ("gribmap: ERROR! malloc failed for new pindx structure\n"); return(1);
     }
     if (upgrade) {
       pindx->type  = g1ver; 
       pindx->hinum = 6;     /* new # of ints in the header */
     }
     else {
       pindx->type  = 1;     /* the original, an oldie but goodie */
       pindx->hinum = 4;     /* old # of ints in the header */
     }
     pindx->hfnum  = 0;     /* # of floats in the header, always zero */  
     pindx->intnum = pfi->pindx->intnum;
     pindx->fltnum = pfi->pindx->fltnum;
     
     /* allocate memory for new arrays */
     if ((pindx->hipnt = (gaint *)galloc(sizeof(gaint)*pindx->hinum,"hipntgm"))==NULL) {
       printf ("gribmap: ERROR! malloc failed for new hipnt array\n"); return(1);
     }
     if ((pindx->intpnt = (gaint *)galloc(sizeof(gaint)*pindx->intnum,"intpntgm"))==NULL) {
       printf ("gribmap: ERROR! malloc failed for new intpnt array\n"); return(1);
     }
     if ((pindx->fltpnt = (gafloat *)galloc(sizeof(gafloat)*pindx->fltnum,"fltpntgm"))==NULL) {
       printf ("gribmap: ERROR! malloc failed for new fltpnt array\n"); return(1);
     }
     pindxb->bignum = 0;    /* assume big file offsets are not in use */
     if (upgrade) {
       if (pfi->pindx->type==4) {
	 pindxb->bignum = pfi->pindxb->bignum;
	 if ((pindxb->bigpnt = (off_t *)galloc(sizeof(off_t)*pindxb->bignum,"bigpntgm"))==NULL) {
	   printf ("gribmap: ERROR! malloc failed for new bigpnt array\n"); return(1);
	 }
       }
     }

     /* set the values of the header integers */
     if (upgrade)
       *(pindx->hipnt+0) = g1ver;
     else
       *(pindx->hipnt+0) = 1;
     *(pindx->hipnt+1) = pfi->dnum[3];
     *(pindx->hipnt+2) = pfi->trecs;
     *(pindx->hipnt+3) = pfi->grbgrd;
     if (pfi->grbgrd<-900) *(pindx->hipnt+3) = 255;
     if (upgrade) {
       if (pfi->pindx->type==4) 
	 *(pindx->hipnt+4) = pfi->pindxb->bignum;
       else                     
	 *(pindx->hipnt+4) = 0;   
       *(pindx->hipnt+5) = pfi->dnum[4];
     }

     /* copy the index arrays */
     for (i=0; i<pindx->intnum; i++) *(pindx->intpnt+i) = *(pfi->pindx->intpnt+i);
     for (i=0; i<pindx->fltnum; i++) *(pindx->fltpnt+i) = *(pfi->pindx->fltpnt+i); 
     if (upgrade) {
       if (pfi->pindx->type==4) 
	 for (i=0; i<pindxb->bignum; i++) *(pindxb->bigpnt+i) = *(pfi->pindxb->bigpnt+i); 
     }

     /* write out the new index file */
     rc = wtg1map(pfi,pindx,pindxb);
     return (rc);
   } /* end of GRIB1 index file upgrade/downgrade */

#if GRIB2
   /* * * GRIB2 index file upgrade/downgrade * * */
   else {
     if (upgrade && pfi->g2indx->version == g2ver) {
       printf("gribmap: GRIB2 index file version is already up to date\n"); return (0);
     }
     if (downgrade)
       if (pfi->g2indx->version==2 || (pfi->g2indx->version==3 && pfi->g2indx->bigflg)) {
       printf("gribmap: GRIB2 index file was created with the \"-big\" option and cannot be downgraded \n"); 
       return (0);
     }
     if (upgrade)
       printf("gribmap: upgrading GRIB2 index file from version %d to %d\n",pfi->g2indx->version,g2ver);
     else
       printf("gribmap: downgrading GRIB2 index file from version %d to 1\n",pfi->g2indx->version);
     
     /* Set up new g2index structure and copy info from pfi->g2indx */
     if ((g2indx = (struct gag2indx *)malloc(sizeof(struct gag2indx)))==NULL) {
       printf ("gribmap: ERROR! malloc failed for new g2indx\n"); return(1);
     }
     if (upgrade)
       g2indx->version = g2ver; 
     else
       g2indx->version = 1; 
     g2indx->g2intnum = pfi->g2indx->g2intnum;
     g2indx->g2intpnt = NULL;

     if (upgrade) {
       if (pfi->g2indx->version==2)
	 g2indx->bigflg = 1;
       else 
	 g2indx->bigflg = 0;
       g2indx->trecs = pfi->trecs; 
       g2indx->tsz = pfi->dnum[3];
       g2indx->esz = pfi->dnum[4];
       g2indx->g2bigpnt = NULL;
     }
     
     /* allocate memory and copy index arrays */
     if ((g2indx->g2intpnt = (gaint *)malloc(sizeof(gaint)*g2indx->g2intnum))==NULL) {
       printf ("gribmap: ERROR! malloc failed for new g2intpnt array\n"); return(1);
     }
     for (i=0; i<g2indx->g2intnum; i++) *(g2indx->g2intpnt+i) = *(pfi->g2indx->g2intpnt+i);
     if (upgrade) {
       if (g2indx->bigflg) {
	 if ((g2indx->g2bigpnt = (off_t *)malloc(sizeof(off_t)*g2indx->g2intnum))==NULL) {
	   printf ("gribmap: ERROR! malloc failed for new g2bigpnt array\n"); return(1);
	 }
	 for (i=0; i<g2indx->g2intnum; i++) *(g2indx->g2bigpnt+i) = *(pfi->g2indx->g2bigpnt+i);
       }
     }

     /* Write out the new index file */
     rc = wtg2map(pfi,g2indx);
     return (rc);
   } /* end of GRIB2 index file upgrade */
#endif
 } /* end of upgrade/downgrade code */


 /* If updating the index file, check info in the existing file */
 if (update) {

   if (pfi->idxflg==1 && pfi->grbgrd==29) {
     printf("gribmap: ERROR! Index files for grbgrd 29 cannot be updated.\n"); return(1);
   }
   /* make sure the existing map will have the metadata we need */
   if (pfi->idxflg==1) {
     oldver = pfi->pindx->type;
     newver = g1ver;
   }
#if GRIB2
   else {
     oldver = pfi->g2indx->version;
     newver = g2ver;
   }
#endif
   if (oldver < newver) {
     printf("gribmap: ERROR! Existing index file is an old version and cannot be updated.\n");
     printf("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n");
     printf("* To use the update feature without having to re-scan all files, follow these steps:  *\n");
     printf("*   1. Edit your descriptor file so that the TDEF and EDEF entries match the grid     *\n"); 
     printf("*      dimensions when the existing index file was created (i.e., undo your update.)  *\n");
     printf("*   2. Run gribmap with the \"-new\" option to upgrade the version of the index file.   *\n");
     printf("*   3. Re-implement the changes to the TDEF or EDEF entries in your descriptor file   *\n");
     printf("*      that expand the grid dimensions. Only one dimension can be updated at a time.  *\n");
     printf("*   4. Run gribmap with the -u option.                                                *\n");
     printf("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n");
     return(1);
   }
   /* get metadata from existing index */
   if (pfi->idxflg==1) {
     oldtrecs = *(pfi->pindx->hipnt+2);
     oldtsz = *(pfi->pindx->hipnt+1);
     oldesz = *(pfi->pindx->hipnt+5);
     oldbigflg = *(pfi->pindx->hipnt+4);
   }
#if GRIB2
   else {
     oldtrecs = pfi->g2indx->trecs;
     oldtsz = pfi->g2indx->tsz;
     oldesz = pfi->g2indx->esz;
     oldbigflg = pfi->g2indx->bigflg;
   }
#endif

   /* compare new and old metadata to see what needs to be updated */
   if (oldtrecs != pfi->trecs) {
     printf("gribmap: ERROR! number of XY grids in existing index file doesn't match descriptor file. \n");
     return(1);
   }
   if (oldtsz != pfi->dnum[3]) {
     if (oldtsz > pfi->dnum[3]) {
       printf("gribmap: ERROR! size of T axis in descriptor file is smaller than in existing index file. \n");
       return(1);
     }
     printf("gribmap: updating the size of the T axis from %d to %d\n",oldtsz,pfi->dnum[3]);
     update++;   /* increment by 1 to update T dimension */
   }
   if (oldesz != pfi->dnum[4]) {
     if (oldesz > pfi->dnum[4]) {
       printf("gribmap: ERROR! Size of E axis in descriptor file is smaller than in existing index file. \n");
       return(1);
     }
     printf("gribmap: updating the size of the E axis from %d to %d\n",oldesz,pfi->dnum[4]);
     update+=2;   /* increment by 2 to update E dimension */
   }
   /* additional checks on the results from comparing E and T sizes */
   /* update=2 : T only  	
      update=3 : E only  
      update=4 : both T and E (not allowed) */
   if (update==1) {
     printf("gribmap: Sizes of T and E axes in descriptor file are the same as in existing index file. \n");
     printf("         Nothing needs to be updated, so a new map file will not be created. \n");
     return(0);
   }
   if (update==4) {
     printf("gribmap: ERRROR! Two dimensions cannot be updated at the same time. \n");
     printf("                 Update one dimension first (it can be T or E), then update the other. \n");
     return(1);
   }
   if (update==3 && pfi->dnum[4]>1 && pfi->tmplat==1) {
     printf("gribmap: ERROR! The E axis cannot be updated when data files are only templated over T. \n");
     return(1);
   }
   /* make sure bigflg matches for existing and new map */
   if (oldbigflg==0 && bigflg>0) {
     printf("gribmap: ERROR! Existing index file is not for large files. \n");
     printf("                If new data files are < 2GB, remove the \"-big\" option. \n");
     printf("                If new data files are >= 2GB, remove the \"-u\" option. \n");
     return(1);
   }
   if (oldbigflg>0 && bigflg==0) {
     printf("gribmap: Warning! Existing index file is for large files; adding \"-big\" option for the update. \n");
     bigflg = 1;
   }
 } /* end of update code common for both GRIB1 and GRIB2 */



 /* * * * * * *
  * * GRIB1 * *
  * * * * * * */
 if (pfi->idxflg==1) {

   /* Allocate memory for gaindx structure */
   if ((pindx = (struct gaindx *)galloc(sizeof(struct gaindx),"pindxgm"))==NULL) {
     printf ("gribmap: ERROR! unable to allocate memory for pindx\n");
     return(1);
   }
   
   /* Save the initial time from the descriptor file for the tau0 option */
   btimdd.yr = *(pfi->abvals[3]);
   btimdd.mo = *(pfi->abvals[3]+1);
   btimdd.dy = *(pfi->abvals[3]+2);
   btimdd.hr = *(pfi->abvals[3]+3);
   btimdd.mn = *(pfi->abvals[3]+4);
   if (no_min) btimdd.mn = 0;
   
   /* Set up for this grid type */
   if (pfi->grbgrd<-900 || pfi->grbgrd==255) {
     nrec = 1;
     gtype[0] = 255;
   } else if (pfi->grbgrd>-1 && pfi->ppflag) {
     nrec=1;
     gtype[0] = pfi->grbgrd;
   } else if (pfi->grbgrd==29) {
     nrec = 2;
     gtype[0] = 29;
     gtype[1] = 30;
     if (pfi->dnum[0]!=144 || pfi->dnum[1]!=73 ||
	 pfi->linear[0]!=1 || pfi->linear[1]!=1 ||
	 *(pfi->grvals[0])!= 2.5 || *(pfi->grvals[0]+1) != -2.5 ||
	 *(pfi->grvals[1])!= 2.5 || *(pfi->grvals[1]+1) != -92.5 ) {
       printf("gribmap: ERROR! grid specification for GRIB grid type 29/30.\n");
       printf("                grid scaling must indicate a 2.5 x 2.5 grid\n");
       printf("                grid size must be 144 x 73\n");
       printf("                grid must go from 0 to 357.5 and -90 to 90\n");
       return(1);
     }
   } else {
     nrec = 1;
     gtype[0] = pfi->grbgrd;
   }
   
   /* Set up grib1 index and initialize values */
   /* nrec is the number of records per grid (usually only 1)
      pfi-trecs is the number of XY grids per time step 
      pfi->dnum[3] is the number of time steps
      pfi->dnum[4] is the number of ensembles */
   pindx->type = g1ver;
   pindx->hfnum  = 0;     /* # of floats in the header, always zero */  
   pindx->hinum  = 6;     /* # of ints in the header */
   if (bigflg) {
     /* factors of 1 and 2 add up to ng1elems */
     pindx->intnum  = 1 * nrec * pfi->trecs * pfi->dnum[3] * pfi->dnum[4];
     pindxb->bignum = 2 * nrec * pfi->trecs * pfi->dnum[3] * pfi->dnum[4];
   }
   else {
     pindx->intnum  = ng1elems * nrec * pfi->trecs * pfi->dnum[3] * pfi->dnum[4];
     pindxb->bignum = 0;
   }
   pindx->fltnum = ng1elems * nrec * pfi->trecs * pfi->dnum[3] * pfi->dnum[4];

   if ((pindx->hipnt = (gaint *)galloc(sizeof(gaint)*pindx->hinum,"hipntgm"))==NULL) {
     printf ("gribmap: ERROR! malloc failed for hipnt array\n"); return(1);
   }
   if ((pindx->intpnt = (gaint *)galloc(sizeof(gaint)*pindx->intnum,"intpntgm"))==NULL) {
     printf ("gribmap: ERROR! malloc failed for intpnt array\n"); return(1);
   }
   if ((pindx->fltpnt = (gafloat *)galloc(sizeof(gafloat)*pindx->fltnum,"fltpntgm"))==NULL) {
     printf ("gribmap: ERROR! malloc failed for fltpnt array\n"); return(1);
   }
   if (bigflg) {
     if ((pindxb->bigpnt = (off_t *)galloc(sizeof(off_t)*pindxb->bignum,"bigpntgm"))==NULL) {
       printf ("gribmap: ERROR! malloc failed for bigpnt array\n"); return(1);
     }
   }
   /* initialize values in the index arrays */
   for (i=0; i<pindx->intnum; i++) *(pindx->intpnt+i) = -999;
   for (i=0; i<pindx->fltnum; i++) *(pindx->fltpnt+i) = -999; 
   if (bigflg) {
      for (i=0; i<pindxb->bignum; i++) *(pindxb->bigpnt+i) = (off_t)-999;
   }
   /* set the values of the 6 header integers */
   *(pindx->hipnt+0) = g1ver;
   *(pindx->hipnt+1) = pfi->dnum[3];
   *(pindx->hipnt+2) = pfi->trecs;
   *(pindx->hipnt+3) = pfi->grbgrd;
   if (pfi->grbgrd<-900) *(pindx->hipnt+3) = 255;
   *(pindx->hipnt+4) = pindxb->bignum;
   *(pindx->hipnt+5) = pfi->dnum[4];

   if (update) {
     /* copy the existing index data into new index buffer */
     for (e=0; e < oldesz; e++ ) {
       for (t=0; t < oldtsz; t++) {
	 oldeoff = e * oldtrecs * oldtsz;
	 oldioff = t * oldtrecs;
	 eoff = e * pfi->trecs * pfi->dnum[3];
	 ioff = t * pfi->trecs;

	 /* when updating a grib1 index file, nrec is always 1 and joff is always 0.
	    nrec=2 and joff>0 are for grbgrd==29, but we aren't updating that ancient format.
	    in gribfill() subroutine, the file offset koff = nrec*ng1elems*(eoff+ioff)+joff 
	    but here it is just ng1elems*(eoff+ioff)  */
	 if (bigflg) {
	   /* copy the int array (here 1 means ng1elems-2) */
	   for (r=0; r<1*oldtrecs; r+=1) {
	     oldi = 1*(oldeoff+oldioff);
	     i = 1*(eoff+ioff);
	     *(pindx->intpnt+i+r) = *(pfi->pindx->intpnt+oldi+r);
	   }
	   /* copy the off_t arrays (here 2 means ng1elems-1) */
	   for (r=0; r<2*oldtrecs; r+=2) {
	     oldi = 2*(oldeoff+oldioff);
	     i = 2*(eoff+ioff);
	     *(pindxb->bigpnt+i+r+0) = *(pfi->pindxb->bigpnt+oldi+r+0);
	     *(pindxb->bigpnt+i+r+1) = *(pfi->pindxb->bigpnt+oldi+r+1);
	   }
	 } else {
	   /* copy the int array */
	   for (r=0; r<ng1elems*oldtrecs; r+=ng1elems) {
	     oldi = ng1elems*(oldeoff+oldioff);
	     i = ng1elems*(eoff+ioff);
	     *(pindx->intpnt+i+r+0) = *(pfi->pindx->intpnt+oldi+r+0);
	     *(pindx->intpnt+i+r+1) = *(pfi->pindx->intpnt+oldi+r+1);
	     *(pindx->intpnt+i+r+2) = *(pfi->pindx->intpnt+oldi+r+2);
	   }
	 }
	 /* copy the float array */
	 for (r=0; r<ng1elems*oldtrecs; r+=ng1elems) {
	   oldi = ng1elems*(oldeoff+oldioff);
	   i = ng1elems*(eoff+ioff);
	   *(pindx->fltpnt+i+r+0) = *(pfi->pindx->fltpnt+oldi+r+0);
	   *(pindx->fltpnt+i+r+1) = *(pfi->pindx->fltpnt+oldi+r+1);
	   *(pindx->fltpnt+i+r+2) = *(pfi->pindx->fltpnt+oldi+r+2);
	 }
       }
     }
     /* set start points for axes that are being updated */
     if (update==2) {                /* T only */
       tstart = oldtsz;
       estart = 1;
     }
     else if (update==3) {           /* E only */
       tstart = 0; 
       estart = oldesz + 1;
     }
     else {                          /* shouldn't get here, but just in case... */
       tstart=0;
       estart=1;
     }
   }
   else {
     /* start at the beginning when creating a new map file */
     tstart=0;
     estart=1;
   }

   /* if updating the map, advance through the chain of ensemble structures to starting point */
   if (estart>1) {
     ee=1;
     ens=pfi->ens1; 
     while (ee<estart) { ee++; ens++; } 
   }
   else {
     ee=1;
     ens=pfi->ens1; 
   }
   /* Begin looping over all files that need to be scanned in the data set */ 
   gfile = NULL;
   /* Loop over ensembles */
   for (e=ee; e<=pfi->dnum[4]; e++) {
     tcur = tstart;
     /* Loop over all times for this ensemble */
     while (1) {    
       if (pfi->tmplat) {
	 /* make sure no file is open */
	 if (gfile!=NULL) { fclose(gfile); gfile=NULL; }
	 /* advance to first valid time step for this ensemble */
	 if (tcur==0) {
	   told = 0;
	   tcur = 1;
	   while (pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] == -1) tcur++;  
	 }
	 else {  /* tcur!=0 */
	   told = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
	   /* increment time step until fnums changes */
	   while (told==pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] && tcur<=pfi->dnum[3]) tcur++;
	 }
	 /* make sure we haven't advanced past end of time axis */
	 if (tcur>pfi->dnum[3]) break;
	 /* check if we're past all valid time steps for this ensemble */
	 if ((told != -1) && (pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] == -1)) break;
	 /* Find the range of t indexes that have the same fnums value.
	    These are the times that are contained in this particular file */
	 tmin = tcur;
	 tmax = tcur-1;
	 fnum = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
	 if (fnum != -1) {
	   while (fnum == pfi->fnums[(e-1)*pfi->dnum[3]+tmax]) tmax++;
	   gr2t(pfi->grvals[3], (gadouble)tcur, &dtim); 
	   gr2t(pfi->grvals[3], ens->gt, &dtimi);
	   ch = gafndt(pfi->name, &dtim, &dtimi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,tcur,e,&flag);
	   if (ch==NULL) {
	     printf("gribmap: ERROR! unable to determine data file name for e=%d t=%d\n",e,tcur);
	     return(1);
	   }
	 }
       }
       else { 
	 /* Data set is not templated, only one data file to open*/
	 ch = pfi->name;
	 tmin = 1;
	 tmax = pfi->dnum[3];
       }
       
       /* Open this GRIB file and position to start of first record */
       if (!quiet) printf("gribmap: opening GRIB file %s \n",ch);
       gfile = fopen(ch,"rb");
       if (gfile==NULL) {
	 if (pfi->tmplat) {
	   if (!quiet) printf ("gribmap: Warning! could not open GRIB file %s\n",ch);
	   fflush(stdout); continue;
	 } 
	 else {
	   printf ("gribmap: ERROR! could not open GRIB file %s\n",ch);
	   fflush(stdout); return(1);
	 }
       }
       if (pfi->tmplat) gree(ch,"312");
       
       /* Get file size */
       fseeko(gfile,(off_t)0,2);
       flen = ftello(gfile);
       
       /* Set up to skip appropriate amount and position */
       if (skip > -1) {
	 fpos = (off_t)skip;
       }
       else {
	 fseeko (gfile,(off_t)0,0);
	 rc = fread (rec,1,100,gfile);
	 if (rc<100) {
	   printf ("gribmap: ERROR! I/O error reading header\n");
	   return(1);
	 }
	 len = gagby(rec,88,4);
	 fpos = (off_t)(len*2 + 100);
       }
       
       /* Main Loop */
       irec=1;
       while (1) {
	 /* read a grib record */
	 rc = gribhdr(&ghdr);      
	 if (rc) break;
	 /* compare to each 2-d variable in the 5-D data volume
	    defined by the descriptor file for a match */
	 rcgr = gribrec(&ghdr,pfi,pindx,tmin,tmax,e);
	 if (rcgr==0) didmatch=1;
	 if (rcgr>=100) didmatch=rcgr;
	 irec++;
       }
       
       /* see how we did */
       if (rc==50) {
	 printf ("gribmap: ERROR! I/O error reading GRIB file\n");
	 printf ("                possible cause is premature EOF\n");
	 break;
       }
       if (rc>1 && rc!=98) {
	 printf ("gribmap: ERROR! GRIB file format error (rc = %i)\n",rc);
	 return(rc);
       }
       
       /* break out if not templating */
       if (!pfi->tmplat) break;
       
     } /* end of while (1) loop */
     ens++;
   } /* end of loop over ensemble members: for (e=1; e<=pfi->dnum[4]; e++) */
   
   if (!quiet) printf ("gribmap: reached end of files\n");
   
   /* check if file closed already for case where template was set,
      but it was not templated and the template code above closed it. */
   if (gfile!=NULL) {
     fclose (gfile);
     gfile=NULL;
   }

   /* Write out the index file */
   if (write_map) {
     rc = wtg1map(pfi,pindx,pindxb);
     if (rc) return (rc);
   }
   return (didmatch);

 }  /* end of GRIB1 handling */

#if GRIB2
 /* * * * * * *
  * * GRIB2 * *
  * * * * * * */
 else {

   /* Set up new g2index structure and initialize values */
   g2indx = (struct gag2indx *)malloc(sizeof(struct gag2indx));
   if (g2indx==NULL) {
     printf ("gribmap: ERROR! unable to allocate memory for g2indx\n");
     fflush(stdout);
     return(1);
   }
   g2indx->version = g2ver; 
   /* ng2elems is 2: fieldnum and file position are written into the map file for each record. 
      Without bigflg, both of these numbers are integers;
        g2intnum gets a factor of 2 (ng2elems) because the integer array contains both numbers. 
      When bigflg is set, fieldnum is an integer, and fileposition is an off_t (large files);
        g2intnum has a factor of 1 (ng2elems-1) because the integer array contains only fieldnum
        and a separate array (g2bigpnt) is written out as off_t with file positions. */
   if (bigflg) {
     g2indx->g2intnum = (ng2elems-1) * pfi->trecs * pfi->dnum[3] * pfi->dnum[4];
   } else {
     g2indx->g2intnum = ng2elems * pfi->trecs * pfi->dnum[3] * pfi->dnum[4];
   }
   g2indx->bigflg = bigflg;
   g2indx->trecs = pfi->trecs; 
   g2indx->tsz = pfi->dnum[3];
   g2indx->esz = pfi->dnum[4];
   g2indx->g2intpnt = NULL;
   g2indx->g2bigpnt = NULL;

   /* allocate memory and intialize index arrays */
   g2indx->g2intpnt = (gaint *)malloc(sizeof(gaint)*g2indx->g2intnum);
   if (g2indx->g2intpnt==NULL) {
     printf ("gribmap: ERROR! unable to allocate memory for g2indx->g2intpnt\n");
     fflush(stdout); goto err;
   }
   for (i=0; i<g2indx->g2intnum; i++) g2indx->g2intpnt[i] = -999;
   if (bigflg) {
     g2indx->g2bigpnt = (off_t *)malloc(sizeof(off_t)*g2indx->g2intnum);
     if (g2indx->g2bigpnt==NULL) {
       printf ("gribmap: ERROR! unable to allocate memory for g2indx->g2bigpnt\n");
       fflush(stdout); goto err;
     }
     for (i=0; i<g2indx->g2intnum; i++) g2indx->g2bigpnt[i] = (off_t)-999;
   }

   if (update) {
     /* copy the existing index data into new index buffer */
     for (e=0; e < oldesz; e++ ) {
       for (t=0; t < oldtsz; t++) {
	 oldeoff = e * oldtrecs * oldtsz;
	 oldioff = t * oldtrecs;
	 eoff = e * g2indx->trecs * g2indx->tsz;
	 ioff = t * g2indx->trecs;
	 if (bigflg) {
	   for (r=0; r<(ng2elems-1)*oldtrecs; r+=(ng2elems-1)) {
	     oldi = (ng2elems-1)*(oldeoff+oldioff);
	     i = (ng2elems-1)*(eoff+ioff);
	     *(g2indx->g2bigpnt+i+r) = *(pfi->g2indx->g2bigpnt+oldi+r);
	     *(g2indx->g2intpnt+i+r) = *(pfi->g2indx->g2intpnt+oldi+r);
	   }
	 } else {
	   for (r=0; r<ng2elems*oldtrecs; r+=ng2elems) {
	     oldi = ng2elems*(oldeoff+oldioff);
	     i = ng2elems*(eoff+ioff);
	     *(g2indx->g2intpnt+i+r+0) = *(pfi->g2indx->g2intpnt+oldi+r+0);
	     *(g2indx->g2intpnt+i+r+1) = *(pfi->g2indx->g2intpnt+oldi+r+1);
	   }
	 }
       }
     }
     /* set start points for axes that are being updated */
     if (update==2) {                /* T only */
       tstart = oldtsz;
       estart = 1;
     }
     else if (update==3) {           /* E only */
       tstart = 0; 
       estart = oldesz + 1;
     }
     else {                          /* shouldn't get here, but just in case... */
       tstart=0;
       estart=1;
     }
   }
   else {
     /* start at the beginning when creating a new map file */
     tstart=0;
     estart=1;
   }

   /* Break out point for case with E>1 but data files are only templated over T 
      (all ensemble members are in one file) */
   if (pfi->dnum[4]>1 && pfi->tmplat==1) {
     /* initialize a few things */
     gfile = NULL;
     e = 1;
     ens = pfi->ens1; 
     tcur = tstart;

     /* Loop over all files in the data set */ 
     while (1) {      
       if (gfile!=NULL) { fclose(gfile); gfile=NULL; }  /* make sure no file is open */
       if (tcur==0) { /* first time step */
	 told = 0;
	 tcur = 1;
       }
       else {  /* tcur!=0 */
	 told = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
	 /* increment time step until fnums changes */
	 while (told==pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] && tcur<=pfi->dnum[3]) tcur++;
       }
       /* make sure we haven't advanced past end of time axis */
       if (tcur>pfi->dnum[3]) break;
              
       /* Find the range of t indexes that have the same fnums value.
	  These are the times that are contained in this particular file */
       tmin = tcur;
       tmax = tcur-1;
       fnum = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
       if (fnum != -1) {
	 while (fnum == pfi->fnums[(e-1)*pfi->dnum[3]+tmax]) tmax++;
	 gr2t(pfi->grvals[3], (gadouble)tcur, &dtim); 
	 gr2t(pfi->grvals[3], ens->gt, &dtimi);
	 ch = gafndt(pfi->name, &dtim, &dtimi, pfi->abvals[3], pfi->pchsub1, pfi->ens1, tcur, e, &flag);
	 if (ch==NULL) {
	   printf("gribmap: ERROR! couldn't determine data file name for e=%d t=%d\n",e,tcur);
	   fflush(stdout); goto err;
	 }
       }

       /* Open this GRIB file and position to start of first record */
       if (!quiet) printf("gribmap: scanning GRIB2 file: %s \n",ch);
       fflush(stdout);
       gfile = fopen(ch,"rb");
       if (gfile==NULL) {
	 if (!quiet) printf ("gribmap: Warning! could not open GRIB file: %s\n",ch);
	 fflush(stdout); continue;
       }
       gree(ch,"f311a");

       /* Loop over fields in the grib file and find matches */
       iseek = (off_t)0;
       record = 1;
       while (1) {
	 /* move to next grib message in file */
	 gaseekgb(gfile,iseek,32000,&lskip,&lgrib);
	 if (lgrib == 0) break;    /* end loop at EOF or problem */
	 
	 /* read the message into memory */
	 sz = lgrib;
	 cgrib = (unsigned char *)galloc(sz,"cgrib2");
	 if (cgrib == NULL) {
	   printf("gribmap: ERROR! unable to allocate memory for record %d at byte %jd\n",record,(intmax_t)iseek); 
	   fflush(stdout); goto err;
	 }
	 ret = fseeko(gfile,lskip,SEEK_SET);
	 lengrib = fread(cgrib,sizeof(unsigned char),lgrib,gfile);
	 if (lengrib < lgrib) {
	   printf("gribmap: ERROR! unable to read record %d at byte %jd\n",record,(intmax_t)iseek); 
	   fflush(stdout); goto err;
	 }

         /* Check for ultra long length -- which we do not yet handle */
         if (gagby(cgrib,8,4)!=0 || gagbb(cgrib+12,0,1)!=0) {
	   printf("gribmap: ERROR! grib2 record too long! record %d at byte %jd\n",record,(intmax_t)iseek); 
	   fflush(stdout); goto err;
         }
	 
	 /* Get info about grib2 message */
	 ierr = 0;
	 ierr = g2_info(cgrib,listsec0,listsec1,&numfields,&numlocal);
	 if (ierr) {
	   printf("gribmap: ERROR! g2_info failed: ierr=%d\n",ierr); 
	   fflush(stdout); goto err;
	 }
	 for (n=0; n<numfields; n++) {
	   ierr = 0;
	   ierr = g2_getfld(cgrib,n+1,unpack,expand,&gfld);
	   if (ierr) {
	     printf("gribmap: ERROR! g2_getfld failed: ierr=%d\n",ierr); 
	     fflush(stdout); goto err;
	   }
	   
	   /* get statistical process type from grib field */
	   sp = g2sp(gfld);
	   sp2 = g2sp2(gfld);
	   
	   /* print out useful codes from grib2 field */
	   if (verb) g2prnt(gfld,record,n+1,sp,sp2);
	   
	   /* Check grid properties */
	   rc = g2grid_check(gfld,pfi,record,n+1);
	   if (rc) {
	     if (verb) printf("\n");
	     fflush(stdout);
	     g2_free(gfld);   	   
	     break; 
	   }
	   
	   /* Check time values in grib field */
	   it = g2time_check(gfld,listsec1,pfi,record,n+1,tmin,tmax);
	   if (it==-99) {
	     if (verb) printf("\n");
	     fflush(stdout);
	     g2_free(gfld);   	   
	     break;
	   }
	   it = (it-1)*pfi->trecs;  /* (it-1)*number of records per time */
	   
	   /* Check if the variable is a match */
	   ioff = g2var_match(gfld,pfi,sp,sp2);
	   if (ioff==-999) {
	     if (verb) printf("\n");
	     fflush(stdout);
	     g2_free(gfld);   	   
	     break;
	   }
	   
	   /* check if any ensemble codes match */
	   ee = g2ens_match(gfld,pfi);
	   if (ee==-999) {
	     if (verb) printf("\n");
	     fflush(stdout);
	     g2_free(gfld);   	   
	     break;
	   }
	   eoff = (ee-1)*pfi->dnum[3]*pfi->trecs;  /* (ee-1)*number of records per ensemble */
	   
	   /* fill in the gribmap entry */
	   if (verb) printf("  MATCH \n");
	   fflush(stdout);
	   g2fill (eoff, it+ioff, ng2elems, iseek, n+1, g2indx);
	   g2_free(gfld); 
	 }
	 /* free memory containing grib record */
	 gree(cgrib,"f310");
	 cgrib=NULL;
	 record++;                    /* increment grib record counter */
	 iseek = lskip+(off_t)lgrib;  /* increment byte offset to next grib msg in file */
       }  /* end of while(1) loop over all fields in the grib message*/

     } /* end of while loop over all times */
     
   }
   else {
     /* Begin handling for all other data sets */

     /* If updating the map, advance through chain of ensemble structures to starting point */
     if (estart>1) {
       ee=1;
       ens=pfi->ens1; 
       while (ee<estart) { ee++; ens++; } 
     }
     else {
       ee=1;
       ens=pfi->ens1; 
     }
     /* Begin looping over all files that need to be scanned in the data set */ 
     gfile=NULL;
     /* Loop over ensembles */
     for (e=ee; e<=pfi->dnum[4]; e++) {
       tcur = tstart;
       /* Loop over all times for this ensemble */
       while (1) {  
	 if (pfi->tmplat) {
	   /* make sure no file is open */
	   if (gfile!=NULL) { fclose(gfile); gfile=NULL; }
	   /* advance to first valid time step for this ensemble */
	   if (tcur==0) {
	     told = 0;
	     tcur = 1;
	     while (pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] == -1) tcur++;  
	   }
	   else {  /* tcur!=0 */
	     told = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
	     /* increment time step until fnums changes */
	     while (told==pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] && tcur<=pfi->dnum[3]) tcur++;
	   }
	   /* make sure we haven't advanced past end of time axis */
	   if (tcur>pfi->dnum[3]) break;
	   /* check if we're past all valid time steps for this ensemble */
	   if ((told != -1) && (pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] == -1)) break;
	   /* Find the range of t indexes that have the same fnums value.
	      These are the times that are contained in this particular file */
	   tmin = tcur;
	   tmax = tcur-1;
	   fnum = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
	   if (fnum != -1) {
	     while (fnum == pfi->fnums[(e-1)*pfi->dnum[3]+tmax]) tmax++;
	     gr2t(pfi->grvals[3], (gadouble)tcur, &dtim); 
	     gr2t(pfi->grvals[3], ens->gt, &dtimi);
	     ch = gafndt(pfi->name, &dtim, &dtimi, pfi->abvals[3], pfi->pchsub1, pfi->ens1, tcur, e, &flag);
	     if (ch==NULL) {
	       printf("gribmap: ERROR! Unable to determine data file name for e=%d t=%d\n",e,tcur);
	       fflush(stdout); goto err;
	     }
	   }
	 }
	 else {  
	   /* not templated -- only one data file to open */
	   ch = pfi->name;
	   tmin = 1;
	   tmax = pfi->dnum[3];
	 }
	 
	 /* Open this GRIB file and position to start of first record */
	 if (!quiet) printf("gribmap: scanning GRIB2 file: %s \n",ch);
	 fflush(stdout);
	 gfile = fopen(ch,"rb");
	 if (gfile==NULL) {
	   if (pfi->tmplat) {
	     if (!quiet) printf ("gribmap: Warning! could not open GRIB file: %s\n",ch);
	     fflush(stdout); continue;
	   }
	   else {
	     printf ("gribmap: ERROR! could not open GRIB file: %s\n",ch);
	     fflush(stdout); goto err;
	   }
	 }
	 if (pfi->tmplat) gree(ch,"f311");
	 
	 /* Loop over fields in the grib file and find matches */
	 iseek=(off_t)0;
	 record=1;
	 while (1) {
	   /* move to next grib message in file */
	   gaseekgb(gfile,iseek,32000,&lskip,&lgrib);
	   if (lgrib == 0) break;    /* end loop at EOF or problem */
	   
	   /* read the message into memory */
	   cgrib = (unsigned char *)galloc(lgrib,"cgrib2");
	   if (cgrib == NULL) {
	     printf("gribmap: ERROR! malloc failed for record %d at byte %jd\n",record,(intmax_t)iseek); 
	     fflush(stdout); goto err;
	   }
	   ret = fseeko(gfile,lskip,SEEK_SET);
	   lengrib = fread(cgrib,sizeof(unsigned char),lgrib,gfile);
	   if (lengrib < lgrib) {
	     printf("gribmap: ERROR! unable to read record %d at byte %jd\n",record,(intmax_t)iseek); 
	     fflush(stdout); goto err;
	   }
	   
	   /* Check for ultra long length -- which we do not yet handle */
	   if (gagby(cgrib,8,4)!=0 || gagbb(cgrib+12,0,1)!=0) {
	     printf("gribmap: ERROR! grib2 record length too long! record %d at byte %jd\n",record,(intmax_t)iseek); 
	     fflush(stdout); goto err;
	   }
	   
	   /* Get info about grib2 message */
	   ierr = 0;
	   ierr = g2_info(cgrib,listsec0,listsec1,&numfields,&numlocal);
	   if (ierr) {
	     printf("gribmap: ERROR! g2_info failed: ierr=%d\n",ierr); 
	     fflush(stdout); goto err;
	   }
	   for (n=0; n<numfields; n++) {
	     ierr = 0;
	     ierr = g2_getfld(cgrib,n+1,unpack,expand,&gfld);
	     if (ierr) {
	       printf("gribmap: ERROR! g2_getfld failed: ierr=%d\n",ierr); 
	       fflush(stdout); goto err;
	     }
	     
	     /* get statistical process type from grib field */
	     sp = g2sp(gfld);
	     sp2 = g2sp2(gfld);
	     
	     /* print out useful codes from grib2 field */
	     if (verb) g2prnt(gfld,record,n+1,sp,sp2);
	     
	     /* Check grid properties */
	     rc = g2grid_check(gfld,pfi,record,n+1);
	     if (rc) {
	       if (verb) printf("\n");
	       fflush(stdout);
	       g2_free(gfld);   	   
	       break; 
	     }
	     
	     /* Check time values in grib field */
	     it = g2time_check(gfld,listsec1,pfi,record,n+1,tmin,tmax);
	     if (it==-99) {
	       if (verb) printf("\n");
	       fflush(stdout);
	       g2_free(gfld);   	   
	       break;
	     }
	     it = (it-1)*pfi->trecs;  /* number of records per time */
	     
	     /* Check if the variable is a match */
	     ioff = g2var_match(gfld,pfi,sp,sp2);
	     if (ioff==-999) {
	       if (verb) printf("\n");
	       fflush(stdout);
	       g2_free(gfld);   	   
	       break;
	     }
	     if (pfi->tmplat) {
	       /* make sure grib codes match for this ensemble */
	       rc = g2ens_check(ens,gfld);
	       if (rc==1) {
		 if (verb) printf("\n");
		 fflush(stdout);
		 g2_free(gfld);   	   
		 break;
	       }
	       else ee = e; 
	     } 
	     else {
	       /* check if any ensemble codes match */
	       ee = g2ens_match(gfld,pfi);
	       if (ee==-999) {
		 if (verb) printf("\n");
		 fflush(stdout);
		 g2_free(gfld);   	   
		 break;
	       }
	     }
	     eoff = (ee-1)*pfi->dnum[3]*pfi->trecs;  /* (ee-1)*number of records per ensemble */
	     
	     /* fill in the gribmap entry */
	     if (verb) printf("  MATCH \n");
	     fflush(stdout);
	     g2fill (eoff,it+ioff,ng2elems,lskip,n+1,g2indx);
	     g2_free(gfld); 
	     
	   }
	   /* free memory containing grib record */
	   gree(cgrib,"f310");
	   cgrib=NULL;
	   record++;                     /* increment grib record counter */
	   iseek = lskip+(off_t)lgrib;   /* increment byte offset to next grib msg in file */
	   
	 }  /* end of while(1) loop over all fields in the grib message*/
	 
	 /* break out if not templating -- only need to scan one grib file */
	 if (!pfi->tmplat) goto done;
	 
       } /* end of while(1) loop over all grib files for a given ensemble member*/
       ens++;
     } /* end of loop over ensemble members: for (e=1; e<=pfi->dnum[4]; e++) */
   } /* end of else statement for if (pfi->dnum[4]>1 && pfi->tmplat==1)  */
   
   if (!quiet) printf ("gribmap: reached end of files\n");
   fflush(stdout);


done:   
   /* check if file not closed */
   if (gfile!=NULL) {
     fclose (gfile);
     gfile=NULL;
   }
   
   /* Write out the index file */
   if (write_map) {
     rc = wtg2map(pfi,g2indx);
     if (rc) return (rc);
   }

   return(0);
   
err: 
   if (g2indx) {
     if (g2indx->g2intpnt) gree(g2indx->g2intpnt,"f314");
     if (g2indx->g2bigpnt) gree(g2indx->g2bigpnt,"f314");
     gree(g2indx,"f315");
   }
   if (cgrib) gree(cgrib,"f316");
   return(1);
 }

#endif  /* matches #if GRIB2 */
 return(0);
}



/* Routine to read a GRIB header and process info */

gaint gribhdr (struct grhdr *ghdr) {
 struct dt atim;
 unsigned char rec[50000],*ch,*gds;
 gaint i,len ,rc,sign,mant,longflg=0;
 off_t cpos;
 
 if (fpos+(off_t)50>=flen) return(1);

 /* look for data between records BY DEFAULT */ 
 i = 0;
 fpos += (off_t)i;
 rc = fseeko(gfile,fpos,0);
 if (rc) return(50);
 ch=&rec[0];
 rc = fread(ch,sizeof(char),4,gfile);
 while ((fpos < flen-(off_t)4) && (i < scanlim) && 
	!(*(ch+0)=='G' && 
	  *(ch+1)=='R' &&
	  *(ch+2)=='I' &&
	  *(ch+3)=='B')) {
   fpos++;
   i++;
   rc = fseeko(gfile,fpos,0);
   if (rc) return(50);
   rc = fread(ch,sizeof(char),4,gfile);
   if (rc<4) return(50);
 } 
 
 if (i == scanlim) {
   printf("gribmap: ERROR! GRIB header not found in scanning between records\n");
   printf("                try increasing the value of the -s argument\n");

   if (scaneof) return(98);
   if (scanEOF) return(0);
   return(52);
 } 
 else if (fpos == flen-(off_t)4) {
   if (scaneof) return(98);
   if (scanEOF) return(0);
   return (53);
 }
 
 /* SUCCESS redo the initial read */    
 rc = fseeko(gfile,fpos,0);
 if (rc) return(50);
 rc = fread(rec,1,8,gfile);
 if (rc<8) {
   if (fpos+(off_t)8 >= flen) return(61);
   else return(62);
 }
 
 cpos = fpos;
 ghdr->vers = gagby(rec,7,1);
 if (ghdr->vers>1) {
   printf ("gribmap: ERROR! file is not GRIB version 0 or 1, version number is %i\n",ghdr->vers);
   if (scaneof) return(98);
   return (99);
 }
 
 if (ghdr->vers==0) {
   cpos += (off_t)4;
   rc = fseeko(gfile,cpos,0);
   if (rc) return(50);
 } else {
   ghdr->len = gagby(rec,4,3);
   longflg = 0;
   if (ghdr->len & 0x800000) longflg = 1;
   cpos = cpos + (off_t)8;
   rc = fseeko(gfile,cpos,0);
   if (rc) return(50);
 }
 
 /* Get PDS length, read rest of PDS */
 rc = fread(rec,1,3,gfile);
 if (rc<3) return(50);
 len = gagby(rec,0,3);
 ghdr->pdslen = len;
 cpos = cpos + (off_t)len;
 rc = fread(rec+3,1,len-3,gfile);
 if (rc<len-3) return(50);
 
 /* Get info from PDS */
 ghdr->id = gagby(rec,6,1);
 ghdr->gdsflg = gagbb(rec+7,0,1);
 ghdr->bmsflg = gagbb(rec+7,1,1);
 ghdr->parm = gagby(rec,8,1);
 ghdr->ltyp = gagby(rec,9,1);
 ghdr->level = gagby(rec,10,2);
 ghdr->l1 = gagby(rec,10,1);
 ghdr->l2 = gagby(rec,11,1);
 if (mpiflg) {                 
   /* use initial time from the descriptor file instead of base time from grib header */
   ghdr->btim.yr = *(pfi->abvals[3]);
   ghdr->btim.mo = *(pfi->abvals[3]+1);
   ghdr->btim.dy = *(pfi->abvals[3]+2);
   ghdr->btim.hr = *(pfi->abvals[3]+3);
   ghdr->btim.mn = *(pfi->abvals[3]+4);
   if (no_min) ghdr->btim.mn = 0;
 } else {
   /* get reference (base) time */
   ghdr->btim.yr = gagby(rec,12,1);
   ghdr->btim.mo = gagby(rec,13,1);
   ghdr->btim.dy = gagby(rec,14,1);
   ghdr->btim.hr = gagby(rec,15,1);
   ghdr->btim.mn = gagby(rec,16,1);
   if (no_min) ghdr->btim.mn = 0;
 }
 if (ghdr->btim.hr>23) ghdr->btim.hr = 0;  /* Special for NCAR */
 if (len>24) {
   ghdr->cent = gagby(rec,24,1);
   ghdr->btim.yr = ghdr->btim.yr + (ghdr->cent-1)*100;
 } else {
   ghdr->cent = -999;
   if (!(mpiflg) || !(mfcmn.fullyear)) {
     if (ghdr->btim.yr>49) ghdr->btim.yr += 1900;
     if (ghdr->btim.yr<50) ghdr->btim.yr += 2000;
   }
 }
 ghdr->ftu = gagby(rec,17,1);    /* forecast time unit */
 ghdr->tri = gagby(rec,20,1);    /* time range indicator */
 /* get P1 and P2 */
 if (ghdr->tri==10) {
   /* P1 occupies octets 19 and 20; product valid at reference time + P1 */
   ghdr->p1 = gagby(rec,18,2);
   ghdr->p2 = 0;
 } else {
   ghdr->p1 = gagby(rec,18,1);
   ghdr->p2 = gagby(rec,19,1);
 }
 
 ghdr->fcstt = ghdr->p1;             /* set P1 as forecast time */
 if (ghdr->tri>1 && ghdr->tri<6) 
   ghdr->fcstt = ghdr->p2;           /* product considered valid at reference time + P2 */
 if ((tauave) && (ghdr->tri==3 || ghdr->tri==7)) 
   ghdr->fcstt = ghdr->p1;           /* Valid time for averages is beginning of period, use P1 */

 /* populate a dt structure with the time to add to base time */
 atim.yr=0; atim.mo=0; atim.dy=0; atim.hr=0; atim.mn=0;
 if      (ghdr->ftu== 0) atim.mn = ghdr->fcstt;
 else if (ghdr->ftu== 1) atim.hr = ghdr->fcstt;
 else if (ghdr->ftu== 2) atim.dy = ghdr->fcstt;
 else if (ghdr->ftu== 3) atim.mo = ghdr->fcstt;
 else if (ghdr->ftu== 4) atim.yr = ghdr->fcstt;
 else if (ghdr->ftu==10) atim.hr = ghdr->fcstt*3;   /* 3Hr incr */
 else if (ghdr->ftu==11) atim.hr = ghdr->fcstt*6;   /* 6Hr incr */  
 else if (ghdr->ftu==12) atim.hr = ghdr->fcstt*12;  /* 12Hr incr */
 else if (ghdr->ftu==13) atim.mn = ghdr->fcstt*15;  /* 15-minute incr */
 else if (ghdr->ftu==14) atim.mn = ghdr->fcstt*30;  /* 30-minute incr */
 else ghdr->fcstt = -999;

 /*  if notau != 0 then FORCE the valid DTG to be the base DTG */ 
 if (notau) ghdr->fcstt = -999 ;
 
 /*  add the forecast time to the time of this grib field */
 if (ghdr->fcstt>-900) {
   if (ghdr->tri==7)
     timsub(&(ghdr->btim),&atim);
   else {
   timadd(&(ghdr->btim),&atim);}
   ghdr->dtim.yr = atim.yr;
   ghdr->dtim.mo = atim.mo;
   ghdr->dtim.dy = atim.dy;
   ghdr->dtim.hr = atim.hr;
   ghdr->dtim.mn = atim.mn;
 } else {
   ghdr->dtim.yr = ghdr->btim.yr;
   ghdr->dtim.mo = ghdr->btim.mo;
   ghdr->dtim.dy = ghdr->btim.dy;
   ghdr->dtim.hr = ghdr->btim.hr;
   ghdr->dtim.mn = ghdr->btim.mn;
 }
 if (len>25) {
   ghdr->dsf = (gafloat)gagbb(rec+26,1,15);
   i = gagbb(rec+26,0,1);
   if (i) ghdr->dsf = -1.0*ghdr->dsf;
   ghdr->dsf = pow(10.0,ghdr->dsf);
 } else ghdr->dsf = 1.0;
 
 /* If it is there, get info from GDS */
 if (ghdr->gdsflg) {
   rc = fread(rec,1,3,gfile);
   if (rc<3) return(50);
   len = gagby(rec,0,3);
   ghdr->gdslen = len;
   cpos = cpos + (off_t)len;
   
   /* handle generic grid where the lon/lats are coded from the GDS */
   gds = (unsigned char *)malloc(len+3);
   if (gds==NULL) return(51);
   rc = fread(gds+3,1,len-3,gfile);
   if (rc<len-3) return(50);
   ghdr->gtyp  = gagby(gds,4,1);
   ghdr->gicnt = gagby(gds,6,2);
   ghdr->gjcnt = gagby(gds,8,2);
   ghdr->gsf1  = gagbb(gds+27,0,1);
   ghdr->gsf2  = gagbb(gds+27,1,1);
   ghdr->gsf3  = gagbb(gds+27,2,1);
   free(gds);
 } 
 else ghdr->gdslen = 0;

 /* Get necessary info about BMS if it is there */
 if (ghdr->bmsflg) {
   rc = fread(rec,1,6,gfile);
   if (rc<6) return(50);
   len = gagby(rec,0,3);
   ghdr->bmsflg = len;
   ghdr->bnumr = gagby(rec,4,2);
   ghdr->bpos = (gaint)cpos+6;
   ghdr->lbpos = cpos+(off_t)6;
   cpos = cpos + (off_t)len;
   rc = fseeko(gfile,cpos,0);
 } 
 else ghdr->bmslen = 0;

 /* Get necessary info from data header */
 rc = fread(rec,1,11,gfile);
 if (rc<11) return(50);
 len = gagby(rec,0,3);
 ghdr->bdslen = len;
 if (longflg && len<120) {   /* ecmwf hack for long records */
   ghdr->len = (ghdr->len & 0x7fffff) * 120 - len + 4;
   ghdr->bdslen = ghdr->len - (12 + ghdr->pdslen + ghdr->gdslen + ghdr->bmslen);
 }
 ghdr->iflg = gagbb(rec+3,0,2);
 i = gagby(rec,4,2);
 if (i>32767) i = 32768-i;
 ghdr->bsf = pow(2.0,(gafloat)i);
 
 i = gagby(rec,6,1);
 sign = 0;
 if (i>127) {
   sign = 1;
   i = i - 128;
 }
 mant = gagby(rec,7,3);
 if (sign) mant = -mant;
 ghdr->ref = (gafloat)mant * pow(16.0,(gafloat)(i-70));
 
 ghdr->bnum = gagby(rec,10,1);
 ghdr->dpos = (gaint)cpos+11;
 ghdr->ldpos = cpos+(off_t)11;

 if (ghdr->vers==0) {
   fpos = fpos + 8 + ghdr->pdslen + ghdr->gdslen +
     ghdr->bmslen + ghdr->bdslen;
 } 
 else fpos = fpos + (off_t)ghdr->len;
 
 return(0);
 
}

/* Routine to determine the location of the GRIB record in terms of the GrADS data set
   and fill in the proper values at the proper slot location. */

gaint gribrec (struct grhdr *ghdr, struct gafile *pfi, struct gaindx *pindx, 
	     gaint tmin, gaint tmax, gaint e) {
 gadouble (*conv) (gadouble *, gadouble);
 gadouble z,t,v1,delta;
 struct gavar *pvar;
 gaint i,ioff,iz,it,joff,nsiz,flag,eoff;

 /* Verify that we are looking at the proper grid type */
 joff =0;
 nsiz = nrec * ng1elems ;
 if (ghdr->iflg) {
   if (verb) {
     printf ("GRIB record contains harmonic or complex packing\n");
     printf ("  Record is skipped.\n");
     printf ("  Variable is %i\n",ghdr->parm);
   }
   return(10);
 }
 if (pfi->grbgrd==255 || pfi->grbgrd<-900) {
   if (!ghdr->gdsflg) {
     if (verb) {
       printf ("GRIB record contains pre-defined grid type: "); 
       printf ("GrADS descriptor specifies type 255\n");
       gribpr(ghdr);
     }
     return(20);
   } 
   if ( pfi->ppflag) {
     if ( ghdr->gicnt != 65535 && 
	  ((ghdr->gicnt != pfi->ppisiz) || (ghdr->gjcnt != pfi->ppjsiz)) ) {
       if (verb) {
	 printf ("GRIB grid size does not match descriptor: "); 
	 gribpr(ghdr);
       }
       return(300);
     }
   } else {
     if (ghdr->gicnt != 65535 && 
	 ((ghdr->gicnt != pfi->dnum[0]) || (ghdr->gjcnt != pfi->dnum[1]))) {
       if (verb) {
	 printf ("GRIB grid size does not match descriptor:");
	 gribpr(ghdr);
       }
       return(301);
     }
   }
 } 
 else {
   /* special case for GRIB grid number (dtype grib NNN) == 29 */
   if (pfi->grbgrd==29) {
     if (ghdr->id!=29 && ghdr->id!=30) {
       if (verb) {
	 printf("Record has wrong GRIB grid type: ") ; 
	 gribpr(ghdr);
       }
       return(400);     
     }
     if (ghdr->id==29) joff = ng1elems;
     nsiz = 2 * ng1elems ;
   } else {
     if (ghdr->id != pfi->grbgrd) {
       if (verb) {
	 printf("%s","Record has wrong GRIB grid type: "); 
	 gribpr(ghdr);
       }
       return(401);     
     }
   }
 }
 
 /* Calculate the grid time for this record.  
    If it is non-integer or if it is out of bounds, just return. */

 /* Check for given forecast time, tauoff (the -fhr switch) */
 if (tauflg && (ghdr->ftu==1 && ghdr->fcstt!=tauoff)) {
   if (verb) {
     printf("%s %d","--f-- Forecast Time does not match : ",tauoff);
     gribpr(ghdr);
   }
   return(32);
 }
 
 /* Check if base time in grib record matches initial time in descriptor file (the -t0 switch) */
 if (tau0 &&
     ((ghdr->btim.yr != btimdd.yr) ||
      (ghdr->btim.mo != btimdd.mo) ||
      (ghdr->btim.dy != btimdd.dy) ||
      (ghdr->btim.hr != btimdd.hr) ||
      (ghdr->btim.mn != btimdd.mn))) {
   if (verb) {
     printf("%s","--b-- Base Time does not match Initial Time in DD: "); 
     gribpr(ghdr);
   }
   return(34);
 }

 /* set threshold for time stamp matches */
 v1 = *(pfi->abvals[3]+5);  /* v1 is non-zero if time axis unit is months */
 if (v1>0) 
   delta = 0.36;  /* large for monthly data, ~10 days */
 else 
   delta = 0.01;  /* small for minutes data, the old default */

 /* Check if valid time is within grid time limits */
 t = t2gr(pfi->abvals[3],&(ghdr->dtim));
 if (t<(1.0-delta) || t>((gafloat)(pfi->dnum[3])+delta)) {
   if (verb) {
     printf("%s","----- Time out of bounds: "); 
     gribpr(ghdr);
   }
   return(36);
 }

 /* Check if valid time is an integer (+/- the designated threshold) */
 it = (gaint)(t+0.01);
 if (fabs((gafloat)it - t) > delta) {
   if (verb) {
     printf("----- Time non-integral. %g %g: ",(gafloat)it,t);  
     gribpr(ghdr);
   }
   return(38);
 }

 /* Check if valid time matches range of times for this file  */
 if (it<tmin || it>tmax) {
   if (verb) {
     printf("-%d-- Time out of file limits: ",it);  
     gribpr(ghdr);
   }
   return(39);
 }
 it = (it-1)*pfi->trecs;
 eoff = (e-1)*pfi->dnum[3]*pfi->trecs;  /* number of records per ensemble */

 /* See if we can match up this grid with a variable in the data descriptor file */
 pvar = pfi->pvar1;
 i = 0;
 flag=0;
 while (i<pfi->vnum) {
   if (pvar->levels>0) {      /* multi level data */
     if (dequal(pvar->units[0],ghdr->parm,1e-8)==0 && 
	 dequal(pvar->units[8],ghdr->ltyp,1e-8)==0) {
       /* look for time range indicator match */
       if (pvar->units[10] < -900 || dequal(pvar->units[10],ghdr->tri,1e-8)==0) {
	 conv = pfi->ab2gr[2];
	 z = conv(pfi->abvals[2],ghdr->level);
	 if (z>0.99 && z<((gafloat)(pvar->levels)+0.01)) {
	   iz = (gaint)(z+0.5);
	   /* check if levels match */
	   if (fabs(z-(gafloat)iz) < 0.01) {
	     iz = (gaint)(z+0.5);
	     ioff = pvar->recoff + iz - 1;
	     gribfill (eoff,it+ioff,joff,nsiz,ghdr,pindx);
	     flag=1;
	     i = pfi->vnum + 1;   /* Force loop to stop */
	   }
	 }
       }
     }
   } 
   else {       /* sfc data */
     if (dequal(pvar->units[0],ghdr->parm,1e-8)==0 && dequal(pvar->units[8],ghdr->ltyp,1e-8)==0) {
       if ((pvar->units[10] < -900 && 
	    dequal(pvar->units[9],ghdr->level,1e-8)==0) ||
	   (pvar->units[10] > -900 && 
	    dequal(pvar->units[9],ghdr->l1,1e-8)==0 && dequal(pvar->units[10],ghdr->l2,1e-8)==0) || 
	   (dequal(pvar->units[10],ghdr->tri,1e-8)==0 && dequal(pvar->units[9],ghdr->level,1e-8)==0)) {
	 ioff = pvar->recoff;
	 gribfill (eoff,it+ioff,joff,nsiz,ghdr,pindx);
	 i = pfi->vnum+1;  /* Force loop to stop */
	 flag=1;
       }
     }
   }
   pvar++; i++;
 }
 
 if (flag && verb) printf("!!!!! MATCH: "); 
 if (!flag && verb) printf("..... NOOOO: "); 
 if (verb) gribpr(ghdr); 
 
 return (flag ? 0 : 1);
 
}


/* Routine to fill in the index values for this record, now that we have found that it matches.  */

void gribfill (gaint eoff, gaint ioff, gaint joff, gaint nsiz, struct grhdr *ghdr, struct gaindx *pindx) {
gaint boff,koff;

/* the variable nsiz=nrec*ng1elems; 
   nrec=1 unless we're dealing with gribgrd 29 which has 2 grids per record so nrec=2 
   ng1elems=3 */
  koff = nsiz*(eoff+ioff) + joff;   /* use this when bigflg is not set */
  if (bigflg) {
    /* ldpos and lbpos are type off_t instead of int */
    boff = 2*(eoff+ioff) + joff;
    *(pindxb->bigpnt+boff) = ghdr->ldpos;
    if (ghdr->bmsflg) *(pindxb->bigpnt+boff+1) = ghdr->lbpos;
    /* bnum is still written out as an int */
    boff = (eoff+ioff) + joff;
    *(pindx->intpnt+boff) = ghdr->bnum;
  } else {
    *(pindx->intpnt+koff) = ghdr->dpos;
    if (ghdr->bmsflg) *(pindx->intpnt+koff+1) = ghdr->bpos;
    *(pindx->intpnt+koff+2) = ghdr->bnum;
  }  
  *(pindx->fltpnt+koff)   = ghdr->dsf;
  *(pindx->fltpnt+koff+1) = ghdr->bsf;
  *(pindx->fltpnt+koff+2) = ghdr->ref;
}


/* Routine to print out fields from the grib header */

void gribpr(struct grhdr *ghdr) {
  if (bigflg) {
    printf ("%6i % 10ld % 3i % 1i % 5i % 4i % 4i %-5i % 10ld % 10ld % 3i  ",
	    irec,(long)fpos,ghdr->id,ghdr->gdsflg,ghdr->bmsflg,ghdr->parm,ghdr->ltyp,
	    ghdr->level,(long)ghdr->ldpos,(long)ghdr->lbpos,ghdr->bnum);
  }
  else {
    printf ("%6i % 10ld % 3i % 1i % 5i % 4i % 4i %-5i % 10i % 10i % 3i  ",
	    irec,(long)fpos,ghdr->id,ghdr->gdsflg,ghdr->bmsflg,ghdr->parm,ghdr->ltyp,
	  ghdr->level,ghdr->dpos,ghdr->bpos,ghdr->bnum);
  }
  printf ("btim: %04i%02i%02i%02i:%02i ",ghdr->btim.yr,
	  ghdr->btim.mo,ghdr->btim.dy,ghdr->btim.hr,ghdr->btim.mn);
  printf ("tau: % 6i ",ghdr->fcstt);
  printf ("dtim: %04i%02i%02i%02i:%02i ",ghdr->dtim.yr,
	  ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr,ghdr->dtim.mn);
  printf("\n");
}


/* Routine to write out machine independent grib1 map file
   This subroutine was used with g1ver=2 or 3, but is now deprecated with g1ver=5. 
 */

gaint wtgmap(void) {
gaint i,nb,bcnt,idum;
gafloat fdum;
unsigned char *map=NULL;
unsigned char ibmfloat[4];
 
 /* calculate the size of the version==1 index file */
 nb = 2 + (4*4) +         /* version in byte 2, then 4 ints with number of each data type */
   pindx->hinum*sizeof(gaint) +
   pindx->hfnum*sizeof(gaint) +
   pindx->intnum*sizeof(gaint) +
   pindx->fltnum*sizeof(gafloat) ;
 
 /* add additional info */
 if (g1ver==2) {
   nb=nb+7;      /* base time (+ sec)  for compatibility with earlier version 2 maps */
   nb=nb+8*4;    /* grvals for time <-> grid conversion */
 }

 /* allocate space for the map */
 map = (unsigned char *)malloc(nb);
 if (map == NULL) {
   fprintf(stderr,"gribmap: ERROR! failed to allocate %d bytes for the map \n",nb);
   return(60);
 }

 /* write out the version number and the sizes of the header and index arrays */
 bcnt=0;
 gapby(0,map,bcnt,1);      bcnt++  ;   /* set the first byte to 0 */
 gapby(g1ver,map,bcnt,1);  bcnt++  ;   /* set the second byte to the version number */
 putint(pindx->hinum,map,&bcnt);              /* # ints in header   */
 putint(pindx->hfnum,map,&bcnt);              /* # floats in header   */
 putint(pindx->intnum,map,&bcnt);             /* # index ints   */
 putint(pindx->fltnum,map,&bcnt);             /* # index floats   */
 
 if (g1ver==2) {
   /* write out base time for consistency with earlier version 2 maps */
   /* base time not needed for version 3 */
   gapby(btimdd.yr,map,bcnt,2);  bcnt+=2 ;   /* initial year */
   gapby(btimdd.mo,map,bcnt,1);  bcnt++  ;   /* initial month */ 
   gapby(btimdd.dy,map,bcnt,1);  bcnt++  ;   /* initial day */
   gapby(btimdd.hr,map,bcnt,1);  bcnt++  ;   /* initial hour */
   gapby(btimdd.mn,map,bcnt,1);  bcnt++  ;   /* initial minute */
   gapby(0,map,bcnt,1);          bcnt++  ;   /* initial second */
 } 

 /* write the header */
 if (pindx->hinum) {
   for (i=0;i<pindx->hinum;i++) {
     idum=*(pindx->hipnt+i);
     putint(idum,map,&bcnt);
   }
 }

 /* write the indices */
 for (i=0;i<pindx->intnum;i++) {
   idum=*(pindx->intpnt+i);
   putint(idum,map,&bcnt);
 }
 for (i=0;i<pindx->fltnum;i++) {
   fdum=*(pindx->fltpnt+i);
   rc=flt2ibm(fdum, ibmfloat); 
   if (rc<0) return(601);
   memcpy(&map[bcnt],ibmfloat,4); bcnt+=4;
 }
 
 if (g1ver==2) {
   /* write out the factors for converting from grid to absolute time */ 
   /* the conversion vals are not needed for version 3 */
   for (i=0;i<8;i++) {
     fdum=*(pfi->grvals[3]+i);
     rc=flt2ibm(fdum, ibmfloat); 
     if (rc<0) return(601);
     memcpy(&map[bcnt],ibmfloat,4); bcnt+=4;
   }
 } 

 /* write to the map file */
 fwrite(map,1,bcnt,mfile);
 free(map);
 return(0);

}

/* Routine to dump a 4 byte int into a character stream */

void putint(gaint dum, unsigned char *buf, gaint *off) {
 gaint offset;

 offset=*off;
 if (dum < 0) {
   dum=-dum;
   gapby(dum,buf,offset,4);
   gapbb(1,buf+offset,0,1);
 } else {
   gapby(dum,buf,offset,4);
 }
 offset+=4;
 *off=offset;
 
}

/* New routine to write out a GRIB1 map file (for g1ver 4+) */

gaint wtg1map(struct gafile *pfi, struct gaindx *pindx, struct gaindxb *pindxb) {
  FILE *mfile=NULL;
  gaint rc;

  mfile = fopen(pfi->mnam,"wb");
  if (mfile==NULL) {
    printf ("gribmap: ERROR! Could not create GRIB1 index file: %s\n",pfi->mnam); 
    return(1);
  } 
  printf("gribmap: writing the GRIB1 index file (version %d) \n",pindx->type);

  rc = fwrite (pindx,sizeof(struct gaindx),1,mfile);
  if (rc!=1) {
    printf("gribmap: ERROR! Unable to write pindx structure to GRIB1 index file\n");
    fflush(stdout); return(1);
  }  
  if (pindx->hinum>0) {
    rc = fwrite(pindx->hipnt,sizeof(gaint),pindx->hinum,mfile);
    if (rc!=pindx->hinum) {
      printf("gribmap: ERROR! Unable to write header integers to GRIB1 index file\n");
      fflush(stdout); return(1);
    }  
  }
  if (pindx->hfnum>0) {
    rc = fwrite(pindx->hfpnt,sizeof(gafloat),pindx->hfnum,mfile);
    if (rc!=pindx->hfnum) {
      printf("gribmap: ERROR! Unable to write header floats to GRIB1 index file\n");
      fflush(stdout); return(1);
    }  
  }
  if (pindx->intnum>0) {
    rc = fwrite(pindx->intpnt,sizeof(gaint),pindx->intnum,mfile);
    if (rc!=pindx->intnum) {
      printf("gribmap: ERROR! Unable to write index integers to GRIB1 index file\n");
      fflush(stdout); return(1);
    }  
  }
  if (pindx->fltnum>0) {
    rc = fwrite(pindx->fltpnt,sizeof(gafloat),pindx->fltnum,mfile);
    if (rc!=pindx->fltnum) {
      printf("gribmap: ERROR! Unable to write index floats to GRIB1 index file\n");
      fflush(stdout); return(1);
    }  
  }
  if (pindxb->bignum>0) {
    rc = fwrite(pindxb->bigpnt,sizeof(off_t),pindxb->bignum,mfile);
    if (rc!=pindxb->bignum) {
      printf("gribmap: ERROR! Unable to write index off_ts to GRIB1 index file\n");
      fflush(stdout); return(1);
    }  
  }
  fclose (mfile);
  return(0);
}

#if GRIB2

/* Routine to fill in values for grib2 record, now that we know it matches. */
void g2fill (gaint eoff, gaint ioff, gaint ng2elems, off_t iseek, g2int fldnum, 
		struct gag2indx *g2indx) {
gaint joff;
  if (g2indx->bigflg) {
    joff = eoff+ioff;
    ioff = (ng2elems-1)*(eoff+ioff);
    *(g2indx->g2bigpnt+joff) = iseek;
    *(g2indx->g2intpnt+ioff) = fldnum;
  } else {
    ioff = ng2elems*(eoff+ioff);
    *(g2indx->g2intpnt+ioff+0) = (gaint)iseek;
    *(g2indx->g2intpnt+ioff+1) = fldnum;
  }
}

/* Routine to write out grib2 index file. 
   All versions of the index file are machine dependent. 
   A test to see if byte-swapping is required is done
   in gaddes.c, when the data descriptor file is opened. 

     g2ver=1 : contains the version number, followed by 
               the array size N, followed by the array of N numbers. 
               All are 4-byte integers (type gaint). 
     g2ver=2 : contains the version number, followed by 
               the array size N, followed by the array of N numbers that are
               4 byte ints, followed by an array of N numbers that are 8 byte 
               off_t integers.
     g2ver=3 : contains the version number, 
               followed by five integers: bigflg, trecs, tsz, esz, array size (N), 
	       then the arrays of index values (N integers).
               If bigflg is set, then off_t array is in use.
*/
gaint wtg2map(struct gafile *pfi, struct gag2indx *g2indx) {
  FILE *mfile;
  gaint rc,i;
  
  /* open the index file for writing */
  mfile = fopen(pfi->mnam,"wb");
  if (mfile==NULL) {
    printf ("gribmap: Error! Unable to open index file: %s\n",pfi->mnam);
    fflush(stdout); return(1);
  } 
  printf("gribmap: Writing out the GRIB2 index file (version %d)\n",g2indx->version);
  /* write the version number */
  rc = fwrite(&g2indx->version, sizeof(gaint),1,mfile);
  if (rc!=1) {
    printf("gribmap: ERROR! Unable to write version number to index file, rc=%d \n",rc);
    fflush(stdout); return(1);
  }  
  if (g2indx->version>1) {
    /* write bigflg */
    rc = fwrite(&g2indx->bigflg, sizeof(gaint),1,mfile);
    if (rc!=1) {
      printf("gribmap: ERROR! Unable to write bigflg to index file, rc=%d \n",rc);
      fflush(stdout); return(1);
    }  
    /* write trecs */
    rc = fwrite(&g2indx->trecs, sizeof(gaint),1,mfile);
    if (rc!=1) {
      printf("gribmap: ERROR! Unable to write trecs to index file, rc=%d \n",rc);
      fflush(stdout); return(1);
    }  
    /* write tsz */
    rc = fwrite(&g2indx->tsz, sizeof(gaint),1,mfile);
    if (rc!=1) {
      printf("gribmap: ERROR! Unable to write tsz to index file, rc=%d \n",rc);
      fflush(stdout); return(1);
    }  
    /* write esz */
    rc = fwrite(&g2indx->esz, sizeof(gaint),1,mfile);
    if (rc!=1) {
      printf("gribmap: ERROR! Unable to write esz to index file, rc=%d \n",rc);
      fflush(stdout); return(1);
    }  
  }
  /* write the array size */
  rc = fwrite(&g2indx->g2intnum,sizeof(gaint),1,mfile);
  if (rc!=1) {
    printf("gribmap: ERROR! Unable to write g2intnum to index file, rc=%d \n",rc);
    fflush(stdout); return(1);
  }  
  /* write the the array of index values */
  rc = fwrite(g2indx->g2intpnt,sizeof(gaint),g2indx->g2intnum,mfile);
  if (rc!=g2indx->g2intnum) {
    printf("gribmap: ERROR! Unable to write g2intpnt to index file, rc=%d \n",rc);
    fflush(stdout); return(1);
  }  
  if (g2indx->version>1) {
    /* if bigflg is set, write the the array of off_t values */
    if (g2indx->bigflg==1) {
      rc = fwrite(g2indx->g2bigpnt,sizeof(off_t),g2indx->g2intnum,mfile);
      if (rc!=g2indx->g2intnum) {
	printf("gribmap: ERROR! Unable to write g2bigpnt to index file, rc=%d \n",rc);
	fflush(stdout); return(1);
      }  
    }
  }
  fclose(mfile);

  /* JMA's extra debugging step: look for variables in descriptor file that were not matched.
     Only the first three time steps for the first ensemble member are checked.  */
/*   gaint joff,ioff,toff,eoff,e,t,tmax; */
/*   for (e=0; e<1; e++) { */
/*     eoff=e*pfi->dnum[3]*pfi->trecs; */
/*     (pfi->dnum[3]>1) ? (tmax=2) : (tmax=1) ; */
/*     for (t=0; t<tmax; t++) { */
/*       toff=t*pfi->trecs; */
/*       for (i=0; i<pfi->trecs; i++) { */
/* 	ioff=toff+i; */
/* 	if (g2indx->bigflg) { */
/* 	  joff = eoff+ioff; */
/* 	  if (g2indx->g2bigpnt[joff] <-900)  */
/* 	    printf("variable record %d is unmatched for e=%d, t=%d\n",i+1,e+1,t+1); */
/* 	} else { */
/* 	  joff = 2*(eoff+ioff); */
/* 	  if (g2indx->g2intpnt[joff] < -900) */
/* 	    printf("variable record %d is unmatched for e=%d, t=%d\n",i+1,e+1,t+1); */
/* 	} */
/*       } */
/*     } */
/*   } */

  return(0);
}

/* Checks grid properties for a grib2 field. 
   Returns 0 if ok, 1 if doesn't match descriptor */

gaint g2grid_check (gribfield *gfld, struct gafile *pfi, gaint r, gaint f) {
gaint xsize=0,ysize=0;

  /* Check total number of grid points */
  if (pfi->grbgrd==255 || pfi->grbgrd<-900) {
    if (((pfi->ppflag) && (gfld->ngrdpts != pfi->ppisiz * pfi->ppjsiz)) ||
        ((pfi->ppflag==0) && (gfld->ngrdpts != pfi->dnum[0] * pfi->dnum[1]))) {
      if (verb) printf ("number of grid points does not match descriptor ");
      return(1);
    }
  } 
  /* Check nx and ny for Lat/Lon, Polar Stereographic, and Lambert Conformal grids */
  if (pfi->ppflag) {
    xsize = pfi->ppisiz;
    ysize = pfi->ppjsiz;
  } else {
    xsize = pfi->dnum[0];
    ysize = pfi->dnum[1];
  }
  if ((gaint)gfld->igdtmpl[7] != -1) {
    if (gfld->igdtnum==0 || gfld->igdtnum==40 || gfld->igdtnum==20 || gfld->igdtnum==30) {
      if (gfld->igdtmpl[7] != xsize) {
	if (verb) printf ("x dimensions are not equal: nx=%d xsize=%d",(gaint)gfld->igdtmpl[7],xsize); 
	return(1);
      } 
      if (gfld->igdtmpl[8] != ysize) {
	if (verb) printf ("y dimensions are not equal: nx=%d xsize=%d",(gaint)gfld->igdtmpl[8],ysize); 
	return(1);
      }
    }
  }
  return(0);
}

/* Checks time metadata in grib2 message. 
   Returns integer value of time axis index if ok, -99 if not */
gaint g2time_check (gribfield *gfld, g2int *listsec1, struct gafile *pfi, 
		    gaint r, gaint f, gaint tmin, gaint tmax) {
  struct dt tref,tfld,tvalid;
  gaint it,tfield,trui_idx,endyr_idx=0;
  gafloat t;
  gadouble v1,delta;
  /* Get reference time from Section 1 of GRIB message */
  tref.yr = listsec1[5];
  tref.mo = listsec1[6];
  tref.dy = listsec1[7];
  tref.hr = listsec1[8];
  tref.mn = listsec1[9];
  tfield = tfld.yr = tfld.mo = tfld.dy = tfld.hr = tfld.mn = 0;  /* initialize */
 	 
  if (notau) {
    /* use reference time as valid time */
    tvalid.yr = tref.yr;
    tvalid.mo = tref.mo;
    tvalid.dy = tref.dy;
    tvalid.hr = tref.hr;
    tvalid.mn = tref.mn;
  }
  else {
    /* For fields at a point in time (PDT<8 or PDT=15 or PDT=48 or PDT=60) */
    if (gfld->ipdtnum < 8 || gfld->ipdtnum==15 || gfld->ipdtnum==48 || gfld->ipdtnum==60) {
      /* trui==Time Range Unit Indicator, trui_idx is the 0-based index for the entries in the PDT 4.n list  */
      trui_idx=7;            
      if (gfld->ipdtnum==48) trui_idx=18;
      if      (gfld->ipdtmpl[trui_idx]== 0) tfld.mn = gfld->ipdtmpl[trui_idx+1];     
      else if (gfld->ipdtmpl[trui_idx]== 1) tfld.hr = gfld->ipdtmpl[trui_idx+1];
      else if (gfld->ipdtmpl[trui_idx]== 2) tfld.dy = gfld->ipdtmpl[trui_idx+1];
      else if (gfld->ipdtmpl[trui_idx]== 3) tfld.mo = gfld->ipdtmpl[trui_idx+1];
      else if (gfld->ipdtmpl[trui_idx]== 4) tfld.yr = gfld->ipdtmpl[trui_idx+1];	 
      else if (gfld->ipdtmpl[trui_idx]==10) tfld.hr = gfld->ipdtmpl[trui_idx+1]*3;   /* 3Hr incr */
      else if (gfld->ipdtmpl[trui_idx]==11) tfld.hr = gfld->ipdtmpl[trui_idx+1]*6;   /* 6Hr incr */  
      else if (gfld->ipdtmpl[trui_idx]==12) tfld.hr = gfld->ipdtmpl[trui_idx+1]*12;  /* 2Hr incr */
      else tfield=-99;
      if (tfield==-99) {
	/* use reference time as valid time */
	tvalid = tref;
      }
      else {
	/* add forecast time to reference time to get valid time */
	timadd(&tref,&tfld);
	tvalid = tfld;
      }
    }
    /* For fields that are statistically processed over a time interval 
       e.g. averages, accumulations, extremes, et al. */
    else if ((gfld->ipdtnum>=8 && gfld->ipdtnum<=12) ||  gfld->ipdtnum==61) {
      trui_idx=7; 
      if      (gfld->ipdtnum==8)  endyr_idx=15;
      else if (gfld->ipdtnum==9)  endyr_idx=22;
      else if (gfld->ipdtnum==10) endyr_idx=16;
      else if (gfld->ipdtnum==11) endyr_idx=18;
      else if (gfld->ipdtnum==12) endyr_idx=17;
      else if (gfld->ipdtnum==61) endyr_idx=24;

      if (tauave==0) {
	/* valid time is the end of the overall time interval */
	tvalid.yr = gfld->ipdtmpl[endyr_idx];
	tvalid.mo = gfld->ipdtmpl[endyr_idx+1];
	tvalid.dy = gfld->ipdtmpl[endyr_idx+2];
	tvalid.hr = gfld->ipdtmpl[endyr_idx+3];
	tvalid.mn = gfld->ipdtmpl[endyr_idx+4];
      }
      else {
	/* valid time is the beginning of the overall time interval */
	if      (gfld->ipdtmpl[trui_idx]== 0) tfld.mn = gfld->ipdtmpl[trui_idx+1];
	else if (gfld->ipdtmpl[trui_idx]== 1) tfld.hr = gfld->ipdtmpl[trui_idx+1];
	else if (gfld->ipdtmpl[trui_idx]== 2) tfld.dy = gfld->ipdtmpl[trui_idx+1];
	else if (gfld->ipdtmpl[trui_idx]== 3) tfld.mo = gfld->ipdtmpl[trui_idx+1];
	else if (gfld->ipdtmpl[trui_idx]== 4) tfld.yr = gfld->ipdtmpl[trui_idx+1];
	else if (gfld->ipdtmpl[trui_idx]==10) tfld.hr = gfld->ipdtmpl[trui_idx+1]*3;   /* 3Hr incr */
	else if (gfld->ipdtmpl[trui_idx]==11) tfld.hr = gfld->ipdtmpl[trui_idx+1]*6;   /* 6Hr incr */
	else if (gfld->ipdtmpl[trui_idx]==12) tfld.hr = gfld->ipdtmpl[trui_idx+1]*12;  /* 2Hr incr */
	else tfield=-99;
	if (tfield==-99) {
	  /* unable to get forecast time, so use reference time as valid time */
	  tvalid = tref;
	}
	else {
	  /* add reference time and forecast time together to get beginnin of overall time interval */
	  timadd(&tref,&tfld);
	  tvalid = tfld;
	}
      }
    }
    else {
      printf("Product Definition Template %ld not handled \n",gfld->ipdtnum);
      return(-99);
    }  
  }
  /* Check if valid time is within grid limits */
  v1 = *(pfi->abvals[3]+5);  /* v1 is non-zero if time axis unit is months */
  if (v1>0) 
    delta = 0.36;  /* large for monthly data, ~10 days */
  else 
    delta = 0.01;  /* small for minutes data, the old default */
  t = t2gr(pfi->abvals[3],&tvalid);
  if (t<(1.0-delta) || t>((gafloat)(pfi->dnum[3])+delta)) {
    if (verb) printf("valid time %4d%02d%02d%02d:%02d (t=%g) is outside grid limits",
	   tvalid.yr,tvalid.mo,tvalid.dy,tvalid.hr,tvalid.mn,t);
    return(-99);
  }
  /* Check if valid time is an integer (+/- the designated threshold) */
  it = (gaint)(t+0.01);
  if (fabs((gafloat)it - t) > delta) {
    if (verb) printf("valid time %4d%02d%02d%02d:%02d (t=%g) has non-integer grid index",
	   tvalid.yr,tvalid.mo,tvalid.dy,tvalid.hr,tvalid.mn,t);
    return(-99);
  }
  /* Check if valid time matches range of times for this file  */
  if (it<tmin || it>tmax) {
    if (verb) printf("valid time %4d%02d%02d%02d:%02d (it=%d) is outside file limits (%d-%d)",
	   tvalid.yr,tvalid.mo,tvalid.dy,tvalid.hr,tvalid.mn,it,tmin,tmax);
    return(-99);
  }
  if (verb) printf("valid at %4d-%02d-%02d-%02d:%02d (t=%d)",tvalid.yr,tvalid.mo,tvalid.dy,tvalid.hr,tvalid.mn,it);
  return (it);
}

/* Loops over variables in descriptor file, looking for match to current grib2 field. 
   If variables match, returns offset, if not, returns -999 */

gaint g2var_match (gribfield *gfld, struct gafile *pfi, gaint sp, gaint sp2) {
  struct gavar *pvar;
  gadouble lev1,lev2,z;
  gadouble (*conv) (gadouble *, gadouble);
  gaint rc1,rc2,rc3,rc4,rc5,rc6,lev_idx;
  gaint i,ioff,iz;
  
  lev_idx=9;
  if (gfld->ipdtnum==48) lev_idx=20;

  /* Get first level values from grib field */
  lev1 = scaled2dbl(gfld->ipdtmpl[lev_idx+1],gfld->ipdtmpl[lev_idx+2]);
  /* Check if we've got info on 2nd level */
  if (gfld->ipdtmpl[lev_idx+3] != 255) 
    lev2 = scaled2dbl(gfld->ipdtmpl[lev_idx+4],gfld->ipdtmpl[lev_idx+5]);
  else 
    lev2 = -999;
  
  /* See if we match any variables in the descriptor file */
  pvar = pfi->pvar1;
  ioff = -999;
  i = 0;
  while (i<pfi->vnum) {
    if (pvar->levels>0) {      
      /* Z-varying data */
      rc1 = (gaint)pvar->units[0]==(gaint)gfld->discipline ? 0 : 1;       /* discipline */
      rc2 = (gaint)pvar->units[1]==(gaint)gfld->ipdtmpl[0] ? 0 : 1;       /* parameter category */
      rc3 = (gaint)pvar->units[2]==(gaint)gfld->ipdtmpl[1] ? 0 : 1;       /* parameter number  */
      rc4 = (gaint)pvar->units[3]==sp  ? 0 : 1;     	                  /* Statistical Process */
      rc5 = (gaint)pvar->units[4]==sp2 ? 0 : 1;	 	                  /* Spatial Process     */
      rc6 = (gaint)pvar->units[8]==(gaint)gfld->ipdtmpl[lev_idx] ? 0 : 1; /* LTYPE1     */
      if (rc1==0 && rc2==0 && rc3==0 && rc4==0 && rc5==0 && rc6==0) {     /* all the above match */
	/* get a Z value for level 1 */
	conv = pfi->ab2gr[2];
	z = conv(pfi->abvals[2],lev1);
	if (z>0.99 && z<((gadouble)(pvar->levels)+0.01)) {
	  iz = (gaint)(z+0.5);
	  /* make sure Z value for level 1 is an integer */
	  if (fabs(z-(gadouble)iz) < 0.01) {
	    /* check if additional grib codes match */
	    if (pvar->g2aflg) {
	      if ((g2a_check(gfld,pvar))==1) {
		ioff = pvar->recoff + iz - 1;
		return(ioff); 
	      }
	    }
	    else {
	      /* no additional grib codes, so match is OK */
	      ioff = pvar->recoff + iz - 1;
	      return(ioff); 
	    }
	  }
	}
      }
    }
    else {       
      /* non-Z-varying data */
      rc1 = (gaint)pvar->units[0]==(gaint)gfld->discipline ? 0 : 1;       /* discipline */
      rc2 = (gaint)pvar->units[1]==(gaint)gfld->ipdtmpl[0] ? 0 : 1;       /* parameter category   */
      rc3 = (gaint)pvar->units[2]==(gaint)gfld->ipdtmpl[1] ? 0 : 1;       /* parameter number */
      rc4 = (gaint)pvar->units[3]==sp ? 0 : 1;                            /* Statistical Process */
      rc5 = (gaint)pvar->units[4]==sp2 ? 0 : 1;                           /* Spatial Process     */
      rc6 = (gaint)pvar->units[8]==(gaint)gfld->ipdtmpl[lev_idx] ? 0 : 1; /* LTYPE1     */
      if (rc1==0 && rc2==0 && rc3==0 && rc4==0 && rc5==0 && rc6==0) {     /* all the above match */
	/* check if level value(s) match those given in descriptor file */
	if (
	    (pvar->units[9] < -900)                /* LVAL not given */
	    ||  
	    (pvar->units[10] < -900 &&             /* LVAL2 not given */
	     dequal(pvar->units[9],lev1,1e-8)==0)                 /* and LVAL1 matches */ 
	    ||  
	    (pvar->units[10] > -900 &&             /* LVAL2 is given */
	     dequal(pvar->units[9],lev1,1e-8)==0 &&               /* and LVAL1 matches */
	     dequal(pvar->units[10],lev2,1e-8)==0)                /* and LVAL2 matches */
	    ||
	    (pvar->units[10] > -900 &&             /* LVAL2 is given */
	     pvar->units[11] > -900 &&             /* LTYPE2 is given */
	     dequal(pvar->units[9],lev1,1e-8)==0 &&               /* and LVAL1 matches */
	     dequal(pvar->units[10],lev2,1e-8)==0 &&              /* and LVAL2 matches */
	     dequal(pvar->units[11],gfld->ipdtmpl[lev_idx+3],1e-8)==0)   /* and LTYPE2 matches */
	    ) { 
	  /* check if additional grib codes match */
	  if (pvar->g2aflg) {
	    if ((g2a_check(gfld,pvar))==1) {
	      ioff = pvar->recoff;
	      return(ioff); 
	    }
	  }
	  else {
	    /* no additional grib codes, so match is OK */
	    ioff = pvar->recoff;
	    return(ioff);
	  }
	}
      }
    }
    pvar++; i++;
  }  /* end of loop over variables in descriptor file */
  return(ioff);
}

/* check additional codes that need to be matched for certain PDTs (5, 9, and 48 for now)
   returns 1 if match is OK, otherwise 0
*/
gaint g2a_check (gribfield *gfld, struct gavar *pvar) {
  gaint match,rc1,rc2,rc3,rc4,rc5,rc6;
  gadouble ll,ul,s1,s2,w1,w2;

  match=0;
  /* probability forecasts */
  if (gfld->ipdtnum==5 || gfld->ipdtnum==9) {
    ll = scaled2dbl(gfld->ipdtmpl[18],gfld->ipdtmpl[19]);  /* get lower limit */
    ul = scaled2dbl(gfld->ipdtmpl[20],gfld->ipdtmpl[21]);  /* get upper limit */
    if ((gaint)pvar->units[16]==(gaint)gfld->ipdtmpl[17]) {     /* probability type matches */
      if ((gfld->ipdtmpl[17]==0 && dequal(pvar->units[17],ll,1e-8)==0) ||  /* check all cases of prob type */
	  (gfld->ipdtmpl[17]==1 && dequal(pvar->units[17],ul,1e-8)==0) ||
	  (gfld->ipdtmpl[17]==2 && dequal(pvar->units[17],ll,1e-8)==0 && dequal(pvar->units[18],ul,1e-8)==0) ||
	  (gfld->ipdtmpl[17]==3 && dequal(pvar->units[17],ll,1e-8)==0) ||
	  (gfld->ipdtmpl[17]==4 && dequal(pvar->units[17],ul,1e-8)==0))
	match=1;
    }
  }
  /* percentile forecasts */
  else if (gfld->ipdtnum==6 || gfld->ipdtnum==10) {
    if ((gaint)pvar->units[16]==(gaint)gfld->ipdtmpl[15]) match=1; /* percentiles match */
  }
  /* optical properties of aerosol */
  else if (gfld->ipdtnum==48) {
    s1 = scaled2dbl(gfld->ipdtmpl[4],gfld->ipdtmpl[5]);
    s2 = scaled2dbl(gfld->ipdtmpl[6],gfld->ipdtmpl[7]);
    w1 = scaled2dbl(gfld->ipdtmpl[9],gfld->ipdtmpl[10]);
    w2 = scaled2dbl(gfld->ipdtmpl[11],gfld->ipdtmpl[12]);
    if ((gaint)pvar->units[16]==(gaint)gfld->ipdtmpl[2]) {     /* aerosol type matches */
      if (gfld->ipdtmpl[3]!=255) {  /* size interval type is not missing */
	if (gfld->ipdtmpl[8]!=255) {  /* wavelength interval type is not missing */
	  /* we must match 6 codes (size and wavelength )*/
	  rc1 = dequal(pvar->units[17],(gadouble)gfld->ipdtmpl[3],1e-8);   /* size interval type */
	  rc2 = dequal(pvar->units[18],s1,1e-8);                           /* size1 */
	  rc3 = dequal(pvar->units[19],s2,1e-8);                           /* size2 */
	  rc4 = dequal(pvar->units[20],(gadouble)gfld->ipdtmpl[8],1e-8);   /* wavelength interval type */
	  rc5 = dequal(pvar->units[21],w1,1e-8);                           /* wavelength1 */
	  rc6 = dequal(pvar->units[22],w2,1e-8);                           /* wavelength2 */
	  if (rc1==0 && rc2==0 && rc3==0 && rc4==0 && rc5==0 && rc6==0)    /* all the above match */
	    match=1;
	}
	else {
	  /* we must match 3 codes (size only) */
	  rc1 = dequal(pvar->units[17],(gadouble)gfld->ipdtmpl[3],1e-8);   /* size interval type */
	  rc2 = dequal(pvar->units[18],s1,1e-8);                           /* size1 */
	  rc3 = dequal(pvar->units[19],s2,1e-8);                           /* size2 */
	  if (rc1==0 && rc2==0 && rc3==0)                                  /* all the above match */
	    match=1;
	}
      }
      else {
	if (gfld->ipdtmpl[8]!=255) {
	  /* we must match 3 codes (wavelength only) */
	  rc4 = dequal(pvar->units[20],(gadouble)gfld->ipdtmpl[8],1e-8);   /* wavelength interval type */
	  rc5 = dequal(pvar->units[21],w1,1e-8);                           /* wavelength1 */
	  rc6 = dequal(pvar->units[22],w2,1e-8);                           /* wavelength2 */
	  if (rc4==0 && rc5==0 && rc6==0)                                  /* all the above match */
	    match=1;
	}
	else {
	  /* only the aerosol type is available for matching; leave this as a non-match for now */
	}
      }
    }
  }
  return(match);
}

/* Loops over ensembles to see if any of the ensemble codes match current grib2 field.
   Returns ensemble index e if codes are present and match, -999 otherwise */
gaint g2ens_match (gribfield *gfld, struct gafile *pfi) {
  struct gaens *ens;
  gaint e;
  e=1;
  ens=pfi->ens1;
  for (e=1; e<=pfi->dnum[4]; e++) {
    if (ens->grbcode[0]>-900) {
      if (ens->grbcode[1]>-900) {
	/* PDT 1, 11, 60 or 61 */
	if ((gfld->ipdtnum==1 || gfld->ipdtnum==11 || gfld->ipdtnum==60 || gfld->ipdtnum==61) &&
	    ((ens->grbcode[0] == gfld->ipdtmpl[15]) && 
	     (ens->grbcode[1] == gfld->ipdtmpl[16]))) {
	  return(e);
	}
      }
      else {
	/* PDT 2 or 12 */
	if ((gfld->ipdtnum==2 || gfld->ipdtnum==12) &&
	    (ens->grbcode[0] == gfld->ipdtmpl[15])) {
	  return(e);
	}
      }
    }
    /* No grib codes and not an ensemble PDT */
    if (ens->grbcode[0]==-999 && ens->grbcode[1]==-999 &&
	(gfld->ipdtnum!=1 && gfld->ipdtnum!=2 && gfld->ipdtnum!=11 && gfld->ipdtnum!=12)) {    
      return(e);
    }
    ens++;
  }
  return(-999);
}

/* Checks ensemble codes, if provided in descriptor file. 
   Returns 0 if ok or not provided, 1 if codes don't match. */
gaint g2ens_check (struct gaens *ens, gribfield *gfld) {
  if (ens->grbcode[0]>-900) {
    if (ens->grbcode[1]>-900) {
      /* PDT 1, 11, 60 or 61 */
      if ((gfld->ipdtnum==1 || gfld->ipdtnum==11 || gfld->ipdtnum==60 || gfld->ipdtnum==61) &&
	  ((ens->grbcode[0] == gfld->ipdtmpl[15]) && 
	   (ens->grbcode[1] == gfld->ipdtmpl[16]))) return(0);
      else return(1);
    }
    else {
      /* PDT 2 or 12 */
      if ((gfld->ipdtnum==2 || gfld->ipdtnum==12) &&
	  (ens->grbcode[0] == gfld->ipdtmpl[15])) return(0);
      else return(1);
    }
  }
  /* No grib codes and not an ensemble PDT */
  if (ens->grbcode[0]==-999 && ens->grbcode[1]==-999 &&
      (gfld->ipdtnum!=1 && gfld->ipdtnum!=2 && gfld->ipdtnum!=11 && gfld->ipdtnum!=12)) return(0);  
  else return(1);
}

/* Gets the statistical process used to derive a variable.
   returns -999 for variables "at a point in time" */
gaint g2sp (gribfield *gfld) {
  gaint sp;
  sp = -999;
  if (gfld->ipdtnum ==  8) sp = gfld->ipdtmpl[23];
  if (gfld->ipdtnum ==  9) sp = gfld->ipdtmpl[30];
  if (gfld->ipdtnum == 10) sp = gfld->ipdtmpl[24];
  if (gfld->ipdtnum == 11) sp = gfld->ipdtmpl[26];
  if (gfld->ipdtnum == 12) sp = gfld->ipdtmpl[25];
  if (gfld->ipdtnum == 15) sp = gfld->ipdtmpl[15];
  if (gfld->ipdtnum == 61) sp = gfld->ipdtmpl[32];
  if (sp==255) sp = -999;
  return(sp);
}

/* get the statistical process used within a spatial area (only for PDT 15) */
gaint g2sp2(gribfield *gfld) {
  gaint sp2;
  sp2 = -999;
  if (gfld->ipdtnum == 15) sp2 = gfld->ipdtmpl[16];
  if (sp2==255) sp2 = -999;
  return(sp2);
}

/* prints out relevant info from a grib2 record */
void g2prnt (gribfield *gfld, gaint r, g2int f, gaint sp, gaint sp2) {
  gaint atyp,styp,wtyp,lev_idx,i;
  gadouble ll,ul,s1,s2,w1,w2;

  lev_idx=9;
  if (gfld->ipdtnum==48) lev_idx=20;

  /* print record/field number */
  printf("%d.%ld: ",r,f);
  /* print level info */
  if (gfld->ipdtmpl[lev_idx+1]<0) 
    printf("lev1=%ld ",gfld->ipdtmpl[lev_idx]); /* just print the level1 type */
  else
    printf("lev1=%ld,%g ",gfld->ipdtmpl[lev_idx],scaled2dbl(gfld->ipdtmpl[lev_idx+1],gfld->ipdtmpl[lev_idx+2]));
  
  if (gfld->ipdtmpl[lev_idx+3]<255) {
    if (gfld->ipdtmpl[lev_idx+4]==-127) 
      printf("lev1=%ld ",gfld->ipdtmpl[lev_idx+3]); /* just print the level2 type */
    else
      printf("lev2=%ld,%g ",gfld->ipdtmpl[lev_idx+3],scaled2dbl(gfld->ipdtmpl[lev_idx+4],gfld->ipdtmpl[lev_idx+5])); 
  }

  /* print additional info for probability forecasts (PDTs 5 and 9) */
  if (gfld->ipdtnum == 5 || gfld->ipdtnum == 9) {
    /* probability forecasts */
    ll = scaled2dbl(gfld->ipdtmpl[18],gfld->ipdtmpl[19]);  /* get lower limit */
    ul = scaled2dbl(gfld->ipdtmpl[20],gfld->ipdtmpl[21]);  /* get upper limit */
    if (gfld->ipdtmpl[17]==2) {
      printf ("a%ld,%g,%g ",gfld->ipdtmpl[17],ll,ul);
    }
    else if (gfld->ipdtmpl[17]==0 || gfld->ipdtmpl[17]==3) {
      printf ("a%ld,%g ",gfld->ipdtmpl[17],ll);  
    }
    else if (gfld->ipdtmpl[17]==1 || gfld->ipdtmpl[17]==4) {
      printf ("a%ld,%g ",gfld->ipdtmpl[17],ul);  
    }
  }

  /* print additional info for PDT 48 */
  if (gfld->ipdtnum ==48) {
    atyp   = gfld->ipdtmpl[2];
    styp   = gfld->ipdtmpl[3];
    wtyp   = gfld->ipdtmpl[8];
    s1 = scaled2dbl(gfld->ipdtmpl[4],gfld->ipdtmpl[5]);
    s2 = scaled2dbl(gfld->ipdtmpl[6],gfld->ipdtmpl[7]);
    w1 = scaled2dbl(gfld->ipdtmpl[9],gfld->ipdtmpl[10]);
    w2 = scaled2dbl(gfld->ipdtmpl[11],gfld->ipdtmpl[12]);
    if (atyp!=65535) {
      if (styp!=255) {
	if (wtyp!=255) {
	  printf ("a%d,%d,%g,%g,%d,%g,%g ",atyp,styp,s1,s2,wtyp,w1,w2);
	}
	else {
	  printf ("a%d,%d,%g,%g ",atyp,styp,s1,s2);
	}
      }
      else {
	if (wtyp!=255) {
	  printf ("a%d,%d,%g,%g ",atyp,wtyp,w1,w2);
	}
	else {
	  printf ("a%d ",atyp);
	}
      } 
    }
  }

  /* print variable info */
  if (sp==-999)
    printf("var=%ld,%ld,%ld ",gfld->discipline,gfld->ipdtmpl[0],gfld->ipdtmpl[1]);
  else {
    if (sp2==-999)
      printf("var=%ld,%ld,%ld,%d ",gfld->discipline,gfld->ipdtmpl[0], gfld->ipdtmpl[1],sp);
    else 
      printf("var=%ld,%ld,%ld,%d,%d ",gfld->discipline,gfld->ipdtmpl[0], gfld->ipdtmpl[1],sp,sp2);
  }
  /* print ensemble info */
  if (gfld->ipdtnum==1 || gfld->ipdtnum==11 || gfld->ipdtnum==60 || gfld->ipdtnum==61) 
    printf("ens=%d,%d ",(gaint)gfld->ipdtmpl[15],(gaint)gfld->ipdtmpl[16]);
  if (gfld->ipdtnum==2 || gfld->ipdtnum==12) 
    printf("ens=%d ",(gaint)gfld->ipdtmpl[15]);
}

void gaseekgb(FILE *lugb, off_t iseek, g2int mseek, off_t *lskip, g2int *lgrib)
//$$$  SUBPROGRAM DOCUMENTATION BLOCK
//
// SUBPROGRAM: seekgb         Searches a file for the next GRIB message.
//   PRGMMR: Gilbert          ORG: W/NP11      DATE: 2002-10-28
//
// ABSTRACT: This subprogram searches a file for the next GRIB Message.
//   The search is done starting at byte offset iseek of the file referenced 
//   by lugb for mseek bytes at a time.
//   If found, the starting position and length of the message are returned
//   in lskip and lgrib, respectively.
//   The search is terminated when an EOF or I/O error is encountered.
//
// PROGRAM HISTORY LOG:
// 2002-10-28  GILBERT   Modified from Iredell's skgb subroutine
// 2009-01-16  VUONG     Changed  lskip to 4 instead of sizof(g2int)
// 2010-03-02  Doty      Modified for off_t sized offsets to support >2GB
//
// USAGE:    seekgb(FILE *lugb,g2int iseek,g2int mseek,int *lskip,int *lgrib)
//   INPUT ARGUMENTS:
//     lugb       - FILE pointer for the file to search.  File must be
//                  opened before this routine is called.
//     iseek      - number of bytes in the file to skip before search
//     mseek      - number of bytes to search at a time
//   OUTPUT ARGUMENTS:
//     lskip      - number of bytes to skip from the beggining of the file
//                  to where the GRIB message starts
//     lgrib      - number of bytes in message (set to 0, if no message found)
//
// ATTRIBUTES:
//   LANGUAGE: C
//
//$$$
{
      g2int  ret;
      g2int k,k4,nread,lim,start,vers,lengrib;
      off_t ipos;
      int    end;
      unsigned char *cbuf;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      *lgrib=0;
      cbuf=(unsigned char *)malloc(mseek);
      nread=mseek;
      ipos=iseek;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  LOOP UNTIL GRIB MESSAGE IS FOUND

      while (*lgrib==0 && nread==mseek) {

//  READ PARTIAL SECTION

        ret=fseeko(lugb,ipos,SEEK_SET);
        nread=fread(cbuf,sizeof(unsigned char),mseek,lugb);
        lim=nread-8;

//  LOOK FOR 'GRIB...' IN PARTIAL SECTION

        for (k=0;k<lim;k++) {
          gbit(cbuf,&start,(k+0)*8,4*8);
          gbit(cbuf,&vers,(k+7)*8,1*8);
          if (start==1196575042 && (vers==1 || vers==2)) {
//  LOOK FOR '7777' AT END OF GRIB MESSAGE
            if (vers == 1) gbit(cbuf,&lengrib,(k+4)*8,3*8);
            if (vers == 2) gbit(cbuf,&lengrib,(k+12)*8,4*8);
            ret=fseeko(lugb,ipos+(off_t)(k+lengrib-4),SEEK_SET);
//          Hard code to 4 instead of sizeof(g2int)
            k4=fread(&end,4,1,lugb);
            if (k4 == 1 && end == 926365495) {      //GRIB message found
                *lskip=ipos+(off_t)k;
                *lgrib=lengrib;
                break;
            }
          }
        }
        ipos=ipos+(off_t)lim;
      }

      free(cbuf);
}

#endif  /* matches #if GRIB2 */
