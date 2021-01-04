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
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "grads.h"
#include "gx.h"
#include "wx.h"

#if GEOTIFF==1
#include "xtiffio.h"
#include "geotiffio.h"
#include "geotiff.h"
#include "geokeys.h"
#include "geo_normalize.h"
#endif

#if USESHP==1
gaint s2shpwrt (SHPHandle, DBFHandle, struct dbfld *);
gaint gxshplin (SHPHandle, DBFHandle, struct dbfld *);
static struct dbfld *dbanch=NULL;   /* Anchor for shapefile data base fields */
static struct dbfld *dblast=NULL;   /* Last dbfld struct in chain */
#endif

/* gaint gxclvert(FILE *);             */
void gatmlb (struct gacmn *);      /* time label*/
static char pout[1256];            /* Build error msgs here */
static struct mapprj mpj;          /* Common map projection structure */
static gadouble wxymin,wxymax;     /* wx symbol limits */
static struct gxdsubs *dsubs=NULL; /* function pointers for graphics display */


/* Current I and J axis translation routines for grid to absolute
   for use by gaconv */

/* A note:  the conversion routines normally handle grid
   coordinates relative to the file (for convenience).
   But the graphics routines normally use grid coordinates
   with respect to the grid being worked with (ie, values
   1 to n).  Thus the gaconv routine handles this translation
   with the ioffset and joffset values.  */

static gadouble (*iconv) (gadouble *, gadouble);
static gadouble (*jconv) (gadouble *, gadouble);
static gadouble *ivars, *jvars;
static gadouble ioffset, joffset;
static gadouble idiv, jdiv;

gaint gagx (struct gacmn *pcm) {
gaint rc=0;
  rc = gxstrt (pcm->xsiz,pcm->ysiz,pcm->batflg,pcm->hbufsz,pcm->gxdopt,pcm->gxpopt,pcm->xgeom);
  if (rc) return (rc);
  pcm->pass = 0;
  pcm->ccolor = -9;
  pcm->cint = 0.0;
  pcm->cstyle = -9;
  pcm->cthick = 4;
  pcm->shdcnt = 0;
  pcm->cntrcnt = 0;
  pcm->lastgx = 0;
  pcm->xdim = -1;
  pcm->ydim = -1;
  pcm->xgr2ab = NULL;
  pcm->ygr2ab = NULL;
  pcm->xab2gr = NULL;
  pcm->yab2gr = NULL;
  return(0);
}

gaint rcols[13] = {9,14,4,11,5,13,3,10,7,12,8,2,6};

/* Figure out which graphics routine to call.  Use the first
   grid hung off gacmn to determine whether we are doing
   a 0-D, 1-D, or 2-D output.    */

void gaplot (struct gacmn *pcm) {
struct gagrid *pgr;
struct gastn *stn;
gaint proj;

  pcm->relnum = pcm->numgrd;
  proj = pcm->mproj;
  if (pcm->mproj>1 && (pcm->xflip || pcm->yflip)) pcm->mproj = 1;

  if (pcm->gout0==1) gastts(pcm);           /* gxout stat     */
  else if (pcm->gout0==2) gadprnt(pcm);     /* gxout print    */
  else if (pcm->gout0==3) gaoutgds(pcm);    /* gxout writegds */
  else {
    /*  If statflg, produce gxout-stat output for all displays  */
    if (pcm->statflg) gastts(pcm);
  
    /* output is a grid */
    if (pcm->type[0] == 1) {
      pgr = pcm->result[0].pgr;
      if (pgr->idim==-1) {                                     /* 0-D */
	if (pcm->gout2a == 7) gafwrt (pcm);
	else if (pcm->gout2a == 12) {
	  gaprnt (0,"Invalid dimension environment for GeoTIFF: \n");
	  gaprnt (0,"  Longitude and Latitude must be varying \n");
	}
	else if (pcm->gout2a == 13) {
	  gaprnt (0,"Invalid dimension environment for KML: \n");
	  gaprnt (0,"  Longitude and Latitude must be varying \n");
	}
	else if (pcm->gout2a == 15) {
	  gaprnt (0,"Invalid dimension environment for Shapefile: \n");
	  gaprnt (0,"  Longitude and Latitude must be varying \n");
	}
	else {
	  if (pgr->umin==1) 
	    snprintf(pout,1255,"Result value = %g \n",pgr->rmin);
	  else
	    snprintf(pout,1255,"Result value = %g \n",pcm->undef);
	  gaprnt (2,pout);
	}
      }
      else if (pgr->jdim==-1) {                                /* 1-D */
	if (pcm->gout2a==7) gafwrt (pcm);
	else if (pcm->gout2a == 12) {
	  gaprnt (0,"Invalid dimension environment for GeoTIFF: \n");
	  gaprnt (0,"  Longitude and Latitude must be varying \n");
	}
	else if (pcm->gout2a == 13) {
	  gaprnt (0,"Invalid dimension environment for KML: \n");
	  gaprnt (0,"  Longitude and Latitude must be varying \n");
	}
	else if (pcm->gout2a == 15) {
	  gaprnt (0,"Invalid dimension environment for Shapefile: \n");
	  gaprnt (0,"  Longitude and Latitude must be varying \n");
	}
	else if (pcm->gout2b==5 && pcm->numgrd>1) gascat(pcm);
	else if (pcm->gout1==1) gagrph(pcm,0);
	else if (pcm->gout1==2) gagrph(pcm,1);
	else if (pcm->gout1==3) gagrph(pcm,2);
	else galfil(pcm);
      }
      else {                                                  /* 2-D */
	if (pcm->numgrd==1) {
	  if (pcm->gout2a == 1) gacntr (pcm,0,0);         /* contour */
	  else if (pcm->gout2a ==  2) gacntr (pcm,1,0);   /* shaded */
	  else if (pcm->gout2a ==  3) gaplvl (pcm);       /* grid */
	  else if (pcm->gout2a ==  6) gafgrd (pcm);       /* fgrid */
	  else if (pcm->gout2a ==  7) gafwrt (pcm);       /* fwrite */
	  else if (pcm->gout2a == 10) gacntr (pcm,2,0);   /* grfill */
	  else if (pcm->gout2a == 12) gagtif (pcm,0);     /* geotiff */
	  else if (pcm->gout2a == 13 && pcm->kmlflg==1) gagtif (pcm,1);  /* kml image output */
	  else if (pcm->gout2a == 13 && pcm->kmlflg>1)  gakml (pcm);     /* kml contours or polygons */
	  else if (pcm->gout2a == 14) gacntr (pcm,3,0);   /* imap */
	  else if (pcm->gout2a == 15) gashpwrt (pcm);     /* shapefile */
	  else if (pcm->gout2a == 16) gacntr (pcm,4,0);   /* gxshad2   */
	  else if (pcm->gout2a == 17) gacntr (pcm,5,0);   /* gxshad2b  */
	  else {
	    gaprnt (0,"Internal logic error: invalid gout2a value\n");
            return;
          }
	} 
	else {
	  if (pcm->gout2b == 3) gaplvl (pcm);
	  else if (pcm->gout2b == 4 ) gavect (pcm,0);
	  else if (pcm->gout2b == 5 ) gascat (pcm);
	  else if (pcm->gout2b == 8 ) gastrm (pcm);
	  else gavect (pcm,1);
	}
      }
    }
    else {
    /* Output is station data */
      stn = pcm->result[0].stn;
      if (stn->idim==0 && stn->jdim==1) {
	if (pcm->goutstn==1 || pcm->goutstn==2 || pcm->goutstn==6) gapstn (pcm);
	else if (pcm->goutstn==3) gafstn(pcm);
	else if (pcm->goutstn==4) gapmdl (pcm);
	else if (pcm->goutstn==7) gasmrk (pcm);
	else if (pcm->goutstn==8) gastnwrt (pcm);
	else if (pcm->goutstn==9) gashpwrt (pcm);   /* shapefile */
	else gawsym (pcm);
      }
      else if (stn->idim==2 && stn->jdim == -1) gapprf (pcm);
      else if (stn->idim==3 && stn->jdim == -1) gatser (pcm);
      else gaprnt (0,"Invalid station data dimension environment\n");
    }
  }
  pcm->mproj = proj;
}

void gawgdsval(FILE* outfile, gafloat *val) {
  if (BYTEORDER != 1) {  /* always write big endian for the GDS */
    gabswp(val, 1);
  }
  fwrite(val, sizeof(gafloat), 1, outfile);
}

void gawgdstime(FILE* outfile, gadouble *val) {
  snprintf(pout,1255, "pre-byteswapped time: %g", *val); gaprnt(0, pout);
  if (BYTEORDER != 1) {  /* always write big endian for the GDS */
    ganbswp((char*)val, sizeof(gadouble));
  }
  snprintf(pout,1255, "byteswapped time: %g", *val); gaprnt(0, pout);
  fwrite(val, sizeof(gadouble), 1, outfile);
}


/* Writes station data out to a file as a DODS sequence */
void gaoutgds (struct gacmn *pcm) {

  const char startrec[4] = {0x5a, 0x00, 0x00, 0x00};
  const char stnidlen[4] = {0x00, 0x00, 0x00, 0x08};
  const char endrec[4]   = {0xa5, 0x00, 0x00, 0x00};
  
  struct garpt **currpt;
  struct garpt *ref, *levelref;
  gadouble coardstime;
  gafloat outFloat;
  gaint i, numvars, retval, levelstart, numreps;
  gaint *varlevels;
  char *sendstnid, *sendlat, *sendlon, *sendlev, *sendtime, *senddep, *sendind;
  char *sendoptions;
  FILE *outfile;
  size_t sz;

  if (pcm->wgds->fname == NULL) {
    gaprnt(0, "error: no file specified (use \"set writegds\").\n");
    return;
  }
  outfile = fopen(pcm->wgds->fname, "ab");
  if (outfile == NULL) {
    gaprnt(0, "error: WRITEGDS unable to open ");
    gaprnt(0, pcm->wgds->fname);
    gaprnt(0, " for write\n");
    return;
  } 
  gaprnt(0, "got options and opened file\n");

  if (pcm->wgds->opts == NULL) {
    gaprnt(0, "No options specified. Defaulting to full output (\"sxyztdi\").\n");
    sendoptions = "sxyztdi";
  } else {
    sendoptions = pcm->wgds->opts;
  }

  if (strchr(sendoptions, 'f')) {
    /* Write a DODS End-Of-Sequence marker to indicate to the client 
     *  that there is no more data. This is separate from gaoutgds() so 
     *  that time loops can be written as a single sequence.
     */
    gaprnt(0, "Finishing sequence:\n");
    gaprnt(0, "EOS\n");
    fwrite(endrec, sizeof(char), 4, outfile);
    fclose(outfile);
    return;
  }

  sendstnid = strchr(sendoptions, 's');
  sendlon = strchr(sendoptions, 'x');
  sendlat = strchr(sendoptions, 'y');
  sendlev = strchr(sendoptions, 'z');
  sendtime = strchr(sendoptions, 't');
  senddep = strchr(sendoptions, 'd');
  sendind = strchr(sendoptions, 'i');

  numvars = pcm->numgrd;
  sz = sizeof(struct garpt *) * numvars;
  currpt = (struct garpt **)galloc(sz,"currpt");
  sz = sizeof(gaint) * numvars;
  varlevels = (gaint *)galloc(sz,"varlevels");
  if (currpt == NULL || varlevels == NULL) {
    gaprnt(0, "error: memory allocation failed\n");
    fclose(outfile);
    return;
  }
  for (i = 0; i < numvars; i++) {
    varlevels[i] =  pcm->result[i].stn->pvar->levels;
    currpt[i] =  pcm->result[i].stn->rpt;
  }

  /* set levelstart to index of first level-dependent variable.
   * if none is found, levelstart = numvars (this is used below)  */
  levelstart = 0;
  while (varlevels[levelstart] == 0 && levelstart < numvars) {
    levelstart++;
  }

  retval = 0;
  numreps = 0;

  /* write reports */
  while (currpt[0]) {
    ref = currpt[0];
    coardstime = ref->tim; /* change to meaningful conversion */

    gaprnt(0, "SOI ");
    fwrite(startrec, sizeof(char), 4, outfile);

    gaprnt(0, ">>\t");

    if (sendstnid) {
      snprintf(pout,1255, "stnid: %.8s  ", ref->stid); gaprnt(0, pout);
      fwrite(stnidlen, sizeof(char), 4, outfile);
      fwrite(&(ref->stid), sizeof(char), 8, outfile);
    }
    if (sendlon) {
      snprintf(pout,1255, "lon: %f  ", ref->lon); gaprnt(0, pout);
      outFloat = ref->lon;
      gawgdsval(outfile, &outFloat);
    } 
    if (sendlat) {
      snprintf(pout,1255, "lat: %f  ", ref->lat); gaprnt(0, pout);
      outFloat = ref->lat;
      gawgdsval(outfile, &outFloat);
    }
    if (sendtime) {
      snprintf(pout,1255, "time: %f  ", coardstime); gaprnt(0, pout);
      gawgdstime(outfile, &coardstime);
    }
    gaprnt(0, "\n\t");

    /* level independent data */
    /* write data value and move ptr to next report simultaneously */
    for (i = 0; i < numvars; i++) {
      if (varlevels[i]) continue;
      if (currpt[i] == NULL || 
	  currpt[i]->lat != ref->lat ||
	  currpt[i]->lon != ref->lon ||
	  currpt[i]->tim != ref->tim) {
	gaprnt(0, "error: bad structure in result\n");
	retval = 1;
	goto cleanup;
      }
      if (sendind) {
	snprintf(pout,1255, "[%s: %f]  ", 
		pcm->result[i].stn->pvar->abbrv, currpt[i]->val); 
	gaprnt(0, pout);
	outFloat = currpt[i]->val;
	gawgdsval(outfile, &outFloat);
      }
      currpt[i] = currpt[i]->rpt;
    }

    /* level dependent data */
    /* write data for z-levels until we hit a new lat/lon/time */
    if (levelstart < numvars) { /* if there are level-dep vars */
      while (currpt[levelstart]&& /* and there are still more reports */
	     ref->lat == currpt[levelstart]->lat && /* and we haven't hit a  */
	     ref->lon == currpt[levelstart]->lon && /* new lat/lon/time */
	     ref->tim == currpt[levelstart]->tim) { 
	
	levelref = currpt[levelstart];
	
	gaprnt(0, "\n\tSOI ");
	fwrite(startrec, sizeof(char), 4, outfile);
	
	if (sendlev) {
	  snprintf(pout,1255, "lev: %f  ", levelref->lev); gaprnt(0, pout);
	  outFloat = levelref->lev;
	  gawgdsval(outfile, &outFloat);
	}
	/* write data value and move ptr to next report simultaneously */
	for (i = levelstart; i < numvars; i++) {
	  if (!varlevels[i]) continue;
	  if (currpt[i] == NULL ||
	      currpt[i]->lat != ref->lat ||
	      currpt[i]->lon != ref->lon ||
	      currpt[i]->tim != ref->tim ||
	      currpt[i]->lev != levelref->lev) {
	    gaprnt(0, "error: bad structure in result\n");
	    retval = 1;
	    goto cleanup;
	  }
	  if (senddep) {
	    snprintf(pout,1255, "[%s: %f]  ", pcm->result[i].stn->pvar->abbrv, 
		    currpt[i]->val); gaprnt(0, pout);
	    outFloat = currpt[i]->val;
	    gawgdsval(outfile, &outFloat);
	  }
	  currpt[i] = currpt[i]->rpt;
	}
      }
      gaprnt(0, "\n\tEOS ");
      fwrite(endrec, sizeof(char), 4, outfile);
    } 
    
    gaprnt(0, "\n");
    numreps++;
  }

  /* don't write the final EOS, so that time loops can be concatenated
   * as a single sequence.  */
  /*    gaprnt(0, "EOS\n"); */
  /*    fwrite(endrec, sizeof(char), 4, outfile); */

  snprintf(pout,1255, "WRITEGDS: %d reports x %d vars written as %d records\n",
	  pcm->result[0].stn->rnum, numvars, numreps); gaprnt(0, pout);

cleanup:
  fclose(outfile);
  gree(varlevels,"f293");
  gree(currpt,"f294");
  return;
}


void gadprnt (struct gacmn *pcm) {
struct gastn *stn;
struct garpt *rpt;
struct gagrid *pgr;
gadouble *gr;
gaint siz,i,j,k,lnum;
char *gru;

  if (pcm->type[0] == 1) {          
    /* Data type grid */
    pgr = pcm->result[0].pgr;
    siz = pgr->isiz*pgr->jsiz;
    snprintf(pout,1255,"Printing Grid -- %i Values -- Undef = %g\n", siz, pcm->undef);
    gaprnt(2,pout);
    gr  = pgr->grid;
    gru = pgr->umask;
    lnum = 0;
    for (i=0; i<siz; i++) {
      if (pcm->prstr) {
        if (*gru==0 && pcm->prudef) {
          pout[0]='U'; pout[1]='n'; pout[2]='d'; pout[3]='e'; pout[4]='f'; pout[5]='\0';
        } 
	else if (*gru==0) {
          snprintf(pout,1255,pcm->prstr,pcm->undef);
	}
	else {
          snprintf(pout,1255,pcm->prstr,*gr);
        }
	/* pad with blanks? */
        if (pcm->prbnum>0) {
          j = 0;
          while (pout[j]) j++;
          for (k=0; k<pcm->prbnum; k++) {
            pout[j] = ' ';
            j++;
          }
          pout[j] = '\0';
        }
        gaprnt (2,pout);
      } 
      else {
        if (*gru==0) 
          snprintf(pout,1255,"%g ",pcm->undef);
	else 
	  snprintf(pout,1255,"%g ",*gr);
        gaprnt (2,pout);
      }
      lnum++;
      if (lnum >= pcm->prlnum)  {
        gaprnt (2,"\n");
        lnum = 0;
      }
      gr++; gru++;
    }
    if (lnum>0) gaprnt (2,"\n");
  } 
  else {                           
    /* Data type station */
    stn = pcm->result[0].stn;
    snprintf(pout,1255,"Printing Stations -- %i Reports -- Undef = %g\n", stn->rnum, pcm->undef);
    gaprnt(2,pout);
    rpt = stn->rpt;
    while (rpt) {
	snprintf(pout,1255,"%c%c%c%c%c%c%c%c %-9.4g %-9.4g %-9.4g \n",
	   rpt->stid[0], rpt->stid[1], rpt->stid[2], rpt->stid[3],
           rpt->stid[4], rpt->stid[5], rpt->stid[6], rpt->stid[7],
           rpt->lon,rpt->lat,rpt->lev);
      gaprnt(2,pout);
      if (pcm->prstr) {
        if (rpt->umask==0 && pcm->prudef) {
          pout[0]='U'; pout[1]='n'; pout[2]='d'; pout[3]='e'; pout[4]='f'; pout[5]='\0';
        } 
	else if (rpt->umask==0) {
          snprintf(pout,1255,pcm->prstr,pcm->undef);
	}
	else {
          snprintf(pout,1255,pcm->prstr,rpt->val);
        }
      } else {
        if (rpt->umask==0) 
          snprintf(pout,1255,"%g ",pcm->undef);
	else 
	  snprintf(pout,1255,"%g ",rpt->val);
      }
      gaprnt(2,pout);
      gaprnt(2,"\n");
      rpt = rpt->rpt;
    }
  }
}

/*  Write info and stats on data item to grads output stream */

void gastts (struct gacmn *pcm) {
struct gastn *stn;
struct garpt *rpt;
struct gagrid *pgr;
struct dt dtim;
gadouble (*conv) (gadouble *, gadouble);
gadouble *gr;
gadouble cint,cmin,cmax,rmin,rmax;
gadouble sum,sumsqr,dum,gcntm1;        
gaint i,ucnt,gcnt,gcnto,siz;
char *grumask;
char lab[20];

  /* Grid Data */
  if (pcm->type[0] == 1) {          
    pgr = pcm->result[0].pgr;
    gaprnt(2,"Data Type = grid\n");
    snprintf(pout,1255,"Dimensions = %i %i\n", pgr->idim, pgr->jdim);
    gaprnt(2,pout);
    if (pgr->idim>-1) {
      snprintf(pout,1255,"I Dimension = %i to %i", pgr->dimmin[pgr->idim], pgr->dimmax[pgr->idim]);
      gaprnt(2,pout);
      /* Linear scaling info */
      if (pgr->idim>-1 && pgr->ilinr==1) {     
        gaprnt(2," Linear");
        if (pgr->idim==3) {
          gr2t (pgr->ivals,pgr->dimmin[3],&dtim);
          if (dtim.mn==0) 
	    gat2ch (&dtim,4,lab,20);
          else 
	    gat2ch (&dtim,5,lab,20);
          if (*(pgr->ivals+5)!=0) {
            snprintf(pout,1255," %s %gmo\n",lab,*(pgr->ivals+5));
          } else {
            snprintf(pout,1255," %s %gmn\n",lab,*(pgr->ivals+6));
          }
          gaprnt (2,pout);
        } else {
          conv = pgr->igrab;
          snprintf(pout,1255," %g %g\n",conv(pgr->ivals,pgr->dimmin[pgr->idim]),*(pgr->ivals));
          gaprnt (2,pout);
        }
      }
      /* Levels scaling info */
      if (pgr->idim>-1 && pgr->ilinr!=1) {     
        gaprnt(2," Levels");
        conv = pgr->igrab;
        for (i=pgr->dimmin[pgr->idim]; i<=pgr->dimmax[pgr->idim]; i++) {
          snprintf(pout,1255," %g",conv(pgr->ivals,i));
          gaprnt (2,pout);
        }
        gaprnt (2,"\n");
      }
    } else {
      gaprnt(2,"I Dimension = -999 to -999\n");
    }
    if (pgr->jdim>-1) {
      snprintf(pout,1255,"J Dimension = %i to %i",pgr->dimmin[pgr->jdim],pgr->dimmax[pgr->jdim]);
      gaprnt(2,pout);
      /* Linear scaling info */
      if (pgr->jdim>-1 && pgr->jlinr==1) {     
        gaprnt(2," Linear");
        if (pgr->jdim==3) {
          gr2t (pgr->jvals,pgr->dimmin[3],&dtim);
          if (dtim.mn==0) 
	    gat2ch (&dtim,4,lab,20);
          else 
	    gat2ch (&dtim,5,lab,20);
          if (*(pgr->jvals+5)!=0) {
            snprintf(pout,1255," %s %gmo\n",lab,*(pgr->jvals+5));
          } else {
            snprintf(pout,1255," %s %gmn\n",lab,*(pgr->jvals+6));
          }
          gaprnt (2,pout);
        } else {
          conv = pgr->jgrab;
          snprintf(pout,1255," %g %g\n",conv(pgr->jvals,pgr->dimmin[pgr->jdim]),*(pgr->jvals));
          gaprnt (2,pout);
        }
      }
      /* Levels scaling info */
      if (pgr->jdim>-1 && pgr->jlinr!=1) {     
        gaprnt(2," Levels");
        conv = pgr->jgrab;
        for (i=pgr->dimmin[pgr->jdim]; i<=pgr->dimmax[pgr->jdim]; i++) {
          snprintf(pout,1255," %g",conv(pgr->jvals,i));
          gaprnt (2,pout);
        }
        gaprnt (2,"\n");
      }
    } else {
      gaprnt(2,"J Dimension = -999 to -999\n");
    }
    siz = pgr->isiz*pgr->jsiz;
    snprintf(pout,1255,"Sizes = %i %i %i\n",pgr->isiz,pgr->jsiz,siz);
    gaprnt(2,pout);
    snprintf(pout,1255,"Undef value = %g\n",pcm->undef);
    gaprnt(2,pout);
    ucnt = 0;  gcnt = 0; sum=0; sumsqr=0; 
    gr = pgr->grid;
    grumask = pgr->umask;
    for (i=0; i<siz; i++) {
      if (*(grumask+i)==0) ucnt++;
      else {
	dum = *(gr+i);
	sum += dum;
	sumsqr += dum*dum;
	gcnt++;
      }
    }
    snprintf(pout,1255,"Undef count = %i  Valid count = %i\n",ucnt,gcnt);
    gaprnt(2,pout);
    if (pgr->idim>-1) {
      gamnmx (pgr);
      snprintf(pout,1255,"Min, Max = %g %g\n",pgr->rmin,pgr->rmax);
      gaprnt(2,pout);
      cint = 0.0;
      gacsel (pgr->rmin,pgr->rmax,&cint,&cmin,&cmax);

      if (pgr->jdim==-1) {
        cmin = cmin - cint*2.0;
        cmax = cmax + cint*2.0;
      }
      if (dequal(cint,0.0,1e-12)==0 || dequal(cint,pgr->undef,1e-12)==0) {
        cmin = pgr->rmin-5.0;
        cmax = pgr->rmax+5.0;
        cint = 1.0;
      }
      snprintf(pout,1255,"Cmin, cmax, cint = %g %g %g\n",cmin,cmax,cint);
      gaprnt(2,pout);
      gcntm1=gcnt-1;
      if(gcntm1<=0) gcntm1=1;
      gcnto=gcnt;
      if(gcnt<=0) gcnt=1;
      snprintf(pout,1255,"Stats[sum,sumsqr,root(sumsqr),n]:     %g %g %g %d\n",
	       sum,sumsqr,sqrt(sumsqr),gcnto);
      gaprnt(2,pout);
      snprintf(pout,1255,"Stats[(sum,sumsqr,root(sumsqr))/n]:     %g %g %g\n",
	       sum/gcnt,sumsqr/gcnt,sqrt(sumsqr/gcnt));
      gaprnt(2,pout);
      snprintf(pout,1255,"Stats[(sum,sumsqr,root(sumsqr))/(n-1)]: %g %g %g\n",
	       sum/gcntm1,sumsqr/gcntm1,sqrt(sumsqr/gcntm1));
      gaprnt(2,pout);
      dum=(sumsqr/gcnt)-((sum/gcnt)*(sum/gcnt));
      if(dum>0){
	snprintf(pout,1255,"Stats[(sigma,var)(n)]:     %g %g\n",sqrt(dum),dum);
      } else {
	snprintf(pout,1255,"Stats[(sigma,var)(n)]:     %g %g\n",0.0,0.0);
      }
      gaprnt(2,pout);
      dum=dum*(gcnt/gcntm1);
      if(dum>0) {
	snprintf(pout,1255,"Stats[(sigma,var)(n-1)]:   %g %g\n",sqrt(dum),dum);
      } else {
	snprintf(pout,1255,"Stats[(sigma,var)(n-1)]:   %g %g\n",0.0,0.0);
      }
      gaprnt(2,pout);
    } else {
      snprintf(pout,1255,"Min, Max = %g %g\n",pgr->rmin,pgr->rmin);
      gaprnt(2,pout);
    }

  } else {                           /* Data type station */
    gaprnt(2,"Data Type = station\n");
    stn = pcm->result[0].stn;
    snprintf(pout,1255,"Dimensions = %i %i\n",stn->idim,stn->jdim);
    gaprnt(2,pout);
    if (stn->idim>-1) {
      if (stn->idim!=3) {
        snprintf(pout,1255,"I Dimension = %g to %g\n",stn->dmin[stn->idim],stn->dmax[stn->idim]);
        gaprnt(2,pout);
      } else {
        snprintf(pout,1255,"I Dimension = %i to %i\n",stn->tmin, stn->tmax);
        gaprnt(2,pout);
      }
    } else {
      gaprnt(2,"I Dimension = -999 to -999\n");
    }
    if (stn->jdim>-1) {
      if (stn->jdim!=3) {
        snprintf(pout,1255,"J Dimension = %g to %g\n",stn->dmin[stn->jdim],stn->dmax[stn->jdim]);
        gaprnt(2,pout);
      } else {
        snprintf(pout,1255,"J Dimension = %i to %i\n",stn->tmin, stn->tmax);
        gaprnt(2,pout);
      }
    } else {
      gaprnt(2,"J Dimension = -999 to -999\n");
    }
    snprintf(pout,1255,"Stn count = %i\n",stn->rnum);
    gaprnt(2,pout);
    snprintf(pout,1255,"Undef value = %g\n",pcm->undef);
    gaprnt(2,pout);
    ucnt = 0;  gcnt = 0; sum=0; sumsqr=0;  
    rmin = 9e33;
    rmax = -9e33;
    rpt = stn->rpt;

    while (rpt) {
      if (rpt->umask==0) ucnt++;
      else {
        gcnt++;
	dum = rpt->val;
        sum += dum;
	sumsqr += dum*dum;
        if (rpt->val<rmin) rmin = rpt->val;
        if (rpt->val>rmax) rmax = rpt->val;
      }
      rpt = rpt->rpt;
    }
    gcntm1 = gcnt-1;
    if (gcntm1 <= 0) gcntm1=1;
    if (gcnt==0) {
      rmin = pcm->undef;
      rmax = pcm->undef;
    }

    snprintf(pout,1255,"Undef count = %i  Valid count = %i \n",ucnt,gcnt);
    gaprnt(2,pout);
    snprintf(pout,1255,"Min, Max = %g %g\n",rmin,rmax);
    gaprnt(2,pout);
    cint = 0.0;

    gacsel (rmin,rmax,&cint,&cmin,&cmax);
    if (stn->jdim==-1) {
      cmin = cmin - cint*2.0;
      cmax = cmax + cint*2.0;
    }
    if (dequal(cint,0.0,1e-12)==0 || dequal(cint,stn->undef,1e-12)==0) {
      cmin = rmin-5.0;
      cmax = rmax+5.0;
      cint = 1.0;
    }
    snprintf(pout,1255,"Cmin, cmax, cint = %g %g %g\n",cmin,cmax,cint);
    gaprnt(2,pout);

    gcntm1 = gcnt-1;
    if(gcntm1 <= 0) gcntm1=1;
    gcnto = gcnt;
    if(gcnt <= 0) gcnt=1;
    snprintf(pout,1255,"Stats[sum,sumsqr,root(sumsqr),n]:     %g %g %g %d\n",
	     sum,sumsqr,sqrt(sumsqr),gcnto);
    gaprnt(2,pout);
    snprintf(pout,1255,"Stats[(sum,sumsqr,root(sumsqr))/n)]:     %g %g %g\n",
	     sum/gcnt,sumsqr/gcnt,sqrt(sumsqr/gcnt));
    gaprnt(2,pout);
    snprintf(pout,1255,"Stats[(sum,sumsqr,root(sumsqr))/(n-1))]: %g %g %g\n",
	     sum/gcntm1,sumsqr/gcntm1,sqrt(sumsqr/gcntm1));
    gaprnt(2,pout);
    dum=(sumsqr/gcnt)-((sum/gcnt)*(sum/gcnt));
    if(dum>0){
      snprintf(pout,1255,"Stats[(sigma,var)(n)]:     %g %g\n",sqrt(dum),dum);
    } else {
      snprintf(pout,1255,"Stats[(sigma,var)(n)]:     %g %g\n",0.0,0.0);
    }
    gaprnt(2,pout);
    dum=dum*(gcnt/gcntm1);
    if(dum>0) {
      snprintf(pout,1255,"Stats[(sigma,var)(n-1)]:   %g %g\n",sqrt(dum),dum);
    } else {
      snprintf(pout,1255,"Stats[(sigma,var)(n-1)]:   %g %g\n",0.0,0.0);
    }
    gaprnt(2,pout);

    if(pcm->stnprintflg) {
      snprintf(pout,1255,"Printing station values:  #obs = %d\n",gcnt);
      gaprnt(2,pout);
      gcnt=0;
      ucnt=0;
      rpt = stn->rpt;
      snprintf(pout,1255,"OB    ID       LON      LAT      LEV      VAL\n");
      gaprnt(2,pout);
      while (rpt) {
	if (rpt->umask==0) ucnt++;
	else {
	  gcnt++;
	  snprintf(pout,1255,"%-5i %.8s %-8.6g %-8.6g %-8.6g %-8.6g\n",
		   gcnt,rpt->stid,rpt->lon,rpt->lat,rpt->lev,rpt->val);
	  gaprnt(2,pout);
	}
	rpt = rpt->rpt;
      }
    }
  }
  gagsav(22,pcm,NULL);
}

/*  Special routine -- find closest station to an X,Y position. */

void gafstn (struct gacmn *pcm) {
struct gastn *stn;
struct garpt *rpt, *srpt;
struct gagrid *pgr;
gadouble x,y,r,d,xpos,ypos,rlon;

  if (pcm->numgrd<3 || pcm->type[0]!=0 || pcm->type[1]!=1 || pcm->type[2]!=1) {
    gaprnt (0,"Error: Invalid data types for findstn\n");
    return;
  }

  gamscl (pcm);       /* Do map level scaling */
  stn = pcm->result[0].stn;
  pgr = pcm->result[1].pgr;
  xpos = pgr->rmin;
  pgr = pcm->result[2].pgr;
  ypos = pgr->rmin;
  rpt = stn->rpt;
  srpt = NULL;
  r = 1.0e30;
  while (rpt) {
    rlon = rpt->lon;
    if (rlon<pcm->dmin[0]) rlon+=360.0;
    if (rlon>pcm->dmax[0]) rlon-=360.0;
    gxconv (rlon,rpt->lat,&x,&y,2);
    d = hypot(x-xpos,y-ypos);
    if (d<r) {
      r = d;
      srpt = rpt;
    }
    rpt = rpt->rpt;
  }
  if (srpt) {
    srpt->stid[7] = '\0';
    snprintf(pout,1255,"%s %g %g %g\n",srpt->stid,srpt->lon,srpt->lat,r);
    gaprnt(2,pout);
  } else gaprnt (2,"No stations found\n");
  gagsav (21,pcm,NULL);
}

/* Plot weather symbols at station locations. */

void gawsym (struct gacmn *pcm) {
struct gastn *stn;
struct garpt *rpt;
gadouble rlon,x,y,scl;
gaint i;

  gamscl (pcm);       /* Do map level scaling */
  gawmap (pcm, 1);    /* Draw map */
  pcm->xdim = 0; pcm->ydim = 1;
  gafram (pcm);
  gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);

  stn = pcm->result[0].stn;
  rpt = stn->rpt;
  gxwide (pcm->cthick);
  if (pcm->ccolor<0) gxcolr(1);
  else gxcolr (pcm->ccolor);
  while (rpt!=NULL) {
    if (rpt->umask != 0) {
      rlon = rpt->lon;
      if (rlon<pcm->dmin[0]) rlon+=360.0;
      if (rlon>pcm->dmax[0]) rlon-=360.0;
      if (rlon>pcm->dmin[0] && rlon<pcm->dmax[0] &&
             rpt->lat>pcm->dmin[1] && rpt->lat<pcm->dmax[1]) {
        i = (gaint)(rpt->val+0.1);
        if (i>0 && i<42) {
          gxconv (rlon,rpt->lat,&x,&y,2);
          scl = pcm->digsiz*1.5;
          gxwide (pcm->cthick);
          if (pcm->wxopt==1) {
            wxsym (i, x, y, scl, pcm->ccolor, pcm->wxcols);
          } else {
            gxmark (i,x,y,scl);
          }
        }
      }
    }
    rpt = rpt->rpt;
  }
  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  gxcolr (pcm->anncol);
  gxwide (4);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gxstyl(1);
  gaaxpl(pcm,0,1);
  gagsav(12,pcm,NULL);
}

/* Plot colorized markers at station locations */

