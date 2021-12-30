/* Copyright (C) 1988-2020 by George Mason University. See file COPYRIGHT for more information. */

/*  
   Reads the metadata from a Self-Describing File 
   and fill in the information into a gafile structure.
   Authored by Don Hooper and modified by Jennifer M. Adams 
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if (USENETCDF==1 || USEHDF==1)

#define Success	0
#define Failure	1
#define XINDEX 0
#define YINDEX 1
#define ZINDEX 2
#define TINDEX 3
#define EINDEX 4
#define DFLTORIGIN " since 1-1-1 00:00:0.0"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "grads.h"
#include "gasdf.h"
#if USENETCDF==1
#include "netcdf.h"
#include "gasdf_std_time.h"
#endif
#if USEHDF == 1
#include "mfhdf.h"
#endif
#include "udunits2.h"


extern struct gamfcmn mfcmn ;
extern FILE *descr ;

char *gxgnam(char *);          /* This is also in gx.h */
static char pout[1256];        /* Build error msgs here */
static gaint utSysInit = 0;    /* Flag for udunits2 system initialization */
static ut_system *utSys=NULL;  /* Pointer to udunits2 system */
static ut_unit *second=NULL, *feet=NULL, *pascals=NULL, *kelvins=NULL;  /* known unit pointers */

/* STNDALN requires gaxdfopen routine and others contained therein, 
   which turns out to be everything except the gasdfopen() routine. 
   It is used for parsing a descriptor file to see if GrADS will open it. */
#ifndef STNDALN


/* Open a self-describing file by reading the metadata 
   and filling in a gafile structure. Chain the gafile 
   structure on to the list anchored in the gastat. */

gaint gasdfopen (char *args, struct gacmn *pcm) {
  struct gafile *pfi, *pfio;
  gaint rc, len;
  char pathname[4096];
  GASDFPARMS parms ;

  /* initialize the udunits2 unit system */
  if (initUnitSys() == Failure) return Failure;
 
  /* allocate memory for the gafile structure */
  pfi = getpfi();
  if (pfi == NULL) {
    gaprnt (0,"sdfopen: Memory Allocation Error (getpfi failed)\n");
    return Failure;
  }
  getwrd(pathname, args, 4095) ;
  gaprnt (2, "Scanning self-describing file:  ");
  gaprnt (2, pathname);
  gaprnt (2, "\n");

  /* set a flag for detecting sdf file type: HDF or NetCDF */
  pfi->ncflg = 1;             /* NetCDF */
#if USEHDF==1
  rc = Hishdf(pathname);
  if (rc==1) pfi->ncflg = 2;  /* HDF-SDS */
#endif
  len=strlen(pathname);
  strncpy(pfi->name,pathname,len);
  strncpy(pfi->dnam,pathname,len);
  pfi->name[len]='\0';
  pfi->dnam[len]='\0';
  pfi->tmplat = 0 ;     /* no more templating with sdfopen */
  initparms(&parms) ;   /* set up for sdfopen calling gadsdf */

  rc = gadsdf(pfi, parms);
  if (rc==Failure) {
    frepfi (pfi, 0) ;
    freeparms (&parms);
    return Failure ;
  }
  if (pcm->pfi1==NULL) {
    pcm->pfi1 = pfi;
  } else {
    pfio = pcm->pfi1;
    while (pfio->pforw!=NULL) pfio = pfio->pforw;
    pfio->pforw = pfi;
  }
  pfi->pforw = NULL;
  pcm->fnum++;

  if (pcm->fnum==1) {
    pcm->pfid = pcm->pfi1; 
    pcm->dfnum = 1;
  }
  snprintf(pout,1255,"SDF file %s is open as file %i\n",pfi->name,pcm->fnum);
  gaprnt (2,pout);

  /* If first file open, set up some default dimension ranges for the user */
  if (pcm->fnum==1) {
    if (pfi->wrap) {
      gacmd ("set lon 0 360",pcm,0);
    }
    else {
      snprintf(pout,1255,"set x 1 %i",pfi->dnum[0]);
      gacmd (pout,pcm,0);
    }
    snprintf(pout,1255,"set y 1 %i",pfi->dnum[1]);
    gacmd (pout,pcm,0);
    gacmd ("set z 1",pcm,0);
    gacmd ("set t 1",pcm,0);
    gacmd ("set e 1",pcm,0);
  }
  if (pfi->pa2mb) {
    gaprnt(1,"Notice: Z coordinate pressure values have been converted from Pa to mb\n");
  }
  freeparms(&parms);
  return Success;
}

#endif  /* matches #ifndef STNDALN */


/* Open an XDF data set by parsing the descriptor and 
   reading the metadata to fill-in a gafile structure. 
   Chain the gafile structure on to the list anchored in the gastat.  */

gaint gaxdfopen (char *args, struct gacmn *pcm) {
  struct gafile *pfi, *pfio;
  gaint rc, idummy,len ;
  char pathname[256];
  char *fileone;
  struct dt tdefi;
  gaint flag;
  GASDFPARMS parms ;

  /* initialize the udunits2 unit system */
  if (initUnitSys() == Failure) return Failure;
 
  /* allocate memory for gafile structure */
  pfi = getpfi();
  if (pfi == NULL) {
    gaprnt (0,"xdfopen: Memory Allocation Error (getpfi failed)\n");
    return Failure;
  }
  getwrd(pathname, args, 255) ;
  gaprnt (2, "Scanning Descriptor File:  ");
  gaprnt (2, pathname);
  gaprnt (2, "\n");
  len = strlen(pathname);
  strncpy(pfi->dnam,pathname,len);
  pfi->dnam[len]='\0';
  pfi->tmplat = 0 ;     /* may be reset when parsing xdf descriptor */
  pfi->ncflg = 1;       /* may be reset when parsing dtype in descriptor */

  /* get metadata from the descriptor file */
  rc = gadxdf(pfi, &parms) ;
  if (rc==Failure) {
    freeparms(&parms);
    frepfi (pfi, 0) ;
    return Failure ;
  }
  if (pcm->pfi1==NULL) {
    pcm->pfi1 = pfi;
  } 
  else {
    pfio = pcm->pfi1;
    while (pfio->pforw!=NULL) pfio = pfio->pforw;
    pfio->pforw = pfi;
  }
  pfi->pforw = NULL;
  pcm->fnum++;
  if (pcm->fnum==1) {
    pcm->pfid = pcm->pfi1; 
    pcm->dfnum = 1;
  }

  /* see if the file format is HDF*/
#if USEHDF==1
  if (pfi->tmplat==0) {
    rc = Hishdf(pfi->name);
    if (rc==1) pfi->ncflg = 2;  /* HDF-SDS */
  }
  else {
    gr2t(pfi->grvals[3], 1.0, &tdefi);
    fileone = gafndt(pfi->name, &tdefi, &tdefi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,1,1,&flag);
    rc = Hishdf(fileone);
    if (rc==1) pfi->ncflg = 2;  /* HDF-SDS */
  }
#endif

  /* get remaining required metadata */
  rc = gadsdf(pfi, parms) ;
  if (rc==Failure) {
    snprintf(pout,1255, "SDF Descriptor file %s was not successfully opened & parsed.\n", pfi->dnam) ;
    gaprnt(0, pout) ;
    frepfi (pfi, 0) ;
    freeparms(&parms);
    if (pcm->fnum <= 1) { 
      pcm->pfid = pcm->pfi1 = (struct gafile *) 0 ;
      pcm->fnum  = 0 ; 
      pcm->dfnum = 0 ;
    } else {
      pcm->fnum-- ;
      for (idummy = 1, pfi = pcm->pfi1 ; (idummy < pcm->fnum) && pfi ; ++idummy) {
	pfi = pfi->pforw ;
      }
      if (pfi) {
	pfi->pforw = (struct gafile *) 0 ;
      }
    }
    pfi = (struct gafile *) 0 ;
    return Failure;
  } 
  else {
    snprintf(pout,1255, "SDF file %s is open as file %i\n", pfi->name, pcm->fnum);
    gaprnt(2, pout);
  }

  /* If SDF is first file open, set up some default dimension ranges for the user */
  if (pcm->fnum==1) {
    if (pfi->wrap) {
      gacmd ("set lon 0 360",pcm,0);
    }
    else {
      snprintf(pout,1255,"set x 1 %i",pfi->dnum[0]);
      gacmd (pout,pcm,0);
    }
    snprintf(pout,1255,"set y 1 %i",pfi->dnum[1]);
    gacmd (pout,pcm,0);
    gacmd ("set z 1",pcm,0);
    gacmd ("set t 1",pcm,0);
    gacmd ("set e 1",pcm,0);
  }
  if (pfi->pa2mb) {
    gaprnt(1,"Notice: Z coordinate pressure values have been converted from Pa to mb\n");
  }

  /* release memory */
  freeparms(&parms);

  return Success;
}

/* Initialize the udunit2 unit system */
gaint initUnitSys(void) {
  char *utname;
  gaint rc;

  if (!utSysInit) {
    utname = gxgnam("udunits2.xml") ;
    if (utname != NULL) {
      /* Suppress error messages */
      ut_set_error_message_handler(ut_ignore);

      /* Initialize the udunits2 unit-system */
      if ((utSys = ut_read_xml(utname)) == NULL) {
	gaprnt(0, "Error: UDUNITS2 package initialization failure.\n") ;
	return Failure ;
      }
      /* set up some known units that we'll check against in other subroutines */
      if ((second = ut_get_unit_by_name(utSys, "second")) == NULL) {
	gaprnt(0, "Error: The udunits library doesn't know 'second'\n") ;
	return Failure;
      }
      if ((feet = ut_get_unit_by_name(utSys, "feet")) == NULL) {
	gaprnt(0, "Error: The udunits library doesn't know 'feet'\n") ;
	return Failure;
      }
      if ((pascals = ut_get_unit_by_name(utSys, "pascals")) == NULL) {
	gaprnt(0, "Error: The udunits library doesn't know 'pascals'\n") ;
	return Failure;
      }
      if ((kelvins = ut_get_unit_by_name(utSys, "kelvins")) == NULL) {
	gaprnt(0, "Error: The udunits library doesn't know 'kelvins'\n") ;
	return Failure;
      }
      utSysInit = 1 ;
    }
  }
  return Success;

}



/* Retrieves self-describing file metadata and fills in the gafile structure. */

