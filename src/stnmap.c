/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT for more information. */

/* This program creates a station map file given the name of a control
   file for a station data set.  It reads the control file, then
   using information from the control file, it reads the data file,
   then creates the map file (using the name specified in the control
   file).  */

/* 
 * Include ./configure's header file
 */
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

gaint help=0;
void command_line_help(void) ;

gaint skstn (void);
gaint rdstn (char *, gaint);

FILE *dfile=NULL,*mfile;
struct gafile *pfi;
char *ifi,*fn=NULL,*ch=NULL;
off_t fpos;

gaint main (gaint argc, char *argv[])  {
  struct gavar *pvar;
  struct rpthdr hdr;
  struct dt dtim, dtimi;
  char rec[256],cname[256],stid[10];
  off_t pos;
  gaint i,j,scnt,lcnt,rc,mxtim,mxcnt,mxsiz,cnt,siz,mpsiz;
  gaint rdw,rtot,verb,iarg,flg,quiet;
  gaint sizhdr,idum;
  gaint *map;
  gaint diag=0;
  gaint vermap=2;          /* default version */ 
  gaint noread=0;
  gaint flag;
  gaint maxlevels=250;
  gaint maxlevelsflag=0;
  gaint offset;

/* Look at command line args */
  ifi = NULL;
  verb = 0;
  quiet = 0;

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
	  ifi = argv[iarg];
	  flg = 0;
	} else iarg--;		/* Let error message pop */
      }
      else if (*ch=='-' && *(ch+1)=='v') {
	verb = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='q') {
	quiet = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='0') {
        noread = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='M') {
	maxlevelsflag=1;
        ch+=2;
        i = 0;
        while(*(ch+i) && i<20) {
          if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
          i++;
        }
        if (i<20) {
          sscanf(ch,"%i",&maxlevels);
          flg = 0;
        }
	if(i == 0) goto err1;
	if(maxlevels < 10 || maxlevels > 10000) goto err1a;
      }
      else if (*ch=='-' && *(ch+1)=='0') {
        noread = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='0') {
        noread = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='1') {
        vermap = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='2') {
        vermap = 2;
	flg = 0;
      }
      if (flg) {
	printf ("Invalid command line argument: %s  Ignored.\n",argv[iarg]);
      }
      iarg++;
    }
  }

  if(!noread && maxlevelsflag) goto err1;
 
  if (ifi==NULL) {
    command_line_help();
    nxtcmd (rec,"Enter stn ctl filename: ");
    getwrd (cname,rec,255);
    ifi = cname;
  }

  /* Scan descriptor file */
  pfi = getpfi();
  if (pfi==NULL) {
    printf ("Memory allocation error \n");
    return (1);
  }
  rc = gaddes (ifi, pfi, 0);
  if (rc) {
    printf ("File name is:  %s\n",ifi);
    return(1);
  }

/* Check that descriptor file is for station data, and that
   the variables are in the correct order (surface before levels) */
  if (pfi->type!=2) {
    printf ("Descriptor file is not for station data\n");
    return (1);
  }
  pvar = pfi->pvar1;
  lcnt=0; scnt=0;
  for (i=0; i<pfi->vnum; i++) {
    if (pvar->levels==0) {
      if (lcnt>0) {
	printf ("Variable records not properly ordered\n");
	printf ("   Surface variables must appear first\n");
	return(1);
      }
      scnt++;
    } else {
      lcnt++;
    }
    pvar++;
  }
  if (pfi->mnam==NULL) {
    printf ("Station map file name not in descriptor file\n");
    return (1);
  }

