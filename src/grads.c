/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* Main program for GrADS (Grid Analysis and Display System).
   This program loops on commands from the user, and calls the
   appropriate routines based on the command entered.             */

/* GrADS is originally authored by Brian E. Doty.  
   Many others have contributed over the years...  

   Jennifer M. Adams      Tom Holt 	      Uwe Schulzweida 
   Reinhard Budich 	  Don Hooper 	      Arlindo da Silva
   Luigi Calori 	  James L. Kinter     Michael Timlin  
   Wesley Ebisuzaki 	  Steve Lord 	      Pedro Tsai 	   
   Mike Fiorino 	  Gary Love 	      Joe Wielgosz    
   Graziano Giuliani 	  Karin Meier 	      Brian Wilkinson 
   Matthias Grzeschik	  Matthias Munnich    Katja Winger    

   We apologize if we have neglected your contribution --
   but let us know so we can add your name to this list.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#else /* undef HAVE_CONFIG_H */
#include <malloc.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include "grads.h"

#if USEGUI == 1
#include "gagui.h"
#endif

#if READLINE ==1
#include <time.h>
#include <readline/history.h>
extern gaint history_length;
void write_command_log(char *logfile);
#endif

static struct gaupb *upba=NULL;     /* Anchor for user defined plug-in */
char *gxgnam(char *);               /* This is also in gx.h */
static struct gacmn gcmn;  
static struct gawgds wgds; 
extern struct gamfcmn mfcmn;

/* * * * * * * * * * * * * * * * *
 * Here's where it all begins... *
 * * * * * * * * * * * * * * * * */