gaint gadsdf(struct gafile *pfi, GASDFPARMS parms) {
struct gavar *pvar,*newpvar,*savepvar;
struct gavar *Xcoord=NULL,*Ycoord=NULL,*Zcoord=NULL,*Tcoord=NULL,*Ecoord=NULL ;
struct gaattr *attr,*attr1,*attr2,*deltat_attr,*timeunits_attr;
struct sdfnames *varnames;
struct gaens *ens;
struct dt dt2,tdef,tdefi,tdefe;
gadouble v1,v2,*zvals,*tvals=NULL,*evals=NULL;
gadouble time1,time2,lat1,lat2,lev1,lev2,incrfactor,sf,sec,res,time1val;
gafloat dsec;
size_t len_time_units,trunc_point,sz ;
gaint len,noname,notinit,nolength;
gaint i,j,c,rc,flag,fwflg,numdvars,e,t,ckflg=0;
gaint iyr,imo,idy,ihr,imn,isec,ispress,isDatavar ;
gaint xdimid,ydimid,zdimid,tdimid,edimid ;
gaint istart,icount,havesf,haveao;
char *ch,*pos=NULL,*pos1=NULL,*pos2=NULL; 
char *time_units=NULL,*trunc_units=NULL,*temp_str ;
ut_unit *fromUnit=NULL,*toUnit=NULL;
cv_converter *converter=NULL;

  /* Enable griping, disable aborting, from within NetCDF library */
#if USENETCDF==1
  ncopts = NC_VERBOSE ;
#endif

  /* Grab the metadata */
  if (read_metadata(pfi) == Failure) {
    gaprnt(0, "Error: Couldn't ingest SDF metadata.\n") ;
    return Failure ;
  }
  if (!parms.isxdf) {
    pfi->calendar = 0 ;  /* 365 day kind not available under COARDS */
  }

  /* Get the title */
  if (parms.needtitle) {
    attr = NULL;
    attr = find_att("global", pfi->attr, "title") ;
    if (attr) {
      if ((attr->len) > 510) {
	getstr(pfi->title, (char *)(attr->value), 511) ;
      } else {
	getstr(pfi->title, (char *)(attr->value), attr->len) ;
      }
    }
  }

  /* Handle undef attribute */
  if (parms.needundef) {
    fwflg = 0;
    pfi->undefattrflg = 0;

    /* Check for user-defined undef attribute */
    attr = NULL;
    attr = find_att("ALL", pfi->attr, "missing_value") ;
    if (attr) {
      /* set the primary undef attribute flag and store the attribute name */
      pfi->undefattrflg = 1;
      len = strlen(attr->name);
      if ((pfi->undefattr = (char *)galloc(len+1,"undefattr1")) == NULL) goto err1;
      strncpy(pfi->undefattr, attr->name, len);
      *(pfi->undefattr+len) = '\0';

      /* If there is a single value of type NC_SHORT, NC_INT, NC_FLOAT or NC_DOUBLE, use it for file-wide */
      if (attr->len==1) {
	if (attr->nctype == 3) { pfi->undef = *(short*)(attr->value);    fwflg=1; }
	if (attr->nctype == 4) { pfi->undef = *(gaint*)(attr->value);    fwflg=1; }
	if (attr->nctype == 5) { pfi->undef = *(gafloat*)(attr->value);  fwflg=1; }
	if (attr->nctype == 6) { pfi->undef = *(gadouble*)(attr->value); fwflg=1; }
      }
    }

    /* Check for secondary (default) undef attribute */
    attr = NULL;
    attr = find_att("ALL", pfi->attr, "_FillValue") ;
    if (attr) {
      if (pfi->undefattrflg==1) {
        /* uptick the undef attribute flag and store the second attribute name */
        pfi->undefattrflg = 2;
        len = strlen(attr->name);
        if ((pfi->undefattr2 = (char *)galloc(len+1,"undefattr2")) == NULL) goto err1;
        strncpy(pfi->undefattr2, attr->name, len);
        *(pfi->undefattr2+len) = '\0';
      } 
      else {
        /* set the primary undef attribute flag and store the attribute name */
        pfi->undefattrflg = 1;
        len = strlen(attr->name);
        if ((pfi->undefattr = (char *)galloc(len+1,"undefattr1")) == NULL) goto err1;
        strncpy(pfi->undefattr, attr->name, len);
        *(pfi->undefattr+len) = '\0';
      }
      if (!fwflg) { 
        /* If we haven't got one already, use this value for file-wide undef */
        if (attr->len==1) {
 	  if (attr->nctype == 3) { pfi->undef = *(short*)(attr->value);    fwflg=1; }
          if (attr->nctype == 4) { pfi->undef = *(gaint*)(attr->value);    fwflg=1; }
	  if (attr->nctype == 5) { pfi->undef = *(gafloat*)(attr->value);  fwflg=1; }
	  if (attr->nctype == 6) { pfi->undef = *(gadouble*)(attr->value); fwflg=1; }
        }
      }
    }
    /* Set a default file-wide undef if we don't have one already */
    if (!fwflg) pfi->undef = 9.96921e+36;  /* use default for NC_FLOAT */
  }

  /* Look for scale factor or slope attribute */
  if (parms.needunpack) {
    havesf = 0; 
    haveao = 0;
    attr = NULL;
    attr = find_att("ALL", pfi->attr, "scale_factor") ;
    if (!attr) {
      attr = find_att("ALL", pfi->attr, "slope") ;
    }
    if (attr) {
      len = strlen(attr->name);
      sz = len+1;
      if ((pfi->scattr = (char *)galloc(sz,"scattr1")) == NULL) goto err1;
      strncpy(pfi->scattr,attr->name,len);
      *(pfi->scattr+len) = '\0';
      havesf = 1;
      if (!strncmp(pfi->scattr, "NULL", 4) || !strncmp(pfi->scattr, "null", 4)) havesf = 0;
    }
    /* Look for add offset or intercept attribute */
    attr = NULL;
    attr = find_att("ALL", pfi->attr, "add_offset") ;
    if (!attr) {
      attr = find_att("ALL", pfi->attr, "intercept") ;
    }
    if (attr) {
      len = strlen(attr->name);
      sz = len+1;
      if ((pfi->ofattr = (char *)galloc(sz,"ofattr1")) == NULL) goto err1;
      strncpy(pfi->ofattr,attr->name,len);
      *(pfi->ofattr+len) = '\0';
      haveao = 1;
      if (!strncmp(pfi->ofattr, "NULL", 4) || !strncmp(pfi->ofattr, "null", 4)) haveao = 0;
    }
    /* set the packflg */
    if (havesf) {
      pfi->packflg = haveao == 1 ? 2 : 1 ;
    } else {
      pfi->packflg = haveao == 1 ? 3 : 0 ;
    }
  }

  /* Set up the X Coordinate */
  if (parms.xsetup) {
    if (parms.xsrch) {   
    /* find an X axis */
      rc = findX(pfi, &Xcoord);
      if ((rc==Failure) || (Xcoord == NULL)) {
	gaprnt(0, "Error: SDF file has no discernable X coordinate.\n") ;
	gaprnt(0,"  To open this file with GrADS, use a descriptor file with an XDEF entry.\n");
	gaprnt(0,"  Documentation is at http://cola.gmu.edu/grads/gadoc/SDFdescriptorfile.html\n"); 
	return Failure ;
      }
    }
    else {    
      /* find the axis named in the descriptor file */
      Xcoord = find_var(pfi, parms.xdimname) ;
      if (!Xcoord) {
	snprintf(pout,1255, "Error: Can't find variable %s for X coordinate.\n",parms.xdimname);
	gaprnt(0,pout);
	return Failure ;
      }
    } 
    /* set the dimension size */
    for (i=0;i<pfi->nsdfdims;i++) {
      if (pfi->sdfdimids[i] == Xcoord->vardimids[0]) {
	pfi->dnum[XINDEX] = pfi->sdfdimsiz[i];
	break;
      }
    }

    /* set the axis values */
    if ((sdfdeflev(pfi, Xcoord, XINDEX, 0)) == Failure)  {
      gaprnt(0, "Error: Failed to define X coordinate values.\n") ;
      return Failure;
    }

  }
  /* set the xdimid */
  if (parms.isxdf && (!parms.xsrch)) {
    xdimid = find_dim(pfi, parms.xdimname) ;
    if (xdimid == -1) {
      snprintf(pout,1255, "Error: Lon dimension %s is not an SDF dimension.\n",parms.xdimname);
      gaprnt(0,pout);
      return Failure;
    }
  } else {
    if (Xcoord) {
      xdimid = find_dim(pfi, Xcoord->longnm) ;
    } else {
      xdimid = -1 ;
    }
  }

  /* Set up the Y coordinate */
  if (parms.ysetup) {
    /* find a Y axis */
    if (parms.ysrch) {
      rc=0;
      rc = findY(pfi, &Ycoord);
      if ((rc==Failure) || (Ycoord == NULL)) {
	gaprnt(0, "Error: SDF file has no discernable Y coordinate.\n") ;
	gaprnt(0,"  To open this file with GrADS, use a descriptor file with a YDEF entry.\n");
	gaprnt(0,"  Documentation is at http://cola.gmu.edu/grads/gadoc/SDFdescriptorfile.html\n"); 
	return Failure ;
      }
    }
    else {
      /* find the axis named in the descriptor file */
      Ycoord = find_var(pfi, parms.ydimname) ;
      if (!Ycoord) {
	snprintf(pout,1255, "Error: Can't find variable %s for Y coordinate.\n",parms.ydimname);
	gaprnt(0,pout);
	return Failure ;
      }
    } 
    /* set the dimension size */
    for (i=0;i<pfi->nsdfdims;i++) {
      if (pfi->sdfdimids[i] == Ycoord->vardimids[0]) {
	pfi->dnum[YINDEX] = pfi->sdfdimsiz[i];
	break;
      }
    }

    /* Read first two values to deduce YREV flag */
    if (pfi->dnum[YINDEX] > 1)  {
      istart = 0;
      icount = 1; 
      if (read_one_dimension(pfi, Ycoord, istart, icount, &lat1) == Failure) {
	gaprnt(0, "Error: Unable to read first latitude value in SDF file.\n") ;
	return Failure;
      }
      istart = 1;
      icount = 1;
      if (read_one_dimension(pfi, Ycoord, istart, icount, &lat2) == Failure) {
	gaprnt(0, "Error: Unable to read second latitude value in SDF file.\n") ;
	return Failure;
      }
      /* Set yrev flag */
      if (lat2 < lat1) pfi->yrflg = 1 ;
    } 
    
    /* Read the axis values */
    if ((sdfdeflev(pfi, Ycoord, YINDEX, pfi->yrflg)) == Failure)  {
      gaprnt(0, "Error: Failed to define Y coordinate values.\n") ;
      return Failure;
    }

  }
  /* set the ydimid */
  if (parms.isxdf && (!parms.ysrch)) {
    ydimid = find_dim(pfi, parms.ydimname) ;
    if (ydimid == -1) {
      snprintf(pout,1255,"Error: Lat dimension %s is not an SDF dimension.\n",parms.ydimname);
      gaprnt(0,pout);
      return Failure;
    }
  } 
  else {
    if (Ycoord) {
      ydimid = find_dim(pfi, Ycoord->longnm) ;
    } else {
      ydimid = -1 ;
    }
  }

  /* Set up the Z coordinate */
  if (parms.zsetup) {
    /* find a Z axis */
    if (parms.zsrch) {
      (void) findZ(pfi, &Zcoord, &ispress);
    }
    else {
      /* find the axis named in the descriptor file */
      Zcoord = find_var(pfi, parms.zdimname) ;
      if (!Zcoord) {
	snprintf(pout,1255,"Error: Can't find variable %s for Z coordinate.\n",parms.zdimname);
	gaprnt(0,pout);
	return Failure ;
      }
    }
    if (!Zcoord) {
      /* set up a dummy zaxis; the equivalent of "zdef 1 linear 0 1" */
      pfi->dnum[ZINDEX] = 1 ;
      sz = sizeof(gadouble)*6;
      if ((zvals = (gadouble *)galloc(sz,"zvals")) == NULL) {
	gaprnt(0,"Error: Unable to allocate memory for dummy Z coordinate axis values\n");
	goto err1;
      }
      *(zvals) = 1.0;
      *(zvals+1) = 0.0 - 1.0;
      *(zvals+2) = -999.9;
      pfi->grvals[ZINDEX] = zvals;
      *(zvals+3) = 1.0;
      *(zvals+4) = 1.0;
      *(zvals+5) = -999.9;
      pfi->abvals[ZINDEX] = zvals+3;
      pfi->ab2gr[ZINDEX] = liconv;
      pfi->gr2ab[ZINDEX] = liconv;
      pfi->linear[ZINDEX] = 1;
    }
    else {
      /* set the dimension size */
      for (i=0;i<pfi->nsdfdims;i++) {
	if (pfi->sdfdimids[i] == Zcoord->vardimids[0]) {
	  pfi->dnum[ZINDEX] = pfi->sdfdimsiz[i];
	  break;
	}
      }
      /* Read first two values to deduce ZREV flag */
      if (pfi->dnum[ZINDEX] > 1)  {
	istart = 0;
	icount = 1; 
	if (read_one_dimension(pfi, Zcoord, istart, icount, &lev1) == Failure) {
	  gaprnt(0, "Error: Unable to read first Zcoord value in SDF file.\n") ;
	  return Failure;
	}
	istart = 1;
	icount = 1;
	if (read_one_dimension(pfi, Zcoord, istart, icount, &lev2) == Failure) {
	  gaprnt(0, "Error: Unable to read second Zcoord value in SDF file.\n") ;
	  return Failure;
	}
	/* Set zrev flag */
	attr = NULL;
	attr = find_att(Zcoord->longnm, pfi->attr, "positive") ;
	if (attr != NULL) {
	  if (!strncmp("down", (char *)attr->value, 4)) {
	    if (lev2 > lev1) pfi->zrflg = 1 ;	    /* positive:down */
	  } 
	  else {
	    if (lev2 < lev1) pfi->zrflg = 1 ;        /* positive:up */
	  }
	}
	else if (ispress) {
	  if (lev2 > lev1) pfi->zrflg = 1 ;  /* pressure is always positive down */
	} else {
	  if (lev2 < lev1) pfi->zrflg = 1 ;  /* default is positive up */
	}
      } 
      /* Read the axis values */
      if ((sdfdeflev(pfi, Zcoord, ZINDEX, pfi->zrflg)) == Failure)  {
	gaprnt(0, "Error: Failed to define Z coordinate values.\n") ;
	return Failure;
      }
    }
  }
  /* set the zdimid */
  if (parms.isxdf && (!parms.zsrch)) {
    zdimid = find_dim(pfi, parms.zdimname) ;
    if (zdimid == -1) {
      snprintf(pout,1255, "Error: Lev dimension %s is not an SDF dimension.\n",parms.zdimname);
      gaprnt(0,pout);
      return Failure;
    }
  } else {
    if (Zcoord) {
      zdimid = find_dim(pfi, Zcoord->longnm) ;
    } else {
      zdimid = -1 ;
    }
  }

  /* Set up the T coordinate */
  if (parms.tsetup) {
    if (parms.tsrch) {
      /* find a T axis */
      (void) findT(pfi, &Tcoord);
    }
    else {
      /* find the axis named in the descriptor file */
      Tcoord = find_var(pfi, parms.tdimname) ;
      if (!Tcoord) {
	snprintf(pout,1255, "Error: Can't find variable %s for T coordinate.\n",parms.tdimname);
	gaprnt(0,pout);
	return Failure ;
      }
    }
    if (!Tcoord) {
      /* initialize a dummy time coordinate */
      pfi->dnum[TINDEX] = 1 ;
      sz = sizeof(gadouble)*8;
      if ((tvals = (gadouble *) galloc(sz,"tvals1")) == NULL) {
	gaprnt(0, "Error: memory allocation failed for dummy time coordinate info.\n") ;
	return Failure ;
      }
      tvals[0] = 1.0 ; /* initial year */
      tvals[1] = 1.0 ; /* initial month */
      tvals[2] = 1.0 ; /* initial day */
      tvals[3] = 0.0 ; /* initial hour */
      tvals[4] = 0.0 ; /* initial minutes */
      tvals[5] = 0.0 ; /* step in months */
      tvals[6] = 1.0 ; /* step in minutes */
      tvals[7] = -999.9 ;
      pfi->grvals[TINDEX] = tvals ;
      pfi->abvals[TINDEX] = tvals ;
      pfi->linear[TINDEX] = 1 ;
      gaprnt(2, "SDF file has no discernable time coordinate -- using default values.\n") ;
    }
    else {
      /* make sure it's not a 360- or 365-day calendar */
      attr = NULL;
      attr = find_att(Tcoord->longnm, pfi->attr, "calendar") ;
      if (attr) {
        if (!strncasecmp((char *)attr->value,"360_day", 7)) {
	  gaprnt(0,"SDF Error: 360 day calendars are not supported by sdfopen.\n"); 
	  return Failure;
	}
	if ((!strncasecmp((char *)attr->value,"cal365",      6)) ||
	    (!strncasecmp((char *)attr->value,"altcal365",   9)) ||
	    (!strncasecmp((char *)attr->value,"common_year",11)) ||
	    (!strncasecmp((char *)attr->value,"365_day",     7)) ||
	    (!strncasecmp((char *)attr->value,"noleap",      6))) {
	  
	  gaprnt(0,"Error: 365 day calendars are not supported by sdfopen.\n"); 
	  gaprnt(0,"  To open this file with GrADS, use a descriptor file with \n");
	  gaprnt(0,"  a complete TDEF entry and OPTIONS 365_day_calendar. \n");
	  gaprnt(0,"  Documentation is at http://cola.gmu.edu/grads/gadoc/SDFdescriptorfile.html\n"); 
	  return Failure;
	}
      }
      /* set dimension size */
      for (i=0;i<pfi->nsdfdims;i++) {
	if (pfi->sdfdimids[i] == Tcoord->vardimids[0]) {
	  pfi->dnum[TINDEX] = pfi->sdfdimsiz[i];
	  break;
	}
      }
      /* Set time axis values */
      /* Get the units attribute */
      timeunits_attr = NULL;
      timeunits_attr = find_att(Tcoord->longnm, pfi->attr, "units") ;
      if (!timeunits_attr) {
	gaprnt(0, "Error: Couldn't find units attribute for Time coordinate.\n") ;
	return Failure;
      }
      /* Read first two values to deduce time increment */
      istart = 0;
      icount = 1; 
      if (read_one_dimension(pfi, Tcoord, istart, icount, &time1) == Failure) {
	gaprnt(0, "Error: Unable to read first time value in SDF file.\n") ;
	return Failure;
      }
      if (pfi->dnum[TINDEX] > 1)  {
	istart = 1;
	icount = 1;
	if (read_one_dimension(pfi, Tcoord, istart, icount, &time2) == Failure) {
	  gaprnt(0, "Error: Unable to read second time values in SDF file.\n") ;
	  return Failure;
	}
      } 
      else {
	time2 = time1;
      }
      sz = sizeof(gadouble)*8;
      if ((tvals = (gadouble *) galloc(sz,"tvals2")) == NULL) {
	gaprnt(0,"Error: Unable to allocate memory for time coordinate in SDF file.\n") ;
	return Failure; 
      }

      /* Handle YYMMDDHH time */
      if ((timeunits_attr->nctype==1) && 
	  (timeunits_attr->len < 10) &&  
	  (!strncmp((char *)timeunits_attr->value, "YYMMDDHH", 8))) {
	tvals[0] = (((gaint) time1) / 1000000) ;
	tvals[1] = ((((gaint) time1) / 10000) % 100) ;
	tvals[2] = ((((gaint) time1) / 100) % 100) ;
	tvals[3] = (((gaint) time1) % 100) ;
	tvals[4] =  0.0 ;
	/* If more than one time step, deduce increment */ 
	if (pfi->dnum[TINDEX] > 1) {
	  dt2.yr = (((gaint) time2) / 1000000) - (((gaint) time1) / 1000000) ;
	  dt2.mo = ((((gaint) time2) / 10000) % 100) - ((((gaint) time1) / 10000) % 100) ;
	  dt2.dy = ((((gaint) time2) / 100) % 100) - ((((gaint) time1) / 100) % 100) ;
	  dt2.hr = (((gaint) time2) % 100) - (((gaint) time1) % 100) ;
	  dt2.mn = 0 ;
	  if ((dt2.yr > 0) || (dt2.mo > 0)) {
	    tvals[5] = (dt2.yr * 12.0) + dt2.mo ;
	    tvals[6] = 0.0 ;
	  } else {
	    tvals[5] = 0.0 ;
	    tvals[6] = (dt2.dy * 1440.0) + (dt2.hr * 60.0) + dt2.mn ;
	    if (tvals[6] < 1.0) {
	      gaprnt(0, "Error: Time unit has too small an increment (min. 1 minute).\n") ;
	      goto err2;
	    }
	  }
	} 
	else  {
	  /* only one time step*/
	  tvals[5] = 0.0 ;
	  tvals[6] = 1.0 ; /* one time-step files get a (meaningless) delta t of one minute */
	}
      } 
      /* Handle YYYYMMDDHHMMSS time */
      else if (pfi->time_type == CDC) {
	if (!decode_standard_time(time1, &iyr, &imo, &idy, &ihr, &imn, &dsec)) {
	  gaprnt(0, "Error: Unable to decipher initial time value in SDF file.\n") ;
	  goto err2;
	}
	if (iyr <= 0) iyr = 1 ;
	if (imo <= 0) imo = 1 ;
	if (idy <= 0) idy = 1 ;
	tvals[0] = iyr ;
	tvals[1] = imo ;
	tvals[2] = idy ;
	tvals[3] = ihr ;
	tvals[4] = imn ;
	/* If more than one time step, deduce increment */ 
	if (pfi->dnum[TINDEX] > 1) {
	  /* For YYYYMMDDHHMMSS time type, delta_t attribute must be present. */
	  deltat_attr = NULL;
	  deltat_attr = find_att(Tcoord->longnm, pfi->attr, "delta_t") ;
	  if (!deltat_attr) {
	    gaprnt(0, "Error: Unable to find 'delta_t' attribute in SDF file.\n") ;
	    goto err2;
	  }
	  ch = (char *) deltat_attr->value ;
	  if (!decode_delta_t(ch, &dt2.yr, &dt2.mo, &dt2.dy, &dt2.hr, &dt2.mn, &isec)) {
	    gaprnt(0, "Error: Unable to decipher 'delta_t' attribute in SDF file.\n") ;
	    goto err2;
	  }
	  if ((dt2.yr > 0) || (dt2.mo > 0)) {
	    tvals[5] = (dt2.yr * 12.0) + dt2.mo ;
	    tvals[6] = 0.0 ;
	  } else {
	    tvals[5] = 0.0 ;
	    tvals[6] = (dt2.dy * 1440.0) + (dt2.hr * 60.0) + dt2.mn ;
	    if (tvals[6] < 1.0) {
	      gaprnt(0, "Error: Time increment 'delta_t' is too small (min. 1 minute).\n") ;
	      goto err2;
	    }
	  }
	} 
	else {
	  /* only one time step*/
	  tvals[5] = 0.0 ;
	  tvals[6] = 1.0 ; /* one time-step files get a (meaningless) delta t of one minute */
	}
      } 
      /* Handle all other udunits-compatible times */ 
      else {
	/* check if units field has an origin */  
	if (!strstr((char *)timeunits_attr->value, " since ")) {    
	  /* no origin, use a default */
	  len_time_units = strlen((char *)timeunits_attr->value) + strlen(DFLTORIGIN) + 1 ;
	  sz = len_time_units;
	  time_units = (char *) galloc(sz,"time_un1") ;
	  if (time_units==NULL) {
	    gaprnt(0, "Error: Memory allocation error for time_units\n") ;
	    goto err2;
	  }
	  strcpy(time_units, (char *) timeunits_attr->value);
	  strcat(time_units, DFLTORIGIN);
	}
	else {
	  len_time_units = strlen((char *)timeunits_attr->value) + 1;
	  sz = len_time_units;
	  time_units = (char *) galloc(sz,"time_un2") ;
	  if (time_units==NULL) {
	    gaprnt(0, "Error: Memory allocation error for time_units\n");
	    goto err2;
	  }
	  strcpy(time_units, (char *) timeunits_attr->value);
	} 
	/* convert unit string to a udunits format */
	fromUnit = ut_parse(utSys, time_units, UT_ASCII);
        if (fromUnit == NULL) {
	  snprintf(pout,1255, "Error: Unable to parse time units (%s) from SDF file.\n",time_units) ;
	  gaprnt(0,pout);
	  goto err2;
	}

	/* convert udunits-formatted time to integer values for yr, mo, etc. */
	ckflg = 0;
	toUnit = ut_offset_by_time(second, ut_encode_time(2001, 1, 1, 0, 0, 0.0));
	if (toUnit != NULL) { 
	  /* the check for campatibility was also done in findT */
	  if (ut_are_convertible(fromUnit, toUnit)) { 
	    /* convert the unit to 'seconds since 2001-1-1 00:00:0.0' */
	    converter = ut_get_converter(fromUnit, toUnit);
	    if (converter != NULL) {
	      time1val = cv_convert_double(converter,time1); 
	      /* decode the converted value to integer values for yr, mo, dy, et al. */
	      ut_decode_time(time1val, &iyr, &imo, &idy, &ihr, &imn, &sec, &res);
	      cv_free(converter);
	      ckflg = 1;
	    }
	  }
	}
	if (!ckflg) {
	  gaprnt(0,"Error: Unable to decode initial time value in SDF file.\n") ;
	  goto err2;
	}

	if (imo == 0) imo = 1 ;
	if (idy == 0) idy = 1 ;
	tvals[0] = iyr ;
	tvals[1] = imo ;
	tvals[2] = idy ;
	tvals[3] = ihr ;
	tvals[4] = imn ;

	/* If more than one time step, deduce increment */
	if (pfi->dnum[TINDEX] > 1) {
	  temp_str = strstr(time_units, " since ") ;
	  if (!temp_str) {
	    trunc_point = strlen(time_units) ;
	  } else {
	    trunc_point = strlen(time_units)-strlen(temp_str)+1;
	  }
	  sz = trunc_point+1;
          trunc_units = (char *) galloc(sz,"trunc_units");
	  if (trunc_units==NULL) {
	    gaprnt(0,"Error: Memory Allocation Error for trunc_units\n");
	    goto err2;
	  }
	  strncpy(trunc_units, time_units, trunc_point) ;
	  trunc_units[trunc_point] = '\0' ;
	  istart = 1 ;
	  incrfactor = time2 - time1 ;

	  if (compare_units("year", trunc_units) == Success) {
	    tvals[5] = 12.0 * incrfactor;
	    if (tvals[5] < 1.0) {
	      snprintf(pout,1255,"Error: Yearly time unit has too small an increment: %g (%g months)\n",incrfactor,tvals[5]) ;
	      gaprnt(0, pout);
	      goto err2;
	    }
	    tvals[6] = 0.0 ;
	  } 
	  else {
	    /* COARDS conventions say only year, day, hour, and minute are OK, not month */
	    /* But I've accepted Camiel's patch for "months since ..."  -Hoop 2K/07/25 */
	    tvals[5] = 0.0 ;
	    if ((!strncmp(time_units, "month", 5)) ||
		(!strncmp(time_units, "common_year/12", 14)) ||
		(!strncmp(time_units, "common_years/12", 15))) {
	      tvals[5] = incrfactor;
	      tvals[6] = 0.0 ;
	      if (tvals[5] < 1.0) {
		gaprnt(0, "Error: Fractional months are ill-defined and not supported by GrADS\n") ;
		goto err2;
	      }
	    } 
	    else if (compare_units("day", trunc_units) == Success) {
	      if (incrfactor < 28.0) {
		tvals[6] = incrfactor * 24.0 * 60.0 ;  /* convert units from days to minutes */
		/* round this to the nearest minute */
		tvals[6]=floor(tvals[6]+0.5);
		if (tvals[6] < 1.0) {
		  snprintf(pout,1255,"Error: Daily time unit has too small an increment %g (%g minutes)\n",incrfactor,tvals[6]);
		  gaprnt(0, pout);
		  goto err2;
		}
	      } 
	      else if (incrfactor < 360.0) /* assume really months */ {
		/* This dirty trick should get the right number of months for monthly, */
		/* bi-monthly, and seasonal data.  If there's anything between that and */
		/* annual data (which should have units of "year(s) since"), we're broken */
		tvals[5] = ((gaint) (incrfactor + 0.5)) / 28;
		tvals[6] = 0.0 ;
		if (tvals[5] < 1.0) {
		  snprintf(pout,1255,"Error: Daily time unit has too small an increment %g (%g months)\n",incrfactor,tvals[5]);
		  gaprnt(0, pout);
		  goto err2;
		}
	      } 
	      else /* annual or multi-annual w/"days since" */{
		/* also a dirty trick to figure out how many years & mult. by 12 */
		tvals[5] = 12.0 * ((gadouble)(((gaint) (incrfactor + 0.5))/360)) ;
		tvals[6] = 0.0 ;
	      }
	    } 
	    else if (compare_units("hour", trunc_units) == Success) {
	      if (incrfactor < (28.0 * 24.0)) {
		tvals[6] = incrfactor * 60.0 ;
		if (tvals[6] < 1.0) {
		  snprintf(pout,1255,"Error: Hourly time unit has too small an increment %g (%g minutes)\n",incrfactor,tvals[6]);
		  gaprnt(0, pout);
		  goto err2;
		}
	      } 
	      else  {
		if (incrfactor >= (360 * 24)) /* try years? */{
		  tvals[5] = 12.0 * ((gadouble) ((gaint)
						  (((gaint)(incrfactor + 0.5)) / (360.0 * 24.0)))) ;
		  tvals[6] = 0.0 ;
		} 
		else /* assume really months */ {
		  tvals[5] = ((gaint) (incrfactor + 0.5)) / (28 * 24);
		  tvals[6] = 0.0 ;
		}
		if (tvals[5] < 1.0) {
		  snprintf(pout,1255,"Error: Hourly time unit has too small an increment %g (%g months)\n",incrfactor,tvals[5]);
		  gaprnt(0, pout);
		  goto err2;
		}
	      }
	    } 
	    else if (compare_units("minute", trunc_units) == Success) {
	      if (incrfactor < (60.0 * 24.0 * 28.0)) {
		tvals[5] = 0.0 ;
		tvals[6] = incrfactor ;
		if (tvals[6] < 1.0) {
		  snprintf(pout,1255,"Error: Minutes time unit has too small an increment: %g (must be >= 1)\n",incrfactor) ;
		  gaprnt(0,pout);
		  goto err2;
		}
	      } 
	      else /* monthly or greater */ {
		if (incrfactor < (60.0 * 24.0 * 360.0)) {
		  tvals[5] = ((gaint) ((incrfactor / (60.0 * 24.0)) + 0.5)) / 28;
		  tvals[6] = 0.0 ;
		} 
		else {
		  snprintf(pout,1255,"Error: Time increment %g is too large for 'minutes since' time units attribute\n",incrfactor);
		  gaprnt(0,pout);
		  goto err2;
		}
	      }
	    } 
	    else if (compare_units("seconds", trunc_units) == Success) {
	      if (incrfactor < 60.0) {
		snprintf(pout,1255, "Error: Time increment %g is too small for 'seconds since' time units attribute (must be >= 60)\n",incrfactor) ;
		gaprnt(0, pout);
		goto err2;
	      } 
	      else {
		if (incrfactor < (60.0 * 60.0 * 24.0 * 28.0)) {
		  /* less than monthly, so use tvals[6] */
		  tvals[6] = incrfactor / 60.0 ;
		} 
		else /* monthly or greater */ {
		  if (incrfactor < (60.0 * 60.0 * 24.0 * 360)) {
		    /* assume monthly */
		    tvals[5] = ((gaint) ((incrfactor/(60.0 * 60.0 * 24.0)) + 0.5)) / 28;
		    tvals[6] = 0.0 ;
		  } 
		  else {
		    snprintf(pout,1255,"Error: Time increment %g is too large for 'seconds since' time units attribute\n",incrfactor);
		    gaprnt(0,pout);
		    goto err2;
		  }
		}
	      }
	    } 
	    else {
	      gaprnt(0,"Error: Unable to parse time units in SDF file.\n") ;
	      goto err2;
	    }
	  } /* finer than years resolution */
	}
	else {
	  /* only one time step */
	  tvals[5] = 0.0 ;
	  tvals[6] = 1.0 ; /* one time-step files get a (meaningless) delta t of one minute */
	} 
	if (time_units)  gree(time_units,"f1");
	if (trunc_units) gree(trunc_units,"f2") ;
      } /* end if udunits time */

      /* Set scaling values */
      tvals[7] = -999.9 ;
      pfi->grvals[TINDEX] = tvals ;
      pfi->abvals[TINDEX] = tvals ;
      pfi->linear[TINDEX] = 1 ;

    } /* has a T coordinate */
  } /* doing T setup */

  /* Set the tdimid */
  if (parms.isxdf && (!parms.tsrch)) {
    if (parms.tdimname != NULL) {
      tdimid = find_dim(pfi, parms.tdimname) ;
      if (tdimid == -1) {
	snprintf(pout,1255, "Error: Time dimension %s is not an SDF dimension.\n",parms.tdimname);
	gaprnt(0,pout);
	return Failure;
      }
    }
    else tdimid = -1;  /* this is for the %nodim% option in TDEF */
  }
  else {
    if (Tcoord) {
      tdimid = find_dim(pfi, Tcoord->longnm) ;
    } else {
      tdimid = -1 ;
    }
  }
  
   /* Set up the E coordinate */
  if (parms.esetup) {
    if (parms.esrch) {   
    /* find an E axis */
      (void) findE(pfi, &Ecoord);
    }
    else {    
      /* find the axis named in the descriptor file */
      Ecoord = find_var(pfi, parms.edimname) ;
      if (!Ecoord) {
	snprintf(pout,1255,"Error: Can't find variable %s for Ensemble coordinate.\n",parms.edimname);
	gaprnt(0,pout);
	return Failure ;
      }
    } 
    if (!Ecoord) {  /* no ensemble dimension found */
      if (parms.esetup != 4 && pfi->ens1==NULL ) { /* the dummy E axis was not set up in gadxdf */
	/* set up the default values */
	pfi->dnum[EINDEX] = 1;
	/* set up linear scaling */
	sz = sizeof(gadouble)*6;
	if ((evals = (gadouble *)galloc(sz,"evals")) == NULL) {
	  gaprnt(0,"Error: memory allocation failed for default ensemble dimension scaling values\n");
	  goto err1;
	}
	v1=v2=1;
	*(evals+1) = v1 - v2;
	*(evals) = v2;
	*(evals+2) = -999.9;
	*(evals+4) = -1.0 * ( (v1-v2)/v2 );
	*(evals+3) = 1.0/v2;
	*(evals+5) = -999.9;
	pfi->grvals[EINDEX] = evals;
	pfi->abvals[EINDEX] = evals+3;
	pfi->ab2gr[EINDEX] = liconv;
	pfi->gr2ab[EINDEX] = liconv;
	pfi->linear[EINDEX] = 1;
	/* allocate a single ensemble structure */
	sz = sizeof(struct gaens);
	if ((ens = (struct gaens *)galloc(sz,"ens1")) == NULL) {
	  gaprnt(0,"Error: memory allocation failed for default E axis values\n");
	  goto err1;
	}
	pfi->ens1 = ens;
	snprintf(ens->name,15,"1");
	ens->length = pfi->dnum[TINDEX];
	ens->gt = 1;
	gr2t(pfi->grvals[TINDEX],1.0,&ens->tinit);
	/* set grib codes to default values */
	for (j=0;j<4;j++) ens->grbcode[j]=-999;
      }
    }
    else { 
      /* We have a dimension */  
      if (parms.esetup == 3) {   /* we still need size, ensemble names, time metadata */
	/* set the dimension size */
	for (i=0;i<pfi->nsdfdims;i++) {
	  if (pfi->sdfdimids[i] == Ecoord->vardimids[0]) {
	    pfi->dnum[EINDEX] = pfi->sdfdimsiz[i];
	    break;
	  }
	}
	/* set up linear scaling */
	sz = sizeof(gadouble)*6;
	if ((evals = (gadouble *)galloc(sz,"evals1")) == NULL) {
	  gaprnt(0,"Error: memory allocation failed for ensemble dimension scaling values\n");
	  goto err1;
	}
	v1=v2=1;
	*(evals+1) = v1 - v2;
	*(evals) = v2;
	*(evals+2) = -999.9;
	*(evals+4) = -1.0 * ( (v1-v2)/v2 );
	*(evals+3) = 1.0/v2;
	*(evals+5) = -999.9;
	pfi->grvals[EINDEX] = evals;
	pfi->abvals[EINDEX] = evals+3;
	pfi->ab2gr[EINDEX] = liconv;
	pfi->gr2ab[EINDEX] = liconv;
	pfi->linear[EINDEX] = 1;
       
	/* allocate an array of ensemble structures */
	sz = pfi->dnum[EINDEX] * sizeof(struct gaens); 
	if ((ens = (struct gaens *)galloc(sz,"ens2")) == NULL) {
	  gaprnt(0,"Error: memory allocation failed for E axis values\n");
	  goto err1;
	}
	pfi->ens1 = ens;
      }
      if (parms.esetup >= 2) {   /* still need ensemble names, time metadata */
	/* first see if there is an attribute containing ensemble names in the file */
	noname=1;
	if (pfi->ncflg==2) {
	  gaprnt(0,"Contact the GrADS developers if you have an HDF file with ensemble metadata\n");
	  return Failure;
	} 
	else {
	  attr = NULL;
	  attr = find_att("ens", pfi->attr, "grads_name");
	  if (attr) {
	    noname=0;
	    pos = (char*)attr->value;  /* set the pointer to the beginning of the string */
	  }
	}
	/* loop through array of ensemble structures, assigning names */
	ens = pfi->ens1;
	i=1;
	while (i<=pfi->dnum[EINDEX]) {
	  if (noname) {
	    snprintf(ens->name,15,"%d",i);   /* default to ensemble index number for a name */
	  }
	  else {
	    /* get the ensemble name */
	    len=0;
	    while (len<16 && *pos!=',' ) {
	      ens->name[len] = *pos;
	      len++; 
	      pos++;
	    }
	    ens->name[len] = '\0';
	    pos++;  /* advance past the comma */
	  }
	  i++; ens++;
	}
      }

      /* Get the time metadata for each ensemble */
      /* Look for attributes containing lengths and initial time indices */
      nolength=1;
      attr1 = NULL;
      attr1 = find_att("ens", pfi->attr, "grads_length");
      if (attr1) {
	nolength=0;
	pos1 = (char*)attr1->value;
      }
      notinit=1;
      attr2 = NULL;
      attr2 = find_att("ens", pfi->attr, "grads_tinit");
      if (attr2) {
	notinit=0;
	pos2 = (char*)attr2->value;
      }
      /* loop through array of ensemble structures, assigning lengths and initial times */
      ens = pfi->ens1;
      i=1;
      while (i<=pfi->dnum[EINDEX]) {
	/* assign length and start time index of each ensemble member */
	if (nolength) {
	  ens->length = pfi->dnum[3];     /* default to length of time axis */
	} else {
	  /* get the ensemble length */
	  pos1 = intprs(pos1,&ens->length);
	  pos1++;  /* advance past the comma */
	}
	if (notinit) {
	  ens->gt = 1;                     /* default to start of time axis */
	} else {
	  pos2 = intprs(pos2,&ens->gt);
	  pos2++;  /* advance past the comma */
	}
	/* populate the tinit structure for each ensemble */
	gr2t(pfi->grvals[TINDEX],(gadouble)ens->gt,&ens->tinit);
	/* set grib codes to default values */
	for (j=0;j<4;j++) ens->grbcode[j]=-999;  
	i++; ens++;

      }
    }
  }

  /* set the edimid */
  if (parms.isxdf && (!parms.esrch)) {
    edimid = find_dim(pfi, parms.edimname) ;
    if (edimid == -1) {
      snprintf(pout,1255,"Error: Ensemble dimension %s is not an SDF dimension.\n",parms.edimname);
      gaprnt(0,pout);
      return Failure;
    }
  } else {
    if (Ecoord) {
      edimid = find_dim(pfi, Ecoord->longnm) ;
    } else {
      edimid = -1 ;
    }
  }

  /* rewrite the fnums array if E > 1 */
  if (pfi->tmplat && pfi->dnum[4]>1) {
    /* first, free the memory used to set up fnums in gadxdf */
    if (pfi->fnums != NULL) gree(pfi->fnums,"f20a");

    /* The fnums array is the size of the time axis multiplied by the
       size of the ensemble axis. It contains the t index which generates
       the filename that contains the data for each timestep. If the ensemble 
       has no data file for a given time, the fnums value will be -1 */
    sz = sizeof(gaint)*pfi->dnum[3]*pfi->dnum[4];
    pfi->fnums = (gaint *)galloc(sz,"fnums1");   
    if (pfi->fnums==NULL) {
      gaprnt(0,"Open Error: memory allocation failed for fnums\n");
      goto err2;
    }
    /* get dt structure for t=1 */
    gr2t(pfi->grvals[3],1.0,&tdefi);
    /* loop over ensembles */
    ens=pfi->ens1;
    e=1;
    while (e<=pfi->dnum[4]) {
      j = -1; 
      t=1;
      /* set fnums value to -1 for time steps before ensemble initial time */
      while (t<ens->gt) {
	pfi->fnums[(e-1)*pfi->dnum[3]+t-1] = j;                                                    
	t++;
      }
      j = ens->gt;
      /* get dt structure for ensemble initial time */
      gr2t(pfi->grvals[3],ens->gt,&tdefe);
      /* get filename for initial time of current ensemble member  */
      ch = gafndt(pfi->name,&tdefe,&tdefe,pfi->abvals[3],pfi->pchsub1,pfi->ens1,ens->gt,e,&flag);   
      if (ch==NULL) {
	snprintf(pout,1255,"Open Error: couldn't determine data file name for e=%d t=%d\n",e,ens->gt);
	gaprnt(0,pout);
	goto err2;
      }
      pfi->fnums[(e-1)*pfi->dnum[3]+t-1] = j;                                                    
      /* loop over remaining valid times for this ensemble */
      for (t=ens->gt+1; t<ens->gt+ens->length; t++) {
	/* get filename for time index=t ens=e */
	gr2t(pfi->grvals[3],(gadouble)t,&tdef);
	pos = gafndt(pfi->name,&tdef,&tdefe,pfi->abvals[3],pfi->pchsub1,pfi->ens1,t,e,&flag);  
	if (pos==NULL) {
	  snprintf(pout,1255,"Open Error: couldn't determine data file name for e=%d t=%d\n",e,t);
	  gaprnt(0,pout);
	  goto err2;
	}
	if (strcmp(ch,pos)!=0) {    /* filename has changed */
	  j = t;   
	  gree(ch,"f47");
	  ch = pos;
	}
	else {
	  gree(pos,"f48");
	}
	pfi->fnums[(e-1)*pfi->dnum[3]+t-1] = j;                                                    
      }
      gree(ch,"f48a");
      /* set fnums value to -1 for time steps after ensemble final time */
      j = -1;
      while (t<=pfi->dnum[3]) {
	pfi->fnums[(e-1)*pfi->dnum[3]+t-1] = j;                                                    
	t++;
      }
      e++; ens++;
    }
    pfi->fnumc = 0;
    pfi->fnume = 0;
  }

  /* Find all the data variables */
  numdvars = 0 ;
  pvar = pfi->pvar1;  
  i=0;
  /* loop through complete variable list */
  while (i<pfi->vnum) {
    isDatavar=0 ;
    pvar->isdvar=0;
    isDatavar = isdvar(pfi, pvar, xdimid, ydimid, zdimid, tdimid, edimid);
    if (isDatavar==Success) {
      /* Compare to varnames in the list from the xdf descriptor */
      if (parms.isxdf && (!parms.dvsrch)) {
	varnames = parms.names1;
	j=0;
        flag=1;
	while (j<parms.dvcount && flag) {
	  if (!strcmp(pvar->longnm, varnames->longnm)) {
	    for (c=0; c<16; c++) pvar->abbrv[c] = varnames->abbrv[c];
	    numdvars++;
	    pvar->isdvar=1;
	    flag=0;
	  }
	  j++; varnames++;
	}
      }
      /* we'll use all data variables we can find */
      else {  
	numdvars++; 
	pvar->isdvar=1;
	/* create a GrADS-friendly variable name */
	strncpy(pvar->abbrv, pvar->longnm, 15);
	pvar->abbrv[15] = '\0';
	lowcas(pvar->abbrv);
      }
    }
    i++; pvar++;
  }                                                                                      
  if (numdvars == 0) {
    gaprnt(0,"Error: SDF file does not have any non-coordinate variables.\n") ;
    return Failure;
  } 
  else {
    /* allocate a new array of gavar structures */
    sz = numdvars*sizeof(struct gavar);
    if ((newpvar = (struct gavar *) galloc(sz,"newpvar")) == NULL) {
      gaprnt(0, "Error: unable to allocate memory for data variable array.\n");
      goto err1;
    }
    savepvar=newpvar;
    /* copy all the valid data variables into the new variable array */
    pvar = pfi->pvar1;  
    i=0;
    while (i<pfi->vnum) {
      if (pvar->isdvar == 1) {
	*newpvar = *pvar;
	newpvar->ncvid = -999; /* reset the varid so undefs will be read in gaio.c */
	newpvar->sdvid = -999; /* reset the varid so undefs will be read in gaio.c */
	newpvar++;
      }
      i++; pvar++;
    }
    /* update the gafile structure with the new gavar array and new vnum */
    gree(pfi->pvar1,"f4");
    pfi->pvar1 = savepvar;
    pfi->vnum = numdvars;

  }

  /* Allocate an I/O buffer the size of one row */
  sz = pfi->dnum[XINDEX] * sizeof(gadouble);
  if ((pfi->rbuf = (gadouble *)galloc(sz,"rbuf")) == NULL) goto err1;
  sz = pfi->dnum[XINDEX] * sizeof(char);
  if ((pfi->ubuf = (char *)galloc(sz,"ubuf")) == NULL) goto err1;

  /* Set one last parameter in the gafile structure */
  pfi->gsiz = pfi->dnum[XINDEX] * pfi->dnum[YINDEX];

  /* Set the default netcdf/hdf5 cache size to be big enough to contain 
     a global 2D grid of 8-byte data values */
  if (pfi->cachesize == (long)-1) {
    sf = 8 * pfi->dnum[0] * pfi->dnum[1];
    pfi->cachesize = (long)floor(sf) ;
  }

  return Success;

err2:
  if (time_units) gree(time_units,"f5");
  if (trunc_units) gree(trunc_units,"f6") ;
  if (tvals) gree(tvals,"f7");
  return Failure;
err1:
  gaprnt (0,"Error: Memory allocation error\n");
  return Failure;

} /* end of gadsdf() */

