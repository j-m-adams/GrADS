/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT for more information*/

/*  Written by Brian Doty and Jennifer M. Adams  */

#ifdef HAVE_CONFIG_H
#include "config.h"

/* If autoconfed, only include malloc.h when it's present */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#else /* undef HAVE_CONFIG_H */

#include <malloc.h>

#endif /* HAVE_CONFIG_H */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "gatypes.h"

/* global variables */
struct dt {                          /* Date/time structure */
  gaint yr;
  gaint mo;
  gaint dy;
  gaint hr;
  gaint mn;
};

struct gag2 {
  gaint discipline,parcat,parnum;     /* Parameter identifiers */
  gaint yr,mo,dy,hr,mn,sc;            /* Reference Time */
  gaint sig;                          /* Significance of Reference Time */
  gaint numdp;                        /* Number of data points */
  gaint gdt;                          /* Grid Definition Template */
  gaint pdt;                          /* Product Definition Template */
  gaint drt;                          /* Data Representation Template */
  gaint trui;                         /* Time range units indicator */
  gaint ftime;                        /* Forecast time */
  gaint lev1type,lev1sf,lev1;         /* Level 1 type, scale factor, scaled value */
  gaint lev2type,lev2sf,lev2;         /* Level 2 type, scale factor, scaled value */
  gaint enstype,enspertnum,ensderiv;  /* Ensemble metadata */
  gaint comptype;                     /* Compression type (for JPEG2000 compression) */
  gaint bmsflg;                       /* Bit Map Section flag */
};
gaint verb=0,ctl=1;
char levs[100],units[100],extra[100];

/* Function Declarations */
gaint gagby (unsigned char *, gaint, gaint);
gaint gagbb (unsigned char *, gaint, gaint);
gaint sect1 (unsigned char *, struct gag2 *);
gaint sect3 (unsigned char *, struct gag2 *);
gaint sect4 (unsigned char *, struct gag2 *);
gaint sect5 (unsigned char *, struct gag2 *);
void CodeTable0p0  (gaint);
void CodeTable1p2  (gaint);
void CodeTable3p1  (gaint);
void CodeTable4p4  (gaint);
void CodeTable4p7  (gaint);
void CodeTable4p10 (gaint);
void CodeTable4p15 (gaint);
void CodeTable5p0  (gaint);
gadouble scaled2dbl(gaint, gaint);
void timadd (struct dt *, struct dt *);