void gasmrk (struct gacmn *pcm) {
struct gastn *stn;
struct garpt *rpt;
gadouble rlon,x,y,cwid,sizstid;
gaint i,len,icnst,cnt,bcol; 
char lab[20];

  gamscl (pcm);       /* Do map level scaling */
  gawmap (pcm, 1);    /* Draw map */
  pcm->xdim = 0; pcm->ydim = 1;
  gafram (pcm);
  gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);

  stn = pcm->result[0].stn;
  gasmnmx (stn);

  if (dequal(stn->smin,stn->undef,1e-12)==0 || 
      dequal(stn->smax,stn->undef,1e-12)==0) return;
  gaselc (pcm,stn->smin,stn->smax);
  rpt = stn->rpt;
  gxwide (pcm->cthick);

  icnst=0;
  if (dequal(stn->smin,stn->smax,1e-12)==0) icnst=1;
  if (pcm->ccolor<0 && icnst) {
    pcm->ccolor=1;
    gxcolr(pcm->ccolor);
  } else if (pcm->ccolor<0) {
    gxcolr(1);
  } else {
    gxcolr (pcm->ccolor);
  }

  cnt = 0;
  sizstid=pcm->digsiz*0.65;

  while (rpt!=NULL) {
    cnt++;
    if (rpt->umask != 0) {
      rlon = rpt->lon;
      if (rlon<pcm->dmin[0]) rlon+=360.0;
      if (rlon>pcm->dmax[0]) rlon-=360.0;
      if (rlon>=pcm->dmin[0] && rlon<=pcm->dmax[0] &&
	  rpt->lat>=pcm->dmin[1] && rpt->lat<=pcm->dmax[1]) {
        gxconv (rlon,rpt->lat,&x,&y,2);
        i = gashdc (pcm,rpt->val);
	/* if constant grid, use user ccolor */
	if (icnst) {
	  gxcolr(pcm->ccolor);
	} else {
	  gxcolr (i);
	}
        gxmark (pcm->cmark,x,y,pcm->digsiz*0.5);
	/* JMA use Cairo to draw open circle -- won't work with X11/GD */
        /* if (pcm->cmark == 2) */
	/*   gxcirc(x,y,pcm->digsiz*0.25,0); /\* open circle *\/ */
	/* else if (pcm->cmark == 3) */
	/*   gxcirc(x,y,pcm->digsiz*0.25,1); /\* closed circle *\/ */
	/* else */
	/*   gxmark (pcm->cmark,x,y,pcm->digsiz*0.5); */

        /* stn id plot */
	if (pcm->stidflg) {
	  gxmark (1,x,y,pcm->digsiz*0.5);
	  getwrd (lab,rpt->stid,8);
          len = strlen(lab);
          cwid = 0.1;
          gxchln (lab,len,sizstid,&cwid);
          x = x-cwid*0.5;
          y = y-(sizstid*1.7);
          if (pcm->ccolor!=0) {
	    bcol = gxdbkq();
	    /* If bcol is neither black nor white, leave it alone. Otherwise, set to 0 for 'background' */
	    if (bcol<2) bcol=0; 
	    gxcolr (bcol);
            gxrecf (x-0.01,x+cwid+0.01,y-0.01,y+sizstid+0.01);
          }
          if (pcm->ccolor<0) gxcolr(1);
          else gxcolr (pcm->ccolor);
          gxchpl (lab,len,x,y,sizstid,sizstid,0.0);
	}

      }
    }
    rpt = rpt->rpt;
  }
  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  gxcolr(pcm->anncol);
  gxwide(4);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gxstyl(1);
  gaaxpl(pcm,0,1);
  gagsav(14,pcm,NULL);
}

/* Plot station values as either one number centered on the station
   location or as two numbers above and below a crosshair, or as
   a wind barb if specified by user */

void gapstn (struct gacmn *pcm) {
struct gastn *stn, *stn2;
struct garpt *rpt, *rpt2;
gadouble x,y,rlon;
gadouble dir,spd,umax,vmax,vscal=0.0,cwid;
gaint len,flag,hemflg,bcol;
char lab[20];

  gamscl (pcm);       /* Do map level scaling */
  gawmap (pcm, 1);    /* Draw map */
  pcm->xdim = 0;
  pcm->ydim = 1;
  gafram(pcm);
  gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);

  /* Plot barb or vector at station location */

  if (pcm->numgrd>1 && (pcm->goutstn==2 || pcm->goutstn==6)) {
    stn = pcm->result[0].stn;
    stn2 = pcm->result[1].stn;
    if (pcm->goutstn==6) {            /* Get vector scaling */
      if (!pcm->arrflg) {
        rpt = stn->rpt;
        umax = -9.99e33;
        while (rpt) {
          if (umax<fabs(rpt->val)) umax = fabs(rpt->val);
          rpt = rpt->rpt;
        }
        rpt = stn2->rpt;
        vmax = -9.99e33;
        while (rpt) {
          if (vmax<fabs(rpt->val)) vmax = fabs(rpt->val);
          rpt = rpt->rpt;
        }
        vscal = hypot(umax,vmax);
        x = floor(log10(vscal));
        y = floor(vscal/pow(10.0,x));
        vscal = y * pow(10.0,x);
        pcm->arrsiz = 0.5;
        pcm->arrmag = vscal;
      } else {
        vscal = pcm->arrmag;
      }
      pcm->arrflg = 1;
    }
    rpt = stn->rpt;
    if (pcm->ccolor<0) gxcolr(1);
    else gxcolr (pcm->ccolor);
    gxwide (pcm->cthick);
    while (rpt!=NULL) {
      if (rpt->umask != 0) {
        rpt2 = stn2->rpt;
        while (rpt2!=NULL) {
          if (rpt2->umask != 0 && 
	      dequal(rpt->lat,rpt2->lat,1e-12)==0 &&
              dequal(rpt->lon,rpt2->lon,1e-12)==0) {
            rlon = rpt->lon;
            if (rlon<pcm->dmin[0]) rlon+=360.0;
            if (rlon>pcm->dmax[0]) rlon-=360.0;
            if (rlon>pcm->dmin[0] && rlon<pcm->dmax[0] &&
                rpt->lat>pcm->dmin[1] && rpt->lat<pcm->dmax[1]) {
              gxconv (rlon,rpt->lat,&x,&y,2);
              if (dequal(rpt2->val,0.0,1e-12)==0 && dequal(rpt->val,0.0,1e-12)==0) 
		dir = 0.0;
              else {
		dir = gxaarw(rpt->lon,rpt->lat);
                if (dir<-900.0) {
                  gaprnt(0,"Error: vector/barb not compatible with the current map projection\n");
                  return;
                }
		dir = dir + atan2(rpt2->val,rpt->val);
              }
              spd = hypot(rpt->val,rpt2->val);
              if (pcm->goutstn==2) {
                hemflg = 0;
                if (pcm->hemflg == 1) hemflg = 1;
                else if (pcm->hemflg == 0) hemflg = 0;
                else if (rpt->lat<0.0) hemflg = 1;
                gabarb (x, y, pcm->digsiz*3.5, pcm->digsiz*2.0,
			pcm->digsiz*0.25, dir, spd, hemflg, pcm->barbolin);
                gxmark (2,x,y,pcm->digsiz*0.5);
              } else {
                if (vscal>0.0) {
                  gaarrw (x, y, dir, pcm->arrsiz*spd/vscal, pcm->ahdsiz);
                } else {
                  gaarrw (x, y, dir, pcm->arrsiz, pcm->ahdsiz);
                }
              }
            }
            break;
          }
          rpt2 = rpt2->rpt;
        }
      }
      rpt = rpt->rpt;
    }

  /* Plot number at station location */

  } else {
    gxwide (pcm->cthick);
    stn = pcm->result[0].stn;
    rpt = stn->rpt;
    flag=0;
    if (pcm->numgrd>1 || pcm->stidflg) flag = 1;
    while (rpt!=NULL) {
      if (rpt->umask != 0) {
        rlon = rpt->lon;
        if (rlon<pcm->dmin[0]) rlon+=360.0;
        if (rlon>pcm->dmax[0]) rlon-=360.0;
        if (rlon>pcm->dmin[0] && rlon<pcm->dmax[0] &&
            rpt->lat>pcm->dmin[1] && rpt->lat<pcm->dmax[1]) {
          gxconv (rlon,rpt->lat,&x,&y,2);
          if (flag) gxmark (1,x,y,pcm->digsiz*0.5);
          snprintf(lab,19,"%.*f",pcm->dignum,(gafloat)rpt->val);
          len = strlen(lab);
          cwid = 0.1;
          gxchln (lab,len,pcm->digsiz,&cwid);
          x = x-cwid*0.5;
          if (flag) {
            y = y+(pcm->digsiz*0.7);
          } else {
            y = y-(pcm->digsiz*0.5);
          }
          if (pcm->ccolor!=0) {
	    bcol = gxdbkq();
	    /* If bcol is neither black nor white, leave it alone. Otherwise, set to 0 for 'background' */
	    if (bcol<2) bcol=0; 
	    gxcolr (bcol);
            gxrecf (x-0.01,x+cwid+0.01,y-0.01,y+pcm->digsiz+0.01);
          }
          if (pcm->ccolor<0) gxcolr(1);
          else gxcolr (pcm->ccolor);
          gxchpl (lab,len,x,y,pcm->digsiz,pcm->digsiz,0.0);
        }
      }
      rpt=rpt->rpt;
    }
    if ( (flag && pcm->type[1]==0) || pcm->stidflg) {
      if (!pcm->stidflg) stn = pcm->result[1].stn;
      rpt = stn->rpt;
      while (rpt!=NULL) {
	if (rpt->umask != 0) {
	  rlon = rpt->lon;
	  if (rlon<pcm->dmin[0]) rlon+=360.0;
	  if (rlon>pcm->dmax[0]) rlon-=360.0;
	  if (rlon>pcm->dmin[0] && rlon<pcm->dmax[0] &&
	      rpt->lat>pcm->dmin[1] && rpt->lat<pcm->dmax[1]) {
	    gxconv (rlon,rpt->lat,&x,&y,2);
	    gxmark (1,x,y,pcm->digsiz*0.5);
	    if (pcm->stidflg) {
	      getwrd (lab,rpt->stid,8);
	    } else {
	      snprintf(lab,19,"%.*f",pcm->dignum,(gafloat)rpt->val); 
	    }
	    len = strlen(lab);
	    cwid = 0.1;
	    gxchln (lab,len,pcm->digsiz,&cwid);
	    x = x-cwid*0.5;
	    y = y-(pcm->digsiz*1.7);
	    if (pcm->ccolor!=0) {
	      bcol = gxdbkq();
	      /* If bcol is neither black nor white, leave it alone. Otherwise, set to 0 for 'background' */
	      if (bcol<2) bcol=0; 
	      gxcolr (bcol);
	      gxrecf (x-0.01,x+cwid+0.01,y-0.01,y+pcm->digsiz+0.01);
	    }
	    if (pcm->ccolor<0) gxcolr(1);
	    else gxcolr (pcm->ccolor);
	    gxchpl (lab,len,x,y,pcm->digsiz,pcm->digsiz,0.0);
	  }
	}
        rpt=rpt->rpt;
      }
    }
  }
  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  gxcolr(pcm->anncol);
  gxwide(4);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gxstyl(1);
  gaaxpl(pcm,0,1);
  gagsav (10,pcm,NULL);
}

/*  Draw wind barb at location x, y, direction dir, speed
    spd, with barb lengths of blen, pointer length of plen
    (before barbs added), starting at rad distance from the
    center point x,y.  If calm, a circle is drawn at rad*1.2
    radius.  Direction is direction wind blowing towards.  */

void gabarb (gadouble x, gadouble y, gadouble plen, gadouble blen, gadouble rad,
	     gadouble dir, gadouble spd, gaint hemflg, gaint fillflg) {
gadouble bgap,padd,var,a70,cosd70,sind70;
gadouble xp1,yp1,xp2,yp2,xp3,yp3,xy[10];
gaint flag,flg2;

  dir = dir + pi;   /* Want direction wind blowing from */

  /* Calculate added length due to barbs */

  bgap = blen*0.3;
  padd = 0.0;
  var = spd;
  if (var<0.01) {
    rad*=2.0;
    if (rad+blen*0.3>rad*1.4) gxmark(2,x,y,rad+blen*0.3);
    else gxmark (2,x,y,rad*1.4);
    return;
  } else {
    if (var<2.5) padd = bgap;
    while (var>=47.5) { var-=50.0; padd+=bgap*1.6;}
    while (var>=7.5)  { var-=10.0; padd+=bgap;}
    if (var>=2.5) padd+=bgap;
  }
  plen+=padd;

  /* Draw pointer */

  xp1 = x + plen*cos(dir);
  yp1 = y + plen*sin(dir);
  xp2 = x + rad*cos(dir);
  yp2 = y + rad*sin(dir);
  gxplot (xp2,yp2,3);
  gxplot (xp1,yp1,2);

  /* Start out at the end of the pointer and add barbs
     til we run out of barbs to add.  */

  var = spd;
  a70 = 70.0*pi/180.0;
  if (hemflg) {
    cosd70 = cos(dir+a70);
    sind70 = sin(dir+a70);
  } else {
    cosd70 = cos(dir-a70);
    sind70 = sin(dir-a70);
  }
  flag = 1;
  flg2 = 0;
  while (var>=47.5) {
    xp1 = x + plen*cos(dir);
    yp1 = y + plen*sin(dir);
    xp2 = xp1 + blen*cosd70;
    yp2 = yp1 + blen*sind70;
    xp3 = x + (plen-bgap*1.45)*cos(dir);
    yp3 = y + (plen-bgap*1.45)*sin(dir);
    /* draw the pennant outline */
    gxplot (xp1,yp1,3);
    gxplot (xp2,yp2,2);
    gxplot (xp3,yp3,2);
    if (fillflg) {
      /* fill in the pennant flag */
      xy[0] = xp1; xy[1] = yp1;
      xy[2] = xp2; xy[3] = yp2;
      xy[4] = xp3; xy[5] = yp3;
      xy[6] = xp1; xy[7] = yp1;
      gxfill(xy,4);
    } 
    plen = plen - bgap*1.6;
    var-=50.0;
    flg2 = 1;
  }
  while (var>=7.5) {
    if (flg2) {plen-=bgap*0.7; flg2 = 0;}
    xp1 = x + plen*cos(dir);
    yp1 = y + plen*sin(dir);
    xp2 = xp1 + blen*cosd70;
    yp2 = yp1 + blen*sind70;
    gxplot (xp1,yp1,3);
    gxplot (xp2,yp2,2);
    plen-=bgap;
    var-=10.0;
    flag = 0;
  }
  if (var>=2.5) {
    if (flag) plen-=bgap;
    xp1 = x + plen*cos(dir);
    yp1 = y + plen*sin(dir);
    xp2 = xp1 + 0.5*blen*cosd70;
    yp2 = yp1 + 0.5*blen*sind70;
    gxplot (xp1,yp1,3);
    gxplot (xp2,yp2,2);
  }
  return;
}

/* Plot station model using station data.  Complexity of model
   depends on the number of result objects available.
   First two are assumed to be u and v winds, which are required
   for the station model to be plotted. */

void gapmdl (struct gacmn *pcm) {
struct gastn *stn, *stn1;
struct garpt *rpt, *rpt1;
struct gagrid *pgr;
gadouble vals[10];
char udefs[10];
gaint i,num,rc;

  gamscl (pcm);       /* Do map level scaling */
  gawmap (pcm, 1);    /* Draw map */
  pcm->xdim = 0;
  pcm->ydim = 1;
  gafram (pcm);
  gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);

  num = pcm->numgrd;
  if (num>10) num=10;

  /* Check validity of data -- single value (shows up as a grid) indicates
     not to plot that variable */

  for (i=0; i<pcm->numgrd; i++) {
    if (pcm->type[i]==1) {
      pgr = pcm->result[i].pgr;
      if (i==0 || pgr->idim!=-1) {
        gaprnt (0,"Invalid data:  Station data required\n");
        return;
      }
    }
  }

  if (pcm->ccolor<0) gxcolr(1);
  else gxcolr (pcm->ccolor);
  gxwide (pcm->cthick);

  /* Get info for each station  -- match station ids accross all the 
     variables which show up as separate result items.*/

  stn1 = pcm->result[0].stn;
  rpt1 = stn1->rpt;
  while (rpt1) {
    if (rpt1->umask == 0) {
      udefs[0] = 0;
    } else {
      vals[0] = rpt1->val;
      udefs[0] = 1;
      for (i=1; i<pcm->numgrd; i++) {
        if (pcm->type[i]!=1) {
          stn = pcm->result[i].stn;
          rpt = stn->rpt;
          while (rpt) {
            if (dequal(rpt->lon,rpt1->lon,1e-12)==0 && 
                dequal(rpt->lat,rpt1->lat,1e-12)==0 &&
                cmpch(rpt->stid,rpt1->stid,8)==0) break;
            rpt = rpt->rpt;
          }
          if (rpt && rpt->umask==1) {
            vals[i] = rpt->val;
            udefs[i] = 1;
          }
          else udefs[i] = 0;
        }
        else udefs[i] = 0;
      }
    }
    rc = gasmdl (pcm,rpt1,vals,udefs);
    if (rc) return;
    rpt1 = rpt1->rpt;
  }

  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  gxcolr(pcm->anncol);
  gxwide(4);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gxstyl(1);
  gaaxpl(pcm,0,1);
  gagsav (11,pcm,NULL);
}

/* Plot an individual station model */

gaint gasmdl (struct gacmn *pcm, struct garpt *rpt, gadouble *vals, char *udefs) {
gaint num,icld,i,col,ivis,itop,ibot,ii,hemflg;
gadouble x,y;
gadouble spd,dir,roff,scl,rad,t,digsiz,msize,plen,wrad;
gadouble xlo[7],xhi[7],ylo[7],yhi[7],orad,xv,yv;
char ch[20],len;

  col = pcm->ccolor;
  if (pcm->ccolor<0) col = 1;
  gxcolr(col);
  num = pcm->numgrd;

  /* If no winds, just return */
  if ( *udefs==0 || *(udefs+1)==0) return(0);

  /* Plot */
  gxconv (rpt->lon,rpt->lat,&x,&y,2);
  hemflg = 0;
  if (pcm->hemflg == 1) hemflg = 1;
  else if (pcm->hemflg == 0) hemflg = 0;
  else if (rpt->lat<0.0) hemflg = 1;

  icld = (gaint)(floor(*(vals+6)+0.1));
  digsiz = pcm->digsiz;
  if (icld>19 && icld<26) msize = digsiz*1.4;
  else msize = digsiz;
  wrad = msize*0.5;
  if (dequal(*vals,0.0,1e-12)==0 && dequal(*(vals+1),0.0,1e-12)==0) 
    rad = msize*0.8;
  else 
    rad = msize*0.65;

  /* Plot clouds: 20 = clr, 21 = sct, 22 = bkn, 23 = ovc, 24 = obsc, 25 = missng */
  icld = (gaint)(floor(*(vals+6)+0.1));
  if (col==0 && icld>19) {
    if (icld!=23) gxmark(3,x,y,msize);
  } else {
    if (icld==20) gxmark(2,x,y,msize);
    else if (icld==21) gxmark(10,x,y,msize);
    else if (icld==22) gxmark(11,x,y,msize);
    else if (icld==23) gxmark(3,x,y,msize);
    else if (icld==24) {
      gxmark(2,x,y,msize);
      roff = msize/2.8284;
      gxplot (x-roff,y+roff,3);
      gxplot (x+roff,y-roff,2);
      gxplot (x+roff,y+roff,3);
      gxplot (x-roff,y-roff,2);
    }
    else if (icld==25) {
      gxmark(2,x,y,msize);
      roff = msize*0.3;
      gxchpl ("`0M",3,x-roff*1.2,y-roff*0.9,roff*2.0,roff*2.0,0.0);
    }
    else {
      if (icld>0 && icld<9) gxmark(icld,x,y,msize);
      else gxmark(2,x,y,msize);
    }
  }

  /* Plot wxsym */
  gxwide (pcm->cthick+1);
  xlo[0] = -999.9;
  if (*(udefs+7)) {
    i = (gaint)(*(vals+7)+0.1);
    if (i>0&&i<40) {
      scl = digsiz*3.0;
      xhi[0] = x - rad*1.1;
      xlo[0] = xhi[0] - (sxwid[i-1]*scl);
      ylo[0] = y + symin[i-1]*scl;
      yhi[0] = y + symax[i-1]*scl;
      wxsym (i,xlo[0]+(sxwid[i-1]*scl*0.5),y,scl,pcm->ccolor,pcm->wxcols);
      gxcolr(col);
    }
  }

  /* Plot visibility */
  gxwide(pcm->cthick);
  xlo[5] = -999.9;
  if (*(udefs+8)) {
    i = (gaint)((*(vals+8)*32.0)+0.5);
    ivis = i/32;
    itop = i - ivis*32;
    ibot = 32;
    while (itop!=0) {
      i = itop/2;
      if (i*2!=itop) break;
      itop = i;
      ibot = ibot/2;
    }
    yv = y-digsiz*0.5;
    if (xlo[0]<-990.0) xv = x - rad*1.6;
    else  xv = xlo[0] - digsiz*0.2;
    xhi[5] = xv;
    ylo[5] = yv;
    yhi[5] = yv+digsiz;
    if (itop>0) {
      snprintf(ch,19,"%i",ibot);
      len = 0;
      while (*(ch+len)) len++;
      xv = xv - ((gadouble)len)*digsiz*0.65;
      gxchpl (ch,len,xv,yv,digsiz*0.65,digsiz*0.65,0.0);
      snprintf(ch,19,"%i",itop);
      len = 0;
      while (*(ch+len)) len++;
      gxplot (xv-digsiz*0.4,yv,3);
      gxplot (xv+digsiz*0.1,yv+digsiz,2);
      xv = xv - ((gadouble)len+0.4)*digsiz*0.65;
      gxchpl (ch,len,xv,yv+digsiz*0.35,digsiz*0.65,digsiz*0.65,0.0);
    }
    if (ivis>0 || (ivis==0 && itop==0)) {
      snprintf(ch,19,"%i",ivis);
      len = 0;
      while (*(ch+len)) len++;
      xv = xv - ((gadouble)len)*digsiz;
      gxchpl (ch,len,xv,yv,digsiz,digsiz,0.0);
    }
    xlo[5] = xv;
  }

  /* Plot temperature */
  xlo[1] = -999.9;
  if (*(udefs+2)) {
    snprintf(ch,19,"%.*f",pcm->dignum,(gafloat)(*(vals+2)));
    len = 0;
    while (*(ch+len)) len++;
    if (xlo[0]>-999.0) ylo[1] = yhi[0]+digsiz*0.4;
    else if (xlo[5]>-999.0) ylo[1] = yhi[5]+digsiz*0.4;
    else ylo[1] = y + digsiz*0.3;
    t = ylo[1]-y;
    t = rad*rad - t*t;
    if (t<=0.0) xhi[1] = x-digsiz*0.3;
    else {
      t = sqrt(t);
      if (t<digsiz*0.3) t = digsiz*0.3;
      xhi[1] = x-t;
    }
    xlo[1] = xhi[1] - (gadouble)len * digsiz;
    yhi[1] = ylo[1] + digsiz;
    gxchpl (ch,len,xlo[1],ylo[1],digsiz,digsiz,0.0);
  }

  /* Plot dewpoint */
  xlo[2] = -999.9;
  if (*(udefs+3)) {
    snprintf(ch,19,"%.*f",pcm->dignum,(gafloat)*(vals+3));
    len = 0;
    while (*(ch+len)) len++;
    if (xlo[0]>-999.0) yhi[2] = ylo[0]-digsiz*0.4;
    else if (xlo[5]>-999.0) yhi[2] = ylo[5]-digsiz*0.4;
    else yhi[2] = y - digsiz*0.3;
    t = y - yhi[2];
    t = rad*rad - t*t;
    if (t<=0.0) xhi[2] = x-digsiz*0.3;
    else {
      t = sqrt(t);
      if (t<digsiz*0.3) t = digsiz*0.3;
      xhi[2] = x-t;
    }
    xlo[2] = xhi[2] - (gadouble)len * digsiz;
    ylo[2] = yhi[2] - digsiz;
    gxchpl (ch,len,xlo[2],ylo[2],digsiz,digsiz,0.0);
  }

  /* Plot Pressure */
  xlo[3] = -999.9;
  if (*(udefs+4)) {
    i = (gaint)(*(vals+4)+0.5);
    ii = 0;
    if (pcm->mdldig3) {
      snprintf(ch,19,"%i",i+10000);
      len = 0;
      while (*(ch+len)) len++;
      ii = len-3;
      len = 3;
    } else {
      snprintf(ch,19,"%.*f",pcm->dignum,(gafloat)*(vals+4));
      len = 0;
      while (*(ch+len)) len++;
    }
    xlo[3] = x + rad*0.87;
    ylo[3] = y + rad*0.5;
    xhi[3] = xlo[3] + digsiz*(gadouble)len;
    yhi[3] = ylo[3] + digsiz;
    gxchpl (ch+ii,len,xlo[3],ylo[3],digsiz,digsiz,0.0);
  }

  /* Plot pressure tendency */
  xlo[4] = -999.9;
  if (*(udefs+5)) {
    if (*(vals+5)>0.0) snprintf(ch,19,"+%.*f",pcm->dignum,(gafloat)*(vals+5));
    else snprintf(ch,19,"%.*f",pcm->dignum,(gafloat)*(vals+5));
    len = 0;
    while (*(ch+len)) len++;
    xlo[4] = x + rad;
    if (xlo[3]>-990.0) yhi[4] = ylo[3]-digsiz*0.4;
    else yhi[4] = y + digsiz*0.5;
    xhi[4] = xlo[4] + digsiz*(gadouble)len;
    ylo[4] = yhi[4] - digsiz;
    gxchpl (ch,len,xlo[4],ylo[4],digsiz,digsiz,0.0);
  }

  /* plot stid lower right */
  xlo[6] = -999.9;
  if (pcm->stidflg) {
    len = 4;
    if (xlo[4]>-990.0) yhi[6] = ylo[4]-digsiz*0.4;
    else yhi[6] = y - rad;
    if (xlo[2]>-990.0) xlo[6] = xhi[2]+0.6*digsiz;
    else xlo[6] = x - 1.2*digsiz;
    xhi[6] = xlo[6] + 0.6*digsiz*(gadouble)len;
    ylo[6] = yhi[6] - 0.6*digsiz;
    gxchpl (rpt->stid,len,xlo[6],ylo[6],0.6*digsiz,0.6*digsiz,0.0);
  }

  /* Plot wind barb */
  if (dequal(*vals,0.0,1e-12)==0 && dequal(*(vals+1),0.0,1e-12)==0) {
    dir = 0.0;
  } else {
    dir = gxaarw(rpt->lon,rpt->lat);
    if (dir<-900.0) {
      gaprnt(0,"Error: vector/barb not compatible with current map projection\n");
      return (1);
    }
    dir = dir + atan2(*(vals+1),*vals);
  }
  spd = hypot(*vals,*(vals+1));
  orad = wndexit (spd*cos(dir), spd*sin(dir), x, y, rad, xlo, xhi, ylo, yhi);
  if (orad<-990.0) {
    plen = pcm->digsiz*3.5;
  } else {
    plen = pcm->digsiz*0.5+orad;
    if (plen<pcm->digsiz*3.5) plen = pcm->digsiz*3.5;
    if (plen>pcm->digsiz*6.0) plen = orad;
    wrad = orad;
  }
  gabarb (x, y, plen, pcm->digsiz*2.5, wrad, dir, spd, hemflg, pcm->barbolin);
  return(0);
}

/* Find exit radius for the wind barb */

gadouble wndexit (gadouble uu, gadouble vv, gadouble x, gadouble y, gadouble rad,
                     gadouble *xlo, gadouble *xhi, gadouble *ylo, gadouble *yhi) {
gadouble u,v,xn,yn,orad,fuzz,fuzz2;

  u = -1.0*uu;
  v = -1.0*vv;
  orad = -999.9;
  fuzz = rad*0.25;
  fuzz2 = fuzz*0.5;

  if (dequal(u,0.0,1e-12)==0 && dequal(v,0.0,1e-12)==0) return(orad);
  if (u<0.0) {
    if (v>0.0 && xlo[1]>-990.0) {
      yhi[1] = yhi[1]+fuzz;
      xn = x + (yhi[1]-y)*u/v;
      if (xn>xlo[1]-fuzz && xn<xhi[1]+fuzz2) {
        orad = hypot(xn-x, yhi[1]-y);
        return (orad);
      }
      xlo[1] = xlo[1] - fuzz2;
      yn = y + (xlo[1]-x)*v/u;
      if (yn>ylo[1]-fuzz && yn<yhi[1]+fuzz) {
        orad = hypot(xlo[1]-x,yn-y);
        return (orad);
      }
    }
    if (v<0.0 && xlo[2]>-990.0) {
      ylo[2] = ylo[2]-fuzz;
      xn = x + (ylo[2]-y)*u/v;
      if (xn>xlo[2]-fuzz && xn<xhi[2]+fuzz2) {
        orad = hypot(xn-x, ylo[2]-y);
        return (orad);
      }
      xlo[2] = xlo[2] - fuzz2;
      yn = y + (xlo[2]-x)*v/u;
      if (yn>ylo[2]-fuzz && yn<yhi[2]+fuzz) {
        orad = hypot(xlo[2]-x,yn-y);
        return (orad);
      }
    }
    if (xlo[5]>-990.0) {
      xlo[5] = xlo[5] - fuzz2;
      yn = y + (xlo[5]-x)*v/u;
      if (yn>ylo[5]-fuzz && yn<yhi[5]+fuzz) {
        orad = hypot(xlo[5]-x,yn-y);
        return (orad);
      }
      if (v>0.0) {
        yhi[5] = yhi[5]+fuzz;
        xn = x + (yhi[5]-y)*u/v;
        if (xn>xlo[5]-fuzz && xn<xhi[5]+fuzz2) {
          orad = hypot(xn-x, yhi[5]-y);
          return (orad);
        }
      }
      if (v<0.0) {
        ylo[5] = ylo[5]-fuzz;
        xn = x + (ylo[5]-y)*u/v;
        if (xn>xlo[5]-fuzz && xn<xhi[5]+fuzz2) {
          orad = hypot(xn-x, ylo[5]-y);
          return (orad);
        }
      }
    }
    if (xlo[0]>-990.0) {
      xlo[0] = xlo[0] - fuzz2;
      yn = y + (xlo[0]-x)*v/u;
      if (yn>ylo[0]-fuzz && yn<yhi[0]+fuzz) {
        orad = hypot(xlo[0]-x,yn-y);
        return (orad);
      }
      if (v>0.0) {
        yhi[0] = yhi[0]+fuzz;
        xn = x + (yhi[0]-y)*u/v;
        if (xn>xlo[0]-fuzz && xn<xhi[0]+fuzz2) {
          orad = hypot(xn-x, yhi[0]-y);
          return (orad);
        }
      }
      if (v<0.0) {
        ylo[0] = ylo[0]-fuzz;
        xn = x + (ylo[0]-y)*u/v;
        if (xn>xlo[0]-fuzz && xn<xhi[0]+fuzz2) {
          orad = hypot(xn-x, ylo[0]-y);
          return (orad);
        }
      }
    }
    return (orad);
  }
  if (u>0.0) {
    if (xlo[4]>-990.0) {
      xhi[4] = xhi[4] + fuzz2;
      yn = y + (xhi[4]-x)*v/u;
      if (yn>ylo[4]-fuzz && yn<=yhi[4]+fuzz) {
        orad = hypot(xhi[4]-x,yn-y);
        return (orad);
      }
    }
    if (v>0.0 && xlo[3]>-990.0) {
      yhi[3] = yhi[3] + fuzz;
      xn = x + (yhi[3]-y)*u/v;
      if (xn>xlo[3]-fuzz2 && xn<xhi[3]+fuzz) {
        orad = hypot(xn-x, yhi[3]-y);
        return (orad);
      }
      xhi[3] = xhi[3] + fuzz2;
      yn = y + (xhi[3]-x)*v/u;
      if (yn>ylo[3]-fuzz && yn<yhi[3]+fuzz) {
        orad = hypot(xhi[3]-x,yn-y);
        return (orad);
      }
    }
    if (v<0.0 && xlo[6]>-990.0) {
      ylo[6] = ylo[6] - fuzz;
      xn = x + (ylo[6]-y)*u/v;
      if (xn>xlo[6]-fuzz2 && xn<xhi[6]+fuzz) {
        orad = hypot(xn-x, ylo[6]-y);
        return (orad);
      }
      xhi[6] = xhi[6] + fuzz2;
      yn = y + (xhi[6]-x)*v/u;
      if (yn>ylo[6]-fuzz && yn<yhi[6]+fuzz) {
        orad = hypot(xhi[6]-x,yn-y);
        return (orad);
      }
    }
  }
  return (orad);

}

/* Plot a time series of one station (for now).  Just plot
   the first station encountered.  */