gaint compare_units(char *known_name, char *trunc_unit) {
  ut_unit *knownUnit=NULL, *thisUnit=NULL ;
  cv_converter *converter=NULL;
  gadouble slope, intercept;
  gaint rc;    

  /* Remove leading/trailing blanks */
  trunc_unit = ut_trim(trunc_unit, UT_ASCII);

  /* parse strings and get unit objects */
  knownUnit = ut_parse(utSys, known_name, UT_ASCII);
  thisUnit  = ut_parse(utSys, trunc_unit, UT_ASCII);
  if (thisUnit == NULL || knownUnit==NULL) {
    rc = Failure;
    goto retrn;
  }
  /* make sure units are convertible */
  if (ut_are_convertible(thisUnit, knownUnit)) { 
    if ((converter = ut_get_converter(thisUnit, knownUnit)) == NULL) {
      rc = Failure;
      goto retrn;
    }
    intercept = cv_convert_double(converter, 0.0);
    slope     = cv_convert_double(converter, 1.0) - intercept;
    /* check slope and intercept */
    if (dequal(slope,     1.0, (gadouble)1.0e-8)==0 && 
	dequal(intercept, 0.0, (gadouble)1.0e-8)==0) {
      rc = Success;
      goto retrn;
    }
    else
      rc = Failure;
  } 
  else 
    rc = Failure;

 retrn:
  if (knownUnit) ut_free(knownUnit);
  if (thisUnit)  ut_free(thisUnit);
  if (converter) cv_free(converter);
  return rc;
}


gaint isdvar(struct gafile *pfi, struct gavar *var, 
	   gaint xdimid, gaint ydimid, gaint zdimid, gaint tdimid, gaint edimid) {
  gaint i, hasX, hasY, hasZ, hasT, hasE;
  struct gaattr *attr; 

  /* Check if var is a coordinate variable */
  if (var->nvardims == 1) {  /* var is 1-D */
    for (i=0; i<pfi->nsdfdims; i++) {
      if (var->vardimids[0] == pfi->sdfdimids[i]) { /* var dimid matches a file dimid */
	if (!strcmp(pfi->sdfdimnam[i], var->longnm)) { /* var name matches dimension name */
	  return Failure;
	}
      }
    }
  }

  /* Determine which of var's coordinate dimensions match the 5 grid dimids */
  hasX = hasY = hasZ = hasT = hasE = 0;
  for (i=0 ; i<var->nvardims; i++) {
    if      (var->vardimids[i] == xdimid) { var->units[i]=-100; hasX=1; }
    else if (var->vardimids[i] == ydimid) { var->units[i]=-101; hasY=1; }
    else if (var->vardimids[i] == zdimid) { var->units[i]=-102; hasZ=1; }
    else if (var->vardimids[i] == tdimid) { var->units[i]=-103; hasT=1; }
    else if (var->vardimids[i] == edimid) { var->units[i]=-104; hasE=1; }
  }

  if (hasX || hasY || hasZ || hasT || hasE) {

    /* Check if any of var's dimids do not match the 5 grid dimids */
    for (i=0 ; i<var->nvardims; i++) {
      if (var->vardimids[i] != xdimid && 
	  var->vardimids[i] != ydimid && 
	  var->vardimids[i] != zdimid && 
	  var->vardimids[i] != tdimid && 
	  var->vardimids[i] != edimid) {
	return Failure;
      }
    }

    /* Set the number of vertical levels for var */
    if (hasZ) {
      var->levels = pfi->dnum[ZINDEX];
    } 
    else {
      var->levels = 0;
    }

    /* Create a variable description */
    attr = NULL;
    attr = find_att(var->longnm, pfi->attr, "long_name");
    if (attr == NULL) attr = find_att(var->longnm, pfi->attr, "standard_name");
    if (attr == NULL) {
      strncpy(var->varnm,var->longnm,159);
      var->varnm[160] = '\0';
    }
    else {
      if (attr->len < 160) {
	strcpy(var->varnm,(char*)attr->value);
	var->varnm[attr->len-1] = '\0';
      }
      else {
	strncpy(var->varnm,(char*)attr->value,159);
	var->varnm[160] = '\0';
      }
    }
    return Success;
  }
  return Failure;
}


