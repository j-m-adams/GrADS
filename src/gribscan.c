/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT for more information. */

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
#include <string.h>
#include "grads.h"
#include "gx.h"
#include "gvt.h"

#define NULCH '\0'
gaint help=0;
void command_line_help(void) ;

/*---
  OLD grib decoding routine...
---*/

FILE *gfile,*ofile;

struct grhdr {
  gaint vers;
  gaint len;  
  gaint pdslen,gdslen,bmslen,bdslen;
  gaint ptvn;
  gaint center;
  gaint procid;
  gaint gridid;
  gaint gdsflg,bmsflg;
  gaint parm;
  gaint ltyp;
  gaint level;
  gaint l1,l2;
  struct dt dtim;
  gaint ftu,p1,p2,tri;
  gaint century;
  float dsf;
  gaint gpv,gnv,gtyp,gicnt,gjcnt,gsf1,gsf2,gsf3;
  gaint bnumr;
  gaint bunb;
  off_t bpos;
  gaint dgflg; 
  gaint dpflg;
  gaint doflg;
  gaint dfflg;
  gaint dbusd;
  float bsf;
  float ref;
  gaint bnum;
  off_t dpos;
};

struct grib_gds_ll{
  unsigned char *gds;
  gaint len;
  gaint nv;
  gaint pv;
  gaint drt;
  gaint ni;
  gaint nj;
  gaint lat1;
  gaint lon1;
  gaint rcdi;
  gaint rcre;
  gaint rcuv;
  gaint lat2;
  gaint lon2;
  gaint dx;
  gaint dy;
  gaint smi;
  gaint smj;
  gaint smdir;
};

/* gds for NMC lat/lon grids */

struct grib_gds_ll gdsn2; 

/* function init */

void gds_init(struct grib_gds_ll *, gaint ) ;
gaint gribhdr(struct grhdr *);
void malloc_err( gaint ) ;

gaint iok[3],iokwrite;      
gaint irec=0;          /* record counter */
gaint gdsout=0;        /* GDS output flag */
gaint bdsout=0;        /* BDS output flag */
gaint qout=0;          /* essential GrADS GRIB parms */
gaint qout1=0;         /* file name + essential GrADS GRIB parms for first record only */
gaint delim=0;         /* comma deliminated output */
gaint verb=0;          /* verbose */
gaint silent=0;        /* silent mode */
gaint gaout=0;         /* ascii output */
gaint gaoutfld=-999;   /* which fields to dump */
gaint gaouttau=-999;   /* which taus to dump */
gaint gaoutlev=-999;   /* which levs to dump */
gaint gfout=0;         /* float output */
gaint ggout=0;         /* GRIB output */
gaint gvout=0;         /* output the variable name, title units based on NMC tables */
gaint xyrevflg;        /* flag indicating whether we have the reversed lat/lon FNMOC grid */


off_t fpos;  /* File pointer into GRIB file */
off_t flen;
gaint nullsk;
gaint scanflg,scanlim=2000;

char *ofname,*ifname;