void gatser (struct gacmn *pcm) {
struct gastn *stn, *stn2;
struct garpt *rpt, *rpt2, *frpt;
gadouble rmin,rmax,x,y;
gadouble tsav,dir;
gaint ipen,im,i,hemflg,bcol;

  stn = pcm->result[0].stn;
  rpt = stn->rpt;
  frpt = rpt;            /* Plot stnid = 1st report */
  if (rpt == NULL) {
    gaprnt (0,"No stations to plot\n");
    return;
  }

  /* First pass just get max and min values for this station */

  rmin = 9.99e33;
  rmax = -9.99e33;
  while (rpt!=NULL) {
    if (!cmpch(frpt->stid,rpt->stid,8)) {
      if (rpt->umask != 0) {
        if (rpt->val < rmin) rmin = rpt->val;
        if (rpt->val > rmax) rmax = rpt->val;
      }
    }
    rpt = rpt->rpt;
  }
  if (rmin>rmax) {
    gaprnt (0,"All stations undefined for this variable\n");
    return;
  }

  /* Do scaling */

  i = pcm->frame;
  if (pcm->tser) pcm->frame = 0;
  gas1d (pcm, rmin, rmax, 3, pcm->rotate, NULL, stn);
  pcm->frame = i;

  /* Set up graphics */

  if (pcm->ccolor<0) pcm->ccolor=1;
  gxcolr (pcm->ccolor);
  if (pcm->cstyle>0) gxstyl(pcm->cstyle);
  else gxstyl(1);
  gxwide (pcm->cthick);

  /* Next pass plot lines. */

  if (pcm->tser == 1) {
    rpt = stn->rpt;
    while (rpt!=NULL) {
      gxconv (rpt->tim,0,&x,&y,3);
      if (rpt->umask != 0) {
        i = (gaint)(rpt->val+0.5);
        wxsym (i, x, pcm->ysiz1, pcm->digsiz*1.5, -1, pcm->wxcols);
      } 
      /* Don't show "M" when there's no weather */
/*       else { */
/*         gxchpl ("M",1,x,pcm->ysiz1,pcm->digsiz,pcm->digsiz,0.0); */
/*       } */
      rpt = rpt->rpt;
    }
  } 
  else if (pcm->tser==2) {
    stn2 = pcm->result[1].stn;
    rpt = stn->rpt;
    rpt2 = stn2->rpt;
    while (rpt!=NULL) {
      if (rpt->umask != 0 && rpt2->umask != 0) {
        gxconv (rpt->tim,rpt->val,&x,&y,3);
        if (dequal(rpt->val,0.0,1e-12)==0 && dequal(rpt2->val,0.0,1e-12)==0) 
	  dir = 0.0;
        else 
	  dir = atan2(rpt2->val,rpt->val);
        hemflg = 0;
        if (pcm->hemflg == 1) hemflg = 1;
        else if (pcm->hemflg == 0) hemflg = 0;
        else if (rpt->lat<0.0) hemflg = 1;
        gabarb (x, pcm->ysiz1, pcm->digsiz*3.5, pcm->digsiz*2.0,
		pcm->digsiz*0.25, dir, hypot(rpt->val,rpt2->val), hemflg, pcm->barbolin);
      }
      rpt = rpt->rpt;
      rpt2 = rpt2->rpt;
    }
  } 
  else {
    gxclip (pcm->xsiz1-0.01, pcm->xsiz2+0.01, pcm->ysiz1, pcm->ysiz2);
    if (pcm->cstyle!=0) {
      rpt = stn->rpt;
      tsav = rpt->tim;
      ipen = 3;
      while (rpt!=NULL) {
	if (!cmpch(frpt->stid,rpt->stid,8)) {
	  if (rpt->umask != 0) {
	    if (rpt->tim - tsav > 1.0 && !pcm->miconn) ipen = 3;
	    if (pcm->rotate) gxconv (rpt->val,rpt->tim,&x,&y,3);
	    else gxconv (rpt->tim,rpt->val,&x,&y,3);
	    gxplot (x,y,ipen);
	    ipen = 2;
	  } else if (!pcm->miconn) ipen=3;
	  tsav = rpt->tim;
	}
	rpt = rpt->rpt;
      }
    }
    
    rpt = stn->rpt;
    im = pcm->cmark;
    if (im>0) {
      while (rpt!=NULL) {
	if (!cmpch(frpt->stid,rpt->stid,8)) {
	  if (rpt->umask != 0) {
	    if (pcm->rotate) gxconv (rpt->val,rpt->tim,&x,&y,3);
	    else gxconv (rpt->tim,rpt->val,&x,&y,3);
	    if (im==1 || im==2 || im==4) {
	      bcol = gxdbkq();
	      /* If bcol is neither black nor white, leave it alone. Otherwise, set to 0 for 'background' */
	      if (bcol<2) bcol=0; 
	      gxcolr (bcol);
	      if (im==1) gxmark (4,x,y,pcm->digsiz+0.01);
	      else gxmark (im+1,x,y,pcm->digsiz+0.01);
	      gxcolr(pcm->ccolor);
	    }
	    gxmark (im,x,y,pcm->digsiz+0.01);
	  }
	}
	rpt = rpt->rpt;
      }
    }

    /* Axis labels are not drawn for gxout tserbarb or tserwx */
    gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
    if (pcm->rotate) 
      gaaxpl(pcm,5,3);   /* hard-coded 4's changed to 5's */
    else 
      gaaxpl(pcm,3,5);   /* hard-coded 4's changed to 5's */
  }
  gxwide (4);
  gxcolr (pcm->anncol);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gagsav (13,pcm,NULL);
}

/* Plot a vertical profile given one or more stations and
   a dimension environment where only z varies */

void gapprf (struct gacmn *pcm) {
struct gastn *stn;
struct garpt *anch, **prev, *next, *rpt, *srpt;
gadouble x,y,rmin,rmax;
gaint flag,i,ipen;
char stid[10];

  /* Reorder the linked list of reports so they are sorted */

  stn = pcm->result[0].stn;
  rpt = stn->rpt;
  anch = NULL;
  while (rpt!=NULL) {
    prev = &anch;
    srpt = anch;
    flag = 0;
    while (srpt!=NULL) {
      if (!cmpch(srpt->stid,rpt->stid,8)) {
        flag = 1;
        if (srpt->lev < rpt->lev) break;
      } else if (flag) break;
      prev = &(srpt->rpt);
      srpt = srpt->rpt;
    }
    next = rpt->rpt;
    rpt->rpt = srpt;
    *prev = rpt;
    rpt = next;
  }
  stn->rpt = anch;

  if (stn->rpt==NULL) {
    gaprnt (0,"No station values to plot\n");
    return;
  }

  /* Get max and min */

  rmin = 9.99e33;
  rmax = -9.99e33;
  rpt = stn->rpt;
  while (rpt!=NULL) {
    if (rpt->umask != 0) {
      if (rmin>rpt->val) rmin = rpt->val;
      if (rmax<rpt->val) rmax = rpt->val;
    }
    rpt=rpt->rpt;
  }
  if (rmin>rmax) {
    gaprnt (0,"All stations values are undefined\n");
    return;
  }

  /* Scaling */

  gas1d (pcm, rmin, rmax, 2, !(pcm->rotate), NULL, stn);

  /* Do plot */

  rpt = stn->rpt;
  for (i=0; i<8; i++) stid[i]=' ';
  gxstyl(pcm->cstyle);
  if (pcm->ccolor<1) gxcolr(1);
  else gxcolr(pcm->ccolor);
  gxwide (pcm->cthick);
  ipen = 3;
  while (rpt!=NULL) {
    if (rpt->umask == 0) {
      if (!pcm->miconn) ipen = 3;
    } else {
      if (pcm->rotate) gxconv (rpt->lev,rpt->val,&x,&y,2);
      else gxconv (rpt->val,rpt->lev,&x,&y,2);
      if (cmpch(stid,rpt->stid,8)) ipen = 3;
      gxplot (x,y,ipen);
      ipen=2;
    }
    for (i=0; i<8; i++) stid[i]=rpt->stid[i];
    rpt=rpt->rpt;
  }
  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  if (pcm->rotate) 
    gaaxpl(pcm,2,5);   /* hard-coded 4's changed to 5's */
  else 
    gaaxpl(pcm,5,2);
  gxcolr(pcm->anncol);
  gxwide (4);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gagsav (13,pcm,NULL);
}

/*  Routine to set up scaling for a 1-D plot.  */

void gas1d (struct gacmn *pcm, gadouble cmin, gadouble cmax, gaint dim, gaint rotflg, 
	    struct gagrid *pgr, struct gastn *stn) {
gadouble x1,x2,y1,y2,xt1,xt2,yt1,yt2,d2r;
gadouble cint,cmn,cmx;
gaint axmov;

  d2r = M_PI/180;
  idiv = 1; jdiv = 1;   /* No grid expansion factor */
  gxrset(3);            /* Reset all scaling        */
  gxrsmapt();           /* Reset map type */

  /* Set plot area limits */

  if (pcm->paflg) {
    pcm->xsiz1 = pcm->pxmin;
    pcm->xsiz2 = pcm->pxmax;
    pcm->ysiz1 = pcm->pymin;
    pcm->ysiz2 = pcm->pymax;
  } else {
    if (rotflg) {
      pcm->xsiz1 = pcm->xsiz/2.0 - pcm->ysiz/3.0;
      pcm->xsiz2 = pcm->xsiz/2.0 + pcm->ysiz/3.0;
      if (pcm->xsiz1<1.0) pcm->xsiz1 = 1.0;
      if (pcm->xsiz2>pcm->xsiz-0.5) pcm->xsiz2 = pcm->xsiz-0.5;
      pcm->ysiz1 = 0.75;
      pcm->ysiz2 = pcm->ysiz-0.75;
    } else {
      pcm->xsiz1 = 2.0;
      pcm->xsiz2 = pcm->xsiz - 0.5;
      pcm->ysiz1 = 0.75;
      pcm->ysiz2 = pcm->ysiz - 0.75;
    }
  }

  /* Determine axis limits and label interval.  Use user
     supplied AXLIM if available.  Also try to use most recent
     limits if frame hasn't been cleared -- if rotated, force
     use of most recent limits, since we don't plot new axis.  */

  cint = 0.0;
  gacsel (cmin,cmax,&cint,&cmn,&cmx);
  if (cint==0.0) {
    cmn = cmin-5.0;
    cmx = cmin+5.0;
    cint = 2.0;
  } else {
    cmn = cmn - 2.0*cint;
    cmx = cmx + 2.0*cint;
  }
  axmov = 1;

  if (pcm->aflag == -1 || (rotflg && pcm->pass>0)) {
    /* put check for 0 value if doing a second + plot on the same page
       this means that the scaling from the first plot is not appropriate  */
    if(!(pcm->rmin == 0 && pcm->rmax == 0)) {
      cmn = pcm->rmin;
      cmx = pcm->rmax;
      cint = pcm->rint;
    }
    axmov = 0;
    
  } else if (pcm->aflag == 1) {
    if ( (cmin > (pcm->rmin-pcm->rint*2.0)) &&
         (cmax < (pcm->rmax+pcm->rint*2.0)) ) {
      cmn = pcm->rmin;
      cmx = pcm->rmax;
      cint = pcm->rint;
      axmov = 0;
    }
  }

  if (!pcm->ylpflg && axmov && pcm->yllow>0.0) {
    pcm->ylpos = pcm->ylpos - pcm->yllow - 0.1;
  }
  pcm->yllow = 0.0;

  /* Set absolute coordinate scaling for this grid.
     Note that this code assumes knowledge of the time
     coordinate scaling setup -- namely, that the same
     vals are used for t2gr as gr2t.                 */

  if (pgr!=NULL) {
    y1 = cmn; y2 = cmx;
    if (dim==3) {
      x1 = t2gr(pgr->ivals,&(pcm->tmin));
      x2 = t2gr(pgr->ivals,&(pcm->tmax));
      if (pcm->log1d > 1) {
        if (cmn<=0.0 || cmx<=0.0) {
          gaprnt (1,"Cannot use log scaling when coordinates <= 0\n");
          gaprnt (1,"Linear scaling used\n");
        } else {
          if (rotflg) { gxproj(galogx); gxback(gaalogx); }
          else { gxproj(galogy); gxback(gaalogy); }
          y1 = log10(cmn); y2 = log10(cmx);
        }
      }
    } else {
      x1 = pcm->dmin[dim];
      x2 = pcm->dmax[dim];
      if (pcm->log1d) {  /* Only one kind of 1D scaling at a time */
        if (pcm->log1d == 1) {
          if (x1<=0.0 || x2<=0.0) {
            gaprnt (1,"Cannot use log scaling when coordinates <= 0\n");
            gaprnt (1,"Linear scaling used\n");
          } else {
            if (rotflg) { gxproj(galogy); gxback(gaalogy); }
            else { gxproj(galogx); gxback(gaalogx); }
            x1 = log10(x1); x2 = log10(x2);
          }
        }
        if (pcm->log1d == 2) {
          if (cmn<=0.0 || cmx<=0.0) {
            gaprnt (1,"Cannot use log scaling when coordinates <= 0\n");
            gaprnt (1,"Linear scaling used\n");
          } else {
            if (rotflg) { gxproj(galogx); gxback(gaalogx); }
            else { gxproj(galogy); gxback(gaalogy); }
            y1 = log10(cmn); y2 = log10(cmx);
          }
        }
        if (pcm->log1d == 3) {
          if (cmn<=0.0 || cmx<=0.0 || x1<=0.0 || x2<=0.0) {
            gaprnt (1,"Cannot use log scaling when coordinates <= 0\n");
            gaprnt (1,"Linear scaling used\n");
          } else {
            gxproj(galog2); gxback(gaalog2); 
            y1 = log10(cmn); y2 = log10(cmx);
            x1 = log10(x1); x2 = log10(x2);
          }
        }
      } 
      else if (dim==2 && pcm->zlog) {
        if (x1<=0.0 || x2<=0.0) {
          gaprnt (1,"Cannot use log scaling when coordinates <= 0\n");
          gaprnt (1,"Linear scaling used\n");
        } else {
          if (rotflg) {
            gxproj(galny);
            gxback(gaalny);
          } else {
            gxproj(galnx);
            gxback(gaalnx);
          }
          x1 = log(x1);
          x2 = log(x2);
        }
      }
      else if (dim==1 && pcm->coslat) {
        if (x1 < -90.0 || x2 > 90.0) {
          gaprnt (1,"Cannot use cos lat scaling when coordinates exceed -90 to 90\n");
          gaprnt (1,"Linear scaling used\n");
        } else {
          if (rotflg) {
            gxproj(gacly);
            gxback(gaacly);
          } else {
            gxproj(gaclx);
            gxback(gaaclx);
          }
          x1 = sin(x1*d2r);
          x2 = sin(x2*d2r);
        }
      }
    }
    if (rotflg) {
      pcm->xdim = 5;  /* hard-coded 4 changed to 5 */
      pcm->ydim = dim;
      pcm->ygr2ab = pgr->igrab;
      pcm->yab2gr = pgr->iabgr;
      pcm->ygrval = pgr->ivals;
      pcm->yabval = pgr->iavals;
      xt1 = y1; xt2 = y2; yt1 = x1; yt2 = x2;
      if (pcm->xflip) {xt1 = y2; xt2 = y1;}
      if (pcm->yflip) {yt1 = x2; yt2 = x1;}
    } else {
      pcm->xdim = dim;
      pcm->ydim = 5;  /* hard-coded 4 changed to 5*/
      pcm->xgr2ab = pgr->igrab;
      pcm->xab2gr = pgr->iabgr;
      pcm->xgrval = pgr->ivals;
      pcm->xabval = pgr->iavals;
      xt1 = x1; xt2 = x2; yt1 = y1; yt2 = y2;
      if (pcm->xflip) {xt1 = x2; xt2 = x1;}
      if (pcm->yflip) {yt1 = y2; yt2 = y1;}
    }
    gxscal (pcm->xsiz1,pcm->xsiz2,pcm->ysiz1,pcm->ysiz2,xt1,xt2,yt1,yt2);
    if (rotflg) {
      if (dim==3) {
	jconv = NULL;
      }
      else {
        jconv = pgr->igrab;
        jvars = pgr->ivals;
      }
      joffset = pgr->dimmin[dim]-1;
      iconv = NULL;
      ioffset = 0;
    } else {
      if (dim==3) {
	iconv = NULL;
      }
      else {
        iconv = pgr->igrab;
        ivars = pgr->ivals;
      }
      ioffset = pgr->dimmin[dim]-1;
      jconv = NULL;
      joffset = 0;
    }
    gxgrid (gaconv);
  }   
  else {
    if (dim==3) {
      x1 = t2gr(stn->tvals, &(pcm->tmin));
      x2 = t2gr(stn->tvals, &(pcm->tmax));
    } else {
      x1 = pcm->dmin[dim];
      x2 = pcm->dmax[dim];
      if (dim==2 && pcm->zlog) {
        if (x1<=0.0 || x2<=0.0) {
          gaprnt (1,"Cannot use log scaling when coordinates <= 0\n");
          gaprnt (1,"Linear scaling used\n");
        } else {
          if (rotflg) {
            gxproj(galny);
            gxback(gaalny);
          } else {
            gxproj(galnx);
            gxback(gaalnx);
          }
          x1 = log(x1);
          x2 = log(x2);
        }
      }
      if (dim==1 && pcm->coslat) {
        if (x1 < -90.0 || x2 > 90.0) {
          gaprnt (1,"Cannot use cos lat scaling when coordinates exceed -90 to 90\n");
          gaprnt (1,"Linear scaling used\n");
        } else {
          if (rotflg) {
            gxproj(gacly);
            gxback(gaacly);
          } else {
            gxproj(gaclx);
            gxback(gaaclx);
          }
          x1 = sin(x1*d2r);
          x2 = sin(x2*d2r);
        }
      }
    }
    if (rotflg) {
      pcm->xdim = 5;
      pcm->ydim = dim;
      if (dim==3) pcm->ygrval = stn->tvals;
      xt1 = cmn; xt2 = cmx; yt1 = x1; yt2 = x2;
      if (pcm->xflip) {xt1 = cmx; xt2 = cmn;}
      if (pcm->yflip) {yt1 = x2; yt2 = x1;}
      gxscal (pcm->xsiz1,pcm->xsiz2,pcm->ysiz1,pcm->ysiz2,xt1,xt2,yt1,yt2);
    } else {
      pcm->xdim = dim;
      pcm->ydim = 5;
      if (dim==3) pcm->xgrval = stn->tvals;
      xt1 = x1; xt2 = x2; yt1 = cmn; yt2 = cmx;
      if (pcm->xflip) {xt1 = x1; xt2 = x2;}
      if (pcm->yflip) {yt1 = cmn; yt2 = cmx;}
      gxscal (pcm->xsiz1,pcm->xsiz2,pcm->ysiz1,pcm->ysiz2,x1,x2,cmn,cmx);
    }
  }
  pcm->rmin = cmn;
  pcm->rmax = cmx;
  pcm->rint = cint;
  if (pcm->aflag == 0) pcm->aflag = 1;
  gafram (pcm);
}

/* Set up grid level scaling for a 2-D plot */

void gas2d (struct gacmn *pcm, struct gagrid *pgr, gaint imap) {
gadouble x1,x2,y1,y2,xt1,xt2,yt1,yt2,d2r;
gaint idim,jdim;

  d2r = M_PI/180;
  gxrset (3);       /* Reset all scaling */
  gxrsmapt();       /* Reset map type */

  /* Set up linear level scaling (level 1) and map level scaling
     (level 2).  If no map drawn, just do linear level scaling.  */

  idim = pgr->idim;
  jdim = pgr->jdim;
  pcm->xdim = idim;
  pcm->ydim = jdim;
  pcm->xgrval = pgr->ivals;  pcm->ygrval = pgr->jvals;
  pcm->xabval = pgr->iavals; pcm->yabval = pgr->javals;
  pcm->xgr2ab = pgr->igrab;  pcm->ygr2ab = pgr->jgrab;
  pcm->xab2gr = pgr->iabgr;  pcm->yab2gr = pgr->jabgr;
  if (idim==0 && jdim==1) {
    gamscl (pcm);               /* Do map level scaling */
    if (imap) gawmap(pcm, 1);   /* Draw map if requested */
  }
  else {
    if (pcm->paflg) {
      pcm->xsiz1 = pcm->pxmin;
      pcm->xsiz2 = pcm->pxmax;
      pcm->ysiz1 = pcm->pymin;
      pcm->ysiz2 = pcm->pymax;
    } else {
      pcm->xsiz1 = 1.5; 
      pcm->xsiz2 = pcm->xsiz-0.5;
      pcm->ysiz1 = 1.0;
      pcm->ysiz2 = pcm->ysiz-0.5;
    }
    if (idim==3) {
      x1 = t2gr(pgr->ivals,&(pcm->tmin));
      x2 = t2gr(pgr->ivals,&(pcm->tmax));
    } else if (idim==5) {
      x1 = 1; 
      x2 = pgr->isiz;  /* COLL */
    } else {
      x1 = pcm->dmin[idim];
      x2 = pcm->dmax[idim];
      if (idim==2 && pcm->zlog) {
        if (x1<=0.0 || x2<=0.0) {
          gaprnt (1,"Cannot use log scaling when coordinates <= 0\n");
          gaprnt (1,"Linear scaling used\n");
        } else {
          gxproj(galnx);
          gxback(gaalnx);
          x1 = log(x1);
          x2 = log(x2);
        }
      }
      else if (idim==1 && pcm->coslat) {  /* can't have zlog and coslat both */
        if (x1 < -90.0 || x2 > 90.0) {
          gaprnt (1,"Cannot use cos lat scaling when coordinates exceed -90 to 90\n");
          gaprnt (1,"Linear scaling used\n");
        } else {
          gxproj(gaclx);
          gxback(gaaclx);
          x1 = sin(x1*d2r);
          x2 = sin(x2*d2r);
        }
      }
    }
    if (jdim==3) {
      y1 = t2gr(pgr->jvals,&(pcm->tmin));
      y2 = t2gr(pgr->jvals,&(pcm->tmax));
    } else {
      y1 = pcm->dmin[jdim];
      y2 = pcm->dmax[jdim];
      if (jdim==2 && pcm->zlog) {
        if (y1<=0.0 || y2<=0.0) {
          gaprnt (1,"Cannot use log scaling when coordinates <= 0\n");
          gaprnt (1,"Linear scaling used\n");
        } else {
          gxproj(galny);
          gxback(gaalny);
          y1 = log(y1);
          y2 = log(y2);
        }
      }
      else if (jdim==1 && pcm->coslat) {  /* can't have zlog and coslat both */
        if (y1 < -90.0 || y2 > 90.0) {
          gaprnt (1,"Cannot use cos lat scaling when coordinates exceed -90 to 90\n");
          gaprnt (1,"Linear scaling used\n");
        } else {
          gxproj(gacly);
          gxback(gaacly);
          y1 = sin(y1*d2r);
          y2 = sin(y2*d2r);
        }
      }
    }
    xt1 = x1; xt2 = x2; yt1 = y1; yt2 = y2;
    if (pcm->xflip) { xt1 = x2; xt2 = x1; }
    if (pcm->yflip) { yt1 = y2; yt2 = y1; }
    gxscal (pcm->xsiz1,pcm->xsiz2,pcm->ysiz1,pcm->ysiz2,xt1,xt2,yt1,yt2);
  }

  /* Now set up level 2 grid scaling done through gaconv */

  if (idim==5) ioffset = 0;  /* COLL */
  else ioffset = pgr->dimmin[idim]-1;
  if (idim==3) iconv = NULL;
  else {
    iconv = pgr->igrab;
    ivars = pgr->ivals;
  }
  if (jdim==5) joffset = 1;  /* COLL */
  else joffset = pgr->dimmin[jdim]-1;
  if (jdim==3) jconv = NULL;
  else {
    jconv = pgr->jgrab;
    jvars = pgr->jvals;
  }
  gxgrid (gaconv);

  if (idim==5) {   /* COLL -- predefine axis */
    pcm->rmin = 1;
    pcm->rmax = (gadouble)(pgr->isiz);
    pcm->axmin = 1.0;
    pcm->axmax = pcm->rmax;
    pcm->axflg = 1;
    pcm->axint = 1.0;
  }
}

/* Line plot fill.  Fill above and below a line plot */

void galfil (struct gacmn *pcm) {
struct gagrid *pgr1, *pgr2;
gadouble *gr1,*gr2,rmin,rmax,*v1,*v2,*u,*xy,uu,vv;
gaint rotflg,sflag,cnt,abflg,i,colr=0;
char *gr1u,*gr2u;
size_t sz;

  /* Check args */

  if (pcm->numgrd<2) {
    gaprnt (0,"Error plotting linefill:  Only one grid provided\n");
    return;
  }
  pgr1 = pcm->result[0].pgr;
  pgr2 = pcm->result[1].pgr;
  if (pgr1->idim!=pgr2->idim || pgr1->jdim!=-1 ||
      pgr2->jdim!=-1 || gagchk(pgr1,pgr2,pgr1->idim)) {
    gaprnt (0,"Error plotting linefill:  Invalid grids --");
    gaprnt (0," different scaling\n");
    return;
  }

  gamnmx(pgr1);
  gamnmx(pgr2);
  if (pgr1->umin==0 || pgr2->umin==0) {
    gaprnt (1,"Cannot plot data - all undefined values \n");
    return;
  }
  rmin = pgr1->rmin;
  if (rmin>pgr2->rmin) rmin = pgr2->rmin;
  rmax = pgr1->rmax;
  if (rmax<pgr2->rmax) rmax = pgr2->rmax;

  /* Do scaling */

  rotflg = 0;
  if (pgr1->idim==2) rotflg = 1;
  if (pcm->rotate) rotflg = !rotflg;
  gas1d (pcm, rmin, rmax, pgr1->idim, rotflg, pgr1, NULL);

  gxclip (pcm->xsiz1-0.01, pcm->xsiz2+0.01, pcm->ysiz1, pcm->ysiz2);

 /* Allocate some buffer areas */

  sz = sizeof(gadouble)*pgr1->isiz;
  u  = (gadouble *)galloc(sz,"gridu");
  v1 = (gadouble *)galloc(sz,"gridv1");
  v2 = (gadouble *)galloc(sz,"gridv2");
  sz = sizeof(gadouble)*(pgr1->isiz*4+8);
  xy = (gadouble *)galloc(sz,"gridxy");
  if (u==NULL || v1==NULL || v2==NULL || xy==NULL) {
    gaprnt(0,"Memory allocation error in linefill\n");
    return;
  }

  /* Find a start point.  It is the first point where
     two points in a row are not missing (in both lines)
     and are not equal.  */

  gr1 = pgr1->grid;
  gr2 = pgr2->grid;
  gr1u = pgr1->umask;
  gr2u = pgr2->umask;

  i = 1;
  while (1) {
    if (i>=pgr1->isiz) break;
    if (*gr1u!=0 && *gr2u!=0 &&
        *(gr1u+1)!=0 && *(gr2u+1)!=0 &&
        (*gr1!=*gr2 || *(gr1+1)!=*(gr2+1))) break;
    i++; gr1++; gr2++; gr1u++; gr2u++;
  }
  if (i>=pgr1->isiz) {
    gree(u,"f280");
    gree(v1,"f281");
    gree(v2,"f282");
    gree(xy,"f283");
    gaprnt (1,"Cannot plot data - too many undefined values \n");
    return;
  }

  /* Set up for loop */

  cnt = 1;
  *u = i;
  *v1 = *gr1;
  *v2 = *gr2;
  abflg = 0;
  if (*gr1>*gr2) abflg = 1;
  if (*gr1<*gr2) abflg = 2;
  i++; gr1++; gr2++; gr1u++; gr2u++;
  if (abflg==0) {      /* if 1st point is same for both lines */
    if (*gr1>*gr2) abflg = 1;
    if (*gr1<*gr2) abflg = 2;
  }

  /* Go through rest of data */

  while (i<=pgr1->isiz) {
    sflag = 0;

    /* If we hit missing data, terminate the current poly fill */
   
    if (*gr1u==0 || *gr2u==0) {
      if (abflg==1) colr=pcm->lfc1;
      if (abflg==2) colr=pcm->lfc2;
      if (colr>-1) {
        gxcolr(colr);
        lfout (u, v1, v2, xy, cnt, rotflg);
      }
      sflag = 1;

    /* No missing data.  Polygon continues until the curves cross.  */  

    } else {
      if (abflg==0) {   /* this shouldn't really happen? */
        if (*gr1>*gr2) abflg = 1;
        else if (*gr1<*gr2) abflg = 2;
        else sflag = 1;
      } else if (abflg==1) {
        if (*gr1<*gr2) {  /* Curves crossed */
          lfint (*(v1+cnt-1),*gr1,*(v2+cnt-1),*gr2,&uu,&vv);
          uu = uu + (gadouble)(i-1);
          *(u+cnt) = uu;
          *(v1+cnt) = vv;
          *(v2+cnt) = vv;
          cnt++;
          if (abflg==1) colr=pcm->lfc1;
          if (abflg==2) colr=pcm->lfc2;
          if (colr>-1) {
            gxcolr(colr);
            lfout (u, v1, v2, xy, cnt, rotflg);
          }
          *u = uu; *v1 = vv; *v2 = vv;
          *(u+1) = i; *(v1+1) = *gr1; *(v2+1) = *gr2;
          cnt = 2;
          abflg = 2;
        } else if (*gr1==*gr2) {  /* Curves touching */
          *(v1+cnt) = *gr1;
          *(v2+cnt) = *gr2;
          *(u+cnt) = i;
          cnt++;
          if (abflg==1) colr=pcm->lfc1;
          if (abflg==2) colr=pcm->lfc2;
          if (colr>-1) {
            gxcolr(colr);
            lfout (u, v1, v2, xy, cnt, rotflg);
          }
          sflag = 1;
        } else {    /* curves not crossing; add to poygon */
          *(u+cnt) = i;
          *(v1+cnt) = *gr1;
          *(v2+cnt) = *gr2;
          cnt++;
        }
      } else if (abflg==2) {
        if (*gr1>*gr2) {
          lfint (*(v1+cnt-1),*gr1,*(v2+cnt-1),*gr2,&uu,&vv);
          uu = uu + (gadouble)(i-1);
          *(u+cnt) = uu;
          *(v1+cnt) = vv;
          *(v2+cnt) = vv;
          cnt++;
          if (abflg==1) colr=pcm->lfc1;
          if (abflg==2) colr=pcm->lfc2;
          if (colr>-1) {
            gxcolr(colr);
            lfout (u, v1, v2, xy, cnt, rotflg);
          }
          *u = uu; *v1 = vv; *v2 = vv;
          *(u+1) = i; *(v1+1) = *gr1; *(v2+1) = *gr2;
          cnt = 2;
          abflg = 1;
        } else if (*gr1==*gr2) {
          *(v1+cnt) = *gr1;
          *(v2+cnt) = *gr2;
          *(u+cnt) = i;
          cnt++;
          if (abflg==1) colr=pcm->lfc1;
          if (abflg==2) colr=pcm->lfc2;
          if (colr>-1) {
            gxcolr(colr);
            lfout (u, v1, v2, xy, cnt, rotflg);
          }
          sflag = 1;
        } else {
          *(u+cnt) = i;
          *(v1+cnt) = *gr1;
          *(v2+cnt) = *gr2;
          cnt++;
        }
      }
    }

    /* If necessary, search for new start point */

    if (sflag) {
      while (1) {
        if (i>=pgr1->isiz) break;
        if (*gr1u!=0 && *gr2u!=0 &&
            *(gr1u+1)!=0 && *(gr2u+1)!=0 &&
            (*gr1!=*gr2 || *(gr1+1)!=*(gr2+1))) break;
        i++; gr1++; gr2++; gr1u++; gr2u++;
      }
      if (i<pgr1->isiz) {
        cnt = 1;
        *u = i;
        *v1 = *gr1;
        *v2 = *gr2;
        abflg = 0;
        if (*gr1>*gr2) abflg = 1;
        if (*gr1<*gr2) abflg = 2;
        i++; gr1++; gr2++; gr1u++; gr2u++;
        if (abflg==0) { 
          if (*gr1>*gr2) abflg = 1;
          if (*gr1<*gr2) abflg = 2;
        }
      } else {
        cnt = 0;
        i = pgr1->isiz+1;
      }
    } else {
      i++; gr1++; gr2++; gr1u++; gr2u++;
    }
  }

  /* Finish up and plot last poly */

  if (cnt>1) {
    if (abflg==1) colr=pcm->lfc1;
    if (abflg==2) colr=pcm->lfc2;
    if (colr>-1) {
      gxcolr(colr);
      lfout (u, v1, v2, xy, cnt, rotflg);
    }
  }
  gree(u,"f284");
  gree(v1,"f285");
  gree(v2,"f286");
  gree(xy,"f287");
  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  if (rotflg) 
    gaaxpl(pcm,5,pgr1->idim);      /* hard coded 4's changed to 5's */
  else 
    gaaxpl(pcm,pgr1->idim,5);      /* hard coded 4's changed to 5's */
  gxwide (4);
  gxcolr (pcm->anncol);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gagsav (17,pcm,pgr1);
}

void lfint (gadouble v11, gadouble v12, gadouble v21, gadouble v22, gadouble *u, gadouble *v) {
  *u = (v21-v11)/(v12-v11-v22+v21);
  *v = v11 + *u*(v12-v11);
}

void lfout (gadouble *u, gadouble *v1, gadouble *v2, gadouble *xyb, gaint cnt, gaint rotflg) {
gaint i,j,ii;
gadouble *xy;

  xy = xyb;
  if (cnt<2) {
    gaprnt (0,"Internal Logic check 4 in linefill\n");
    return;
  }
  j = 0;
  for (i=0; i<cnt; i++) {
    if (rotflg) gxconv (*(v1+i),*(u+i),xy,xy+1,3);
    else gxconv (*(u+i),*(v1+i),xy,xy+1,3);
    j++; xy+=2;
  }
  i = cnt-1;
  if (*(v1+i) != *(v2+i) ) {
    if (rotflg) gxconv (*(v2+i),*(u+i),xy,xy+1,3);
    else gxconv (*(u+i),*(v2+i),xy,xy+1,3);
    j++; xy+=2;
  }
  for (i=2; i<cnt; i++) {
    ii = cnt-i;
    if (rotflg) gxconv (*(v2+ii),*(u+ii),xy,xy+1,3);
    else gxconv (*(u+ii),*(v2+ii),xy,xy+1,3);
    j++; xy+=2;
  }
  if (*v1 != *v2) {
    if (rotflg) gxconv (*v2,*u,xy,xy+1,3);
    else gxconv (*u,*v2,xy,xy+1,3);
    j++; xy+=2;
  }
  gxfill (xyb,j);
}

/* Do line graph (barflg=0) or bar graph (barflg=1) or
   1-D wind vector/barb plot (barflg=2) */