/* Adapted from deflev routine */
gaint sdfdeflev(struct gafile *pfi, struct gavar *coord, gaint dim, gaint revflag) {
gadouble *axisvals=NULL, *vals=NULL,*aptr=NULL,*vvs=NULL,*ddata=NULL;
gadouble delta1,delta2,val1,val2,incr,v1,v2;
gafloat  *fdata=0;
size_t sz,start[16],count[16];
gaint    rc,i,len,flag=0,status=0;
#if USEHDF == 1
int32 hstart[16],hcount[16];
int32 rank,natts,dim_sizes[H4_MAX_VAR_DIMS],dtype,sds_id;
int32 *idata=NULL;
uint32 *uidata=NULL;
#endif

  /* allocate an array of doubles to hold coordinate axis values */
  len = pfi->dnum[dim] ; 
  sz = sizeof(gadouble)*len;
  if ((axisvals = (gadouble *)galloc(sz,"axisvals")) == NULL) {
    gaprnt(0,"sdfdeflev: Unable to allocate memory for reading coordinate axis values\n");
    return Failure;
  }
  aptr=axisvals;  /* keep a copy of pointer to start of array */

  /* initialize start and count arrays */
#if USENETCDF==1
  for (i=0 ; i<16 ; i++) {
    start[i] = 0; 
    count[i] = 1; 
  }
  count[0] = (size_t)len; 
#endif
#if USEHDF == 1    
  for (i=0 ; i<16 ; i++) {
    hstart[i]=0;
    hcount[i]=0;
  }
  hcount[0] = (int32)len; 
#endif

  /* read the data */
#if USEHDF == 1
  if (pfi->ncflg==2) {
    /* get the data type */
    if ((sds_id = SDselect(pfi->sdid,coord->sdvid))==FAIL) return Failure;
    rc = SDgetinfo(sds_id, coord->longnm, &rank, dim_sizes, &dtype, &natts);
    if (rc == -1) {
      gaprnt(0,"sdfdeflev: unable to determine coordinate axis data type\n");
      goto err1;
    }
    switch (dtype) {
    case (DFNT_INT32):    /* definition value 24 */
      sz = len * sizeof (int32);
      if ((idata = (int32 *)galloc(sz,"idata"))==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype INT32\n");
	return(1);
      }
      if (SDreaddata(sds_id, hstart, NULL, hcount, (VOIDP *)idata) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype INT32\n");
	return(1);
      } 
      if (revflag) {
	for (i=len-1; i>=0; i--) {
	  *aptr = (gadouble)idata[i]; 
	  aptr++;
	}
      }
      else {
	for (i=0; i<len; i++) {
	  *aptr = (gadouble)idata[i]; 
	  aptr++;
	}
      }
      gree(idata,"f130a");
      break;

    case (DFNT_UINT32):   /* definition value 25 */
      sz = len * sizeof (uint32);
      if ((uidata = (uint32 *)galloc(sz,"uidata"))==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype UINT32\n");
	return(1);
      }
      if (SDreaddata(sds_id, hstart, NULL, hcount, (VOIDP *)uidata) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype UINT32\n");
	return(1);
      } 
      if (revflag) {
	for (i=len-1; i>=0; i--) {
	  *aptr = (gadouble)uidata[i]; 
	  aptr++;
	}
      }
      else {
	for (i=0; i<len; i++) {
	  *aptr = (gadouble)uidata[i]; 
	  aptr++;
	}
      }
      gree(uidata,"f131a");
      break;

    case (DFNT_FLOAT32):   /* definition value 5 */
      sz = len * sizeof (gafloat);
      if ((fdata = (gafloat *)galloc(sz,"fdata"))==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype FLOAT32\n");
	return(1);
      }
      if (SDreaddata(sds_id, hstart, NULL, hcount, (VOIDP *)fdata) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype FLOAT32\n");
	return(1);
      } 
      if (revflag) {
	for (i=len-1; i>=0; i--) {
	  *aptr = (gadouble)fdata[i]; 
	  aptr++;
	}
      }
      else {
	for (i=0; i<len; i++) {
	  *aptr = (gadouble)fdata[i]; 
	  aptr++;
	}
      }
      gree(fdata,"f131");
      break;

    case (DFNT_FLOAT64):  /* definition value  6 */
      sz = sizeof(gadouble)*len;
      if ((ddata = (gadouble *)galloc(sz,"ddata")) == NULL) {
	gaprnt(0,"sdfdeflev: unable to allocate memory for coordinate axis type NC_DOUBLE\n");
	goto err1;
      }
      if (SDreaddata(sds_id, hstart, NULL, hcount, (VOIDP *)ddata) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype FLOAT32\n");
	return(1);
      } 
      if (revflag) {
	for (i=len-1; i>=0; i--) {
	  *aptr = (gadouble)ddata[i]; 
	  aptr++;
	}
      }
      else {
	for (i=0; i<len; i++) {
	  *aptr = (gadouble)ddata[i]; 
	  aptr++;
	}
      }
      gree(ddata,"f131");
      break;
      
    default:
      snprintf(pout,1255,"HDF coordinate axis data type %d not handled\n",dtype);
      gaprnt(0,pout);
      goto err1;
    };
    
  }
#endif

#if USENETCDF==1
  if (pfi->ncflg==1) {
    sz = sizeof(gadouble)*len;
    if ((ddata = (gadouble *)galloc(sz,"ddata")) == NULL) {
      gaprnt(0,"sdfdeflev: unable to allocate memory for coordinate axis values\n");
      goto err1;
    }
    status = nc_get_vara_double(pfi->ncid, coord->ncvid, start, count, ddata);
    if (status != NC_NOERR) {
      gaprnt(0,"sdfdeflev: nc_get_vara_double failed to read coordinate axis values \n");
      handle_error(status);
      gree(ddata,"f15");
      goto err1;
    }
    if (revflag) {
      for (i=len-1; i>=0; i--) {
	*aptr = ddata[i];
	aptr++;
      }
    }
    else {
      for (i=0; i<len; i++) {
	*aptr = ddata[i]; 
	aptr++;
      }
    }
    if (ddata) gree(ddata,"f16");
  }
#endif
  
  /* Check if dimension is linear */
  if (len < 3) {
    pfi->linear[dim] = 1;
  }
  else {
    flag=0;
    delta1 = axisvals[1]-axisvals[0];
    for (i=2; i<len; i++) {
      delta2 = axisvals[i]-axisvals[i-1];
      if (dequal(delta1, delta2, 1.0e-8) == 1) {
 	flag=1;
	break; 
      } 
    }
    if (flag) 
      pfi->linear[dim] = 0;
    else 
      pfi->linear[dim] = 1;
  }

  /* Set linear scaling values */
  if (pfi->linear[dim]==1) {
    sz = sizeof(gadouble)*6;
    vals = (gadouble *)galloc(sz,"linearvals");
    if (vals==NULL) goto err1;
    v1 = axisvals[0];
    if (len==1)
      v2 = 1.0;
    else
      v2 = axisvals[1]-axisvals[0];
    *(vals)   = v2;
    *(vals+1) = v1 - v2;
    *(vals+2) = -999.9;
    pfi->grvals[dim] = vals;
    *(vals+3) = 1.0/v2;
    *(vals+4) = -1.0 * ((v1-v2)/v2);
    *(vals+5) = -999.9;
    pfi->abvals[dim] = vals+3;
    pfi->ab2gr[dim] = liconv;
    pfi->gr2ab[dim] = liconv;
  }
  else {
    /* set non-linear scaling values */
    sz = (pfi->dnum[dim]+5)*sizeof(gadouble);
    vals = (gadouble *)galloc(sz,"levelvals");
    if (vals==NULL) goto err1;
    
    vvs = vals;
    *vvs = (gadouble)pfi->dnum[dim];
    vvs++;
    for (i=0; i<len; i++) {
      *vvs = axisvals[i];
      vvs++;
    }
    *vvs = -999.9;
    pfi->abvals[dim] = vals;
    pfi->grvals[dim] = vals;
    pfi->ab2gr[dim] = lev2gr;
    pfi->gr2ab[dim] = gr2lev;
  }

  /* Check if we need to convert Pa to mb */
  if ((dim==2) && (pfi->pa2mb)) {
    for (i=1; i<=pfi->dnum[2]; i++) {
      *(pfi->grvals[2]+i) = *(pfi->grvals[2]+i)/100;
    }
  }


  /* check if longitudes wrap around the globe */
  if ((dim==0) && (pfi->linear[dim]) && (len > 2) ) {
    val1 = axisvals[0]; 
    incr = axisvals[1]-axisvals[0];
    val2 = val1 + (len * incr);
    if (fabs((val2-360.0)-val1)<0.01) pfi->wrap = 1;
  }
  
  gree(axisvals,"f17a");
  return Success ;
 
err1:
  gree(axisvals,"f17");
  return Failure;
}


/* check for coordinate variable that 
   1) has units degrees_east, degree_east, degrees_E, or degree_E, or
   2) has an "axis" attribute with a value of "X"
*/
gaint findX (struct gafile *pfi, struct gavar **Xcoordptr) {
  struct gaattr *attr ;
  struct gavar *lclvar ;
  gaint iscoordvar, i, j, match;

  i=0;
  lclvar = pfi->pvar1;  
  while (i<pfi->vnum) {
    iscoordvar = 0 ;
    if (lclvar->nvardims == 1) {  /* variable must be 1-D */
      for (j=0; j<pfi->nsdfdims; j++) {
	if (lclvar->vardimids[0] == pfi->sdfdimids[j]) { /* var dimid matches a file dimid */
	  if (!strcmp(pfi->sdfdimnam[j], lclvar->longnm)) { /* var name matches dimension name */
	    iscoordvar = 1 ;
	  }
	}
      }
    }
    if (iscoordvar) {
      /* look for "units" attribute */
      attr = NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "units");
      if (attr) {
	match=0;
	if (!strncmp(attr->value, "degrees_east", 12)) match=1;
	if (!strncmp(attr->value, "degree_east",  11)) match=1;
	if (!strncmp(attr->value, "degrees_E",     9)) match=1;
	if (!strncmp(attr->value, "degree_E",      8)) match=1;
	if (match) {
	  *Xcoordptr = lclvar ;
	  return Success ;  /* got a match on one of them */
	}
      }
      /* look for "axis" attribute */
      attr=NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "axis");
      if (attr) {
	match=0;
	if (!strncmp(attr->value, "X", 1)) match=1;
	if (!strncmp(attr->value, "x", 1)) match=1;
	if (match) {
	  *Xcoordptr = lclvar ;
	  return Success ;  /* got a match on */
	}
      }
    }
    i++; lclvar++;
  } 
  return Failure ;
}
  
/* check for coordinate variable that 
   1) has units degrees_north, degree_north, degrees_N, or degree_N, or
   2) has an "axis" attribute with a value of "Y"
*/
gaint findY(struct gafile *pfi, struct gavar **Ycoordptr) {
  struct gaattr *attr;
  struct gavar *lclvar;
  gaint iscoordvar, i, j, match;

  i=0;
  lclvar=pfi->pvar1;
  while (i<pfi->vnum) {
    iscoordvar = 0 ;
    if (lclvar->nvardims == 1) {  /* variable must be 1-D */
      for (j=0; j<pfi->nsdfdims; j++) {
	if (lclvar->vardimids[0] == pfi->sdfdimids[j]) { /* variable dimid matches a file dimid */
	  if (!strcmp(pfi->sdfdimnam[j], lclvar->longnm)) { /* variable name matches dimension name */
	    iscoordvar = 1 ;
	  }
	}
      } 
    }
    if (iscoordvar) {
      /* look for "units" attribute */
      attr=NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "units");
      if (attr) {
	match=0;
	if (!strncmp(attr->value, "degrees_north", 13)) match=1;
	if (!strncmp(attr->value, "degree_north",  12)) match=1;
	if (!strncmp(attr->value, "degrees_N",      9)) match=1;
	if (!strncmp(attr->value, "degree_N",       8)) match=1;
	if (match) {
	  *Ycoordptr = lclvar;
	  return Success;  /* got a match on one of them */
	}
      }
      /* look for "axis" attribute */
      attr=NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "axis");
      if (attr) {
	match=0;
	if (!strncmp(attr->value, "Y", 1)) match=1;
	if (!strncmp(attr->value, "y", 1)) match=1;
	if (match) {
	  *Ycoordptr = lclvar ;
	  return Success ;  /* got a match on */
	}
      }
    }
    i++; lclvar++;
  }  
  return Failure;
}

/* check for coordinate variable that 
   1) has units of pressure or another unit approved by COARDS conventions. 
   initially, the pressure units are "millibars" or "pascals" (caseless)
   should probably allow for prefixes through udunits package
   Will also allow exact match on "mb" 
   2) has an "axis" attribute with a value of "Z", or 
*/
gaint findZ(struct gafile *pfi, struct gavar **Zcoordptr, gaint *ispressptr) {
  struct gaattr *attr;
  struct gavar  *lclvar ;
  gaint iscoordvar, i, j, match, rc;
  ut_unit *thisguy=NULL;

  i=0;
  lclvar=pfi->pvar1;
  while (i<pfi->vnum) {
    iscoordvar = 0 ;
    if (lclvar->nvardims == 1) {  /* variable must be 1-D */
      for (j=0; j<pfi->nsdfdims; j++) {
	if (lclvar->vardimids[0] == pfi->sdfdimids[j]) { /* variable dimid matches a file dimid */
	  if (!strcmp(pfi->sdfdimnam[j], lclvar->longnm)) { /* variable name matches dimension name */
	    iscoordvar = 1 ;
	  }
	}
      } 
    }
    if (iscoordvar) {
      /* look for "units" attribute */
      attr = NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "units") ;
      if (attr) {
	match=0;
	if (!strncasecmp(attr->value, "hybrid_sigma_pressure", 21)) match=1;
	if (!strncasecmp(attr->value, "mb", 2)) match=1;  
	if (!strncasecmp(attr->value, "millibar", 8)) match=1;  
	if (!strncasecmp(attr->value, "hPa", 3)) match=1;
	if (match) {
	  *Zcoordptr = lclvar ;
	  *ispressptr = 1 ;
	  rc = Success;
	  goto retrn;
	}
	match=0;
	if (!strncasecmp(attr->value, "sigma_level", 11)) match=1;
	if (!strncasecmp(attr->value, "degreesk",     8)) match=1;
	if (!strncasecmp(attr->value, "degrees_k",    9)) match=1;
	if (!strncasecmp(attr->value, "level",        5)) match=1; 
	if (!strncasecmp(attr->value, "layer",        5)) match=1;
	if (!strncasecmp(attr->value, "layers",       6)) match=1;
	if (match) {
 	  *Zcoordptr = lclvar ;
	  *ispressptr = 0 ;
	  rc = Success;
	  goto retrn;
	}
	if ((thisguy = ut_parse(utSys, attr->value, UT_ASCII)) != NULL) {
          /* if we can convert the units to feet, then it could be depth */
	  if (ut_are_convertible(thisguy, feet)) {
	    *Zcoordptr = lclvar ;
	    *ispressptr = 0 ;
	    rc = Success;
	    goto retrn;
	  }
	  /* if we can convert the units to pascals, then it could be pressure */
	  if (ut_are_convertible(thisguy, pascals)) {
	    pfi->pa2mb = 1; 
	    *Zcoordptr = lclvar ;
	    *ispressptr = 1 ;
	    rc = Success;
	    goto retrn;
	  }
	  /* if we can convert the units to kelvins, then it could be isothermic */
	  if (ut_are_convertible(thisguy, kelvins)) {
	    *Zcoordptr = lclvar ;
	    *ispressptr = 0 ;
	    rc = Success;
	    goto retrn;
	  }
	} /* if unit can be parsed */
      }
      /* look for "axis" attribute */
      attr=NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "axis");
      if (attr) {
	match=0;
	if (!strncmp(attr->value, "Z", 1)) match=1;
	if (!strncmp(attr->value, "z", 1)) match=1;
	if (match) {
	  *Zcoordptr = lclvar ;
          *ispressptr = 0 ;
	  rc = Success;
	  goto retrn;
	}
      }
    }
    i++; lclvar++;
  } 
  rc = Failure;

 retrn:
  if (rc == Failure && thisguy) ut_free(thisguy);
  return rc;
}

/* find a coordinate variable 
   1) has units that mark it as one sort of time or another, or
   2) has an "axis" attribute with a value of "T" 
*/
gaint findT(struct gafile *pfi, struct gavar **Tcoordptr) {
  struct gaattr *attr ;
  struct gavar *lclvar ;
  gaint iscoordvar, i, j, rc, match;
  ut_unit *attrUnit=NULL, *timeUnit=NULL;

  i=0;
  lclvar=pfi->pvar1;
  while (i<pfi->vnum) {
    iscoordvar = 0 ;
    if (lclvar->nvardims == 1) {  /* variable must be 1-D */
      for (j=0; j<pfi->nsdfdims; j++) {
	if (lclvar->vardimids[0] == pfi->sdfdimids[j]) { /* variable dimid matches a file dimid */
	  if (!strcmp(pfi->sdfdimnam[j], lclvar->longnm)) { /* variable name matches dimension name */
	    iscoordvar = 1 ;
	  }
	}
      } 
    }
    if (iscoordvar) {
      attr = NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "units") ;
      if (attr) {
	match=0;
	if (!strncasecmp((char*)attr->value, "yyyymmddhhmmss", 14)) match=1;
	if (!strncasecmp((char*)attr->value, "yymmddhh",        8)) match=1;
	if (match) {
	  *Tcoordptr = lclvar ;
	  rc = Success;
	  goto retrn;
	}
	/* Use udunits to check if this is a time unit */
	attrUnit = ut_parse(utSys, (char*)attr->value, UT_ASCII);
	timeUnit = ut_offset_by_time(second, ut_encode_time(2001, 1, 1, 0, 0, 0.0));

	if (attrUnit != NULL && timeUnit != NULL) {
	  if (ut_are_convertible(attrUnit, timeUnit)) { 
	    /* we can convert it to known time unit, so we have a T coordinate */
	    *Tcoordptr = lclvar ;
	    rc = Success;
	    goto retrn;
	  }
	}
      }
      /* look for "axis" attribute */
      attr=NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "axis");
      if (attr) {
	match=0;
	if (!strncmp(attr->value, "T", 1)) match=1;
	if (!strncmp(attr->value, "t", 1)) match=1;
	if (match) {
	  *Tcoordptr = lclvar ;
	  rc = Success;
	  goto retrn;
	}
      }
    }
    i++; lclvar++;
  } 
  rc = Failure ;

 retrn:
  if (attrUnit) ut_free(attrUnit);
  if (timeUnit) ut_free(timeUnit);
  return rc;
}

/* check for ensemble coordinate variable with attribute "axis" or "grads_dim" equal to "e" */
gaint findE(struct gafile *pfi, struct gavar **Ecoordptr) {
  struct gaattr *attr;
  struct gavar *lclvar;
  gaint iscoordvar, i, j, match;

  i=0;
  lclvar=pfi->pvar1;
  while (i<pfi->vnum) {
    iscoordvar = 0 ;
    if (lclvar->nvardims == 1) {  /* variable must be 1-D */
      for (j=0; j<pfi->nsdfdims; j++) {
	if (lclvar->vardimids[0] == pfi->sdfdimids[j]) { /* variable dimid matches a file dimid */
	  if (!strcmp(pfi->sdfdimnam[j], lclvar->longnm)) { /* variable name matches dimension name */
	    iscoordvar = 1 ;
	  }
	}
      } 
    }
    if (iscoordvar) {
      /* look for "grads_dim" attribute */
      attr=NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "grads_dim");
      if (attr) {
	match=0;
	if (!strncmp(attr->value, "e", 1)) match = 1;
	if (match) {
	  *Ecoordptr = lclvar;
	  return Success;  /* got a match */
	}
      }
      /* look for "axis" attribute */
      attr=NULL;
      attr = find_att(lclvar->longnm, pfi->attr, "axis");
      if (attr) {
	match=0;
	if (!strncmp(attr->value, "E", 1)) match=1;
	if (!strncmp(attr->value, "e", 1)) match=1;
	if (match) {
	  *Ecoordptr = lclvar ;
	  return Success ;  
	}
      }
    }
    i++; lclvar++;
  }
  return Failure;
}