gaint main (gaint argc, char *argv[]) {
 struct grhdr ghdr;
 gaint cnt,rc,i,flg,iarg,len,skip;
 char cmd[256], *ch;
 unsigned char rec[50000], *uch;
 
 skip   = -999;
 nullsk = -999;
 ifname = NULL;
 ofname = NULL;

 if (argc>1) {
    iarg = 1;
    while (iarg<argc) {

      flg = 1;
      ch = argv[iarg];

      if (*(ch)=='-' && *(ch+1)=='h' && *(ch+2)=='e' && *(ch+3)=='l' && *(ch+4)=='p' ) {
	command_line_help();
	return(0);
      } else if (*ch=='-' && *(ch+1)=='i') {
        iarg++; 
        if (iarg<argc) {
	  ifname = (char *)malloc(strlen(argv[iarg])+1);
          strcpy(ifname,argv[iarg]);
          flg = 0;
        } else iarg--;   /* Let error message pop */
      }
      else if (*ch=='-' && *(ch+1)=='o' && *(ch+2) == NULCH ) {
        iarg++; 
        if (iarg<argc) {
	  ofname = (char *)malloc(strlen(argv[iarg])+1);
          strcpy(ofname,argv[iarg]);
          flg = 0;
        } else iarg--;   /* Let error message pop */
      }
      else if (*ch=='-' && *(ch+1)=='n') {
        ch+=2;
        i = 0;
        while(*(ch+i) && i<900) {
          if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
          i++;
        }
        if (i<900) {
          sscanf(ch,"%i",&nullsk);
          flg = 0;
        }
      }
      else if (*ch=='-' && *(ch+1)=='g' && *(ch+2)=='d' ) {
	gdsout = 1;
	flg = 0;
      }      
      else if (*ch=='-' && *(ch+1)=='g' && *(ch+2)=='v' ) {
	gvout = 1;
	flg = 0;
      }      
      else if (*ch=='-' && *(ch+1)=='v' ) {
	verb = 1;
	flg = 0;
      }      
      else if (*ch=='-' && *(ch+1)=='S' ) {
	silent = 1;
	flg = 0;
      }      
      else if (*ch=='-' && *(ch+1)=='b' && *(ch+2)=='d'  ) {
	bdsout = 1;
	flg = 0;
      }      
      else if (*ch=='-' && *(ch+1)=='d' ) {
	delim = 1;
	flg = 0;
      }      
      else if (*ch=='-' && *(ch+1)=='q' && *(ch+2)=='1' ) {
	qout1 = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='o' && *(ch+2)=='g' ) {
	ggout = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='o' && *(ch+2)=='a' ) {
	gaout = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='s' && *(ch+2)=='p' ) {
	flg = 0;
        ch+=3;
        if (*ch != NULCH) {
          i = 0;
          while(*(ch+i) && i<900) {
            if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
            i++;
          }
          if (i<900) {
            sscanf(ch,"%i",&gaoutfld);
            flg = 0;
          }
	}
      }
      else if (*ch=='-' && *(ch+1)=='s' && *(ch+2)=='t' ) {
	flg = 0;
        ch+=3;
        if (*ch != NULCH) {
          i = 0;
          while(*(ch+i) && i<900) {
            if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
            i++;
          }
          if (i<900) {
            sscanf(ch,"%i",&gaouttau);
            flg = 0;
          }
	}
      }
      else if (*ch=='-' && *(ch+1)=='s' && *(ch+2)=='l' ) {
	flg = 0;
        ch+=3;
        if (*ch != NULCH) {
          i = 0;
          while(*(ch+i) && i<900) {
            if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
            i++;
          }
          if (i<900) {
            sscanf(ch,"%i",&gaoutlev);
            flg = 0;
          }
	}
      }
      else if (*ch=='-' && *(ch+1)=='o' && *(ch+2)=='f' ) {
	gfout = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='q' ) {
	qout = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='s') {
        scanflg = 1;
	ch+=2;
        i = 0;
        while(*(ch+i) && i<900) {
          if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
          i++;
        }
        if (i<900) {
          sscanf(ch,"%i",&scanlim);
	  printf("scanlim = %d\n",scanlim);
          flg = 0;
        }
      }
      else if (*ch=='-' && *(ch+1)=='h') {
        ch+=2;
        if (*ch=='n' && *(ch+1)=='m' &&  *(ch+2)=='c') {
          skip = -1;
          flg = 0;
        } else if (*ch == NULCH) {
	  skip = -999;
	  flg = 0;
	} else {
          i = 0;
          while(*(ch+i) && i<900) {
            if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
            i++;
          }
          if (i<900) {
            sscanf(ch,"%i",&skip);
            flg = 0;
          }
        }
      }
      if (flg) {
        printf ("Invalid command line argument: %s  Ignored.\n",argv[iarg]);
      }
      iarg++;
    }
  }
  
  if (ifname == NULL) { 
    command_line_help();
    cnt = nxtcmd (cmd,"Enter name of GRIB file: ");
    if (cnt==0) return(1);
    getwrd(ifname,cmd,250);
  }

  gfile = fopen(ifname,"rb");
  if (gfile==NULL) {
    printf ("Could not open GRIB file.  File name is:\n");
    printf ("  %s\n",ifname);
    return(1);
  }

  /* open output file */
  
  if(ofname == NULL) {
   ofname = (char *)malloc(7+5);
   strcat(ofname,"zy0x1w2");
  }

  if(gaout) {
    strcat(ofname,".asc");
    ofile = fopen(ofname,"w");
    if (ofile==NULL) {
      printf ("Could not open ASCII output file: zy0x1w2.ascii");
      return(1);
    }
  } else if (gfout) {
    strcat(ofname,".dat");
    ofile = fopen(ofname,"wb");
    if (ofile==NULL) {
      printf ("Could not open float  output file: zy0x1w2.ascii");
      return(1);
    }
  } else if (ggout) {
    strcat(ofname,".grb");
    ofile = fopen(ofname,"wb");
    if (ofile==NULL) {
      printf ("Could not open NMC GRIB output file: zy0x1w2.grb");
      return(1);
    }
  }    

  /* initialize NMC lat/lon gds for grid #2 */

  if (ggout) gds_init(&gdsn2,2);

  /* Get file size */

  rc = fseeko(gfile,0L,2);
  if (rc) return(50);

  flen = ftell(gfile);

  /* Set up to skip appropriate amount.  */

 if (skip == -999) { 

   /* we have no idea what the header is; find it within reason */

   /* read in four bytes */
 
   rc = fseeko(gfile,0,0);
   if (rc) return(50);

   uch=&rec[0]; 
   rc = fread(uch,sizeof(char),4,gfile);
   if (rc < 4) return(50);

   fpos = 0;
   while (fpos < flen && !(*(uch)=='G' && *(uch+1)=='R' &&
	 *(uch+2)=='I' && *(uch+3)=='B') ) {
       fpos++;
       rc = fseeko(gfile,fpos,0);
       if (rc) return(50);
       rc = fread(uch,1,4,gfile);
       if (rc<4) fpos = flen;
     }
 
   if (fpos == flen) {
     printf ("GRIB header not found.  File apparantly not GRIB data\n");
     printf ("->%c%c%c%c%c%c<-\n",*uch,*(uch+1),*(uch+2),*(uch+3),*(uch+4),*(uch+5));
     return (99);
   } else {
     if (verb) printf("the header is %lld bytes long\n",fpos);
   }

 } else if (skip > -1) {

   /* fixed header length */

   fpos = skip;

 } else {

   /* hard-wired nmc header */

    rc = fseeko(gfile,0,0);
    if (rc) return(50);

    rc = fread (rec,1,100,gfile);
    if (rc<100) {
      printf ("I/O Error reading header\n");
      return (1);
    }
    len = gagby(rec,88,4);
    fpos = len*2 + 100;
  }

  /* We are positioned.  Go read the first GRIB header */

  while (1) {
    rc = gribhdr(&ghdr);
    if (rc) break;
  }

  fclose (gfile);
  if (gaout || ggout || gfout) fclose(ofile);

/* error conditions */

  if (rc == 2) {
    return(0);
  } else if (rc==50) {
    printf ("I/O Error reading GRIB file\n");
    printf ("Possible cause: premature EOF\n");
    return(1);
  } else if (rc==51) {
    printf ("I/O Error reading GRIB file\n");
    printf ("Possible cause: premature EOF\n");
    return(1);
  } else if (rc==52) {
    printf ("I/O Error reading GRIB file\n");
    printf ("premature EOF between records\n");
    return(1);
  } else if (rc==53 && verb) {
    printf ("Junk at end of GRIB file\n");
    return(1);
  } else if (rc==99) {
    printf ("GRIB format error\n");
    return(1);
  }

  if(verb) printf ("Reached EOF\n"); 
  return(0);
}