void gagrph (struct gacmn *pcm, gaint barflg) {
struct gagrid *pgr,*pgr2=NULL,*pgr3=NULL;
gadouble x1,x2,y1,y2,x,y,xz,yz,rmin,rmax,xx,yy;
gadouble *gr,*gr2=0,*gr3=0;
gadouble gap,umax,vmax,vscal=0,dir;
gaint ip,i,im,bflg,rotflg,flag,pflg,hflg=0,hemflg,bcol;
char *gru,*gr2u=NULL,*gr3u=NULL;

  /* Determine min and max */

  pgr = pcm->result[0].pgr;
  gamnmx(pgr);
  if (pgr->umin==0) {
    gaprnt (1,"Cannot plot data - all undefined values \n");
    gxcolr(1);
    if (pcm->dwrnflg) gxchpl ("Entire Grid Undefined",21,3.0,4.5,0.3,0.25,0.0);
    return;
  }
  rmin = pgr->rmin; 
  rmax = pgr->rmax;

  flag = 0;
  if (pcm->numgrd>1) {
    flag = 1;
    pgr2 = pcm->result[1].pgr;
    if (pgr2->idim!=pgr->idim || pgr2->jdim!=pgr->jdim ||
        gagchk(pgr2,pgr,pgr->idim) || gagchk(pgr2,pgr,pgr->jdim)) {
      flag = 0;
      gaprnt (0,"Error in line/bar graph:  Invalid 2nd field");
      gaprnt (0,"   2nd grid ignored -- has different scaling");
    } else {
      if (barflg==1) {
        gamnmx(pgr2);
        if (pgr2->umin==0) {
          gaprnt (1,"Cannot plot data - 2nd arg all undefined values \n");
          return;
        }
        if (rmin>pgr2->rmin) rmin = pgr2->rmin;
        if (rmax<pgr2->rmax) rmax = pgr2->rmax;
      }
    }
    if (pcm->numgrd>2 && flag==1) {
      flag = 2;
      pgr3 = pcm->result[2].pgr;
      if (pgr3->idim!=pgr->idim || pgr3->jdim!=pgr->jdim ||
          gagchk(pgr3,pgr,pgr->idim) || gagchk(pgr3,pgr,pgr->jdim)) {
        flag = 1;
        gaprnt (0,"Error in line/bar graph:  Invalid 3rd field");
        gaprnt (0,"   3rd grid ignored -- has different scaling");
      }
    }
  }

  /* Do scaling */

  rotflg = 0;
  if (pgr->idim==2) rotflg = 1;
  if (pcm->rotate) rotflg = !rotflg;
  gas1d (pcm, rmin, rmax, pgr->idim, rotflg, pgr, NULL);

  /* Set up graphics */

  if (pcm->ccolor<0) pcm->ccolor=1;
  gxcolr (pcm->ccolor);
  gxstyl(pcm->cstyle);
  gxwide (pcm->cthick);
  gr  = pgr->grid;
  gru = pgr->umask;
  if (flag) {
    gr2  = pgr2->grid;
    gr2u = pgr2->umask;
  }
  gxclip (pcm->xsiz1-0.01, pcm->xsiz2+0.01, pcm->ysiz1, pcm->ysiz2);

  /* Do bar graph */

  if (barflg) {
    bflg = pcm->barflg;
    gap = ((gadouble)pcm->bargap)*0.5/100.0;
    gap = 0.5-gap;
    if (rotflg) {
      if (bflg==1) gxconv (pcm->barbase,1.0,&xz,&y,3);
      else if (bflg==0) xz = pcm->xsiz1;
      else xz = pcm->xsiz2;
      if (bflg==1 && (xz<pcm->xsiz1||xz>pcm->xsiz2)) {
        gaprnt(0,"Error drawing bargraph: base value out of range\n");
        gaprnt(0,"    Drawing graph from bottom\n");
        bflg = 0;
        xz = pcm->xsiz1;
      }
      for (i=1;i<=pgr->isiz;i++) {
        pflg = 1;
        if (flag) {
          if (*gru == 0 || *gr2u == 0) {
            pflg=0;
          } else {
            gxconv (*gr,(gadouble)(i)-gap,&x2,&y1,3);
            gxconv (*gr,(gadouble)(i)+gap,&x2,&y2,3);
            gxconv (*gr,(gadouble)(i),&x2,&yz,3);
            gxconv (*gr2,(gadouble)(i),&x1,&yz,3);
          }
        } else {
          if (*gru == 0) {
            pflg = 0;
          } else {
            gxconv (*gr,(gadouble)(i)-gap,&x2,&y1,3);
            gxconv (*gr,(gadouble)(i)+gap,&x2,&y2,3);
            gxconv (*gr,(gadouble)(i),&x2,&yz,3);
            x1 = xz;
          }
        }
        if (pflg) {
          if (barflg==2) {
            gxplot(x1,yz,3);
            gxplot(x2,yz,2);
            gxplot(x1,y1,3);
            gxplot(x1,y2,2);
            gxplot(x2,y1,3);
            gxplot(x2,y2,2);
          } else if (pcm->barolin) {
            gxplot(x1,y1,3);
            gxplot(x1,y2,2);
            gxplot(x2,y2,2);
            gxplot(x2,y1,2);
            gxplot(x1,y1,2);
          } else gxrecf (xz, x, y1, y2);
        }
        gr++; gru++;
        if (flag) {
	  gr2++; gr2u++;
	}
      }
    } else {
      if (bflg==1) gxconv (1.0,pcm->barbase,&x,&yz,3);
      else if (bflg==0) yz = pcm->ysiz1;
      else yz = pcm->ysiz2;
      if (bflg==1 && (yz<pcm->ysiz1||yz>pcm->ysiz2)) {
        gaprnt(0,"Error drawing bargraph: base value out of range\n");
        gaprnt(0,"    Drawing graph from bottom\n");
        bflg = 0;
        yz = pcm->ysiz1;
      }
      for (i=1;i<=pgr->isiz;i++) {
        pflg = 1;
        if (flag) {
          if (*gru == 0 || *gr2u == 0) {
            pflg=0;
          } else {
            gxconv ((gadouble)(i)-gap,*gr,&x1,&y1,3);
            gxconv ((gadouble)(i)+gap,*gr,&x2,&y1,3);
            gxconv ((gadouble)(i),*gr,&xz,&y1,3);
            gxconv ((gadouble)(i),*gr2,&xz,&y2,3);
          }
        } else {
          if (*gru == 0) {
            pflg = 0;
          } else {
            gxconv ((gadouble)(i)-gap,*gr,&x1,&y1,3);
            gxconv ((gadouble)(i)+gap,*gr,&x2,&y1,3);
            gxconv ((gadouble)(i),*gr,&xz,&y1,3);
            y2 = yz;
          }
        }
        if (pflg) {
          if (barflg==2) {
            gxplot(xz,y1,3);
            gxplot(xz,y2,2);
            gxplot(x1,y1,3);
            gxplot(x2,y1,2);
            gxplot(x1,y2,3);
            gxplot(x2,y2,2);
          } else if (pcm->barolin) {
            gxplot(x1,y1,3);
            gxplot(x1,y2,2);
            gxplot(x2,y2,2);
            gxplot(x2,y1,2);
            gxplot(x1,y1,2);
          } else gxrecf (x1, x2, y1, y2);
        }
        gr++; gru++;
        if (flag) {
	  gr2++; gr2u++;
	}
      }
    }

  /* Do line graph, or wind vectors/barbs when 3 grids */

  } else {

    /*  Draw the line first */

    if (pcm->cstyle!=0 && flag<2) {
      ip=3;
      for (i=1;i<=pgr->isiz;i++) {
        if (*gru == 0) {
          if (!pcm->miconn) ip = 3;
        } else {
          if (rotflg) gxconv (*gr,(gadouble)i,&x,&y,3);
          else gxconv ((gadouble)i,*gr,&x,&y,3);
          gxplot (x,y,ip);
          ip=2;
        }
        gr++; gru++;
      }
    }

    /* Now draw the markers, or wind vectors/barbs */

    im = pcm->cmark;
    if (im>0 || flag==2) {
      gxstyl (1);
      gr=pgr->grid;
      gru=pgr->umask;
      if (flag==2) {   /* if vectors/barbs, get ready */
        gr2 = pgr2->grid;
        gr3 = pgr3->grid;
        gr2u = pgr2->umask;
        gr3u = pgr3->umask;

        /* hflg=0 idim is lat
           hflg=2 plot nhem
           hflg=3 plot shem */

        if (pcm->hemflg==0) hflg = 2;
        else if (pcm->hemflg==1) hflg = 3;
        else {
          if (pgr2->idim==1) hflg = 0;
          else {
            if (pcm->dmin[1] < 0.0) hflg = 3; 
            else hflg = 2;
          }
        }
        if (!pcm->arrflg) {
          gamnmx (pgr2);
          gamnmx (pgr3);
          umax = pgr2->rmax;
          if (umax<fabs(pgr2->rmin)) umax = fabs(pgr2->rmin);
          vmax = pgr3->rmax;
          if (vmax<fabs(pgr3->rmin)) vmax = fabs(pgr3->rmin);
          vscal = hypot(umax,vmax);
          if (vscal>0.0) {
            x = floor(log10(vscal));
            y = floor(vscal/pow(10.0,x));
            vscal = y * pow(10.0,x);
            pcm->arrsiz = 0.5;
          } else vscal=1.0;
          pcm->arrmag = vscal;
        } else {
          vscal = pcm->arrmag;
        }
        pcm->arrflg = 1;
      }
      for (i=1;i<=pgr->isiz;i++) {
        if (*gru != 0) {
          if (rotflg) gxconv (*gr,(gadouble)i,&x,&y,3);
          else gxconv ((gadouble)i,*gr,&x,&y,3);
          if (flag==2) {   /* handle vectors/barbs */
             /*  xcv */
            if (*gr2u != 0 && *gr3u != 0) {
              if (rotflg) gxgrmp (*gr,(gadouble)i,&xx,&yy);
              else gxgrmp ((gadouble)i,*gr,&yy,&xx);
              hemflg = 0;
              if (hflg==0 && yy<0.0) hemflg = 1;
              if (hflg==3) hemflg = 1;
              if (*gr2==0.0 && *gr3==0.0) dir = 0.0;
              else dir = atan2(*gr3,*gr2);
              if (pcm->gout1a==2) {
                gabarb (x, y, pcm->digsiz*3.5, pcm->digsiz*2.0,
			pcm->digsiz*0.25, dir, hypot(*gr2,*gr3), hemflg, pcm->barbolin);
              } else {
                if (vscal>0.0) {
                  gaarrw (x, y, dir, pcm->arrsiz*hypot(*gr2,*gr3)/vscal, pcm->ahdsiz);
                } else {
                  gaarrw (x, y, dir, pcm->arrsiz, pcm->ahdsiz);
                }
              }
            }
          } else {
	    /* draw cmarks */
            if (im==1 || im==2 || im==4 || im==7 || im==8) {
	      bcol = gxdbkq();
	      /* If bcol is neither black nor white, leave it alone. Otherwise, set to 0 for 'background' */
	      if (bcol<2) bcol=0; 
              gxcolr (bcol);
              if (im==1) gxmark (4,x,y,pcm->digsiz+0.01);
	      else if (im==7) gxmark (12,x,y,pcm->digsiz+0.01); /* draw a closed diamond */
              else gxmark (im+1,x,y,pcm->digsiz+0.01);
              gxcolr(pcm->ccolor);
            }
            gxmark (im,x,y,pcm->digsiz+0.01);
          }
        }
        gr++; gru++;
        if (flag==2) { 
	  gr2++; gr3++; 
	  gr2u++; gr3u++;
	}
      }
    }
  }
  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  if (rotflg) 
    gaaxpl(pcm,5,pgr->idim);   /* hard coded 4 changed to 5 */
  else 
    gaaxpl(pcm,pgr->idim,5);   /* hard coded 4 changed to 5 */
  gxwide (4);
  gxcolr (pcm->anncol);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  if (barflg) gagsav (8,pcm,pgr);
  else  gagsav (6,pcm,pgr);
}

/* Do 2-D streamlines */

void gastrm (struct gacmn *pcm) {
struct gagrid *pgru, *pgrv, *pgrc=NULL;
gadouble *u, *v, *c=0;
gaint flag,lcol;
char *umask, *vmask, *cmask=NULL;

  if (pcm->numgrd<2) {
    gaprnt (0,"Error plotting streamlines:  Only one grid provided\n");
    return;
  }
  pgru = pcm->result[0].pgr;
  pgrv = pcm->result[1].pgr;
  if (pgru->idim!=pgrv->idim || pgru->jdim!=pgrv->jdim ||
      gagchk(pgru,pgrv,pgru->idim) || gagchk(pgru,pgrv,pgru->jdim)) {
    gaprnt (0,"Error plotting streamlines:  Invalid grids\n");
    gaprnt (0,"   Vector component grids have difference scaling\n");
    return;
  }
  flag = 0;
  if (pcm->numgrd>2) {
    flag = 1;
    pgrc = pcm->result[2].pgr;
    if (pgrc->idim!=pgru->idim || pgrc->jdim!=pgru->jdim ||
        gagchk(pgrc,pgru,pgru->idim) || gagchk(pgrc,pgru,pgru->jdim)) {
      flag = 0;
      gaprnt (0,"Error plotting streamlines:  Invalid color grid");
      gaprnt (0,"   Color grid ignored -- has different scaling");
    }
  }

  if ( (pcm->rotate && (pgru->idim!=2 || pgru->jdim!=3)) ||
      (!pcm->rotate && pgru->idim==2 && pgru->jdim==3)) {
    pgru = gaflip(pgru,pcm);
    pgrv = gaflip(pgrv,pcm);
    if (flag) pgrc = gaflip(pgrc,pcm);
  }

  gxstyl (1);
  gxwide (1);
  gas2d (pcm, pgru, 1);     /* Set up scaling, draw map if apprprt */

  gafram (pcm);
  idiv = 1.0; jdiv = 1.0;

  if (flag) {
    gamnmx (pgrc);
    if (pgrc->umin==0) {
      gaprnt (0,"Connot color vectors -- Color grid all undefined\n");
      flag = 0;
    } else gaselc(pcm,pgrc->rmin,pgrc->rmax);
  }

  u = pgru->grid;
  v = pgrv->grid;
  umask = pgru->umask;
  vmask = pgrv->umask;
  if (flag) {
    c = pgrc->grid;
    cmask = pgrc->umask;
  }

  if (pcm->ccolor>=0) lcol = pcm->ccolor;
  else lcol=1;
  gxcolr (lcol);
  gxstyl(pcm->cstyle);
  gxwide(pcm->cthick);
  gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);

  if (flag) {
    gxstrm (u,v,c,pgru->isiz,pgru->jsiz,umask,vmask,cmask,flag,
      pcm->shdlvs,pcm->shdcls,pcm->shdcnt,pcm->strmden,
      pcm->strmarrd,pcm->strmarrsz, pcm->strmarrt);
  } else {
    gxstrm (u,v,NULL,pgru->isiz,pgru->jsiz,umask,vmask,0,flag,
      pcm->shdlvs,pcm->shdcls,pcm->shdcnt,pcm->strmden,
      pcm->strmarrd,pcm->strmarrsz, pcm->strmarrt);
  }

  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  gxwide (4);
  gxcolr (pcm->anncol);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gaaxpl(pcm,pgru->idim,pgru->jdim);
  gagsav (9,pcm,pgru);
}

/* Do 2-D vector plot */

void gavect (struct gacmn *pcm, gaint brbflg) {
struct gagrid *pgru, *pgrv, *pgrc=NULL;
gadouble *u, *v, *c=0, x, y, lon, lat, umax, vmax;
gadouble vscal, adj, dir;
gaint i, j, flag, lcol, len, hemflg, hflg;
char *umask,*vmask,*cmask=NULL;

  if (pcm->numgrd<2) {
    gaprnt (0,"Error plotting vector field:  Only one grid provided\n");
    return;
  }
  pgru = pcm->result[0].pgr;
  pgrv = pcm->result[1].pgr;
  if (pgru->idim!=pgrv->idim || pgru->jdim!=pgrv->jdim ||
      gagchk(pgru,pgrv,pgru->idim) || gagchk(pgru,pgrv,pgru->jdim)) {
    gaprnt (0,"Error plotting vector/barb field:  Invalid grids\n");
    gaprnt (0,"   Vector component grids have difference scaling\n");
    return;
  }
  flag = 0;
  if (pcm->numgrd>2) {
    flag = 1;
    pgrc = pcm->result[2].pgr;
    if (pgrc->idim!=pgru->idim || pgrc->jdim!=pgru->jdim ||
        gagchk(pgrc,pgru,pgru->idim) || gagchk(pgrc,pgru,pgru->jdim)) {
      flag = 0;
      gaprnt (0,"Error plotting vector/barb field:  Invalid color grid");
      gaprnt (0,"   Color grid ignored -- has different scaling");
    }
  }

  if ( (pcm->rotate && (pgru->idim!=2 || pgru->jdim!=3)) ||
      (!pcm->rotate && pgru->idim==2 && pgru->jdim==3)) {
    pgru = gaflip(pgru,pcm);
    pgrv = gaflip(pgrv,pcm);
    if (flag) pgrc = gaflip(pgrc,pcm);
  }

  gxstyl (1);
  gxwide (1);
  gas2d (pcm, pgru, 1);     /* Set up scaling, draw map if apprprt */

  gafram (pcm);
  idiv = 1.0; jdiv = 1.0;

  if (flag) {
    gamnmx (pgrc);
    if (pgrc->umin==0) {
      gaprnt (0,"Cannot color vectors/barbs -- Color grid all undefined\n");
      flag = 0;
    } else gaselc(pcm,pgrc->rmin,pgrc->rmax);
  }

  gamnmx (pgru);
  gamnmx (pgrv);
  if (pgru->umin==0) {
    gaprnt (0,"Cannot draw vectors/barbs -- U field all undefined\n");
    return;
  }
  if (pgrv->umin==0) {
    gaprnt (0,"Cannot draw vectors/barbs -- V field all undefined\n");
    return;
  }

  if (pcm->ccolor>=0) lcol = pcm->ccolor;
  else lcol=1;
  gxcolr (lcol);
  gxstyl(1);
  gxwide(pcm->cthick);
  gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);
  if (!pcm->arrflg) {
    umax = pgru->rmax;
    if (umax<fabs(pgru->rmin)) umax = fabs(pgru->rmin);
    vmax = pgrv->rmax;
    if (vmax<fabs(pgrv->rmin)) vmax = fabs(pgrv->rmin);
    vscal = hypot(umax,vmax);
    if (vscal>0.0) {
      x = floor(log10(vscal));
      y = floor(vscal/pow(10.0,x));
      vscal = y * pow(10.0,x);
      pcm->arrsiz = 0.5;
    } else vscal=1.0;
    pcm->arrmag = vscal;
  } else {
    vscal = pcm->arrmag;
  }
  pcm->arrflg = 1;

  u = pgru->grid;
  v = pgrv->grid;
  umask = pgru->umask;
  vmask = pgrv->umask;
  if (flag) {
    c = pgrc->grid;
    cmask = pgrc->umask;
  }

  /* hflg=0 idim is lat
     hflg=1 jdim is lat
     hflg=2 plot nhem
     hflg=3 plot shem */

  if (pcm->hemflg==0) hflg = 2;
  else if (pcm->hemflg==1) hflg = 3;
  else {
    if (pgru->idim==1) hflg = 0;
    else if (pgru->jdim==1) hflg = 1;
    else {
      if (pcm->dmin[1] < 0.0) hflg = 3; 
      else hflg = 2;
    }
  }
  for (j=1; j<=pgru->jsiz; j++) {
  for (i=1; i<=pgru->isiz; i++) {
    if (*umask!=0 && *vmask!=0) {
      gxconv ((gadouble)i,(gadouble)j,&x,&y,3);
      gxgrmp ((gadouble)i,(gadouble)j,&lon,&lat);
      adj = gxaarw (lon, lat);
      if (adj < -900.0) {
        gaprnt(0,"Error: vector/barb not compatible with current map projection\n");
        return;
      }
      hemflg = 0;
      if (hflg==0 && lon<0.0) hemflg = 1;
      if (hflg==1 && lat<0.0) hemflg = 1;
      if (hflg==3) hemflg = 1;
      if (flag) {
        if (*cmask==0) gxcolr(15);
        else {
          lcol = gashdc(pcm,*c);
          if (lcol>-1) gxcolr(lcol);
        }
      }
      if (lcol>-1) {
        if (*v==0.0 && *u==0.0) dir = 0.0;
        else dir = atan2(*v,*u);
        if (brbflg) {
          gabarb (x, y, pcm->digsiz*3.5, pcm->digsiz*2.0,
                  pcm->digsiz*0.25, dir+adj, hypot(*u,*v), hemflg, pcm->barbolin);
        } else {
          if (vscal>0.0) {
            gaarrw (x, y, dir+adj, pcm->arrsiz*hypot(*u,*v)/vscal, pcm->ahdsiz);
          } else {
            gaarrw (x, y, dir+adj, pcm->arrsiz, pcm->ahdsiz);
          }
        }
      }
    }
    u++; v++; umask++; vmask++;
    if (flag) { c++; cmask++; }
  }}

  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);

  if (pcm->arlflg && vscal>0.0 && !brbflg) {
    gxcolr (pcm->anncol);
    gxwide (pcm->annthk-2);
    gaarrw (pcm->xsiz2-2.0,pcm->ysiz1-0.5,0.0,pcm->arrsiz, pcm->ahdsiz);
    snprintf(pout,1255,"%g",vscal);
    len = strlen(pout);
    x = pcm->xsiz2 - 2.0 + (pcm->arrsiz/2.0) - 0.5*0.13*(gadouble)len;
    gxchpl (pout,len,x,pcm->ysiz1-0.7,0.13,0.13,0.0);
  }
  gxwide (4);
  gxcolr (pcm->anncol);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gaaxpl(pcm,pgru->idim,pgru->jdim);
  if (brbflg) gagsav (15,pcm,pgru);
  else gagsav (3,pcm,pgru);
}

/* Do (for now 2-D) scatter plot */

gaint scatcol[6] = {1,3,2,4,7,8};
gaint scattyp[6] = {1,6,4,8,7,2};

void gascat (struct gacmn *pcm) {
struct gagrid *pgr1, *pgr2, *pgrc=NULL;
gadouble *r1, *r2, *c=NULL, x, y;
gadouble cmin1,cmax1,cmin2,cmax2,cint1,cint2;
gaint siz,i,pass,im,flag,lcol,drawthismark;
char *r1mask, *r2mask, *cmask=NULL;

  if (pcm->numgrd<2) {
    gaprnt (0,"Error plotting scatter field:  Only one grid provided\n");
    return;
  }
  if (pcm->type[0]==0 || pcm->type[1]==0) {
    gaprnt (0,"Error plotting scatter field:  stn argument(s) used\n");
    return;
  }
  pgr1 = pcm->result[0].pgr;
  pgr2 = pcm->result[1].pgr;
  if (pgr1->idim!=pgr2->idim || pgr1->jdim!=pgr2->jdim ||
      gagchk(pgr1,pgr2,pgr1->idim) ||
      (pgr1->jdim>-1 && gagchk(pgr1,pgr2,pgr1->jdim))) {
    gaprnt (0,"Error plotting scatter plot:  Invalid grids\n");
    gaprnt (0,"   The two grids have difference scaling\n");
    return;
  }
  flag = 0;
  if (pcm->numgrd>2) {
    flag = 1;
    pgrc = pcm->result[2].pgr;
    if (pgrc->idim!=pgr1->idim || pgrc->jdim!=pgr2->jdim ||
        gagchk(pgrc,pgr1,pgr1->idim) || gagchk(pgrc,pgr2,pgr2->jdim)) {
      flag = 0;
      gaprnt (0,"Error plotting scatter plot:  Invalid color grid");
      gaprnt (0,"   Color grid ignored -- has different scaling");
    }
  }

  pcm->xdim = 5;  /* hard coded 4's changed to 5's */
  pcm->ydim = 5;  /* hard coded 4's changed to 5's */

  gamnmx (pgr1);
  gamnmx (pgr2);
  if (pgr1->umin==0) {
    gaprnt (0,"Cannot draw scatter plot -- 1st field all undefined\n");
    return;
  }
  if (pgr2->umin==0) {
    gaprnt (0,"Cannot draw scatter plot -- 2nd field all undefined\n");
    return;
  }

  if (pcm->paflg) {
    pcm->xsiz1 = pcm->pxmin;
    pcm->xsiz2 = pcm->pxmax;
    pcm->ysiz1 = pcm->pymin;
    pcm->ysiz2 = pcm->pymax;
  } else {
    pcm->xsiz1 = 2.0;
    pcm->xsiz2 = pcm->xsiz - 1.5;
    pcm->ysiz1 = 1.00;
    pcm->ysiz2 = pcm->ysiz - 1.00;
  }
  gafram (pcm);
  gxwide (pcm->cthick);
  idiv = 1.0; jdiv = 1.0;

  if (flag) {
    gamnmx (pgrc);
    if (pgrc->umin==0) {
      gaprnt (0,"Cannot colorize scatterplot -- Color grid all undefined\n");
      flag = 0;
    } else gaselc(pcm,pgrc->rmin,pgrc->rmax);
  }

  if (pcm->aflag != 0) {
    cmin1 = pcm->rmin;
    cmax1 = pcm->rmax;
  } else {
    cint1 = 0.0;
    gacsel (pgr1->rmin,pgr1->rmax,&cint1,&cmin1,&cmax1);
    if (cint1==0.0) {
      cmin1 = pgr1->rmin-5.0;
      cmax1 = cmin1+10.0;
      cint1 = 2.0;
    } else {
      cmin1 = cmin1 - 2.0*cint1;
      cmax1 = cmax1 + 2.0*cint1;
    }
  }
  if (pcm->aflag2 != 0) {
    cmin2 = pcm->rmin2;
    cmax2 = pcm->rmax2;
  } else {
    cint2 = 0.0;
    gacsel (pgr2->rmin,pgr2->rmax,&cint2,&cmin2,&cmax2);
    if (cint2==0.0) {
      cmin2 = pgr2->rmin-5.0;
      cmax2 = cmin2+10.0;
      cint2 = 2.0;
    } else {
      cmin2 = cmin2 - 2.0*cint2;
      cmax2 = cmax2 + 2.0*cint2;
    }
  }
  snprintf(pout,1255,"%g %g %g %g \n",cmin1,cmax1,cmin2,cmax2);
  gaprnt(2,pout);
  gxscal (pcm->xsiz1,pcm->xsiz2,pcm->ysiz1,pcm->ysiz2,
          cmin1,cmax1,cmin2,cmax2);
  pass = pcm->gpass[3];
  if (pass>5) pass=5;
  gxwide (pcm->cthick);
  siz = pgr1->isiz*pgr1->jsiz;
  r1 = pgr1->grid;
  r2 = pgr2->grid;
  r1mask = pgr1->umask;
  r2mask = pgr2->umask;
  if (flag) {
    c = pgrc->grid;
    cmask = pgrc->umask;
  }
  for (i=0; i<siz; i++) {
    if (*r1mask!=0 && *r2mask!=0) {
      drawthismark=1;
      /* add color */
      if (flag) {
        if (*cmask==0) {
	  /* if color grid has missing data, use color 15 */
	  gxcolr(15);
          /* don't draw marks when color grid is missing */
          drawthismark=0;
	}
        else {
          lcol = gashdc(pcm,*c);
          if (lcol>-1) {
	    gxcolr(lcol);
	  }
	  else {
            drawthismark=0;
	  }
        }
      }
      else {
	/* if no color grid is given, draw it the old way */
	if (pcm->ccolor<0) gxcolr(1);   
	else gxcolr(pcm->ccolor);
      }
      if (drawthismark) {
	gxconv (*r1,*r2,&x,&y,1);
	im = pcm->cmark;
        gxmark (im,x,y,pcm->digsiz*0.5);
      }
    }
    r1++; r2++; r1mask++; r2mask++;
    if (flag) { c++; cmask++; }
  }

  pcm->rmin = cmin1;
  pcm->rmax = cmax1;
  pcm->rint = cint1;
  gaaxis(1,pcm,5);   /* hard coded 4's changed to 5's */
  pcm->rmin = cmin2;
  pcm->rmax = cmax2;
  pcm->rint = cint2;
  gaaxis(0,pcm,5);   /* hard coded 4's changed to 5's */

  pcm->rmin = cmin1;
  pcm->rmax = cmax1;
  pcm->aflag = 1;
  pcm->rmin2 = cmin2;
  pcm->rmax2 = cmax2;
  pcm->aflag2 = 1;

  if (cmin1<0.0 && cmax1>0.0) {
    gxstyl(1);
    gxwide(3);
    gxconv (0.0, cmin2, &x, &y, 1);
    gxplot (x,y,3);
    gxconv (0.0, cmax2, &x, &y, 1);
    gxplot (x,y,2);
  }
  if (cmin2<0.0 && cmax2>0.0) {
    gxstyl(1);
    gxwide(3);
    gxconv (cmin1, 0.0, &x, &y, 1);
    gxplot (x,y,3);
    gxconv (cmax1, 0.0, &x, &y, 1);
    gxplot (x,y,2);
  }
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gagsav (7,pcm,pgr1);
  pcm->gpass[3]++;
}

static gadouble a150 = 150.0*M_PI/180.0;

void gaarrw (gadouble x, gadouble y, gadouble ang, gadouble siz, gadouble asiz) {
gadouble xx,yy;

  if (siz<0.0001) {
    gxmark (2, x, y, 0.01);
    return;
  }
  gxplot (x,y,3);
  xx = x+siz*cos(ang);
  yy = y+siz*sin(ang);
  gxplot (xx,yy,2);
  if (asiz==0.0) return;
  if (asiz<0.0) asiz = -1.0*asiz*siz;
  gxplot (xx+asiz*cos(ang+a150),yy+asiz*sin(ang+a150),2);
  gxplot (xx,yy,3);
  gxplot (xx+asiz*cos(ang-a150),yy+asiz*sin(ang-a150),2);
}

/* Do 2-D grid value plot */

void gaplvl (struct gacmn *pcm) {
struct gagrid *pgr,*pgrm=NULL;
gadouble xlo,ylo,xhi,yhi,*r,*m=0,cwid;
gaint i,j,len,lcol,flag;
char *rmask,*mmask=NULL,lab[20];

  pgr = pcm->result[0].pgr;
  flag = 0;
  if (pcm->numgrd>1) {
    flag = 1;
    pgrm = pcm->result[1].pgr;
    if (pgrm->idim!=pgr->idim || pgrm->jdim!=pgr->jdim ||
        gagchk(pgrm,pgr,pgr->idim) || gagchk(pgrm,pgr,pgr->jdim)) {
      flag = 0;
      gaprnt (0,"Error plotting grid values:  Invalid Mask grid");
      gaprnt (0,"   Mask grid ignored -- has different scaling");
    }
  }

  if ( (pcm->rotate && (pgr->idim!=2 || pgr->jdim!=3)) ||
      (!pcm->rotate && pgr->idim==2 && pgr->jdim==3)) {
    pgr = gaflip(pgr,pcm);
    if (flag) pgrm = gaflip(pgrm,pcm);
  }

  gxstyl (1);
  gxwide (1);
  gas2d (pcm, pgr, 1);     /* Draw map and set up scaling */

  gafram (pcm);
  gxwide (pcm->cthick);
  idiv = 1.0; jdiv = 1.0;
  if (pcm->ccolor>=0) lcol = pcm->ccolor;
  else lcol=1;
  gxcolr(lcol);
  gxstyl(1);
  gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);

  /* Draw grid lines  */

  if (pcm->gridln != -1) {
    if (pcm->gridln > -1) gxcolr(pcm->gridln);
    for (i=1; i<=pgr->isiz; i++) {
      for (j=1; j<=pgr->jsiz; j++) {
        gxconv ((gadouble)(i)+0.5,(gadouble)(j)-0.5,&xlo,&ylo,3);
        gxconv ((gadouble)(i)+0.5,(gadouble)(j)+0.5,&xhi,&yhi,3);
        gxplot (xlo,ylo,3);
        gxplot (xhi,yhi,2);
      }
    }

    for (j=1; j<=pgr->jsiz; j++) {
      for (i=1; i<=pgr->isiz; i++) {
        gxconv ((gadouble)(i)-0.5,(gadouble)(j)+0.5,&xlo,&ylo,3);
        gxconv ((gadouble)(i)+0.5,(gadouble)(j)+0.5,&xhi,&yhi,3);
        gxplot (xlo,ylo,3);
        gxplot (xhi,yhi,2);
      }
    }
  }

  r = pgr->grid;
  rmask = pgr->umask;
  if (flag) {
    m = pgrm->grid;
    mmask = pgrm->umask;
  }
  for (j=1; j<=pgr->jsiz; j++) {
  for (i=1; i<=pgr->isiz; i++) {
    if (*rmask!=0) {
      if (flag && *mmask!=0 && *m<=0.0) {
        gxwide (1);
        gxcolr (15);
      } else {
        gxwide (pcm->cthick);
        gxcolr (lcol);
      }
      gxconv ((gadouble)i,(gadouble)j,&xlo,&ylo,3);
      snprintf(lab,19,"%.*f",pcm->dignum,(gafloat)*r);
      len = strlen(lab);
      cwid = pcm->digsiz*(gadouble)len;
      gxchln (lab,len,pcm->digsiz,&cwid);
      gxchpl (lab,len,xlo-cwid*0.5,ylo-pcm->digsiz*0.5,
              pcm->digsiz,pcm->digsiz,0.0);
    }
    r++; rmask++;
    if (flag) { m++; mmask++; }
  }}

  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  gxcolr(pcm->anncol);
  gxwide(4);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gaaxpl(pcm,pgr->idim,pgr->jdim);
  gagsav (4,pcm,pgr);

}

/* Writes out a shapefile */