/* Strip the given number of characters from the string and return the
   new length.  If the number of characters to strip is less than
   or equal to zero, or if the strip length is greater than the
   string length, return Failure.  Otherwise, return Success. */
gaint strip_char (gaint strip_num, char *str1, gaint *int_len) {
  gaint slen;

  slen = strlen (str1);
  if (strip_num <= 0) return Failure;
  slen -= strip_num;
  if (slen < 0) return Failure;
  *int_len = slen;
  str1[slen] = '\0';
  return Success;
}

/* Decode standard time.  Return Success if OK, Failure if error. */
gaint decode_standard_time (gadouble time_val, gaint *year, gaint *month, 
			   gaint *day, gaint *hour, gaint *minn, gafloat *sec) {
char   str1[100];
gaint  i,slen,int_len;

  /* Make time value into character string. */
  snprintf(str1,99,"%f", time_val);

  /* Find decimal point. */
  slen = strlen (str1);
  for (i = 0; i < slen; i++) {
    if (str1[i] == '.') {
      int_len = i;
      break;
    }
  }
  if (int_len == 0) return Failure;

  /* Get second. */
  if (int_len <= 2) {
      sscanf (str1, "%g", sec);
      int_len = 0;
  }
  else {
    sscanf (&str1[int_len - 2], "%g", sec);
    str1[int_len] = '\0';
    (void) strip_char (2, str1, &int_len);
  }

  /* Get minute. */
  *minn = MISSING;
  if (int_len > 0) {
    if (int_len <= 2) {
      sscanf (str1, "%d", minn);
      int_len = 0;
    }
    else {
      sscanf (&str1[int_len - 2], "%d", minn);
      str1[int_len] = '\0';
      (void) strip_char (2, str1, &int_len);
    }
  }

  /* Get hour. */
  *hour = MISSING;
  if (int_len > 0) {
    if (int_len <= 2) {
      sscanf (str1, "%d", hour);
      int_len = 0;
    }
    else {
      sscanf (&str1[int_len - 2], "%d", hour);
      str1[int_len] = '\0';
      (void) strip_char (2, str1, &int_len);
    }
  }
  
  /* Get day. */
  *day = MISSING;
  if (int_len > 0) {
    if (int_len <= 2) {
      sscanf (str1, "%d", day);
      int_len = 0;
    }
    else {
      sscanf (&str1[int_len - 2], "%d", day);
      str1[int_len] = '\0';
      (void) strip_char (2, str1, &int_len);
    }
  }
  
  /* Get month. */
  *month = MISSING;
  if (int_len > 0) {
    if (int_len <= 2)	{
      sscanf (str1, "%d", month);
      int_len = 0;
    }
    else {
      sscanf (&str1[int_len - 2], "%d", month);
      str1[int_len] = '\0';
      (void) strip_char (2, str1, &int_len);
    }
  }
  
  /* Get year.  A year of 0000 or 9999 defaults to missing. */
  *year = MISSING;
  if (int_len > 0) 
    sscanf (str1, "%d", year);
  if ((*year == 0) || (*year == 9999))
    *year = MISSING;

  /* All OK. */
  return Success;
}


/* Free a netCDF attribute list. */
gaint free_att_info (struct gafile *pfi) { 
  struct gaattr *attrib, *nextattrib;

  if (pfi->attr) {
    for (attrib = pfi->attr; attrib != NULL; attrib = nextattrib) { 
      nextattrib = attrib->next;
      if (attrib->value) gree(attrib->value,"f18");
      gree(attrib,"f19");
    }
    pfi->attr = NULL;
  }
  return Success;
}

/* open and read the metadata in a netCDF file */
/* gafile structure should already be initialized */
gaint read_metadata (struct gafile *pfi) {
struct gavar *pvar;
gaint rc,i,ii,j,len,ngatts,natts,ndims,nvars,status,oflg,dummy;
size_t sz,size=0;
char name[300];
#if USEHDF ==1
int32 ndsets,dimsize,sds_id,dim_id,dtype,ndatts;
int32 dim_sizes[H4_MAX_VAR_DIMS],rank;
char sdsname[H4_MAX_NC_NAME+1];
char dimname[H4_MAX_NC_NAME+1];
#endif

 if (strlen (pfi->name) == 0) return Failure;

  /* Open the file */
  if (pfi->tmplat) {
    i = gaopfn(1,1,&dummy,&oflg,pfi);   /* assume 1st ensemble member */
    if (i==-99999) {
      gaprnt(0,"read_metadata: gaopfn failed (rc=-99999)\n");
      return Failure;
    }
    if (i==-88888) {
      gaprnt(0,"read_metadata: gaopfn failed (rc=-88888)\n");
      return Failure;
    }
  }
  else {
    if (pfi->ncflg==1) {
      rc = gaopnc (pfi,0,1);
      if (rc) return Failure;
    }
    if (pfi->ncflg==2) {
      rc = gaophdf (pfi,0,1);
      if (rc) return Failure;
    }    
  }

  /* get general information. */
#if USEHDF==1
  if (pfi->ncflg==2) {
    status = SDfileinfo(pfi->sdid, &ndsets, &ngatts);
    if (status == -1) {
      gaprnt(0,"read_metadata: SDfileinfo failed\n");
      goto err1;
    }
    /* find out how many data sets are coordinate variables */
    ndims=0;
    for (i=0; i<ndsets; i++) {
      /* get info about this data set */
      sds_id = SDselect(pfi->sdid, i);
      status = SDgetinfo(sds_id, sdsname, &rank, dim_sizes, &dtype, &natts);
      if (status == -1) {
	snprintf(pout,1255,"read_metadata: SDgetinfo failed for sds_id=%d\n",sds_id);
	gaprnt(0,pout);
	goto err3;
      }
      /* coordinate variables must have only 1 dimension */
      if (rank==1) {   
	dim_id = SDgetdimid(sds_id,0);                                  
	status = SDdiminfo(dim_id, dimname, &dimsize, &dtype, &ndatts); 
	if (status == -1) {
	  snprintf(pout,1255,"read_metadata: SDdiminfo failed for sds_id=%d, dimid=%d\n",sds_id,dim_id);
	  gaprnt(0,pout);
	  goto err3;
	}
	/* name of dimension must match name of variable */
	if (strcmp(dimname,sdsname)==0) {
	  /* it's a coordinate variable */
	  if (dimsize==0) {
	    /* This is the unlimited dimension. 
	       The first element of the dim_sizes array contains 
	       the number of records in the unlimited dimension. */
	    dimsize = dim_sizes[0]; 
	  }
	  pfi->sdfdimids[ndims] = (gaint)dim_id;
	  strcpy(&pfi->sdfdimnam[ndims][0],dimname);
	  pfi->sdfdimsiz[ndims] = (gaint)dimsize;
	  ndims++;
	}
      }
    }

    if (ndims==0 && ndsets>0) {
      /* file has data variables but no coordinate variables. 
	 Get the dimension ids and names from the first data variable */
      sds_id = SDselect(pfi->sdid, 0);
      status = SDgetinfo(sds_id, sdsname, &rank, dim_sizes, &dtype, &natts);
      for (j=0; j<rank; j++) {
	dim_id = SDgetdimid(sds_id, j);
	status = SDdiminfo(dim_id, dimname, &dimsize, &dtype, &ndatts);
	pfi->sdfdimids[ndims] = (gaint)dim_id;
	strcpy(&pfi->sdfdimnam[ndims][0],dimname);
	pfi->sdfdimsiz[ndims] = (gaint)dimsize;
	ndims++;
      }
    }
    pfi->nsdfdims = ndims;
    pfi->vnum = ndsets;

  }
#endif

#if USENETCDF==1
  if (pfi->ncflg==1) {
    status = nc_inq(pfi->ncid, &ndims, &nvars, &ngatts, NULL);
    if (status != NC_NOERR) {
      handle_error(status);
      goto err1;
    }
    pfi->nsdfdims = ndims;
    pfi->vnum = nvars;
    /* get NC coordinate information */
    for (i=0; i<pfi->nsdfdims; i++) {
      pfi->sdfdimids[i] = i;
      status = nc_inq_dim (pfi->ncid, i, name, &size);
      if (status != NC_NOERR) {
	handle_error(status);
	goto err1;
      }
      strcpy(&pfi->sdfdimnam[i][0],name);
      pfi->sdfdimsiz[i] = (gaint)size;
    }
  }
#endif
  /* Retrieve global attributes */
#if USEHDF==1
  if (pfi->ncflg==2) {
    read_hdfatts (pfi->sdid, "global", ngatts, pfi);
  }
#endif
#if USENETCDF==1
  if (pfi->ncflg==1) {
    read_ncatts (pfi->ncid, NC_GLOBAL, NULL, ngatts, pfi); 
  }
#endif
  /* Get variable info and attributes */
  sz = pfi->vnum * sizeof(struct gavar);
  if ((pvar = (struct gavar *)galloc(sz,"pvar")) == NULL) {
    gaprnt(0,"read_metadata: memory allocation failed for pvar array \n");
    goto err2;
  }
  pfi->pvar1 = pvar;
  i = 0;
  while (i<pfi->vnum)  {
    /* initialize variables in the pvar structure */
    pvar->offset = 0;
    pvar->recoff = 0;
    pvar->ncvid = -999;
    pvar->sdvid = -999;
#if USEHDF5==1
    pvar->h5vid = -999;
#endif
    pvar->levels = 0;
    pvar->dfrm = 0;
    pvar->var_t = 0;
    pvar->scale = 1;
    pvar->add = 0;
    pvar->undef= -9.99E33;
    pvar->vecpair = -999;
    pvar->isu = 0;
    pvar->isdvar = 0;
    pvar->nvardims = 0;
    pvar->nh5vardims = 0; 
    for (ii=0; ii<16; ii++) pvar->units[ii]=-999;

    /* get the variable info */
    natts=0;
#if USEHDF==1
    if (pfi->ncflg==2) {
      /* get info about the current data set */
      sds_id = SDselect(pfi->sdid, i);
      if (sds_id==FAIL) {
	snprintf(pout,1255,"read_metadata: SDselect failed for varid %d\n",i);
	gaprnt(0,pout);
	goto err3;
      }
      status = SDgetinfo(sds_id, name, &(pvar->nvardims), dim_sizes, &dtype, &natts);
      if (status == -1) {
	snprintf(pout,1255,"read_metadata: SDgetinfo failed for varid %d\n",i);
	gaprnt(0,pout);
	goto err3;
      }
      status = SDnametoindex(pfi->sdid, name);
      if (status == -1) {
	snprintf(pout,1255,"read_metadata: SDnametoindex failed for varid %d\n",i);
	gaprnt(0,pout);
	goto err3;
      }
      pvar->sdvid = status;
      for (j=0; j<pvar->nvardims; j++) {
	dim_id = SDgetdimid(sds_id,j);
	pvar->vardimids[j] = (gaint)dim_id;
      }
    }
#endif
#if USENETCDF==1
    if (pfi->ncflg==1) {
      status = nc_inq_var(pfi->ncid, i, name, NULL, &(pvar->nvardims), pvar->vardimids, &natts);
      if (status != NC_NOERR) {
	snprintf(pout,1255,"read_metadata: nc_inq_var failed to retrieve variable info for varid %d\n",i);
	gaprnt(0,pout);
	handle_error(status);
	goto err3;
      }
      pvar->ncvid = i;
    }
#endif
    len = strlen(name);
    strncpy(pvar->longnm,name,len+1);

    /* Retrieve variable attribute values */
#if USEHDF==1
    if (pfi->ncflg==2) {
      read_hdfatts (pfi->sdid, pvar->longnm, natts, pfi);
    }
#endif
#if USENETCDF==1
    if (pfi->ncflg==1) {
      read_ncatts (pfi->ncid, pvar->ncvid, pvar->longnm, natts, pfi);
    }
#endif

    i++; pvar++;
  }  
  
  /* determine if new or old units are being used */
  set_time_type (pfi);
  
  /* set up standard tables according to time unit being used */
  init_standard_arrays (pfi->time_type);
  
  return Success;

err3:
  if (pfi->pvar1) gree(pfi->pvar1,"f20");
  goto err2;

err2:
  free_att_info (pfi);
  goto err1;

err1:
  close_sdf (pfi);
  return Failure;
} /* end of read_metadata() */

/* Close a SDF file. */
void close_sdf (struct gafile *pfi) { 
#if USENETCDF==1
  if (pfi->ncflg==1) gaclosenc(pfi);
#endif
#if USEHDF==1
  if (pfi->ncflg==2) gaclosehdf(pfi); 
#endif
}
		
gaint set_time_type (struct gafile *pfi) {
#if USENETCDF==1
  struct gavar *time, *lclvar;
  ut_unit *timeUnit, *checkUnit;
  struct gaattr *attr;
  gaint i,flag;

  time = NULL;
  time = find_var(pfi, cdc_vars[TIME_IX]);
  if (time == NULL) {
    i=0; flag=1;
    lclvar = pfi->pvar1;  
    while (i<pfi->vnum && (flag) && (lclvar != NULL)) {
      if (lclvar->nvardims == 1) {
	attr = NULL;
	attr = find_att(lclvar->longnm, pfi->attr, cdc_time_atts[T_UNITS_IX]);
	if (attr != NULL) {
          if ((timeUnit = ut_parse(utSys, (char*)attr->value, UT_ASCII)) != NULL) {
	    /* if we can convert this unit to seconds, then it's a time unit */
	    checkUnit = ut_offset_by_time(second, ut_encode_time(2001, 1, 1, 0, 0, 0.0));
	    if (ut_are_convertible(timeUnit, checkUnit)) { 
	      time = lclvar ;
	      flag=0;
	    }
	    ut_free(timeUnit);
	    ut_free(checkUnit);
	  }
	}
      }
      i++; lclvar++;
    } 
  }
  if (time == NULL) return Failure ;
  attr = NULL;
  attr = find_att(time->longnm, pfi->attr, cdc_time_atts[T_UNITS_IX]);
  if ((attr!=NULL) && (!strncasecmp ("yyyy", (char*)attr->value, 4))) 
    pfi->time_type = CDC;        /* it's the old style */
  else 
    pfi->time_type = COOP;
#endif
  return Success;
}
 
 
gaint init_standard_arrays (gaint time_type) {
#if USENETCDF==1
  if (time_type == CDC) {
    dims = cdc_dims;
    vars = cdc_vars;
    var_type = cdc_var_type;
    var_atts = cdc_var_atts;
    var_atts_type = cdc_var_atts_type;
    var_atts_val = cdc_var_atts_val;
    obs_atts_val = cdc_obs_atts_val;
    vatts_abbrev = cdc_vatts_abbrev;
    time_atts = cdc_time_atts;
    time_atts_val = cdc_time_atts_val;
    latlon_atts = cdc_latlon_atts;
    num_reqd_vatts = NUM_REQD_VATTS;
    num_reqd_vars = NUM_REQD_VARS;
    num_reqd_dims = NUM_REQD_DIMS;
  }
  else {
    dims = coop_dims;
    vars = coop_vars;
    var_type = coop_var_type;
    var_atts = coop_var_atts;
    var_atts_type = coop_var_atts_type;
    var_atts_val = coop_var_atts_val;
    obs_atts_val = coop_obs_atts_val;
    vatts_abbrev = coop_vatts_abbrev;
    time_atts = coop_time_atts;
    time_atts_val = coop_time_atts_val;
    latlon_atts = coop_latlon_atts;
    num_reqd_vatts = NUM_REQD_COOP_VATTS;
    num_reqd_vars = NUM_REQD_COOP_VARS;
    num_reqd_dims = NUM_REQD_COOP_DIMS;
  }
#endif
  return Success;
}


/* Return an attribute structure, given the variable and attribute names.
   The varname argument may be "global", "ALL", or a specific variable name. */

struct gaattr *find_att (char *varname, struct gaattr *first_att, char *attname) {
  static struct gaattr *attr = NULL;
  
  attr = first_att;
  while (attr != NULL) {
    if (!strcmp(varname,"ALL")) {
      /* don't test if varnames match */
      if (!strcasecmp(attname, attr->name)) { 
	return (attr);
      }
    }
    else {
      /* do test if varnames match */
      if (!strcmp(varname, attr->varname)) {
	if (!strcasecmp(attname, attr->name)) {
	  return (attr);
	}
      }    
    }
    attr = attr->next;
  }
  /* didn't find any match */
  return(NULL);
}