void gds_init(struct grib_gds_ll *gds, gaint nmc_grid) {
  
gaint bstrt;
  
  if(nmc_grid == 2) {
    gds->len=32;
    gds->gds = (unsigned char *)malloc(gds->len);
    gds->nv=0;
    gds->pv=255;
    gds->drt=0;
    gds->ni=144;
    gds->nj=73;
    gds->lat1=90000;
    gds->lon1=0;
    gds->rcdi=0;
    gds->rcre=0;
    gds->rcuv=0;
    gds->dx=2500;
    gds->dy=2500;
    gds->lat2=-90000;
    gds->lon2=-2500;
    gds->smi=0;
    gds->smj=0;
    gds->smdir=0;

    /* code it up */
    gapby(gds->len,gds->gds,0,3);       /* octets 1-3 length of GDS */  
    gapby(gds->nv,gds->gds,3,1);        /* octet 4 -- nv number of vertical coordinates */
    gapby(gds->pv,gds->gds,4,1);        /* octet 5 -- pv location of vert coors */
    gapby(gds->drt,gds->gds,5,1);       /* octet 6 -- data representation type (TABLE 6) */
    gapby(gds->ni,gds->gds,6,2);        /* octets 7-8 -- number of lons */
    gapby(gds->nj,gds->gds,8,2);        /* octets 9-10 -- number of lats */
    bstrt=10*8;                         /* octets 11-13, 14-16  lat1, lon1 */
    if(gds->lat1 < 0) {
      gapbb(1,gds->gds,bstrt,1);
      gds->lat1=-gds->lat1;
    } else {
      gapbb(0,gds->gds,bstrt,1);
    }
    bstrt++;
    gapbb(gds->lat1,gds->gds,bstrt,23);
    bstrt+=23;
    if(gds->lon1 < 0) {
      gapbb(1,gds->gds,bstrt,1);
      gds->lon1=-gds->lon1;
    } else {
      gapbb(0,gds->gds,bstrt,1);
    }
    bstrt++;
    gapbb(gds->lon1,gds->gds,bstrt,23);
    bstrt+=23;
    gapbb(gds->rcdi,gds->gds,bstrt,1);   /* octet 17 resolution component flag -- table 7 */
    bstrt++; 
    gapbb(gds->rcre,gds->gds,bstrt,1);
    bstrt++;
    gapbb(0,gds->gds,bstrt,2);
    bstrt+=2;
    gapbb(gds->rcuv,gds->gds,bstrt,1);
    bstrt++;
    gapbb(0,gds->gds,bstrt,3);
    bstrt+=3;
    if(gds->lat2 < 0) {                  /* octets 18-20,21-23 -- lat2,lon2 */
      gapbb(1,gds->gds,bstrt,1);
      gds->lat2=-gds->lat2;
    } else {
      gapbb(0,gds->gds,bstrt,1);
    }
    bstrt++;
    gapbb(gds->lat1,gds->gds,bstrt,23);
    bstrt+=23;
    
    if(gds->lon2 < 0) {
      gapbb(1,gds->gds,bstrt,1);
      gds->lon2=-gds->lon2;
    } else {
      gapbb(0,gds->gds,bstrt,1);
    }
    bstrt++;
    gapbb(gds->lon2,gds->gds,bstrt,23);
    bstrt+=23;
    if(gds->dx < 0) {                    /* octets 24-25 -- dlon */
      gapbb(1,gds->gds,bstrt,1);
      gds->dx=-gds->dx;
    } else {
      gapbb(0,gds->gds,bstrt,1);
    }
    bstrt++;
    gapbb(gds->dx,gds->gds,bstrt,15);
    bstrt+=15;
    if(gds->dy < 0) {                    /* octets 26-27 -- dlat */
      gapbb(1,gds->gds,bstrt,1);
      gds->dy=-gds->dy;
    } else {
      gapbb(0,gds->gds,bstrt,1);
    }
    bstrt++;
    gapbb(gds->dy,gds->gds,bstrt,15);
    bstrt+=15;
    
    gapbb(gds->smi,gds->gds,bstrt,1);
    bstrt++;
    gapbb(gds->smj,gds->gds,bstrt,1);
    bstrt++;
    gapbb(gds->smdir,gds->gds,bstrt,1);
    bstrt++;
    gapbb(0,gds->gds,bstrt,5);
    bstrt+=5;
    gapby(0,gds->gds,28,4);               /* octets 29 -32 - reserved 0 filled */
  }
}


/* Read a GRIB header, and get needed info out of it.  */