/* We are ready to start reading the binary station data file.
   We read for the specified number of times, and build our
   table of time starting points in the file. */
  if(! quiet) {
    printf ("  Name of binary data set: %s\n",pfi->name);
    printf ("  Number of times in the data set: %i\n",pfi->dnum[3]);
    printf ("  Number of surface variables: %i\n",scnt);
    printf ("  Number of level dependent variables: %i\n\n",lcnt);
    printf ("Starting scan of station data binary file.\n");
  }
  sizhdr = sizeof(struct rpthdr);
  mpsiz = pfi->dnum[3]*2 + 2;
  map = (gaint *)malloc(sizeof(gaint)*mpsiz);
  if (map==NULL) {
    printf ("  Error allocating memory.  \n");
    printf ("  Probable cause: Invalid TDEF record\n");
    return(1);
  }
  mxtim = 0;
  mxcnt = -1;
  mxsiz = 0;
  i = 0;
  if (!pfi->tmplat) {
    dfile = fopen(pfi->name,"rb");
    if (dfile==NULL) {
      printf ("Error opening station data binary file\n");
      printf (" File name is: %s\n",pfi->name);
      return (1);
    }
    fpos = 0;
    if (verb || ( !quiet ) ) printf ("Binary data file open: %s\n",pfi->name);
  }
  while (i<pfi->dnum[3]) {
    if (verb || ( !quiet ) ) printf ("\nProcessing time step %i\n",i+1);
    if (pfi->tmplat) {
      gr2t(pfi->grvals[3],(gadouble)(i+1),&dtim);
      gr2t(pfi->grvals[3],1.0,&dtimi);
      ch = gafndt(pfi->name,&dtim, &dtimi,pfi->abvals[3],pfi->pchsub1,pfi->ens1,i+1,1,&flag);
      if (ch==NULL) {
	printf ("Memory allocation error\n");
	return (1);
      }
      if (i==0 || strcmp(ch,fn)!=0) {     
	/* we need to open a new file */
	if (i>0) {
	  /* close the current file */
	  if (fn != NULL) free(fn);
	  if (dfile != NULL) { fclose(dfile); dfile = NULL; }
	}
	fn = ch;
	dfile = fopen(fn,"rb");
	if (dfile==NULL) {
	  printf ("  Error opening binary data file %s\n",fn);
	}
	else {
	  if (verb || (!quiet)) printf ("  Binary data file open: %s\n",fn);
	}
	fpos = 0;
      } 
      else free(ch);
    }
    cnt = 0;
    pos = fpos;
    if(noread == 0) {
      while (1) {
	if (dfile==NULL) break;
	if (!pfi->seqflg) {
	  rc = skstn();
	  if (rc) return (rc);
	} else {
	  rc = rdstn ((char *)(&rdw), 4);
	  if (rc) return (rc);
	  if (pfi->bswap) gabswp((gafloat *)(&rdw),1);
	}
	siz = sizhdr;
	rc = rdstn ((char *)(&hdr),siz);

	if (rc) return (rc);
	if (pfi->bswap) gahswp(&hdr);
      
	if (hdr.flag<0 || hdr.flag>1 || hdr.nlev<0 || hdr.nlev>10000 ||
	    hdr.t < -2.0 || hdr.t>2.0 ) {
	  printf ("  Invalid station hdr found in station binary file\n");
	  printf ("  Possible causes:  Invalid level count in hdr\n");
	  printf ("                    Descriptor file mismatch\n");
	  printf ("                    File not station data\n");
	  printf ("                    Invalid relative time\n");
	  if (pfi->seqflg) printf ("                    Invalid sequential format\n");
	  printf ("    levs = %i  flag = %i  time = %g \n", hdr.nlev,hdr.flag,hdr.t);
	  if (pfi->tmplat) printf("  File name = %s\n",fn);
	  return(1);
	}
	if (hdr.flag) {
	  siz = scnt + (hdr.nlev-1)*(lcnt+1);
	} else {
	  siz = hdr.nlev*(lcnt+1);
	}
	if (siz>mxsiz) mxsiz=siz;
	siz = sizeof(gafloat)*siz + sizeof(struct rpthdr);
	if (hdr.nlev==0) {
	  siz = sizhdr;
	}
	if (verb) {
	  for (j=0; j<8; j++) stid[j] = hdr.id[j];
	  stid[8] = '\0';
	  printf ("  ID,LON,LAT,T,NLEV,FLAG: ");
	  printf ("%s %8.3f %7.3f %g %i %i  ",stid,hdr.lon,hdr.lat,hdr.t,hdr.nlev,hdr.flag);
	  printf ("SIZE = %i\n",siz);
	}
	if (pfi->seqflg) {
	  if (diag) printf ("   Seq Rec Lens: %i",rdw);
	  rtot = rdw;
	  while (rtot<=siz) {
	    fpos = fpos + rdw + 8;
	    rc = skstn();
	    if (rc) return (rc);
	    if (rtot==siz) break;
	    rc = rdstn ((char *)(&rdw), 4);
	    if (rc) return (rc);
	    if (pfi->bswap) gabswp((gafloat *)(&rdw),1);
	    if (diag) printf (" %i",rdw);
	    rtot += rdw;
	  }
	  if (diag) printf ("\n");
	  if (rtot>siz) {
	    printf ("Sequential Read Error: ");
	    printf ("Record size greater than one station report: %d :: %d\n",rtot,siz);
	    return (1);
	  }  
	} else {
	  fpos += siz;
	}
	if (hdr.nlev==0) break;
	cnt++;
      }
    }
    *(map+i+2) = pos;
    *(map+pfi->dnum[3]+i+2) = cnt;
    if(! quiet) printf ("     Time = %i has stn count = %i \n",i+1,cnt);
    if (cnt>mxcnt) { mxcnt = cnt; mxtim = i+1; }
    i++;
  }
  *(map) = pfi->dnum[3];
  if(noread)  mxsiz=maxlevels ;
  *(map+1) = mxsiz;
  if(! quiet) {
    printf ("  Max reports per time:  %i reports at t = %i\n",mxcnt,mxtim);
    printf ("  Max data elements in largest report: %i\n",mxsiz);
  }
  mfile = fopen (pfi->mnam, "wb");
  if (mfile==NULL) {
    printf ("  Could not open output map data set: %s \n",pfi->mnam);
    return(1);
  }
  if( vermap == 1) {
    fwrite (map,sizeof(gaint),mpsiz,mfile);
  } 
  else if (vermap == 2) {
    strncpy(rec,"GrADS_stnmapV002",16);
    fwrite(rec,1,16,mfile);
    for(i=0;i<mpsiz;i++) {
      idum=*(map+i);
      offset=i*4;
      if(idum < 0) {
	idum=-idum;
	gapby(idum,(unsigned char*)map,offset,4);
	gapbb(1,(unsigned char*)map,offset*8,1);
      } else {
	gapby(idum,(unsigned char*)map,offset,4);
      }
    }
    fwrite (map,4,mpsiz,mfile);
  }

  if(! quiet) {
    printf ("\nVersion %1d Station map file created: %s\n\n",vermap,pfi->mnam);
    if(vermap == 2) {
      printf("stnmap: WARNING!!  This stnmap file can only be accessed by GrADS Version " GRADS_VERSION "\n");
      printf("stnmap: WARNING!!  However, GrADS Version " GRADS_VERSION " can read both versions\n\n");
      printf("stnmap: COMMENT  -- use the -1 command line option to create a map for older GrADS versions\n");
    }
    if(verb) {
      printf("\nstnmap: COMMENT  -- use the -q command line option to disable the station header listing\n");
    }

  }

  return(0);


 err1:
  gaprnt (0,"-------------------------------------------------------------------\n\n");
  gaprnt (0,"-M option Error:  maxlevels for -0 option not set or -0 not set\n\n");
  gaprnt (0,"-------------------------------------------------------------------\n\n");
  command_line_help();
  return(1);

 err1a:
  gaprnt (0,"-------------------------------------------------------------------\n\n");
  gaprnt (0,"-M option Error:  maxlevels is < 10 or > 10000 ; not a good idea for -0\n\n");
  gaprnt (0,"-------------------------------------------------------------------\n\n");
  command_line_help();
  return(1);

}