void gashpwrt (struct gacmn *pcm) {
#if USESHP==1
FILE *fp=NULL;
gadouble (*conv) (gadouble *, gadouble);
gadouble lon,lat;  
struct gagrid *pgr=NULL;
struct gastn *stn=NULL;
struct garpt *rpt;
SHPObject *shp;
gaint *pstart=NULL,nParts,shpid,i,j,gx,grid,len;
gadouble *gr=NULL,val,dval;
char *gru=NULL, *prjname=NULL, *fnroot=NULL;
gaint rc,width,prec,fldindx,ival,error=0;
char fldname[12];
SHPHandle sfid=NULL;
DBFHandle dbfid=NULL;
struct dbfld *fld=NULL,*newfld=NULL,*nextfld;


  /* Make sure projection is latlon */
  if (pcm->mproj !=2) {
    gaprnt (0,"Error in gashpwrt: mproj latlon required for gxout shapefile\n");
    error = 1;
    goto cleanup;
  }

  /* Determine if output is a grid or a station result */
  grid = 1;
  if (pcm->type[0] == 1) {
    /* gridded data */
    pgr = pcm->result[0].pgr;
    gr  = pgr->grid;
    gru = pgr->umask;

    /* Make sure grid varies in X and Y */
    if (pgr->idim!=0 || pgr->jdim!=1) {
      gaprnt(0,"Error in gashpwrt: grid is not varying in X and Y \n");
      error = 1;
      goto cleanup;
    }
    /* set up scaling without the map (this is not done in gacntr when shpflg=1) */
    gas2d (pcm, pgr, 0);    
  }
  else {
    /* station data */
    grid = 0;
    stn = pcm->result[0].stn;
    /* shapefile type is always point for station data */
    if (pcm->shptype!=1) {
      gaprnt(0,"Error in gashpwrt: Incorrect shapefile output type for station data \n");
      gaprnt(0,"   You must use the -pt option with the 'set shp' command \n");
      error = 1;
      goto cleanup;
    }
    /* Make sure reports are not all undefined */
    gasmnmx (stn);
    if (dequal(stn->smin,stn->undef,1e-12)==0 || 
	dequal(stn->smax,stn->undef,1e-12)==0) {
      gaprnt (0,"Error in gashpwrt: all reports are undefined \n");
      error = 1;
      goto cleanup;
    }
    if (dequal(stn->smin,stn->smax,1e-12)==0) {
      snprintf(pout,1255,"Warning from gashpwrt: all reports have the same value = %g\n",stn->smin);
      gaprnt(2,pout);
    }
    /* Do map level scaling (copied from gasmrk) */
    gamscl (pcm);
  }

  /* Create the output files */
  if (pcm->shpfname == NULL) {
    fnroot = (char *)galloc(6,"shpfn");
    snprintf(fnroot,6,"grads");
  }
  else {
    len = strlen(pcm->shpfname);
    fnroot = (char *)galloc(len+1,"shpfn");
    snprintf(fnroot,len+1,"%s",pcm->shpfname);
  }
  if ((dbfid = DBFCreate(fnroot))==NULL) {
    gaprnt(0,"Error in gashpwrt: Unable to create data base file\n");
    error = 1; goto cleanup;
  }
  if (pcm->shptype==1) {
    if ((sfid = SHPCreate(fnroot,SHPT_POINTM))==NULL) {
      gaprnt(0,"Error in gashpwrt: Unable to create shapefile for point data\n");
      error = 1; goto cleanup;
    }
  } 
  else if (pcm->shptype==2) {
    if ((sfid = SHPCreate(fnroot,SHPT_ARCM))==NULL) {
      gaprnt(0,"Error in gashpwrt: Unable to create shapefile for contour lines\n");
      error = 1; goto cleanup;
    }
  } 
  else {
    if ((sfid = SHPCreate(fnroot,SHPT_POLYGONM))==NULL) {
      gaprnt(0,"Error in gashpwrt: Unable to create shapefile for polygons\n");
      error = 1; goto cleanup;
    }
  }
  /* Set up the list of data base fields. */

  /* Allocate a new field, the GrADS version, set it as the anchor in the local chain */
  snprintf(pout,1255,"GrADS-"GRADS_VERSION"");
  len = strlen(pout);
  fld = newdbfld("CREATED_BY", FTString, len, 0, 0, pout);
  if (fld==NULL) {
    error = 1; goto cleanup;
  }
  if (dbanch==NULL) 
    dbanch = fld;                      /* this is the first field */
  else 
    dblast->next = fld;                /* hang it off the end of the chain */
  dblast = fld;                        /* reset the last pointer */
  dblast->next = NULL;                 /* make sure the chain is terminated */

  /* Copy the user-provided fields that are linked off of gacmn onto the local chain */
  if (pcm->dbfld) { 
    fld = pcm->dbfld;  
    while (fld) {
      if ((newfld = (struct dbfld*) galloc (sizeof(struct dbfld),"udbfld"))==NULL) {
	error = 1; goto cleanup;
      }
      strcpy(newfld->name,fld->name); 
      newfld->type  = fld->type;
      newfld->len   = fld->len;
      newfld->prec  = fld->prec;
      newfld->index = fld->index;
      newfld->flag  = fld->flag;
      if ((newfld->value = (void*)galloc(fld->len,"newdbval"))==NULL) {
	gree(newfld,"g292");
	error = 1; goto cleanup;
      }
      strcpy(newfld->value,fld->value);
      dblast->next = newfld;
      dblast = newfld;
      dblast->next = NULL;  
      fld = fld->next;
    } 
  }

  /* Now add more GrADS-provided fields that vary depending on shape file type.
     These are 'dynamic' dbase fields, where values are different for each shape:
     grid points: longitude, latitude, and grid value 
     station points: longitude, latitude, stid, and station value 
     contours: contour value 
     polygons: 
  */

  width = pcm->dblen;
  prec  = pcm->dbprec;
  if (pcm->shptype==1) {
    /* all point types get lon and lat */
    snprintf(fldname,11,"LONGITUDE");
    if ((fld = newdbfld (fldname, FTDouble, width, prec, 1, NULL)) != NULL) {
      dblast->next = fld; 
      dblast = fld;
      fld->next = NULL;
    } 
    snprintf(fldname,11,"LATITUDE");
    if ((fld = newdbfld (fldname, FTDouble, width, prec, 1, NULL)) != NULL) {
      dblast->next = fld; 
      dblast = fld;
      fld->next = NULL;
    }
    if (grid) {
      /* add grid point value */
      snprintf(fldname,11,"GRID_VALUE");
      if ((fld = newdbfld (fldname, FTDouble, width, prec, 1, NULL)) != NULL) { 
	dblast->next = fld; 
	dblast = fld;
	fld->next = NULL;
      }
    } else {
      /* add stid and station data value */
      snprintf(fldname,11,"STN_ID");
      if ((fld = newdbfld (fldname, FTString, 9, 0, 1, NULL)) != NULL) {
	dblast->next = fld; 
	dblast = fld;
	fld->next = NULL;
      }
      snprintf(fldname,11,"STN_VALUE");
      if ((fld = newdbfld (fldname, FTDouble, width, prec, 1, NULL)) != NULL) { 
	dblast->next = fld; 
	dblast = fld;
	fld->next = NULL;
      }
    }
  } else if  (pcm->shptype==2) {
    /* add contour value */
    snprintf(fldname,11,"CNTR_VALUE");
    if ((fld = newdbfld (fldname, FTDouble, width, prec, 1, NULL)) != NULL) { 
      dblast->next = fld; 
      dblast = fld;
      fld->next = NULL;
    }
  } else {
    /* add polygon index number and range values */
    snprintf(fldname,11,"INDEX");
    if ((fld = newdbfld (fldname, FTDouble, width, prec, 1, NULL)) != NULL) { 
      dblast->next = fld; 
      dblast = fld;
      fld->next = NULL;
    }
    snprintf(fldname,11,"MIN_VALUE");
    if ((fld = newdbfld (fldname, FTDouble, width, prec, 1, NULL)) != NULL) { 
      dblast->next = fld; 
      dblast = fld;
      fld->next = NULL;
    }
    snprintf(fldname,11,"MAX_VALUE");
    if ((fld = newdbfld (fldname, FTDouble, width, prec, 1, NULL)) != NULL) { 
      dblast->next = fld; 
      dblast = fld;
      fld->next = NULL;
    }
  }

  /* Add list of data base fields to the output file */
  fld = dbanch;  
  while (fld != NULL) {
    if (fld->type==FTString) {
      if ((fldindx = DBFAddField(dbfid,fld->name,FTString,fld->len,0))==-1) {
	gaprnt(0,"Error in gashpwrt: Unable to add string field to data base file\n");
	error = 1; goto cleanup;
      }
      fld->index = fldindx; 
    }
    else if (fld->type==FTInteger) {
      if ((fldindx = DBFAddField(dbfid,fld->name,FTInteger,fld->len,0))==-1) {
	gaprnt(0,"Error in gashpwrt: Unable to add integer field to data base file\n");
	error = 1; goto cleanup;
      }
      fld->index = fldindx; 
    }
    else if (fld->type==FTDouble) {
      if (fld->len > pcm->dblen) fld->len = pcm->dblen;
      if ((fldindx = DBFAddField(dbfid,fld->name,FTDouble,fld->len,pcm->dbprec))==-1) {
	gaprnt(0,"Error in gashpwrt: Unable to add integer field to data base file\n");
	error = 1; goto cleanup;
      }
      fld->index = fldindx; 
    }
    fld = fld->next;
  }
  
  /* Write out point values */
  if (pcm->shptype==1) {

    /* when you only have one point, pstart will always be zero */
    nParts = 1;  
    if ((pstart = (gaint*)galloc(nParts*sizeof(gaint),"pstart"))==NULL) {
      gaprnt(2,"Memory allocation error in gashpwrt\n");
      error = 1; goto cleanup;
    }
    *pstart = 0;

    /* For grid expressions, loop over all grid points */
    if (grid) {
      shpid = 0;   /* shape index/count */
      gx = 0;      /* grid index */
      for (j=pgr->dimmin[1]; j<=pgr->dimmax[1]; j++) {
	for (i=pgr->dimmin[0]; i<=pgr->dimmax[0]; i++) {
	  if (*(gru+gx) != 0) {
	    /* get the data value and the lat/lon for each grid point that is not undefined */
	    val = *(gr+gx);
	    conv = pcm->xgr2ab;
	    lon = conv(pcm->xgrval, i);
	    conv = pcm->ygr2ab;
	    lat = conv(pcm->ygrval, j);
	    
	    /* create the shape, write it to the file, then release it */
	    shp = SHPCreateObject (SHPT_POINTM,shpid,nParts,pstart,NULL,1,&lon,&lat,NULL,&val);
	    rc = SHPWriteObject (sfid,-1,shp);
	    SHPDestroyObject (shp);
	    if (rc!=shpid) {
	      snprintf(pout,1255,"Error in gashpwrt: SHPWriteObject returned %d, shpid=%d\n",rc,shpid);
	      gaprnt (0,pout);
	      error = 1; goto cleanup;
	    }
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
		if (strcmp(fld->name,"LONGITUDE")==0) {
		  DBFWriteDoubleAttribute (dbfid,shpid,fld->index,lon);
		} else if (strcmp(fld->name,"LATITUDE")==0) {
		  DBFWriteDoubleAttribute (dbfid,shpid,fld->index,lat);
		} else if (strcmp(fld->name,"GRID_VALUE")==0) {
		  DBFWriteDoubleAttribute (dbfid,shpid,fld->index,val);
		}
	      }
	      fld = fld->next;      /* advance to next field */
	    }
	    shpid++;
	  }
	  gx++;
	}
      }
      snprintf(pout,1255,"%d grid point values written to shapefile %s\n",shpid,fnroot);
      gaprnt(2,pout);
    }
    else {
      /* Loop over all reports */
      shpid = 0;   /* shape index/count */
      rpt = stn->rpt;
      while (rpt!=NULL) {
	if (rpt->umask != 0) {
	  lon = rpt->lon;
	  lat = rpt->lat;
	  /* normalize the longitude */
	  if (lon<pcm->dmin[0]) lon+=360.0;
	  if (lon>pcm->dmax[0]) lon-=360.0;
	  /* check if report is within specified domain */
	  if (lon>pcm->dmin[0] && 
	      lon<pcm->dmax[0] &&
	      lat>pcm->dmin[1] && 
	      lat<pcm->dmax[1]) {
	    /* get the data value */
	    val = rpt->val;
	    /* create the shape, write it to the file, then release it */
	    shp = SHPCreateObject (SHPT_POINTM,shpid,nParts,pstart,NULL,1,&lon,&lat,NULL,&val);
	    rc = SHPWriteObject (sfid,-1,shp);
	    SHPDestroyObject (shp);
	    if (rc!=shpid) {
	      snprintf(pout,1255,"Error in gashpwrt: SHPWriteObject returned %d, shpid=%d\n",rc,shpid);
	      gaprnt (0,pout);
	      error = 1; goto cleanup;
	    }
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
		if (strcmp(fld->name,"LONGITUDE")==0) {
		  DBFWriteDoubleAttribute (dbfid,shpid,fld->index,lon);
		} else if (strcmp(fld->name,"LATITUDE")==0) {
		  DBFWriteDoubleAttribute (dbfid,shpid,fld->index,lat);
		} else if (strcmp(fld->name,"STN_ID")==0) {
		  DBFWriteStringAttribute (dbfid,shpid,fld->index,rpt->stid);
		} else if (strcmp(fld->name,"STN_VALUE")==0) {
		  DBFWriteDoubleAttribute (dbfid,shpid,fld->index,val);
		}
	      }
	      fld = fld->next;      /* advance to next field */
	    }
	    shpid++;
	  }
	}
	rpt = rpt->rpt;
      }
      snprintf(pout,1255,"%d station reports written to shapefile %s\n",shpid,fnroot);
      gaprnt(2,pout);
    }

  }
  /* Write out contour lines */
  else if (pcm->shptype==2) {
    /* Call gacntr() to create buffered contour lines */
    rc = gacntr (pcm, 0, 1); 
    if (rc) {
      error = 1; goto cleanup;
    }

    /* call routine in gxcntr.c to write out contour line vertices and values */
    rc = gxshplin(sfid,dbfid,dbanch);
    if (rc>0) {
      snprintf(pout,1255,"%d contours written to shapefile %s\n",rc,fnroot);
      gaprnt(2,pout);
    }
    else if (rc==-1) {
      error = 1;
    }

    /* release the contour buffer from memory */
    gxcrel();
  
  } 
  /* Write out polygons */
  else {
    s2setbuf(1);    /* turn on polygon buffering */
    s2setdraw(1);   /* disable drawing of polygons to display */

    /* call gacntr() to create shaded polygons with gxshad2 */
    rc = gacntr (pcm,4,1);
    if (rc) {
      error = 1; goto cleanup;
    }

    /* call routine in gxshad2.c to write out polygon vertices and values */
    rc = s2shpwrt(sfid,dbfid,dbanch);
    if (rc>0) {
      snprintf(pout,1255,"%d polygons written to shapefile %s\n",rc,fnroot);
      gaprnt(2,pout);
    }
    else if (rc==-1) {
      error = 1;
    }
    s2frepbuf();    /* release the polygon buffer from memory */
    s2setbuf(0);    /* turn off polygon buffering */
    s2setdraw(0);   /* restore drawing of polygons */

  }

  /* write the projection file */
  if ((prjname = (char *)galloc(5+strlen(fnroot),"prjname"))==NULL) {
    gaprnt(0,"Error in gashpwrt: memory allocation error for prjname\n");
    error = 1; goto cleanup;
  }
  sprintf(prjname,"%s.prj",fnroot);
  fp = fopen(prjname,"w");
  snprintf(pout,1255,"GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]\n");
  if ((fwrite(pout,1,strlen(pout),fp))!=strlen(pout)) {
    gaprnt(0,"Error in gashpwrt when writing to .prj file \n");
    error = 1; goto cleanup; 
  }
  gree(prjname,"f293d");
  prjname = NULL;

  gagsav(25,pcm,pgr);
  
cleanup:
  if (pstart  != NULL) gree(pstart,"f293c");
  if (prjname != NULL) gree(prjname,"f293d");
  if (fnroot  != NULL) gree(fnroot,"f293e");
  /* release local copy of data base attributes */
  if (dbanch != NULL) {
    while (dbanch != NULL) {
      /* point to first block in chain */
      fld = dbanch;  
      if (fld->next == NULL) {
	/* first block is only block */
	dbanch = NULL; 
      } 
      else { 
	/* move start of chain from 1st to 2nd block */
	nextfld = fld->next;
	dbanch = nextfld;
      }
      /* release memory from 1st block */
      if (fld->value != NULL) gree(fld->value,"f292b");
      gree(fld,"f293b");
    }
  }
  if (dblast != NULL) dblast = NULL;
  /* if an error occurred, no shapefile is written */
  if (error) {
    gaprnt(0,"Shapefile not created\n");
  }
  /* close files */
  if (sfid  != NULL) SHPClose(sfid);
  if (dbfid != NULL) DBFClose(dbfid);
  fclose(fp);


  return;
#else 
  gaprnt(0,"Creating shapefiles is not supported in this build\n");
#endif
}

/* allocates and populates a data base field 
   flag = 0 for static fields (the same values for all shapes)
   flag = 1 for dynamic fields (values vary with shape)
   
*/
#if USESHP==1
struct dbfld* newdbfld (char *fldname, DBFFieldType dbtype, gaint len, gaint prec, 
			gaint flag, char *val) {
  gaint sz;
  struct dbfld *newfld;
  char *value;
  
  /* create the new field */
  newfld = (struct dbfld *) galloc (sizeof(struct dbfld),"dbfld"); 
  if (newfld != NULL) {
    strcpy(newfld->name,fldname);
    newfld->type = dbtype;
    newfld->len = len;
    newfld->prec = prec;
    newfld->flag = flag;        
    if (flag==0) {
      /* allocate space for the field value */
      sz = (len+1) * sizeof(char);
      if ((value = (void *)galloc(sz,"valuec")) == NULL) {
	gaprnt (0,"Error in newdbfld: memory allocation failed for data base field value \n");
	gree (newfld,"g1");
	return NULL;
      }
      strcpy(value,val);
      newfld->value = value;
    }
    else {
      newfld->value = NULL;
    }
    newfld->next = NULL;
    return newfld;  
  } else {
    gaprnt (0,"Error in newdbfld: memory allocation failed for new data base field \n");
    return NULL;
  }
}
#endif

/* Writes out a KML file containing output from contour/shade2 routine */

void gakml (struct gacmn *pcm) {
  FILE *kmlfp=NULL;
  struct gagrid *pgr;
  struct gxdbquery dbq;
  gaint r,g,b,a,err=0,i,rc;
  
  /* Determine if output is a grid or a station result */
  if (pcm->type[0] != 1) {
    gaprnt (0,"Error in gakml: expression is not a grid \n");
    goto cleanup;
  }

  /* Make sure projection is latlon */
  if (pcm->mproj !=2) {
    gaprnt (0,"Error in gakml: mproj latlon required for gxout kml\n");
    goto cleanup;
  }

  /* Make sure we have an X-Y plot */
  pgr = pcm->result[0].pgr;
  if (pgr->idim!=0 || pgr->jdim!=1) {
    gaprnt(0,"Error in gakml: Grid is not varying in X and Y \n");
    goto cleanup;
  }
  
  /* set up scaling without the map */
  gas2d (pcm, pgr, 0);    

  /* Determine data min/max, make sure grid is not undefined */
  gamnmx (pgr);
  if (pgr->umin==0) {
    gaprnt (0,"Error in gakml: Entire grid is undefined \n");
    goto cleanup;
  }
  
  /* open the output file */
  if (pcm->kmlname)
    kmlfp = fopen (pcm->kmlname,"wb");
  else
    kmlfp = fopen ("grads.kml","wb");
  if (kmlfp==NULL) {
    if (pcm->kmlname)
      snprintf(pout,1255,"Error: fopen failed for KML text output file %s\n",pcm->kmlname);
    else
      snprintf(pout,1255,"Error: fopen failed for KML text output file grads.kml\n");
    gaprnt(0,pout);
    goto cleanup;
  }

  if (pcm->kmlflg==2) {
    rc = gacntr (pcm, 0, 1);     /* Call gacntr() to create buffered contour lines for KML file */
    if (rc) goto cleanup;
  } 
  else if (pcm->kmlflg==3) {
    s2setbuf(1);                 /* turn on polygon buffering */
    s2setdraw(1);                /* disable drawing polygons to display */
    rc = gacntr (pcm, 4, 1);     /* Call gacntr() to create buffered polygons for KML file */
    if (rc) goto cleanup;

  } else {
    gaprnt(9,"logic errror in subroutine gakml\n");
    goto cleanup;
  }



  /* write out KML headers */
  snprintf(pout,1255,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
  snprintf(pout,1255,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
  if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
  snprintf(pout,1255,"  <Document id=\"Created by GrADS-"GRADS_VERSION"\">\n");
  if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}

  /* Contours */
  if (pcm->kmlflg==2) {
    /* write out the contour colors as a set of Style tags with LineStyle */
    for (i=0; i<pcm->cntrcnt; i++) {
      snprintf(pout,1255,"    <Style id=\"%d\">\n      <LineStyle>\n",pcm->cntrcols[i]);
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      /* get rgb values for this color */
      gxdbqcol(pcm->cntrcols[i], &dbq);
      r = dbq.red;
      g = dbq.green;
      b = dbq.blue;
      a = dbq.alpha;
      snprintf(pout,1255,"        <color>%02x%02x%02x%02x</color>\n",a,b,g,r);
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"        <width>%d</width>\n",pcm->cthick);
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"      </LineStyle>\n    </Style>\n");
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
    }
    /* write out the locations of the contour vertices */
    rc = gxclvert(kmlfp);
    if (rc>0) {
      if (pcm->kmlname)
	snprintf(pout,1255,"%d contours written to KML file %s\n",rc,pcm->kmlname);
      else
	snprintf(pout,1255,"%d contours written to KML file grads.kml\n",rc);
      gaprnt(2,pout);
    }
    else err=1;
  }
  /* Polygons */
  else {
    /* write out the polygon colors as a set of Style tags with LineStyle and PolyStyle */
    for (i=0; i<pcm->shdcnt; i++) {
      snprintf(pout,1255,"    <Style id=\"%d\">\n",pcm->shdcls[i]);
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"      <LineStyle>\n");
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      /* get rgb values for this color */
      gxdbqcol(pcm->shdcls[i], &dbq);
      r = dbq.red;
      g = dbq.green;
      b = dbq.blue;
      a = dbq.alpha;
      snprintf(pout,1255,"        <color>%02x%02x%02x%02x</color>\n",a,b,g,r);
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"        <width>0</width>\n");
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"      </LineStyle>\n");
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"      <PolyStyle>\n");
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"        <color>%02x%02x%02x%02x</color>\n",a,b,g,r);
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"        <fill>1</fill>\n");
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"      </PolyStyle>\n");
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
      snprintf(pout,1255,"    </Style>\n");
      if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
    }

    /* write out the locations of the polygon vertices */
    /* call routine in gxshad2.c to write out polygon vertices and values */
    rc = s2polyvert(kmlfp);
    if (rc>0) {
      if (pcm->kmlname)
	snprintf(pout,1255,"%d polygons written to KML file %s\n",rc,pcm->kmlname);
      else
	snprintf(pout,1255,"%d polygons written to KML file grads.kml\n",rc);
      gaprnt(2,pout);
    }
    else err=1;

  }

  /* write out footers */
  snprintf(pout,1255,"  </Document>\n</kml>\n");
  if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
  /* set the last graphic code */
  gagsav (24,pcm,NULL);
    
 cleanup:
  if (pcm->kmlflg==2) {
    gxcrel();       /* release storage used by the contouring package */
  } else {
    s2frepbuf();    /* release the polygon buffer from memory */
    s2setbuf(0);    /* turn off polygon buffering */
    s2setdraw(0);   /* restore drawing of polygons */
  }
  if (kmlfp) fclose(kmlfp);    /* close the file */
  if (err) gaprnt(0,"Error from fwrite when writing KML file\n");
  return;
}


/* Writes out the grid in GeoTIFF format.

   If kmlflag is 0, one output file is created:
   a GeoTIFF containing a grid of floating-point or 
   double precision data values and the geolocation metadata. 

   If kmlflag is 1, two output files are created.
   The first is a GeoTIFF containing a grid of color index values 
   (based on default or user-specified contour levels), a color map 
   with RGB values for each color index, and the geolocation metadata. 
   The second is a KML file, which points to the GeoTIFF image 
   and contains the geolocation metadata in text form. 
   The KML file is intended for use with Google Earth.

 */

void gagtif (struct gacmn *pcm, gaint kmlflg) {
#if GEOTIFF==1 
 struct gagrid *pgr;
 struct gxdbquery dbq;
 gadouble *gr,cmin,cmax,cint,pmin,pmax,dval;
 gadouble pixelscale[3],tiepoints[24];
 gafloat fval;
 gaint i,j,rc,grsize,isize,jsize,color,r,g,b,CMAX;
 char *gru;
 TIFF *tif=NULL;
 GTIF *gtif=NULL;
 gadouble xresolution,yresolution,smin,smax;
 uint32 imagewidth,imagelength,rowsperstrip;
 uint16 *colormap=NULL,*cm;
 uint16 bitspersample,samplesperpixel,compression;
 uint16 photometric,resolutionunit,sampleformat;
 short depth;
 unsigned char *cbuf=NULL,*cbuf0=NULL;
 gafloat *fbuf=NULL,*fbuf0=NULL;
 gadouble *dbuf=NULL,*dbuf0=NULL;


 /* set up scaling without the map */
 pgr = pcm->result[0].pgr;
 gas2d (pcm, pgr, 0);    
 isize = pgr->isiz;
 jsize = pgr->jsiz;
 grsize = isize * jsize;
 gr  = pgr->grid;
 gru = pgr->umask;

 /* Make sure we have an X-Y plot */
 if (pgr->idim!=0 || pgr->jdim!=1) {
   gaprnt(0,"Error: Grid is not varying in X and Y \n");
   goto cleanup;
 }

 /* Make sure projection is latlon */
 if (pcm->mproj !=2) {
   gaprnt (0,"Error: mproj latlon required for gxout kml\n");
   goto cleanup;
 }
 
 /* Determine data min/max, make sure grid is not undefined */
 gamnmx (pgr);
 if (pgr->umin==0) {
   gaprnt (0,"Error: Entire grid is undefined \n");
   goto cleanup;
 }

 /* Open output files */
 if (kmlflg) {
   /* open the file for the image output, we'll open the KML file later */
   if (pcm->tifname)
     tif = XTIFFOpen(pcm->tifname, "w");
   else
     tif = XTIFFOpen("grads.tif", "w");
   if (tif==NULL) {
     if (pcm->tifname)
       snprintf(pout,1255,"Error: XTiffOpen failed for KML image output file %s\n",pcm->tifname);
     else
       snprintf(pout,1255,"Error: XTiffOpen failed for KML image output file grads.tif\n");
     gaprnt (0,pout);
     goto cleanup;
   }
   gtif = GTIFNew(tif);
   if (gtif==NULL) {
     if (pcm->tifname)
       snprintf(pout,1255,"Error: GTIFNew failed for KML image output file %s\n",pcm->tifname);
     else
       snprintf(pout,1255,"Error: GTIFNew failed for KML image output file grads.tif\n");
     gaprnt (0,pout);
     goto cleanup;
   }
 }
 else { 
   /* For gxout GeoTIFF, we open only one output file */
   if (pcm->gtifname)
     tif = XTIFFOpen(pcm->gtifname, "w");
   else
     tif = XTIFFOpen("gradsgeo.tif", "w");
   if (tif==NULL) {
     if (pcm->gtifname)
       snprintf(pout,1255,"Error: XTiffOpen failed for GeoTIFF output file %s\n",pcm->gtifname);
     else
       snprintf(pout,1255,"Error: XTiffOpen failed for GeoTIFF output file gradsgeo.tif\n");
     gaprnt (0,pout);
     goto cleanup;
   }
   gtif = GTIFNew(tif);
   if (gtif==NULL) {
     if (pcm->gtifname)
       snprintf(pout,1255,"Error: GTIFNew failed for GeoTIFF output file %s\n",pcm->gtifname);
     else
       snprintf(pout,1255,"Error: GTIFNew failed for GeoTIFF output file gradsgeo.tif\n");
     gaprnt (0,pout);
     goto cleanup;
   }
 }

 /* Determine the data type of the output */
 if (kmlflg) { 
   depth = TIFFDataWidth(TIFF_BYTE);
 }
 else {
   if (pcm->gtifflg==2) 
     depth = TIFFDataWidth(TIFF_DOUBLE);
   else 
     depth = TIFFDataWidth(TIFF_FLOAT);
 }

 /* Set values for required TIFF fields, converted to proper data types */
 imagewidth  = (uint32)isize;                /* #cols, #longitudes */
 imagelength = (uint32)jsize;                /* #rows, #latitudes */
 bitspersample = depth * 8;                  /* number of bits per component */
 samplesperpixel = 1;                        /* number of components per pixel */
 compression = 1;                            /* no compression used */
 if (kmlflg) 
   photometric = 3;                          /* palette color image */
 else
   photometric = 1;                          /* grayscale image */
 rowsperstrip = 1;                           /* one row per strip */
 resolutionunit = 2;                         /* inches */
 xresolution = (gadouble)(pcm->xsiz/isize);  /* gridpoints per inch */
 yresolution = (gadouble)(pcm->ysiz/jsize);  /* gridpoints per inch */
 if (!kmlflg) {
   sampleformat = 3;                         /* IEEE floating point data */
   smin = pgr->rmin;                         /* minimum value of grid */
   smax = pgr->rmax;                         /* maximum value of grid */
 }
 else {
   sampleformat = 1;                         /* unsigned integer */
   smin = 0;
   smax = 255;
 }
 /* write out the required TIFF metadata */
 if (TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, imagewidth)!=1)  { 
   gaprnt(0,"Error: TIFFSetField failed for imagewidth\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_IMAGELENGTH, imagelength)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for imagelength\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitspersample)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for bitspersample\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for samplesperpixel\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_COMPRESSION, compression)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for compression\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for photometric\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for rowsperstrip\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, resolutionunit)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for resolutionunit\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_XRESOLUTION, xresolution)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for xresolution\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_YRESOLUTION, yresolution)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for yresolution\n"); goto cleanup; 
 }
 if (TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, sampleformat)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for sampleformat\n"); goto cleanup; 
 }
 if (!kmlflg) {
   if (TIFFSetField(tif, TIFFTAG_SMINSAMPLEVALUE, smin)!=1) {
     gaprnt(0,"Error: TIFFSetField failed for sminsamplevalue\n"); goto cleanup;
   }
   if (TIFFSetField(tif, TIFFTAG_SMAXSAMPLEVALUE, smax)!=1) {
     gaprnt(0,"Error: TIFFSetField failed for smaxsamplevalue\n"); goto cleanup;
   }
 }
 snprintf(pout,1255,"GrADS version "GRADS_VERSION" ");
 if (TIFFSetField(tif, TIFFTAG_SOFTWARE, pout)!=1) { 
   gaprnt(0,"Error: TIFFSetField failed for software\n"); goto cleanup; 
 }

 /* Get georeferencing info */
 getcorners (pcm, pgr, tiepoints);

 /* Write georeferencing info to output file */
 if (!kmlflg) {
   if (pgr->ilinr && pgr->jlinr) {
     /* If the grid is linear in both dimensions, write out the ModelPixelScale and 1 tiepoint */
     pixelscale[0] = *(pgr->ivals+0);
     pixelscale[1] = *(pgr->jvals+0);
     pixelscale[2] = 0.0;
     if (TIFFSetField(tif, TIFFTAG_GEOPIXELSCALE, 3, pixelscale)!=1) { 
       gaprnt(0,"Error: TIFFSetField failed for geopixelscale\n"); goto cleanup; 
     }
     /* write out one tie point */
     if (TIFFSetField(tif, TIFFTAG_GEOTIEPOINTS, 6, tiepoints)!=1) { 
       gaprnt(0,"Error: TIFFSetField failed for geotiepoint\n"); goto cleanup; 
     }
   }
   else {
     /* write out four tie points */
     if (TIFFSetField(tif, TIFFTAG_GEOTIEPOINTS, 24, tiepoints)!=1) { 
       gaprnt(0,"Error: TIFFSetField failed for tiepoints\n"); goto cleanup; 
     }
   }
   
   /* set and write the GeoKeys */
   GTIFKeySet(gtif, GTModelTypeGeoKey,    TYPE_SHORT, 1, ModelGeographic);
   GTIFKeySet(gtif, GTRasterTypeGeoKey,   TYPE_SHORT, 1, RasterPixelIsArea);
   GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, GCS_WGS_84);
   GTIFWriteKeys(gtif);
 }

 /* For KML image output: 
    get contour info, write the color map, and create the KML text file */
 if (kmlflg) {   
   if (!pcm->cflag) {
     /* Determine contour interval */
     gacsel (pgr->rmin,pgr->rmax,&(pcm->cint),&cmin,&cmax);
     cint = pcm->cint;
     /* reject constant fields for now */
     if (cint==0.0) {
       gaprnt (0,"Error: Grid is a constant \n");
       goto cleanup;
     }
     /* make sure there aren't too many levels */
     pmin = cmin;
     if (pmin<pcm->cmin) pmin = pcm->cmin;
     pmax = cmax;
     if (pmax>pcm->cmax) pmax = pcm->cmax;
     if ((pmax-pmin)/cint>100.0) {
       while ((pmax-pmin)/cint>100.0) cint*=10.0;
       pcm->cint = cint;
       gacsel (pgr->rmin,pgr->rmax,&cint,&cmin,&cmax);
     }
   }
   if (pcm->ccolor>=0) gxcolr(pcm->ccolor);
   if (pcm->ccolor<0 && pcm->rainmn==0.0 && pcm->rainmx==0.0 && !pcm->cflag) {
     pcm->rainmn = cmin;
     pcm->rainmx = cmax;
   }
   gaselc (pcm,pgr->rmin,pgr->rmax);
   
   /* create and write out the color map */
   /* palette-color image in TIFF cannot have more than 256 colors */
   CMAX = 256;
   colormap = (uint16*)_TIFFmalloc(3 * CMAX * sizeof (uint16));
   if (colormap==NULL) {
     gaprnt(0,"Error: TIFFmalloc failed for colormap\n"); goto cleanup;
   }
   cm=colormap;
   for (j=0;j<CMAX;j++){
     /* get rgb values for each color */
     gxdbqcol(j, &dbq);
     r = dbq.red;
     g = dbq.green;
     b = dbq.blue;
     *(cm+0*CMAX+j) = (uint16)r;
     *(cm+1*CMAX+j) = (uint16)g;
     *(cm+2*CMAX+j) = (uint16)b;
   }
   if (TIFFSetField(tif, TIFFTAG_COLORMAP, colormap, colormap+CMAX, colormap+(2*CMAX))!=1) { 
     gaprnt(0,"Error: TIFFSetField failed for colormap\n"); goto cleanup; 
   }

   /* Create the KML text file */
   if ((write_kml(pcm,tiepoints))!=0) goto cleanup;
 }

 /* convert the data to appropriate format */
 if (kmlflg) {
   /* color index geotiff (for KML output) */
   cbuf = (unsigned char *)_TIFFmalloc(grsize * depth);
   if (cbuf==NULL) {
     gaprnt(0,"Error: TIFFmalloc failed for color index data buffer\n"); goto cleanup;
   }
   cbuf0 = cbuf;
   for (i=0; i<grsize; i++) {
     if (gru[i]!=0) {
       color = gashdc (pcm, gr[i]);     /* get the relevent color for grid data value */
     }
     else {                             
       color = gxdbkq();                /* use the device background for undefined values */
     }
     cbuf[i] = (unsigned char)color;
   }
 }
 else {
   if (pcm->gtifflg==1) { 
     /* floating point geotiff */
     fbuf = (gafloat*)_TIFFmalloc(grsize * depth);
     if (fbuf==NULL) {
       gaprnt(0,"Error: TIFFmalloc failed for floating point data buffer\n"); goto cleanup;
     }
     fbuf0 = fbuf;
     for (i=0; i<grsize; i++) {
       if (gru[i] != 0) 
	 fval = (gafloat)gr[i];	         /* convert data value to float */
       else 
	 fval = (gafloat)pcm->undef;	 /* convert output undef value to float */
       fbuf[i] = fval;
     }

   } else {               
     /* double precision geotiff */
     dbuf = (gadouble*)_TIFFmalloc(grsize * depth);
     if (dbuf==NULL) {
       gaprnt(0,"Error: TIFFmalloc failed for double precision data buffer\n"); goto cleanup;
     }
     dbuf0 = dbuf;
     for (i=0; i<grsize; i++) {
       if (gru[i] != 0) 
	 dval = gr[i];	         /* use data value as is */
       else 
	 dval = pcm->undef;	 /* use output undef value */
       dbuf[i] = dval;
     }
   }
 }

 /* write the data buffer in strips (one row per strip) */
 for (j=0; j<pgr->jsiz; j++) {
   /* i points to the beginning of the correct row in the grid */
   i = (grsize - (j+1)*pgr->isiz); 
   if (kmlflg) {
     rc = TIFFWriteScanline(tif, cbuf0+i, j, 0);
   } else {
     if (pcm->gtifflg==1) 
       rc = TIFFWriteScanline(tif, fbuf0+i, j, 0);
     else 
       rc = TIFFWriteScanline(tif, dbuf0+i, j, 0);
   }
   if (rc!=1) {
     snprintf(pout,1255,"Error: TIFFWriteScanline failed at row %d\n",j);
     gaprnt(0,pout); goto cleanup; 
   }
 }

 if (kmlflg) {
   gagsav (24,pcm,NULL);
   if (pcm->tifname)
     snprintf(pout,1255,"Created TIFF image file %s\n",pcm->tifname);
   else
     snprintf(pout,1255,"Created TIFF image file grads.tif\n");
   gaprnt (2,pout);
   if (pcm->kmlname)
     snprintf(pout,1255,"  and complementary KML file %s\n",pcm->kmlname);
   else
     snprintf(pout,1255,"  and complementary KML file grads.kml\n");
   gaprnt (2,pout);
 }
 else {
   gagsav (23,pcm,NULL);
   if (pcm->gtifname)
     snprintf(pout,1255,"Created GeoTIFF file %s\n",pcm->gtifname);
   else
     snprintf(pout,1255,"Created GeoTIFF file gradsgeo.tif\n");
   gaprnt (2,pout);
 }
 cleanup:
 if (colormap) _TIFFfree(colormap);
 if (cbuf) { cbuf = cbuf0; _TIFFfree(cbuf); }
 if (fbuf) { fbuf = fbuf0; _TIFFfree(fbuf); }
 if (dbuf) { dbuf = dbuf0; _TIFFfree(dbuf); }
 if (gtif) GTIFFree(gtif);
 if (tif) TIFFClose(tif);
 return;
 
#else 
 if (kmlflg) {
   gaprnt(0,"Error: Creating TIFF images for KML output is not supported in this build. \n");
   gaprnt(0,"  Try the \'-line\' option with \'set kml\' to output contour lines in KML format instead.\n");
 }
 else
   gaprnt(0,"Error: Creating GeoTIFF files is not supported in this build\n");