/* read netcdf attribute information for a file or variable */
gaint read_hdfatts (gaint sdid, char *vname, gaint natts, struct gafile *pfi) 
{
#if USEHDF ==1 
struct gaattr *attrib=NULL,*newattrib=NULL;
gaint   i,len=0,gotatt;
int32   sds_id,attr_dtype,attr_count;
char    *varname,*attname;
char8   *cval=NULL;
uchar8  *ucval=NULL;
int8    *icval=NULL;
uint8   *uicval=NULL;
int16   *sval=NULL;
uint16  *usval=NULL;
int32   *ival=NULL;
uint32  *uival=NULL;
float32 *fval=NULL;
float64 *dval=NULL;
size_t sz;

  if (cmpwrd("global",vname)) 
    len=7;
  else
    len=strlen(vname)+2;
  sz = len;
  if ((varname=(char*)galloc(sz,"hdfvname"))==NULL) {
    gaprnt(0,"read_hdfatts error: memory allocation failed for varname\n");
    return(Failure);
  }
  if (cmpwrd("global",vname)) {
    sds_id = sdid;
    strncpy(varname,"global",len);
  }
  else {
    strncpy(varname,vname,len);
    sds_id = SDnametoindex(sdid, vname);
    if (sds_id == -1) return (0);
    sds_id = SDselect(sdid,sds_id);
  }
  
  /* Loop through list of attributes */ 
  for (i = 0 ; i < natts ; i++) {
	 
    /* Get info about the current attribute  */
    attr_count = attr_dtype = 0;
    sz = H4_MAX_NC_NAME+1;
    if ((attname=(char*)galloc(sz,"hdatname"))==NULL) {
      gaprnt(0,"read_hdfatts error: memory allocation failed for attname\n");
      gree(varname,"f145a");
      return(Failure);
    }
    if (SDattrinfo(sds_id, i, attname, &attr_dtype, &attr_count) == -1) {
      snprintf(pout,1255,"SDattrinfo failed for variable %s, attribute number %d\n", varname, i);
      gaprnt(2,pout);
    }
    else {
      if (attr_count > 0) {
	len = attr_count;
	gotatt = 0;
	switch (attr_dtype) 
	  {
	  case (DFNT_CHAR8):    /* definition value 4 */
	    len = len + 1; 
	    sz = len * sizeof (char8);
	    cval = (char8*) galloc(sz,"catval");
	    if (SDreadattr(sds_id, i, cval) == -1) {
	      gaprnt(2,"SDreadattr failed for type CHAR8\n"); 
	      gree(cval,"f145"); cval=NULL;
	    }
	    else { 
	      gotatt=1; 
	      cval[len-1]='\0';
	    }
	    break;
	  case (DFNT_UCHAR8):   /* definition value 3 */
	    sz = len * sizeof (uchar8);
	    ucval = (uchar8*) galloc(sz,"ucatval");
	    if (SDreadattr(sds_id, i, ucval) == -1) { 
	      gaprnt(2,"SDreadattr failed for type UCHAR8\n"); 
	      gree(ucval,"f146"); ucval=NULL;
	    }
	    else { 
	      gotatt=1; 
	    }
	    break;
	  case (DFNT_INT8):     /* definition value 20 */
	    sz = len * sizeof (int8);
	    icval = (int8*) galloc(sz,"iatval");
	    if (SDreadattr(sds_id, i, icval) == -1) {
	      gaprnt(2,"SDreadattr failed for type INT8\n"); 
	      gree(icval,"f147"); icval=NULL;
	    }
	    else { 
	      gotatt=1; 
	    }
	    break;
	  case (DFNT_UINT8):    /* definition value 21 */
	    sz = len * sizeof (uint8);
	    uicval = (uint8*) galloc(sz,"uiatval");
	    if (SDreadattr(sds_id, i, uicval) == -1) {
	      gaprnt(2,"SDreadattr failed for type UINT8\n"); 
	      gree(uicval,"f148"); uicval=NULL;
	    }
	    else { 
	      gotatt=1; 
	    }
	    break;
	  case (DFNT_INT16):    /* definition value 22 */
	    sz = len * sizeof (int16);
	    sval = (int16*) galloc(sz,"satval");
	    if (SDreadattr(sds_id, i, sval) == -1) {
	      gaprnt(2,"SDreadattr failed for type INT16\n"); 
	      gree(sval,"f149"); sval=NULL;
	    }
	    else { 
	      gotatt=1; 
	    }
	    break;
	  case (DFNT_UINT16):   /* definition value 23 */
	    sz = len * sizeof (uint16);
	    usval = (uint16*) galloc(sz,"usatval");
	    if (SDreadattr(sds_id, i, usval) == -1) { 
	      gaprnt(2,"SDreadattr failed for type UINT16\n"); 
	      gree(usval,"f150"); usval=NULL;
	    }
	    else { 
	      gotatt=1; 
	    }
	    break;
	  case (DFNT_INT32):    /* definition value 24 */
	    sz = len * sizeof (int32);
	    ival = (int32*) galloc(sz,"latval");
	    if (SDreadattr(sds_id, i, ival) == -1) {
	      gaprnt(2,"SDreadattr failed for type INT32\n"); 
	      gree(ival,"f151"); ival=NULL;
	    }
	    else { 
	      gotatt=1; 
	    }
	    break;
	  case (DFNT_UINT32):   /* definition value 25 */
	    sz = len * sizeof (uint32);
	    uival = (uint32*) galloc(sz,"ulatval");
	    if (SDreadattr(sds_id, i, uival) == -1) { 
	      gaprnt(2,"SDreadattr failed for type UINT32\n"); 
	      gree(uival,"f151"); uival=NULL;
	    }
	    else { 
	      gotatt=1; 
	    }
	    break;
	  case (DFNT_FLOAT32):  /* definition value  5 */
	    sz = len * sizeof (float32);
	    fval = (float32*) galloc(sz,"fatval");
	    if (SDreadattr(sds_id, i, fval) == -1) {
	      gaprnt(2,"SDreadattr failed for type FLOAT32\n"); 
	      gree(fval,"f153"); fval=NULL;
	    }
	    else { 
	      gotatt=1; 
	    }
	    break;
	  case (DFNT_FLOAT64):  /* definition value  6 */
	    sz = len * sizeof (float64);
	    dval = (float64*) galloc(sz,"datval");
	    if (SDreadattr(sds_id, i, dval) == -1) {
	      gaprnt(2,"SDreadattr failed for type FLOAT64\n"); 
	      gree(dval,"f154"); dval=NULL;
	    }
	    else { 
	      gotatt=1; 
	    }
	    break;
	  default:
	    snprintf(pout,1255,"Failed to retrieve attribute %d of type %d \n", i, attr_dtype);
	    gaprnt(2,pout);
	  };
	
	if (gotatt) {
	  /* Successfully extracted the attribute, so add a link to the list */
	  sz = sizeof(struct gaattr);
	  if ((newattrib = (struct gaattr *) galloc(sz,"newathdf")) == NULL) {
	    snprintf(pout,1255,"read_hdfatts error: memory allocation failed when adding attribute number %d\n",i);
	    gaprnt(2,pout);
	    if (cval)   { gree(cval,"f145");   cval=NULL;   }
	    if (ucval)  { gree(ucval,"f146");  ucval=NULL;  }
	    if (icval)  { gree(icval,"f147");  icval=NULL;  }
	    if (uicval) { gree(uicval,"f148"); uicval=NULL; }
	    if (sval)   { gree(sval,"f149");   sval=NULL;   }
	    if (usval)  { gree(usval,"f150");  usval=NULL;  }
	    if (ival)   { gree(ival,"f151");   ival=NULL;   }
	    if (uival)  { gree(uival,"f151");  uival=NULL;  }
	    if (fval)   { gree(fval,"f153");   fval=NULL;   }
	    if (dval)   { gree(dval,"f154");   dval=NULL;   }
	  }
	  else {
	    if (pfi->attr) { /* some attributes already exist */
	      /* advance to end of chain */
	      attrib = pfi->attr;
	      while (attrib->next != NULL) attrib = attrib->next;
	      /* hang new attribute on end of chain */
	      attrib->next = newattrib;
	    }
	    else {
	      /* new attribute is the chain anchor */
	      pfi->attr = newattrib;
	    }
	    newattrib->next = NULL;
	    strcpy(newattrib->varname,varname);      
	    strcpy(newattrib->name,attname);      
	    newattrib->len  = len;
	    /* We're going to save HDF types as NC types */
	    /*	NC_BYTE =	1,	 signed 1 byte integer */
	    /*	NC_CHAR =	2,	 ISO/ASCII character */
	    /*	NC_SHORT =	3,	 signed 2 byte integer */
	    /*	NC_INT =	4,	 signed 4 byte integer */
	    /*	NC_FLOAT =	5,	 single precision floating point number */
	    /*	NC_DOUBLE =	6	 double precision floating point number */
	    if      (attr_dtype == DFNT_CHAR8)   { 
	      newattrib->value = cval;	
	      newattrib->nctype = 1;
	    }
	    else if (attr_dtype == DFNT_UCHAR8)  { 
	      newattrib->value = ucval;
	      newattrib->nctype = 2;
	    }
	    else if (attr_dtype == DFNT_INT8)    { 
	      newattrib->value = icval;
	      newattrib->nctype = 2;
	    }
	    else if (attr_dtype == DFNT_UINT8)   { 
	      newattrib->value = uicval; 
	      newattrib->nctype = 2;
	    }
	    else if (attr_dtype == DFNT_INT16)   { 
	      newattrib->value = sval;	
	      newattrib->nctype = 3;
	    }
	    else if (attr_dtype == DFNT_UINT16)  { 
	      newattrib->value = usval;
	      newattrib->nctype = 3;
	    }
	    else if (attr_dtype == DFNT_INT32)   { 
	      newattrib->value = ival;	
	      newattrib->nctype = 4;
	    }
	    else if (attr_dtype == DFNT_UINT32)  { 
	      newattrib->value = uival;
	      newattrib->nctype = 4;
	    }
	    else if (attr_dtype == DFNT_FLOAT32) { 
	      newattrib->value = fval;	
	      newattrib->nctype = 5;
	    } 
	    else if (attr_dtype == DFNT_FLOAT64) { 
	      newattrib->value = dval; 
	      newattrib->nctype = 6;
	    }
	  }
	}
      } /* end of if statement for attr_count > 0 */
    } /* end of if-else statement for getting attribute info */
    gree(attname,"155b");
  } /* end of for loop on i */
  gree(varname,"f155a");
  return (Success);
#endif
  return(Success);
}


/* read netcdf attribute information for a file or variable */
/* gaint read_ncatts (gaint cdfid, gaint varid, char *vname, gaint natts, struct gafile *pfi)  */
gaint read_ncatts (gaint cdfid, gaint varid, char *vname, gaint natts, struct gafile *pfi) 
{
#if USENETCDF == 1
struct gaattr *attrib=NULL,*newattrib=NULL;
gadouble *dval=NULL;
gafloat  *fval=NULL;
long   *ival=NULL;
short  *sval=NULL;
char   *bval=NULL;
char   *cval=NULL;
char  **strval=NULL;
char   *attname,*varname=NULL;
gaint   i,j,len,status,gotatt;
size_t  sz,attlen;
nc_type type;

  /* Get the variable name */
  if ((varid == NC_GLOBAL) && (vname == NULL)) 
    len=8;
  else 
    len = strlen(vname)+2;
  sz = len;
  if ((varname=(char*)galloc(sz,"ncvname"))==NULL) {
    gaprnt(0,"read_ncatts: memory allocation failed for varname\n");
    return(Failure);
  }
  if ((varid == NC_GLOBAL) && (vname == NULL)) {
    strncpy(varname,"global",7);
  }
  else {
    strncpy(varname,vname,len);
  }
  
  /* Loop through all attributes */
  for (i=0 ; i<natts ; i++) {
    /* Get current attribute's name */
    sz = MAX_NC_NAME+2;
    if ((attname=(char*)galloc(sz,"ncattname"))==NULL) {
      gaprnt(0,"read_ncatts: memory allocation failed for attname\n");
      gree(varname,"f55c");
      return(Failure);
    }
    status = nc_inq_attname(cdfid, varid, i, attname);
    if (status != NC_NOERR) { 
      handle_error(status); 
      snprintf(pout,1255,"read_ncatts: ncattname failed for varid %d attribute number %d\n",varid,i);
      gaprnt(2,pout);
    }
    else {
      attlen=0; type=0;
      /* Get current attribute's data type and length */
      status = nc_inq_att(cdfid, varid, attname, &type, &attlen);
      if (status != NC_NOERR) { 
	handle_error(status); 
	snprintf(pout,1255,"read_ncatts: nc_inq_att failed for varid %d attribute number %d\n",varid,i);
	gaprnt(2,pout);
      }
      else {
        if (attlen > 0) {
	  gotatt=0;
	  /* Retrieve the attribute's value */
	  switch (type) 
	    {
	    case (NC_BYTE):
	      sz = attlen*sizeof(char);
	      bval = (char *) galloc(sz,"bval");
  	      status = nc_get_att_schar(cdfid, varid, attname, (signed char*)bval);
              if (status != NC_NOERR) { 
		gree(bval,"f22"); 
		bval = NULL;
		handle_error(status); 
		snprintf(pout,1255,"read_ncatts: failed to get %s attribute %d type BYTE\n",varname,i);
		gaprnt(2,pout);
	      }
	      else { 
		gotatt=1; 
	      }
	      break;
	    case (NC_CHAR):
	      attlen = attlen + 1;
	      sz = attlen*sizeof(char);
	      cval = (char *) galloc(sz,"cval");
	      status = nc_get_att_text(cdfid, varid, attname, cval);
              if (status != NC_NOERR) { 
		gree(cval,"f24");
		cval = NULL;
		handle_error(status); 
		snprintf(pout,1255,"read_ncatts: failed to get %s attribute %d type CHAR\n",varname,i);
		gaprnt(2,pout);
	      }
	      else { 
		gotatt=1; 
		cval[attlen-1]='\0';
	      }
	      break;
	    case (NC_STRING):
              strval = (char**)galloc(attlen * sizeof(char*),"strval");
	      memset(strval, 0, attlen * sizeof(char*));  /* fill with zeros as in nc doc example */
	      status = nc_get_att_string(cdfid, varid, attname, strval); 
              if (status != NC_NOERR) { 
		gree(strval,"f24b"); strval = NULL;
		handle_error(status); 
		snprintf(pout,1255,"read_ncatts: failed to get %s attribute %d type STRING\n",varname,i);
		gaprnt(2,pout);
	      }
	      else { 
		gotatt=1; 
		/* NC_STRING attribute is an array (size attlen) of pointers to variable length strings. 
                   We only look at the first one, strval[0], and don't check if attlen>1.
		   We get the length of the first string, copy it into cval, then free the array */
                sz = 1 + (int)strlen(strval[0]);
	        cval = (char *)galloc(sz*sizeof(char),"cval");
                for (j=0; j<sz; j++) cval[j] = strval[0][j];
		cval[sz]='\0';
                nc_free_string(attlen, strval);
		gree(strval);
	      }
	      break;
	    case (NC_SHORT):
	      sz = attlen * sizeof(short);
	      sval = (short *) galloc(sz,"sval");
	      status = nc_get_att_short(cdfid, varid, attname, sval);
              if (status != NC_NOERR) { 
		gree(sval,"f26");
		sval=NULL;
		handle_error(status); 
		snprintf(pout,1255,"read_ncatts: failed to get %s attribute %d type SHORT\n",varname,i);
		gaprnt(2,pout);
	      }
	      else { 
		gotatt=1; 
	      }
	      break;
	    case (NC_LONG):
	      sz = attlen * sizeof(long);
	      ival = (long *) galloc(sz,"ival");
	      status = nc_get_att_long(cdfid, varid, attname, ival);
              if (status != NC_NOERR) { 
		gree(ival,"f28");
		ival = NULL;
		handle_error(status); 
		snprintf(pout,1255,"read_ncatts: failed to get %s attribute %d type LONG\n",varname,i);
		gaprnt(2,pout);
	      }
	      else { 
		gotatt=1; 
	      }
	      break;
	    case (NC_FLOAT):
	      sz = attlen * sizeof(gafloat);
	      fval = (gafloat *) galloc(sz,"fval");
	      status = nc_get_att_float(cdfid, varid, attname, fval);
              if (status != NC_NOERR) {
		gree(fval,"f30");
		fval = NULL;
		handle_error(status); 
		snprintf(pout,1255,"read_ncatts: failed to get %s attribute %d type FLOAT\n",varname,i);
		gaprnt(2,pout);
	      }
	      else { 
		gotatt=1; 
	      }
	      break;
	    case (NC_DOUBLE): 
	      sz = attlen * sizeof(gadouble);
	      dval = (gadouble *) galloc(sz,"dval");
	      status = nc_get_att_double(cdfid, varid, attname, dval);
              if (status != NC_NOERR) { 
		gree(dval,"f32");
		dval = NULL;
		handle_error(status); 
		snprintf(pout,1255,"read_ncatts: failed to get %s attribute %d type DOUBLE\n",varname,i);
		gaprnt(2,pout);
	      }
	      else { 
		gotatt=1; 
	      }
            break;
	    default:
	      snprintf(pout,1255,"read_ncatts: %s attribute %d type %d not supported\n",varname,i,type);
	      gaprnt(2,pout);
	    };
	  
	  if (gotatt) {
	    /* Successfully extracted the attribute, so add a link to the list */
	    sz = sizeof(struct gaattr);
	    if ((newattrib = (struct gaattr *) galloc(sz,"newattr")) == NULL) {
	      snprintf(pout,1255,"read_ncatts: memory allocation failed when adding attribute number %d\n",i);
	      gaprnt(2,pout);
	      if (bval) { gree(bval,"f33"); bval = NULL; }
	      if (cval) { gree(cval,"f34"); cval = NULL; }
	      if (sval) { gree(sval,"f35"); sval = NULL; }
	      if (ival) { gree(ival,"f36"); ival = NULL; }
	      if (fval) { gree(fval,"f37"); fval = NULL; }
	      if (dval) { gree(dval,"f38"); dval = NULL; }
	    }
	    else { 
	      if (pfi->attr) { /* some attributes already exist */
		/* advance to end of chain */
		attrib = pfi->attr;
		while (attrib->next != NULL) attrib = attrib->next;
		/* hang new attribute on end of chain */
		attrib->next = newattrib;
	      }
	      else {
		/* new attribute is first link in chain */
		pfi->attr = newattrib;
	      }
	      newattrib->next = NULL;
	      strcpy(newattrib->varname,varname);
	      strcpy(newattrib->name,attname);
	      newattrib->len  = attlen;
	      newattrib->nctype = (gaint)type;
	      if      (type == NC_BYTE)   newattrib->value = bval;
	      else if (type == NC_CHAR)   newattrib->value = cval;
	      else if (type == NC_STRING) newattrib->value = cval;
	      else if (type == NC_SHORT)  newattrib->value = sval;
	      else if (type == NC_LONG)   newattrib->value = ival;
	      else if (type == NC_FLOAT)  newattrib->value = fval;
	      else if (type == NC_DOUBLE) newattrib->value = dval;
	    }
	  } /* end of if (gotatt) statement */
	} /* end of if statement for attlen > 0 */
      } /* end of if-else statement for getting attribute type and length */
    } /* end of if-else statement for getting attribute name */
    gree(attname,"f39a");
  } /* end of for loop on i */
  gree(varname,"f39b");
  return (Success);
#endif 
}


/* find a dimension id, given the dimension name */
gaint find_dim (struct gafile *pfi, char *name) {
  gaint i;
  for (i=0; i<pfi->nsdfdims; i++) {
    if (!strcmp (pfi->sdfdimnam[i], name)) return(pfi->sdfdimids[i]);
  }
  return (-1);
}


/* Reads one dimension axis value. 
   Used to determine start and increment for time axis setup. */
gaint read_one_dimension (struct gafile *pfi, struct gavar *coord, 
			gaint start, gaint count, gadouble *data) {
  gaint rc;
  gadouble ddata;
  size_t st,cnt;

#if USEHDF == 1 
  int32 lst,lcnt,sds_id,dtype,rank,dim_sizes[H4_MAX_NC_DIMS],natts;
  float32  fdata;
  int32    idata;
  uint32   uidata;
 

  if (pfi->ncflg==2) {
    lst=start;
    lcnt=count;
    /* get the data type */
    if ((sds_id = SDselect(pfi->sdid,coord->sdvid))==FAIL) return Failure;
    rc = SDgetinfo(sds_id, coord->longnm, &rank, dim_sizes, &dtype, &natts);
    if (rc == -1) {
      gaprnt(0,"sdfdeflev: unable to determine coordinate axis data type\n");
      return Failure;
    }
    switch (dtype) {
    case (DFNT_INT32):  
      if ((SDreaddata (sds_id, &lst, NULL, &lcnt, (VOIDP *)&idata)) != 0) {
	gaprnt(0,"SDF Error: SDreaddatda failed to read coordinate axis value \n");
	return Failure;
      }
      *data = (gadouble)idata; 
      break;
    case (DFNT_UINT32):  
      if ((SDreaddata (sds_id, &lst, NULL, &lcnt, (VOIDP *)&uidata)) != 0) {
	gaprnt(0,"SDF Error: SDreaddatda failed to read coordinate axis value \n");
	return Failure;
      }
      *data = (gadouble)uidata; 
      break;
    case (DFNT_FLOAT32):
      if ((SDreaddata (sds_id, &lst, NULL, &lcnt, (VOIDP *)&fdata)) != 0) {
	gaprnt(0,"SDF Error: SDreaddatda failed to read coordinate axis value \n");
	return Failure;
      }
      *data = (gadouble)fdata; 
      break;
    case (DFNT_FLOAT64):
      if ((SDreaddata (sds_id, &lst, NULL, &lcnt, (VOIDP *)&ddata)) != 0) {
	gaprnt(0,"SDF Error: SDreaddatda failed to read coordinate axis value \n");
	return Failure;
      }
      *data = ddata;
      break;
    default:
      snprintf(pout,1255,"SDF coordinate axis data type %d not handled\n",dtype);
      gaprnt(0,pout);
      return Failure;
    };
  }
#endif

#if USENETCDF == 1
  if (pfi->ncflg==1) {
    st=start;
    cnt=count;
    rc = nc_get_vara_double(pfi->ncid, coord->ncvid, &st, &cnt, &ddata);
    if (rc != NC_NOERR) {
      handle_error(rc);
      gaprnt(0,"SDF Error: nc_get_vara_double failed to read coordinate axis value \n");
      return Failure;
    }
    *data = ddata;
  }
#endif
  return Success ;
}


/* find a variable pointer, given the variable name */
struct gavar *find_var (struct gafile *pfi, char *varname) {
  gaint i;
  struct gavar *pvar;
  i=0;
  pvar = pfi->pvar1;
  while (i < pfi->vnum) {
    if (!strcmp(varname, pvar->longnm)) return (struct gavar *)pvar;
    i++; pvar++;
  }
  return NULL;  
}