gaint gribhdr (struct grhdr *ghdr) {
unsigned char rec[512],*ch;
gaint i,j,ifnmoc,jfnmoc,inmc,jnmc,len,rc,sign,mant;
off_t cpos,cposis;
gaint npts=0,bstrt,yy;
float la1,la2,lo1,lo2,ladi,lodi;
gaint *gr=NULL,*gri=NULL;
float *grf=NULL,*grfi=NULL;

struct grib_len {     /* length of grib sections */
  gaint is;
  gaint pds;
  gaint gds;
  gaint bms;
  gaint bds;
  gaint es;
  gaint msg;
} glen;

struct grib {         /* grib data */
  unsigned char *is;
  unsigned char *pds;
  unsigned char *gds;
  unsigned char *bms;
  unsigned char *bds;
  unsigned char es[4];
} gd;


  if (fpos>=flen) return(1);

  /* Position at start of next record */
  rc = fseeko(gfile,fpos,0);
  if (rc) return(50);

  /* Read start of record -- length, version number */
  rc = fread(rec,1,8,gfile);
  if (rc<8){
    if(fpos+8 >= flen) return(51);
    else return(51);
  }
  if (*rec!='G' || *(rec+1)!='R' || *(rec+2)!='I' || *(rec+3)!='B') {

    /* look for data between records BY DEFAULT */ 
    i = 1;
    fpos += i;
    rc = fseeko(gfile,fpos,0);
    if (rc) return(50);
    ch=&rec[0];
    rc = fread(ch,sizeof(unsigned char),4,gfile);
    while ( (fpos < flen-4) && (i < scanlim) && !( *ch=='G' && *(ch+1)=='R' &&
			   *(ch+2)=='I' && *(ch+3)=='B' ) ) {
      fpos++;
      i++;
      rc = fseeko(gfile,fpos,0);
      if (rc) return(50);
      rc = fread(ch,sizeof(unsigned char),4,gfile);
      if (rc<4) return(50);
    } 
    if (i == scanlim ) {
      printf ("GRIB header not found in scanning between records\n");
      printf ("->%c%c%c%c<-\n",*rec,*(rec+1),*(rec+2),*(rec+3));
      return (52);
    } else if (fpos == flen -4) {
      return (53);
    }
    /* SUCCESS redo the initial read */    
    rc = fseeko(gfile,fpos,0);
    if (rc) return(50);
    rc = fread(rec,1,8,gfile);
    if (rc<8){
      if(fpos+8 >= flen) return(51);
      else return(52);
    }
  }
  cpos = fpos;
  cposis = fpos;

  /* store IS */
  glen.is=8;
  gd.is = (unsigned char *)malloc(glen.is);
  if(gd.is == NULL) malloc_err(1);
  memcpy (gd.is,&rec[0],glen.is);
  ghdr->vers = gagby(rec,7,1);
  if (ghdr->vers>1) {
    printf ("File is not GRIB version 0 or 1, 0 or 1 is required. \n");
    printf (" Version number is %i\n",ghdr->vers);
    return (99);
  }
  /* bump the record # */
  irec++;
  if (ghdr->vers==0) {
    cpos += 4;
    rc = fseeko(gfile,cpos,0);
    if (rc) return(50);
  } else {
    ghdr->len = gagby(rec,4,3);
    glen.msg = ghdr->len;
    cpos = cpos + 8;
    rc = fseeko(gfile,cpos,0);
    if (rc) return(50);
  }

/*
  PPPPPPPPPPPPPP DDDDDDDDDDDDDD SSSSSSSSSSSSSSS
*/

  rc = fread(rec,1,3,gfile);
  if (rc<3) return(50);
  len = gagby(rec,0,3);
  ghdr->pdslen = len;
  cpos = cpos + len;
  rc = fread(rec+3,1,len-3,gfile);
  if (rc<len-3) return(50);

  /* mf store PDS */

  gd.pds=NULL;
  glen.pds=0;
  if(ggout || gaout || gfout) {
    glen.pds=len;
    gd.pds = (unsigned char *)malloc(glen.pds);
    if(gd.pds == NULL) malloc_err(2);
    memcpy (gd.pds,&rec[0],glen.pds);
  }

  /* get info from PDS */

  ghdr->ptvn = gagby(rec,3,1);
  ghdr->center = gagby(rec,4,1);
  ghdr->procid = gagby(rec,5,1);
  ghdr->gridid = gagby(rec,6,1);
  ghdr->gdsflg = gagbb(rec+7,0,1);
  ghdr->bmsflg = gagbb(rec+7,1,1);
  ghdr->parm = gagby(rec,8,1);
  ghdr->ltyp = gagby(rec,9,1);
  ghdr->level = gagby(rec,10,2);
  ghdr->l1 = gagby(rec,10,1);
  ghdr->l2 = gagby(rec,11,1);
  ghdr->dtim.yr = gagby(rec,12,1);
  yy=ghdr->dtim.yr;
  ghdr->dtim.mo = gagby(rec,13,1);
  ghdr->dtim.dy = gagby(rec,14,1);
  ghdr->dtim.hr = gagby(rec,15,1);
  ghdr->dtim.mn = gagby(rec,16,1);
  ghdr->ftu = gagby(rec,17,1);
  ghdr->tri = gagby(rec,20,1);
  if (ghdr->tri==10) {
    ghdr->p1 = gagby(rec,18,2);
    ghdr->p2 = 0;
  } else {
    ghdr->p1 = gagby(rec,18,1);
    ghdr->p2 = gagby(rec,19,1);
  }
/*
ghdr->nave = gagby(rec,21,2);
ghdr->nmiss = gagby(rec,23,1);
*/
  if (len>24) {
    ghdr->century = gagby(rec,24,1);
    ghdr->dtim.yr = ghdr->dtim.yr + (ghdr->century-1)*100;
  } else {
    ghdr->century = -999;
    if (ghdr->dtim.yr>49) ghdr->dtim.yr += 1900;
    else ghdr->dtim.yr += 2000;
  }
  if (len>25) {
/*
ghdr->subcent = gagby(rec,25,1);
*/
    ghdr->dsf = (float)gagbb(rec+26,1,15);
    i = gagbb(rec+26,0,1);
    if (i) ghdr->dsf = -1.0*ghdr->dsf;
    ghdr->dsf = pow(10.0,ghdr->dsf);
    if(ghdr->dsf == 0.0) ghdr->dsf = 1.0;  /* mf make sure dsf != 0 */
  } else ghdr->dsf = 1.0;

if(!silent) {
if(qout) {

  if(ghdr->ltyp == 100) {
    printf("%d, F ,%d,%d,%d,%d,%5g, T ,%d,%d,%d,%d,%d,%d,%d,%d",
	   irec,ghdr->parm,ghdr->ltyp,ghdr->level,
	   ghdr->tri,ghdr->dsf,
	   ghdr->dtim.yr,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr,
	   ghdr->dtim.mn,ghdr->ftu,ghdr->p1,ghdr->p2);

    if(gvout) printf(", V , %s, %s, %s",
		 vt[ghdr->parm].name,vt[ghdr->parm].desc,vt[ghdr->parm].units);

    printf(", G ,%d, BDTG, %02d%02d%02d%02d ",ghdr->gridid,
	   yy,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr);

  } else {
    printf("%d, F ,%d,%d,%d,%d,%d,%d,%5g, T ,%d,%d,%d,%d,%d,%d,%d,%d",
	   irec,ghdr->parm,ghdr->ltyp,ghdr->level,
	   ghdr->l1,ghdr->l2,ghdr->tri,ghdr->dsf,
	   ghdr->dtim.yr,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr,
	   ghdr->dtim.mn,ghdr->ftu,ghdr->p1,ghdr->p2);
    if(gvout) printf(", V ,%s ,%s ,%s",
		 vt[ghdr->parm].name,vt[ghdr->parm].desc,vt[ghdr->parm].units);
    printf(", G ,%d, BDTG, %02d%02d%02d%02d ",ghdr->gridid,
	   yy,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr);
  }
  if(!bdsout) printf("\n");

} else if(qout1 == 1) {

  if(ghdr->ltyp == 100) {
    printf("%40s, F ,%d,%d,%d,%d,%d,%d,%5g, T ,%d,%d,%d,%d,%d,%d,%d,%d",
	   ifname,ghdr->parm,ghdr->ltyp,ghdr->level,
	   ghdr->l1,ghdr->l2,ghdr->tri,ghdr->dsf,
	   ghdr->dtim.yr,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr,
	   ghdr->dtim.mn,ghdr->ftu,ghdr->p1,ghdr->p2);
    printf(", G ,%d, BDTG, %02d%02d%02d%02d ",ghdr->gridid,
	   yy,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr);
  } else {
    printf("%40s, F ,%d,%d,%d,%d,%d,%d,%5g, T ,%d,%d,%d,%d,%d,%d,%d,%d",
	   ifname,ghdr->parm,ghdr->ltyp,ghdr->level,
	   ghdr->l1,ghdr->l2,ghdr->tri,ghdr->dsf,
	   ghdr->dtim.yr,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr,
	   ghdr->dtim.mn,ghdr->ftu,ghdr->p1,ghdr->p2);
    printf(", G ,%d, BDTG, %02d%02d%02d%02d ",ghdr->gridid,
	   yy,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr);
  }
  if(!bdsout) printf("\n");
  return(2);

} else if( qout == 0 ) {

  if( delim ) {
    printf("PDS,%d,%d,%d,%d,%d,%d,%d,",
	   irec,ghdr->gridid,ghdr->parm,ghdr->ltyp,ghdr->level,ghdr->l1,ghdr->l2);
    printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%g",
	   ghdr->dtim.yr,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr,
	   ghdr->dtim.mn,ghdr->ftu,ghdr->p1,ghdr->p2,ghdr->tri,ghdr->dsf);
    if(gvout) printf(",%s,%s,%s",
		 vt[ghdr->parm].name,vt[ghdr->parm].desc,vt[ghdr->parm].units);
  } else {
    printf("PDS #%-4d %2d %3d %3d %4d %3d %3d BMSFLG: %1d ",
	   irec,ghdr->gridid,ghdr->parm,ghdr->ltyp,ghdr->level,ghdr->l1,ghdr->l2,ghdr->bmsflg);
    printf("%4d %02d %02d %02d %02d % 3d % 3d % 3d % 3d %8g",
	   ghdr->dtim.yr,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr,
	   ghdr->dtim.mn,ghdr->ftu,ghdr->p1,ghdr->p2,ghdr->tri,ghdr->dsf);
    if(gvout) printf(" %6s %38s %16s",
		 vt[ghdr->parm].name,vt[ghdr->parm].desc,vt[ghdr->parm].units);

  }

  if(gdsout == 0 && !bdsout) printf("\n");

}
}