#endif
 
}

/* This routine gets the georeferencing information for the four corners of the grid */
void getcorners(struct gacmn *pcm, struct gagrid *pgr, gadouble *tiepoints) {

  /* For GeoTIFF, the raster space is treated as PixelIsArea. 
     
  (0,0)
  +-----+-----+-> I
  |     |     |        * denotes the center of the grid cell (GrADS uses this)
  |  *  |  *  |        + denotes i,j points in standard TIFF PixelIsArea raster space
  |   (1,1)   |
  +-----+-----+   
  |     |     |
  |  *  |  *  |
  |     |   (2,2) 
  |-----+-----+
  V
  J        
  
  i,j raster values correspond to the corners of the grid cells instead of the centers  */
  
  /* geotiff raster value (0,0) gets upper left corner lat,lon
     this is the max j index, since in GrADS j goes south->north  */
  ij2ll (pcm, 0, 0, pgr->dimmin[0]-0.5, pgr->dimmax[1]+0.5, tiepoints, 0);
  
  /* geotiff raster value (0,jsize) gets lower left corner lat,lon 
     this is the min j index, since in GrADS j goes south->north */
  ij2ll (pcm, 0, (gadouble)pgr->jsiz, pgr->dimmin[0]-0.5, pgr->dimmin[1]-0.5, tiepoints, 6);
  
  /* geotiff raster value (isize,0) gets upper right corner lat,lon */
  ij2ll (pcm, (gadouble)pgr->isiz, 0, pgr->dimmax[0]+0.5, pgr->dimmax[1]+0.5, tiepoints, 12);
  
  /* geotiff raster value (isize,jsize) gets lower right corner lat,lon */
  ij2ll (pcm, (gadouble)pgr->isiz, (gadouble)pgr->jsiz, pgr->dimmax[0]+0.5, pgr->dimmin[1]-0.5, tiepoints, 18);
}

/* given a grid i,j, calculate the corresponding lat/lon, populate the tiepoints array */
void ij2ll (struct gacmn *pcm, gadouble i, gadouble j, gadouble gx, gadouble gy, 
            gadouble *tiepoints, gaint index) {
gadouble (*conv) (gadouble *, gadouble);
gadouble lon,lat;  

  conv = pcm->xgr2ab;
  lon = conv(pcm->xgrval, gx);
  conv = pcm->ygr2ab;
  lat = conv(pcm->ygrval, gy);
  tiepoints[index+0] = i;
  tiepoints[index+1] = j;
  tiepoints[index+2] = 0.0;
  tiepoints[index+3] = lon;
  tiepoints[index+4] = lat;
  tiepoints[index+5] = 0.0;
}