gaint decode_delta_t (char *delta_t_str, gaint *year, gaint *month, gaint *day, gaint *hour, gaint *minn, gaint *sec) {
  char temp_str[100];
  gaint delta_t_len;
  gaint year_mark = 4, month_mark = 7, day_mark = 10;
  gaint hour_mark = 13, minute_mark = 16, second_mark = 19;

  *year = *month = *day = *hour = *minn = *sec = MISSING;

  delta_t_len = strlen (delta_t_str);
  if ((delta_t_len > day_mark)    && (delta_t_str[day_mark] != ' '))    return Failure;
  if ((delta_t_len > hour_mark)   && (delta_t_str[hour_mark] != ':'))   return Failure;
  if ((delta_t_len > minute_mark) && (delta_t_str[minute_mark] != ':')) return Failure;

  /* Get year. */
  strcpy (temp_str, delta_t_str);
  temp_str[year_mark] = '\0';
  sscanf (temp_str, "%d", year);

  /* Get month. */
  strcpy (temp_str, &delta_t_str[year_mark + 1]);
  temp_str[month_mark - year_mark - 1] = '\0';
  sscanf (temp_str, "%d", month);

  /* Get day. */
  strcpy (temp_str, &delta_t_str[month_mark + 1]);
  temp_str[day_mark - month_mark - 1] = '\0';
  sscanf (temp_str, "%d", day);

  /* Get other fields if present. */
  if (delta_t_len > day_mark) {

    /* Get hour. */
    strcpy (temp_str, &delta_t_str[day_mark + 1]);
    temp_str[hour_mark - day_mark - 1] = '\0';
    sscanf (temp_str, "%d", hour);
    
    /* Get minute. */
    strcpy (temp_str, &delta_t_str[hour_mark + 1]);
    temp_str[minute_mark - hour_mark - 1] = '\0';
    sscanf (temp_str, "%d", minn);
    
    /* Get second. */
    strcpy (temp_str, &delta_t_str[minute_mark + 1]);
    temp_str[second_mark - minute_mark - 1] = '\0';
    sscanf (temp_str, "%d", sec);
    
  }
  else {
    *hour = *minn = *sec = MISSING;
  }
  return Success;
}


/* Handle return codes */
void handle_error(gaint status) {
#if USENETCDF==1
  snprintf(pout,1255," %s\n",nc_strerror(status));
  gaprnt(0,pout);
#endif
}

gaint gadxdf(struct gafile *pfi, GASDFPARMS *parms) 
{
struct gaens *ens;
struct gachsub *pchsub;
struct sdfnames *varnames=NULL;
struct dt tdef,tdefi,tdefe,dt1,dt2;
gadouble *tvals,*evals,v1,v2,temp;
gaint rc,len,ichar,tim1,tim2;
gaint flgs[1],i,j,ii,jj,t,e,reclen,err,flag;
char rec[512], mrec[512], *ch, *pos, *sname; 
size_t sz;
  
  /* Initialize variables */
  sname=NULL;
  initparms(parms);
  parms->isxdf = 1 ;
  
  /* Open descriptor file */
  descr = fopen (pfi->dnam, "r");  
  if (descr == NULL) {
    /* Add default suffix of .ctl */
    sz = strlen(pfi->dnam)+5;
    sname = (char *)galloc(sz,"sname");
    if (sname == NULL) {
      gaprnt(0,"gadxdf: memory allocation error in creating data descriptor file name\n");
      return Failure;
    }
    for(i=0;i<=strlen(pfi->dnam);i++) *(sname+i)=*(pfi->dnam+i);
    strcat(sname,".ctl");
    descr = fopen (sname, "r");
  }
  
  if (descr == NULL) {
    gaprnt (0,"gadxdf: Can't open description file\n");
    if (sname) gree(sname,"f45");
    return Failure;
  }
  
  /* Copy modified descriptor file name into gafile structure */
  if (sname != NULL) {
    getwrd (pfi->dnam,sname,255);
    if (sname) gree(sname,"f46");
  } 
  
  /* initialize variables */
  for (i=0;i<1;i++) flgs[i] = 1;
  
  /* Parse the descriptor file */
  pfi->vnum = 0 ;
  while (fgets(rec,512,descr)!=NULL) {
    
    /* Remove any leading blanks from rec */
    reclen = strlen(rec);
    jj = 0;
    while (jj<reclen && rec[0]==' ') {
      for (ii=0; ii<reclen; ii++) rec[ii] = rec[ii+1];
      jj++;
    }
    /* replace newline with null at end of record */
    for (ichar = strlen(rec) - 1 ;  ichar >= 0 ;  --ichar) {
      if (rec[ichar] == '\n') {
	rec[ichar] = '\0' ;
	break ; 
      }
    }
    /* Keep mixed case and lower case versions of rec handy */
    strcpy (mrec,rec);
    lowcas(rec);
    
    /* Parse comment -- check for attribute metadata */
    if (!isalnum(*(mrec))) {
      if ((strncmp("*:attr",mrec,6)==0) || (strncmp("@",mrec,1)==0)) {
	if ((ddfattr(mrec,pfi)) == -1) goto retrn;
      }
    } 
    
    /* Parse OPTIONS */
    else if (cmpwrd("options",rec)) {
      if ((ch=nxtwrd(rec))!=NULL) {
        while (ch != NULL) {
          if (cmpwrd("yrev",ch)) pfi->yrflg = 1;
          else if (cmpwrd("zrev",ch)) pfi->zrflg = 1;
          else if (cmpwrd("template",ch)) pfi->tmplat = 1; 
          else if (cmpwrd("365_day_calendar",ch)) {
	    pfi->calendar=1;
	    mfcmn.cal365=pfi->calendar;
	  }
	  else {
	    gaprnt (0,"gadxdf error: invalid options keyword\n");
	    goto err9;
          }
          ch = nxtwrd(ch);
        }
      }
    } 
    
    /* Parse TITLE */
    else if (cmpwrd("title",rec)) {
      parms->needtitle = 0 ;
      if ((ch=nxtwrd(mrec))==NULL) {
        gaprnt (1,"gadxdf warning: missing title string\n");
	pfi->title[0] = '\0' ;
      } else {
        getstr (pfi->title,ch,511);
      }
    }
    
    /* Parse DTYPE */
    else if (cmpwrd("dtype",rec)) {
      if ((ch=nxtwrd(rec))==NULL ) pfi->ncflg = 1;   /* default to netcdf */
      else if (cmpwrd("netcdf",ch)) pfi->ncflg = 1;
      else if (cmpwrd("hdfsds",ch) || cmpwrd("hdf4",ch)) pfi->ncflg = 2;
      else {
        gaprnt (0,"gadxdf Error:  Data file type invalid\n");
        goto err9;
      }
    }

    /* Parse DSET */
    else if (cmpwrd("dset",rec)) {
      ch = nxtwrd(mrec);
      if (ch==NULL) {
        gaprnt (0,"gadxdf error: data file name is missing\n");
        goto err9;
      }
      if (*ch=='^' || *ch=='$') {
        fnmexp (pfi->name,ch,pfi->dnam);
      } else {
        getwrd (pfi->name,ch,511);
      }
      flgs[0] = 0;
    } 
    
    /* Parse CHSUB records.  time1, time2, then a string,  multiple times */
    else if (cmpwrd("chsub",rec)) {
      /* point to first block in chain */
      pchsub = pfi->pchsub1;    
      if (pchsub!=NULL) {
        while (pchsub->forw!=NULL) {
          pchsub = pchsub->forw;       /* advance to end of chain */
        }
      }
      flag = 0;
      ch = mrec;
      while (1) {
        if ( (ch=nxtwrd(ch)) == NULL ) break;
        flag = 1;
        if ( (ch = intprs(ch,&tim1)) == NULL) break;
        if ( (ch=nxtwrd(ch)) == NULL ) break;
        if (*ch=='*' && (*(ch+1)==' '||*(ch+1)=='\t')) tim2 = -99;
        else if ( (ch = intprs(ch,&tim2)) == NULL) break;
        if ( (ch=nxtwrd(ch)) == NULL ) break;
        flag = 0;
        if (pchsub) {   /* chain exists */
	  sz = sizeof(struct gachsub);
          pchsub->forw = (struct gachsub *)galloc(sz,"chsub2");
          if (pchsub->forw==NULL) {
	    gaprnt(0,"gadxdf error: memory allocation failed for pchsub\n");
	    goto err8; 
	  }
          pchsub = pchsub->forw;
	  pchsub->forw = NULL;
        } else {        /* start a new chain */
	  sz = sizeof(struct gachsub);
          pfi->pchsub1 = (struct gachsub *)galloc(sz,"chsub3");
          if (pfi->pchsub1==NULL)  {
	    gaprnt(0,"gadxdf error: memory allocation failed for pchsub1\n");
	    goto err8; 
	  }
          pchsub = pfi->pchsub1;
	  pchsub->forw = NULL;
        }
        len = wrdlen(ch);
	sz = len+1;
        if ((pchsub->ch = (char *)galloc(sz,"chsub4")) == NULL) goto err8;
        getwrd(pchsub->ch,ch,len);
        pchsub->t1 = tim1;
        pchsub->t2 = tim2;
      }
      if (flag) {
        gaprnt (1,"gadxdf warning: Invalid chsub record; Ignored\n");
      }
    }

    /* Parse UNDEF */
    else if (cmpwrd("undef",rec)) {
      ch = nxtwrd(mrec);
      if (ch==NULL) {
        gaprnt (0,"gadxdf error: missing undef value\n");
        goto err9;
      }
      pos = getdbl(ch,&(pfi->undef));
      if (pos==NULL) {
        gaprnt (0,"gadxdf error: invalid undef value\n");
        goto err9;
      }
      /* Get the undef attribute name, if it's there */
      if ((ch=nxtwrd(ch))!=NULL) {
	len = 0;
	while (*(ch+len)!=' ' && *(ch+len)!='\n' && *(ch+len)!='\t') len++;
	sz = len+1;
	if ((pfi->undefattr = (char *)galloc(sz,"undefattr5")) == NULL) goto err8;
	for (i=0; i<len; i++) *(pfi->undefattr+i) = *(ch+i);
	*(pfi->undefattr+len) = '\0';
	/* Set the undef attribute flag */
	pfi->undefattrflg = 1;
        /* Get the secondary undef attribute name, if it's there */
        if ((ch=nxtwrd(ch))!=NULL) {
	  len = 0;
	  while (*(ch+len)!=' ' && *(ch+len)!='\n' && *(ch+len)!='\t') len++;
	  sz = len+1;
	  if ((pfi->undefattr2 = (char *)galloc(sz,"undefattr6")) == NULL) goto err8;
	  for (i=0; i<len; i++) *(pfi->undefattr2+i) = *(ch+i);
	  *(pfi->undefattr2+len) = '\0';
	  /* uptick the undef attribute flag */
	  pfi->undefattrflg = 2;
        }
      }
      pfi->ulow = fabs(pfi->undef/EPSILON);
      pfi->uhi  = pfi->undef + pfi->ulow;
      pfi->ulow = pfi->undef - pfi->ulow;
      parms->needundef = 0 ;
      parms->hasDDFundef = 1 ;
    } 
      
    /* Parse XDEF */
    else if (cmpwrd("xdef",rec)) {
      if (pfi->type == 2) continue;
      if ((ch = nxtwrd(mrec)) == NULL) goto err0;   /* xdimname must be mixed case version*/
      parms->xsrch = 0 ;                    
      /* Copy the X dimension name into parms structure*/
      len = 0;
      while (*(ch+len)!=' ' && *(ch+len)!='\n' && *(ch+len)!='\t') len++;
      sz = len+1;
      if ((parms->xdimname = (char *)galloc(sz,"xdimname")) == NULL) goto err8;
      for (i=0; i<len; i++) *(parms->xdimname+i) = *(ch+i);
      *(parms->xdimname+len) = '\0';
      ch = nxtwrd(rec) ;                             /* skip over xdef in lowcase version */
      if ((ch = nxtwrd(ch)) == NULL) {
        parms->xsetup = 1 ;                    
      } 
      else {
        if ((pos = intprs(ch,&(pfi->dnum[0])))==NULL) goto err1;
	if (pfi->dnum[0]<1) {
	  snprintf(pout,1255,"Warning: Invalid XDEF syntax in %s -- Changing size of X axis from %d to 1 \n",
		  pfi->dnam,pfi->dnum[0]);
	  gaprnt (1,pout);
	  pfi->dnum[0] = 1;
	}
        if (*pos != ' ') goto err1;
        if ((ch = nxtwrd(ch))==NULL) goto err2;
        if (cmpwrd("linear",ch)) {
	  rc = deflin(ch, pfi, 0, 0);
	  if (rc==-1) goto err8;
	  if (rc) goto err9;
	  /* Check if grid wraps around the globe */
	  v2 = *(pfi->grvals[0]);
	  v1 = *(pfi->grvals[0]+1) + v2;
	  temp = v1+(pfi->dnum[0])*v2;
	  temp=temp-360.0;
	  if (fabs(temp-v1)<0.01) pfi->wrap = 1;
        } 
	else if (cmpwrd("levels",ch)) {
	  rc = deflev (ch, rec, pfi, 0);
	  if (rc==-1) goto err8;
	  if (rc) goto err9;
        } else goto err2;
        parms->xsetup = 0 ;
      }
    } 

    /* Parse YDEF */
    else if (cmpwrd("ydef",rec)) {
      if (pfi->type == 2) continue;
      if ((ch = nxtwrd(mrec)) == NULL) goto err0;   /* ydimname must be mixed case version*/
      parms->ysrch = 0 ;
      /* Copy the Y dimension name into parms structure*/
      len = 0;
      while (*(ch+len)!=' ' && *(ch+len)!='\n' && *(ch+len)!='\t') len++;
      sz = len+1;
      if ((parms->ydimname = (char *)galloc(sz,"ydimname")) == NULL) goto err8;
      for (i=0; i<len; i++) *(parms->ydimname+i) = *(ch+i);
      *(parms->ydimname+len) = '\0';
      ch = nxtwrd(rec) ;                             /* skip over ydef in lowcase version*/
      if ((ch = nxtwrd(ch))  == NULL) { 
        parms->ysetup = 1 ;
      } 
      else {
        if ((pos = intprs(ch,&(pfi->dnum[1])))==NULL) goto err1 ;
	if (pfi->dnum[1]<1) {
	  snprintf(pout,1255,"Warning: Invalid YDEF syntax in %s -- Changing size of Y axis from %d to 1 \n",
		  pfi->dnam,pfi->dnum[1]);
	  gaprnt (1,pout);
	  pfi->dnum[1] = 1;
	}
        if (*pos!=' ') goto err1;
        if ((ch = nxtwrd(ch))==NULL) goto err2;
        if (cmpwrd("linear",ch)) {
	  rc = deflin(ch, pfi, 1, 0);
	  if (rc==-1) goto err8;
	  if (rc) goto err9;
        } else if (cmpwrd("levels",ch)) {
	  rc = deflev (ch, rec, pfi, 1);
	  if (rc==-1) goto err8;
	  if (rc) goto err9;
        } else if (cmpwrd("gausr40",ch)) {
	  if ((ch = nxtwrd(ch))==NULL) goto err3;
	  if ((pos = intprs(ch,&i))==NULL) goto err3;
	  pfi->grvals[1] = gagaus(i,pfi->dnum[1]);
	  if (pfi->grvals[1]==NULL) goto err9;
	  pfi->abvals[1] = pfi->grvals[1];
	  pfi->ab2gr[1] = lev2gr;
	  pfi->gr2ab[1] = gr2lev;
	  pfi->linear[1] = 0;
        } else if (cmpwrd("mom32",ch)) {
	  if ((ch = nxtwrd(ch))==NULL) goto err3;
	  if ((pos = intprs(ch,&i))==NULL) goto err3;
	  pfi->grvals[1] = gamo32(i,pfi->dnum[1]);
	  if (pfi->grvals[1]==NULL) goto err9;
	  pfi->abvals[1] = pfi->grvals[1];
	  pfi->ab2gr[1] = lev2gr;
	  pfi->gr2ab[1] = gr2lev;
	  pfi->linear[1] = 0;
        } else if (cmpwrd("gausr30",ch)) {
	  if ((ch = nxtwrd(ch))==NULL) goto err3;
	  if ((pos = intprs(ch,&i))==NULL) goto err3;
	  pfi->grvals[1] = gags30(i,pfi->dnum[1]);
	  if (pfi->grvals[1]==NULL) goto err9;
	  pfi->abvals[1] = pfi->grvals[1];
	  pfi->ab2gr[1] = lev2gr;
	  pfi->gr2ab[1] = gr2lev;
	  pfi->linear[1] = 0;
        } else if (cmpwrd("gausr20",ch)) {
	  if ((ch = nxtwrd(ch))==NULL) goto err3;
	  if ((pos = intprs(ch,&i))==NULL) goto err3;
	  pfi->grvals[1] = gags20(i,pfi->dnum[1]);
	  if (pfi->grvals[1]==NULL) goto err9;
	  pfi->abvals[1] = pfi->grvals[1];
	  pfi->ab2gr[1] = lev2gr;
	  pfi->gr2ab[1] = gr2lev;
	  pfi->linear[1] = 0;
        } else if (cmpwrd("gausr15",ch)) {
	  if ((ch = nxtwrd(ch))==NULL) goto err3;
	  if ((pos = intprs(ch,&i))==NULL) goto err3;
	  pfi->grvals[1] = gags15(i,pfi->dnum[1]);
	  if (pfi->grvals[1]==NULL) goto err9;
	  pfi->abvals[1] = pfi->grvals[1];
	  pfi->ab2gr[1] = lev2gr;
	  pfi->gr2ab[1] = gr2lev;
	  pfi->linear[1] = 0;
        } else goto err2;
        parms->ysetup = 0 ;
      }
    } 

    /* Parse ZDEF */
    else if (cmpwrd("zdef",rec)) {
      if ((ch = nxtwrd(mrec)) == NULL) goto err0; /* get mixed case version */
      parms->zsrch = 0 ;
      /* Copy the Z dimension name into parms structure*/
      len = 0;
      while (*(ch+len)!=' ' && *(ch+len)!='\n' && *(ch+len)!='\t') len++;
      sz = len+1;
      if ((parms->zdimname = (char *)galloc(sz,"zdimname")) == NULL) goto err8;
      for (i=0; i<len; i++) *(parms->zdimname+i) = *(ch+i);
      *(parms->zdimname+len) = '\0';
      ch = nxtwrd(rec) ;                           /* point past zdef in lowcased version */
      if ((ch = nxtwrd(ch))  == NULL) {
        parms->zsetup = 1 ;
      } 
      else {
        if ((pos = intprs(ch,&(pfi->dnum[2])))==NULL) goto err1 ;
	if (pfi->dnum[2]<1) {
	  snprintf(pout,1255,"Warning: Invalid ZDEF syntax in %s -- Changing size of Z axis from %d to 1 \n",
		  pfi->dnam,pfi->dnum[2]);
	  gaprnt (1,pout);
	  pfi->dnum[2] = 1;
	}
        if (*pos!=' ') goto err1;
        if ((ch = nxtwrd(ch))==NULL) goto err2;
        if (cmpwrd("linear",ch)) {
	  rc = deflin(ch, pfi, 2, 0);
	  if (rc==-1) goto err8;
	  if (rc) goto err9;
        } else if (cmpwrd("levels",ch)) {
	  rc = deflev (ch, rec, pfi, 2);
	  if (rc==-1) goto err8;
	  if (rc) goto err9;
        } else goto err2;
        parms->zsetup = 0 ;
      }
    } 

    /* Parse TDEF */
    else if (cmpwrd("tdef",rec)) {
      if ((ch = nxtwrd(mrec)) == NULL) goto err0; /* get mixed case version */
      parms->tsrch = 0 ;
      if (!strncasecmp(ch, "%nodim%", 7)) {
        parms->tdimname = NULL ;             /* we won't be using any tdimname */
	pfi->dnum[TINDEX] = 1 ;              /* 1 time step ; be sure not to map any tdim */
      } 
      else {
	/* Copy the T dimension name into parms structure*/
	len = 0;
	while (*(ch+len)!=' ' && *(ch+len)!='\n' && *(ch+len)!='\t') len++;
	sz = len+1;
	if ((parms->tdimname = (char *)galloc(sz,"tdimname")) == NULL) goto err8;
	for (i=0; i<len; i++) *(parms->tdimname+i) = *(ch+i);
	*(parms->tdimname+len) = '\0';
      }
      ch = nxtwrd(rec) ;                           /* skip over tdef in lowcased version */
      if ((ch = nxtwrd(ch)) == NULL) {
        if (parms->tdimname == NULL) {
	  sz = sizeof(gadouble)*8;
	  if ((tvals = (gadouble *)galloc(sz,"tvals3")) == NULL) goto err8;
	  tvals[0] = 1.0 ;
	  tvals[1] = 1.0 ;
	  tvals[2] = 1.0 ;
	  tvals[3] = 0.0 ; /* initial hours */
	  tvals[4] = 0.0 ;
	  tvals[5] = 0.0 ; /* step in months */
	  tvals[6] = 1.0 ; /* step in minutes */
	  tvals[7] = -999.9 ;
	  pfi->grvals[TINDEX] = tvals ;
	  pfi->abvals[TINDEX] = tvals ;
	  pfi->linear[TINDEX] = 1 ;
	  parms->tsetup = 0 ;
        } 
	else {
	  parms->tsetup = 1 ;
        }
      } 
      else { 
        if ((pos = intprs(ch,&(pfi->dnum[3])))==NULL) goto err1 ;
	if (parms->tdimname == NULL) {
	  /*  %nodim% case can only have 1 timestep */
	  if (pfi->dnum[3] != 1) {
	    gaprnt(0, "TDEF with %nodim% has timestep count != 1; resetting to 1.\n") ;
	    pfi->dnum[3] = 1 ;
 	  }
	}
	else if (pfi->dnum[3]<1) {
	  snprintf(pout,1255,"Warning: Invalid TDEF syntax in %s -- Changing size of T axis from %d to 1 \n",
		  pfi->dnam,pfi->dnum[3]);
	  gaprnt (1,pout);
	  pfi->dnum[3] = 1;
	}
        if (*pos!=' ') goto err1;
        if ((ch = nxtwrd(ch))==NULL) goto err2;
        if (cmpwrd("linear",ch)) {
	  if ((ch = nxtwrd(ch))==NULL) goto err3;
	  tdef.yr = -1000;
	  tdef.mo = -1000;
	  tdef.dy = -1000;
	  if ((pos = adtprs(ch,&tdef,&dt1))==NULL) goto err3;
	  if (*pos!=' ' || dt1.yr == -1000 || dt1.mo == -1000.0 || dt1.dy == -1000) goto err3;
	  if ((ch = nxtwrd(ch))==NULL) goto err4;
	  if ((pos = rdtprs(ch,&dt2))==NULL) goto err4;
	  v1 = (dt2.yr * 12) + dt2.mo;
	  v2 = (dt2.dy * 1440) + (dt2.hr * 60) + dt2.mn;
	  if (dequal(v1, 0.0, 1.0e-08)==0 && dequal(v2, 0.0, 1.0e-08)==0) goto err4a ;
	  sz = sizeof(gadouble)*8;
	  if ((tvals = (gadouble *)galloc(sz,"tvals4")) == NULL) goto err8;
	  *(tvals) = dt1.yr;
	  *(tvals+1) = dt1.mo;
	  *(tvals+2) = dt1.dy;
	  *(tvals+3) = dt1.hr;
	  *(tvals+4) = dt1.mn;
	  *(tvals+5) = v1;
	  *(tvals+6) = v2;
	  *(tvals+7) = -999.9;
	  pfi->grvals[3] = tvals;
	  pfi->abvals[3] = tvals;
	  pfi->linear[3] = 1;
        } else goto err2;
        parms->tsetup = 0 ;
      }
    } 

    /* Parse EDEF */
    else if (cmpwrd("edef",rec)) {
      if ((ch = nxtwrd(mrec)) == NULL) goto err1;   /* get mixed case version */
      parms->esrch  = 0;  /* got the coordinate variable name */
      /* copy the E dimension name into parms structure */
      len = 0;
      while (*(ch+len)!=' ' && *(ch+len)!='\n' && *(ch+len)!='\t') len++;
      sz = len+1;
      if ((parms->edimname = (char *)galloc(sz,"edimname")) == NULL) {
	gaprnt(0,"Unable to allocate memory for E coordinate axis name\n");
	goto err8;
      }
      for (i=0; i<len; i++) *(parms->edimname+i) = *(ch+i);
      *(parms->edimname+len) = '\0';
      parms->esetup = 3;       /* still need size, ensemble names, time metadata */
      ch = nxtwrd(rec) ;                           /* point past edef in lowcased version */
      if ((ch = nxtwrd(ch)) != NULL) {
	if ((pos = intprs(ch,&(pfi->dnum[EINDEX]))) == NULL) goto err1;
	if (pfi->dnum[EINDEX]<1) {
	  snprintf(pout,1255,"Warning: Invalid EDEF syntax in %s -- Changing size of E axis from %d to 1 \n",
		  pfi->dnam,pfi->dnum[EINDEX]);
	  gaprnt (1,pout);
	  pfi->dnum[EINDEX] = 1;
	}
	/* set up linear scaling */
	sz = sizeof(gadouble)*6;
	if ((evals = (gadouble *)galloc(sz,"evals")) == NULL) {
	  gaprnt(0,"gadxdf: memory allocation failed for ensemble dimension scaling values\n");
	  goto err1;
	}
	v1=v2=1;
	*(evals+1) = v1 - v2;
	*(evals) = v2;
	*(evals+2) = -999.9;
	*(evals+4) = -1.0 * ( (v1-v2)/v2 );
	*(evals+3) = 1.0/v2;
	*(evals+5) = -999.9;
	pfi->grvals[EINDEX] = evals;
	pfi->abvals[EINDEX] = evals+3;
	pfi->ab2gr[EINDEX] = liconv;
	pfi->gr2ab[EINDEX] = liconv;
	pfi->linear[EINDEX] = 1;
	/* allocate an array of ensemble structures */
	sz = pfi->dnum[EINDEX] * sizeof(struct gaens); 
	if ((ens = (struct gaens *)galloc(sz,"ens3")) == NULL) {
	  gaprnt(0,"Unable to allocate memory for E coordinate axis values\n");
	  goto err8;
	}
	pfi->ens1 = ens;
	parms->esetup = 2;    /* still need ensemble names, time metadata */
	j = 0;
	ch = nxtwrd(ch);
	/* Check for keyword "names" followed by list of ensemble members.
	   The option for separate lines containing names, lengths, and 
	   initial times is not supported in xdfopen */
	if ((ch!=NULL) && cmpwrd("names",ch)) {
	  while (j<pfi->dnum[4]) {
	    if ((ch=nxtwrd(ch))==NULL) goto err7b;
	    /* get the ensemble name */
	    if ((getenm(ens, ch))!=0) goto err7c;
	    /* initialize remaining fields in ensemble structure */
	    for (jj=0;jj<4;jj++) ens->grbcode[jj]=-999;
	    ens->length=0;
	    ens->gt=1;
	    ens->tinit.yr=0;
	    ens->tinit.mo=0;
	    ens->tinit.dy=0;
	    ens->tinit.hr=0;
	    ens->tinit.mn=0;
	    j++; ens++;
	  }
	  parms->esetup=1;  /* still need time metadata */
	} 
      }
    } 

    /* parse the variable declarations */
    else if (cmpwrd("vars",rec)) {
      if ((ch = nxtwrd(rec)) == NULL) goto err5;
      if ((pos = intprs(ch,&(pfi->vnum)))==NULL) goto err5;
      sz = pfi->vnum * sizeof(struct sdfnames) ;
      if ((varnames = (struct sdfnames *) galloc(sz,"varnames")) == NULL) goto err8;
      parms->names1 = varnames;
      parms->dvsrch = 0 ;
      parms->dvcount = pfi->vnum ;
      i = 0;
      while (i<pfi->vnum) {
        if (fgets(rec,512,descr)==NULL) {
          gaprnt (0,"gadxdf error: Unexpected EOF reading variables\n");
          snprintf(pout,1255, "Was expecting %i records.  Found %i.\n", pfi->vnum, i);
          gaprnt (2,pout);
          goto retrn;
        }
	/* Remove any leading blanks from rec */
	reclen = strlen(rec);
	jj = 0;
	while (jj<reclen && rec[0]==' ') {
	  for (ii=0; ii<reclen; ii++) rec[ii] = rec[ii+1];
	  jj++;
	}
	/* replace newline with null at end of record */
        for (ichar = strlen(rec) - 1 ;  ichar >= 0 ;  --ichar) {
	  if (rec[ichar] == '\n') {
	    rec[ichar] = '\0' ;
	    break ; 
	  }
        }
	/* Keep mixed case and lower case versions of rec handy */
        strcpy (mrec,rec);
        lowcas(rec);
	/* Allow comments between VARS and ENDVARS */
        if (!isalnum(*(mrec))) {
	  /* Parse comment if it contains attribute metadata  */
	  if ((strncmp("*:attr",mrec,6)==0) || (strncmp("@",mrec,1)==0)) {
	    if ((ddfattr(mrec,pfi)) == -1) goto retrn;
	    else continue;
	  }
  	  else continue; 
	}
        if (cmpwrd("endvars",rec)) {
          gaprnt (0,"gadxdf error: Unexpected ENDVARS record\n");
          snprintf(pout,1255, "Was expecting %i records.  Found %i.\n", pfi->vnum, i);
          gaprnt (2,pout);
          goto err9;
        }
	/* Get the compound variable name (longnm=>abbrv). */
	/* We'll extract # levels and other metadata later. */
        if ((getncvnm(varnames, mrec))!=0) goto err6;
        i++; varnames++;
      }

      /* Check for final record */
      if (fgets(rec,512,descr)==NULL) {
        gaprnt (0,"gadxdf error: Missing ENDVARS statement.\n");
        goto retrn;
      }

      /* See if final record is an attribute comment or 'endvars'. If not, send error message */
      strcpy (mrec,rec);
      lowcas(rec);
      while (!cmpwrd("endvars",rec)) {
	if ((strncmp("*:attr",mrec,6)==0) || (strncmp("@",mrec,1)==0)) {
	  if ((ddfattr(mrec,pfi)) == -1) goto retrn;
	}
        else {
	  snprintf(pout,1255,"gadxdf error: Looking for \"endvars\", found \"%s\" instead.\n",rec);
	  gaprnt (0,pout);
	  goto err9;
	}
  	if (fgets(rec,256,descr)==NULL) {
	  gaprnt (0,"gadxdf error: Missing ENDVARS statement.\n");
	  goto retrn;
	}
      }

    } else {
      /* Parse error of descriptor file */
      gaprnt (0,"gadxdf error: Unknown keyword in description file\n");
      goto err9;
    }
  }
  
  /* Check if required DSET entry is present */
  err=0;
  for (i=0; i<1; i++) {
    if (flgs[i]) {
      gaprnt (0,"gadxdf error: missing DSET record \n");
      err=1;
    }
  }
  if (err) goto retrn;
  
  /* Done scanning.  Check if scanned stuff makes sense, 
     and then set things up correctly */

  /* set the global calendar and check if we are trying to change with a new file... */
  if(mfcmn.cal365<0) {
    mfcmn.cal365=pfi->calendar;
  } else {
    if (pfi->calendar != mfcmn.cal365) {
      gaprnt(0,"Attempt to change the global calendar...\n");
      if(mfcmn.cal365) {
	gaprnt(0,"The calendar is NOW 365 DAYS and you attempted to open a standard calendar file\n");
      } else {
	gaprnt(0,"The calendar is NOW STANDARD and you attempted to open a 365-day calendar file\n");
      }
      goto retrn;
    }
  }

  /* if time series templating was specified, & the TDEF line was incomplete, ERROR! */
  if (pfi->tmplat && parms->tsetup) {
    gaprnt (0,"gadxdf error: Use of OPTIONS template requires a complete TDEF entry\n");
    goto retrn ;
  }

  /* temporarily set the E dimension size to 1 until we can parse the rest of the metadata */
  if (pfi->tmplat && parms->esetup==3) {
    pfi->dnum[4]=1;
    /* set up linear scaling */
    sz = sizeof(gadouble)*6;
    if ((evals = (gadouble *)galloc(sz,"evals")) == NULL) {
      gaprnt(0,"Error: memory allocation failed for default ensemble dimension scaling values\n");
      goto err1;
    }
    v1=v2=1;
    *(evals+1) = v1 - v2;
    *(evals) = v2;
    *(evals+2) = -999.9;
    *(evals+4) = -1.0 * ( (v1-v2)/v2 );
    *(evals+3) = 1.0/v2;
    *(evals+5) = -999.9;
    pfi->grvals[EINDEX] = evals;
    pfi->abvals[EINDEX] = evals+3;
    pfi->ab2gr[EINDEX] = liconv;
    pfi->gr2ab[EINDEX] = liconv;
    pfi->linear[EINDEX] = 1;
    /* allocate a single ensemble structure */
    sz = sizeof(struct gaens);
    if ((ens = (struct gaens *)galloc(sz,"ens1")) == NULL) {
      gaprnt(0,"Error: memory allocation failed for default E axis values\n");
      goto err1;
    }
    pfi->ens1 = ens;
    snprintf(ens->name,15,"1");
    ens->length = pfi->dnum[TINDEX];
    ens->gt = 1;
    gr2t(pfi->grvals[TINDEX],1.0,&ens->tinit);
    /* set grib codes to default values */
    for (j=0;j<4;j++) ens->grbcode[j]=-999;
    parms->esetup=4; /* set this to 4 so we know there's a dummy E axis set up */
  }


  /* Create the fnums array. 
     If the file name is a time series template, figure out
     which times go with which files, so we don't waste a lot
     of time later opening and closing files unnecessarily. */

  if (pfi->tmplat) {
    /* The fnums array is the size of the time axis multiplied by the size of the ensemble axis. 
       It contains the t index which generates the filename that contains the data for each timestep 
       If the ensemble has no data file for a given time, the fnums value will be -1 */
    sz = sizeof(gaint)*pfi->dnum[3]*pfi->dnum[4];
    pfi->fnums = (gaint *)galloc(sz,"fnums1");   
    if (pfi->fnums==NULL) {
      gaprnt(0,"Open Error: memory allocation failed for fnums\n");
      goto err2;
    }
    /* get dt structure for t=1 */
    gr2t(pfi->grvals[3],1.0,&tdefi);
    /* loop over ensembles */
    ens=pfi->ens1;
    e=1;
    while (e<=pfi->dnum[4]) {
      j = -1; 
      t=1;
      /* set fnums value to -1 for time steps before ensemble initial time */
      while (t<ens->gt) {
	pfi->fnums[(e-1)*pfi->dnum[3]+t-1] = j;                                                    
	t++;
      }
      j = ens->gt;
      /* get dt structure for ensemble initial time */
      gr2t(pfi->grvals[3],ens->gt,&tdefe);
      /* get filename for initial time of current ensemble member  */
      ch = gafndt(pfi->name,&tdefe,&tdefe,pfi->abvals[3],pfi->pchsub1,pfi->ens1,ens->gt,e,&flag);   
      if (ch==NULL) {
	snprintf(pout,1255,"Open Error: couldn't determine data file name for e=%d t=%d\n",e,ens->gt);
	gaprnt(0,pout);
	goto err2;
      }
      /* set the pfi->tmplat flag to the flag returned by gafndt */
      if (flag==0) {
	gaprnt(1,"Warning: OPTIONS keyword \"template\" is used, but the \n");
	gaprnt(1,"   DSET entry contains no substitution templates.\n");
	pfi->tmplat = 1;
      } else {
	pfi->tmplat = flag; 
      }
      pfi->fnums[(e-1)*pfi->dnum[3]+t-1] = j;                                                    
      /* loop over remaining valid times for this ensemble */
      for (t=ens->gt+1; t<ens->gt+ens->length; t++) {
	/* get filename for time index=t ens=e */
	gr2t(pfi->grvals[3],(gadouble)t,&tdef);
	pos = gafndt(pfi->name,&tdef,&tdefe,pfi->abvals[3],pfi->pchsub1,pfi->ens1,t,e,&flag);  
	if (pos==NULL) {
	  snprintf(pout,1255,"Open Error: couldn't determine data file name for e=%d t=%d\n",e,t);
	  gaprnt(0,pout);
	  goto err2;
	}
	if (strcmp(ch,pos)!=0) {    /* filename has changed */
	  j = t;   
	  gree(ch,"f47");
	  ch = pos;
	}
	else {
	  gree(pos,"f48");
	}
	pfi->fnums[(e-1)*pfi->dnum[3]+t-1] = j;                                                    
      }
      gree(ch,"f48a");
      /* set fnums value to -1 for time steps after ensemble final time */
      j = -1;
      while (t<=pfi->dnum[3]) {
	pfi->fnums[(e-1)*pfi->dnum[3]+t-1] = j;                                                    
	t++;
      }
      e++; ens++;
    }
    pfi->fnumc = 0;
    pfi->fnume = 0;
  }

  fclose (descr);
  return Success;
  
 err0:
  gaprnt(0, "gadxdf error: Missing or invalid dimension name.\n") ;
  goto err9;

 err1:
  gaprnt (0,"gadxdf error: Missing or invalid dimension size.\n");
  goto err9;
  
 err2:
  gaprnt (0,"gadxdf error: Missing or invalid dimension");
  gaprnt (0," scaling type\n");
  goto err9;
  
 err3:
  gaprnt (0,"gadxdf error: Missing or invalid dimension");
  gaprnt (0," starting value\n");
  goto err9;
  
 err4:
  gaprnt (0,"gadxdf error: Missing or invalid dimension");
  gaprnt (0," increment value\n");
  goto err9;

 err4a:
  gaprnt (0,"gadxdf error: 0 time increment in tdef\n");
  gaprnt (0," use 1 for single time data\n");
  goto err9;
  
 err5:
  gaprnt (0,"gadxdf error: Missing or invalid variable");
  gaprnt (0," count\n");
  goto err9;
  
 err6:
  gaprnt (0,"gadxdf error: Invalid variable record\n");
  goto err9;
  
 err7b:
  gaprnt (0,"gadxdf error: Invalid number of ensembles\n");
  goto err9;

 err7c:
  gaprnt (0,"gadxdf error: Invalid ensemble name\n");
  goto err9;

 err8:
  gaprnt (0,"gadxdf error: Memory allocation Error\n");
  goto retrn;
  
 err9:
  gaprnt (0,"  --> The invalid description file record is: \n");
  gaprnt (0,"  --> ");
  gaprnt (0,rec);
  gaprnt (0,"\n");
  
 retrn:
  gaprnt (0,"  The data file was not opened. \n");
  fclose (descr);
  return Failure;
  
}