/*
  GGGGGGGGGGGGGGG DDDDDDDDDDDDDDDDDDDD SSSSSSSSSSSSSSS
*/

  gd.gds=NULL;
  glen.gds=0;
  if (ghdr->gdsflg) {

    rc = fread(rec,1,3,gfile);
    if (rc<3) return(50);
    len = gagby(rec,0,3);
    ghdr->gdslen = len;
    cpos = cpos + len;
    rc = fread(rec+3,1,len-3,gfile);
    if (rc<len-3) return(50);

  /* mf store GDS */
 
    if(ggout || gaout || gfout) {
      glen.gds=len;
      gd.gds = (unsigned char *)malloc(glen.gds);
      if(gd.gds == NULL) malloc_err(3);
      memcpy (gd.gds,&rec[0],glen.gds);
    }

    ghdr->gpv = gagby(rec,3,1);
    ghdr->gnv = gagby(rec,4,1);
    ghdr->gtyp = gagby(rec,5,1);
    ghdr->gicnt = gagby(rec,6,2);
    ghdr->gjcnt = gagby(rec,8,2);

    la1 = gagbb(rec+10,1,23)*0.001;
    i = gagbb(rec+10,0,1);
    if (i) la1 = -1*la1;
    lo1 = gagbb(rec+13,1,23)*0.001;
    i = gagbb(rec+13,0,1);

    if (i) lo1 = -1*lo1;
    la2 = gagbb(rec+17,1,23)*0.001;
    i = gagbb(rec+17,0,1);
    if (i) la2 = -1*la2;
    lo2 = gagbb(rec+20,1,23)*0.001;
    i = gagbb(rec+20,0,1);
    if (i) lo2 = -1*lo2;
    lodi = gagby(rec,23,2);
    ghdr->gsf1 = gagbb(rec+27,0,1);
    ghdr->gsf2 = gagbb(rec+27,1,1);
    ghdr->gsf3 = gagbb(rec+27,2,1);
    ladi = gagby(rec,25,2);
    if(ghdr->gtyp == 0) {
      ladi *= 0.001;
      lodi *= 0.001;
    } else {
      ladi *= 1.0;
      lodi *= 1.0;
    }
/*
   special case of thinned grids in lat from ecmwf .......
*/
  npts=0;
  if(ghdr->gicnt == 65535) {
    yy=gagby(rec,28,4);
    for(i=0;i<(ghdr->gdslen-32)/2;i++) {
      yy=gagby(rec,32+i*2,2);
      npts += yy;
/*      printf("qqq i yy = %d %d\n",i+1,yy); */
    }
  }

if(!silent) {
    if( gdsout && qout == 0 ) {
      if( delim ) {
	printf(",GDS,%d,%d,%d,%-6.3f,%-6.3f,%-6.3f,%-6.3f,%-6.3f,%-6.3f,",
	       ghdr->gtyp,ghdr->gicnt,ghdr->gjcnt,lo1,lo2,lodi,la1,la2,ladi);
	printf("%d,%d,%1d",ghdr->gsf1,ghdr->gsf2,ghdr->gsf3);
      } else {
	printf(" GDS % 3d % 3d % 3d %6.3f %6.3f %6.3f %6.3f %6.3f %6.3f",
	       ghdr->gtyp,ghdr->gicnt,ghdr->gjcnt,lo1,lo2,lodi,la1,la2,ladi);
	printf(" %1d %1d %1d",ghdr->gsf1,ghdr->gsf2,ghdr->gsf3);
	printf(" GDS pv nv % 3d % 3d",ghdr->gpv,ghdr->gnv);
      }
      if(!bdsout) printf("\n");

    }

  }

  }