gaint skstn (void) {
gaint rc;
  rc = fseeko(dfile, fpos, 0);
  if (rc!=0) {
    printf ("  Low Level I/O Error:  Seek error on data file \n");
    printf ("  Error occurred when seeking to byte %lld \n",fpos);
    printf ("  Possible cause:  Fewer times than expected\n");
    if (pfi->seqflg) printf ("  Possible cause:  Invalid sequential format\n");
    if (pfi->tmplat) printf("  File name = %s\n",fn);
    return(1);
  }
  return (0);
}

gaint rdstn (char *rec, gaint siz) {
gaint rc;
  rc = fread (rec, 1, siz, dfile);
  if (rc<siz) {
    printf ("  Low Level I/O Error:  Read error on data file \n");
    printf ("  Error reading %i bytes at location %lld \n", siz, fpos);
    printf ("  Possible cause: Premature EOF\n");
    if (pfi->seqflg) printf ("  Possible cause: Invalid sequential data\n");
    if (pfi->tmplat) printf("  File name = %s\n",fn);
    return(1);
  }
  return (0);
}

void gaprnt (gaint i, char *ch) {
  printf ("%s",ch);
}


void command_line_help(void) {

printf("stnmap for GrADS Version " GRADS_VERSION "\n\n");
printf("Create the \"map\" file for using station data in grads\n\n");
printf("Command line options: \n\n");
printf("          -help   Just this help\n");
printf("          -i      the data descriptor file (.ctl) to map\n");
printf("          -v      turn ON verbose listing\n");
printf("          -q      very quiet mode\n");
printf("          -0      DO NOT READ - used for templating and no variation in variables\n");
printf("          -MLLLL  for -0 set max number of levels LLLL (e.g., -M1000 for 1000 levels)\n");
printf("          -1      create a machine specific version 1 map \n");
printf("          -2      create a machine-INDEPENDENT version 2 map (the default for " GRADS_VERSION " and above\n\n");
printf("   Example:\n\n");
printf("   stnmap -i station_data.ctl\n");
printf("   stnmap -0 -M100 -i station_data.ctl ;  do not read the data \n\n");

}

/* Query env symbol */

char *gxgsym(char *ch) {
  return (getenv(ch));
}