gaint write_kml(struct gacmn *pcm, gadouble *tpts) {
 FILE *kmlfp=NULL;
 struct gagrid *pgr;
 gaint err;

 /* open the file */
 if (pcm->kmlname)
   kmlfp = fopen (pcm->kmlname,"wb");
 else
   kmlfp = fopen ("grads.kml","wb");
 if (kmlfp==NULL) {
   if (pcm->kmlname)
     snprintf(pout,1255,"Error: fopen failed for KML text output file %s\n",pcm->kmlname);
   else
     snprintf(pout,1255,"Error: fopen failed for KML text output file grads.kml\n");
   gaprnt(0,pout);
   return(1);
 }
 pgr = pcm->result[0].pgr;

 /* write out the KML text */
 err=0;
 snprintf(pout,1255,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 snprintf(pout,1255,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 snprintf(pout,1255,"  <GroundOverlay>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 snprintf(pout,1255,"    <name>%s</name>\n",pgr->pvar->varnm);
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 snprintf(pout,1255,"    <Icon>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 if (pcm->tifname)
   snprintf(pout,1255,"      <href>%s</href>\n",pcm->tifname);
 else
   snprintf(pout,1255,"      <href>grads.tif</href>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 snprintf(pout,1255,"    </Icon>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 snprintf(pout,1255,"    <LatLonBox>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 if ((tpts[0+3]==tpts[6+3]) && (tpts[12+3]==tpts[18+3])) {
   if (tpts[0+3]<tpts[12+3])
     snprintf(pout,1255,"      <west>%10.5g</west>\n      <east>%10.5g</east>\n",tpts[0+3],tpts[12+3]);
   else
     snprintf(pout,1255,"      <west>%10.5g</west>\n      <east>%10.5g</east>\n",tpts[12+3],tpts[0+3]);
   if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 }
 if ((tpts[0+4]==tpts[12+4]) && (tpts[6+4]==tpts[18+4])) {
   if (tpts[0+4]<tpts[12+4])
     snprintf(pout,1255,"      <south>%10.5g</south>\n      <north>%10.5g</north>\n",tpts[0+4],tpts[6+4]);
   else
     snprintf(pout,1255,"      <south>%10.5g</south>\n      <north>%10.5g</north>\n",tpts[6+4],tpts[0+4]);
   if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 }
 snprintf(pout,1255,"      <rotation>0.0</rotation>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 snprintf(pout,1255,"    </LatLonBox>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 snprintf(pout,1255,"  </GroundOverlay>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}
 snprintf(pout,1255,"</kml>\n");
 if ((fwrite(pout,sizeof(char),strlen(pout),kmlfp))!=strlen(pout)) {err=1; goto cleanup;}

 cleanup:
 /* close the file */
 fclose(kmlfp); 
 if (err) gaprnt(0,"Error from fwrite when writing KML file\n");
 return(err);
}


/* Write grid to a file.  */

void gafwrt (struct gacmn *pcm) {
struct gagrid *pgr;
struct stat s;
gaint size, exsz, rdw, ret, diff, fexists;
gaint i, row, goff, xoff, yoff, xsiz, ysiz, incr;
size_t write,written,sz;
char fmode[3],*gru; 
gafloat *fval,*fval0;
gadouble *gr;

  xoff = yoff = xsiz = ysiz = 0;
  pgr = pcm->result[0].pgr;
  size = pgr->isiz * pgr->jsiz;

  /* Special case: output to stdout */
  if (pcm->fwname) {
    if ((strcmp(pcm->fwname,"-")==0) && (pcm->ffile==NULL)) {
      pcm->ffile = stdout;
      printf("\n<FWRITE>\n");
    }
  }

  if (pcm->ffile == NULL) {
    /* use the stat command to check if the file exists */
    if (pcm->fwname) {
      if (stat (pcm->fwname,   &s) == 0) fexists=1; 
      else                               fexists=0; 
    } else {
      if (stat ("grads.fwrite",&s) == 0) fexists=1; 
      else                               fexists=0; 
    }

    /* set the write mode */
    if (pcm->fwappend) {
      strcpy(fmode,"ab");
      if (fexists) {
	snprintf(pout,1255,"Appending data to file %s.\n",pcm->fwname);
	gaprnt (2,pout);
      }
    } else {
      strcpy(fmode,"wb");
      if (fexists) {
	snprintf(pout,1255,"Replacing file %s.\n",pcm->fwname);
	gaprnt (2,pout);
      }
    }

    if (pcm->fwname) pcm->ffile = fopen(pcm->fwname,fmode);
    else pcm->ffile = fopen ("grads.fwrite",fmode);
    if (pcm->ffile==NULL) {
      gaprnt (0,"Error opening output file for fwrite\n");
      if (pcm->fwname) {
        gaprnt (0,"  File name is: ");
        gaprnt (0,pcm->fwname);
        gaprnt (0,"\n");
      } else {
        gaprnt (0,"  File name is: grads.fwrite\n");
      }
      return;
    }
  }

  /* convert the grid values to floats/undefs */
  gr=pgr->grid;
  gru=pgr->umask;
  fval = NULL;
  sz = sizeof(gafloat)*size;
  fval = (gafloat *)galloc(sz,"fwrite1");
  fval0 = fval;
  if (fval==NULL) {
    gaprnt(0,"Error allocating memory for fwrite\n");
    return;
  }
  for (i=0; i<size; i++) {
    if (*gru!=0) {
      *fval=(gafloat)*gr;
    }
    else {
      *fval=(gafloat)pcm->undef;
    }
    gr++; gru++; fval++;
  }
  fval = fval0;


  /* swap if needed.  assumes 4 byte values */  
  rdw = size*4; 
  if (BYTEORDER != pcm->fwenflg) { 
    gabswp(fval,size);
    gabswp(&rdw,1);
  }

  /* Handle -ex flag -- try to use exact grid coords */
  written = 0;
  exsz = size;
  diff = 0;
 /* only X or Y can vary  for new fwrite code */
  if (pcm->fwexflg && pgr->idim<2 && pgr->jdim<2) {  
    if (pcm->xexflg) {
      if (pcm->x1ex != pgr->dimmin[0]) diff=1;
      if (pcm->x2ex != pgr->dimmax[0]) diff=1;
    }
    if (pcm->yexflg) {
      if (pcm->y1ex != pgr->dimmin[1]) diff=1;
      if (pcm->y2ex != pgr->dimmax[1]) diff=1;
    }
  }

  if (diff) {
    if (pgr->idim==0) {     /* x is varying */
      if (pcm->xexflg) {
        xoff = pcm->x1ex - pgr->dimmin[0];  
        xsiz = 1 + pcm->x2ex - pcm->x1ex;
      } else {
        xoff = 0;
        xsiz = pgr->isiz; 
      }
      if (pgr->jdim==1 && pcm->yexflg) {  /* both x and y vary */
        yoff = pcm->y1ex - pgr->dimmin[1];  
        ysiz = 1 + pcm->y2ex - pcm->y1ex;
      } else {
        yoff = 0;
        ysiz = pgr->jsiz; 
      }
    }
    incr = pgr->isiz;
    if (pgr->idim==1) {   /* x is fixed; y is varying */
      if (pcm->yexflg) {
        yoff = pcm->y1ex - pgr->dimmin[1];  
        ysiz = 1 + pcm->y2ex - pcm->y1ex;
      } else {
        yoff = 0;
        ysiz = pgr->isiz; 
      }
      xoff = 0; xsiz = 1;
      incr = 1;
    }
    exsz = xsiz * ysiz;
    rdw = exsz*4; 
    /* Swap the record header if necessary. fix by LIS @ NASA, 3/8/2004 ***/
    if (BYTEORDER != pcm->fwenflg) gabswp(&rdw,1);
    if (pcm->fwsqflg) fwrite(&rdw,sizeof(gaint),1,pcm->ffile);
    if (pgr->idim==1) {
      goff = yoff;
    } else {
      goff = xoff + yoff*pgr->isiz;
    }
    for (row=0; row<ysiz; row++) {
      write = fwrite (fval+goff, sizeof(gafloat), xsiz, pcm->ffile);
      ret = ferror(pcm->ffile);
      if (ret || (write != xsiz)) {
        snprintf(pout,1255,"Error writing data for fwrite: %s\n", strerror(errno) ); 
        gaprnt(0, pout);
      }
      written = written + write;
      goff = goff + incr;
    }
    if (pcm->fwsqflg) fwrite(&rdw,sizeof(gaint),1,pcm->ffile);
  } 
  else {
    if (pcm->fwsqflg) fwrite(&rdw,sizeof(gaint),1,pcm->ffile);
    written = fwrite (fval, sizeof(gafloat), size, pcm->ffile);
    ret = ferror(pcm->ffile);
    if (ret || (written != size)) {
      snprintf(pout,1255,"Error writing data for fwrite: %s\n", strerror(errno) ); 
      gaprnt(0, pout);
    }
    if (pcm->fwsqflg) fwrite(&rdw,sizeof(gaint),1,pcm->ffile);
  }

  if (pcm->ffile != stdout) {
    if (pcm->fwname) {
      snprintf(pout,1255,"Wrote %ld of %i elements to ", written, exsz);
      gaprnt (2,pout);
      gaprnt (2,pcm->fwname);
    } else {
      snprintf(pout,1255,"Wrote %ld of %i elements to grads.fwrite", written, exsz);
      gaprnt (2,pout);
    }
  
    if (pcm->fwsqflg) gaprnt(2," as Sequential");
    else gaprnt(2," as Stream");
    if (pcm->fwenflg) gaprnt(2," Big_Endian\n");
    else gaprnt(2," Little_Endian\n");
  }

  gagsav (20,pcm,NULL);
  /* free the array of copied floats/undefs */
  if (fval!=NULL) {
    fval = fval0;
    gree(fval,"f333");
    fval0=NULL;
  }
}

/* Write stations to file.  This is a hack.  Assumes x/y varying.  Does not 
   handle levels.  Does not write sequential.  Writes only to grads.stnwrt.  
   Writes one variable only.  Writes that variable as level-independent. 
   Does no error checking.  For each time called, writes out a time delimeter
   header.  There are probably more limitations I am not thinking of.
   Close the file with disable stnwrt */

void gastnwrt (struct gacmn *pcm) {
struct gastn *stn;
struct garpt *rpt;
struct rpthdr hdr;
gafloat val;
int i;

  if (pcm->sfile==NULL) {
    pcm->sfile = fopen ("grads.stnwrt","wb");
    if (pcm->sfile==NULL) {
      gaprnt (0,"Error opening output file for stnwrt\n");
      gaprnt (0,"  File name is: grads.stnwrt\n");
      return;
    }
  }
  stn = pcm->result[0].stn;
  rpt = stn->rpt;
  while (rpt) {
    if (rpt->umask!=0) {
      for (i=0; i<8; i++) hdr.id[i] = rpt->stid[i];
      hdr.lon = rpt->lon;  hdr.lat = rpt->lat;
      hdr.t = 0.0; hdr.nlev = 1;  hdr.flag = 1;
      fwrite (&(hdr), sizeof(struct rpthdr), 1, pcm->sfile);
      val = rpt->val;
      fwrite (&(val), sizeof(float), 1, pcm->sfile);
    }
    rpt = rpt->rpt;
  }
  hdr.nlev = 0;
  fwrite (&(hdr), sizeof(struct rpthdr), 1, pcm->sfile);
}

/* Do 2-D grid fill plots */

void gafgrd (struct gacmn *pcm) {
struct gagrid *pgr;
gaint i,j,k,iv,col,scol,isav,ii,siz;
gadouble *xybuf,*xy,*r;
char *rmask;
size_t sz;

  pgr = pcm->result[0].pgr;

  if ((pcm->rotate && (pgr->idim!=2 || pgr->jdim!=3)) ||
      (!pcm->rotate && pgr->idim==2 && pgr->jdim==3)) 
    pgr = gaflip(pgr,pcm);

  gas2d (pcm, pgr, 0);     /* Set up scaling */
  idiv = 1.0; jdiv = 1.0;
  gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);

  /* Allocate point buffer */

  siz = (pgr->isiz+2)*4;
  sz = sizeof(gadouble)*siz;
  xybuf = (gadouble *)galloc(sz,"xybuf");
  if (xybuf==NULL) {
    gaprnt(0,"Memory allocation error in FGRID\n");
    return;
  }
  *(xybuf+siz-1) = -1;

  /* Fill grid "boxes" */

  r = pgr->grid;
  rmask = pgr->umask;
  for (j=1; j<=pgr->jsiz; j++) {
    col = -1; scol = -1;
    isav = 1;
    for (i=1; i<=pgr->isiz; i++) {
      col = -1;
      if (*rmask != 0) {
        iv = floor(*r+0.5);   /* check with bdoty */
        for (k=0; k<pcm->fgcnt; k++) {
          if (iv==pcm->fgvals[k]) col = pcm->fgcols[k];
        }
      }
      if (col!=scol) {
        if (scol>-1) {
          xy = xybuf;
          for (ii=isav; ii<=i; ii++) {
            gxconv ((gadouble)(ii)-0.5,(gadouble)(j)+0.5,xy,xy+1,3);
            xy+=2;
          }
          for (ii=i; ii>=isav; ii--) {
            gxconv ((gadouble)(ii)-0.5,(gadouble)(j)-0.5,xy,xy+1,3);
            xy+=2;
          }
          *xy = *xybuf;  
	  *(xy+1) = *(xybuf+1);
          gxcolr(scol);
          gxfill (xybuf,(1+i-isav)*2+1);
        }
        isav = i;
        scol = col;
      }
      r++; rmask++;
    }
    if (scol>-1) {
      xy = xybuf;
      for (ii=isav; ii<=pgr->isiz+1; ii++) {
        gxconv ((gadouble)(ii)-0.5,(gadouble)(j)+0.5,xy,xy+1,3);
        xy+=2;
      }
      for (ii=pgr->isiz+1; ii>=isav; ii--) {
        gxconv ((gadouble)(ii)-0.5,(gadouble)(j)-0.5,xy,xy+1,3);
        xy+=2;
      }
      *xy = *xybuf;  *(xy+1) = *(xybuf+1);
      gxcolr(scol);
      gxfill (xybuf,(2+pgr->isiz-isav)*2+1);
    }
  }
  if (*(xybuf+siz-1) != -1) {
    gaprnt (0,"Logic Error 16 in gafgrd.  Please report error.\n");
  }
  gree(xybuf,"f290");
  if (pgr->idim==0 && pgr->jdim==1) gawmap (pcm, 0);
  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  gxcolr(pcm->anncol);
  gxwide(4);
  if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
  if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
  gaaxpl(pcm,pgr->idim,pgr->jdim);
  gafram (pcm);
  gagsav (5,pcm,pgr);
}

/* Do 2-D contour plots */
/* filflg:  
   0  line contours
   1  shaded contours
   2  grfill
   3  imap
   4,5  gxshad2

   shpflg:
   0  no shapefile
   1  line or shaded contours for shapefile output 

   return code ignored in all cases except when shpflg=1, in which case
   rc=0 contours created
   rc=1 no contours

*/

gaint gacntr (struct gacmn *pcm, gaint filflg, gaint shpflg) {
struct gagrid *pgr;
gadouble cmin,cmax,cint;
gadouble rl,rr,rrb,rmin,rmax, *rrr=0,*r;
gadouble pmin,pmax,tt;
gaint smooth, irb, clcnt, clflg, clopt;
gaint i,iexp,jexp,isz=0,jsz=0,ii,ii2=0,isav,cnt;
gaint cntrcnt,cntrcol;
char chlab[50],*rmask,*rrrmask=NULL,umin=0;
struct gxcntr scntr;
size_t sz;

  pgr = pcm->result[0].pgr;

  /* If gxout imap, check for validity.  Must by x/y plot and all must be linear. */
  if (filflg==3 && 
     (pcm->mproj<1 || pcm->mproj>2 || pgr->idim != 0 || pgr->jdim != 1) ) {
    gaprnt (0,"Invalid dimension and/or scaling environment for gxout imap\n");
    gaprnt (0,"   Mproj latlon or scaled required; x/y varying plot required\n");
    return 1;
  }

  /* check if user-provided contour levels are strictly increasing for shaded output */
  if (filflg && pcm->cflag) {
    for (i=1; i<pcm->cflag; i++) {
      if (pcm->clevs[i] <= pcm->clevs[i-1]) {
	gaprnt(2,"Invalid user-specified contour levels for shaded output.\n");
	gaprnt(2,"   Contour levels must be strictly increasing.\n");
	return 1;
      }
    }
  }

  /* flip axes for Z-T plots (skip this for shapefile output )*/
  if (shpflg==0) {
    if ( (pcm->rotate && (pgr->idim!=2 || pgr->jdim!=3)) ||
	 (!pcm->rotate && pgr->idim==2 && pgr->jdim==3)) pgr = gaflip(pgr,pcm);
    
    gxstyl (1);
    gxwide (1);
    if (filflg) gas2d (pcm, pgr, 0);   /* No map yet if shaded cntrs */
    else gas2d (pcm, pgr, 1);          /* Scaling plus map */
  }

  /* Determine data min/max */
  gamnmx (pgr);
  if (pgr->umin==0) {
    gaprnt (1,"Cannot contour grid - all undefined values \n");
    gxcolr(1);
    if (pcm->dwrnflg) gxchpl ("Entire Grid Undefined",21,3.0,4.5,0.3,0.25,0.0);
    return 1;
  }

  /* Determine contour interval */
  if (!pcm->cflag) {
    gacsel (pgr->rmin,pgr->rmax,&(pcm->cint),&cmin,&cmax);
    cint = pcm->cint;
    if (cint==0.0) {  /* field is a constant */
      if (pcm->dwrnflg && shpflg==0) {
	/* use fgrid and red to display the grid, print message to user */
        isav=pcm->gout2a;
        pcm->fgvals[0]=pgr->rmin;
	if (pcm->ccolor>0)
	  pcm->fgcols[0]=pcm->ccolor;
	else
	  pcm->fgcols[0]=2;
        pcm->gout2a = 6;
        pcm->fgcnt = 1;
        gaplot (pcm);
        pcm->gout2a = isav;
        pcm->fgcnt = 0;
        snprintf(pout,1255,"Constant field.  Value = %g\n",pgr->rmin);
        gaprnt (1,pout);
      } else {
	/* just print the message */
        snprintf(pout,1255,"Constant field.  Value = %g\n",pgr->rmin);
        gaprnt (1,pout);
      }
      return 1;
    }
    /* make sure there aren't too many levels */
    pmin = cmin;
    if (pmin<pcm->cmin) pmin = pcm->cmin;
    pmax = cmax;
    if (pmax>pcm->cmax) pmax = pcm->cmax;
    if ((pmax-pmin)/cint>100.0) {
      gaprnt (0,"Too many contour levels -- adjusting cint\n");
      while ((pmax-pmin)/cint>100.0) cint*=10.0;
      pcm->cint = cint;
      gacsel (pgr->rmin,pgr->rmax,&cint,&cmin,&cmax);
    }
  }

  /* set contour color */
  if (pcm->ccolor>=0) gxcolr(pcm->ccolor);
  if (pcm->ccolor<0 && pcm->rainmn==0.0 && pcm->rainmx==0.0 && !pcm->cflag) {
    pcm->rainmn = cmin;
    pcm->rainmx = cmax;
  }

  /* Expand grid if smoothing was requested */
  idiv = 1.0; jdiv = 1.0;
  smooth = 0;
  rmin = pgr->rmin; rmax = pgr->rmax;
  if (pcm->csmth && (pgr->isiz<51 || pgr->jsiz<51)) {
    smooth = 1;
    iexp = 100 / pgr->isiz;
    jexp = 100 / pgr->jsiz;
    if (iexp>5) iexp = 4;
    if (jexp>5) jexp = 4;
    if (iexp<1) iexp = 1;
    if (jexp<1) jexp = 1;
    isz = ((pgr->isiz-1)*iexp) + 1;
    jsz = ((pgr->jsiz-1)*jexp) + 1;
    sz = isz*jsz*sizeof(gadouble);
    rrr = (gadouble *)galloc(sz,"rrr");
    if (rrr==NULL) {
      gaprnt (0,"Memory Allocation Error:  CSMOOTH operation\n");
      return 1;
    }
    sz = isz*jsz*sizeof(char);
    rrrmask = (char *)galloc(sz,"rrrmask");
    if (rrrmask==NULL) {
      gaprnt (0,"Memory Allocation Error:  CSMOOTH operation\n");
      gree(rrr,"f291b");
      return 1;
    }
    idiv = (gadouble)iexp;
    jdiv = (gadouble)jexp;
    if (pcm->csmth==1) {
      gagexp (pgr->grid, pgr->isiz, pgr->jsiz, rrr, iexp, jexp, pgr->umask, rrrmask);
    } else {
      gaglin (pgr->grid, pgr->isiz, pgr->jsiz, rrr, iexp, jexp, pgr->umask, rrrmask);
    }

    /* When clevs are set, gxshad2 needs to know the new rmax */
    if (pcm->cflag && (filflg==4 || filflg==5)) {
      r = rrr;
      rmask = rrrmask;
      for (i=0;i<isz*jsz;i++) {
        if (*rmask != 0) {
          if (rmax<*r) rmax = *r;
        }
        r++; rmask++;
      }
    }

    /* We may have created new contour levels.  Adjust cmin and cmax appropriately */
    if (!pcm->cflag) {
      rmin = 9.99e8;
      rmax = -9.99e8;
      r = rrr;
      rmask = rrrmask;
      cnt=0;
      for (i=0;i<isz*jsz;i++) {
        if (*rmask != 0) {
          cnt++;
          if (rmin>*r) rmin = *r; 
          if (rmax<*r) rmax = *r;
        }
        r++; rmask++;
      }
      if (cnt==0 || dequal(rmin,9.99e8,1e-8)==0 || dequal(rmax,-9.99e8,1e-8)==0) 
	umin = 0;
      else 
	umin = 1;

      if (umin==0) {
        gaprnt (1,"Cannot contour grid - all undefined values \n");
        if (pcm->dwrnflg) gxchpl ("Entire Grid Undefined",21,3.0,4.5,0.3,0.25,0.0);
        gree(rrr,"f291");
        gree(rrrmask,"f291a");
        return 1;
      }
      while (rmin<cmin) cmin -= cint;
      while (rmax>cmax) cmax += cint;
    }
  }
  if (pcm->cflag) {
    gaprnt (2,"Contouring at clevs = ");
    for (i=0; i<pcm->cflag; i++) {
      snprintf(pout,1255," %g",pcm->clevs[i]);
      gaprnt (2,pout);
    }
    gaprnt (2,"\n");
  } else {
    snprintf(pout,1255,"Contouring: %g to %g interval %g \n",cmin,cmax,cint);
    gaprnt (2,pout);
  }

  gxclip (pcm->xsiz1,pcm->xsiz2,pcm->ysiz1,pcm->ysiz2);
  if (pcm->rbflg) irb = pcm->rbflg - 1;
  else irb = 12;
  rrb = irb+1;
  if (filflg) {
    gaselc (pcm,pgr->rmin,pgr->rmax);
    if (smooth) {
      if (filflg==1) {
	gxshad (rrr,isz,jsz,pcm->shdlvs,pcm->shdcls,pcm->shdcnt,rrrmask);
      } else if (filflg==2) {
	gagfil (rrr,isz,jsz,pcm->shdlvs,pcm->shdcls,pcm->shdcnt,rrrmask);
      } else if (filflg==3) {
	gaimap (rrr,isz,jsz,pcm->shdlvs,pcm->shdcls,pcm->shdcnt,rrrmask,
                    pcm->xsiz1,pcm->xsiz2,pcm->ysiz1,pcm->ysiz2);
      } else if (filflg==4) {
        gxshad2 (rrr,isz,jsz,pcm->shdlvs,rmax,pcm->shdcls,pcm->shdcnt,rrrmask);
      } else if (filflg==5) {
        gxshad2b (rrr,isz,jsz,pcm->shdlvs,rmax,pcm->shdcls,pcm->shdcnt,rrrmask);
      }
    } 
    else {
      if (filflg==1) {
        gxshad (pgr->grid,pgr->isiz,pgr->jsiz,pcm->shdlvs,pcm->shdcls,pcm->shdcnt,pgr->umask);
      } else if (filflg==2) {
        gagfil (pgr->grid,pgr->isiz,pgr->jsiz,pcm->shdlvs,pcm->shdcls,pcm->shdcnt,pgr->umask);
      } else if (filflg==3) {
        gaimap (pgr->grid,pgr->isiz,pgr->jsiz,pcm->shdlvs,pcm->shdcls,pcm->shdcnt,
             pgr->umask, pcm->xsiz1,pcm->xsiz2,pcm->ysiz1,pcm->ysiz2);
      } else if (filflg==4) {
        gxshad2 (pgr->grid,pgr->isiz,pgr->jsiz,pcm->shdlvs,pgr->rmax,pcm->shdcls,pcm->shdcnt,pgr->umask);
      } else if (filflg==5) {
        gxshad2b (pgr->grid,pgr->isiz,pgr->jsiz,pcm->shdlvs,pgr->rmax,pcm->shdcls,pcm->shdcnt,pgr->umask);
      }
    }
    if (pgr->idim==0 && pgr->jdim==1 && shpflg==0) gawmap (pcm, 0);
  } 
  else {
    gxwide (pcm->cthick);
    cntrcnt = 0;
    cntrcol = -1;
    if (pcm->cflag) {          /* user has specified contour levels */
      for (i=0; i<pcm->cflag; i++) {
        rr = pcm->clevs[i];
        if (rr<0.0&&pcm->cstyle==-9) gxstyl (3);
        else gxstyl(pcm->cstyle);
        if (pcm->ccolor < 0 && pcm->ccflg == 0) {   /* user has not specified contour colors */
          if (pcm->cflag==1) ii=irb/2;              /* only one specified level */
          else ii = (gaint)((gadouble)(i*irb)/((gadouble)(pcm->cflag-1)));
          if (ii>irb) ii=irb;
          if (ii<0) ii=0;
          if (pcm->rbflg>0) {
            if (pcm->ccolor==-1) {
	      gxcolr(pcm->rbcols[ii]);
	      cntrcol = pcm->rbcols[ii];
	    }
            else {
	      gxcolr(pcm->rbcols[irb-ii]);
	      cntrcol = pcm->rbcols[irb-ii];
	    }
          } else {
	    if (pcm->ccolor==-1) {
	      gxcolr(rcols[ii]);
	      cntrcol = rcols[ii];
	    }
            else {
	      gxcolr(rcols[12-ii]);
	      cntrcol = rcols[12-ii];
	    }
          }
        }
        if (pcm->ccflg) {   /* overlays */
          ii = i;
          if (ii>=pcm->ccflg) ii = pcm->ccflg-1;
          gxcolr (pcm->ccols[ii]);
	  cntrcol = pcm->ccols[ii];  
        }
	if (pcm->ccolor > 0 && pcm->ccflg == 0) cntrcol = pcm->ccolor;
        if (pcm->clstr) 
	  snprintf(chlab,49,pcm->clstr,rr);
        else 
	  snprintf(chlab,49,"%g",rr);
        chlab[21] = '\0';   /* Limit length of label to 20 chars */
        scntr.label = chlab;       scntr.spline = pcm->cterp;
        scntr.ltype = pcm->clab;   scntr.ccol = gxqclr();
        scntr.labcol = pcm->clcol; scntr.labwid = pcm->clthck;
        scntr.labsiz = pcm->clsiz; scntr.val = rr;
	/* label masking and shpflag must be turned on for shapefiles */
        if (pcm->clab==3 || shpflg==1) {
	  scntr.mask=1; 
	  if (shpflg) scntr.shpflg=1;
	  else scntr.shpflg=0;
	}
	else {
	  scntr.mask=0;
	}
        if (smooth) {
          gxclev (rrr,isz,jsz,1,isz,1,jsz,rr,rrrmask,&scntr);
        } else {
          gxclev (pgr->grid,pgr->isiz,pgr->jsiz,1,pgr->isiz,1,
                  pgr->jsiz,rr,pgr->umask,&scntr);
        }
	pcm->cntrcols[cntrcnt] = cntrcol; 
	pcm->cntrlevs[cntrcnt] = rr;
	cntrcnt++;
      }
      pcm->cntrcnt = cntrcnt; 
    } 
    else {   /* user has not specified contour levels */
      clcnt = 0;
      clopt = 0;  /* normalize clskip only when well-behaved */
      if (fabs(cmin/cint)<1e6 || fabs(cmax/cint)<1e6) clopt=1; 
      for (rl=cmin;rl<=cmax+(cint/2.0);rl+=cint) {
	if (dequal(rl,0.0,1e-15)==0) rl=0.0; /* a quick patch */
        if (rl<pcm->cmin || rl>pcm->cmax) continue;
        if (pcm->blkflg && rl>=pcm->blkmin && rl<=pcm->blkmax) continue;
        rr = rl;
        if (rr<0.0 && pcm->cstyle==-9) gxstyl (3);
        else gxstyl(pcm->cstyle);
        if (pcm->ccolor < 0) {
          ii = (gaint)(rrb*(rr-pcm->rainmn)/(pcm->rainmx-pcm->rainmn));
          if (ii>irb) ii=irb;
          if (ii<0) ii=0;
          if (pcm->rbflg>0) {
            if (pcm->ccolor==-1) {
	      gxcolr(pcm->rbcols[ii]);
	      cntrcol = pcm->rbcols[ii];
	    }
            else {
	      gxcolr(pcm->rbcols[irb-ii]);
	      cntrcol = pcm->rbcols[irb-ii];
	    }
          } else {
            if (pcm->ccolor==-1) {
	      gxcolr(rcols[ii]);
	      cntrcol = rcols[ii];
	    }
            else {
	      gxcolr(rcols[12-ii]);
	      cntrcol = rcols[12-ii];
	    }
          }
        }
	else {
	  cntrcol = pcm->ccolor;
	}
        clflg = 0;
        if (clopt) {
          tt = rl/(cint*(gadouble)pcm->clskip);
          ii = (gaint)(tt+0.009);
          ii2 = (gaint)(tt-0.009);
          if (fabs(tt-(gadouble)ii)<0.01 || fabs(tt-(gadouble)ii2)<0.01) clflg=1;
        } else {
          if (clcnt == pcm->clskip) {
            clflg = 1; clcnt = 0;
          } else clcnt++;
        }
        if (clflg) {
          if (pcm->clstr) {
	    snprintf(chlab,49,pcm->clstr,rr);
	  } else {
	    snprintf(chlab,49,"%g",rr);
	  }
          chlab[21] = '\0';  /* Limit length to 20 chars */
        } else {
          chlab[0] = '\0';
        }
        scntr.label = chlab;       scntr.spline = pcm->cterp;
        scntr.ltype = pcm->clab;   scntr.ccol = gxqclr();
        scntr.labcol = pcm->clcol; scntr.labwid = pcm->clthck;
        scntr.labsiz = pcm->clsiz; scntr.val = rr;
	/* label masking and shpflag must be turned on for shapefiles */
        if (pcm->clab==3 || shpflg==1) {
	  scntr.mask=1; 
	  if (shpflg) scntr.shpflg=1;
	  else scntr.shpflg=0;
	}
	else {
	  scntr.mask=0;
	}
        if (smooth) {
          gxclev (rrr,isz,jsz,1,isz,1,jsz,rr,rrrmask,&scntr);
        } else {
          gxclev (pgr->grid,pgr->isiz,pgr->jsiz,1,pgr->isiz,1,
                  pgr->jsiz,rr,pgr->umask,&scntr);
        }
	pcm->cntrcols[cntrcnt] = cntrcol; 
	pcm->cntrlevs[cntrcnt] = rr;
	cntrcnt++;
      }
      pcm->cntrcnt = cntrcnt;
    }
    if (shpflg==0) {
      if (pcm->clab == 3) {
	gxpclin();
      }
      else {
	if (pcm->clcol>-1) gxcolr (pcm->clcol);
	if (pcm->clthck>-1) gxwide (pcm->clthck);
	gxclab (pcm->clsiz,pcm->clab,pcm->clcol);
      }
      gxcrel ();
    }
  }
  if (smooth) {
    gree(rrr,"f292");
    gree(rrrmask,"f292a");
  }
  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  gxcolr (pcm->anncol);
  gxwide (4); 
  if (shpflg==0) {
    if (pcm->pass==0 && pcm->grdsflg) gxchpl("GrADS/COLA",10,0.05,0.05,0.1,0.09,0.0);
    if (pcm->pass==0 && pcm->timelabflg) gatmlb(pcm);
    gaaxpl(pcm,pgr->idim,pgr->jdim);
    gafram (pcm);
  }
  if (filflg==1) {
    gagsav (2,pcm,pgr);
  }
  else if (filflg==2) {
    gagsav (16,pcm,pgr);
  }
  else if (filflg==3) {
    gagsav (26,pcm,pgr);
  }
  else if (filflg==4) {
    gagsav (27,pcm,pgr);
  }
  else if (filflg==5) {
    gagsav (28,pcm,pgr);
  }
  else {
    if (shpflg==0) gagsav (1,pcm,pgr);
  }
  return 0;
}

/* Routine to perform grid level scaling.  The address of this
   routine is passed to gxgrid.  */

void gaconv (gadouble s, gadouble t, gadouble *x, gadouble *y) {
  s = ((s-1.0)/idiv)+1.0;
  t = ((t-1.0)/jdiv)+1.0;
  if (iconv==NULL) *x = s+ioffset;
  else *x = iconv(ivars, (s+ioffset));
  if (jconv==NULL) *y = t+joffset;
  else *y = jconv(jvars, (t+joffset));
}

/* Draw axis labels.  axis = 1 is x axis, axis = 0 is y axis */

void gaaxis (gaint axis, struct gacmn *pcm, gaint dim) {
struct gafile *pfi;
struct dt tstrt,tincr,addmo,twrk;
gadouble vmin,vmax,*tvals=0,x,y,tt;
gadouble v,vincr,vstrt,vend;
gadouble m,b,cs,xx,cwid,pos;
gaint ii,len,tinc=0,colr,thck,flg,i,cnt;
char lab[30],olab[30],*chlb=NULL;
 
  if (axis && pcm->xlab==0) return;
  if (!axis && pcm->ylab==0) return;
  addmo.yr = 0L;
  addmo.mo = 1L;
  addmo.dy = 0L;
  addmo.hr = 0L;
  addmo.mn = 0L;
  if (axis==1) {
    cs   = pcm->xlsiz; 
    colr = pcm->xlcol;
    thck = pcm->xlthck;
    if (pcm->xlside) pos = pcm->ysiz2 + pcm->xlpos;
    else pos = pcm->ysiz1 + pcm->xlpos;
    if (pcm->xlpos!=0.0) { 
      /* X axis is offset from frame */
      gxcolr (pcm->anncol);
      gxwide (pcm->annthk);
      gxstyl (1);
      gxplot (pcm->xsiz1,pos,3);
      gxplot (pcm->xsiz2,pos,2);
    }
  } else {
    cs = pcm->ylsiz;
    colr = pcm->ylcol;
    thck = pcm->ylthck;
    if (pcm->ylside) pos = pcm->xsiz2 + pcm->ylpos;
    else pos = pcm->xsiz1 + pcm->ylpos;
    if (pcm->ylpos!=0.0) {
      /* Y axis is offset from frame */
      gxcolr (pcm->anncol);
      gxwide (pcm->annthk);
      gxstyl (1);
      gxplot (pos,pcm->ysiz1,3);
      gxplot (pos,pcm->ysiz2,2);
    }
  }

  /* Select axis min and max */
  vincr=0.0;
  if (dim==5) {         /* hard coded 5 means 1D plot  */
    vmin = pcm->rmin; 
    vmax = pcm->rmax;
  } 
  else if (dim==3) {
    pfi = pcm->pfid;
    tvals = pfi->abvals[3];
    vmin = t2gr(tvals,&(pcm->tmin));
    vmax = t2gr(tvals,&(pcm->tmax));
  } else {
    vmin = pcm->dmin[dim];
    vmax = pcm->dmax[dim];
  }
  if (axis && pcm->xlabs) {  /* doesn't allow only one label? */
    vmin = 1.0;
    vmax = (gadouble)pcm->ixlabs;
    vincr = 1.0;
    dim = 5;
  }
  if (!axis && pcm->ylabs) {
    vmin = 1.0;
    vmax = (gadouble)pcm->iylabs;
    vincr = 1.0;
    dim = 5;
  }
  if (axis && pcm->axflg && (dim!=2 || pcm->zlog==0)) {
    vmin = pcm->axmin;
    vmax = pcm->axmax;
    vincr = pcm->axint;
    dim = 5;
    gaprnt (1,"Warning:  X axis labels overridden by SET XAXIS.\n");
    gaprnt (1,"   Labels may not reflect correct scaling for dimensions or data.\n");
  }
  if (!axis && pcm->ayflg && (dim!=2 || pcm->zlog==0)) {
    vmin = pcm->aymin;
    vmax = pcm->aymax;
    vincr = pcm->ayint;
    dim = 5;
    gaprnt (1,"Warning:  Y axis labels overridden by SET YAXIS.\n");
    gaprnt (1,"   Labels may not reflect correct scaling for dimensions or data.\n");
  }
  if (vmin==vmax) {   /* no precision check */
    gaprnt(0,"gaaxis internal logic check 24\n");
    return;
  }

  /* Handle axis flipping */
  if (axis) {
    if (pcm->xflip) {
      m=(pcm->xsiz2-pcm->xsiz1)/(vmin-vmax);
      b=pcm->xsiz1-(m*vmax);
    } else {
      m=(pcm->xsiz2-pcm->xsiz1)/(vmax-vmin);
      b=pcm->xsiz1-(m*vmin);
    }
  } else {
    if (pcm->yflip) {
      m=(pcm->ysiz2-pcm->ysiz1)/(vmin-vmax);
      b=pcm->ysiz1-(m*vmax);
    } else {
      m=(pcm->ysiz2-pcm->ysiz1)/(vmax-vmin);
      b=pcm->ysiz1-(m*vmin);
    }
  }

  /* Select label interval */
  if (vmin>vmax) {
    v = 1.0*vmax;  /* Avoid optimization */
    vmax = vmin;
    vmin = v;
  }
  if (dim==3) {
    tinc = gatinc (pcm, &tstrt, &tincr);
  } else {
    flg = 1;
    if (axis==1 && pcm->xlint!=0.0) {vincr=pcm->xlint; flg=0;}
    if (axis==0 && pcm->ylint!=0.0) {vincr=pcm->ylint; flg=0;}
    if (vincr<0.0) {
      vincr = -1.0 * vincr;
      vstrt = vmin;
      vend = vmax;
      vend = vend+(vincr*0.5);
    } else {
      gacsel (vmin,vmax,&vincr,&vstrt,&vend);
      if (dequal(vincr, 0.0, 1e-08)==0) {
        gaprnt (0,"gaaxis internal logic check 25\n");
        return;
      }
      if (dim==1 && flg) {
        if (vincr>19.9) vincr=30.0;
        else if (vincr>10.0) vincr=10.0;
      }
      if (dim==0 && flg) {
        if (vincr>74.5) vincr=90.0;
        else if (vincr>44.5) vincr=60.0;
        else if (vincr>24.9) vincr=30.0;
        else if (vincr>14.5) vincr=20.0;
        else if (vincr>10.0) vincr=10.0;
      }
      gacsel (vmin,vmax,&vincr,&vstrt,&vend);
      vend = vend+(vincr*0.5);
    }
  }

  /* draw labels, tic marks, and grid lines */
  gxcolr(colr);
  gxwide(thck);
  gxstyl(1);
  if (dim!=3) {
    /* determine the label values */
    if (axis==1 && pcm->xlflg>0) cnt = pcm->xlflg;
    else if (axis==0 && pcm->ylflg>0) cnt = pcm->ylflg;
    else {
      cnt = 1.0 + (vend-vstrt)/vincr;
      if (cnt>50) cnt=50;
      for (i=0; i<cnt; i++) {
        v = vstrt+vincr*(gadouble)i;
        if (fabs(v/vincr)<1e-5) v=0.0;
        if (axis) *(pcm->xlevs+i) = v;
        else *(pcm->ylevs+i) = v;
      }
    }
    i = 0;
    if (axis==1 && pcm->xlabs) chlb = pcm->xlabs;
    if (axis==0 && pcm->ylabs) chlb = pcm->ylabs;
    while (i<cnt) {
      /* convert labels to strings */
      if (axis) v = *(pcm->xlevs+i);
      else v = *(pcm->ylevs+i);
      if (axis==1 && pcm->xlstr) 
        snprintf(lab,29,pcm->xlstr,v);
      else if (axis==0 && pcm->ylstr) 
        snprintf(lab,29,pcm->ylstr,v);
      else if (( axis==1 && pcm->xlabs ) || ( axis==0 && pcm->ylabs)) {
        snprintf(lab,29,chlb,v);
        while (*chlb) chlb++;
        chlb++;
      }
      else {
        if (dim==0 && pcm->mproj>0) len = galnch(v,lab);
        else if (dim==1 && pcm->mproj>0) len = galtch(v,lab);
        else snprintf(lab,29,"%g",v);
      }
      len=0;
      while (lab[len]) len++;
      cwid = (gadouble)len*cs;
      gxchln (lab,len,cs,&cwid);
      if (axis) {
        x = (v*m)+b;
        if (dim==2 && pcm->zlog) gxconv(v,pos,&x,&tt,2);
        else if (dim==1 && pcm->coslat) gxconv(v,pos,&x,&tt,2); 
        else if (pcm->log1d) gxconv(v,pos,&x,&tt,2); 
        if (x<pcm->xsiz1-0.05 || x>pcm->xsiz2+0.05) {
          i++;
          continue;
        }
	/* X tic marks */
        gxplot (x,pos,3);
        if (pcm->xlside) gxplot (x,pos+(cs*0.4),2);
        else gxplot (x,pos-(cs*0.4),2);
	/* X grid lines */
        if (pcm->grflag==1 || pcm->grflag==3) {
          gxwide (pcm->grthck); 
          gxstyl (pcm->grstyl);
          gxcolr (pcm->grcolr);
          gxplot (x,pcm->ysiz1,3);
          gxplot (x,pcm->ysiz2,2);
	  gxcolr(colr);
          gxwide(thck);
          gxstyl(1);
        }
        x = x - cwid*0.4;
        if (pcm->xlside) y = pos + (cs*0.7);
        else y = pos - (cs*1.7);
      } else {
        y = (v*m)+b;
        if (dim==2 && pcm->zlog) gxconv(pcm->xsiz1,v,&tt,&y,2);
        else if (dim==1 && pcm->coslat) gxconv(pcm->xsiz1,v,&tt,&y,2); 
        else if (pcm->log1d) gxconv(pcm->xsiz1,v,&tt,&y,2); 
        if (y<pcm->ysiz1-0.05 || y>pcm->ysiz2+0.05) {
          i++;
          continue;
        }
	/* Y tic marks */
        gxplot (pos,y,3);
        if (pcm->ylside) {
          gxplot (pos+(cs*0.4),y,2);
          x = pos + cs*0.8;
        } else {
          gxplot (pos-(cs*0.4),y,2);
          x = pos - (cwid+cs)*0.8;
          if (pcm->yllow<(cwid+cs)*0.8) pcm->yllow = (cwid+cs)*0.8;
        }
	/* X grid lines */
        if (pcm->grflag==1 || pcm->grflag==2) {
          gxwide (pcm->grthck);
          gxstyl (pcm->grstyl);
          gxcolr (pcm->grcolr);
          gxplot (pcm->xsiz1,y,3);
          gxplot (pcm->xsiz2,y,2);
          gxwide(thck);
          gxcolr(colr);
          gxstyl(1);
        }
        y = y - (cs*0.5);
      }
      gxchpl(lab,len,x,y,cs,cs*0.8,0.0);
      lab[9] = '\0';
      i++;
    }
  } else {

    /*  Do Date/Time labeling  */

    strcpy (olab,"mmmmmmmmmmmmmmmm");
    while (timdif(&tstrt,&(pcm->tmax),0)>-1L) {
      len = gat2ch(&tstrt,tinc,lab,30);
      v = t2gr(tvals,&tstrt);
      if (axis) {
        x = (v*m)+b;
        gxplot (x,pos,3);
        if (pcm->xlside) gxplot (x,pos+(cs*0.4),2);
        else gxplot (x,pos-(cs*0.4),2);
        if (pcm->grflag==1 || pcm->grflag==3) {
          gxwide (pcm->grthck);
          gxstyl (pcm->grstyl);
          gxcolr (pcm->grcolr);
          gxplot (x,pcm->ysiz1,3);
          gxplot (x,pcm->ysiz2,2);
          gxwide(thck);
          gxcolr(colr);
          gxstyl(1);
        }
        if (pcm->xlside) y = pos + (cs*0.7);
        else y = pos - (cs*1.7);
        ii = 0;
        if (tinc>3) {
          if (tinc==5) len = 6;
          if (tinc==4) len = 3;
          if (cmpch(&(lab[ii]),&(olab[ii]),len)) {
            cwid = len*cs;
            gxchln(&lab[ii],len,cs,&cwid);
            xx = x - cwid*0.4;
            gxchpl(&lab[ii],len,xx,y,cs,cs*0.8,0.0);
          }
          if (pcm->xlside) y = y + cs*1.4;
          else y = y - cs*1.4;
          ii = len;
        }
        if (tinc>1 && pcm->tlsupp<2) {
          len = 5;
          if (tinc==2) len=3;
          if (cmpch(&(lab[ii]),&(olab[ii]),len)) {
            if (lab[ii]=='0') {ii++; len--;}
            cwid = len*cs;
            gxchln(&lab[ii],len,cs,&cwid);
            xx = x - cwid*0.4;
            gxchpl(&lab[ii],len,xx,y,cs,cs*0.8,0.0);
          }
          if (pcm->xlside) y = y + cs*1.4;
          else y = y - cs*1.4;
          ii += len;
        }
        len = 4;
        if (tinc!=2 || tstrt.yr!=9999) {
          if (pcm->tlsupp==0) {
          if (cmpch(&(lab[ii]),&(olab[ii]),len)) {
            cwid = len*cs;
            gxchln(&lab[ii],len,cs,&cwid);
            xx = x - cwid*0.4;
            gxchpl(&lab[ii],len,xx,y,cs,cs*0.8,0.0);
          }
          }
        }
        strcpy (olab,lab);
      } else {
        y = (v*m)+b;
        gxplot (pos,y,3);
        if (pcm->ylside) gxplot (pos+(cs*0.4),y,2);
        else gxplot (pos-(cs*0.4),y,2);
        if (pcm->grflag==1 || pcm->grflag==2) {
          gxwide (pcm->grthck);
          gxstyl (pcm->grstyl);
          gxcolr (pcm->grcolr);
          gxplot (pcm->xsiz1,y,3);
          gxplot (pcm->xsiz2,y,2);
          gxwide(thck);
          gxcolr(colr);
          gxstyl(1);
        }
        ii = 0;
        if (pcm->tlsupp==1) len = len - 4;
        if (pcm->tlsupp==2) len = len - 9;
        if (len > 1) {
          if (tinc==3&&lab[0]=='0') {ii=1; len--;}
          y = y - (cs*0.5);
          cwid = len*cs;
          gxchln(&lab[ii],len,cs,&cwid);
          if (pcm->ylside) {
            x = pos + cs*0.8;
          } else {
            x = pos - (cwid+cs)*0.8;
            if (pcm->yllow<(cwid+cs)*0.8) pcm->yllow = (cwid+cs)*0.8;
          }
          gxchpl(&lab[ii],len,x,y,cs,cs*0.8,0.0);
        }
      }

      /* Get next date/time. */

      twrk = tincr;
      timadd (&tstrt, &twrk);
      tstrt = twrk;
      if (tincr.dy>1L&&(tstrt.dy==31L||(tstrt.dy==29L&&tstrt.dy==2))) {
        tstrt.dy = 1L;
        twrk = addmo;
        timadd (&tstrt, &twrk);
        tstrt = twrk;
      }
      if (tincr.dy>3&&tstrt.dy==3&&(tstrt.dy==2||tstrt.dy==3)) tstrt.dy = 1;
    }
  }
}

/* Set up map projection scaling.  */

void gamscl (struct gacmn *pcm) {
gaint rc=0, flag;

  if (pcm->paflg) {
    mpj.xmn = pcm->pxmin;
    mpj.xmx = pcm->pxmax;
    mpj.ymn = pcm->pymin;
    mpj.ymx = pcm->pymax;
  } else {
    mpj.xmn = 0.5;
    mpj.xmx = pcm->xsiz - 0.5;
    mpj.ymn = 0.75;
    mpj.ymx = pcm->ysiz - 0.75;
  }
  if (pcm->mpflg==4 && pcm->mproj>2 && pcm->mproj!=7) {
    mpj.lnmn = pcm->mpvals[0]; mpj.lnmx = pcm->mpvals[1];
    mpj.ltmn = pcm->mpvals[2]; mpj.ltmx = pcm->mpvals[3];
  } else {
    mpj.lnmn = pcm->dmin[0]; mpj.lnmx = pcm->dmax[0];
    mpj.ltmn = pcm->dmin[1]; mpj.ltmx = pcm->dmax[1];
  }
  flag = 0;
  if (pcm->mproj==3) {
    rc = gxnste (&mpj);
    if (rc) {
      gaprnt (0,"Map Projection Error:  Invalid coords for NPS\n");
      gaprnt (0,"  Will use linear lat-lon projection\n");
      flag = 1;
    }
  } else if (pcm->mproj==4) {
    rc = gxsste (&mpj);
    if (rc) {
      gaprnt (0,"Map Projection Error:  Invalid coords for SPS\n");
      gaprnt (0,"  Will use linear lat-lon projection\n");
      flag = 1;
    }
  } else if (pcm->mproj==5) {
    rc = gxrobi (&mpj);
    if (rc) {
      gaprnt (0,"Map Projection Error:  Invalid coords for Robinson\n");
      gaprnt (0,"  Will use linear lat-lon projection\n");
      flag = 1;
    }
  } else if (pcm->mproj==6) {
    rc = gxmoll (&mpj);
    if (rc) {
      gaprnt (0,"Map Projection Error:  Invalid coords for Mollweide\n");
      gaprnt (0,"  Will use linear lat-lon projection\n");
      flag = 1;
    }
  } else if (pcm->mproj==7) {
    if (pcm->mpflg==4) {
      mpj.axmn = pcm->mpvals[0]; mpj.axmx = pcm->mpvals[1];
      mpj.aymn = pcm->mpvals[2]; mpj.aymx = pcm->mpvals[3];
    } else mpj.axmn = -999.9;
    rc = gxortg (&mpj);
    if (rc) {
      gaprnt (0,"Map Projection Error:  Invalid coords for Orthographic Projection\n");
      gaprnt (0,"  Will use linear lat-lon projection\n");
      flag = 1;
    }
  } else if (pcm->mproj==13) {
    rc = gxlamc (&mpj);
    if (rc) {
      gaprnt (0,"Map Projection Error:  Invalid coords for Lambert conformal Projection\n");
      gaprnt (0,"  Will use linear lat-lon projection\n");
      flag = 1;
    }
  }

  if (pcm->mproj==2 || flag) rc = gxltln (&mpj);
  else if (pcm->mproj<2) rc = gxscld (&mpj, pcm->xflip, pcm->yflip);
  if (rc) {
    gaprnt (0,"Map Projection Error:  Internal Logic check\n");
    return;
  };

  pcm->xsiz1 = mpj.axmn;
  pcm->xsiz2 = mpj.axmx;
  pcm->ysiz1 = mpj.aymn;
  pcm->ysiz2 = mpj.aymx;
}

/* Plot map with proper data set, line style, etc. */
/* If iflg true, check pass number */

void gawmap (struct gacmn *pcm, gaint iflg) {
struct mapopt mopt;
gaint i;

  if (pcm->mproj==0 || (pcm->pass > 0 && iflg)) return;
  if (pcm->mpdraw==0) return;
  gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);
  if (pcm->mapcol<0) {
    /* set default map color, style, thickness */
    if (iflg) {
      mopt.dcol = pcm->mcolor;
      mopt.dstl = 1;
      mopt.dthk = 3;
    } else {
      mopt.dcol = 1;
      mopt.dstl = 1;
      mopt.dthk = 3;
    }
  } else {
    /* user has invoked 'set map' */
    mopt.dcol = pcm->mapcol;
    mopt.dstl = pcm->mapstl;
    mopt.dthk = pcm->mapthk;
  }
  /* set boundaries */
  mopt.lnmin = pcm->dmin[0]; mopt.lnmax = pcm->dmax[0];
  mopt.ltmin = pcm->dmin[1]; mopt.ltmax = pcm->dmax[1];
  /* arrays of map line attributes */
  mopt.mcol = pcm->mpcols;
  mopt.mstl = pcm->mpstls;
  mopt.mthk = pcm->mpthks;
  i = 0;
  while (pcm->mpdset[i]) {
    mopt.mpdset = pcm->mpdset[i];
    gxdmap (&(mopt));
    i++;
  }
  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
}

/* Select a contour interval.  */

void gacsel (gadouble rmin, gadouble rmax, gadouble *cint, gadouble *cmin, gadouble *cmax) {
  gadouble rdif,ndif,norml,w1,w2,t1,t2,absmax;

  /* determine if field is a constant */
  absmax = ( fabs(rmin)+fabs(rmax) + fabs(fabs(rmin)-fabs(rmax)) )/2;
  if (absmax > 0.0) {
    ndif = (fabs(rmax-rmin))/absmax;
  }
  else {
    /* rmin and rmax are both zero */
    *cint=0.0;
    *cmin=0.0;
    *cmax=0.0;
    return;
  }
      
  if (ndif > 0) {
    rdif = rmax-rmin;
  }
  else {
    /* rmin and rmax are the same */
    *cint=0.0;
    *cmin=0.0;
    *cmax=0.0;
    return;
  }
 
  /* determine appropriate contour min, max, and interval */
  if (*cint==0.0) {            /* no precision check */
    rdif = rdif/10.0;          /* appx. 10 contour intervals */
    w2 = floor(log10(rdif));
    w1 = pow(10.0,w2);
    norml = rdif/w1;           /* normalized contour interval */
    if     (norml>=1.0 && norml<=1.5) *cint=1.0;
    else if (norml>1.5 && norml<=2.5) *cint=2.0;
    else if (norml>2.5 && norml<=3.5) *cint=3.0;
    else if (norml>3.5 && norml<=7.5) *cint=5.0;
    else *cint=10.0;
    *cint = *cint*w1;
  }
  *cmin = *cint * ceil(rmin/(*cint));  
  *cmax = *cint * floor(rmax/(*cint));

  /* Check for interval being below machine epsilon for these values */
  t1 = rmin + *cint;
  t2 = rmax - *cint;
  if ((dequal(rmin, t1, 1.0e-16)==0) ||
      (dequal(rmax, t2, 1.0e-16)==0)) {
    *cint=0.0;
    *cmin=0.0;
    *cmax=0.0;
  }
  return;
}

/* Convert longitude to character */

gaint galnch (gadouble lon, char *ch) {
gaint len;

  while (lon<=-180.0) lon+=360.0;
  while (lon>180.0) lon-=360.0;

  snprintf(ch,29,"%g",fabs(lon));
  len=0;
  while (ch[len]) len++;
  if (lon<0.0) {
    ch+=len;
    *ch='W';
    *(ch+1)='\0';
    len++;
  }
  if (lon>0.0 && lon<180.0) {
    ch+=len;
    *ch='E';
    *(ch+1)='\0';
    len++;
  }
  return (len);
}

/* Convert latitude to character */

gaint galtch (gadouble lat, char *ch) {

gaint len;

  snprintf(ch,29,"%g",fabs(lat));
  len=0;
  while (ch[len]) len++;
  if (lat<0.0) {
    ch+=len;
    *ch='S';
    *(ch+1)='\0';
    len++;
  } else if (lat>0.0) {
    ch+=len;
    *ch='N';
    *(ch+1)='\0';
    len++;
  } else {
    *ch='E';
    *(ch+1)='Q';
    *(ch+2)='\0';
    len=2;
  }
  return (len);
}

/* Expand a grid using bi-linear interpolation.  Same args as  gagexp.  */

void gaglin (gadouble *g1, gaint cols, gaint rows, 
	     gadouble *g2, gaint exp1, gaint exp2, char *g1u, char *g2u) {
gadouble v1,v2,xoff,yoff,x,y,xscl,yscl;
gaint i,j,p1,p2,p3,p4,isiz,jsiz;

  isiz = (cols-1)*exp1+1;
  jsiz = (rows-1)*exp2+1;
  xscl = (gadouble)(cols-1)/(gadouble)(isiz-1);
  yscl = (gadouble)(rows-1)/(gadouble)(jsiz-1);
  for (j=0; j<jsiz; j++) {
  for (i=0; i<isiz; i++) {
    x = (gadouble)i*xscl;
    y = (gadouble)j*yscl;
    if (x<0.0) x=0.0;
    if (y<0.0) y=0.0;
    if (x>=(gadouble)cols) x=(gadouble)cols-0.001;  /* fudge the edges  */ 
    if (y>=(gadouble)rows) y=(gadouble)rows-0.001;  /* check with bdoty */
    p1 = (gaint)x + cols*(gaint)y;
    p2 = p1+1;
    p4 = p1+cols;
    p3 = p4+1;
    if (g1u[p1]==0 || g1u[p2]==0 || g1u[p3]==0 || g1u[p4]==0) {
      *g2u = 0;
    } else {
      xoff = x-floor(x);
      yoff = y-floor(y);
      v1 = g1[p1] + (g1[p2]-g1[p1])*xoff;
      v2 = g1[p4] + (g1[p3]-g1[p4])*xoff;
      *g2 = v1 + (v2-v1)*yoff;
      *g2u = 1;
    }
    g2++; g2u++;
  }}
}

/*   gagexp 

    Routine to expand a grid (Grid EXPand) into a larger grid
    using bi-directional cubic interpolation.  It is expected that this
    routine will be used to make a finer grid for a more pleasing
    contouring result.

    g1 -   addr of smaller grid
    cols - number of columns in smaller grid
    rows - number of rows in smaller grid
    g2   - addr of where larger grid is to go
    exp1 - expansion factor for rows (1=no expansion, 20 max);
    exp2 - expansion factor for columns (1=no expansion, 20 max);
    mval - missing data value

    number of output columns = (cols-1)*exp1+1
    number of output rows    = (rows-1)*exp2+1   */


void gagexp (gadouble *g1, gaint cols, gaint rows, 
	     gadouble *g2, gaint exp1, gaint exp2, char *g1u, char *g2u) {

gadouble *p1, *p2;
gadouble s1,s2,a,b,t;
gadouble pows1[20],pows2[20],pows3[20];
gadouble kurv;
gaint ii,jj,k,c,d,e;
char *p1u,*p2u;

 kurv=1.0;     /* Curviness factor for spline */

 /* First go through each row and interpolate the missing columns.
    Edge conditions (sides and missing value boundries) are handled
    by assuming a linear slope at the edge.  */

  for (k=0; k<exp1; k++) {
    t=( (gadouble)k ) / ( (gadouble)exp1 ) ;
    pows1[k]=t;
    pows2[k]=t*t;
    pows3[k]=t*pows2[k];
  }
  
  p1=g1;  
  p2=g2;
  p1u=g1u;
  p2u=g2u;
  
  c=((cols-1)*exp1)+1;
  d=c*exp2;
  e=c*(exp2-1);
  
  for (jj=0; jj<rows; jj++) {
    for (ii=0; ii<cols-1; ii++,p1++,p1u++) {
      if (*p1u==0 || *(p1u+1)==0) {
	*p2u = 0;
	p2++; p2u++;
	for (k=1; k<exp1; k++,p2++,p2u++) *p2u=0;
      } else {
	if (ii==0 || *(p1u-1)==0) 
	  s1 = *(p1+1) - *p1;
	else  
	  s1 = ( *(p1+1) - *(p1-1) )*0.5;
	if (ii==(cols-2) || *(p1u+2)==0) 
	  s2 = *(p1+1) - *p1;
	else  
	  s2 = ( *(p1+2) - *p1 )*0.5;
	s1*=kurv; 
	s2*=kurv;
	a = s1 + 2.0*(*p1) - 2.0*(*(p1+1)) + s2;
	b = 3.0*(*(p1+1)) - s2 - 2.0*s1 - 3.0*(*p1);
	*p2 = *p1;
	*p2u = 1;
	p2++; p2u++;
	for (k=1; k<exp1; k++,p2++,p2u++) {
	  *p2 = a*pows3[k] + b*pows2[k] + s1*pows1[k] + *p1;
	  *p2u = 1;
	}
      }
    }
    *p2 = *p1;
    *p2u = *p1u;
    p1++; p1u++;
    p2+=e+1; p2u+=e+1;
  }

  /* Now go through each column and interpolate the missing rows.
     This is done purely within the output grid, since we have already
     filled in as much info as we can from the input grid.  */
  
  if (exp2==1) return;
  
  for (k=0; k<exp2; k++) {
    t=( (gadouble)k ) / ( (gadouble)exp2 ) ;
    pows1[k]=t;
    pows2[k]=t*t;
    pows3[k]=t*pows2[k];
  }
  
  p2=g2;
  p2u=g2u;
  
  for (jj=0; jj<rows-1; jj++) {
    for (ii=0; ii<c; ii++,p2++,p2u++) {
      if (*p2u==0 || *(p2u+d)==0) {
	for (k=1; k<exp2; k++) *(p2u+(c*k))=0;
      } else {
	if (jj==0 || *(p2u-d)==0) 
	  s1 = *(p2+d) - *p2;
	else  
	  s1 = ( *(p2+d) - *(p2-d) )*0.5;
	if (jj==(rows-2) || *(p2u+(2*d))==0) 
	  s2 = *(p2+d) - *p2;
	else  
	  s2 = ( *(p2+(2*d)) - *p2 )*0.5;
	s1*=kurv; 
	s2*=kurv;
	a = s1 + 2.0*(*p2) - 2.0*(*(p2+d)) + s2;
	b = 3.0*(*(p2+d)) - s2 - 2.0*s1 - 3.0*(*p2);
	for (k=1; k<exp2; k++) {
	  *(p2+(c*k)) = a*pows3[k] + b*pows2[k] + s1*pows1[k] + *p2;
	  *(p2u+(c*k)) = 1;
	}
      }
    }
    p2+=e; p2u+=e;
  }
  return;
}

/* Rotate a grid.  Return rotated grid. */

struct gagrid *gaflip (struct gagrid *pgr, struct gacmn *pcm) {
struct gagrid *pgr2;
gadouble *gr1, *gr2;
char *gru1, *gru2;
gaint i, j, size;
size_t sz;

  sz = sizeof(struct gagrid);
  pgr2 = (struct gagrid *)galloc(sz,"pgrflip");
  if (pgr2==NULL) {
    gaprnt (0,"Memory Allocation Error:  gaflip pgr2 \n");
    return (NULL);
  }
  *pgr2 = *pgr;
  size = pgr->isiz * pgr->jsiz;
  sz = size*sizeof(gadouble);
  pgr2->grid = (gadouble *)galloc(sz,"grflip");
  if (pgr2->grid == NULL) {
    gaprnt (0,"Memory Allocation Error:  gaflip grid \n");
    gree(pgr2,"f265");
    return (NULL);
  }
  sz = size*sizeof(char);
  pgr2->umask = (char *)galloc(sz,"gruflip");
  if (pgr2->umask == NULL) {
    gaprnt (0,"Memory Allocation Error:  gaflip umask \n");
    gree(pgr2->grid,"f266");
    gree(pgr2,"f267");
    return (NULL);
  }

  gr1 = pgr->grid;
  gru1 = pgr->umask;
  for (j=0; j<pgr->jsiz; j++) {
    gr2  = pgr2->grid + j;
    gru2 = pgr2->umask + j;
    for (i=0; i<pgr->isiz; i++) {
      *gr2 = *gr1;
      *gru2 = *gru1;
      gr1++; gru1++;
      gr2 += pgr->jsiz;
      gru2 += pgr->jsiz;
    }
  }

  pgr2->idim = pgr->jdim;
  pgr2->jdim = pgr->idim;
  pgr2->iwrld = pgr->jwrld; pgr2->jwrld = pgr->iwrld;
  pgr2->isiz = pgr->jsiz;
  pgr2->jsiz = pgr->isiz;
  pgr2->igrab = pgr->jgrab;
  pgr2->jgrab = pgr->igrab;
  pgr2->ivals = pgr->jvals;
  pgr2->jvals = pgr->ivals;
  pgr2->ilinr = pgr->jlinr;
  pgr2->jlinr = pgr->ilinr;

  /* Add new grid to list to be freed later */

  i = pcm->relnum;
  pcm->type[i] = 1;
  pcm->result[i].pgr = pgr2;
  pcm->relnum++;

  return (pgr2);
}

/* Figure out appropriate date/time increment for time axis.  */

gaint gatinc (struct gacmn *pcm, struct dt *tstrt, struct dt *tincr) {
gaint tdif;
gadouble y1,y2;
gadouble c1,c2,c3;
struct dt twrk,temp;

  tincr->yr = 0L;
  tincr->mo = 0L;
  tincr->dy = 0L;
  tincr->hr = 0L;
  tincr->mn = 0L;
  twrk.yr = 0L;
  twrk.mo = 0L;
  twrk.dy = 0L;
  twrk.hr = 0L;
  twrk.mn = 0L;

  /* Check for a time period that covers a period of years.  */

  if (pcm->tmax.yr-pcm->tmin.yr>4) {
    y1 = pcm->tmin.yr;
    y2 = pcm->tmax.yr;
    c1 = 0.0;
    gacsel (y1,y2,&c1,&c2,&c3);
    tincr->yr = (gaint)(c1+0.5);
    if (tincr->yr==3) tincr->yr=5;
    goto cont;
  }

  /* Get time difference in minutes */

  tdif = timdif (&(pcm->tmin),&(pcm->tmax),0);

  /* Set time increment based on different time differences.
     The test is entirely arbitrary. */

  if (tdif>1576800L) tincr->mo = 6L;
  else if (tdif>788400L) tincr->mo = 3L;
  else if (tdif>262800L) tincr->mo = 1L;
  else if (tdif>129600L) tincr->dy = 15L;
  else if (tdif>42000L) tincr->dy = 5L;
  else if (tdif>14399L) tincr->dy = 2L;
  else if (tdif>7199L) tincr->dy = 1L;
  else if (tdif>3599L) tincr->hr = 12L;
  else if (tdif>1799L) tincr->hr = 6L;
  else if (tdif>899L) tincr->hr = 3L;
  else if (tdif>299L) tincr->hr = 1L;
  else if (tdif>149L) tincr->mn = 30L;
  else if (tdif>74L)  tincr->mn = 15L;
  else if (tdif>24L)  tincr->mn = 5L;
  else if (tdif>9L)   tincr->mn = 2L;
  else tincr->mn = 1L;

  /* Round tmin to get the correct starting time for this increment.
     Note that this algorithm assumes that only the above increment
     settings are possible.  So you can change the range tests
     (tdiff>something), but do not change the possible increments. */

cont:
  *tstrt = pcm->tmin;

  if (tincr->mn>0L) {
    tdif =  tincr->mn*(tstrt->mn/tincr->mn);
    if (tdif!=tstrt->mn) tstrt->mn = tdif+tincr->mn;
    if (tstrt->mn>59) {
      tstrt->mn = 0L;
      temp = twrk;
      temp.hr = 1L;
      timadd (tstrt,&temp);
      *tstrt = temp;
    }
    return(5);
  }
  if (tstrt->mn>0L) {
    tstrt->mn = 0L;
    temp = twrk;
    temp.hr = 1L;
    timadd (tstrt,&temp);
    *tstrt = temp;
  }

  if (tincr->hr>0L) {
    tdif = tincr->hr*(tstrt->hr/tincr->hr);
    if (tdif!=tstrt->hr) tstrt->hr = tdif+tincr->hr;
    if (tstrt->hr>23) {
      tstrt->hr = 0L;
      temp = twrk;
      temp.dy = 1L;
      timadd (tstrt,&temp);
      *tstrt = temp;
    }
    return (4);
  }
  if (tstrt->hr>0L) {
    tstrt->hr = 0L;
    temp = twrk;
    temp.dy = 1L;
    timadd (tstrt,&temp);
    *tstrt = temp;
  }

  if (tincr->dy>0L) {
    tdif = 1L+tincr->dy*((tstrt->dy-1L)/tincr->dy);
    if (tdif!=tstrt->dy) {
      tstrt->dy = tdif+tincr->dy;
      if (tstrt->dy==29L || tstrt->dy==31L) {
        tstrt->dy = 1L;
        temp = twrk;
        temp.mo = 1L;
        timadd (tstrt,&temp);
        *tstrt = temp;
      }
    }
    return (3);
  }
  if (tstrt->dy>1L) {
    tstrt->dy = 1L;
    temp = twrk;
    temp.mo = 1L;
    timadd (tstrt,&temp);
    *tstrt = temp;
  }

  if (tincr->mo>0L) {
    tdif = 1L+tincr->mo*((tstrt->mo-1L)/tincr->mo);
    if (tdif!=tstrt->mo) tstrt->mo = tdif+tincr->mo;
    if (tstrt->mo>12) {
      tstrt->mo = 1L;
      temp = twrk;
      temp.yr = 1L;
      timadd (tstrt,&temp);
      *tstrt = temp;
    }
    return (2);
  }
  if (tstrt->mo>1L) {
    tstrt->mo = 1L;
    temp = twrk;
    temp.yr = 1L;
    timadd (tstrt,&temp);
    *tstrt = temp;
  }

  tdif = tincr->yr*(tstrt->yr/tincr->yr);
  if (tdif!=tstrt->yr) tstrt->yr = tdif+tincr->yr;
  return(1);
}

/* Plot lat/lon lines when polar stereographic plots are done */

void gampax (struct gacmn *pcm) {
gadouble x1,y1,x2,y2;
gadouble lnmin,lnmax,ltmin,ltmax;
gadouble lnincr,ltincr,lnstrt,lnend,ltstrt,ltend;
gadouble v,s,plincr,lndif,ln,lt,cs;

  if (!pcm->grflag) return;

  if (pcm->mproj==5) {
    lnmin = pcm->dmin[0]; lnmax = pcm->dmax[0];
    ltmin = pcm->dmin[1]; ltmax = pcm->dmax[1];
    /* these are labels when lon ranges from -180 to 180 */
    if (fabs(lnmin+180.0)<1.0 && fabs(lnmax-180.0)<1.0 && fabs(ltmin+90.0)<1.0 && fabs(ltmax-90.0)<1.0) {
      cs = 0.10;
      gxconv (-180.0,90.0,&x1,&y1,2);   gxchpl ("180",3,x1-cs*1.5,y1+cs*0.6,cs*1.1,cs,0.0);
      gxconv (-90.0,90.0,&x1,&y1,2);    gxchpl ("90W",3,x1-cs*1.5,y1+cs*0.6,cs*1.1,cs,0.0);
      gxconv (0.0,90.0,&x1,&y1,2);      gxchpl ("0",  1,x1-cs*0.5,y1+cs*0.6,cs*1.1,cs,0.0);
      gxconv (90.0,90.0,&x1,&y1,2);     gxchpl ("90E",3,x1-cs*1.5,y1+cs*0.6,cs*1.1,cs,0.0);
      gxconv (180.0,90.0,&x1,&y1,2);    gxchpl ("180",3,x1-cs*1.5,y1+cs*0.6,cs*1.1,cs,0.0);

      gxconv (-180.0,-90.0,&x1,&y1,2);  gxchpl ("180",3,x1-cs*1.5,y1-cs*1.6,cs*1.1,cs,0.0);
      gxconv (-90.0,-90.0,&x1,&y1,2);   gxchpl ("90W",3,x1-cs*1.5,y1-cs*1.6,cs*1.1,cs,0.0);
      gxconv (0.0,-90.0,&x1,&y1,2);     gxchpl ("0",  1,x1-cs*0.5,y1-cs*1.6,cs*1.1,cs,0.0);
      gxconv (90.0,-90.0,&x1,&y1,2);    gxchpl ("90E",3,x1-cs*1.5,y1-cs*1.6,cs*1.1,cs,0.0);
      gxconv (180.0,-90.0,&x1,&y1,2);   gxchpl ("180",3,x1-cs*1.5,y1-cs*1.6,cs*1.1,cs,0.0);

      gxconv (-180.0,-60.0,&x1,&y1,2);  gxchpl ("60S",3,x1-cs*4.5,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (-180.0,-30.0,&x1,&y1,2);  gxchpl ("30S",3,x1-cs*4.0,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (-180.0,0.0,&x1,&y1,2);    gxchpl ("EQ", 2,x1-cs*2.7,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (-180.0,30.0,&x1,&y1,2);   gxchpl ("30N",3,x1-cs*4.0,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (-180.0,60.0,&x1,&y1,2);   gxchpl ("60N",3,x1-cs*4.5,y1-cs*0.55,cs*1.1,cs,0.0);

      gxconv (180.0,-60.0,&x1,&y1,2);   gxchpl ("60S",3,x1+cs*1.5,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (180.0,-30.0,&x1,&y1,2);   gxchpl ("30S",3,x1+cs*1.0,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (180.0,0.0,&x1,&y1,2);     gxchpl ("EQ", 2,x1+cs*0.7,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (180.0,30.0,&x1,&y1,2);    gxchpl ("30N",3,x1+cs*1.0,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (180.0,60.0,&x1,&y1,2);    gxchpl ("60N",3,x1+cs*1.5,y1-cs*0.55,cs*1.1,cs,0.0);

    }
    /* these are labels when lon ranges from 0 to 360 */
    else if (fabs(lnmin+0.0)<1.0 && fabs(lnmax-360.0)<1.0 && fabs(ltmin+90.0)<1.0 && fabs(ltmax-90.0)<1.0) {
      cs = 0.10;
      gxconv (0.0,90.0,&x1,&y1,2);      gxchpl ("0",  1,x1-cs*0.5,y1+cs*0.6,cs*1.1,cs,0.0);
      gxconv (90.0,90.0,&x1,&y1,2);     gxchpl ("90E",3,x1-cs*1.5,y1+cs*0.6,cs*1.1,cs,0.0);
      gxconv (180.0,90.0,&x1,&y1,2);    gxchpl ("180",3,x1-cs*1.5,y1+cs*0.6,cs*1.1,cs,0.0);
      gxconv (270.0,90.0,&x1,&y1,2);    gxchpl ("90W",3,x1-cs*1.5,y1+cs*0.6,cs*1.1,cs,0.0);
      gxconv (360.0,90.0,&x1,&y1,2);    gxchpl ("0",  1,x1-cs*0.5,y1+cs*0.6,cs*1.1,cs,0.0);

      gxconv (0.0,-90.0,&x1,&y1,2);     gxchpl ("0",  1,x1-cs*0.5,y1-cs*1.6,cs*1.1,cs,0.0);
      gxconv (90.0,-90.0,&x1,&y1,2);    gxchpl ("90E",3,x1-cs*1.5,y1-cs*1.6,cs*1.1,cs,0.0);
      gxconv (180.0,-90.0,&x1,&y1,2);   gxchpl ("180",3,x1-cs*1.5,y1-cs*1.6,cs*1.1,cs,0.0);
      gxconv (270.0,-90.0,&x1,&y1,2);   gxchpl ("90W",3,x1-cs*1.5,y1-cs*1.6,cs*1.1,cs,0.0);
      gxconv (360.0,-90.0,&x1,&y1,2);   gxchpl ("0",  1,x1-cs*0.5,y1-cs*1.6,cs*1.1,cs,0.0);

      gxconv (0.0,-60.0,&x1,&y1,2);     gxchpl ("60S",3,x1-cs*4.5,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (0.0,-30.0,&x1,&y1,2);     gxchpl ("30S",3,x1-cs*4.0,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (0.0,0.0,&x1,&y1,2);       gxchpl ("EQ", 2,x1-cs*2.7,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (0.0,30.0,&x1,&y1,2);      gxchpl ("30N",3,x1-cs*4.0,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (0.0,60.0,&x1,&y1,2);      gxchpl ("60N",3,x1-cs*4.5,y1-cs*0.55,cs*1.1,cs,0.0);

      gxconv (360.0,-60.0,&x1,&y1,2);   gxchpl ("60S",3,x1+cs*1.5,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (360.0,-30.0,&x1,&y1,2);   gxchpl ("30S",3,x1+cs*1.0,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (360.0,0.0,&x1,&y1,2);     gxchpl ("EQ", 2,x1+cs*0.7,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (360.0,30.0,&x1,&y1,2);    gxchpl ("30N",3,x1+cs*1.0,y1-cs*0.55,cs*1.1,cs,0.0);
      gxconv (360.0,60.0,&x1,&y1,2);    gxchpl ("60N",3,x1+cs*1.5,y1-cs*0.55,cs*1.1,cs,0.0);
    }
    /* draw longitude lines */
    for (ln=lnmin; ln<lnmax+10.0; ln+=45.0) {
      if (ln<lnmin+10.0 || ln>lnmax-10.0) {
	/* these form the frame, so use annotation color and thickness */
        gxstyl (1);
        gxcolr (pcm->anncol);
        gxwide (pcm->annthk);
      } else {
	/* these are grid lines */
        gxstyl (pcm->grstyl);
        gxcolr (pcm->grcolr);
        gxwide (pcm->grthck);
      }
      gxconv (ln,ltmin,&x1,&y1,2);
      gxplot (x1,y1,3);
      for (lt=ltmin; lt<ltmax+1.0; lt+=2.0) {
        gxconv (ln,lt,&x1,&y1,2);
        gxplot (x1,y1,2);
      }
    }
    /* draw latitude lines */
    for (lt=ltmin; lt<ltmax+10.0; lt+=30.0) {
      if (lt<ltmin+10.0 || lt>ltmax-10.0) {
	/* these form the frame, so use annotation color and thickness */
        gxstyl (1);
        gxcolr (pcm->anncol);
        gxwide (pcm->annthk);
      } else {
	/* these are grid lines */
        gxstyl (pcm->grstyl);
        gxcolr (pcm->grcolr);
        gxwide (pcm->grthck);
      }
      gxconv (lnmin,lt,&x1,&y1,2);
      gxplot (x1,y1,3);
      for (ln=lnmin; ln<lnmax+1.0; ln+=2.0) {
        gxconv (ln,lt,&x1,&y1,2);
        gxplot (x1,y1,2);
      }
    }
    return;
  }  /* end of robinson projection case */

  /* Choose grid interval for longitude */
  lnincr=0.0;
  lnmin = pcm->dmin[0];
  lnmax = pcm->dmax[0];
  gacsel (lnmin,lnmax,&lnincr,&lnstrt,&lnend);
  if (lnincr==0.0)  {
    gaprnt (0,"gaaxis internal logic check 25\n");
    return;
  }
  if (lnincr>24.9) lnincr=30.0;
  else if (lnincr>14.5) lnincr=20.0;
  else if (lnincr>10.0) lnincr=10.0;
  gacsel (lnmin,lnmax,&lnincr,&lnstrt,&lnend);
  lndif = lnmax - lnmin;

  if (pcm->xlint!=0.0) lnincr=pcm->xlint;   /* set the lon increment from xlint */

  /* Choose grid interval for latitude */
  ltincr=0.0;
  ltmin = pcm->dmin[1];
  ltmax = pcm->dmax[1];
  gacsel (ltmin,ltmax,&ltincr,&ltstrt,&ltend);
  if (ltincr==0.0)  {
    gaprnt (0,"gaaxis internal logic check 25\n");
    return;
  }
  if (lndif>300.0) {
    if (ltincr>9.9) ltincr = 20.0;
    else if (ltincr>4.9) ltincr = 10.0;
    else if (ltincr>1.9) ltincr = 5.0;
    else if (ltincr>0.8) ltincr = 2.0;
    else if (ltincr>0.4) ltincr = 1.0;
    else if (ltincr>0.2) ltincr = 0.5;
  } else {
    if (ltincr>14.9) ltincr = 20.0;
    else if (ltincr>7.9) ltincr = 10.0;
    else if (ltincr>2.9) ltincr = 5.0;
    else if (ltincr>1.4) ltincr = 2.0;
    else if (ltincr>0.6) ltincr = 1.0;
    else if (ltincr>0.2) ltincr = 0.5;
  }

  if (pcm->ylint!=0.0) ltincr=pcm->ylint;   /* set the lat increment from ylint */

  if (ltstrt<-89.9) {
    ltstrt = ltstrt + ltincr;
    ltmin = ltstrt;
  }
  if (ltend>89.9) {
    ltend = ltend - ltincr;
    ltmax = ltend;
  }

  gxstyl (pcm->grstyl);
  gxcolr (pcm->grcolr);
  gxwide (pcm->grthck);
  gxclip (pcm->xsiz1,pcm->xsiz2,pcm->ysiz1,pcm->ysiz2);
  for (v=lnstrt; v<lnend+lnincr*0.5; v+=lnincr) {
    gxconv (v,ltmin,&x1,&y1,2);
    gxconv (v,ltmax,&x2,&y2,2);
    gxplot (x1,y1,3);
    gxplot (x2,y2,2);
  }

  if (lndif>180.0) plincr = lndif/100.0;
  else if (lndif>60.0) plincr = lndif/50.0;
  else plincr = lndif/25.0;
  for (v=ltstrt; v<ltend+ltincr*0.5; v+=ltincr) {
    gxconv (lnmin,v,&x1,&y1,2);
    gxplot (x1,y1,3);
    for (s=lnmin+plincr; s<lnmax+plincr*0.5; s+=plincr) {
      gxconv (s,v,&x1,&y1,2);
      gxplot (x1,y1,2);
    }
  }

  /* Plot circular frame if user requested such a thing */

  if (pcm->frame==2) {
    gxcolr (pcm->anncol);
    gxwide (pcm->annthk);
    gxstyl (1);
    /* for orthographic projections */
    if (pcm->mproj==7) {
      /* draw line along min longitude */
      v=lnmin;
      gxconv (v,ltmin,&x1,&y1,2);
      gxplot (x1,y1,3);
      for (s=ltmin+plincr; s<ltmax+plincr*0.5; s+=plincr) {
	gxconv (v,s,&x1,&y1,2);
	gxplot (x1,y1,2);
      }
      /* draw 2nd line along max longitude */
      v=lnmax;
      gxconv (v,ltmin,&x1,&y1,2);
      gxplot (x1,y1,3);
      for (s=ltmin+plincr; s<ltmax+plincr*0.5; s+=plincr) {
	gxconv (v,s,&x1,&y1,2);
	gxplot (x1,y1,2);
      }
    }
    /* for other projections: nps, sps */
    else {
      if (pcm->mproj==3) v=ltmin;
      else v=ltmax;
      /* draw line around latitude circle */
      gxconv (lnmin,v,&x1,&y1,2);
      gxplot (x1,y1,3);
      for (s=lnmin+plincr; s<lnmax+plincr*0.5; s+=plincr) {
	gxconv (s,v,&x1,&y1,2);
	gxplot (x1,y1,2);
      }
    }
  }

  gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);
  gxstyl(1);
}

/* Plot weather symbol */

void wxsym (gaint type, gadouble xpos, gadouble ypos, gadouble scale, gaint colr, gaint *wxcols) {
gaint ioff,len,i,part,icol;
gadouble x=0,y=0;

  wxymin = 9e30;
  wxymax = -9e30;
  if (type<1||type>43) return;
  ioff = strt[type-1]-1;
  len = scnt[type-1];
  for (i=0; i<len; i++) {
    part = stype[i+ioff]-1;
    if (colr<0) {
      if (type==25) icol=0;
      else if (type==14 || type==15) icol=1;
      else if (type==19 || type==20) icol=2;
      else if (type==31 || type==32) icol=0;
      else icol = scol[part];
      gxcolr (*(wxcols+icol));
    } else gxcolr (colr);
    x = xpos + sxpos[i+ioff]*scale;
    y = ypos + sypos[i+ioff]*scale;
    wxprim (part, x, y, scale);
  }
  if (type==40 || type==42) gxmark (2,x,y,scale*0.4);
  if (type==41 || type==43) gxmark (3,x,y,scale*0.4);
}


/* Plot primitive wx symbol */

void wxprim (gaint part, gadouble xpos, gadouble ypos, gadouble scale) {
gaint i,len,pos;
gadouble x,y, xy[40];

  len = slen[part];
  pos = soff[part]-1;
  for (i=0; i<len; i++) {
    x = xpts[i+pos];
    y = ypts[i+pos];
    x = x*scale + xpos;
    y = y*scale + ypos;
    if (part==1 || part==2) {
      xy[i*2] = x;
      xy[i*2+1] = y;
    } else {
      gxplot (x,y,spens[i+pos]);
    }
    if (y<wxymin) wxymin = y;
    if (y>wxymax) wxymax = y;
  }
  if (part==1 || part==2) gxfill(xy,len);
}

void gagsav (gaint type, struct gacmn *pcm, struct gagrid *pgr) {
  pcm->lastgx = type;
}

/* Log axis scaling */

void galnx (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = log(xin);
  *yout = yin;
}

void galny (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = xin;
  *yout = log(yin);
}

void gaalnx (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = pow(2.71829,xin);
  *yout = yin;
}

void gaalny (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = xin;
  *yout = pow(2.71829,yin);
}

void galogx (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = log10(xin);
  *yout = yin;
}

void galogy (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = xin;
  *yout = log10(yin);
}

void galog2 (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = log10(xin);
  *yout = log10(yin);
}

void gaalogx (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = pow(10.0,xin);
  *yout = yin;
}

void gaalogy (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = xin;
  *yout = pow(10.0,yin);
}

void gaalog2 (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = pow(10.0,xin);
  *yout = pow(10.0,yin);
}


/* cos lat scaling */

void gaclx (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = sin(xin*M_PI/180);
  *yout = yin;
}

void gacly (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = xin;
  *yout = sin(yin*M_PI/180);
}

void gaaclx (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = asin(xin)*180/M_PI;
  *yout = yin;
}

void gaacly (gadouble xin, gadouble yin, gadouble *xout, gadouble *yout) {
  *xout = xin;
  *yout = asin(yin)*180/M_PI;
}


/* Grid fill -- using color ranges as in shaded contours */

void gagfil (gadouble *r, gaint is, gaint js, gadouble *vs, gaint *clrs, gaint lvs, char *ru) {
gadouble x,y,*v,xybox[10];
gaint i,j,k,flag;
char *vu;

  v = r;
  vu = ru;
  for (j=1; j<=js; j++) {
    for (i=1; i<=is; i++) {
      if (*vu != 0) {
	flag = 1;
	for (k=0; k<lvs-1; k++) {
	  if (*v>*(vs+k) && *v<=*(vs+k+1)) {
	    gxcolr(*(clrs+k));
	    gxconv ((gadouble)(i)-0.5,(gadouble)(j)-0.5,&x,&y,3);
	    xybox[0] = x; xybox[1] = y;
	    gxconv ((gadouble)(i)+0.5,(gadouble)(j)-0.5,&x,&y,3);
	    xybox[2] = x; xybox[3] = y;
	    gxconv ((gadouble)(i)+0.5,(gadouble)(j)+0.5,&x,&y,3);
	    xybox[4] = x; xybox[5] = y;
	    gxconv ((gadouble)(i)-0.5,(gadouble)(j)+0.5,&x,&y,3);
	    xybox[6] = x; xybox[7] = y;
	    gxconv ((gadouble)(i)-0.5,(gadouble)(j)-0.5,&x,&y,3);
	    xybox[8] = x; xybox[9] = y;
	    gxfill (xybox,5);
	    flag = 0;
	    break;
	  }
	}
	if (flag) {
	  gxcolr(*(clrs+lvs-1));
	  gxconv ((gadouble)(i)-0.5,(gadouble)(j)-0.5,&x,&y,3);
	  xybox[0] = x; xybox[1] = y;
	  gxconv ((gadouble)(i)+0.5,(gadouble)(j)-0.5,&x,&y,3);
	  xybox[2] = x; xybox[3] = y;
	  gxconv ((gadouble)(i)+0.5,(gadouble)(j)+0.5,&x,&y,3);
	  xybox[4] = x; xybox[5] = y;
	  gxconv ((gadouble)(i)-0.5,(gadouble)(j)+0.5,&x,&y,3);
	  xybox[6] = x; xybox[7] = y;
	  gxconv ((gadouble)(i)-0.5,(gadouble)(j)-0.5,&x,&y,3);
	  xybox[8] = x; xybox[9] = y;
	  gxfill (xybox,5);
	}
      }
      v++; vu++;
    } 
  }
}

/* Direct image map to X display.  Bypasses metafile (ie, no refresh on
   resize and no hardcopy output */

void gaimap (gadouble *r, gaint is, gaint js, gadouble *vs, gaint *clrs, gaint lvs, 
	     char *ru, gadouble xlo, gadouble xhi, gadouble ylo, gadouble yhi) {
gadouble x,y,*xvals,*yvals,xscl,yscl;
gaint i,j,k,off,ii,jj,*imap,*im,flag,imn,imx,jmn,jmx,iz,jz,*ivals,*jvals;

  if (dsubs==NULL) dsubs = getdsubs();  /* get ptrs to the graphics display functions */

  /* Determine the pixel size of our image, which is the clipping area.
     We assume a linear trasform from grid to physical page.  ie, only latlon
     and scaled map projections. */
 
  dsubs->gxdgcoord(xlo,ylo,&imn,&jmn);
  dsubs->gxdgcoord(xhi,yhi,&imx,&jmx);
  if (imn<-900) return;   /* indicates batch mode */

  /* pixel to page scaling */

  xscl = (xhi-xlo)/(gadouble)(imx-imn);
  yscl = (yhi-ylo)/(gadouble)(jmx-jmn);

  /* Allocate memory for the image and some work arrays */

  if (imx>imn) iz = 1+imx-imn;
  else iz = 1+imn-imx;
  if (jmx>jmn) jz = 1+jmx-jmn;
  else jz = 1+jmn-jmx;

  imap = NULL; xvals = NULL; yvals = NULL; ivals = NULL; jvals = NULL;
  imap = (gaint *)malloc(sizeof(gaint)*iz*jz);
  if (imap == NULL) goto err;
  xvals = (gadouble *)malloc(sizeof(gadouble)*is);
  if (xvals == NULL) goto err;
  yvals = (gadouble *)malloc(sizeof(gadouble)*js);
  if (yvals == NULL) goto err;
  ivals = (gaint *)malloc(sizeof(gaint)*iz);
  if (ivals == NULL) goto err;
  jvals = (gaint *)malloc(sizeof(gaint)*jz);
  if (jvals == NULL) goto err;

  /* Get x,y page coord values for the grid axes */

  for (i=1; i<=is; i++) {
    gxconv ((gadouble)i,(gadouble)1.0,&x,&y,3);
    *(xvals+i-1) = x; 
  }
  for (j=1; j<=js; j++) {
    gxconv ((gadouble)1.0,(gadouble)j,&x,&y,3);
    *(yvals+j-1) = y; 
  }

  /* For each pixel (for one row and one column), convert from pixel space
     to page space, and then search through the list of xvals/yvals to 
     find the nearest grid i and j to this pixel.  */

  /* First do a row */

  for (i=0; i<iz; i++) {
    ii = i;
    if (imn>imx) ii = 1+i-iz;
    x = xlo + xscl * (gadouble)ii;
    ii = 1;
    while (ii<is) {
      if ( (*(xvals+ii) >= x && *(xvals+ii-1) <= x ) ||
           (*(xvals+ii) <= x && *(xvals+ii-1) >= x ) ) {
        if ( (fabs(*(xvals+ii)-x)) > (fabs(*(xvals+ii-1)-x)) ) ii = ii - 1;
        break;
      }
      ii++;
    }
    if (ii==is) *(ivals+i) = -999;
    else *(ivals+i) = ii;
  }

  /* Now do a column */

  for (j=0; j<jz; j++) {
    jj = j;
    if (jmn>jmx) jj = 1+j-jz;
    y = ylo + yscl * (gadouble)jj;
    jj = 1;
    while (jj<js) {
      if ( (*(yvals+jj) >= y && *(yvals+jj-1) <= y ) ||
           (*(yvals+jj) <= y && *(yvals+jj-1) >= y ) ) {
        if ((fabs(*(yvals+jj)-y)) > (fabs(*(yvals+jj-1)-y)) ) jj = jj - 1;
        break;
      }
      jj++;
    }
    if (jj==js) *(jvals+j) = -999;
    else *(jvals+j) = jj;
  }

  /* Fill image with appropriate color numbers, based on the grid 
     values closest to each pixel.  */

  im = imap;
  for (j=0; j<jz; j++) {
    for (i=0; i<iz; i++) {
      *im = 255;
      if (*(ivals+i)>=0 && *(jvals+j)>=0) {
        off = *(jvals+j)*is + *(ivals+i);
        if (*(ru+off) != 0) {
          flag = 1;
          for (k=0; k<lvs-1; k++) {
            if (*(r+off)>*(vs+k) && *(r+off)<=*(vs+k+1)) {
              *im = *(clrs+k);
              flag = 0;
	      break;
            }
	  }
          if (flag) {
	    *im = *(clrs+lvs-1);
          }
        }
      }
      im++; 
    } 
  }

  /* Dump image to X screen */

  ii = imn;
  if (imn>imx) ii=imx;
  jj = jmn;
  if (jmn>jmx) jj=jmx;
  dsubs->gxdimg(imap,ii,jj,iz,jz);

  free (imap);
  free (xvals); free (yvals);
  free (ivals); free (jvals);
  return;

err:
  gaprnt (0,"Memory error in gaimap\n");
  if (imap) free(imap);
  if (xvals) free(xvals);
  if (yvals) free(yvals);
  if (ivals) free(ivals);
  return;
}

void gafram (struct gacmn *pcm) {

  gxcolr (pcm->anncol);
  if (pcm->frame==0) return;
  if (pcm->frame==2 && pcm->mproj>2) return;
  if (pcm->mproj>4) return;
  gxwide (pcm->annthk);
  gxstyl (1);
  gxplot (pcm->xsiz1,pcm->ysiz1,3);
  gxplot (pcm->xsiz2,pcm->ysiz1,2);
  gxplot (pcm->xsiz2,pcm->ysiz2,2);
  gxplot (pcm->xsiz1,pcm->ysiz2,2);
  gxplot (pcm->xsiz1,pcm->ysiz1,2);
}

void gaaxpl (struct gacmn *pcm, gaint idim, gaint jdim) {

  if (idim==0 && jdim==1 && pcm->mproj>2) gampax(pcm);
  else {
    gaaxis(1,pcm,idim);
    gaaxis(0,pcm,jdim);
  }
}

/* Select colors and intervals for colorized items such as
   colorized vectors, streamlines, barbs, scatter, etc.  */

void gaselc (struct gacmn *pcm, gadouble rmin, gadouble rmax) {
  gadouble rdif,w1,norml,cint,cmin,cmax,ndif,absmax;
  gadouble t1,t2,fact,rr,rrb,smin,ci,ccnt;
  gaint i,ii,irb,lcnt;

  /* determine if field is a constant */
  absmax = ( fabs(rmin)+fabs(rmax) + fabs(fabs(rmin)-fabs(rmax)) )/2;
  if (absmax > 0.0) {
    ndif = (fabs(rmax-rmin))/absmax;
  }
  else {
    /* rmin and rmax are both zero */
    pcm->shdcnt = 0;
    return;
  }
      
  if (ndif > 0) {
    rdif = rmax - rmin;
  }
  else {
    /* rmin and rmax are the same */
    pcm->shdcnt = 0;
    return;
  }

  if (pcm->rbflg) irb = pcm->rbflg - 1;
  else irb = 12;
  rrb = irb+1;

  /* Choose a contour interval if one is needed.  We choose
     an interval that maximizes the number of colors displayed,
     rather than an interval that maximizes the attribute of
     being a 'nice' interval */

  if (!pcm->cflag) {
    if (pcm->cint==0.0) {           /* no precision check */
      if (pcm->rbflg) rdif = rdif/((gadouble)(pcm->rbflg)-0.5);
      else rdif = rdif/12.5;
      w1 = floor(log10(rdif));
      w1 = pow(10.0,w1);

      norml = rdif/w1;              /* normalized contour interval */
      if (norml<2.0) fact = 10.0;
      else if (norml>=2.0 && norml<4.0) fact = 5.0;
      else if (norml>=4.0 && norml<7.0) fact = 2.0;
      else fact = 1.0;
      norml*=fact;
      i = (gaint)(norml+0.5);
      cint = (gadouble)i * w1 / fact;
    } 
    else cint = pcm->cint;

    cmin = cint * ceil(rmin/cint);
    cmax = cint * floor(rmax/cint);

    /* Check for interval being below machine epsilon for these values */
    t1 = rmin + cint;
    t2 = rmax - cint;
    if ((dequal(rmin, t1, 1.0e-16)==0) ||
	(dequal(rmax, t2, 1.0e-16)==0)) {
      pcm->shdcnt = 0;
      return;
    }

    /* Figure out actual contour levels and colors, based on
       possible user settings such as cmin, etc.  */

    if (pcm->rainmn==0.0 && pcm->rainmx==0.0) {
      pcm->rainmn = cmin;
      pcm->rainmx = cmax;
    }
    lcnt=0;
    smin = pcm->rainmn - cint;
    for (rr=cmin-cint;rr<=cmax+(cint/2.0);rr+=cint) {
      if (rr<0.0) i = (gaint)((rr/cint)-0.1);
      else i = (gaint)((rr/cint)+0.1);
      rr = (gadouble)i * cint;
      pcm->shdlvs[lcnt] = rr;
      pcm->shdcls[lcnt] = 1;
      if (rr<pcm->cmin) pcm->shdcls[lcnt] = -1;
      if (rr+cint>pcm->cmax) pcm->shdcls[lcnt] = -1;
      if (pcm->blkflg && rr<pcm->blkmax && rr+cint>pcm->blkmin) pcm->shdcls[lcnt] = -1;
      if (pcm->shdcls[lcnt]==1) {
        ii = (gaint)(rrb*(rr-smin)/(pcm->rainmx-smin));
        if (ii>irb) ii=irb;
        if (ii<0) ii=0;
        if (pcm->rbflg) pcm->shdcls[lcnt] = pcm->rbcols[ii];
        else pcm->shdcls[lcnt] = rcols[ii];
      }
      if (lcnt<1 || pcm->shdcls[lcnt]!=pcm->shdcls[lcnt-1]) lcnt++;
    }
    pcm->shdlvs[0] -= cint*10.0;
    pcm->shdcnt = lcnt;

  /* User has specified actual contour levels.  Just use those */

  } else {

    if (rmin<pcm->clevs[0]) pcm->shdlvs[0] = rmin-100.0;
    else pcm->shdlvs[0] = pcm->clevs[0]-100.0;
    ccnt = rrb/(gadouble)(pcm->cflag+1);
    ci = ccnt/2.0;
    ii = (gaint)ci;
    if (pcm->ccflg>0) pcm->shdcls[0] = pcm->ccols[0];
    else {
      if (pcm->rbflg>0) pcm->shdcls[0] = pcm->rbcols[ii];
      else pcm->shdcls[0] = rcols[ii];
    }
    lcnt = 1;
    for (i=0; i<pcm->cflag; i++) {
      pcm->shdlvs[lcnt] = pcm->clevs[i];
      if (pcm->ccflg>0) {
        if (i+1>=pcm->ccflg) pcm->shdcls[lcnt]=15;
        else pcm->shdcls[lcnt]=pcm->ccols[i+1];
      } else {
        ci += ccnt;
        ii = (gaint)ci;
        if (ii>irb) ii=irb;
        if (ii<0) ii=0;
        if (pcm->rbflg>0) pcm->shdcls[lcnt] = pcm->rbcols[ii];
        else pcm->shdcls[lcnt] = rcols[ii];
      }
      if (lcnt<1 || pcm->shdcls[lcnt]!=pcm->shdcls[lcnt-1]) lcnt++;
    }
    pcm->shdcnt = lcnt;
  }
}

/* Given a shade value, return the relevent color */

gaint gashdc (struct gacmn *pcm, gadouble val) {
gaint i;
  if (pcm->shdcnt==0) return(1);
  if (pcm->shdcnt==1) return(pcm->shdcls[0]);
  if (val<=pcm->shdlvs[1]) return(pcm->shdcls[0]);
  for (i=1; i<pcm->shdcnt-1; i++) {
    if (val>pcm->shdlvs[i] && val<=pcm->shdlvs[i+1]) return(pcm->shdcls[i]);
  }
  return(pcm->shdcls[pcm->shdcnt-1]);
}

struct cxclock {
  time_t year ;
  time_t month ;
  time_t date ;
  time_t hour ;
  time_t minute ;
  time_t second ;
  char timezone[32] ;
  char clockenv[64] ;
  gaint julian_day ;
  time_t epoch_time_in_sec ;
} ;

/* global variable of the time object */
static struct cxclock timeobj;


void get_tm_struct(time_t t)
{
  struct tm *ptr_tm ;
  char buf[10] ;
  time_t sec ;
  sec=t ;

  /* get tm struct from seconds */
  ptr_tm=localtime(&sec) ;

  if ( ! ptr_tm ) {
    fprintf(stderr,"error in gmtime function,errno = %d\n",errno);
    exit(-1) ;
  }
	
  /* copy tm struct to cxclock private members */
  timeobj.epoch_time_in_sec=sec ;
  timeobj.year=ptr_tm->tm_year + 1900  ;
  timeobj.month=(ptr_tm->tm_mon) + 1 ;
  timeobj.date=ptr_tm->tm_mday ;
  timeobj.hour=ptr_tm->tm_hour ;
  timeobj.minute=ptr_tm->tm_min ;
  timeobj.second=ptr_tm->tm_sec ;

  /* get julian date */
  strftime(buf, sizeof(buf),"%j", ptr_tm) ;
  timeobj.julian_day=atoi(buf) ;

}

void sys_time() {
  time_t sec ;
  time(&sec) ; /* get the GMT offset from the local machine */
  if (sec == -1 ) {
    fprintf(stderr,"error in time function,errno = %d\n",errno) ;
    exit(-1) ;
  }
  /* Set tm struct using new epoch time */
  get_tm_struct(sec) ;
  return;
}


void gatmlb (struct gacmn *pcm) {
  gaint i,vcnt;
  char dtgstr[32] ;

  vcnt = 0;
  for (i=0; i<5; i++) if (pcm->vdim[i]) vcnt++;
  /* -- only plot if 2-D or less, i.e., turn off on looping */
  if (vcnt<=2) {
    sys_time();
    snprintf(dtgstr,31,"%04ld-%02ld-%02ld-%02ld:%02ld",
	  timeobj.year, timeobj.month, timeobj.date, timeobj.hour, timeobj.minute) ;
    gxchpl(dtgstr,strlen(dtgstr),pcm->pxsize-1.7,0.05,0.1,0.09,0.0);
  }
}