/*
  BBBBBBBBBBBBBBB MMMMMMMMMMMMMMMMMMM SSSSSSSSSSSSSSSSS
*/

  gd.bms=NULL;
  ghdr->bmslen = 0;
  glen.bms=0;
  if (ghdr->bmsflg && ghdr->gicnt != 65535) {
    rc = fread(rec,1,6,gfile);
    if (rc<6) return(50);
    len = gagby(rec,0,3);
    ghdr->bmslen = len;

  /* mf store BMS */

    if(ggout || gaout || gfout) {
      glen.bms=len;

      gd.bms = (unsigned char *)malloc(glen.bms);
      if(gd.bms == NULL) malloc_err(4);
      rc=fread(gd.bms,1,glen.bms,gfile);
    }

    ghdr->bnumr = gagby(rec,4,2);
    ghdr->bunb = gagby(rec,3,1);
    ghdr->bpos = cpos+6;
    cpos = cpos + len;
    rc = fseeko(gfile,cpos,0);
    if (rc) return(50);

  } 
/*
  BBBBBBBBBBBBBB DDDDDDDDDDDDDDDDDD SSSSSSSSSSSSSSSSS
*/

  rc = fread(rec,1,11,gfile);
  if (rc<11) return(50);
  len = gagby(rec,0,3);
  ghdr->bdslen = len;
  ghdr->dgflg = gagbb(rec+3,1,0);
  ghdr->dpflg = gagbb(rec+3,1,1);
  ghdr->doflg = gagbb(rec+3,1,2);
  ghdr->dfflg = gagbb(rec+3,1,3);