/*  handle var name of the form longnm=>abbrv
    or just the abbrv with no long name */

gaint getncvnm (struct sdfnames *var, char *mrec) {
gaint ib,i,j,k,len,flag;

  ib = 0;
  while (*(mrec+ib)==' ') ib++;

  if (*(mrec+ib)=='\0' || *(mrec+ib)=='\n') return(1);

  /* Scan for the '=>' string */
  len = 0;
  i = ib;
  flag = 0;

  while (1) {
    if (*(mrec+i)==' ' || *(mrec+i)=='\0' || *(mrec+i)=='\n') break;
    if (*(mrec+i)=='=' && *(mrec+i+1)=='>') {
      flag = 1;
      break;
    }
    len++ ; i++; 
  }

  if (flag) {
    for (j=ib; j<i; j++) {
      k = j-ib;
      var->longnm[k] = *(mrec+j); 
    }
    var->longnm[len] = '\0';
    i+=2;
  } else {
    i = 0;
    var->longnm[0] = '\0';
  } 

  if (*(mrec+i)=='\n' || *(mrec+i)=='\0') return (1);

  getwrd(var->abbrv, mrec+i, 15);
  lowcas(var->abbrv);

  /* Check if 1st character is lower-case alphabetic */
  if (islower(*(var->abbrv))) return(0);
  else return (1);
}

/* Initialize parms structure */
void initparms(GASDFPARMS *parms) {
  parms->isxdf = 0;
  parms->xsrch = 1;
  parms->ysrch = 1;
  parms->zsrch = 1;
  parms->tsrch = 1;
  parms->esrch = 1;
  parms->dvsrch = 1;
  parms->xsetup = 1;
  parms->ysetup = 1;
  parms->zsetup = 1;
  parms->tsetup = 1;
  parms->esetup = 3;
  parms->needtitle = 1;
  parms->needundef = 1;
  parms->needunpack = 1;
  parms->xdimname = NULL;
  parms->ydimname = NULL;
  parms->zdimname = NULL;
  parms->tdimname = NULL;
  parms->edimname = NULL;
  parms->names1 = NULL;
  parms->dvcount = -1;
  parms->dvsetup = (gaint *) 0 ; 
  parms->hasDDFundef = 0 ;
  return;
}

/* Free memory for parms structure */
void freeparms (GASDFPARMS *parms) {
  if (parms->xdimname) gree(parms->xdimname,"f50");
  if (parms->ydimname) gree(parms->ydimname,"f51");
  if (parms->zdimname) gree(parms->zdimname,"f52");
  if (parms->tdimname) gree(parms->tdimname,"f53");
  if (parms->edimname) gree(parms->edimname,"f54");
  if (parms->names1)   gree(parms->names1,"f55");
  return;
}

#endif