/* MAIN PROGRAM */
gaint main (gaint argc, char *argv[]) {
 FILE *gfile=NULL;
 struct gag2 g2;
 off_t fpos;
 char *ch;
 unsigned char s0[16],work[260],*rec;
 unsigned char *s1,*s2,*s3,*s4,*s5,*s6,*s7,*s8;
 gaint i,flg,iarg,rc,edition,rlen,roff,field,recnum;
 gaint s1len,s2len,s3len=0,s4len,s5len,s6len,s7len;

  
 /* Scan input args. The filename is the argument that is not an option */
 flg = 0;
 if (argc>1) {
   iarg = 1;
   while (iarg<argc) {
     ch = argv[iarg];
     if (*(ch)=='-' && *(ch+1)=='v') { 
       verb=1;
     } 
     else if (*(ch)=='-' && *(ch+1)=='V') { 
       verb=2;
     } 
     else { 
       gfile = fopen(ch,"rb");
       if (gfile==NULL) {
	 printf ("Could not open input file %s\n",ch);
	 return(1);
       }
       flg = 1;
     }
     iarg++;
   }
 } 
 if (!flg) {
   printf ("Usage: grib2scan [-v] filename\n");
   printf ("       Use -v for verbose output \n");
   return (1);
 }

 /* Main loop through all GRIB records in the file */
 fpos = 0;
 recnum=1;
 while (recnum) {
   
   /* Scan for the 'GRIB' char string */
   flg = 0;
   while (1) {
     rc = fseeko(gfile,fpos,SEEK_SET);
     rc = fread(work,1,260,gfile);
/*      if (rc<260) { */
     if (rc==0) {
       printf ("EOF rc=%d\n",rc);
       return(0);
     }
     for (i=0; i<257; i++) { 
       if (*(work+i+0)=='G' && 
	   *(work+i+1)=='R' &&
	   *(work+i+2)=='I' && 
	   *(work+i+3)=='B') { 
	 fpos = fpos + (off_t)i;
	 flg = 1;
	 i = 999;
       }
     }
     if (flg) break;
     else fpos = fpos + (off_t)(256);
   }
   
   /* Section 0, the Indicator Section */
   rc = fseeko(gfile,fpos,SEEK_SET);
   rc = fread(s0,1,16,gfile);
   if (rc < 16) {
     printf ("I/O Error Reading Section 0\n");
     return(99);
   }
   if (s0[0]!='G' || s0[1]!='R' || s0[2]!='I' || s0[3]!='B') {
     printf ("GRIB indicator not found \n");
     return (2);
   }
   edition = gagby(s0,7,1);
   if (edition != 2) {
     printf ("GRIB edition number not 2; value = %i\n",edition);
     return (3);
   }
   rlen = gagby(s0,8,4);   /* Note: need to fix gamach.c and elsewhere */  
   if (rlen!=0) {
     printf ("GRIB record length too large\n");
     return (4);
   }
   rlen = gagby(s0,12,4);   
   g2.discipline = gagby(s0,6,1);
   i = fpos;
   printf ("\nRecord %d ",recnum);
   if (verb) printf("starts at %i of length %i \n",i,rlen); 
   else printf("\n");
   if (verb) {
     printf (" Discipline=%d ",g2.discipline);
     CodeTable0p0(g2.discipline);
     printf ("\n");
   }
   
   /* Read the entire GRIB 2 record */
   rec = (unsigned char *)malloc(rlen);
   if (rec==NULL) {
     printf ("memory allocation error\n");
     return (99);
   }
   rc = fseeko(gfile,fpos,SEEK_SET);
   rc = fread(rec,1,rlen,gfile);
   if (rc!=rlen) {
     printf ("I/O Error reading GRIB2 record \n");
     return(99);
   }
   
   /* Section 1, the Identification Section */
   roff = 16;
   s1 = rec+roff;              
   i = gagby(s1,4,1);
   if (i!=1) {
     printf ("Header error, section 1 expected, found %i\n",i);
     return (5);
   }
   s1len = gagby(s1, 0,4);     
   rc = sect1(s1,&g2);
   if (rc) return (rc);
   roff += s1len;
   
   /* Loop over multiple fields */
   field = 1;
   while (1) {
     
     printf (" Field %d ",field);
     if (verb>1) printf("starts at location %i\n",roff);
     else printf("\n");

     /* Section 2, the Local Use Section */
     s2 = rec+roff;
     i = gagby(s2,4,1);
     if (i==2) {
       s2len = gagby(s2,0,4);
       roff += s2len;
     } else s2len = 0;
     
     /* Section 3, the Grid Definition Section */
     s3 = rec+roff;
     i = gagby(s3,4,1);
     if (i==3) {
       s3len = gagby(s3,0,4);  
       rc = sect3(s3,&g2);
       if (rc) return (rc);
       roff += s3len;
     } else if (field==1) {
       printf ("Header error, section 3 expected, found %i\n",i);
       return (5);
     } 
     
     /* Section 4, the Product Definition Section */
     s4 = rec+roff;
     i = gagby(s4,4,1);
     if (i!=4) {
       printf ("Header error, section 4 expected, found %i\n",i);
       return (5);
     }
     s4len  = gagby(s4,0,4);
     rc = sect4(s4,&g2);
     if (rc) return (rc);
     roff += s4len;

     /* Section 5, the Data Representation Section */
     s5 = rec+roff;
     i = gagby(s5,4,1);
     if (i!=5) {
       printf ("Header error, section 5 expected, found %i\n",i);
       return (5);
     }
     s5len = gagby(s5,0,4);
     rc = sect5(s5,&g2);
     if (rc) return (rc);
     roff += s5len;
     
     /* Section 6, the Bit Map Section*/
     s6 = rec+roff;
     i = gagby(s6,4,1);
     if (i==6) {
       s6len = gagby(s6,0,4);
       g2.bmsflg = gagby(s6,5,1);
       if (verb>1) printf("  BMI=%d \n",g2.bmsflg);
       roff += s6len;
     } else s6len = 0;
     
     /* Section 7, the Data Section */
     s7 = rec+roff;
     i = gagby(s7,4,1);
     if (i!=7) {
       printf ("Header error, section 7 expected, found %i\n",i);
       return (5);
     }
     s7len = gagby(s7,0,4);
     if (verb>1) printf ("  Lengths of Sections 1-7: %i %i %i %i %i %i %i\n",
		       s1len, s2len, s3len, s4len, s5len, s6len, s7len);
     roff += s7len;
     
     if (ctl) printf(" ctl:  var%d.%d   0,%s  %s  %s   description\n",recnum,field,levs,extra,units);  

     /* Section 8, the End Section */
     s8 = rec+roff;
     if (*s8=='7' && *(s8+1)=='7' && *(s8+2)=='7' && *(s8+3)=='7') {
       break;
     }
     

     /* If it wasn't the End, look for another field */
     field++;
   }
   
   fpos = fpos + (off_t)rlen;
   free(rec);
   recnum++;
 }
 return(0);
}

/* Look at contents of Section 1 */
gaint sect1 (unsigned char *s1, struct gag2 *pg2) {
  pg2->sig = gagby(s1,11,1);     
  pg2->yr  = gagby(s1,12,2);
  pg2->mo  = gagby(s1,14,1);
  pg2->dy  = gagby(s1,15,1);
  pg2->hr  = gagby(s1,16,1);
  pg2->mn  = gagby(s1,17,1);
  pg2->sc  = gagby(s1,18,1);
  printf (" Reference Time = %4i-%02i-%02i %02i:%02i:%02i  ",
	  pg2->yr,pg2->mo,pg2->dy,pg2->hr,pg2->mn,pg2->sc); 
  CodeTable1p2(pg2->sig);
  printf("\n");
  return(0);

 }