/*
  if (ghdr->dpflg) {
    printf ("GRIB data does not use simple packing.  Cannot handle.\n");
    return(99);
  }
*/
  i = gagby(rec,4,2);
  if (i>32767) i = 32768-i;
  ghdr->bsf = pow(2.0,(float)i);

  i = gagby(rec,6,1);
  sign = 0;
  if (i>127) {
    sign = 1;
    i = i - 128;
  }
  mant = gagby(rec,7,3);
  if (sign) mant = -mant;
  ghdr->ref = (float)mant * pow(16.0,(float)(i-70));

  ghdr->bnum = gagby(rec,10,1);
  ghdr->dpos = cpos+11;

  /* find number of DEFINED grid points */

  if(!ghdr->dfflg && ghdr->bnum !=0 && npts != 0) {
    ghdr->dbusd = gagbb(rec+3,4,4);
    npts = ( (len-11)*8 - ghdr->dbusd)/ghdr->bnum;
  } else {
    npts = (len-13)*8;
  }

  /* bump the file pointer and output */

  if (ghdr->vers==0) {
    ghdr->len=8+ghdr->pdslen+ghdr->gdslen +ghdr->bmslen + ghdr->bdslen;
    fpos = fpos + ghdr->len;
    printf ("\nLengths: pds,gds,bms,bds = %i %i %i %i \n",
      ghdr->pdslen,ghdr->gdslen,ghdr->bmslen,ghdr->bdslen);
  } else fpos = fpos + ghdr->len;

  if(verb) {
    printf ("\nLengths: pds,gds,bms,bds = %d %d %d %d \n",
	    ghdr->pdslen,ghdr->gdslen,ghdr->bmslen,ghdr->bdslen);
  }

  if( bdsout && !silent ) {
    if ( !qout) {
      if( delim ) {
	printf(",BDS,%d,%10g,%d,%lld,%d",ghdr->bnum,ghdr->ref,npts,cposis,ghdr->len);
      } else {
	printf(" BDS % 3d %10g % 10d % 10lld % 10d",ghdr->bnum,ghdr->ref,npts,cposis,ghdr->len);
      }
    } else {
      printf(", B ,%2d,%g,%d,%lld,%d",ghdr->bnum,ghdr->ref,npts,cposis,ghdr->len);
    }
    printf("\n");
  } 
    if(verb) printf("npts = %d %d %d\n",npts,ghdr->center,ghdr->gridid);

  /* mf store BDS  and get data  */

  gd.bds=NULL;

  if(ggout || gaout || gfout) {

    glen.bds=len;
    gd.bds = (unsigned char *)malloc(glen.bds);
    if(gd.bds == NULL) malloc_err(5);


    rc = fseeko(gfile,cpos,0);
    rc = fread(gd.bds,1,glen.bds,gfile);
    if (rc<glen.bds) return(50);

    gr = (gaint *)malloc(npts*sizeof(gaint));
    if(gr == NULL) malloc_err(6);

    if(gaout || gfout) {

      grf = (float *)malloc(npts*sizeof(float));
      if(grf == NULL) malloc_err(7);

    }

    /* decode the grib data to int and float */

    bstrt=11*8;
    for(i=0;i<npts;i++) {
      *(gr+i)  = gagbb(gd.bds,bstrt,ghdr->bnum);
      if(gaout || gfout) {
	*(grf+i) = ( ghdr->ref + (float)(*(gr+i)) * ghdr->bsf )/ghdr->dsf;
      }
      bstrt += ghdr->bnum;
    }
/*
  invert to NMC grid if FNMOC 2.5 global grid
*/

    xyrevflg=0;

    if(ghdr->center == 58 && ghdr->gridid == 223) {

      xyrevflg=1;
      gri = (gaint *)malloc(npts*sizeof(gaint));
      if(gri == NULL) malloc_err(8);
      grfi = (float *)malloc(npts*sizeof(float));
      if(grfi == NULL) malloc_err(9);

      for(i=0;i<npts;i++) {

	ifnmoc=i%73+1;
	jfnmoc=i/73+1;
	inmc=jfnmoc+24;
	if(inmc>144) inmc=inmc-144;
	jnmc=ifnmoc;
	j=(jnmc-1)*144+inmc-1; 
	
	if(gfout || gaout) {
	  *(grfi+j)=*(grf+i);
	} else if(ggout) {
	  *(gri+j)=*(gr+i);
	}
      }
    
      if(ggout) {

    /* encode ints to GRIB and load into bds */
	
	bstrt=11*8;
	for(i=0;i<npts;i++) {
	  gapbb(*(gri+i),gd.bds,bstrt,ghdr->bnum);
	  bstrt += ghdr->bnum;
	}

    /* fix century problem */

        gapby(20,gd.pds,24,1);

    /* change the grid param */

	gapby(2,gd.pds,6,1);

    /* change the gdsflg */
      
	gapbb(1,gd.pds,7*8,1);

	gapby(2,gd.pds,6,1);

   /* calc the length of the GRIB msg */
     
	len = glen.is + glen.pds + gdsn2.len + glen.bds + 4;
	gapby(len,gd.is,4,3);

   /* write out the GRIB msg */
        
        rc=fwrite(gd.is,1,glen.is,ofile);
        rc=fwrite(gd.pds,1,glen.pds,ofile);
        rc=fwrite(gdsn2.gds,1,gdsn2.len,ofile);
        rc=fwrite(gd.bds,1,glen.bds,ofile);
	strncpy((char*)gd.es,"7777",4);
        rc=fwrite(gd.es,1,4,ofile);

      }


    } /* end of check for FNMOC 73x144 grids */


/* conditions for dumping fields */

    iok[0]=0;
    iok[1]=0;
    iok[2]=0;
    iokwrite=0;

    if( gaoutfld != -999 ) {
      iok[0]=-1;
      if(ghdr->parm == gaoutfld ) iok[0]=1;
    }
    if( gaouttau != -999 ) {
      iok[1]=-1;
      if(ghdr->p1 == gaouttau ) iok[1]=1;
    }

    if( gaoutlev != -999 ) {
      iok[2]=-1;
      if( ghdr->level == gaoutlev ) iok[2]=1;
    }
/*	special restrictions */

    if( (iok[0] != 0) || (iok[1] != 0) || (iok[2] != 0) ) {

	if( iok[0] == 1 && iok[1] == 0 && iok[2] == 0 ) iokwrite=1;
	if( iok[0] == 0 && iok[1] == 1 && iok[2] == 0 ) iokwrite=1;
	if( iok[0] == 0 && iok[1] == 0 && iok[2] == 1 ) iokwrite=1;
	if( iok[0] == 1 && iok[1] == 1 && iok[2] == 0 ) iokwrite=1;
	if( iok[0] == 0 && iok[1] == 1 && iok[2] == 1 ) iokwrite=1;
	if( iok[0] == 1 && iok[1] == 0 && iok[2] == 1 ) iokwrite=1;
	if( iok[0] == 1 && iok[1] == 1 && iok[2] == 1 ) iokwrite=1;

    } else {
/*	dump all fields */
      if( gaout || gfout || ggout ) iokwrite = 1;
    }

    if( iokwrite ) {

      if (gaout) {

/* ASCII I/O */


	fprintf(ofile,"GRIB parm#= % 3d  npts= % 6d  grid id=% 3d\n",
		ghdr->parm,npts,ghdr->gridid);
	fprintf(ofile,"Base DTG= %02d%02d%02d%02d  ",
		yy,ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr);
	fprintf(ofile,"ftu= % 2d  p1= % 4d  p2= % 4d  tri= % 5d\n",
		ghdr->ftu,ghdr->p1,ghdr->p2,ghdr->tri);
	for (i=0;i<npts;i++) {
	  if(xyrevflg) {
	    fprintf(ofile,"%8g",*(grfi+i));
	  } else { 
	    fprintf(ofile,"%8g",*(grf+i));
	  }
          if((i+1)%10 == 0 ) fprintf(ofile,"\n");
	}
	fprintf(ofile,"\n");
      } else if(gfout) {

/* float I/O */

	if(xyrevflg) {
	  rc=fwrite(grfi,sizeof(float),npts,ofile);
	} else {
	  rc=fwrite(grf,sizeof(float),npts,ofile);
	}

	if(rc != npts) return(51);

      } else if(ggout && !xyrevflg) {

/* GRIB I/O */

	rc=fwrite(gd.is,1,glen.is,ofile);
	rc=fwrite(gd.pds,1,glen.pds,ofile);
	if(gd.gds != NULL) rc=fwrite(gd.gds,1,glen.gds,ofile);
	if(gd.bms != NULL) rc=fwrite(gd.bms,1,glen.bms,ofile);
	rc=fwrite(gd.bds,1,glen.bds,ofile);
	strncpy((char*)gd.es,"7777",4);
	rc=fwrite(gd.es,1,4,ofile);
	
      }

    }

  }

  free (gd.is);
  if(gd.pds != NULL) {
    free (gd.pds);
  }
  if(gd.gds != NULL) {
    free(gd.gds);
  }
  if(gd.bms != NULL) free(gd.bms);

  if(xyrevflg) {
    free(gri); 
    free(grfi);
  }

  if(gfout || gaout) {
    free(grf);
  }

  if(ggout || gaout || gfout) {
    free(gr);
    free(gd.bds); 
  } 


  return(0);

}