#ifdef OPENGRADS
/* For opengrads, which needs a capital M for its 'main' routine */
int Main (int argc, char *argv[])  {
#else
#ifdef SHRDOBJ
/* For the gradspy shared object, which can't have a 'main' routine */
int gamain (int argc, char *argv[])  {  
#else
/* For the stand-alone grads exectuable */
int main (int argc, char *argv[])  {     
#endif
#endif

  void gapysavpcm(struct gacmn *pcm);
  void command_line_help(void) ;
  void gxdgeo (char *);
  void gxend (void);
  void gatxti(gaint on, gaint cs);
  char *gatxtl(char *str,gaint level);
  
  char cmd[1024];
  gaint rc,i,j,land,port,cmdflg,hstflg,gflag,killflg,ratioflg;
  gaint metabuff,size=0,g2size=0,geomflg=0;
  gaint gxd=0,gxp=0;
  char gxdopt[16],gxpopt[16],xgeom[100]; 
  gaint txtcs=-2;
  gaint ipcflg = 0; /* for IPC friendly interaction via pipes */
  char *icmd,*arg,*rc1;
  void gasigcpu() ;
  gaint wrhstflg=0; 
  gadouble aspratio;
  char *logfile,*userhome=NULL;
  
  /*--- common block sets before gainit ---*/
  gcmn.batflg = 0;
  land = 0;
  port = 0;
  cmdflg = 0;
  metabuff = 0;
  hstflg = 1; 
  gflag = 0;
  icmd = NULL;
  arg = NULL;
  rc1 = NULL;
  killflg = 0;
  ratioflg = 0;
  aspratio = -999.9;
  gxdopt[0] = '\0';
  gxpopt[0] = '\0';
  xgeom[0] = '\0';
  
  
#if READLINE == 1
#ifdef __GO32__  /* MSDOS case */ 
  logfile= (char *) malloc(22);
  logfile= "c:\windows\grads.log";
#else  /* Unix */
  userhome=getenv("HOME");
  if (userhome==NULL) {
    logfile=NULL;
  }
  else {
    logfile= (char *) malloc(strlen(userhome)+12);
    if(logfile==NULL) {
      printf("Memory allocation error for logfile name.\n");
    }
    else {
      strcpy(logfile,userhome);
      strcat(logfile,"/.grads.log");
    }
  }
#endif /* __GO32__ */
#endif /* READLINE == 1 */
  
  if (argc>1) {
    for (i=1; i<argc; i++) {
      /* user needs help */
      if (*(argv[i]+0)=='-' &&
	  *(argv[i]+1)=='h' &&
	  *(argv[i]+2)=='e' &&
	  *(argv[i]+3)=='l' &&
	  *(argv[i]+4)=='p') {
	command_line_help(); 
	return(0);
      } else if (cmdflg==1) {    /* next arg is the command to execute */
	icmd = argv[i];
	cmdflg = 0;
      } else if (metabuff) {     /* next arg is the metafile buffer size */
	arg = argv[i];
	rc1 = intprs(arg,&size);
	if (rc1!=NULL) metabuff = 0;
      } else if (gxd) {     /* next arg is the graphics display back end choice */
	snprintf(gxdopt,15,"%s",argv[i]);
	gxd=0;
      } else if (gxp) {     /* next arg is the graphics hardcopy printing back end choice */
	snprintf(gxpopt,15,"%s",argv[i]);
	gxp=0;
      } else if ((txtcs==-1) && (*argv[i]!='-')) {  /* next arg is the colorization scheme */
	txtcs = (gaint) strtol(argv[i], (char **)NULL, 10);
	if (txtcs>2) {
	  printf("Valid colorization schemes are 0, 1, or 2. Option %d will be ignored.\n",txtcs);
	  txtcs = -2;
	}
      } else if (ratioflg) {                      /* next arg is aspect ratio */
        aspratio = atof(argv[i]);
        ratioflg = 0;
      } else if (geomflg) {                       /* next arg is the geometry string */
	snprintf(xgeom,100,"%s",argv[i]);
	geomflg=0;
      } else if (wrhstflg && *(argv[i])!='-') {   /* next arg is optional log file name */
        logfile=argv[i];
      } else if (*(argv[i])=='-') {
	j = 1;
	while (*(argv[i]+j)) {
	  if      (*(argv[i]+j)=='a') ratioflg = 1;    /* aspect ratio to follow */
	  else if (*(argv[i]+j)=='b') gcmn.batflg = 1; /* batch mode */
	  else if (*(argv[i]+j)=='c') cmdflg = 1;      /* command to follow */
	  else if (*(argv[i]+j)=='C') txtcs = -1;      /* text color scheme */
	  else if (*(argv[i]+j)=='d') gxd = 1;         /* graphics display back end choice  */
	  else if (*(argv[i]+j)=='E') hstflg = 0;      /* disable command line editing */
	  else if (*(argv[i]+j)=='g') geomflg = 1;     /* geometry specification to follow */
	  else if (*(argv[i]+j)=='h') gxp = 1;         /* graphics hardcopy printing back end choice */
	  else if (*(argv[i]+j)=='H') wrhstflg = 1;    /* write history to log file */
	  else if (*(argv[i]+j)=='l') land = 1;        /* landscape mode */
	  else if (*(argv[i]+j)=='m') metabuff = 1;    /* metafile buffer size to follow */
	  else if (*(argv[i]+j)=='p') port = 1;        /* portrait mode */
	  else if (*(argv[i]+j)=='u') {                /* unbuffer output: needed for IPC via pipes */
	    hstflg = 0;                                /* no need for readline in IPC mode */
	    ipcflg = 1;
	    setvbuf(stdin,  (char *) NULL,  _IONBF, 0 );
	    setvbuf(stdout, (char *) NULL,  _IONBF, 0 );
	    setvbuf(stderr, (char *) NULL,  _IONBF, 0 );
	  }
	  else if (*(argv[i]+j)=='x') killflg = 1;     /* quit after finishing (usually used with -c) */
	  else printf ("Unknown command line option: %c\n",*(argv[i]+j));
	  j++;
	}
      } else printf ("Unknown command line keyword: %s\n",argv[i]);
    }
  }
 
  if (txtcs > -2) gatxti(1,txtcs); /* turn on text colorizing */
  
  if (ratioflg==1) printf ("Note: -a option was specified, but no aspect ratio was provided\n");
  if (cmdflg==1)   printf ("Note: -c option was specified, but no command was provided\n");
  if (geomflg==1)  printf ("Note: -g option was specified, but no geometry specification was provided\n");
  if (metabuff==1) printf ("Note: -m option was specified, but no metafile buffer size was provided\n");
  
  if (ipcflg) printf("\n<IPC>" );  /* delimit splash screen */
 
  printf("\nGrid Analysis and Display System (GrADS) Version %s\n",gatxtl(GRADS_VERSION,0));
  printf("Copyright (C) 1988-2018 by George Mason University\n");
  printf("GrADS comes with ABSOLUTELY NO WARRANTY\n");
  printf("See file COPYRIGHT for more information\n\n");
  
  gacfg(0);
 
#ifdef OPENGRADS
  gaudi(&gcmn); /* Initialize OpenGrADS User Defined Extensions */
#endif
  if (!land && !port && aspratio<-990) {
    nxtcmd (cmd,"Landscape mode? ('n' for portrait): ");
    if (cmd[0]=='n') port = 1;
  }
  if (port) {
    gcmn.xsiz = 8.5;
    gcmn.ysiz = 11.0;
  } else {
    gcmn.xsiz = 11.0;
    gcmn.ysiz = 8.5;
  }
  if (aspratio>-990) { /* user has specified aspect ratio */
    if (aspratio>0.2 && aspratio < 5.0) {   /* range is limited here. */
      if (aspratio < 1.0) {
	gcmn.xsiz = 11.0*aspratio;
	gcmn.ysiz = 11.0;
      } else {
	gcmn.ysiz = 11.0/aspratio;
	gcmn.xsiz = 11.0;
      }
    }
    else {
      printf("Warning: Aspect ratio must be between 0.2 and 5.0 -- defaulting to landscape mode\n");
    }
  }
  gainit();
  mfcmn.cal365=-999;
  mfcmn.warnflg=2;
  mfcmn.winx=-999;      /* Window x  */         
  mfcmn.winy=-999;      /* Window y */     
  mfcmn.winw=0;         /* Window width */ 
  mfcmn.winh=0;         /* Window height */ 
  mfcmn.winb=0;         /* Window border width */
  gcmn.pfi1 = NULL;     /* No data sets open      */
  gcmn.pfid = NULL;
  gcmn.fnum = 0;
  gcmn.dfnum = 0;
  gcmn.undef = -9.99e8; /* default undef value */
  gcmn.fseq = 10; 
  gcmn.pdf1 = NULL;
  gcmn.sdfwname = NULL;
  gcmn.sdfwtype = 1;
  gcmn.sdfwpad = 0;
  gcmn.sdfrecdim = 0;
  gcmn.sdfchunk = 0;
  gcmn.sdfzip = 0;
  gcmn.sdfprec = 8;
  gcmn.ncwid = -999;
  gcmn.xchunk = 0;
  gcmn.ychunk = 0;
  gcmn.zchunk = 0;
  gcmn.tchunk = 0;
  gcmn.echunk = 0;
  gcmn.attr = NULL;
  gcmn.ffile = NULL;
  gcmn.sfile = NULL;
  gcmn.fwname = NULL;
  gcmn.gtifname = NULL;    /* for GeoTIFF output */
  gcmn.tifname = NULL;     /* for KML output */
  gcmn.kmlname = NULL;     /* for KML output */
  gcmn.kmlflg = 1;         /* default KML output is an image file */
  gcmn.shpfname = NULL;    /* for shapefile output */
  gcmn.shptype = 2;        /* default shape type is line */
  gcmn.fwenflg = BYTEORDER;
  gcmn.fwsqflg = 0;        /* default is stream */
  gcmn.fwexflg = 0;        /* default is not exact -- old bad way */
  gcmn.gtifflg = 1;        /* default geotiff output format is float */
  if (size) gcmn.hbufsz = size;
  if (g2size) gcmn.g2bufsz = g2size;
  gcmn.fillpoly = -1;      /* default is to not fill shapefile polygons */
  gcmn.marktype = 3;       /* default is to draw points as closed circe */
  gcmn.marksize = 0.05;    /* default mark size */
  for (i=0; i<32; i++) {
    gcmn.clct[i] = NULL;  /* initialize collection pointers */
    gcmn.clctnm[i] = 0;
  }
  if (xgeom[0]!='\0') 
    snprintf(gcmn.xgeom,100,"%s",xgeom);    /* copy user-specified window geometry string   */
  if (gxdopt[0]!='\0') 
    snprintf(gcmn.gxdopt,15,"%s",gxdopt);   /* copy user-specified GX display plug-in name  */
  else 
    snprintf(gcmn.gxdopt,15,"Cairo");       /* ... if not specified, default is Cairo       */
  if (gxpopt[0]!='\0') 
    snprintf(gcmn.gxpopt,15,"%s",gxpopt);   /* copy user-specified GX printing plug-in name */
  else 
    snprintf(gcmn.gxpopt,15,"Cairo");       /* ... if not specified, default is Cairo       */

  /* Read the user defined plug-in table */
  gaudpdef();  

  /* Give gafunc.c the anchor for chain of upb structures */
  setupba (upba);
 
  /* if graphics (subroutine gxstrt) failed to initialize, quit */
  rc=gagx(&gcmn);
  if (rc) exit(rc);

  /* Inform gaio.c what the global scale factor for netcdf4/hdf5 cache */
  setcachesf(gcmn.cachesf);

#if !defined(__CYGWIN32__) && !defined(__GO32__)
  signal(CPULIMSIG, gasigcpu) ;  /* CPU time limit signal; just exit */
#endif
  
#if READLINE == 1
  if (wrhstflg && logfile != NULL) {
    printf("Command line history in %s\n",logfile);
    history_truncate_file(logfile,256); 
    read_history(logfile); /* read last 256 cmd */
  }
#endif

  if (icmd) rc = gacmd(icmd,&gcmn,0);   /* execute command given on startup */
  else      rc = 0;                     /* set up for entering the main command line loop */
  
  signal(2,gasig);                      /* Trap cntrl c */
  
#if USEGUI == 1
  if (!ipcflg) 
    gagui_main (argc, argv);   /*ams Initializes GAGUI, and if the environment
				 variable GAGUI is set it starts a GUI
				 script. Otherwise, it just returns. ams*/
#endif
  if (ipcflg) printf("\n<RC> %d </RC>\n</IPC>\n",rc);
  
/* This is for GradsPy */
#ifdef SHRDOBJ
  gapysavpcm(&gcmn);
  return(0);  
#endif

  /* Main command line loop */
  while (rc>-1) {
    
    if (killflg) return(99);
    
#if READLINE == 1
#if defined(MM_NEW_PROMPT) 
    char prompt[13];
    if (hstflg) {
      snprintf(prompt,12,"ga[%d]> ",history_length+1);
      rc=nxrdln(&cmd[0],prompt);
    }
#else
    if (hstflg) rc=nxrdln(&cmd[0],"ga-> ");
#endif
    else rc=nxtcmd(&cmd[0],"ga> ");
#else
    rc=nxtcmd(&cmd[0],"ga> ");
#endif
    
    if (rc < 0) {
      strcpy(cmd,"quit");   /* on EOF, just quit */
      printf("[EOF]\n");
    }
    
    if (ipcflg) printf("\n<IPC> %s", cmd );  /* echo command in IPC mode */
    
    gcmn.sig = 0;
    rc = gacmd(cmd,&gcmn,0);
    
    if (ipcflg) printf("\n<RC> %d </RC>\n</IPC>\n",rc);
  }
  
  /* All done */
  gxend();
  
#if READLINE == 1
  if (wrhstflg) write_command_log(logfile);
#endif
  
  exit(0);
  
}  /* end of main routine */

/* Initialize most gacmn values. Values involving real page size,
   and values involving open files are not modified   */
void gainit (void) {
gaint i;

  gcmn.wgds = &wgds;
  gcmn.wgds->fname = NULL;
  gcmn.wgds->opts = NULL;
  gcmn.hbufsz = 1000000;
  gcmn.g2bufsz = 10000000;
  gcmn.loopdim = 3;
  gcmn.csmth = 0;
  gcmn.cterp = 1;
  gcmn.cint = 0;
  gcmn.cflag = 0;
  gcmn.ccflg = 0;
  gcmn.cmin = -9.99e33;
  gcmn.cmax = 9.99e33;
  gcmn.arrflg = 0;
  gcmn.arlflg = 1;
  gcmn.ahdsiz = 0.05;
  gcmn.hemflg = -1;
  gcmn.aflag = 0;
  gcmn.axflg = 0;
  gcmn.ayflg = 0;
  gcmn.rotate = 0;
  gcmn.xflip = 0;
  gcmn.yflip = 0;
  gcmn.gridln = -9;
  gcmn.zlog = 0;
  gcmn.log1d = 0;
  gcmn.coslat = 0;
  gcmn.numgrd = 0;
  gcmn.gout0 = 0;
  gcmn.gout1 = 1;
  gcmn.gout1a = 0;
  gcmn.gout2a = 1;
  gcmn.gout2b = 4;
  gcmn.goutstn = 1;
  gcmn.cmark = -9;
  gcmn.grflag = 1;
  gcmn.grstyl = 5;
  gcmn.grthck = 4;
  gcmn.grcolr = 15;
  gcmn.blkflg = 0;
  gcmn.dignum = 0;
  gcmn.digsiz = 0.07;
  gcmn.reccol = 1;
  gcmn.recthk = 3;
  gcmn.lincol = 1;
  gcmn.linstl = 1;
  gcmn.linthk = 3;
  gcmn.mproj = 2;
  gcmn.mpdraw = 1;
  gcmn.mpflg = 0;
  gcmn.mapcol = -9; gcmn.mapstl = 1; gcmn.mapthk = 1;
  for (i=0; i<256; i++) {
    gcmn.mpcols[i] = -9;
    gcmn.mpstls[i] = 1;
    gcmn.mpthks[i] = 3;
  }
  gcmn.mpcols[0] = -1;  gcmn.mpcols[1] = -1; gcmn.mpcols[2] = -1;
  for (i=0; i<8; i++) {
    if (gcmn.mpdset[i]) gree(gcmn.mpdset[i],"g1");
    gcmn.mpdset[i] = NULL;
  }
  gcmn.mpdset[0] = (char *)galloc(7,"mpdset");
  *(gcmn.mpdset[0]+0) = 'l'; 
  *(gcmn.mpdset[0]+1) = 'o';
  *(gcmn.mpdset[0]+2) = 'w'; 
  *(gcmn.mpdset[0]+3) = 'r';
  *(gcmn.mpdset[0]+4) = 'e'; 
  *(gcmn.mpdset[0]+5) = 's';
  *(gcmn.mpdset[0]+6) = '\0';
  for (i=1;i<8;i++) gcmn.mpdset[i]=NULL;

  gcmn.strcol = 1;
  gcmn.strthk = 3;
  gcmn.strjst = 0;
  gcmn.strrot = 0.0;
  gcmn.strhsz = 0.1;
  gcmn.strvsz = 0.12;
  gcmn.anncol = 1;
  gcmn.annthk = 5;
  gcmn.tlsupp = 0;
  gcmn.xlcol = 1;
  gcmn.ylcol = 1;
  gcmn.xlthck = 4;
  gcmn.ylthck = 4;
  gcmn.xlsiz = 0.11;
  gcmn.ylsiz = 0.11;
  gcmn.xlflg = 0;
  gcmn.ylflg = 0;
  gcmn.xtick = 1;
  gcmn.ytick = 1;
  gcmn.xlint = 0.0;
  gcmn.ylint = 0.0;
  gcmn.xlpos = 0.0;
  gcmn.ylpos = 0.0;
  gcmn.ylpflg = 0;
  gcmn.yllow = 0.0;
  gcmn.xlside = 0;
  gcmn.ylside = 0;
  gcmn.clsiz = 0.09;
  gcmn.clcol = -1;
  gcmn.clthck = -1;
  gcmn.stidflg = 0;
  gcmn.grdsflg = 1;
  gcmn.timelabflg = 1;
  gcmn.stnprintflg = 0;
  gcmn.fgcnt = 0;
  gcmn.barbolin = 0;
  gcmn.barflg = 0;
  gcmn.bargap = 0;
  gcmn.barolin = 0;
  gcmn.clab = 1;
  gcmn.clskip = 1;
  gcmn.xlab = 1;
  gcmn.ylab = 1;
  gcmn.clstr = NULL;
  gcmn.xlstr = NULL;
  gcmn.ylstr = NULL;
  gcmn.xlabs = NULL;
  gcmn.ylabs = NULL;
  gcmn.dbflg = 0;
  gcmn.rainmn = 0.0;
  gcmn.rainmx = 0.0;
  gcmn.rbflg = 0;
  gcmn.miconn = 0;
  gcmn.impflg = 0;
  gcmn.impcmd = 1;
  gcmn.strmden = 5;
  gcmn.strmarrd = 0.4;
  gcmn.strmarrsz = 0.05;
  gcmn.strmarrt = 1;
  gcmn.frame = 1;
  gcmn.pxsize = gcmn.xsiz;
  gcmn.pysize = gcmn.ysiz;
  gcmn.vpflag = 0;
  gcmn.xsiz1 = 0.0;
  gcmn.xsiz2 = gcmn.xsiz;
  gcmn.ysiz1 = 0.0;
  gcmn.ysiz2 = gcmn.ysiz;
  gcmn.paflg = 0;
  for (i=0; i<10; i++) gcmn.gpass[i] = 0;
  gcmn.btnfc = 1;
  gcmn.btnbc = 0;
  gcmn.btnoc = 1;
  gcmn.btnoc2 = 1;
  gcmn.btnftc = 1;
  gcmn.btnbtc = 0;
  gcmn.btnotc = 1;
  gcmn.btnotc2 = 1;
  gcmn.btnthk = 3;
  gcmn.dlgpc = -1;
  gcmn.dlgfc = -1;
  gcmn.dlgbc = -1;
  gcmn.dlgoc = -1;
  gcmn.dlgth = 3;
  gcmn.dlgnu = 0;
  for (i=0; i<15; i++) gcmn.drvals[i] = 1;
  gcmn.drvals[1] = 0; gcmn.drvals[5] = 0;
  gcmn.drvals[9] = 0;
  gcmn.drvals[14] = 1;
  gcmn.sig = 0;
  gcmn.lfc1 = 2;
  gcmn.lfc2 = 3;
  gcmn.wxcols[0] = 2; gcmn.wxcols[1] = 10; gcmn.wxcols[2] = 11;
  gcmn.wxcols[3] = 7; gcmn.wxcols[4] = 15;
  gcmn.wxopt = 1;
  gcmn.ptflg = 0;
  gcmn.ptopt = 1;
  gcmn.ptden = 5;
  gcmn.ptang = 0;
  gcmn.statflg = 0;
  gcmn.prstr = NULL;  gcmn.prlnum = 8; 
  gcmn.prbnum = 1; gcmn.prudef = 0;
  gcmn.dwrnflg = 1;
  gcmn.xexflg = 0; gcmn.yexflg = 0;
  gcmn.cachesf = 1;
  gcmn.dblen = 12;
  gcmn.dbprec = 6;
  gcmn.loopflg = 0;
  gcmn.aaflg = 1;
  gcmn.xgeom[0] = '\0';
  gcmn.gxdopt[0] = '\0';
  gcmn.gxpopt[0] = '\0';
}

void gasig(gaint i) {
  if (gcmn.sig) exit(0);
  gcmn.sig = 1;
}

gaint gaqsig (void) {
  return(gcmn.sig);
}

#if READLINE == 1
/* write command history to log file */
void write_command_log(char *logfile) {
   char QuitLabel[60];
   time_t thetime;
   struct tm *ltime;
   if ((thetime=time(NULL))!=0) {  
      ltime=localtime(&thetime);
      strftime(QuitLabel,59,"quit # (End of session: %d%h%Y, %T)",ltime);
      remove_history(where_history());
      add_history(QuitLabel);
   }
   write_history(logfile);  
   return;
}
#endif

/* output command line options */

void command_line_help (void) {
  printf("\nCommand line options for GrADS version " GRADS_VERSION ": \n\n");
  printf("    -help       Prints out this help message \n");
  printf("    -a ratio    Sets the aspect ratio of the real page \n");
  printf("    -b          Enables batch mode (graphics window is not opened) \n");
  printf("    -c cmd      Executes the command 'cmd' after startup \n");
  printf("    -C N        Enables colorization of text with color scheme N (default scheme is 0)\n");
  printf("    -d name     Name of the graphics display package, given in 'gxdisplay' entry of UDPT\n");
  printf("    -E          Disables command line editing \n");
  printf("    -g WxH+X+Y  Sets size of graphics window \n");
  printf("    -H fname    Enables command line logging to file 'fname' (default fname is $HOME/.grads.log) \n");
  printf("    -h name     Name of the graphics hardcopy printing package, given in 'gxprint' entry of UDPT\n");
  printf("    -l          Starts in landscape mode with real page size 11 x 8.5 \n");
  printf("    -p          Starts in portrait mode with real page size 8.5 x 11 \n");
  printf("    -m NNN      Sets metafile buffer size to NNN (must be an integer) \n");
  printf("    -u          Unbuffers stdout/stderr, disables command line editing \n");
  printf("    -x          Causes GrADS to automatically quit after executing supplied command (used with -c) \n");
  printf("\n    Options that do not require arguments may be concatenated \n");
  printf("\nExamples:\n");
  printf("   grads -pb \n");
  printf("   grads -lbxc \"myscript.gs\" \n");
  printf("   grads -Ca 1.7778 \n");
  printf("   grads -C 2 -a 1.7778 \n");
  printf("   grads -pHm 5000000 -g 1100x850+70+0 \n");
  printf("   grads -pH mysession.log -m 5000000 -g 1100x850+70+0 \n\n");
}
 
/* For CPU time limit signal */
void gasigcpu(gaint i) {  
  exit(1) ;
}

/* Routine to read the user defined plug-in table(s)
   and build link list of plug-in definition blocks.
   The table file name is pointed to by the GAUDPT environment variable 
   and/or a file called "udpt" in the GADDIR directory */

void gaudpdef (void) {
struct gaupb *upb, *oupb=NULL;
char *cname=NULL;
FILE *cfile;
char *ch,ptype[16],rec[2000];
gaint i,sz,pass,line,err;
char newname[1000];
gaint len;

  /* Make two passes:
     1. Read user specified plug-in table (in GAUDPT), 
     2. Read system plug-in table (in GADDIR) */
  pass = 0;
  while (pass<2) {
    if (pass==0) {
      /* check if user has set the GAUDPT environment variable */
      cname = getenv("GAUDPT");
      if (cname==NULL) {
        pass++;
        continue;
      }
      cfile = fopen(cname,"r");
      if (cfile==NULL) {
        pass++;
        continue;
      }
    }
    /* check for a file called "udpt" in the GADDIR directory */
    else {
      cname = gxgnam("udpt");
      cfile = fopen(cname,"r");
      if (cfile==NULL) {
	break;
      }
    }
    
    /* Read the file. */
    line=0;
    while (1) {
      /* Read a record from the file */
      ch = fgets(rec,2000,cfile);
      if (ch==NULL) break;
      ch = rec;
      line++;
      err=0;
      if (*ch=='*' || *ch=='#') continue;            /* Check if this is a comment field */
      /* allocate memory */
      upb = (struct gaupb *)malloc(sizeof(struct gaupb));
      if (upb==NULL) goto memerr;
      while (*ch==' ') ch++;                                 /* move past leading blanks */

      /* parse the plug-in type keyword*/
      i = 0;
      while (*ch!=' ' && *ch!='\0' && *ch!='\n') {
        if (i<15) {
          ptype[i] = *ch;
          i++;
        }
        ch++;
      }
      ptype[i] = '\0';
      lowcas(ptype);                               
      upb->type=0;
      if (!strncmp(ptype,"function",8))  upb->type=1; 
      /* if (!strncmp(ptype,"defop",5))     upb->type=2; */
      if (!strncmp(ptype,"gxdisplay",9)) upb->type=3; 
      if (!strncmp(ptype,"gxprint",7))   upb->type=4; 
      if (upb->type==0) { err=1; goto fmterr; }
      while (*ch==' ') ch++;                          /* move past any in-between blanks */

      /* parse the plug-in name, must be 15 characters or less */
      upb->name[0] = '\0';
      i = 0;
      while (*ch!=' ' && *ch!='\0' && *ch!='\n') {
        if (i<15) {
          upb->name[i] = *ch;
          i++;
        }
        ch++;
      }
      upb->name[i] = '\0';
      if (i==0) { err=2; goto fmterr; }
      if (upb->type<=2) lowcas(upb->name);    /* gaexpr needs functions to be lower case */
      while (*ch==' ') ch++;                          /* move past any in-between blanks */


      /* check for the shared object filename */
      sz = 0;
      while (*(ch+sz)!=' '&&*(ch+sz)!='\n'&&*(ch+sz)!='\0') sz++;   /* no spaces allowed */
      if (sz==0) { err=3; goto fmterr; }

    fmterr:
      if (err) {
	printf("Warning: Format error in line %d of %s\n",line,cname);
	if (err==1) printf("  Plug-in type must be either 'function', 'gxdisplay', or 'gxprint'\n");
	if (err==2) printf("  Plug-in name and shared object filename are missing\n"); 
	if (err==3) printf("  Shared object filename is missing\n"); 
        printf("  This record will be ignored\n");
	if (upb!=NULL) free(upb);
	continue;
      }
      
      /* look for ^ or $ at beginning of the shared object filename */
      if (*ch=='^' || *ch=='$') 
        fnmexp (newname,ch,cname);
      else
        getwrd (newname,ch,1000);
      len = strlen(newname);

      /* allocate memory for shared object filename, copy it */
      upb->fname = NULL;
      upb->fname = (char *)malloc(len+2);
      if (upb->fname==NULL) { free(upb); goto memerr; }
      for (i=0; i<len; i++) upb->fname[i] = newname[i];
      upb->fname[len] = '\0';

      for (i=0; i<sz; i++) ch++;          /* move past the shared object file name */
      while (*ch==' ') ch++;              /* move past any in-between blanks */

      if (upb->type<=2) {
      /* Look for an optional alias that is the actual function name 
	 in the plug-in source code, which may not meet the requirements 
	 for GrADS function names. The alias size is hardcoded
	 to be no more than 512 characters (that ought to be big enough) */
	upb->alias[0] = '\0';
	i = 0;
	while (*ch!=' ' && *ch!='\0' && *ch!='\n') {
	  if (i<512) {
	    upb->alias[i] = *ch;
	    i++;
	  }
	  ch++;
	}
	upb->alias[i] = '\0';
	
	if (i==0) {
	  /* user did not provide an alias, so we'll just copy the name */
	  for (i=0; i<16; i++) upb->alias[i] = upb->name[i];
	}
      }

      /* Add this entry to the chain */
      upb->pfunc = NULL;
      upb->upb = NULL;
      if (upba==NULL) upba = upb;
      else oupb->upb = upb;
      oupb = upb;
    } /* matches while(1) */

    fclose (cfile);
    if (pass>0 && cname!=NULL) gree(cname,"f306");
    pass++;
  } /* matches while (pass<2) */

  return;

memerr:
  printf("Memory allocation error when parsing user defined plug-in table\n");
  return;
}


/* Search the contents of the chain of upb structures
   If the name and type match, return the pointer to the fname */
char * gaqupb (char *name, gaint type) {
struct gaupb *upb;
  upb = upba;
  while (upb) {
    if (cmpwrd(upb->name,name) && upb->type==type) return (upb->fname);
    upb = upb->upb;
  }
  return (NULL);
}