/* Look at contents of Section 3 */
gaint sect3 (unsigned char *s3, struct gag2 *pg2) {
  gaint nx,ny,angle,lat1,lon1,di,dj,lon2,lat2,nlats;
  gaint rlon,dx,dy,latin1,latin2;
  gaint sbit1,sbit2,sbit3,sbit4;

  pg2->numdp = gagby(s3,6,4);    
  pg2->gdt   = gagby(s3,12,2);
  printf ("  GDT=%i ",pg2->gdt);
  CodeTable3p1(pg2->gdt);
  printf(" nx*ny=%d\n",pg2->numdp);
  if (verb) {
    if (pg2->gdt==0) {         /* lat-lon grid */
      nx    = gagby(s3,30,4);
      ny    = gagby(s3,34,4);
      angle = gagby(s3,38,4);
      if (angle==0) {
	lat1 = gagby(s3,46,4);
	lon1 = gagby(s3,50,4);
	di   = gagby(s3,63,4);
	dj   = gagby(s3,67,4);
	printf("   XDEF %i linear %f %f\n",nx,lon1*1e-6,di*1e-6);
	printf("   YDEF %i linear %f %f\n",ny,lat1*1e-6,dj*1e-6);
      }
    }
    else if (pg2->gdt==20) {   /* Polar Stereographic */
      nx    = gagby(s3,30,4);
      ny    = gagby(s3,34,4);
      lat1  = gagby(s3,38,4);
      lon1  = gagby(s3,42,4);
      rlon  = gagby(s3,51,4);
      dx    = gagby(s3,55,4);
      dy    = gagby(s3,59,4);
      printf("   nx=%i ny=%i lon1=%f lat1=%f\n",nx, ny, lon1*1e-6, lat1*1e-6);
      printf("   reflon=%f dx=%f dy=%f\n",rlon*1e-6,dx*1e-3,dy*1e-3); 

    }
    else if (pg2->gdt==30) {   /* Lambert Conformal */
      nx     = gagby(s3,30,4);
      ny     = gagby(s3,34,4);
      lat1   = gagby(s3,38,4);
      lon1   = gagby(s3,42,4);
      rlon   = gagby(s3,51,4);
      dx     = gagby(s3,55,4);
      dy     = gagby(s3,59,4);
      latin1 = gagby(s3,65,4);
      latin2 = gagby(s3,69,4);
      printf("   nx=%i ny=%i lon1=%f lat1=%f\n",nx, ny, lon1*1e-6, lat1*1e-6);
      printf("   reflon=%f dx=%f dy=%f\n",rlon*1e-6, dx*1e-3, dy*1e-3); 
      printf("   latin1=%f latin2=%f \n",latin1*1e-6, latin2*1e-6);
      if (verb>1) {
	sbit1 = gagbb(s3+64,0,1);
	sbit2 = gagbb(s3+64,1,1);
	sbit3 = gagbb(s3+64,2,1);
	sbit4 = gagbb(s3+64,3,1);
	printf("   scan_mode_bits = %i%i%i%i\n",sbit1, sbit2, sbit3, sbit4);
      }      
    }
    else if (pg2->gdt==40) {   /* Gaussian lat-lon grid */
      nx    = gagby(s3,30,4);
      ny    = gagby(s3,34,4);
      angle = gagby(s3,38,4);
      if (angle==0) {
	lat1  = gagby(s3,46,4);
	lon1  = gagby(s3,50,4);
	lat2  = gagby(s3,55,4);
	lon2  = gagby(s3,59,4);
	di    = gagby(s3,63,4);
	nlats = gagby(s3,67,4);
	printf("   nx=%i ny=%i di=%f nlats=%i\n",nx, ny, di*1e-6,nlats);
	printf("   lon: %f to %f\n", lon1*1e-6, lon2*1e-6);
	printf("   lat: %f to %f\n", lat1*1e-6, lat2*1e-6);
      }
    }
  }
  return (0);
}