void malloc_err(gaint i) {
  printf("Malloc error # %d \n",i);
  exit(69);
}
void gaprnt (gaint i, char *ch) {
  printf ("%s",ch);
}


void command_line_help(void) {
/*--- 
  output command line options 
---*/

printf("gribscan utility for GrADS Version " GRADS_VERSION " \n");
printf("extracts grid and variable information from GRIB data\n");
printf("command line options: \n");
printf("          -help   Just this help\n");
printf("          -i      input GRIB file\n");
printf("          -v      verbose listing\n");
printf("          -d      comma deliminited format for the listing\n");
printf("          -q      list essential GRIB parameters for the GrADS interface\n");
printf("          -q1     list only one line of essential GRIB parameters for the GrADS interface\n");
printf("          -S      silent option (no listing)\n");
printf("          -gv     list variable names, description and units based on the NCEP tables\n");
printf("          -gd     display info from the Grid Definition Section\n");
printf("          -bd     display info from the Binary Data Section\n");
printf("          -o      output file name (default is zy0x1w2.EEE)\n");
printf("                EEE = asc for ASCII output\n");
printf("                EEE = dat for binary float output\n");
printf("                EEE = grb for GRIBoutput\n");
printf("          -og     output GRIB\n");
printf("          -of     output binary floats\n");
printf("          -og     output ASCII\n");
printf("          -spNNNN output only parameter number NNNN\n");
printf("          -slNNNN output only level NNNN\n");
printf("          -stNNNN output only forecast time NNNN\n");
printf("          -sNNNN  where NNNN is the maximum number of bytes to skip between GRIB messages (default is 1000)\n");
printf("          -hnmc   SPECIAL OPTION for NCEP (before the name change from NMC)\n\n");
printf("   Example:\n\n");
printf("   gribscan -gd -bd -d -i mygrib.data\n");

}

char *gxgsym(char *ch) {
  return (getenv(ch));
}