/* Look at contents of Section 4 */
gaint sect4 (unsigned char *s4, struct gag2 *pg2) {
  struct dt reft,fcst,endt,versiont;
  gaint enstotal,sp,sp2,gotfcst;
  gaint ptyp,llsf,llval,ulsf,ulval,i,pctle;
  gadouble ll=0,ul=0;
  gadouble lev1=0,lev2=0;
  gaint var1,var2,var3,var4,var5,numtr,pos1=0,pos2=0,pos3=0;
  gaint off,atyp,styp,s1sf,s1sval,s2sf,s2sval,wtyp,w1sf,w1sval,w2sf,w2sval;
  gadouble s1,s2,w1,w2;
  
  pg2->pdt      = gagby(s4, 7,2);
  pg2->parcat   = gagby(s4, 9,1); 
  pg2->parnum   = gagby(s4,10,1); 
  /* PDT 48 has 24 unique octets inserted after octet 11 instead of starting at octet 35 */
  off=0;
  if (pg2->pdt==48) off=24; 
  pg2->trui     = gagby(s4,17+off,1); 
  pg2->ftime    = gagby(s4,18+off,4); 
  pg2->lev1type = gagby(s4,22+off,1);
  pg2->lev1sf   = gagby(s4,23+off,1);
  /* check for a negative level1 value */
  pg2->lev1 = gagbb(s4,(24+off)*8+1,31);
  i = gagbb(s4,(24+off)*8,1);
  if (i) pg2->lev1 = -1.0*pg2->lev1;
  pg2->lev2type = gagby(s4,28+off,1);
  pg2->lev2sf   = gagby(s4,29+off,1);
  /* check for a negative level2 value */
  pg2->lev2 = gagbb(s4,(30+off)*8+1,31);
  i = gagbb(s4,(30+off)*8,1);
  if (i) pg2->lev2 = -1.0*pg2->lev2;

  /* get the reference time */
  reft.yr = pg2->yr;
  reft.mo = pg2->mo;
  reft.dy = pg2->dy;
  reft.hr = pg2->hr;
  reft.mn = pg2->mn;
  /* initialize forecast time structure */
  fcst.yr = fcst.mo = fcst.dy = fcst.hr = fcst.mn = 0;  
  gotfcst=0;
  if      (pg2->trui== 0) {fcst.mn = pg2->ftime; gotfcst=1;}
  else if (pg2->trui== 1) {fcst.hr = pg2->ftime; gotfcst=1;}
  else if (pg2->trui== 2) {fcst.dy = pg2->ftime; gotfcst=1;}
  else if (pg2->trui== 3) {fcst.mo = pg2->ftime; gotfcst=1;}
  else if (pg2->trui== 4) {fcst.yr = pg2->ftime; gotfcst=1;}
  else if (pg2->trui==10) {fcst.hr = pg2->ftime*3; gotfcst=1;}   /* 3Hr incr */
  else if (pg2->trui==11) {fcst.hr = pg2->ftime*6; gotfcst=1;}   /* 6Hr incr */
  else if (pg2->trui==12) {fcst.hr = pg2->ftime*12; gotfcst=1;}  /* 2Hr incr */
  if (gotfcst) {
    /* add reference time and forecast time together */
    timadd(&reft,&fcst);
  }

  sp = -1;
  sp2 = -1;
  printf("  PDT=%d",pg2->pdt);
  /* instantaneous fields */
  if (pg2->pdt<=7 || pg2->pdt==15 || pg2->pdt==48 || pg2->pdt==60) {
    if (pg2->pdt<=7 || pg2->pdt==48 || pg2->pdt==60) {
      printf ("  %d ",pg2->ftime);
      CodeTable4p4(pg2->trui);
      printf ("Forecast");
      if (pg2->pdt == 60) {
	pos3=37;
	/* get the model version date */
	versiont.yr = gagby(s4,pos3+0,2);
	versiont.mo = gagby(s4,pos3+2,1);
	versiont.dy = gagby(s4,pos3+3,1);
	versiont.hr = gagby(s4,pos3+4,1);
	versiont.mn = gagby(s4,pos3+5,1);
      }
    }
    else if (pg2->pdt==15) {
      /* for PDT 4.15, get info about the type of spatial and statistical processing used */
      /* pos2 is the location of octet for statistical process code (sp), 
	 subsequent octet is for spatial process */
      pos2=34;
      sp  = gagby(s4,pos2+0,1);
      sp2 = gagby(s4,pos2+1,1);
      printf("  Spatial ");
      CodeTable4p10(sp);
      printf(", ");
      CodeTable4p15(sp2);
    }
    printf("  Valid Time = %4i-%02i-%02i %02i:%02i  ",fcst.yr,fcst.mo,fcst.dy,fcst.hr,fcst.mn);
    if (pg2->pdt == 60) printf("  Version = %4i%02i%02i",versiont.yr,versiont.mo,versiont.dy);
  }
  /* fields spanning a time interval */
  else if ((pg2->pdt>=8 && pg2->pdt<=12) || pg2->pdt==61) {
    /* get the beg/end times and statistical process  */
    /* pos1 is location of octets describing end of overall time period */
    /* pos2 is the location of octet for statistical process code (sp) */
    if      (pg2->pdt ==  8) {pos1=34; pos2=46;}
    else if (pg2->pdt ==  9) {pos1=47; pos2=59;}
    else if (pg2->pdt == 10) {pos1=35; pos2=47;}
    else if (pg2->pdt == 11) {pos1=37; pos2=49;}
    else if (pg2->pdt == 12) {pos1=36; pos2=48;}
    else if (pg2->pdt == 61) {pos1=44; pos2=56; pos3=37;}
    
    /* get the model version date */
    versiont.yr = gagby(s4,pos3+0,2);
    versiont.mo = gagby(s4,pos3+2,1);
    versiont.dy = gagby(s4,pos3+3,1);
    versiont.hr = gagby(s4,pos3+4,1);
    versiont.mn = gagby(s4,pos3+5,1);

    /* get the ending time of the overall averaging period */
    endt.yr = gagby(s4,pos1+0,2);
    endt.mo = gagby(s4,pos1+2,1);
    endt.dy = gagby(s4,pos1+3,1);
    endt.hr = gagby(s4,pos1+4,1);
    endt.mn = gagby(s4,pos1+5,1);

    /* get number of time range specifications */
    numtr = gagby(s4,pos1+7,1);       

    /* get info about outermost (or only) statistical process */
    if (numtr) {
      sp   = gagby(s4,pos2+0,1);  /* statistical process */
      var1 = gagby(s4,pos2+1,1);  /* not used, type of time increment... */
      var2 = gagby(s4,pos2+2,1);  /* time unit for var3 */
      var3 = gagby(s4,pos2+3,4);  /* length of time over which statistical processing is done */
      var4 = gagby(s4,pos2+7,1);  /* not used, time unit for var5 */
      var5 = gagby(s4,pos2+8,4);  /* not used, time increment between successive fields, 0 if continuous */
      printf("  %d ",var3);
      CodeTable4p4(var2);
      if (sp==255)
        printf(" Interval");
      else
        CodeTable4p10(sp);
      printf("  BegTime = %4i-%02i-%02i %02i:%02i",fcst.yr,fcst.mo,fcst.dy,fcst.hr,fcst.mn);
      printf("  EndTime = %4i-%02i-%02i %02i:%02i",endt.yr,endt.mo,endt.dy,endt.hr,endt.mn);
      printf("  Version = %4i%02i%02i",versiont.yr,versiont.mo,versiont.dy);
      if ((verb) & (numtr>1)) printf("\n   (Record has %d time range specifications for statistical processing)",numtr);
    }
  }
  printf(" \n");

  /* Print the grib codes that should be included in the descriptor file entry */
  if (sp<0 || sp==255) {
    printf ("   Parameter: disc,cat,num = %d,%d,%d\n",pg2->discipline,pg2->parcat,pg2->parnum);
    if (ctl) sprintf (units,"%d,%d,%d",pg2->discipline,pg2->parcat,pg2->parnum);
  }
  else {
    if (sp2<0) {
      printf ("   Parameter: disc,cat,num,sp = %d,%d,%d,%d\n",pg2->discipline,pg2->parcat,pg2->parnum,sp);
      if (ctl) sprintf (units,"%d,%d,%d,%d",pg2->discipline,pg2->parcat,pg2->parnum,sp);
    }
    else {
      printf ("   Parameter: disc,cat,num,sp,sp2 = %d,%d,%d,%d,%d\n",pg2->discipline,pg2->parcat,pg2->parnum,sp,sp2);
      if (ctl) sprintf (units,"%d,%d,%d,%d,%d",pg2->discipline,pg2->parcat,pg2->parnum,sp,sp2);
    }
  }

  /* Get level information */
  if (pg2->lev1 != -1) lev1 = scaled2dbl(pg2->lev1sf,pg2->lev1);
  if (pg2->lev2type != 255) {  /* we have two level types */
    if (pg2->lev2 != -1) {
      lev2 = scaled2dbl(pg2->lev2sf,pg2->lev2);
      if (pg2->lev2type == pg2->lev1type) {
	printf ("   Levels: ltype,lval,lval2 = %d,%g,%g ",pg2->lev1type,lev1,lev2);
	if (ctl) sprintf (levs,"%d,%g,%g",pg2->lev1type,lev1,lev2);
      }
      else {
	printf ("   Levels: ltype,lval,lval2,ltype2 = %d,%g,%g,%d ",pg2->lev1type,lev1,lev2,pg2->lev2type);
	if (ctl) sprintf (levs,"%d,%g,%g,%d",pg2->lev1type,lev1,lev2,pg2->lev2type);
      }
      if (verb) 
	printf("  (sf1,sval1,sf2,sval2 = %d %d %d %d) \n",pg2->lev1sf,pg2->lev1,pg2->lev2sf,pg2->lev2);
      else printf ("\n");
    }
    else {  /* level values are missing */
      if (pg2->lev2type == pg2->lev1type) {
	printf ("   Levels: ltype = %d \n",pg2->lev1type);
	if (ctl) sprintf (levs,"%d",pg2->lev1type);
      }
      else {
	printf ("   Levels: ltype,,,ltype2 = %d,,,%d \n",pg2->lev1type,pg2->lev2type);
	if (ctl) sprintf (levs,"%d,,,%d",pg2->lev1type,pg2->lev2type);
      }
    }
  }	
  else {    /* only one level type */
    if (pg2->lev1 != -1 && pg2->lev1type>10) {     /* some level types don't have an associated level value */
      printf ("   Level: ltype,lval = %d,%g ",pg2->lev1type,lev1); 
      if (ctl) sprintf (levs,"%d,%g",pg2->lev1type,lev1);
      if (verb) printf("  (sf,sval = %d %d)",pg2->lev1sf,pg2->lev1);
      printf ("\n");
    }
    else {
      printf ("   Level: ltype = %d \n",pg2->lev1type);  /* level value is missing */
      if (ctl) sprintf (levs,"%d",pg2->lev1type);
    }
  }

  /* Ensemble Metadata */
  if (pg2->pdt==1 || pg2->pdt==2 || pg2->pdt==11 || pg2->pdt==12 || pg2->pdt==60 || pg2->pdt==61) {
    if (pg2->pdt==1 || pg2->pdt==11 || pg2->pdt==60 || pg2->pdt==61) {   /* individual ensemble members */
      pg2->enstype    = gagby(s4,34,1); 
      pg2->enspertnum = gagby(s4,35,1);
      enstotal        = gagby(s4,36,1);
      printf("   Ens: type,pert = %d,%d ",pg2->enstype,pg2->enspertnum); 
      if (verb) printf("(total=%d) \n",enstotal);
      else printf("\n");
    }
    else {                            /* derived fields from all ensemble members */
      pg2->ensderiv   = gagby(s4,34,1);
      enstotal        = gagby(s4,35,1);
      printf("   Ens: deriv = %d  ",pg2->ensderiv);
      if (verb) CodeTable4p7(pg2->ensderiv);
      printf("\n");
    }
  }

  /* Probability Forecasts */
  /* The value of octet 35 (Forecast Probability Number) is not used by NCEP */
  /* The value of octet 36 (Total Number of Forecast Probabilities) is the ensemble size */
  /* We don't need to look at either of these octets */
  if (pg2->pdt==9 || pg2->pdt==5) {
    ptyp  = gagby(s4,36,1);
    llsf  = gagby(s4,37,1);
    /* check for a negative lower limit value */
    llval = gagbb(s4,38*8+1,31);
    i = gagbb(s4,38*8,1);
    if (i) llval = -1.0*llval;
    ulsf  = gagby(s4,42,1);
    /* check for a negative upper limit value */
    ulval = gagbb(s4,43*8+1,31);
    i = gagbb(s4,43*8,1);
    if (i) ulval = -1.0*ulval;
    ll = scaled2dbl(llsf,llval);
    ul = scaled2dbl(ulsf,ulval);
    if (ptyp<=4) {
      if (ptyp==2) {
	printf ("   Prob: type,llim,ulim = %d,%g,%g",ptyp,ll,ul);
	if (ctl) sprintf (extra,"a%d,%g,%g",ptyp,ll,ul);
      }
      else if (ptyp==0 || ptyp==3) {
	printf ("   Prob: type,lim = %d,%g",ptyp,ll);  
	if (ctl) sprintf (extra,"a%d,%g",ptyp,ll);  
      }
      else {
	printf ("   Prob: type,lim = %d,%g",ptyp,ul);  
	if (ctl) sprintf (extra,"a%d,%g",ptyp,ul);  
      }
    }
    else if (ptyp==255) printf ("   Probability Type is missing");
    else printf ("   Probability Type = %d",ptyp);

    /* decoding code table 4p9 */
    if (verb) {
      if      (ptyp==0) printf ("  (Probability of event below %g)",ll); 
      else if (ptyp==1) printf ("  (Probability of event above %g)",ul);
      else if (ptyp==2) printf ("  (Probability of event between %g and %g)",ll,ul);
      else if (ptyp==3) printf ("  (Probability of event above %g)",ll);
      else if (ptyp==4) printf ("  (Probability of event below %g)",ul);
    }
    if (verb>1) printf ("   (llsf,llval=%d,%d  ulsf,ulval=%d,%d)",llsf,llval,ulsf,ulval);
    printf("\n");
  }

  /* Percentile Forecasts */
  if ((pg2->pdt==6) | (pg2->pdt==10)) {
    pctle = gagby(s4,34,1);
    printf ("   Percentile: %d\n",pctle);
    if (ctl) sprintf (extra,"a%d",pctle);
  }

  /* Optical Properties of Aerosols */
  if (pg2->pdt==48) {
    atyp   = gagby(s4,11,2);
    styp   = gagby(s4,13,1);
    s1sf   = gagby(s4,14,1);
    s1sval = gagby(s4,15,4);
    s2sf   = gagby(s4,19,1);
    s2sval = gagby(s4,20,4);
    wtyp   = gagby(s4,24,1);
    w1sf   = gagby(s4,25,1);
    w1sval = gagby(s4,26,4);
    w2sf   = gagby(s4,30,1);
    w2sval = gagby(s4,31,4);
    s1 = scaled2dbl(s1sf,s1sval);
    s2 = scaled2dbl(s2sf,s2sval);
    w1 = scaled2dbl(w1sf,w1sval);
    w2 = scaled2dbl(w2sf,w2sval);
    if (atyp!=65535) {
      if (styp!=255) {
	if (wtyp!=255) {
	  printf ("   Aerosol: atyp,styp,s1,s2,wtyp,w1,w1 = %d,%d,%g,%g,%d,%g,%g",atyp,styp,s1,s2,wtyp,w1,w2);
	  if (ctl) sprintf (extra,"a%d,%d,%g,%g,%d,%g,%g",atyp,styp,s1,s2,wtyp,w1,w2);
	}
	else {
	  printf ("   Aerosol: atyp,styp,s1,s2 = %d,%d,%g,%g",atyp,styp,s1,s2);
	  if (ctl) sprintf (extra,"a%d,%d,%g,%g",atyp,styp,s1,s2);
	}
      }
      else {
	if (wtyp!=255) {
	  printf ("   Aerosol: atyp,wtyp,w1,w1 = %d,%d,%g,%g",atyp,wtyp,w1,w2);
	  if (ctl) sprintf (extra,"a%d,%d,%g,%g",atyp,wtyp,w1,w2);
	}
	else {
	  printf ("   Aerosol: atyp = %d",atyp);
	  if (ctl) sprintf (extra,"a%d",atyp);
	}
      }
    }
    if (verb) {
      if (styp<=11) {
	/* code table 4p91 */
	if      (styp==0)  printf("  (size is < %g)",s1);
	else if (styp==1)  printf("  (size is > %g)",s2);
	else if (styp==2)  printf("  (size is >= %g and < %g)",s1,s2);
	else if (styp==3)  printf("  (size is > %g)",s1);
	else if (styp==4)  printf("  (size is < %g)",s2);
	else if (styp==5)  printf("  (size is <= %g)",s1);
	else if (styp==6)  printf("  (size is >= %g)",s2);
	else if (styp==7)  printf("  (size is >= %g and <= %g)",s1,s2);
	else if (styp==8)  printf("  (size is >= %g)",s1);
	else if (styp==9)  printf("  (size is <= %g)",s2);
	else if (styp==10) printf("  (size is > %g and <= %g)",s1,s2);
	else if (styp==11) printf("  (size is = %g)",s1);
      }
      if (wtyp<=11) {
	/* code table 4p91 */
	if      (wtyp==0)  printf("  (wavelength is < %g)",w1);
	else if (wtyp==1)  printf("  (wavelength is > %g)",w2);
	else if (wtyp==2)  printf("  (wavelength is >= %g and < %g)",w1,w2);
	else if (wtyp==3)  printf("  (wavelength is > %g)",w1);
	else if (wtyp==4)  printf("  (wavelength is < %g)",w2);
	else if (wtyp==5)  printf("  (wavelength is <= %g)",w1);
	else if (wtyp==6)  printf("  (wavelength is >= %g)",w2);
	else if (wtyp==7)  printf("  (wavelength is >= %g and <= %g)",w1,w2);
	else if (wtyp==8)  printf("  (wavelength is >= %g)",w1);
	else if (wtyp==9)  printf("  (wavelength is <= %g)",w2);
	else if (wtyp==10) printf("  (wavelength is > %g and <= %g)",w1,w2);
	else if (wtyp==11) printf("  (wavelength is = %g)",w1);
      }
      if (verb>1) printf("\n            (size: sf1,val1=%d,%d  sf2,val2=%d,%d)",s1sf,s1sval,s2sf,s2sval);
      if (verb>1) printf("  (wavelength: sf1,val1=%d,%d  sf2,val2=%d,%d)",w1sf,w1sval,w2sf,w2sval);
    }
    printf(" \n");
  }
  return(0);
}

/* Look at contents of Section 5 */
gaint sect5 (unsigned char *s5, struct gag2 *pg2) {
  gafloat r;
  gaint e,d,nbits,otype;

  if (verb) {
    pg2->drt = gagby(s5,9,2);
    r        = gagby(s5,11,4);
    e        = gagby(s5,15,2);
    d        = gagby(s5,17,2);
    nbits    = gagby(s5,19,1);
    otype    = gagby(s5,20,1);
    printf ("  DRT=%i ",pg2->drt);
    CodeTable5p0(pg2->drt);
    if (pg2->drt == 40) {  
      pg2->comptype = gagby(s5,21,1);
      if (pg2->comptype==0) printf(" (Lossless) ");
      if (pg2->comptype==1) printf(" (Lossy) ");
    }
    printf("\n");
  }
  return(0);
}


/* Discipline */
void CodeTable0p0 (gaint i) {
  if      (i== 0) printf ("(Meteorological)");
  else if (i== 1) printf ("(Hydrological)");
  else if (i== 2) printf ("(Land Surface)");
  else if (i== 3) printf ("(Space)");
  else if (i==10) printf ("(Oceanographic)");
}
/* Significance of Reference Time */
void CodeTable1p2 (gaint i) {
  if      (i==0) printf ("(Analysis)");
  else if (i==1) printf ("(Start of Forecast)");
  else if (i==2) printf ("(Verifying Time of Forecast)");
  else if (i==3) printf ("(Observation Time)");
}
/* Grid Definition Template Number */
void CodeTable3p1 (gaint i) {
  if      (i==0)   printf("(Lat/Lon)");
  else if (i==1)   printf("(Rotated Lat/Lon)");
  else if (i==2)   printf("(Stretched Lat/Lon)");
  else if (i==3)   printf("(Rotated and Stretched Lat/Lon)");
  else if (i==10)  printf("(Mercator)");
  else if (i==20)  printf("(Polar Stereographic)");
  else if (i==30)  printf("(Lambert Conformal)");
  else if (i==31)  printf("(Albers Equal Area)");
  else if (i==40)  printf("(Gaussian Lat/Lon)");
  else if (i==41)  printf("(Rotated Gaussian Lat/Lon)");
  else if (i==42)  printf("(Stretched Gaussian Lat/Lon)");
  else if (i==43)  printf("(Rotated and Stretched Gaussian Lat/Lon)");
  else if (i==50)  printf("(Spherical Harmonic Coefficients)");
  else if (i==51)  printf("(Rotated Spherical Harmonic Coefficients)");
  else if (i==52)  printf("(Stretched Spherical Harmonic Coefficients)");
  else if (i==53)  printf("(Rotated and Stretched Spherical Harmonic Coefficients)");
  else if (i==90)  printf("(Orthoraphic)");
  else if (i==100) printf("(Triangular Grid Based on an Icosahedron)");
  else if (i==110) printf("(Equatorial Azimuthal Equidistant Projection)");
  else if (i==120) printf("(Azimuth-Range Projection)");
}
/* Time Range Unit Indicator */
void CodeTable4p4 (gaint i) {
  if      (i==0)  printf ("Minute ");
  else if (i==1)  printf ("Hour ");
  else if (i==2)  printf ("Day ");
  else if (i==3)  printf ("Month ");
  else if (i==4)  printf ("Year ");
  else if (i==5)  printf ("Decade ");
  else if (i==6)  printf ("Normal ");
  else if (i==7)  printf ("Century ");
  else if (i==10) printf ("3-Hour ");
  else if (i==11) printf ("6-Hour ");
  else if (i==12) printf ("12-Hour ");
  else if (i==13) printf ("Second ");
}
/* Derived Ensemble Forecast */
void CodeTable4p7 (gaint i) {
  if      (i==0)  printf ("Unweighted Mean All Members ");
  else if (i==1)  printf ("Weighted Mean All Members ");
  else if (i==2)  printf ("StdDev (Cluster Mean) ");
  else if (i==3)  printf ("StdDev (Cluster Mean, normalized) ");
  else if (i==4)  printf ("Spread of All Members ");
  else if (i==5)  printf ("Large Anomaly Index All Members ");
  else if (i==6)  printf ("Unweighted Mean Cluster Members ");
}
/* Type of Statistical Processing */
void CodeTable4p10 (gaint i) {
  if      (i==0)  printf ("Average");
  else if (i==1)  printf ("Accumulation");
  else if (i==2)  printf ("Maximum");
  else if (i==3)  printf ("Minimum");
  else if (i==4)  printf ("Diff(end-beg)");
  else if (i==5)  printf ("RMS");
  else if (i==6)  printf ("StdDev");
  else if (i==7)  printf ("Covariance");
  else if (i==8)  printf ("Diff(beg-end)");
  else if (i==9)  printf ("Ratio");
}
/* Type of Spatial Processing */
void CodeTable4p15 (gaint i) {
  if      (i==0)  printf ("No interpolation");
  else if (i==1)  printf ("Bilinear interpolation");
  else if (i==2)  printf ("Bicubic interpolation");
  else if (i==3)  printf ("Nearest neighbor");
  else if (i==4)  printf ("Budget interpolation");
  else if (i==5)  printf ("Spectral interpolation");
  else if (i==6)  printf ("Neighbor budget interpolation");
  else            printf ("sp2=%d",i);
}
/* Data Representation Template Number */
void CodeTable5p0 (gaint i) {
  if      (i==0)  printf ("(Grid Point Data - Simple Packing)");
  else if (i==1)  printf ("(Matrix Value at Grid Point - Simple Packing)");
  else if (i==2)  printf ("(Grid Point Data - Complex Packing)");
  else if (i==3)  printf ("(Grid Point Data - Complex Packing and Spatial Differencing)");
  else if (i==40) printf ("(Grid Point Data - JPEG2000 Compression)");
  else if (i==41) printf ("(Grid Point Data - PNG Compression )");
  else if (i==50) printf ("(Spectral Data - Simple Packing)");
  else if (i==51) printf ("(Spectral Data - Simple Packing)");
}

void gaprnt (gaint i, char *ch) {
  printf ("%s",ch);
}
char *gxgsym(char *ch) {
  return (getenv(ch));
}


