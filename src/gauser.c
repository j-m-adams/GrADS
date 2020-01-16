/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* Authored by B. Doty */

#ifdef HAVE_CONFIG_H
#include "config.h"
/* If autoconfed, only include malloc.h when it's present */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#else /* undef HAVE_CONFIG_H */
#include <malloc.h>
#endif /* HAVE_CONFIG_H */

#if USESHP==1
#include "shapefil.h"
#endif

#if USENETCDF == 1
#include "netcdf.h"
#endif 

#if USEHDF == 1
#include "mfhdf.h"
#endif 
#if USEHDF5 == 1
#include "hdf5.h"
#endif 

#if USEGUI == 1
#include "gagui.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "grads.h"
#include "gx.h"

#ifdef OPENGRADS
#include "gaudx.h"
#endif

char *gatxtl(char *str, int level);

extern struct gamfcmn mfcmn;
static char *cdims[7] = {"None","Lon","Lat","Lev","Time","Ens","Val"};
static char *ccdims[6] = {"Xdim","Ydim","Zdim","Tdim","Edim","Val"}; 
static char pout[1256];   /* Build error msgs here */
static struct gacmn *savpcm;
static struct gxdsubs *dsubs=NULL; /* function pointers for display  */
static struct gxpsubs *psubs=NULL; /* function pointers for printing */

void gapysavpcm(struct gacmn *);   /* function prototype, used by gradspy */

/*  Variables to handle message buffering for the script language */
static gaint msgflg = 0;
struct msgbuf {
  struct msgbuf *forw;
  gaint levl;
  gaint len;
  char *msg;
};
static struct msgbuf *msgstk, *msgcurr, *msgnew;


/* Handle all user commands */
gaint gacmd (char *com, struct gacmn *pcm, gaint exflg) {
struct gafile *pfi,*pfi2;
struct gadefn *pdf,*pdf2;
struct gaclct *clct,*clct2;
gaint rc,reinit,fnum,i,j,len,retcod,flag,xin,yin,bwin,fmtflg,tcolor,pos,size,*ioff,rem;
gadouble border,*dx,*dy;
gafloat *rvals=NULL,*ivals=NULL,*jvals=NULL,*ival,*jval;
char cc[260], bgImage[256], fgImage[256], pdefname[256];
char *cmd,*rslt,*ccc,*ch,ext[10];
size_t sz;
char *formats[7] = {"EPS","PS","PDF","SVG","PNG","GIF","JPG"};
FILE *pdefid=NULL;

  ccc = NULL;
  gaiomg();   /* enable interpolation message */
  if (dsubs==NULL) dsubs = getdsubs();  /* get ptrs to the graphics display functions */
  if (psubs==NULL) psubs = getpsubs();  /* get ptrs to the graphics printing functions */

  len = 0;
  while(*(com+len)) len++;
  len++;
  sz = len+1;
  ccc = (char *)galloc(sz,"cmd");
  if (ccc==NULL) {
    gaprnt(0,"Memory allocation error: Command Processing\n");
    return(1);
  }
  for (i=0; i<len; i++) *(ccc+i) = *(com+i);
  *(ccc+len) = '\0';          /* Maintain mixed case version */
  cmd = ccc;
  lowcas (cmd);
  while (*cmd==' ') cmd++;
  while (*com==' ') com++;

  retcod = 0;

  /* Check for implied define */
  flag = 0;
  if (*cmd>='a' && *cmd<='z') {
    i = 0;
    ch = cmd;
    /* defined variable names are alphanumeric, and must start with a letter */
    while ( (*ch>='a' && *ch<='z') || (*ch>='0' && *ch<='9' ) ) {
      i++;
      if (i>16) break;
      ch++;
    }
    if (i<17) {
      while (*ch==' ') ch++;
      if (*ch=='=') {
        flag = 1;
        ch++;
        while (*ch==' ') ch++;
      }
    }
    if (flag) {
      if (pcm->pfid==NULL) {
        gaprnt (0,"DEFINE error:  no file open yet\n");
        retcod = 1;
        goto retrn;
      }
      retcod = gadef (cmd, pcm, 1);
      goto retrn;
    }
  }

#ifdef OPENGRADS
  /* OpenGrADS User Defined Extensions */
  savpcm = pcm;
  retcod = (int) gaudc (com);
  if ( retcod >= 0 ) goto retrn; /* found a udc, all done */
  else retcod = 0;               /* not found, keep going */
#endif

  /* if command is NOT clear ...*/
  if (!(cmpwrd("clear",cmd) || cmpwrd("c",cmd))) {  
    gxfrme (9);  /* ... clear the X request buffer */
  }

  if (*com=='\0' || *com=='\n') goto retrn; 

  if (cmpwrd("quit",cmd)) {
     retcod = -1;
     goto retrn;
  } 

  else if (*cmd=='!') {
    system(com+1);
    goto retrn;

#if READLINE == 1
    /* print history or repeat commands from history  */
  }  
  else if (cmpwrd("history", cmd) ||
	   cmpwrd("his",     cmd) ||
	   cmpwrd("repeat",  cmd) ||
	   cmpwrd("r",       cmd)    ) {
    retcod = gahistory(cmd, com, pcm);
    goto retrn;
#endif
#if USEGUI == 1
  } 
  else if (cmpwrd("gui",cmd)) {
    char *tmp ;
    if ((cmd=nxtwrd(com)) == NULL) {
      gaprnt (0,"GUI error:  No file name specified\n");
      retcod = 1;
      goto retrn;
    } else {
      gaint lentmp ;

      lentmp = (gaint) strlen(cmd) ;
      sz = lentmp + 1;
      tmp = (char *) galloc(sz,"gui") ;
      getwrd(tmp, cmd, lentmp) ;
      retcod = Custom_GUI(tmp);
      if(tmp) gree(tmp,"f186");
      goto retrn;
    }
#endif
#if GRIB2
  } else if (cmpwrd("flush",cmd)) {
    g2clear();
    gaprnt (1,"grib2 cache cleared\n");
    goto retrn;
#endif
  } else if (cmpwrd("stack",cmd)) {
    goto retrn;

  } else if (cmpwrd("reset",cmd) || cmpwrd("reinit",cmd)) {
    /* reset vpage */
    pcm->xsiz = pcm->pxsize;
    pcm->ysiz = pcm->pysize;
    gxvpag (pcm->xsiz, pcm->ysiz, 0.0, pcm->xsiz, 0.0, pcm->ysiz);
    /* reset gacmn and call gacln */
    gainit();
    gacln (pcm,1);
    gacln (pcm,2);
    gacln (pcm,3);
    gacln (pcm,4);
    reinit = 0;
    if (cmpwrd("reinit",cmd)) {
      reinit = 1;
      mfcmn.cal365=-999;
      mfcmn.warnflg=2;
      /* release all define blocks */
      pdf = pcm->pdf1;
      while (pdf) {
        pdf2 = pdf->pforw;
        pfi = pdf->pfi;
        if (pfi->rbuf) gree(pfi->rbuf,"f189a");
        if (pfi->sbuf) gree(pfi->sbuf,"f189b");
        if (pfi->ubuf) gree(pfi->ubuf,"f189c");
	for (i=0; i<5; i++) {
	  gree(pfi->grvals[i],"f189d");
	  gree(pfi->abvals[i],"f189e");
	}
        gree(pfi,"f190");
        gree(pdf,"f191");
        pdf = pdf2;
      }
      /* release all collections */
      for (i=0; i<32; i++) {
	clct = pcm->clct[i];
	while (clct) {
	  gasfre(clct->stn);
	  clct2 = clct->forw;
	  gree(clct,"f218");
	  clct = clct2;
	}
	pcm->clct[i] = NULL;
	pcm->clctnm[i] = 0;
      }
      /* close all files */
      pfi = pcm->pfi1;
      while (pfi) {
        if (pfi->infile) fclose(pfi->infile); 
        if (pfi->mfile)  fclose(pfi->mfile);
	if (pfi->ncflg==1) gaclosenc(pfi);
	if (pfi->ncflg==2) gaclosehdf(pfi);
	if (pfi->ncflg==3) gacloseh5(pfi);
	if (pfi->bufrdset) {            /* bufr station data */
	  gabufr_close(pfi->bufrdset);  /* free memory */
	  pfi->bufrdset=NULL;           /* reset the pointer */
	}
#if USEGADAP
        if (pfi->dhandle > -999) dapclo(pfi);  /* opendap station data */
#endif
        pfi2 = pfi->pforw;
        frepfi(pfi,0);
        pfi = pfi2;
      }
      pcm->pfi1 = NULL;
      pcm->pfid = NULL;
      pcm->fnum = 0;
      pcm->dfnum = 0;
      pcm->undef =  -9.99e8;         /* default undef value */
      pcm->pdf1 = NULL;
      if (pcm->ffile) fclose(pcm->ffile);
      pcm->ffile = NULL;
      if (pcm->sfile) fclose(pcm->sfile);
      pcm->sfile = NULL;
      if (pcm->fwname) gree(pcm->fwname,"f192");
      pcm->fwname = NULL;
#if USENETCDF == 1
      if (pcm->ncwid != -999) nc_close(pcm->ncwid);
#endif
      pcm->fwenflg = BYTEORDER;
#if GRIB2
      g2clear();
#endif
      gaprnt (1,"All files closed; all defined objects released;\n");
    } /* end of actions for 'reinit' */

    /* reset the default file and the dimension environment */
    if (pcm->fnum>0 && pcm->pfi1) {
      pcm->pfid = pcm->pfi1;
      pcm->dfnum = 1;
      pfi = pcm->pfi1;
      if (pfi->type==2 || pfi->wrap) gacmd ("set lon 0 360",pcm,0);
      else {
        snprintf(pout,1255,"set x 1 %i",pfi->dnum[0]);
        gacmd (pout,pcm,0);
      }
      if (pfi->type==2) {
        gacmd ("set lat -90 90",pcm,0);
        gacmd ("set lev 500",pcm,0);
      } else {
        snprintf(pout,1255,"set y 1 %i",pfi->dnum[1]);
        gacmd (pout,pcm,0);

	/* set z to max if x or y = 1 */
	if(pfi->type==1 && pfi->dnum[2] > 1
	   && ( (pfi->dnum[0] == 1) || (pfi->dnum[1] == 1) ) ) {
	  if(pfi->dnum[2] <= 1) {
	    snprintf(pout,1255,"set z 1");
	  } else {
	    snprintf(pout,1255,"set z 1 %i",pfi->dnum[2]);
	  }
	  gacmd (pout,pcm,0);
	} else {
	  gacmd ("set z 1",pcm,0);
	}
      }
      gacmd ("set t 1",pcm,0);
      gacmd ("set e 1",pcm,0);
    }
    /* reset graphics settings */
    if (reinit) {
      gxdbck(0);             /* device backgroud reset to black only on reinit, not reset */
    }
    dsubs->gxdsignal(3);     /* tell hardware to re-enable anti-aliasing */
    gxdbsethersh(0);         /* set hershey font on   */
    gxchdf(0);               /* set default font to 0 */ 
    gxcolr(1);               /* set color to foreground */    
    gxdbsettransclr(-1);     /* unset any transparent color override */
    gxfrme (1);
    if (reinit)
      gaprnt (1,"All GrADS attributes have been reinitialized\n");
    else gaprnt (1,"Most GrADS attributes have been reset\n");
    goto retrn;
  }

  else if (cmpwrd("screen",cmd)) {
    if ((cmd=nxtwrd(cmd)) == NULL) {
      gaprnt (0,"Screen Error: Missing keyword\n");
      retcod = 1;
      goto retrn;
    }
    i = 0;
    if (cmpwrd("save",cmd)) i = 1;
    if (cmpwrd("show",cmd)) i = 2;
    if (cmpwrd("free",cmd)) i = 3;
    if (i) {
      if ((cmd=nxtwrd(cmd)) == NULL) {
        gaprnt (0,"Screen Error: Missing screen number\n");
        retcod = 1;
        goto retrn;
      }
      if (intprs(cmd,&(fnum)) == NULL ) {
        gaprnt (0,"Screen Error: Invalid screen number\n");
        retcod = 1;
        goto retrn;
      }
      if (i==1) dsubs->gxdssv(fnum);
      if (i==2) dsubs->gxdssh(fnum);
      if (i==3) dsubs->gxdsfr(fnum);
      gxfrme(9);
    } else {
      gaprnt (0,"Screen Error: Unknown keyword\n");
      retcod = 1;
      goto retrn;
    }
    goto retrn;
  }
  else if (cmpwrd("close",cmd)) {
    if ((cmd=nxtwrd(cmd)) == NULL) {
      gaprnt (0,"Close Error: Missing file number\n");
      retcod = 1;
      goto retrn;
    }
    if (intprs(cmd,&(fnum)) == NULL ) {
      gaprnt (0,"Close Error: Invalid file number\n");
      retcod = 1;
      goto retrn;
    }
    if (fnum != pcm->fnum) {
      gaprnt (0,"Close Error: Only last file may be closed\n");
      retcod = 1;
      goto retrn;
    }
    /* advance to end of chain of open file structures */
    pfi = pcm->pfi1;
    for (i=0; i<fnum-1 && pfi; i++) pfi = pfi->pforw;
    if (pfi==NULL || pfi->pforw!=NULL) {
      gaprnt (0,"Logic Error 4 on Close.  No Action Taken\n");
      retcod = 1;
      goto retrn;
    }
    /* close any data and map files that are open */
    if (pfi->infile) fclose (pfi->infile);
    if (pfi->mfile) fclose(pfi->mfile);
    if (pfi->ncflg==1) gaclosenc(pfi);
    if (pfi->ncflg==2) gaclosehdf(pfi);
    if (pfi->ncflg==3) gacloseh5(pfi);
    if (pfi->bufrdset) {            /* bufr station data */
      gabufr_close(pfi->bufrdset);  /* free memory */
      pfi->bufrdset=NULL;           /* reset the pointer */
    }
#if USEGADAP
    if (pfi->dhandle > -999) dapclo(pfi);  /* opendap station data */
#endif
    frepfi(pfi,0);
    pcm->fnum--;                           /* decrease number of open files */
    if (pcm->dfnum==fnum) {                /* if closed file was default ... */
      pcm->dfnum = 1;                      /* ...reset default file number to 1  */
      pcm->pfid = pcm->pfi1;               /* ...reset pointer to default gafile */
    }
    if (pcm->fnum==0) {                    /* no files open, so ... */
      pcm->dfnum = 0;                      /*    set default file to zero */
      pcm->pfi1 = NULL;                    /*    pointer to file chain is null */
      pcm->pfid = NULL;                    /*    pointer to default file is null */
    } else {
      pfi = pcm->pfi1;
      for (i=0; i<fnum-2 && pfi; i++) pfi = pfi->pforw;  /* move to end of chain */
      pfi->pforw = NULL;                                 /* set last link to null */
    }
    snprintf(pout,1255,"File %i has been closed\n",fnum);
    gaprnt (2,pout);
    goto retrn;
  }
  else if (cmpwrd("clear",cmd) || cmpwrd("c",cmd)) {
    rc = 0;
    if ((cmd=nxtwrd(cmd)) != NULL) {
      rc=99;
      if (cmpwrd("norset",cmd))   rc = 1;  /* clears without resetting user options */
      if (cmpwrd("events",cmd))   rc = 2;  /* flushes the Xevents buffer */
      if (cmpwrd("graphics",cmd)) rc = 3;  /* clears the graphics but not the widgets */
      if (cmpwrd("hbuff",cmd))    rc = 4;  /* clears the buffered display and turns off double buffer mode */
      if (cmpwrd("button",cmd))   rc = 5;  /* clears a button */
      if (cmpwrd("rband",cmd))    rc = 6;  /* clears a rubber band */
      if (cmpwrd("dropmenu",cmd)) rc = 7;  /* clears a dropmenu */
      if (cmpwrd("sdfwrite",cmd)) rc = 8;  /* clears sdfwrite filename and attributes only */
      if (cmpwrd("mask",cmd))     rc = 9;  /* clears contour label mask */
      if (cmpwrd("shp",cmd))      rc = 10; /* clears shapefile output filename and attributs only */
    }
    if (rc==99) {
      gaprnt (0,"Invalid option on clear command\n");
      goto retrn;
    }
    /* clear the frame and X-related other options */
    if (rc<2) {
      if (exflg) gxfrme (0);
      else gxfrme (1);
    }
    else if (rc==2) gxfrme(8);
    else if (rc==3) gxfrme(7);
    else if (rc==4) gxhfrm(0);
    else if (rc>4 && rc<8) {
      if ((cmd=nxtwrd(cmd)) == NULL) {
        gaprnt (0,"Invalid or missing widget number on clear command\n");
        goto retrn;
      }
      if (intprs(cmd,&(fnum)) == NULL ) {
        gaprnt (0,"Invalid or missing widget number on clear command\n");
        goto retrn;
      }
      dsubs->gxrs1wd (rc-4, fnum);
    }
    else if (rc==9) gxmaskclear();
    /* call gacln (reset gacmn) with different options */
    if (pcm->aaflg==0) dsubs->gxdsignal(3); /* tell hardware to re-enable anti-aliasing */
    if (rc==1) gacln(pcm,0);
    else if (rc<5 || rc==99) {
      gacln (pcm,1);
      pcm->dbflg = 0;
    }
    else if (rc==8) gacln(pcm,2);  /* clears sdfwrite file name and attributes */
    else if (rc==10) gacln(pcm,3);  /* clears user-provided shapefile attributes */
    goto retrn;
  }

  else if (cmpwrd("swap",cmd)) {
    if (pcm->dbflg) gxfrme(2);
    gacln(pcm,1);
    goto retrn;
  } 

  else if (cmpwrd("outxwd", cmd)) { 
    if (pcm->batflg) {
      gaprnt(0,"The outxwd command does not work in batch mode\n");
      retcod = 1;
      goto retrn;
    }
    char *fname, name_file_xwd[256] ; 
    fname = nxtwrd(com) ; 
    if (fname) { 
      if (sscanf(fname, "%s", name_file_xwd) == 1) { 
	if (pcm->dbflg) { 
	  dsubs->gxdbb(name_file_xwd) ; 
	} else { 
	  dsubs->gxdfb(name_file_xwd) ; 
	} 
      } 
    } else { 
      gaprnt(0,"command outxwd: missing output filename\n") ; 
    } 
    gacln(pcm,1) ; 
    goto retrn;
  } 

  else if (cmpwrd("q",cmd)||cmpwrd("query",cmd)) {
    retcod = gaqury (cmd, com, pcm);
    goto retrn;
  }
  else if (cmpwrd("help",cmd)) {
    retcod = gahelp (cmd, pcm);
    goto retrn;
  }
  else if (cmpwrd("exec",cmd)) {
    retcod = gaexec (com, pcm);
    goto retrn;
  }
  else if (cmpwrd("run",cmd)) {
    if ((cmd=nxtwrd(com)) == NULL) {
      gaprnt (0,"RUN error:  No file name specified\n");
      retcod = 1;
      goto retrn;
    }
    savpcm = pcm;
    rslt = gsfile(cmd,&rc,0);
    if (rc==0 && rslt!=NULL) gaprnt(2,rslt);
    if (rslt!=NULL) gree(rslt,"f193");
    retcod = rc;
    goto retrn;
  }
  else if (cmpwrd("enable",cmd)) {
    retcod = gaenab (com, pcm);
    goto retrn;
  }
  else if (cmpwrd("disable",cmd)) {
    if ((cmd=nxtwrd(com)) == NULL) {
      gaprnt(0,"DISABLE error: Missing keyword\n");
      goto retrn;
    }
    else {
      if (cmpwrd("print",cmd)) 
	gaprnt(0,"ERROR: The \"disable print\" command is no longer valid. Use \"gxprint\" instead.\n");
      else if (cmpwrd("fwrite",cmd)) {
	if (pcm->ffile) {                           /* don't close a file unless it's open  */ 
	  if (pcm->ffile!=stdout) fclose(pcm->ffile);    /* don't close stdout */    
	}
        pcm->ffile = NULL;
        if (pcm->fwname) gree(pcm->fwname,"f194");  /* reset fwrite file name on disable. */
        pcm->fwname = NULL;                         
        pcm->fwenflg = BYTEORDER;                   /* set fwrite to default state */
        pcm->fwsqflg = 0;                           /* default is stream */
        pcm->fwexflg = 0;                           /* default is not exact -- old bad way */
      }
      else if (cmpwrd("stnwrt",cmd)) {
        if (pcm->sfile) fclose(pcm->sfile);
        pcm->sfile = NULL;
      }
      else gaprnt (0,"DISABLE error: Invalid keyword\n");
    }
    goto retrn;
  }
  else if (cmpwrd("redraw",cmd)) {
    gardrw (com,pcm);
    gxfrme (9);
    goto retrn;
  }
  else if (cmpwrd("draw",cmd)) {
    gadraw (com,pcm);
    gxsignal (1);        /* tell rendering layer and metabuffer draw is done */
    gxfrme (9);          /* flush X request buffer */
    goto retrn;
  }
  else if (cmpwrd("print",cmd)) {
    gaprnt(0,"ERROR: The \"print\" command is no longer valid. Use \"gxprint\" instead.\n");
    goto retrn;
  }
  else if (cmpwrd("gxprint",cmd) || cmpwrd("printim",cmd)) {
    /* check for output file name */
    if ((ch=nxtwrd(com)) == NULL) {   
      gaprnt (0,"GXPRINT error:  missing output file name\n");
      retcod = 1;
      goto retrn;
    }
    getwrd (cc,ch,256);                   /* copy output file name into cc */

    /* advance past the output file name in mixed and lower case versions of the command */
    com=nxtwrd(com);
    cmd=nxtwrd(cmd);

    /* initialize */
    xin = -999;
    yin = -999;
    bwin = -999;
    fmtflg = 0;
    bgImage[0] = '\0';
    fgImage[0] = '\0';
    tcolor = -1;
    border = -1.0;

    /* parse the user-provided options */
    while ( (cmd=nxtwrd(cmd))!=NULL && (com=nxtwrd(com))!=NULL) {
      /* set backgroud/foreground colors */
      if      (cmpwrd("black",cmd))  bwin = 0;
      else if (cmpwrd("white",cmd))  bwin = 1;
      /* explicit format types -- the union of all possibilities from all rendering engines */
      else if (cmpwrd("eps",cmd))  fmtflg = 1;
      else if (cmpwrd("ps" ,cmd))  fmtflg = 2;
      else if (cmpwrd("pdf",cmd))  fmtflg = 3;
      else if (cmpwrd("svg",cmd))  fmtflg = 4;
      /* ... image formats start here ... */
      else if (cmpwrd("png",cmd))  fmtflg = 5;
      else if (cmpwrd("gif",cmd))  fmtflg = 6;
      else if (cmpwrd("jpg",cmd))  fmtflg = 7;
      else if (cmpwrd("jpeg",cmd)) fmtflg = 7;
      /* get background image filename */
      else if (cmpwrd("-b",cmd)) {
        com=nxtwrd(com);
        if((cmd=nxtwrd(cmd)) != NULL) {
	  /* use mixed case version for bgImage */
          if (strlen(com) < 256){
            getwrd(bgImage,com,255);
          }
        } else {
          gaprnt(1,"GXPRINT warning: Background image file name not provided\n");
	  if (cmd == NULL) break;
	}
      }
      /* get foreground image filename */
      else if (cmpwrd("-f",cmd)) {
        com=nxtwrd(com);
        if((cmd=nxtwrd(cmd)) != NULL) {
	  /* use mixed case version for fgImage */
          if (strlen(com) < 256){
            getwrd(fgImage,com, 255);
          }
	} else {
          gaprnt(1,"GXPRINT warning: Foreground image file name not provided\n");
	  if (cmd == NULL) break;
	}
      }
      /* set transparent color number */
      else if (cmpwrd("-t",cmd)) {
	/* set transparent color number */
        com=nxtwrd(com);
        if((cmd=nxtwrd(cmd)) != NULL) {
	  if(sscanf(cmd, "%i", &tcolor) != 1) {    /* get a color number  */
	     gaprnt(1,"GXPRINT warning: Invalid transparent color number\n");
	  }
	} else {
	  gaprnt(1,"GXPRINT warning: Missing transparent color number\n");
	  if (cmd == NULL) break;
	}
      }
      /* set horizontal image size */
      else if (*cmd=='x') {
        if (intprs(cmd+1,&(xin)) == NULL ) {
          gaprnt (0,"GXPRINT error:  Invalid x option; ignored\n");
          xin = -999;
        }
      }
      /* set vertical image size */
      else if (*cmd=='y') {
        if (intprs(cmd+1,&(yin)) == NULL ) {
          gaprnt (0,"GXPRINT error: Invalid y option; ignored\n");
          yin = -999;
        }
      }
      /* set width of border around the edge of the plot */
      else if (cmpwrd("-e",cmd)) {
        com=nxtwrd(com);
        if((cmd=nxtwrd(cmd)) != NULL) {
	  if (getdbl(cmd,&border) == NULL) {   
	     gaprnt(1,"GXPRINT warning: Invalid edge width \n");
	  }
	} else {
	  gaprnt(1,"GXPRINT warning: Missing edge width \n");
	  if (cmd == NULL) break;
	}
      }
      else {
        gaprnt (0,"GXPRINT error: Invalid option; ignored\n");
      }
    }    
    /* done parsing options */

    /* try to divine the format from the output filename extension */
    if (fmtflg == 0) {
      len = 0;
      while (*(cc+len)) len++;
      pos = len;
      for (pos=len; pos>=0; pos--) if (*(cc+pos)=='.') break;
      len = len-pos;
      if (pos>0) {
	for (i=0;i<len;i++) ext[i] = *(cc+pos+i); 
	lowcas (ext);
	if (!strncmp(ext, ".eps",4)) fmtflg = 1;
	if (!strncmp(ext, ".ps" ,3)) fmtflg = 2;
	if (!strncmp(ext, ".pdf",4)) fmtflg = 3;
	if (!strncmp(ext, ".svg",4)) fmtflg = 4;
	if (!strncmp(ext, ".png",4)) fmtflg = 5;
	if (!strncmp(ext, ".gif",4)) fmtflg = 6;
	if (!strncmp(ext, ".jpg",4)) fmtflg = 7;
      }
    }
    if (fmtflg == 0) {
      gaprnt (0,"GXPRINT error: Unable to determine output format\n");
      retcod = 1;
      goto retrn;
    }
    
    /* Image size specs are incompatible with non-image formats */
    if (fmtflg==1 || fmtflg==2 || fmtflg==3 || fmtflg==4) {
      if (xin>0 || yin>0) {
	snprintf(pout,1255,"GXPRINT Warning: Image size specifications are incompatible with the %s format and will be ignored\n",
		 formats[fmtflg-1]);
	gaprnt (1,pout);
	xin = -999;
	yin = -999;
      }
    }

    /* fgImage and bgImage options are incompatible with non-image formats */
    if (fmtflg==1 || fmtflg==2 || fmtflg==3 || fmtflg==4) {
      if (*bgImage || *fgImage) {
	snprintf(pout,1255,"GXPRINT Warning: Background/Foreground images are incompatible with the %s format and will be ignored\n",
		 formats[fmtflg-1]);
	gaprnt (1,pout);
	bgImage[0]='\0';
	fgImage[0]='\0';
      }
    }

    /* vector formats ps, eps, and pdf get a 5/32" edge by default, all others have no edge */
    if (border < 1.0) { 
      border = 0.0;  
      if (fmtflg==1 || fmtflg==2 || fmtflg==3) border = 0.15625;
    }

    /* Create the hardcopy output */
    rc = psubs->gxprint (cc,xin,yin,bwin,fmtflg,bgImage,fgImage,tcolor,border);
    if (rc==1)  gaprnt (0,"GXPRINT error: something went wrong during rendering of the output file\n");
    if (rc==2)  gaprnt (0,"GXPRINT error: output error\n");
    if (rc==3)  gaprnt (0,"GXPRINT error: background image open error\n");
    if (rc==4)  gaprnt (0,"GXPRINT error: foreground image open error\n");
    if (rc==5)  gaprnt (0,"GXPRINT error: background image must be .png\n");
    if (rc==6)  gaprnt (0,"GXPRINT error: foreground image must be .png\n");
    if (rc==7)  gaprnt (0,"GXPRINT error: failed to import background image\n");
    if (rc==8)  gaprnt (0,"GXPRINT error: failed to import foreground image\n");
    if (rc==9)  gaprnt (0,"GXPRINT error: unsupported output format\n");
    if (rc==10) gaprnt (0,"GXPRINT error: background image must be the same size as output\n");
    if (rc==11) gaprnt (0,"GXPRINT error: foreground image must be the same size as output\n");
    if (rc) retcod = 1;
    else {
      retcod = 0;
      snprintf(pout,1255,"Created %s file %s",formats[fmtflg-1], cc); gaprnt(2,pout); 
      if (*bgImage) { snprintf(pout,1255," ; bgImage = %s",bgImage); gaprnt(2,pout); }
      if (*fgImage) { snprintf(pout,1255," ; fgImage = %s",fgImage); gaprnt(2,pout); }
      gaprnt(2,"\n");
    }
    goto retrn;
  }
  else if (cmpwrd("set",cmd)) {
    retcod = gaset (cmd, com, pcm);
    goto retrn;
  }
  else if (cmpwrd("open",cmd)) {
    if ((cmd=nxtwrd(com)) == NULL) {
      gaprnt (0,"OPEN error:  missing data description file name\n");
      retcod = 1;
      goto retrn;
    }
    getwrd (cc,cmd,256);
    retcod = gaopen (cc, pcm);
    if (!retcod) mygreta(cc);   /* (for IGES only) keep track of user's opened files */

    goto retrn;
#if (USENETCDF==1 || USEHDF ==1)
  } else if (cmpwrd("sdfopen", cmd)) {
    if ((cmd = nxtwrd(com)) == NULL) {
      gaprnt(0, "SDFOPEN error:  missing self-describing file pathname\n") ;
      retcod = 1;
      goto retrn;
    }
    retcod = gasdfopen(cmd, pcm) ;
    if (!retcod) mygreta(cmd);  /* (for IGES only) keep track of user's opened files */
    goto retrn;
  }
  else if (cmpwrd("xdfopen", cmd)) {
    if ((cmd = nxtwrd(com)) == NULL) {
        gaprnt(0, "XDFOPEN error:  missing data descriptor file name\n") ;
        retcod = 1 ;
        goto retrn ;
    }
    retcod = gaxdfopen(cmd, pcm) ;
    if (!retcod) mygreta(cmd);  /* (for IGES only) keep track of user's opened files */
    goto retrn ;
#endif
  } else if (cmpwrd("d",cmd) || cmpwrd("display",cmd)) {
    if (pcm->pfid==NULL) {
      gaprnt (0,"DISPLAY error:  no file open yet\n");
      retcod = 1;
      goto retrn;
    }
    retcod = gadspl (cmd, pcm);
    gxsignal (1);        /* tell rendering engine and metafile buffer display is finished */
    gxfrme (9);          /* flush any buffers as needed */
    goto retrn;
  }
  else if (cmpwrd("coll",cmd) || cmpwrd("collect",cmd)) {
    if (pcm->pfid==NULL) {
      gaprnt (0,"COLLECT error:  no file open yet\n");
      retcod = 1;
      goto retrn;
    }
    retcod = gacoll(cmd, pcm);
    goto retrn;
  }
  else if (cmpwrd("define",cmd)) {
    if (pcm->pfid==NULL) {
      gaprnt (0,"DEFINE error:  no file open yet\n");
      retcod = 1;
      goto retrn;
    }
    retcod = gadef (cmd, pcm, 0);
    goto retrn;
  }
  else if (cmpwrd("undefine",cmd)) {
    if (pcm->pfid==NULL) {
      gaprnt (0,"DEFINE error: no file open yet\n");
      retcod = 1;
      goto retrn;
    }
    retcod = gaudef (cmd, pcm);
    goto retrn;
  }
  else if (cmpwrd("modify",cmd)) {
    if (pcm->pfid==NULL) {
      gaprnt (0,"MODIFY error: no file open yet\n");
      retcod = 1;
      goto retrn;
    }
    retcod = gamodf (cmd, pcm);
    goto retrn;
  }
  else if (cmpwrd("sdfwrite",cmd)) {
    if (pcm->pdf1==NULL) {
      gaprnt (0,"SDFWRITE error: no defined variables\n");
      retcod = 1;
      goto retrn;
    } 
    retcod = ncwrite(cmd, pcm);
    goto retrn;
  }
  else if (cmpwrd("pdefwrite",cmd)) {
    if (pcm->pfid==NULL) {
      gaprnt (0,"PDEFWRITE error: no file open yet\n"); retcod=1; goto retrn;}
    if (pcm->pfid->ppflag==0) {
      gaprnt (0,"PDEFWRITE error: default file does not use PDEF\n"); retcod=1; goto retrn;} 
    if (pcm->pfid->ppflag==7 || pcm->pfid->ppflag==8) {
      gaprnt (0,"PDEFWRITE error: default file already has an external PDEF file\n"); retcod=1; goto retrn;} 

    /* Get the name of the output file and open it */
    if ((cmd=nxtwrd(cmd)) != NULL) {
      if (strlen(cmd) < 256) {
        getwrd(pdefname,cmd, 255);
      } else {
        gaprnt (0,"PDEFWRITE error: name of output file is missing \n"); retcod=1; goto retrn;}
    }
    if ((pdefid = fopen (pdefname, "w"))==NULL) { 
      gaprnt(0,"PDEFWRITE error: failed to open output file \n"); retcod=1; goto retrn;}

    /* allocate memory for grids to be written out to file */
    size = pcm->pfid->dnum[0] * pcm->pfid->dnum[1];
    if ((ivals = (gafloat*)galloc(size*sizeof(gafloat),"ppivals"))==NULL) {retcod=1; goto retrn;}
    if ((jvals = (gafloat*)galloc(size*sizeof(gafloat),"ppjvals"))==NULL) {retcod=1; goto retrn;}
    if ((rvals = (gafloat*)galloc(size*sizeof(gafloat),"pprvals"))==NULL) {retcod=1; goto retrn;}
    ival = ivals;  /* save these initial pointers for the fwrite commands */
    jval = jvals;

    /* Fill grids of file offsets and weights (dx,dy) calculated for pdef grid interpolation */
    ioff = pcm->pfid->ppi[0]; 
    dx = pcm->pfid->ppf[0];
    dy = pcm->pfid->ppf[1];
    for (j=0; j<pcm->pfid->dnum[1]; j++) {
      for (i=0; i<pcm->pfid->dnum[0]; i++) {
        if (*ioff == -1) {
          *ivals = -999;
	  *jvals = -999;
        } else {
          rem = *ioff % (pcm->pfid->ppisiz);  /* 0-based x index */
          *ivals = rem + 1 + (gafloat)*dx ;   /* make x index 1-based and add the interp wgt */
 	  *jvals = ((*ioff-rem)/pcm->pfid->ppisiz) + 1 + (gafloat)*dy; /* 1-based y index plus interp wgt */
	}
        ioff++; dx++; dy++; ivals++; jvals++;
      }
    }
    rc = fwrite(ival, sizeof(gafloat), size, pdefid); if (rc!=size) {retcod=1; goto retrn;}
    rc = fwrite(jval, sizeof(gafloat), size, pdefid); if (rc!=size) {retcod=1; goto retrn;}
    /* Fill the grid of rotation values, then write it out */
    if (pcm->pfid->ppwrot)
      for (i=0; i<size; i++) *(rvals+i) = (gafloat)(*(pcm->pfid->ppw+i));
    else
      for (i=0; i<size; i++) *(rvals+i) = -999;
    rc = fwrite(rvals, sizeof(gafloat), size, pdefid); if (rc!=size) {retcod=1; goto retrn;}

    fclose(pdefid);
    snprintf(pout,1255,"pdef %d %d bilin stream binary-",pcm->pfid->ppisiz,pcm->pfid->ppjsiz);
    gaprnt(2,pout);
#if BYTEORDER==1
    snprintf(pout,1255,"big");
#else
    snprintf(pout,1255,"little");
#endif
    gaprnt(2,pout);
    snprintf(pout,1255," ^%s\n",pdefname);
    gaprnt(2,pout);

    if (ival!=NULL) gree(ival,"f194a");
    if (jval!=NULL) gree(jval,"f194a");
    if (rvals!=NULL) gree(rvals,"f194a");

    retcod = 0;
    goto retrn;
  }
  else {
    if (pcm->impcmd) {
      savpcm = pcm;
      rslt = gsfile(com,&rc,1);
      if (rc==0 && rslt!=NULL) gaprnt(2,rslt);
      if (rslt!=NULL) gree(rslt,"f195");
      retcod = rc;
      goto retrn;
    }
    gaprnt (0,"Unknown command: ");
    gaprnt (0,cmd);
    gaprnt (0,"\n");
    retcod = 1;
    goto retrn;
  }

retrn:
  if (ccc) {
    gree(ccc,"f196");
  }
  return (retcod);
}

void mygreta(char *path) {
/* char mypath[1500],mymsg[1551],date[20]; */
/*  if (strncmp(path,"http://",7)) { */
/*   realpath(path,mypath); */
/*   strcpy(date,"`date +%Y%m%d`"); */
/*   snprintf(mymsg,1550,"echo %s %s >> $HOME/.mygreta",date,mypath); */
/*   system(mymsg); */
/*  } */
}



/* if flag is 0, clean without resetting user options. 
   if flag is 1, clean and reset user options except sdfwrite & shapefile parameters
   if flag is 2, clean and reset only sdfwrite parameters 
   if flag is 3, clean and reset only shapefile fields
   if flag is 4, clean and reset only KML and GeoTIFF options

   The clear command calls gacln with flag=1.
   The reset/reinit commands calls gacln four times, with flag=1 and flag=2, flag=3, and flag=4.
*/

void gacln (struct gacmn *pcm, gaint flg) {
struct gaattr *attr,*nextattr;
gaint i;
#if USESHP==1
struct dbfld *fld,*nextfld;
#endif

  /* The 'basic' clean */
  pcm->pass = 0;
  for (i=0; i<10; i++) pcm->gpass[i] = 0;
  if (!pcm->ylpflg) pcm->yllow = 0.0;
  pcm->xexflg = 0; 
  pcm->yexflg = 0;
  pcm->shdcnt = 0;
  pcm->cntrcnt = 0;
  pcm->lastgx = 0;
  pcm->xdim = -1;
  pcm->ydim = -1;
  pcm->xgr2ab = NULL;
  pcm->ygr2ab = NULL;
  pcm->xab2gr = NULL;
  pcm->yab2gr = NULL;
  pcm->aaflg = 1;      

  /* reset sdfwrite parameters */
  if (flg==2) {
    pcm->ncwid = -999;
    pcm->sdfwtype = 1;
    pcm->sdfwpad = 0;
    pcm->sdfrecdim = 0;
    pcm->sdfchunk = 0;
    pcm->xchunk = 0;
    pcm->ychunk = 0;
    pcm->zchunk = 0;
    pcm->tchunk = 0;
    pcm->echunk = 0;
    pcm->sdfzip = 0;
    pcm->sdfprec = 8;
    if (pcm->sdfwname) {
      gree(pcm->sdfwname,"g225");
      pcm->sdfwname = NULL;
    }
    while (pcm->attr != NULL) {
      attr = pcm->attr;           /* point to first block in chain */
      if (attr->next == NULL) {
	pcm->attr = NULL;         /* first block is only block */
      }
      else {        /* move start of chain from 1st to 2nd block */
	nextattr = attr->next;
	pcm->attr = nextattr;
      }
      if (attr->value) gree(attr->value,"g85");  /* release memory from 1st block */
      gree(attr,"g86");
    }
  } 

  /* reset shapefile fields */ 
  if (flg==3) {
#if USESHP==1    
    /* reset shapefile type to line and no fill */
    pcm->shptype=2; 
    pcm->fillpoly = -1;
    /* release file name */
    if (pcm->shpfname) {
      gree (pcm->shpfname,"g89");
      pcm->shpfname = NULL;
    }
    /* release chain of data base fields */
    while (pcm->dbfld != NULL) {
      /* point to first block in chain */
      fld = pcm->dbfld;  
      if (fld->next == NULL) {
	/* first block is only block */
	pcm->dbfld = NULL; 
      } 
      else { 
	/* move start of chain from 1st to 2nd block */
	nextfld = fld->next;
	pcm->dbfld = nextfld;
      }
      /* release memory from 1st block */
      if (fld->value != NULL) gree(fld->value,"g87");
      gree(fld,"g88");
    }
#endif
  }

  /* reset KML and GeoTIFF options */
  if (flg==4) {
    pcm->gtifflg=1;               /* reset GeoTIFF output to float */
    pcm->kmlflg=1;                /* reset KML output to img */
    if (pcm->kmlname) {           /* release KML file name */
      gree (pcm->kmlname,"g90");
      pcm->kmlname = NULL;
    }
    if (pcm->tifname) {           /* release TIFF file name */
      gree (pcm->tifname,"g91");
      pcm->tifname = NULL;
    }
    if (pcm->gtifname) {          /* release GeoTIFF file name */
      gree (pcm->gtifname,"g92");
      pcm->gtifname = NULL;
    }
  }

  /* reset user options */
  if (flg==1) {
    pcm->cstyle = -9;
    pcm->ccolor = -9;
    pcm->cthick = 4;
    pcm->cmark = -9;
    pcm->cint = 0;
    pcm->cflag = 0;
    pcm->ccflg = 0;
    pcm->cmin = -9.99e33;
    pcm->cmax = 9.99e33;
    pcm->blkflg = 0;
    pcm->aflag = 0;
    pcm->aflag2 = 0;
    pcm->axflg = 0;
    pcm->ayflg = 0;
    pcm->gridln = -9;
    pcm->rainmn = pcm->rainmx = 0.0;
    pcm->grdsflg = 1;
    pcm->arrflg = 0;
    pcm->hemflg = -1;
    pcm->rotate = 0;
    pcm->xflip = 0;
    pcm->yflip = 0;
    if (pcm->xlstr) gree(pcm->xlstr,"f197");
    if (pcm->ylstr) gree(pcm->ylstr,"f198");
    if (pcm->clstr) gree(pcm->clstr,"f199");
    if (pcm->xlabs) gree(pcm->xlabs,"f200");
    if (pcm->ylabs) gree(pcm->ylabs,"f201");
    pcm->xlstr = NULL;
    pcm->ylstr = NULL;
    pcm->clstr = NULL;
    pcm->xlabs = NULL;
    pcm->ylabs = NULL;
    pcm->xlint = 0.0;
    pcm->ylint = 0.0;
    pcm->xlflg = 0;
    pcm->ylflg = 0;
    pcm->xlpos = 0.0;
    pcm->ylpos = 0.0;
    pcm->ylpflg = 0;
    pcm->yllow = 0.0;
    pcm->xlside = 0;
    pcm->ylside = 0;
    pcm->tlsupp = 0;
    pcm->ptflg = 0;
    pcm->cachesf = 1;  setcachesf(pcm->cachesf);
    pcm->marktype = 3;
    pcm->marksize = 0.05;
  }
}

/* Handle redraw command */

gaint gardrw (char *cmd, struct gacmn *pcm) {
struct gbtn btn;
char oper[12];
gaint num,state,i,sflg;
size_t sz;

  if ((cmd=nxtwrd(cmd)) == NULL) {
    gaprnt (0,"REDRAW error: Missing operand\n");
    return (1);
  }
  getwrd (oper,cmd,10);
  lowcas (oper);

  if (cmpwrd("button",oper)) {
    if ((cmd = nxtwrd (cmd)) == NULL) goto errrbn;
    if (intprs(cmd,&(num)) == NULL ) goto errrbn;
    if (num<0 || num>255) goto errrbn;
    state = -1;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errrbn;
    if (intprs(cmd,&(state)) == NULL ) goto errrbn;
    btn.bc = pcm->btnbc;
    btn.fc = pcm->btnfc;
    btn.oc1 = pcm->btnoc;
    btn.oc2 = pcm->btnoc2;
    btn.btc = pcm->btnbtc;
    btn.ftc = pcm->btnftc;
    btn.otc1 = pcm->btnotc;
    btn.otc2 = pcm->btnotc2;
    btn.thk = pcm->btnthk;
    btn.state = state;

    sflg = 2;
    if ((cmd = nxtwrd (cmd)) == NULL) {
      btn.len = 0;
      btn.ch = NULL;
    } else {
      if (intprs(cmd,&(i)) == NULL ) goto errrbn;
      if (i) sflg = 3;
      if ((cmd = nxtwrd (cmd)) == NULL) {
        btn.len = 0;
        btn.ch = NULL;
      } else {
        btn.len = 0;
        while (*(cmd+btn.len)) btn.len++;
	sz = btn.len+1;
        btn.ch = (char *)galloc(sz,"btn1");
        *(btn.ch+btn.len) = '\0';
        if (btn.ch==NULL) {
          gaprnt(0,"Memory allocation error; DRAW BUTTON cmd\n");
          return(1);
        }
        for (i=0; i<btn.len; i++) *(btn.ch+i) = *(cmd+i);
      }
    }
    dsubs->gxdpbn(num, &btn, sflg, 0, state);
  } else {
    gaprnt (0,"REDRAW error:  Invalid operand\n");
    return(1);
  }
  return (0);

  errrbn:
  gaprnt (0,"REDRAW error: Syntax is REDRAW BUTTON number state flag text\n");
  return (1);
}


/* handle draw command */

static gadouble justx[9] = {0.0,0.5,1.0,0.0,0.5,1.0,0.0,0.5,1.0};
static gadouble justy[9] = {0.0,0.0,0.0,0.5,0.5,0.5,1.0,1.0,1.0};

gaint gadraw (char *cmd, struct gacmn *pcm) {
struct gbtn btn;
struct gdmu dmu;
gadouble *xy, *newxy, llinc;
gadouble x,y,xlo,xhi,ylo,yhi,r,cs,swide,shite,ang;
gaint i,cnt,newcnt,cflg,ipos,len,mk,wx,thk,col;
char oper[12],chars[250];
char *c1,*c2,*ccmd;
size_t sz;
#if USESHP==1
SHPHandle shpid=NULL;
SHPObject *shp=NULL;
gadouble *pxy,newx,newy,lnfact;
int shape,begshp,endshp=0,shpcnt,shptype,j,v,begv,endv,p,haveinfo,numparts,ival;
char shparg[4096];
#endif

  /* Check initial operands */
  if ((cmd=nxtwrd(cmd)) == NULL) {
    gaprnt (0,"DRAW error: Missing operand\n");
    return (1);
  }
  getwrd (oper,cmd,10);
  lowcas (oper);

  if (cmpwrd("map",oper)) {
    if (pcm->dmax[0]>pcm->dmin[0] && pcm->dmax[1]>pcm->dmin[1]) {
      gamscl(pcm);
      gawmap(pcm,0);
    }
    return (0);
  }
  if (cmpwrd("button",oper)) {
    if ((cmd = nxtwrd (cmd)) == NULL) goto errbn;
    if (intprs(cmd,&(cnt))   == NULL) goto errbn;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errbn;
    if (getdbl(cmd,&(btn.x)) == NULL) goto errbn;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errbn;
    if (getdbl(cmd,&(btn.y)) == NULL) goto errbn;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errbn;
    if (getdbl(cmd,&(btn.w)) == NULL) goto errbn;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errbn;
    if (getdbl(cmd,&(btn.h)) == NULL) goto errbn;
    btn.bc = pcm->btnbc;
    btn.fc = pcm->btnfc;
    btn.oc1 = pcm->btnoc;
    btn.oc2 = pcm->btnoc2;
    btn.btc = pcm->btnbtc;
    btn.ftc = pcm->btnftc;
    btn.otc1 = pcm->btnotc;
    btn.otc2 = pcm->btnotc2;
    btn.thk = pcm->btnthk;
    btn.state = 0;

    if ((cmd = nxtwrd (cmd)) == NULL) goto errbn;
    btn.len = 0;
    while (*(cmd+btn.len)) btn.len++;
    sz = btn.len+1;
    btn.ch = (char *)galloc(sz,"btn2");
    *(btn.ch+btn.len) = '\0';
    if (btn.ch==NULL) {
      gaprnt(0,"Memory allocation error; DRAW BUTTON cmd\n");
      return(1);
    }
    for (i=0; i<btn.len; i++) *(btn.ch+i) = *(cmd+i);
    dsubs->gxdpbn(cnt, &btn, 0, 0, -1);
    return (0);

    errbn:
    gaprnt (0,"DRAW error: Syntax is DRAW BUTTON number x y w h ");
    gaprnt (0,"string\n");
    return (1);
  }

  if (cmpwrd("dropmenu",oper)) {
    if ((cmd = nxtwrd (cmd)) == NULL) goto errpm;
    if (intprs(cmd,&(cnt)) == NULL ) goto errpm;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errpm;
    if (cmpwrd("cascade", cmd)) {
      dmu.casc = 1;
    } else {
      dmu.casc = 0;
      if (getdbl(cmd,&(dmu.x)) == NULL ) goto errpm;
      if ((cmd = nxtwrd (cmd)) == NULL) goto errpm;
      if (getdbl(cmd,&(dmu.y)) == NULL ) goto errpm;
      if ((cmd = nxtwrd (cmd)) == NULL) goto errpm;
      if (getdbl(cmd,&(dmu.w)) == NULL ) goto errpm;
      if ((cmd = nxtwrd (cmd)) == NULL) goto errpm;
      if (getdbl(cmd,&(dmu.h)) == NULL ) goto errpm;
    }
    dmu.fc = pcm->drvals[0]; dmu.bc = pcm->drvals[1];
    dmu.oc1 = pcm->drvals[2]; dmu.oc2 = pcm->drvals[3];
    dmu.tfc = pcm->drvals[4]; dmu.tbc = pcm->drvals[5];
    dmu.toc1 = pcm->drvals[6]; dmu.toc2 = pcm->drvals[7];
    dmu.bfc = pcm->drvals[8]; dmu.bbc = pcm->drvals[9];
    dmu.boc1 = pcm->drvals[10]; dmu.boc2 = pcm->drvals[11];
    dmu.soc1 = pcm->drvals[12]; dmu.soc2 = pcm->drvals[13];
    dmu.thk = pcm->drvals[14];

    if ((cmd = nxtwrd (cmd)) == NULL) goto errpm;
    dmu.len = 0;
    while (*(cmd+dmu.len)) dmu.len++;
    sz = dmu.len+1;
    dmu.ch = (char *)galloc(sz,"dmu1");
    *(dmu.ch+dmu.len) = '\0';
    if (dmu.ch==NULL) {
      gaprnt(0,"Memory allocation error; DRAW DROPMENU cmd\n");
      return(1);
    }
    for (i=0; i<dmu.len; i++) {
      *(dmu.ch+i) = *(cmd+i);
      if (*(dmu.ch+i)=='|') *(dmu.ch+i) = '\0';
    }
    dsubs->gxdrmu (cnt,&dmu,0,-1);
    return (0);

    errpm:
    gaprnt (0,"DRAW error: Syntax is DRAW DROPMENU number x y w h ");
    gaprnt (0,"string | string | ...\n");
    return (1);
  }

  if (cmpwrd("wxsym",oper)) {
    if ((cmd = nxtwrd (cmd)) == NULL) goto errwx;
    if (intprs(cmd,&wx) == NULL ) goto errwx;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errwx;
    if (getdbl(cmd,&x) == NULL ) goto errwx;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errwx;
    if (getdbl(cmd,&y) == NULL ) goto errwx;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errwx;
    if (getdbl(cmd,&cs) == NULL ) goto errwx;
    thk = 3;
    col = -1;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&col) == NULL ) goto errwx;
      if ((cmd = nxtwrd (cmd)) != NULL) {
        if (intprs(cmd,&thk) == NULL ) goto errwx;
      }
    }
    if (wx<1 || wx>43) goto errwx;
    gxwide (thk);
    gxstyl(1);
    wxsym (wx,x,y,cs,col,pcm->wxcols);
    return (0);

    errwx:
    gaprnt (0,"DRAW error: Syntax is DRAW WXSYM sym x y siz <color <thickness>>");
    gaprnt (0,"<color <thick> >\n");
    return (1);
  }
  if (cmpwrd("string",oper)) {
    if ((cmd = nxtwrd (cmd)) == NULL) goto errst;
    if (getdbl(cmd,&x) == NULL ) goto errst;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errst;
    if (getdbl(cmd,&y) == NULL ) goto errst;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errst;
    c1 = cmd;
    len=0;
    while (*c1!='\0' && *c1!='\n') {len++; c1++;}
    gxwide (pcm->strthk);
    gxcolr (pcm->strcol);

    swide = 0.2;
    gxchln (cmd,len,pcm->strhsz,&swide);
    shite = pcm->strvsz;

    ang = pcm->strrot*M_PI/180.0;
    x = x - justx[pcm->strjst] * swide * cos(ang);
    y = y - justx[pcm->strjst] * swide * sin(ang);
    x = x - justy[pcm->strjst] * shite * cos(ang+1.5708);
    y = y - justy[pcm->strjst] * shite * sin(ang+1.5708);

    gxchpl (cmd,len,x,y,pcm->strvsz,pcm->strhsz,pcm->strrot);
    return (0);

    errst:
    gaprnt (0,"DRAW error: Syntax is DRAW STRING x y string\n");
    return (1);
  }
  if (cmpwrd("rec",oper)) {
    if ((cmd = nxtwrd (cmd)) == NULL) goto errrc;
    if (getdbl(cmd,&xlo) == NULL ) goto errrc;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errrc;
    if (getdbl(cmd,&ylo) == NULL ) goto errrc;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errrc;
    if (getdbl(cmd,&xhi) == NULL ) goto errrc;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errrc;
    if (getdbl(cmd,&yhi) == NULL ) goto errrc;
    if (xlo>=xhi || ylo>=yhi) goto errrc;
    gxwide (pcm->linthk);
    gxcolr (pcm->lincol);
    gxstyl (pcm->linstl);
    gxplot (xlo,ylo,3);
    gxplot (xhi,ylo,2);
    gxplot (xhi,yhi,2);
    gxplot (xlo,yhi,2);
    gxplot (xlo,ylo,2);
    return (0);

    errrc:
    gaprnt (0,"DRAW error: Syntax is DRAW REC xlo ylo xhi yhi\n");
    return (1);
  }
  if (cmpwrd("circ",oper) || cmpwrd("circf",oper)) {
    if ((cmd = nxtwrd (cmd)) == NULL) goto errcirc;
    if (getdbl(cmd,&x) == NULL ) goto errrc;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errcirc;
    if (getdbl(cmd,&y) == NULL ) goto errrc;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errcirc;
    if (getdbl(cmd,&r) == NULL ) goto errrc;
    gxwide (pcm->linthk);
    gxcolr (pcm->lincol);
    gxstyl (pcm->linstl);
    if (cmpwrd("circf",oper)) 
      gxcirc (x,y,r,1);
    else
      gxcirc (x,y,r,0);
    return (0);

    errcirc:
    if (cmpwrd("circf",oper)) 
      gaprnt (0,"DRAW error: Syntax is DRAW CIRCF x y radius\n");
    else
      gaprnt (0,"DRAW error: Syntax is DRAW CIRC x y radius\n");
    return (1);
  }

  /* The interface for drawing shapefiles is adapted from a design by Graziano Giuliani */
  if (cmpwrd("shp",oper)) {
#if USESHP==1
    /* Shapefiles require an x/y varying environment, and input is lon/lat pairs.  
       Interpolation along the sides is done to insure curvature along the map projection. 
       Conversion from lon/lat to x/y is performed, then the polygon is plotted.  */
    if (pcm->xdim<0 && pcm->ydim<0) {
      gaprnt (2,"No scaling environment\n");
      return (1);
    }
    if (pcm->xdim!=0 || pcm->ydim!=1) {
      gaprnt (0,"DRAW SHP error: Invalid Dimension Environment\n");
      gaprnt (0,"                X and Y must be varying\n");
      return (1);
    }

    /* count the args */
    i = 0;
    ccmd = cmd;
    while ( (ccmd = nxtwrd (ccmd)) != NULL) i++;
    if (i<1) {
      gaprnt(0,"DRAW error: Syntax is DRAW SHP filename ... \n"); return (1);
    }
    cmd=nxtwrd(cmd);

    /* parse the shapefile name */
    getwrd(shparg,cmd,4095);
    begshp = -1;

    /* parse user-specified range of shapes to draw */
    if (i>1) {
      cmd=nxtwrd(cmd);
      if (intprs(cmd,&ival)==NULL) return(1);
      begshp=ival;
      if (i>2) {
	cmd=nxtwrd(cmd);
	if (intprs(cmd,&ival)==NULL) return(1);
	endshp=ival;
      }
      else endshp = begshp;
    }

    /* open the shapefile */
    shpid = gaopshp(shparg);
    if (shpid==NULL) return(1);
    SHPGetInfo (shpid, &shpcnt, &shptype, NULL, NULL);
    if (begshp==-1) {  /* draw all shapes in file */
      begshp=0; 
      endshp=shpcnt-1; 
    } 

    /* Determine increment for interpolation */
    llinc = hypot(pcm->dmax[0]-pcm->dmin[0], pcm->dmax[1]-pcm->dmin[1]);
    llinc = llinc/200.0;
    if (llinc<0.0001) llinc=0.0001;

    /* set the clipping region */
    gxclip (pcm->xsiz1, pcm->xsiz2, pcm->ysiz1, pcm->ysiz2);
    /* loop over requested shapes */
    for (i=begshp; i<=endshp; i++) {
      if ((shp = SHPReadObject (shpid,i))==NULL) { 
	gaprnt(0,"SHPReadObject failed\n"); 
	SHPClose (shpid);
	return(1); 
      }
      /* skip shapes of a certain kind ... */
      if ((shp->nSHPType==0) ||             /* type is NULL */
	  (shp->nSHPType!=shptype) ||       /* type is different from file header */
	  (shp->nParts<0) ||                /* shape has no parts */
	  (shp->nVertices<=0) ||            /* shape has no vertices */
	  (shp->dfYMax < pcm->dmin[1]) ||   /* shape is below dim env lat range */
	  (shp->dfYMin > pcm->dmax[1]) ) {  /* shape is above dim env lat range */
	SHPDestroyObject (shp); shp=NULL;
	continue;
      }
      /* determine shape type */
      shape=0;
      if ((shp->nSHPType==SHPT_POINT) || 
	  (shp->nSHPType==SHPT_MULTIPOINT) || 
	  (shp->nSHPType==SHPT_POINTZ) || 
	  (shp->nSHPType==SHPT_MULTIPOINTZ) || 
	  (shp->nSHPType==SHPT_POINTM) || 
	  (shp->nSHPType==SHPT_MULTIPOINTM)) 
	shape=1;  /* points */
      if ((shp->nSHPType==SHPT_ARC)  || 
	  (shp->nSHPType==SHPT_ARCZ) ||
	  (shp->nSHPType==SHPT_ARCM)) 
	shape=2;  /* lines */
      if ((shp->nSHPType==SHPT_POLYGON)  || 
	  (shp->nSHPType==SHPT_POLYGONZ) ||
	  (shp->nSHPType==SHPT_POLYGONM)) 
	shape=3;  /* polygons */
      
      if (shape>0) {
	/* get the number of parts in this shape */
	haveinfo=1;
	if (shp->nParts==0) {	/* implies single part with no info */
	  numparts=1;
	  haveinfo=0;
	} 
	else {
	  numparts=shp->nParts;
	}
	/* loop over all parts  */
	for (p=0; p<numparts; p++) {
	  if (haveinfo) {
	    begv = shp->panPartStart[p];         /* starting vertex for this part */
	    if (p==shp->nParts-1) 
	      endv = shp->nVertices-1;           /* final vertex */
	    else
	      endv = shp->panPartStart[p+1]-1;   /* vertex preceding start of next part */
	  } 
	  else {
	    begv = 0;
	    endv = shp->nVertices-1;
	  }  
	  cnt = endv - begv + 1;                 /* number of vertices in this part */


	  /* ensure shapes will wrap the globe and be drawn within the entire dim env lon range */
	  lnfact = 0.0;
	  while (shp->dfXMax+lnfact > pcm->dmin[0]) lnfact -= 360.0;
	  lnfact += 360.0; 
	  
	  while (shp->dfXMin+lnfact < pcm->dmax[0]) { 
	    if (shp->dfXMax+lnfact < pcm->dmin[0]) {
	      lnfact += 360.0;
	      continue;
	    }

	    /* allocate memory for x/y coordinates */
	    if ((pxy = (gadouble *)galloc(2*(cnt+1)*sizeof(gadouble),"shpxy"))==NULL) {
	      gaprnt (0,"DRAW error: Memory allocation failed for shp vertices\n");
	      SHPDestroyObject (shp); shp=NULL;
	      SHPClose (shpid);
	      return (1);
	    }
	    
	    /* copy the lat/lon pairs for this part into the pxy array */
	    j=0;
	    for (v=begv; v<=endv; v++) {
	      *(pxy+j+0) = shp->padfX[v]+lnfact;
	      *(pxy+j+1) = shp->padfY[v];
	      j+=2;
	    }
	    /* Make sure the polygon is closed. It should be, 
	       if shapefile conforms to ESRI specs, but just in case ... */
	    if (shape==3) {
	      if (*pxy != *(pxy+(cnt-1)*2) || *(pxy+1) != *(pxy+(cnt-1)*2+1)) {
		*(pxy+cnt*2) = *pxy;
		*(pxy+cnt*2+1) = *(pxy+1);
		cnt++;
	      }
	    }
	    /* Draw the shape */	  
	    if (shape==1) {   
	      /* shape is a point */
	      gxcolr (pcm->lincol);
	      j=0;
	      for (v=begv; v<=endv; v++) {
		/* convert lon,lat to x,y */
		gxconv (*(pxy+j+0),*(pxy+j+1),&newx,&newy,2);
		gxmark (pcm->marktype,newx,newy,pcm->marksize);
		j+=2;
	      }
	      gree(pxy,"f204a");
	    }
	    else {     
	      /* shape is a line or polygon  */
	      /* gxmpoly routine converts lon,lat to x,y and interpolates to smaller 
		 line segments so that shapes drawn on curved projections look right. */
	      newxy = gxmpoly(pxy,cnt,llinc,&newcnt); 
	      gree(pxy,"f204a");
	      if (newxy==NULL) { 
		gaprnt (0,"DRAW SHP error: Memory allocation in gxmpoly\n");
		SHPDestroyObject (shp); shp=NULL;
		SHPClose (shpid);
		return (1);
	      }
	      /* Make sure the new polygon is closed, then draw it */
	      if (shape==3) {
		if (*newxy != *(newxy+(newcnt-1)*2) || *(newxy+1) != *(newxy+(newcnt-1)*2+1)) {
		  *(newxy+newcnt*2) = *newxy;
		  *(newxy+newcnt*2+1) = *(newxy+1);
		  newcnt++;
		}
		/* fill in the polygon */
		if (pcm->fillpoly!=-1) {
		  gxcolr (pcm->fillpoly);
		  gxfill (newxy,newcnt);
		}
	      }
	      
	      /* draw the line, or draw border around the polygon */
  
	      gxwide (pcm->linthk);            
	      gxstyl(pcm->linstl); 
	      gxcolr (pcm->lincol);
	      gxplot (*(newxy),*(newxy+1),3);
	      for (j=1; j<newcnt; j++) {
		gxplot (*(newxy+j*2),*(newxy+j*2+1),2);
	      }
  
	      /* release memory holding the x,y coordinates for this part */
	      gree(newxy,"f204a");
	    }

	    lnfact += 360.0;
	  }
	} /* end of loop over numparts */
      }
      /* The only type that should get trapped here is a MultiPatch (value 31) */
      else {
	snprintf(pout,1255,"Warning: shape type %d is not supported \n",shp->nSHPType);
	gaprnt(2,pout);
      }
      
      /* release memory for this shape */
      SHPDestroyObject (shp); shp=NULL;
    }
    SHPClose (shpid);
    /* Calling gafram() here is important because the shapes are likely to overlap the frame 
       and it will need to be redrawn. Also, gafram() will change the color which will
       trigger the reset of the color mask.  */
    gafram(pcm);
    /* reset the clipping area */
    gxclip (0.0, pcm->xsiz, 0.0, pcm->ysiz);

    return(0);
#else
    gaprnt(0,"This build does not support drawing shapefiles\n");
    return(1);
#endif
  }
  if (cmpwrd("polyf",oper) || cmpwrd("mappoly",oper)) {
    i = 0;
    ccmd = cmd;
    while ( (ccmd = nxtwrd (ccmd)) != NULL) i++;
    if (i<6) {
      gaprnt (0,"DRAW error: Syntax is DRAW POLYF x1 y1 x2 y2 ...\n");
      return (1);
    }
    sz = sizeof(gadouble)*(i+2);
    xy = (gadouble *)galloc(sz,"xy");
    if (xy==NULL) {
      gaprnt (0,"DRAW error: Memory allocation error\n");
      return (1);
    }
    i = 0;
    while ( (cmd = nxtwrd(cmd)) != NULL) {
      if (getdbl(cmd,xy+i) == NULL ) {
        gaprnt (0,"DRAW error: Invalid polyf coordinate\n");
        gree(xy,"f202");
        return(1);
      }
      i++;
    }
    cnt = i/2;
    if (*xy != *(xy+(cnt-1)*2) || *(xy+1) != *(xy+(cnt-1)*2+1)) {
      *(xy+cnt*2) = *xy;
      *(xy+cnt*2+1) = *(xy+1);
      cnt++;
    }

    /*  Mappoly requires an x/y varying environment, and assumes
        input is lon/lat pairs.  Interpolation along the side
        is done to insure curvature along the map projection, 
        then conversion from lon/lat to x/y is performed, then
        the polygon is plotted.  */

    if (cmpwrd("mappoly",oper)) {

      /* Check for x/y varying -- not really required, but 
         cannot think of any reason to allow for other dimension
         environments. */

      if (pcm->xdim!=0 && pcm->ydim!=1) {
        gree(xy,"f203");
        gaprnt (0,"DRAW MAPPOLY error: Invalid Dimension Environment\n");
        gaprnt (0,"                    X and Y must be varying\n");
        return (1);
      }

      /* Determine increment for interpolation */

      llinc = hypot(pcm->dmax[0]-pcm->dmin[0], pcm->dmax[1]-pcm->dmin[1]);
      llinc = llinc/200.0;
      if (llinc<0.0001) llinc=0.0001;
  
      /* Do the interpolation and convert to x,y -- 
         the gxmpoly routine does this and is located in gxwmap */

      newxy = gxmpoly(xy,cnt,llinc,&newcnt); 
      gree(xy,"f204");
      if (newxy==NULL) { 
        gaprnt (0,"DRAW MAPPOLY error: Memory allocation\n");
        return (1);
      }
      xy = newxy;
      cnt = newcnt;
      gxcolr (pcm->lincol);
      /*  draw border to avoid gaps */
      gxwide (1);
      gxstyl(1);
      gxcolr (pcm->lincol);
      gxplot (*(xy),*(xy+1),3);
      for (i=1; i<cnt; i++) {
        gxplot (*(xy+i*2),*(xy+i*2+1),2);
      }
    }
    gxcolr (pcm->lincol);
    if (pcm->ptflg) gxptrn (pcm->ptopt,pcm->ptden,pcm->ptang);
    gxfill (xy,cnt);
    if (pcm->ptflg) gxptrn (1,1,0);
    gree(xy,"f205");
    return (0);
  }
  if (cmpwrd("recf",oper) || cmpwrd("maskrec",oper)) {
    if ((cmd = nxtwrd (cmd)) == NULL) goto errfc;
    if (getdbl(cmd,&xlo) == NULL ) goto errfc;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errfc;
    if (getdbl(cmd,&ylo) == NULL ) goto errfc;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errfc;
    if (getdbl(cmd,&xhi) == NULL ) goto errfc;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errfc;
    if (getdbl(cmd,&yhi) == NULL ) goto errfc;
    if (xlo>=xhi || ylo>=yhi) goto errfc;
    if (cmpwrd("maskrec",oper)) {
      gxmaskrec (xlo,xhi,ylo,yhi);
      return(0);
    }
    gxcolr (pcm->lincol);
    if (pcm->ptflg) gxptrn (pcm->ptopt,pcm->ptden,pcm->ptang);
    gxrecf (xlo,xhi,ylo,yhi);
    if (pcm->ptflg) gxptrn (1,1,0);
    return (0);

    errfc:
    gaprnt (0,"DRAW error: Syntax is DRAW RECF xlo ylo xhi yhi\n");
    return (1);
  }
  if (cmpwrd("line",oper)) {
    gxwide (pcm->linthk);
    gxcolr (pcm->lincol);
    gxstyl (pcm->linstl);
    if ((cmd = nxtwrd (cmd)) == NULL) goto errln;
    if (getdbl(cmd,&xlo) == NULL ) goto errln2;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errln;
    if (getdbl(cmd,&ylo) == NULL ) goto errln2;
    gxplot (xlo,ylo,3);
    while ( (cmd = nxtwrd(cmd)) != NULL) {
      if (getdbl(cmd,&xlo) == NULL ) goto errln2;
      if ((cmd = nxtwrd (cmd)) == NULL) goto errln3;
      if (getdbl(cmd,&ylo) == NULL ) goto errln2;
      gxplot (xlo,ylo,2);
    }
    return (0);

    errln:
    gaprnt (0,"DRAW error: Syntax is DRAW LINE x1 y1 x2 y2\n");
    return (1);
    errln2:
    gaprnt (0,"DRAW error: Invalid LINE coordinate\n");
    return (1);
    errln3:
    gaprnt (0,"DRAW LINE error: Missing Y coordinate\n");
    return (1);
  }
  if (cmpwrd("mark",oper)) {
    if ((cmd = nxtwrd (cmd)) == NULL) goto errmk;
    if (intprs(cmd,&mk) == NULL ) goto errmk;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errmk;
    if (getdbl(cmd,&x) == NULL ) goto errmk;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errmk;
    if (getdbl(cmd,&y) == NULL ) goto errmk;
    if ((cmd = nxtwrd (cmd)) == NULL) goto errmk;
    if (getdbl(cmd,&cs) == NULL ) goto errmk;
    gxwide (pcm->linthk);
    gxcolr (pcm->lincol);
    gxstyl (1);
    gxmark (mk,x,y,cs);
    return (0);

    errmk:
    gaprnt (0,"DRAW error: Syntax is DRAW MARK marktype x y size\n");
    return (1);
  }
  if (cmpwrd("title",oper) || cmpwrd("xlab",oper) ||
      cmpwrd("ylab",oper)) {

    /* Count number of strings, delete leading blanks, change string
       delimeter (the backslash) to nulls. */

    if ((cmd=nxtwrd(cmd)) == NULL) {
      gaprnt (0,"DRAW error: Missing character string \n");
      return (1);
    }

    cnt = 1;
    c1 = cmd;
    c2 = chars;
    cflg = 1;
    while (*c1!='\0' && *c1!='\n') {
      if (cflg && *c1==' ') c1++;
      else {
        *c2 = *c1;
        cflg = 0;
        if (*c2=='\\') {
          *c2 = '\0';
          cnt++;
          cflg = 1;
        }
        c1++; c2++;
      }
    }
    *c2 = '\0';

    if (cmpwrd("title",oper)) {
      gxcolr (pcm->anncol);
      gxwide (pcm->annthk);
      cs = 0.2;
      if (cnt==2) cs=0.175;
      if (cnt==3) cs=0.15;
      if (cnt>3) cs=0.125;
      x = pcm->xsiz1 + (pcm->xsiz2-pcm->xsiz1)/2.0;
      y = pcm->ysiz2 + cs*(gadouble)(cnt-1)*1.7 + cs*0.4 + 0.1;
      ipos = 0;
      for (i=0; i<cnt; i++) {
        swide = 1.0;
        gxchln (&(chars[ipos]),500,cs,&swide);
        gxchpl (&(chars[ipos]),500,x-swide*0.5,y,cs*1.2,cs,0.0);
        y = y - cs*1.7;
        while (chars[ipos]) ipos++;
        ipos++;
      }
    }
    else if (cmpwrd("xlab",oper)) {
      gxcolr (pcm->anncol);
      gxwide (pcm->annthk);
      cs = 0.16;
      if (cnt==2) cs=0.14;
      if (cnt>2)  cs=0.12;
      x = pcm->xsiz1 + (pcm->xsiz2-pcm->xsiz1)/2.0;
      y = pcm->ysiz1 - cs*1.7 - 0.3;
      ipos = 0;
      for (i=0; i<cnt; i++) {
        swide = 1.0;
        gxchln (&(chars[ipos]),500,cs,&swide);
        gxchpl (&(chars[ipos]),500,x-swide*0.5,y,cs*1.2,cs,0.0);
        y = y - cs*1.7;
        while (chars[ipos]) ipos++;
        ipos++;
      }
    }
    else if (cmpwrd("ylab",oper)) {
      gxcolr (pcm->anncol);
      gxwide (pcm->annthk);
      cs = 0.16;
      if (cnt==2) cs=0.14;
      if (cnt>2)  cs=0.12;
      y = pcm->ysiz1 + (pcm->ysiz2-pcm->ysiz1)/2.0;
      x = pcm->xsiz1 - pcm->yllow - cs*1.7*(gadouble)(cnt-1) - cs;
      ipos = 0;
      for (i=0; i<cnt; i++) {
        swide = 1.0;
        gxchln (&(chars[ipos]),500,cs,&swide);
        gxchpl (&(chars[ipos]),500,x,y-swide*0.5,cs*1.2,cs,90.0);
        x = x + cs*1.7;
        while (chars[ipos]) ipos++;
        ipos++;
      }
    }
    return (0);
  }
  gaprnt (0,"DRAW error:  Invalid operand\n");
  return (1);
}

/* handle enable command */

gaint gaenab (char *cmd, struct gacmn *pcm) {
gaint rc;
char cc[256], *ch;

  for (rc=0; rc<190; rc++) cc[rc] = *(cmd+rc);
  cc[40]='\0';
  lowcas (cc);
  if ((ch=nxtwrd(cc)) == NULL) {
    gaprnt (0,"ENABLE error: Missing operand\n");
    return (1);
  }
  if (!cmpwrd("print",ch)) {
    gaprnt (0,"ENABLE error: Invalid operand \n");
    return (1);
  }
  else {
    gaprnt(0,"ERROR: The \"enable print\" command is no longer valid. Use \"gxprint\" instead.\n");
    return (1);
  }
  if ((ch=nxtwrd(ch)) == NULL) {
    gaprnt (0,"ENABLE error: Missing file name \n");
    return (1);
  }
  cmd = nxtwrd(cmd);
  cmd = nxtwrd(cmd);
  getwrd (cc,cmd,256);
  /* if we ever re-use the enable command, this is where the function call will go */
  return (rc);
}

/* For gradspy -- called in grads.c when SHRDOBJ==1 */
/* Saves a pointer to the gacmn structure */
void gapysavpcm(struct gacmn *pcm) {
  savpcm = pcm;
}

/* Execute command on behalf of the scripting language.
   Return the result in a dynamically allocated buffer */

char *gagsdo (char *cmd, gaint *rc) {
gaint savflg,tlen,i;
struct gacmn *pcm;
char *mbuf, *ch;

   savflg = msgflg;
   msgflg = 1;          /* Buffer messages */
   pcm = savpcm;        /* Get common pointer */
   msgstk = NULL;

   *rc = gacmd(cmd, pcm, 0);

   /* Set up output buffer */

   if (msgstk==NULL) {
     msgflg = savflg;
     return(NULL);
   }

   tlen = 0;
   msgcurr = msgstk;
   while (msgcurr) {
     tlen += msgcurr->len;
     msgcurr = msgcurr->forw;
   }

   mbuf = (char *)malloc(tlen+1);
   if (mbuf==NULL) {
     printf ("Memory allocation error: Message Return Buffer\n");
     msgflg = savflg;
     return (NULL);
   }

   msgcurr = msgstk;
   ch = mbuf;
   while (msgcurr!=NULL) {
     for (i=0; i<msgcurr->len; i++) {
       *ch = *(msgcurr->msg+i);
       ch++;
     }
     msgcurr = msgcurr->forw;
   }
   msgcurr = msgstk;
   while (msgcurr!=NULL) {
     if (msgcurr->msg) free(msgcurr->msg);
     msgstk = msgcurr->forw;
     free(msgcurr);
     msgcurr = msgstk;
   }

   *(mbuf+tlen) = '\0';
   msgflg = savflg;
   return (mbuf);
}

/* Handle exec command.  Read exec file, then recursively
   call gacmd.  */

gaint gaexec (char *cmd, struct gacmn *pcm) {
FILE *efile;
gaint i,j,n,flag,iarg;
char ename[50];
char ccc[500],cout[500];
char *args[10],*ptr;
gaint rc, savflg, ret;

  efile = NULL;
  savflg = msgflg;
  msgflg = 0;         /* Don't buffer messages */

  if ((cmd=nxtwrd(cmd)) == NULL) {
    gaprnt (0,"EXEC error:  missing file name \n");
    ret = 1;
    goto retrn;
  }

  /* Get file name and open file.  If we cannot open file, exit */

  getwrd (ename,cmd,49);
  efile = fopen(ename,"r");
  if (efile==NULL) {
    gaprnt (0,"EXEC error:  Can't open file\n");
    snprintf(pout,1255,"  File name is: %s\n",ename);
    gaprnt (0,pout);
    ret = 1;
    goto retrn;
  }

  /* Locate any arguments, save pointers to them, and terminate them
     with a null.  We modify the cmd line at this point.  */

  for (i=0;i<10;i++) args[i]=NULL;
  i = 0;
  cmd = nxtwrd(cmd);
  while (cmd != NULL && i<10) {
    args[i] = cmd;
    ptr = cmd;
    if (*ptr == '~') {
      ptr++;
      args[i] = ptr;
      while (*ptr != '~') ptr++;
      cmd = nxtwrd(ptr);
      *ptr = '\0';
    } else {
      cmd = nxtwrd(cmd);
      while (*ptr!=' ' && *ptr!='\0' && *ptr!='\n') ptr++;
      *ptr = '\0';
    }
    i++;
  }


  /* Read and execute the commands in the file.  Scan each command for
     any arg tokens and replace them with the arg strings */

  while ( (fgets(ccc,300,efile))!=NULL) {
    if (*ccc=='*') continue;
    /* Remove cr for PC/cygwin version */
    n=strlen(ccc);
    if ( n > 1 ) {
      if ((gaint)ccc[n-2] == 13) ccc[n-2] = ' ';
    }
    i = 0; j = 0;
    while (ccc[i]!='\n') {
      flag = 1;
      if (ccc[i] == '&' && ccc[i+1]>='0' && ccc[i+1]<='9') {
        iarg = (gaint)(ccc[i+1]) - 48;
        ptr = args[iarg];
        if (ptr!=NULL) {
          while (*ptr!='\0') {
            cout[j] = *ptr;
            j++; ptr++;
          }
          i+=2;
          flag = 0;
        }
      }
      if (flag) {
        cout[j] = ccc[i];
        i++; j++;
      }
    }
    cout[j] = ccc[i];
    cout[j+1] = '\0';
    gaprnt (0,cout);
    rc = gacmd(cout,pcm,1);
    if (rc) {
      snprintf(pout,1255,"EXEC error:  error in %s.  EXEC stopped.\n",ename);
      gaprnt (0,pout);
      ret = rc;
      goto retrn;
    }
  }
  snprintf(pout,1255,"EOF EXECuting %s \n",ename);
  gaprnt (0,pout);
  ret = 0;

retrn:
  if (efile) fclose(efile);
  msgflg = savflg;
  return (ret);
}

/* Handle undefine command */

gaint gaudef (char *cmd, struct gacmn *pcm) {
struct gafile *pfi;
struct gadefn *pdf, *opdf;
char name[20];
gaint i;

  /* Get the define name */
  if ((cmd=nxtwrd(cmd)) == NULL) {
    gaprnt (0,"UNDEFINE error:  name is missing \n");
    return(1);
  }
  i=0;
  while (*cmd!='\0' && *cmd!='\n') {
    name[i] = *cmd;
    cmd++; i++;
    if (i>16) break;
  }
  name[i] = '\0';

  pdf = pcm->pdf1;
  opdf = NULL;
  while (pdf!=NULL) {
    if (cmpwrd(name,pdf->abbrv)) break;
    opdf = pdf;
    pdf = pdf->pforw;
  }
  if (pdf==NULL) {
    gaprnt (1,"UNDEFINE Warning:  name not found\n");
    return (0);
  } else {
    if (opdf==NULL) pcm->pdf1 = pdf->pforw;
    else opdf->pforw = pdf->pforw;
    pfi = pdf->pfi;
    gree(pfi->rbuf,"f160");
    gree(pfi->ubuf,"f160a");
    for (i=0; i<5; i++) {
      gree(pfi->grvals[i],"f161");
      /* gradspy defined objects don't need to free abvals */
      if (pfi->type!=5) gree(pfi->abvals[i],"f162"); 
    }
    gree(pfi,"f163");
    gree(pdf,"f164");
    snprintf(pout,1255,"%s UNDEFINEd and storage released\n",name);
    gaprnt (2,pout);
  }
  return (0);
}

/* Query or modify define data value -- modify if flag = 1 */

gaint gaqdef (char *cmd, struct gacmn *pcm, gaint flag) {
struct gafile *pfi;
struct gadefn *pdf;
gadouble grval,*gr;
gaint i,gri,grj;
char name[20];
char *ch;
char *gru;

  /* Get the defined variable name */
  if ((cmd=nxtwrd(cmd)) == NULL) goto err;
  i=0;
  ch = cmd;
  while (*ch!='\0' && *ch!='\n') {
    name[i] = *ch;
    ch++; i++;
    if (i>16) break;
  }
  name[i] = '\0';

  /* Get i, j, and value */
  if ((cmd=nxtwrd(cmd)) == NULL) goto err;
  if (intprs(cmd,&gri)  == NULL) goto err;
  if ((cmd=nxtwrd(cmd)) == NULL) goto err;
  if (intprs(cmd,&grj)  == NULL) goto err;
  if (flag) {
    if ((cmd=nxtwrd(cmd))  == NULL) goto err;
    if ((strncmp(cmd,"missing",7)) == 0) flag=2;
    else if (getdbl(cmd,&grval) == NULL) goto err;
  }

  /* Locate defined object in link list */
  pdf = pcm->pdf1;
  while (pdf!=NULL) {
    if (cmpwrd(name,pdf->abbrv)) break;
    pdf = pdf->pforw;
  }
  if (pdf==NULL) {
    gaprnt (1,"Warning:  defined name not found\n");
    return (0);
  }

  /* Locate desired value in defined object */
  pfi = pdf->pfi;
  gri = gri - (pfi->dimoff[0]+1);
  grj = grj - (pfi->dimoff[1]+1);

  if (flag) {
    /* Modify existing data point */
    if (gri<0 || gri>=pfi->dnum[0] || grj<0 || grj>=pfi->dnum[1]) {
      gaprnt (0,"SET DEFVAL Error:  Out of Range\n");
      return (1);
    }
    gru = pfi->ubuf;
    gru = gru + (grj*pfi->dnum[0]+gri);    
    if (flag==2) {
      /* set value to be undefined */
      *gru = 0;
     }
    else {
      /* set new value */
      gr = pfi->rbuf;
      gr = gr + (grj*pfi->dnum[0]+gri);
      *gr = grval;
      *gru = 1;
    }
  } 
  else {
    /* Print defined value */
    if (gri<0 || gri>=pfi->dnum[0] || grj<0 || grj>=pfi->dnum[1]) {
      gaprnt(2,"DEFVAL is out of range\n");
    } 
    else {
      gr = pfi->rbuf;
      gr = gr + (grj*pfi->dnum[0]+gri);
      gru = pfi->ubuf;
      gru = gru + (grj*pfi->dnum[0]+gri);
      if (*gru == 1) {
	snprintf(pout,1255,"DEFVAL is %g\n",*gr);
	gaprnt (2,pout);
      }
      else {
	gaprnt(2,"DEFVAL is missing\n");
      }
    }
  }
  return(0);

err:
  if (flag) {
    gaprnt(0,"SET DEFVAL Error: Syntax is: ");
    gaprnt(0,"set defval name i j value (value may be \"missing\")\n");
  } else {
    gaprnt(0,"QUERY DEFVAL Error: Syntax is: ");
    gaprnt(0,"query defval name i j\n");
  }
  return(1);
}

/* Modify attributes or contents of a defined grid */

gaint gamodf (char *cmd, struct gacmn *pcm) {
struct gadefn *pdf;
struct gafile *pfi;
struct dt dtim,otim;
gadouble t1,t2,d1;
gaint i,flg;
char name[20];

  /* Get the name of the defined grid (2nd arg) */
  if ((cmd=nxtwrd(cmd)) == NULL) {
    gaprnt (0,"MODIFY error:  name is missing \n");
    return (1);
  }

  i=0;
  while (*(cmd+i)!=' ' && *(cmd+i)!='\n' && *(cmd+i)!='\0' && i<17) {
    name[i] = *(cmd+i);
    i++;
  }
  name[i] = '\0';

  /* See if the name is a defined grid */
  pdf = pcm->pdf1;
  while (pdf!=NULL && !cmpwrd(name,pdf->abbrv)) pdf = pdf->pforw;
  if (pdf==NULL) {
    snprintf(pout,1255,"MODIFY Error: Defined grid %s not found\n",name);
    gaprnt (0,pout);
    return (1);
  }

  /* Take action based on 3rd argument */
  if ((cmd=nxtwrd(cmd)) == NULL) {
    gaprnt (0,"MODIFY error:  Action keyword is missing\n");
    return (1);
  }
  else if (cmpwrd("seasonal",cmd) || cmpwrd("diurnal",cmd)) {
    pfi = pdf->pfi;
    if (pfi->dnum[3]==1) {
      gaprnt(0,"MODIFY Error:  ");
      gaprnt(0,"Time not varying for this defined variable\n");
      return(1);
    }

    /* Convert starting grid time to world time, add appropriate
       increment, and convert back to grid */
    t1 = (gadouble)(pfi->dimoff[3]+1);
    gr2t (pfi->grvals[3], t1, &dtim);
    otim.yr=0; otim.mo=0; otim.dy=0; otim.hr=0; otim.mn=0;
    if (cmpwrd("seasonal",cmd)) {
      otim.yr = 1;
      flg = 1;
    } else {
      otim.dy = 1;
      flg = 2;
    }
    timadd (&dtim, &otim);
    t2 = t2gr(pfi->abvals[3],&otim);

    /* Check that the final time is integral -- otherwise ... a serious problem */
    if (t2<0.0) i = (gaint)(t2-0.1);
    else i = (gaint)(t2+0.1);
    d1 = (gadouble)i;
    if (fabs(t2-d1)>0.001 || (flg==1 && (t2-t1>12.5))) { 
      gaprnt (0,"MODIFY Error:  Invalid time scaling in defined variable\n");
      return (1);
    }

    /* Calculate cyclic time in grid units, set up pfi block, return */
    pfi->cysiz = (gaint)(0.1 + t2 - t1);
    pfi->climo = flg;
    gaprnt (0,"Defined variable is now climatological\n");
    return (0);
  }
  else {
    gaprnt (0,"MODIFY error: Invalid operand\n");
    gaprnt (0,"  Operand = ");
    gaprnt (0,cmd);
    gaprnt (0,"\n");
    return (1);
  }

}


/* Handle define command */

gaint gadef (char *cmd, struct gacmn *pcm, gaint impf) {
struct gagrid *pgr,*pgr1;
struct gastat *pst;
struct gafile *pfi,*pfiv,*pfic;
struct gadefn *pdf,*pcurr,*psave;
struct gadefn **prev;
struct dt tmin,tmax;
gadouble (*conv) (gadouble *, gadouble);
gadouble vmin,vmax,zmin,zmax,emin,emax,*res,*gr;
gaint itmin,itmax,it,izmin,izmax,iz,iemin,iemax,ie;
gaint i,rc,gsiz,vdz,vdt,vde;
size_t sz,siz;
char *resu,*gru;
char name[20];

  pdf = NULL;
  pst = NULL;
  pgr1 = NULL;
  pfiv = NULL;

  /* Save user dim limits */
  zmin = pcm->dmin[2];
  zmax = pcm->dmax[2];
  tmin = pcm->tmin;
  tmax = pcm->tmax;                    
  emin = pcm->dmin[4];
  emax = pcm->dmax[4];
  vdz  = pcm->vdim[2];
  vdt  = pcm->vdim[3];
  vde  = pcm->vdim[4];

  /* Get the define name */
  if (!impf) {
    if ((cmd=nxtwrd(cmd)) == NULL) {
      gaprnt (0,"DEFINE error:  name is missing \n");
      goto retrn;
    }
  }
  garemb (cmd);
  i=0;
  while ( (*cmd>='a' && *cmd<='z') || (*cmd>='0' && *cmd<='9' ) ) {
    name[i] = *cmd;
    cmd++; i++;
    if (i>16) break;
  }
  name[i] = '\0';
  if (*cmd!='=') {
    gaprnt (0,"DEFINE error: Failed to find '=' after parsing the defined variable name \n");
    gaprnt (0,"              The name may be too long or contain non-alphanumeric characters \n");
    goto retrn;
  }
  cmd++;
  if (*cmd=='\0') {
    gaprnt (0,"DEFINE error: Missing expression\n");
    goto retrn;
  }

  /* We are now pointing to the expression.  We need to set up
     our looping environment -- we are going to always loop
     through Z and T and E.                                */

  pfi = pcm->pfid;
  if (pfi->type==2 || pfi->type==3) {
    gaprnt (0,"DEFINE error: Define not yet valid for station data\n");
    gaprnt (0,"              Default file is a station data file\n");
    goto retrn;
  }

  conv = pfi->ab2gr[2];                /* Get Z grid limits */
  vmin = conv(pfi->abvals[2],zmin);
  vmax = conv(pfi->abvals[2],zmax);
  if (dequal(vmin,vmax,1.0e-08)==0) {
    if (vmin<0.0) izmin = (gaint)(vmin-0.5);
    else izmin = (gaint)(vmin+0.5);
    izmax = izmin;
  } else {
    izmin = (gaint)(floor(vmin+0.001));
    izmax = (gaint)(ceil(vmax-0.001));
  }
  

  vmin = t2gr(pfi->abvals[3],&tmin);   /* Get T grid limits */
  vmax = t2gr(pfi->abvals[3],&tmax);
  if (dequal(vmin,vmax,1.0e-08)==0) {
    if (vmin<0.0) itmin = (gaint)(vmin-0.5);
    else itmin = (gaint)(vmin+0.5);
    itmax = itmin;
  } else {
    itmin = (gaint)(floor(vmin+0.001));
    itmax = (gaint)(ceil(vmax-0.001));
  }

  conv = pfi->ab2gr[4];                /* Get E grid limits */
  vmin = conv(pfi->abvals[4],emin);
  vmax = conv(pfi->abvals[4],emax);
  if (dequal(vmin,vmax,1.0e-08)==0) {
    if (vmin<0.0) iemin = (gaint)(vmin-0.5);
    else iemin = (gaint)(vmin+0.5);
    iemax = iemin;
  } else {
    iemin = (gaint)(floor(vmin+0.001));
    iemax = (gaint)(ceil(vmax-0.001));
  }
 
  /* Fix Z and T and E dimensions */
  pcm->dmax[2] = pcm->dmin[2];
  pcm->tmax    = pcm->tmin;               
  pcm->dmax[4] = pcm->dmin[4];               
  pcm->vdim[2] = 0;
  pcm->vdim[3] = 0;
  pcm->vdim[4] = 0;

  /* Get the first grid */
  pst = getpst(pcm);
  if (pst==NULL) goto retrn;

  conv = pfi->gr2ab[2];
  pst->dmin[2] = conv(pfi->grvals[2],(gadouble)izmin);
  pst->dmax[2] = pst->dmin[2];
  gr2t (pfi->grvals[3], (gadouble)itmin, &(pst->tmin));
  pst->tmax = pst->tmin;
  conv = pfi->gr2ab[4];
  pst->dmin[4] = conv(pfi->grvals[4],(gadouble)iemin);
  pst->dmax[4] = pst->dmin[4];

  rc = gaexpr(cmd, pst);
  if (!rc) rc = gaqsig();
  if (rc) {
    gaprnt (0,"DEFINE error:  Invalid expression. \n");
    goto retrn;
  }
  if (pst->type!=1) {
    gaprnt (0,"DEFINE Error:  Define does not yet support station data\n");
    gaprnt (0,"    Expression results in station data object\n");
    goto retrn;
  }

  /* Based on the grid we just got, we can now figure out the size of
     the final defined grid and fill in the gafile structure for the
     defined grid.  Allocate all the necessary stuff and fill it all
     in.  */

  /* Allocate the pdf and pfi blocks */
  sz = sizeof(struct gadefn);
  pdf = (struct gadefn *)galloc(sz,"pdfdef");
  if (pdf==NULL) {
    gaprnt (0,"Memory Allocation Error:  DEFINE operation\n");
    goto retrn;
  }
  sz = sizeof(struct gafile);
  pfiv = (struct gafile *)galloc(sz,"pfiv");
  if (pfiv==NULL) {
    gaprnt (0,"Memory Allocation Error:  DEFINE operation\n");
    goto retrn;
  }
  pdf->pfi = pfiv;
  pfiv->rbuf = NULL;
  pfiv->sbuf = NULL;
  pfiv->ubuf = NULL;

  /* Fill in the pfi block */
  pgr1 = pst->result.pgr;
  pfiv->type = 4;
  pfiv->climo = 0;
  pfiv->calendar = pfi->calendar;
  pfiv->undef = pgr1->undef;
  pfiv->dnum[2] = 1 + izmax - izmin;
  pfiv->dnum[3] = 1 + itmax - itmin;
  pfiv->dnum[4] = 1 + iemax - iemin;
  pfiv->gr2ab[2] = pfi->gr2ab[2];
  pfiv->ab2gr[2] = pfi->ab2gr[2];
  pfiv->gr2ab[4] = pfi->gr2ab[4];
  pfiv->ab2gr[4] = pfi->ab2gr[4];

  if ((pfiv->grvals[2] = cpscal (pfi->grvals[2], pfi->linear[2], 0, 2))==NULL) goto etrn;
  if ((pfiv->abvals[2] = cpscal (pfi->abvals[2], pfi->linear[2], 1, 2))==NULL) goto etrn;

  if ((pfiv->grvals[3] = cpscal (pfi->grvals[3], pfi->linear[3], 0, 3))==NULL) goto etrn;
  if ((pfiv->abvals[3] = cpscal (pfi->abvals[3], pfi->linear[3], 1, 3))==NULL) goto etrn;

  if ((pfiv->grvals[4] = cpscal (pfi->grvals[4], pfi->linear[4], 0, 4))==NULL) goto etrn;
  if ((pfiv->abvals[4] = cpscal (pfi->abvals[4], pfi->linear[4], 1, 4))==NULL) goto etrn;

  pfiv->linear[2] = pfi->linear[2];
  pfiv->linear[3] = pfi->linear[3];
  pfiv->linear[4] = pfi->linear[4];
  pfiv->dimoff[2] = izmin-1;
  pfiv->dimoff[3] = itmin-1;
  pfiv->dimoff[4] = iemin-1;
  pfiv->ppflag = 0;

  if (pgr1->idim>-1 && pgr1->jdim>-1) {             /* I and J are varying */
    pfiv->gr2ab[0] = pgr1->igrab;
    pfiv->ab2gr[0] = pgr1->iabgr;
    if ((pfiv->grvals[0] = cpscal (pgr1->ivals,  pgr1->ilinr, 0, pgr1->idim))==NULL) goto etrn;
    if ((pfiv->abvals[0] = cpscal (pgr1->iavals, pgr1->ilinr, 1, pgr1->idim))==NULL) goto etrn;
    pfiv->linear[0] = pgr1->ilinr;
    pfiv->dimoff[0] = pgr1->dimmin[0]-1;
    pfiv->dnum[0]   = pgr1->isiz;

    pfiv->gr2ab[1] = pgr1->jgrab;
    pfiv->ab2gr[1] = pgr1->jabgr;
    if ((pfiv->grvals[1] = cpscal (pgr1->jvals,  pgr1->jlinr, 0, pgr1->jdim))==NULL) goto etrn;
    if ((pfiv->abvals[1] = cpscal (pgr1->javals, pgr1->jlinr, 1, pgr1->jdim))==NULL) goto etrn;
    pfiv->linear[1] = pgr1->jlinr;
    pfiv->dimoff[1] = pgr1->dimmin[1]-1;
    pfiv->dnum[1]   = pgr1->jsiz;
  }
  else if (pgr1->idim > -1 && pgr1->jdim == -1) {   /* I is varying, J is fixed */
    if (pgr1->idim == 0) {           /* I is X */
      pfiv->gr2ab[0] = pgr1->igrab;
      pfiv->ab2gr[0] = pgr1->iabgr;
      if ((pfiv->grvals[0] = cpscal (pgr1->ivals,  pgr1->ilinr, 0, pgr1->idim))==NULL) goto etrn;
      if ((pfiv->abvals[0] = cpscal (pgr1->iavals, pgr1->ilinr, 1, pgr1->idim))==NULL) goto etrn;
      pfiv->linear[0] = pgr1->ilinr;
      pfiv->dimoff[0] = pgr1->dimmin[0]-1;
      pfiv->dnum[0]   = pgr1->isiz;

      pfiv->gr2ab[1] = pfi->gr2ab[1];
      pfiv->ab2gr[1] = pfi->ab2gr[1];
      if ((pfiv->grvals[1] = cpscal (pfi->grvals[1], pfi->linear[1], 0, 1))==NULL) goto etrn;
      if ((pfiv->abvals[1] = cpscal (pfi->abvals[1], pfi->linear[1], 1, 1))==NULL) goto etrn;
      pfiv->linear[1] = pfi->linear[1];
      pfiv->dimoff[1] = 0;
      pfiv->dnum[1]   = 1;
    } 
    else {                           /* I is Y */
      pfiv->gr2ab[1] = pgr1->igrab;
      pfiv->ab2gr[1] = pgr1->iabgr;
      if ((pfiv->grvals[1] = cpscal (pgr1->ivals,  pgr1->ilinr, 0, pgr1->idim))==NULL) goto etrn;
      if ((pfiv->abvals[1] = cpscal (pgr1->iavals, pgr1->ilinr, 1, pgr1->idim))==NULL) goto etrn;
      pfiv->linear[1] = pgr1->ilinr;
      pfiv->dimoff[1] = pgr1->dimmin[1]-1;
      pfiv->dnum[1]   = pgr1->isiz;

      pfiv->gr2ab[0] = pfi->gr2ab[0];
      pfiv->ab2gr[0] = pfi->ab2gr[0];
      if ((pfiv->grvals[0] = cpscal (pfi->grvals[0], pfi->linear[0], 0, 0))==NULL) goto etrn;
      if ((pfiv->abvals[0] = cpscal (pfi->abvals[0], pfi->linear[0], 1, 0))==NULL) goto etrn;
      pfiv->linear[0] = pfi->linear[0];
      pfiv->dimoff[0] = 0;
      pfiv->dnum[0]   = 1;
    }
  }
  else {                             /* I and J are fixed */
    pfiv->gr2ab[0] = pfi->gr2ab[0];
    pfiv->ab2gr[0] = pfi->ab2gr[0];
    if ((pfiv->grvals[0] = cpscal (pfi->grvals[0], pfi->linear[0], 0, 0))==NULL) goto etrn;
    if ((pfiv->abvals[0] = cpscal (pfi->abvals[0], pfi->linear[0], 1, 0))==NULL) goto etrn;
    pfiv->linear[0] = pfi->linear[0];
    pfiv->dimoff[0] = 0;
    pfiv->dnum[0]   = 1;
    
    pfiv->gr2ab[1] = pfi->gr2ab[1];
    pfiv->ab2gr[1] = pfi->ab2gr[1];
    if ((pfiv->grvals[1] = cpscal (pfi->grvals[1], pfi->linear[1], 0, 1))==NULL) goto etrn;
    if ((pfiv->abvals[1] = cpscal (pfi->abvals[1], pfi->linear[1], 1, 1))==NULL) goto etrn;
    pfiv->linear[1] = pfi->linear[1];
    pfiv->dimoff[1] = 0;
    pfiv->dnum[1]   = 1;
  }

  /* If the first grid is all the data we need, then we are pretty much done.  */

  if (izmin==izmax && itmin==itmax && iemin==iemax) {
    if (pgr1->idim<0) {   /* grid is a single data value */
      sz = sizeof(gadouble);
      pfiv->rbuf = (gadouble *)galloc(sz,"pfivrbuf");
      if (pfiv->rbuf==NULL) {
        gaprnt (0,"Define Error:  Unable to allocate data memory\n");
        goto retrn;
      }
      sz = sizeof(char);
      pfiv->ubuf = (char *)galloc(sz,"pfivubuf");
      if (pfiv->ubuf==NULL) {
        gaprnt (0,"Define Error:  Unable to allocate undef mask memory\n");
        goto retrn;
      }
      *(pfiv->rbuf) = *(pgr1->grid);
      *(pfiv->ubuf) = *(pgr1->umask);
    } else {
      pfiv->rbuf = pgr1->grid;
      pfiv->ubuf = pgr1->umask;
    }
    pgr1->grid = NULL;
    pgr1->umask = NULL;  
    siz = pgr1->isiz;
    siz = siz * pgr1->jsiz;
  } 
  else {
    /* We need to get multiple grids.  Set this up.  */

    /* Calculate size and then allocate the storage for the defined object */
    gsiz = pgr1->isiz*pgr1->jsiz;
    siz = (size_t)gsiz * (size_t)pfiv->dnum[2] * (size_t)pfiv->dnum[3] * (size_t)pfiv->dnum[4];
    sz = siz * sizeof(gadouble);
    pfiv->rbuf = (gadouble *)galloc(sz,"defnrbuf");
    if (pfiv->rbuf==NULL) {
      gaprnt (0,"Define Error:  Unable to allocate data memory\n");
      snprintf(pout,1255,"  Size of request was %ld grid elements\n",siz);
      gaprnt (0,pout);
      goto retrn;
    }
    sz = siz * sizeof(char);
    pfiv->ubuf = (char *)galloc(sz,"defnubuf");
    if (pfiv->ubuf==NULL) {
      gaprnt (0,"Define Error:  Unable to allocate memory for undef mask\n");
      snprintf(pout,1255,"  Size of request was %ld grid elements\n",siz);
      gaprnt (0,pout);
      goto retrn;
    }

    /* Now we can loop and get all the data */

    res  = pfiv->rbuf;
    resu = pfiv->ubuf;
    for (ie=iemin; ie<=iemax; ie++) {
      for (it=itmin; it<=itmax; it++) {
	for (iz=izmin; iz<=izmax; iz++) {

	  /* fix dmin and dmax values for Z, T, E dimensions */
	  conv = pfi->gr2ab[2];
	  pst->dmin[2] = conv(pfi->grvals[2],(gadouble)iz);
	  pst->dmax[2] = pst->dmin[2];
	  gr2t (pfi->grvals[3], (gadouble)it, &(pst->tmin));
	  pst->tmax = pst->tmin;
	  conv = pfi->gr2ab[4];
	  pst->dmin[4] = conv(pfi->grvals[4],(gadouble)ie);
	  pst->dmax[4] = pst->dmin[4]; 

	  /* reuse first grid evaluated above */
	  if (ie==iemin && it==itmin && iz==izmin ) {
	    gr  = pgr1->grid; 
	    gru = pgr1->umask;
	  }
	  else {
	    /* evaluate the expression again */
	    rc = gaexpr(cmd, pst);
	    if (!rc) rc = gaqsig();
	    if (rc) {
	      gaprnt (0,"DEFINE error:  Invalid expression. \n");
	      goto retrn;
	    }
	    pgr = pst->result.pgr;
	    if (pgr->idim!=pgr1->idim || pgr->jdim!=pgr1->jdim ||
		gagchk(pgr,pgr1,pgr1->idim) ||
		gagchk(pgr,pgr1,pgr1->jdim) ) {
	      gaprnt (0,"Define Error: Internal Logic Check 4\n");
	      goto retrn;
	    }
	    gr  = pgr->grid;
	    gru = pgr->umask;
	  }

	  /* copy the grid into defined variable space */
	  for (i=0; i<gsiz; i++) {
	    *res  = *gr;
	    *resu = *gru;
	    /* make sure defined vars have undef values, use pcm->undef */
	    if (*gru == 0) *res = pcm->undef; 
	    res++; resu++; 
	    gr++;  gru++;
	  }

	  /* free the result grid  */
	  if (ie==iemin && it==itmin && iz==izmin ) {
	    if (pgr1->idim>-1) {
	      gree(pgr1->grid,"f86a");
	      gree(pgr1->umask,"f87a");
	    }
	    pgr1->grid = NULL;
	    pgr1->umask = NULL;
	    pst->result.pgr = NULL;
	  }
	  else {
	    gafree (pst);
	  }
	}
      }
    }
  }

  siz = siz * sizeof(gadouble);
  snprintf(pout,1255,"Define memory allocation size = %ld bytes\n",siz);
  gaprnt (2,pout);

  /* Now we will chain our new object to the chain of define blocks
     hung off the common area */

  pcurr = pcm->pdf1;
  prev = &(pcm->pdf1);
  while (pcurr!=NULL) {
    if (cmpwrd(name,pcurr->abbrv)) {
      gaprnt (2,"Name already DEFINEd: '");
      gaprnt (2,name);
      gaprnt (2,"' will be deleted and replaced\n");
      pfic = pcurr->pfi;
      gree(pfic->rbuf,"f93");
      gree(pfic->ubuf,"f93a");
      for (i=0; i<5; i++) {
	gree(pfic->grvals[i],"f93b");
	gree(pfic->abvals[i],"f93c");
      }
      gree(pfic,"f94");
      *prev = pcurr->pforw;
      psave = pcurr;
      pcurr = pcurr->pforw;
      gree(psave,"f95");
      continue;
    }
    prev = &(pcurr->pforw);
    pcurr = pcurr->pforw;
  }

  *prev = pdf;          /* Chain it up */
  pdf->pforw = NULL;
  getwrd(pdf->abbrv,name,19);

  gree(pst,"f97");
  gree(pgr1,"f96"); 

  /* Restore user dim limits*/
  pcm->dmax[2] = zmax;
  pcm->tmax    = tmax;                    
  pcm->dmax[4] = emax;
  pcm->vdim[2] = vdz;
  pcm->vdim[3] = vdt;
  pcm->vdim[4] = vde;
  return (0);

etrn:
  gaprnt(0,"Memory allocation error for Define\n");

retrn:
  if (pst!=NULL) {
    gafree(pst);
    gree(pst,"f98");
  }
  if (pgr1!=NULL) {
    if (pgr1->grid!=NULL && pgr1->idim>-1) gree(pgr1->grid,"f208");
    gree(pgr1,"f99");
  }
  if (pdf!=NULL) gree(pdf,"f209");
  if (pfiv!=NULL) {
    if (pfiv->rbuf!=NULL) gree(pfiv->rbuf,"f210a");
    if (pfiv->ubuf!=NULL) gree(pfiv->ubuf,"f210b");
    gree(pfiv,"f100");
  }
  pcm->tmax = tmax;                    /* Restore user dim limits*/
  pcm->dmax[2] = zmax;
  pcm->vdim[2] = vdz;
  pcm->vdim[3] = vdt;
  return(1);
}

/* Handle query command */

char *gxnms[29] = {"Clear","Contour","Shaded","Vector","Grid","Fgrid",
		   "Line","Scatter","Bar","Stream","Value","Model","Wxsym","Line",
		   "StnMark","Barb","GrFill","LineFill","","","Fwrite","Findstn","Stat",
                   "GeoTIFF", "KML", "Shapefile","ImageMap","Shaded2","Shaded2b"};
/* messages for query gxout */
static char *gxout0D[3]   = {"Display", "Stat", "Print"};
static char *gxout1D[5]   = {"0", "Line", "Bar", "Errbar", "Linefill"};
static char *gxout1Da[3]  = {"0", "Vector", "Barb"};
static char *gxout2Da[20] = {"0", "Contour", "Shaded","Grid", "4", "5", "Fgrid",
			     "Fwrite", "8", "9", "Grfill", "Pgrid", "GeoTIFF", "KML",
			     "ImageMap","Shapefile","16","17","18","19"};
static char *gxout2Db[10] = {"0", "1", "2", "Grid", "Vector", "Scatter", "6", "7", "Stream", "Barb"};
static char *gxoutStn[10]  = {"0", "Value", "Barb", "Findstn", "Model", 
			      "Wxsym", "6", "Stnmark", "Stnwrite", "Shapefile"};

char *dweek[8] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat","???"};

gaint gaqury (char *cmd, char *ccc, struct gacmn *pcm) {
struct gafile *pfi;
struct gavar *pvar;
struct gadefn *pdf;
struct gaens *ens;
struct dt dtim;
struct xinfo xinf;
struct gdlg dlg; 
struct gaattr *attr;
struct gxdbquery dbq;
gadouble (*conv) (gadouble *, gadouble);
gadouble x, y, v, v1, v2, lon, lat, rinfo[10];
gaint i, j, cnt, fnum, flag, etype, info[10], fn;
gaint closethisfilelater=0, hdrflg, hdrflgd;
char *arg, lab[20], lab2[20], *ch, *name, *name2;
char *varnam;
short *sptr=NULL;
long *lptr=NULL;
gafloat *fptr=NULL;
gadouble *dptr=NULL;
size_t sz;
#if (USENETCDF == 1 || USEHDF == 1)
struct dt tdef, tdefi;
char *tfile, *tfile2;
gaint ncid=0, rc, error, n_atts, n_gatts;
gaint sdid=0;
#endif
#if USEHDF5 == 1
gaint h5id=-999;
hid_t fid=0;
#endif
#if USESHP==1
SHPHandle shpid=NULL;
SHPObject *shp=NULL;
DBFHandle dbfid=NULL;
struct dbfld *fld,*fld1;
char shparg[4096];
int fcnt,rcnt,shptype,shpcnt, ival;
gadouble minvals[4], maxvals[4],dval;
#endif

  arg = nxtwrd(cmd);
  if (arg == NULL) {
    gaprnt (2,"query or q Options: \n");
    gaprnt (2,"  q attr     Returns global and variable attributes for an open file\n");
    gaprnt (2,"  q cache    Returns netcdf4/hdf5 cache size for a particular file \n");
    gaprnt (2,"  q cachesf  Returns global cache scale factor\n");
    gaprnt (2,"  q config   Returns GrADS configuration information\n");
    gaprnt (2,"  q ctlinfo  Returns contents of data descriptor file\n");
    gaprnt (2,"  q contours Returns colors and levels of line contours\n");
    gaprnt (2,"  q dbf      Lists contents of a shapefile attribute database\n");
    gaprnt (2,"  q define   Lists currently defined variables\n");
    gaprnt (2,"  q defval   Returns value of a defined variable at a point\n");
    gaprnt (2,"  q dialog   Launches a dialog box\n");
    gaprnt (2,"  q dims     Returns current dimension environment\n");
    gaprnt (2,"  q ens      Returns a list of ensemble members\n");
    gaprnt (2,"  q fgvals   Returns values and colors specified for gxout fgrid\n");
    gaprnt (2,"  q file     Returns info about a particular file\n");
    gaprnt (2,"  q files    Lists all open files\n");
    gaprnt (2,"  q font     Returns info about a particular font \n");
    gaprnt (2,"  q fwrite   Returns status of fwrite output file\n");
    gaprnt (2,"  q gxinfo   Returns graphics environment info\n");
    gaprnt (2,"  q gxout    Gives current gxout settings\n");
    gaprnt (2,"  q pos      Waits for mouse click, returns position and widget info\n");
    gaprnt (2,"  q sdfwrite Returns status of self-describing fwrite options\n");
    gaprnt (2,"  q shades   Returns colors and levels of shaded contours\n");
    gaprnt (2,"  q shp      Lists the contents of a shapefile\n");
    gaprnt (2,"  q shpopts  Returns settings for drawing and writing shapefiles\n");
    gaprnt (2,"  q string   Returns width of a string\n");
    gaprnt (2,"  q time     Returns info about time settings\n");
    gaprnt (2,"  q udpt     Returns list of user defined plug-ins\n");
    gaprnt (2,"  q undef    Returns output undef value \n");
    gaprnt (2,"  q xinfo    Returns characteristics of graphics display window\n");
    gaprnt (2,"  q xy2w     Converst XY screen to world coordinates\n");
    gaprnt (2,"  q xy2gr    Converts XY screen to grid coordinates\n");
    gaprnt (2,"  q w2xy     Converts world to XY screen coordinates\n");
    gaprnt (2,"  q w2gr     Converts world to grid coordinates\n");
    gaprnt (2,"  q gr2w     Converts grid to world coordinates\n");
    gaprnt (2,"  q gr2xy    Converts grid to XY screen coordinates\n");
    gaprnt (2,"  q pp2xy    Converts virtual page XY to real page XY coordinates\n");
    gaprnt (2,"Details about argument syntax for some of these options are in the \n");
    gaprnt (2,"online documentation: http://cola.gmu.edu/grads/gadoc/gradcomdquery.html\n");
  }
  else if (cmpwrd(arg,"mem")) {
    glook();
  }
  else if (cmpwrd(arg,"bcol")) {
    snprintf(pout,1255,"Background color is %d\n",gxdbkq());
    gaprnt(2,pout);
  }
  else if (cmpwrd(arg,"lcol")) {
    snprintf(pout,1255,"Current line color is %d\n",gxqclr());
    gaprnt(2,pout);
    snprintf(pout,1255,"pcm->lincol is %d\n",pcm->lincol);
    gaprnt(2,pout);
  }
  else if (cmpwrd(arg,"col")) {
    /* Check if user provided a color number */
    if ((arg = nxtwrd (arg)) == NULL ) fn = -1;
    else {
      if (intprs(arg,&fn) == NULL ) fn = -1; 
    }
    if (fn > -1) {
      /* get color info from graphics database */
      /* get rgb values for this color */
      gxdbqcol(fn, &dbq);
      snprintf(pout,1255,"Color #%d  R,G,B,A = %3d %3d %3d %3d\n",fn,dbq.red,dbq.green,dbq.blue,dbq.alpha);
      gaprnt(2,pout);
    }
  }
  else if (cmpwrd(arg,"undef")) {
    snprintf(pout,1255,"Output undef value is set to %12f\n",pcm->undef);
    gaprnt(2,pout);
  }
  else if (cmpwrd(arg,"dbuff")) {
    if (pcm->dbflg) snprintf(pout,1255,"double buffering is on\n");
    else snprintf(pout,1255,"double buffering is off\n");
    gaprnt(2,pout);
  }
  else if (cmpwrd(arg,"calendar")) {
    if (mfcmn.cal365==-999) snprintf(pout,1255,"calendar mode not yet set\n");
    else if (mfcmn.cal365==1) snprintf(pout,1255,"365-day calendar in effect\n");
    else snprintf(pout,1255,"standard calendar in effect\n");
    gaprnt(2,pout);
  }
  else if (cmpwrd(arg,"font")) {
    /* Check if user provided a font number, otherwise use use current default font */
    if ((arg = nxtwrd (arg)) == NULL) {
      fn = gxqdf();
    } else {
      if (intprs(arg,&fn) == NULL ) {
        gaprnt (0,"Invalid QUERY FONT argument -- it should be a number from 0 to 99.\n");
        return (1);
      }
      if (fn<0 || fn>99) {
        gaprnt (0,"Invalid QUERY FONT argument -- it should be a number from 0 to 99.\n");
        return (1);
      }
    }
    snprintf(pout,1255,"Font %d: ",fn); gaprnt(2,pout);

    /* get font info from graphics database */
    gxdbqfont (fn, &dbq);
    if (fn<=5) {
      if (fn==3) { 
	gaprnt(2,"hershey symbol \n");
      } 
      else {
	if (gxdbqhersh()) { 
	  gaprnt(2,"generic ");
	} else {
	  gaprnt(2,"hershey ");
	}
	snprintf(pout,1255,"%s ",dbq.fname); gaprnt(2,pout);
	if (dbq.fbold) gaprnt(2,"bold ");
	if (dbq.fitalic) gaprnt(2,"italic ");
	gaprnt(2,"\n");
      }
    } 
    else if (fn>5 && fn<10) {
      gaprnt(2,"user-defined \n");
    } 
    else  {
      snprintf(pout,1255,"%s \n",dbq.fname); gaprnt(2,pout);
    }
  }
  else if (cmpwrd(arg,"cachesf")) {
    snprintf(pout,1255,"Global cache scale factor is %ld\n",pcm->cachesf);
    gaprnt(2,pout);
  }
  else if (cmpwrd(arg,"cache")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if ((arg = nxtwrd (arg)) == NULL) {
      /* no file number given, use the default file */
      pfi = pcm->pfid;
      fnum = pcm->dfnum;
    } else {
      /* get the file number */
      if (intprs(arg,&fnum) == NULL ) {
        gaprnt(0,"QUERY CACHE Error: invalid argument \n");
        return (1);
      }
      pfi = pcm->pfi1;
      for (i=0; i<fnum-1; i++) {
        pfi = pfi->pforw;
        if (pfi==NULL) {
          snprintf(pout,1255,"QUERY CACHE Error: file %i not open\n",fnum);
          gaprnt (0,pout);
          return(1);
        }
      }
    }
    snprintf(pout,1255,"File %i cache size in bytes: %ld \n",fnum,pfi->cachesize);
    gaprnt(2,pout);
  }
  else if (cmpwrd(arg,"dialog")) {
    if ((arg = nxtwrd (arg)) == NULL) goto errdl;
    if (getdbl(arg,&(dlg.x)) == NULL ) goto dialog;
    if ((arg = nxtwrd (arg)) == NULL) goto errdl;
    if (getdbl(arg,&(dlg.y)) == NULL ) goto errdl;
    if ((arg = nxtwrd (arg)) == NULL) goto errdl;
    if (getdbl(arg,&(dlg.w)) == NULL ) goto errdl;
    if ((arg = nxtwrd (arg)) == NULL) goto errdl;
    if (getdbl(arg,&(dlg.h)) == NULL ) goto errdl;
    dlg.pc = pcm->dlgpc;
    dlg.fc = pcm->dlgfc;
    dlg.bc = pcm->dlgbc;
    dlg.oc = pcm->dlgoc;
    dlg.th = pcm->dlgth;
    dlg.nu = pcm->dlgnu;

    if ((arg = nxtwrd (arg)) == NULL) goto errdl;
    dlg.len = 0;
    while (*(arg+dlg.len)) dlg.len++;
    sz = dlg.len+1;
    dlg.ch = (char *)galloc(sz,"dlg1");
    *(dlg.ch+dlg.len) = '\0';
    if (dlg.ch==NULL) {
      gaprnt(0,"Memory allocation error; QUERY DIALOG cmd\n");
     return(1);
    }
    for (i=0; i<dlg.len; i++) *(dlg.ch+i) = *(ccc+(arg-cmd)+i);
    ch = dsubs->gxdlg(&dlg);
    if (ch!=NULL) {
      gaprnt (2,ch);
      gree(ch,"f211");
    }
    return (0);

    errdl:
    gaprnt (0,"QUERY error: Syntax is QUERY DIALOG x y w h ");
    gaprnt (0,"string\n");
    return (1);
    
    dialog:
    dlg.x = -1;
    dlg.y = -1;
    dlg.w = -1;
    dlg.h = -1;
    dlg.pc = pcm->dlgpc;
    dlg.fc = pcm->dlgfc;
    dlg.bc = pcm->dlgbc;
    dlg.oc = pcm->dlgoc;
    dlg.th = pcm->dlgth;
    dlg.nu = pcm->dlgnu;

    dlg.len = 0;
    while (*(arg+dlg.len)) dlg.len++;
    sz = dlg.len+1;
    dlg.ch = (char *)galloc(sz,"dlg2");
    *(dlg.ch+dlg.len) = '\0';
    if (dlg.ch==NULL) {
      gaprnt(0,"Memory allocation error; QUERY DIALOG cmd\n");
      return(1);
    }
    for (i=0; i<dlg.len; i++) *(dlg.ch+i) = *(ccc+(arg-cmd)+i);
    ch = dsubs->gxdlg(&dlg);
    if (ch!=NULL) {
      gaprnt (2,ch);
      gree(ch,"f212");
    }
    return (0);
  }
  else if (cmpwrd(arg,"ens")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if ((arg = nxtwrd (arg)) == NULL) {
      pfi = pcm->pfid;
      fnum = pcm->dfnum;
    } else {
      if (intprs(arg,&fnum) == NULL ) {
        i = 0;
        while (*arg!=' ' && *arg!='\0' && *arg!='\n' && i<19) {
          lab[i] = *arg;
          arg++;
          i++;
        }
        lab[i] = '\0';
        snprintf(pout,1255,"Invalid QUERY ENS argument: %s \n",lab);
        gaprnt (0,pout);
        return (1);
      }
      pfi = pcm->pfi1;
      for (i=0; i<fnum-1; i++) {
        pfi = pfi->pforw;
        if (pfi==NULL) {
          snprintf(pout,1255,"QUERY ENS Error: file %i not open\n",fnum);
          gaprnt (0,pout);
          return(1);
        }
      }
    }
    ens = pfi->ens1;
    i=0;
    while (i<pfi->dnum[4]) {
      gat2ch(&(ens->tinit),4,lab,20);
      snprintf(pout,1255,"Ensemble %d named %s has %d timesteps and begins at %s (t=%d) ",
	     i+1, ens->name, ens->length, lab, ens->gt );
      gaprnt(2,pout);
      if (ens->grbcode[0]>-900) {
	if (ens->grbcode[1]>-900) 
	  snprintf(pout,1255,"grbcode=%d,%d",ens->grbcode[0],ens->grbcode[1]);
	else
	  snprintf(pout,1255,"grbcode=%d",ens->grbcode[0]);
	gaprnt(2,pout);
      }
      gaprnt(2,"\n");
      
      i++; ens++;
    }
  }
  /* this command is used by the GDS -- formatted specifically */
  else if (cmpwrd(arg,"ens_name")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if ((arg = nxtwrd (arg)) == NULL) {
      pfi = pcm->pfid;
      fnum = pcm->dfnum;
    } else {
      if (intprs(arg,&fnum) == NULL ) {
        i = 0;
        while (*arg!=' ' && *arg!='\0' && *arg!='\n' && i<19) {
          lab[i] = *arg;
          arg++;
          i++;
        }
        lab[i] = '\0';
        snprintf(pout,1255,"Invalid QUERY ENS_NAME argument: %s \n",lab);
        gaprnt (0,pout);
        return (1);
      }
      pfi = pcm->pfi1;
      for (i=0; i<fnum-1; i++) {
        pfi = pfi->pforw;
        if (pfi==NULL) {
          snprintf(pout,1255,"QUERY ENS_NAME Error: file %i not open\n",fnum);
          gaprnt (0,pout);
          return(1);
        }
      }
    }
    ens = pfi->ens1;
    i=0;
    gaprnt(2,"ens String grads_name ");
    while (i<pfi->dnum[4]) {
      snprintf(pout,1255,"%s",ens->name);
      gaprnt(2,pout);
      if (i<pfi->dnum[4]-1) gaprnt(2,",");
      i++; ens++;
    }
    gaprnt(2,"\n");
  }
  /* this command is used by the GDS -- formatted specifically */
  else if (cmpwrd(arg,"ens_length")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if ((arg = nxtwrd (arg)) == NULL) {
      pfi = pcm->pfid;
      fnum = pcm->dfnum;
    } else {
      if (intprs(arg,&fnum) == NULL ) {
        i = 0;
        while (*arg!=' ' && *arg!='\0' && *arg!='\n' && i<19) {
          lab[i] = *arg;
          arg++;
          i++;
        }
        lab[i] = '\0';
        snprintf(pout,1255,"Invalid QUERY ENS_LENGTH argument: %s \n",lab);
        gaprnt (0,pout);
        return (1);
      }
      pfi = pcm->pfi1;
      for (i=0; i<fnum-1; i++) {
        pfi = pfi->pforw;
        if (pfi==NULL) {
          snprintf(pout,1255,"QUERY ENS_LENGTH Error: file %i not open\n",fnum);
          gaprnt (0,pout);
          return(1);
        }
      }
    }
    ens = pfi->ens1;
    i=0;
    gaprnt(2,"ens String grads_length ");
    while (i<pfi->dnum[4]) {
      snprintf(pout,1255,"%d",ens->length);
      gaprnt(2,pout);
      if (i<pfi->dnum[4]-1) gaprnt(2,",");
      i++; ens++;
    }
    gaprnt(2,"\n");
  }
  /* this command is used by the GDS -- formatted specifically */
  else if (cmpwrd(arg,"ens_tinit")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if ((arg = nxtwrd (arg)) == NULL) {
      pfi = pcm->pfid;
      fnum = pcm->dfnum;
    } else {
      if (intprs(arg,&fnum) == NULL ) {
        i = 0;
        while (*arg!=' ' && *arg!='\0' && *arg!='\n' && i<19) {
          lab[i] = *arg;
          arg++;
          i++;
        }
        lab[i] = '\0';
        snprintf(pout,1255,"Invalid QUERY ENS_TINIT argument: %s \n",lab);
        gaprnt (0,pout);
        return (1);
      }
      pfi = pcm->pfi1;
      for (i=0; i<fnum-1; i++) {
        pfi = pfi->pforw;
        if (pfi==NULL) {
          snprintf(pout,1255,"QUERY ENS_TINIT Error: file %i not open\n",fnum);
          gaprnt (0,pout);
          return(1);
        }
      }
    }
    ens = pfi->ens1;
    i=0;
    gaprnt(2,"ens String grads_tinit ");
    while (i<pfi->dnum[4]) {
      snprintf(pout,1255,"%d",ens->gt);
      gaprnt(2,pout);
      if (i<pfi->dnum[4]-1) gaprnt(2,",");
      i++; ens++;
    }
    gaprnt(2,"\n");
  }
  else if (cmpwrd(arg,"vars")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if ((arg = nxtwrd (arg)) == NULL) {
      pfi = pcm->pfid;
      fnum = pcm->dfnum;
    } else {
      if (intprs(arg,&fnum) == NULL ) {
        i = 0;
        while (*arg!=' ' && *arg!='\0' && *arg!='\n' && i<19) {
          lab[i] = *arg;
          arg++;
          i++;
        }
        lab[i] = '\0';
        snprintf(pout,1255,"Invalid QUERY VARS argument: %s \n",lab);
        gaprnt (0,pout);
        return (1);
      }
      pfi = pcm->pfi1;
      for (i=0; i<fnum-1; i++) {
        pfi = pfi->pforw;
        if (pfi==NULL) {
          snprintf(pout,1255,"QUERY VARS Error: file %i not open\n",fnum);
          gaprnt (0,pout);
          return(1);
        }
      }
    }
    pvar = pfi->pvar1;
    j = 0;
    while (j<pfi->vnum) {
      printf("%s\n",pvar->abbrv);
      printf(" description: %s\n",pvar->varnm);
      printf(" levels: %d ",pvar->levels); printf("\n");
      printf(" units: "); for (i=0;i<16;i++) printf("%-4g ",pvar->units[i]); printf("\n");
      printf(" vecpair=%d \n", pvar->vecpair);
      printf(" isu=%d \n", pvar->isu);
      printf(" offset=%lld \n",pvar->offset);
      printf(" recoff=%d \n",pvar->recoff);
      printf(" dfrm=%d \n",pvar->dfrm);
      printf(" var_t=%d \n",pvar->var_t);
      if (pfi->ncflg) {
	printf(" %d dimids: ",pvar->nvardims);
        for (i=0;i<pvar->nvardims;i++) printf("%-4d ",pvar->vardimids[i]); printf("\n");
	if (pfi->ncflg==1) printf(" ncvid=%d \n",pvar->ncvid);
	if (pfi->ncflg==2) printf(" sdvid=%d \n",pvar->sdvid);
	if (pfi->ncflg==3) printf(" h5vid=%d \n",pvar->h5vid);
	printf(" scale=%f \n",pvar->scale);
	printf(" add=%f \n",pvar->add);
      }
      printf("\n");
      pvar++; j++;
    }	
  }
  else if (cmpwrd(arg,"shpopts"))  {
#if USESHP==1
    snprintf(pout,1255,"Settings for drawing shapefiles:\n polygon fill color: %i \n",pcm->fillpoly);
    gaprnt (2,pout);
    snprintf(pout,1255," mark type: %i \n",pcm->marktype);
    gaprnt (2,pout);
    snprintf(pout,1255," mark size: %g \n",pcm->marksize);
    gaprnt (2,pout);
    snprintf(pout,1255,"Settings for writing shapefiles:\n");
    gaprnt (2,pout);
    if (pcm->shpfname)
      snprintf(pout,1255," output filename root: %s\n",pcm->shpfname);
    else 
      snprintf(pout,1255," output filename root: grads\n");
    gaprnt (2,pout);
    if (pcm->shptype==1) gaprnt(2," output type: point\n");
    if (pcm->shptype==2) gaprnt(2," output type: line\n");
    snprintf(pout,1255," format string: \%%%d.%df\n",pcm->dblen,pcm->dbprec);
    gaprnt(2,pout);
    /* print the user-provided fields that are linked off of gacmn  */
    if (pcm->dbfld) { 
      gaprnt(2," attributes:\n");
      fld = pcm->dbfld;  
      while (fld) {
	snprintf(pout,1255,"  %s: ",fld->name);
	gaprnt(2,pout);
	if (fld->type == FTInteger) {
	  intprs(fld->value,&ival);
	  snprintf(pout,1255,"%d \n",ival); 
	} 
	else if (fld->type == FTDouble) {
	  getdbl(fld->value,&dval);
          snprintf(lab,20,"\%%-%d.%df\n",pcm->dblen,pcm->dbprec);
	  snprintf(pout,1255,lab,dval); 
	} 
	else {
	  snprintf(pout,1255,"%s\n",(char*)fld->value); 
	}
	gaprnt(2,pout);
	fld = fld->next;
      } 
    }
#else
  gaprnt(0,"This build does not support shapefiles\n");
#endif
  }
  else if (cmpwrd(arg,"shp"))  {
#if USESHP==1
    if ((arg = nxtwrd (arg)) == NULL) {
      gaprnt(0,"Query Error: Missing shapefile name \n");
    } 
    else {
      getwrd(shparg,ccc+(arg-cmd),4095);      /* use mixed-case version */
      shpid = gaopshp(shparg);
      if (shpid) {
	SHPGetInfo (shpid, &shpcnt, &shptype, minvals, maxvals);
	snprintf(pout,1255,"Shapefile Type=%s #Shapes=%d XBounds=%g:%g YBounds=%g:%g\n", 
		SHPTypeName(shptype),shpcnt,minvals[0],maxvals[0],minvals[1],maxvals[1]);
	gaprnt(2,pout);
	for (i=0; i<shpcnt; i++) {
	  shp = NULL;
	  if ((shp = SHPReadObject (shpid,i))!=NULL) { 
	    snprintf(pout,1255,"%d:  %s  parts=%d  vertices=%d  ",
		    shp->nShapeId,SHPTypeName(shp->nSHPType),shp->nParts,shp->nVertices);
	    gaprnt(2,pout);
	    snprintf(pout,1255,"XBounds=%g:%g  ",shp->dfXMin,shp->dfXMax); gaprnt(2,pout);
	    snprintf(pout,1255,"YBounds=%g:%g  ",shp->dfYMin,shp->dfYMax); gaprnt(2,pout);
	    snprintf(pout,1255,"ZBounds=%g:%g  ",shp->dfZMin,shp->dfZMax); gaprnt(2,pout);
	    snprintf(pout,1255,"MBounds=%g:%g \n",shp->dfMMin,shp->dfMMax); gaprnt(2,pout);
	    SHPDestroyObject (shp);
	  }
	}
	SHPClose(shpid);
      }
    }
#else
  gaprnt(0,"This build does not support querying shapefiles\n");
#endif
  }
  else if (cmpwrd(arg,"dbf")) {
#if USESHP==1
    char delimiter[4];
    sprintf(delimiter,","); /* the default delimiter is a comma */

    if ((arg = nxtwrd (arg)) == NULL) {
      gaprnt(0,"Query Error: Missing shapefile name \n");
    } 
    else {
      getwrd(shparg,ccc+(arg-cmd),4095);   /* use mixed-case version */
      if ((arg = nxtwrd (arg)) != NULL) {
        /* check for extra argument, a different delimiter (in case the DBF fields contain commas) */
	getwrd(delimiter,ccc+(arg-cmd),3);
      }
      dbfid = gaopdbf(shparg);
      if (dbfid) {
	fcnt = DBFGetFieldCount (dbfid);
	rcnt = DBFGetRecordCount (dbfid);
	snprintf(pout,1255,"RECORD#%s",delimiter);
	gaprnt(2,pout);
	fld = NULL;
	if ((fld = (struct dbfld *)galloc(fcnt*sizeof(struct dbfld),"dbfld"))==NULL) {
	  gaprnt(0,"failed to allocate memory for dbf fields \n");
	}
	else {
	  fld1 = fld; 
	  i=0;
	  /* print out the attribute names */
	  while (i<fcnt) {
	    fld->type = DBFGetFieldInfo (dbfid, i, fld->name, &fld->len, &fld->prec);
	    snprintf(pout,1255,"%s",fld->name);
	    gaprnt(2,pout);
/* 	    if (i<fcnt-1) gaprnt(2,","); */
            snprintf(pout,1255,"%s",delimiter);
	    if (i<fcnt-1) gaprnt(2,pout);
	    i++; fld++;
	  }
	  gaprnt(2,"\n");
	  gree(fld1,"aa1");
	  /* print out the attribute values for each record */
	  for (i=0; i<rcnt; i++) {
/* 	    snprintf(pout,1255,"%d,",i); */
	    snprintf(pout,1255,"%d%s",i,delimiter);
	    gaprnt(2,pout);
	    for (j=0; j<fcnt; j++) {
	      if (DBFIsAttributeNULL(dbfid,i,j)) {
		gaprnt(2,"(NULL)");
	      }
	      else {
		snprintf(pout,1255,"%s",DBFReadStringAttribute(dbfid,i,j));
		gaprnt(2,pout);
	      }
/* 	      if (j<fcnt-1) gaprnt(2,","); */
              snprintf(pout,1255,"%s",delimiter);
	      if (j<fcnt-1) gaprnt(2,pout);
	    }
	    gaprnt(2,"\n");
	  }
	}
	DBFClose(dbfid);
      }
    }
#else
  gaprnt(0,"This build does not support querying shapefiles\n");
#endif
  }
  else if (cmpwrd(arg,"string")) {
    if ((arg = nxtwrd (arg)) == NULL) {
      gaprnt(0,"Query String Error: Missing String\n");
    } else {
      i = 0;
      while (*(arg+i)!='\0' && *(arg+i)!='\n') i++;
      v = 0.2;
      gxchln (ccc+(arg-cmd),i,pcm->strhsz,&v);
      snprintf(pout,1255,"String Width = %g\n",v);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd(arg,"fwrite")) {
    if (pcm->ffile) gaprnt(2,"FWrite file is open\n");
    else gaprnt(2,"FWrite file is closed\n");
    if (pcm->fwname) {
      gaprnt (2,"FWrite file name is: ");
      gaprnt (2,pcm->fwname);
      gaprnt (2,"\n");
    } else gaprnt (2,"FWrite file name is: grads.fwrite\n");
    if (pcm->fwenflg==0) {
      gaprnt (2,"FWrite byte order is little_endian ");
    } else {
      gaprnt (2,"FWrite byte order is big_endian ");
    }
    if (BYTEORDER==0) {
      gaprnt (2,"; machine byte order is little_endian\n");
    } else {
      gaprnt (2,"; machine byte order is big_endian\n");
    }
    snprintf(pout,1255,"Fwrite output undef value is set to %12f\n",pcm->undef);
    gaprnt(2,pout);
  }
  else if (cmpwrd(arg,"sdfwrite")) {
    if (pcm->sdfwname) {
      gaprnt (2,"SDFwrite file name is: ");
      gaprnt (2,pcm->sdfwname);
      gaprnt (2,"\n");
    } else gaprnt (2,"SDFwrite file name is: grads.sdfwrite.nc\n");
    gaprnt(2,"SDFwrite format is ");
    if (pcm->sdfzip==1) gaprnt (2,"compressed ");
    if (pcm->sdfwtype==1) gaprnt (2,"NetCDF ");
    else if (pcm->sdfwtype==2) gaprnt (2,"NetCDF-4 ");
    if (pcm->sdfprec==8) gaprnt (2,"double precision ");
    else if (pcm->sdfprec==4) gaprnt (2,"floating point precision ");
    gaprnt(2,"\n");
    if (pcm->sdfchunk || pcm->sdfzip) {
      gaprnt(2,"SDFWrite output chunk dimensions: ");
      if (pcm->xchunk) snprintf(pout,1255,"%d ",pcm->xchunk);
      else snprintf(pout,1255,"Xsize ");
      gaprnt(2,pout);
      if (pcm->ychunk) snprintf(pout,1255,"%d ",pcm->ychunk);
      else snprintf(pout,1255,"Ysize ");
      gaprnt(2,pout);
      snprintf(pout,1255,"%d ",pcm->zchunk ? pcm->zchunk : 1 );
      gaprnt(2,pout);
      snprintf(pout,1255,"%d ",pcm->tchunk ? pcm->echunk : 1 );
      gaprnt(2,pout);
      snprintf(pout,1255,"%d\n",pcm->echunk ? pcm->echunk : 1 );
      gaprnt(2,pout);
    }
    snprintf(pout,1255,"SDFwrite output undef value is set to %12f\n",pcm->undef);
    gaprnt(2,pout);
    /* information about dimensions */
    if (pcm->sdfwpad==0)
      gaprnt (2,"SDFwrite file will have the same number of dimensions as the defined variable\n");
    if (pcm->sdfwpad==1)
      gaprnt (2,"SDFwrite file will have 3 dimensions: lon, lat, and lev\n");
    if (pcm->sdfwpad==2)
      gaprnt (2,"SDFwrite file will have 3 dimensions: lon, lat, and time\n");
    if (pcm->sdfwpad==3) 
      gaprnt (2,"SDFwrite file will have 4 dimensions: lon, lat, lev, and time\n");
    if (pcm->sdfwpad==4)
      gaprnt (2,"SDFwrite file will have 4 dimensions: lon, lat, time, and ens\n");
    if (pcm->sdfwpad==5)
      gaprnt (2,"SDFwrite file will have 5 dimensions: lon, lat, lev, time, and ens\n");
    /* including the record dimension */
    if (pcm->sdfrecdim==1) 
      gaprnt (2,"SDFwrite file will have 1 record dimension: time\n");
    else if (pcm->sdfrecdim==2)
      gaprnt (2,"SDFwrite file will have 1 record dimension: ens\n");
    else if (pcm->sdfrecdim==3)
      gaprnt (2,"SDFwrite file will have 2 record dimensions: time and ens\n");
    else
      gaprnt (2,"SDFwrite file will not have a record dimension\n");

    if (pcm->attr) {
      attr=pcm->attr;
      gaprnt (2,"SDFwrite attribute metadata:\n");
      while (attr) {
	/* print strings */
	if (attr->nctype <= 2) {
	  snprintf(pout,1255,"  %s %s %s %s\n",
		  attr->varname,attr->type,attr->name,(char*)attr->value);
	    gaprnt(2,pout);
	} 
	else {
	  snprintf(pout,1255,"  %s %s %s ",attr->varname,attr->type,attr->name);
	  gaprnt(2,pout);
	  if      (attr->nctype == 3) sptr = (short*)attr->value;
	  else if (attr->nctype == 4) lptr = (long*)attr->value;
	  else if (attr->nctype == 5) fptr = (gafloat*)attr->value;
	  else if (attr->nctype == 6) dptr = (gadouble*)attr->value;
	  for (i=0; i<attr->len; i++) {
	    /* print numbers */
	    if (attr->nctype == 3) {
	      snprintf(pout,1255,"%i",(gaint)*(sptr));
	      gaprnt(2,pout);
	      sptr++;
	    } else if (attr->nctype == 4) {
	      snprintf(pout,1255,"%li",*(lptr));
	      gaprnt(2,pout);
	      lptr++;
	    } else if (attr->nctype == 5) {
	      snprintf(pout,1255,"%f",*(fptr));
	      gaprnt(2,pout);
	      fptr++;
	    } else { 
	      snprintf(pout,1255,"%g",*(dptr));
	      gaprnt(2,pout);
	      dptr++;
	    }
	    if (i != attr->len-1) {
	      snprintf(pout,1255,",");
	      gaprnt(2,pout);
	    }
	  }
	  snprintf(pout,1255,"\n");
	  gaprnt(2,pout);
	}
	attr=attr->next;
      }
    }
  }

  /* Configuration options */
  else if (cmpwrd(arg,"config")) {
    gacfg(2);
  }
  else if (cmpwrd(arg,"gxconfig")) {
    gxcfg(pcm->gxdopt, pcm->gxpopt);
  }

  else if (cmpwrd(arg,"dims")||cmpwrd(arg,"dim")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No files open\n");
      return (1);
    }
    pfi = pcm->pfid;
    snprintf(pout,1255,"Default file number is: %i \n",pcm->dfnum);
    gaprnt (2,pout);
    /* Longitude */
    if (pfi->type==2) {
      v1 = pcm->dmin[0];
      v2 = pcm->dmax[0];
    } else {
      conv = pfi->ab2gr[0];
      v1 = conv(pfi->abvals[0],pcm->dmin[0]);
      v2 = conv(pfi->abvals[0],pcm->dmax[0]);
    }
    if (pcm->dmin[0]==pcm->dmax[0]) {
      snprintf(pout,1255,"X is fixed     Lon = %g  X = %g\n",pcm->dmin[0],v1);
    } else {
      snprintf(pout,1255,"X is varying   Lon = %g to %g   X = %g to %g\n",
	       pcm->dmin[0],pcm->dmax[0],v1,v2);
    }
    gaprnt (2,pout);
    /* Latitude  */
    if (pfi->type==2) {
      v1 = pcm->dmin[1];
      v2 = pcm->dmax[1];
    } else {
      conv = pfi->ab2gr[1];
      v1 = conv(pfi->abvals[1],pcm->dmin[1]);
      v2 = conv(pfi->abvals[1],pcm->dmax[1]);
    }
    if (pcm->dmin[1]==pcm->dmax[1]) {
      snprintf(pout,1255,"Y is fixed     Lat = %g  Y = %g\n",pcm->dmin[1],v1);
    } else {
      snprintf(pout,1255,"Y is varying   Lat = %g to %g   Y = %g to %g\n",
           pcm->dmin[1],pcm->dmax[1],v1,v2);
    }
    gaprnt (2,pout);
    /* Level */
    if (pfi->type==2) {
      v1 = pcm->dmin[2];
      v2 = pcm->dmax[2];
    } else {
      conv = pfi->ab2gr[2];
      v1 = conv(pfi->abvals[2],pcm->dmin[2]);
      v2 = conv(pfi->abvals[2],pcm->dmax[2]);
    }
    if (pcm->dmin[2]==pcm->dmax[2]) {
      snprintf(pout,1255,"Z is fixed     Lev = %g  Z = %g\n",pcm->dmin[2],v1);
    } else {
      snprintf(pout,1255,"Z is varying   Lev = %g to %g   Z = %g to %g\n",
           pcm->dmin[2],pcm->dmax[2],v1,v2);
    }
    gaprnt (2,pout);
    /* Time */
    v1 = t2gr(pfi->abvals[3],&(pcm->tmin));
    v2 = t2gr(pfi->abvals[3],&(pcm->tmax));
    if (pcm->tmin.mn==0) gat2ch(&(pcm->tmin),4,lab,20);
    else gat2ch (&(pcm->tmin),5,lab,20);
    if (pcm->tmax.mn==0) gat2ch(&(pcm->tmax),4,lab2,20);
    else gat2ch (&(pcm->tmax),5,lab2,20);
    if (v1==v2) {
      snprintf(pout,1255,"T is fixed     Time = %s  T = %g\n",lab,v1);
    } else {
      snprintf(pout,1255,"T is varying   Time = %s to %s  T = %g to %g\n",
                  lab,lab2,v1,v2);
    }
    gaprnt (2,pout);
    /* Ensemble */
    v1=pcm->dmin[4];
    v2=pcm->dmax[4];
    if (v1==v2) {
      name = e2ens(pfi,v1);
      snprintf(pout,1255,"E is fixed     Ens = %s  E = %g\n",name,v1);
    } else {
      name = e2ens(pfi,v1);
      name2 = e2ens(pfi,v2);
      snprintf(pout,1255,"E is varying   Ens = %s to %s  E = %g to %g\n",name,name2,v1,v2);
    }
    gaprnt (2,pout);
  }

  else if (cmpwrd(arg,"w2xy") || cmpwrd(arg,"w2gr")) {
    if (pcm->xdim<0 || pcm->ydim<0) {
      gaprnt (2,"No scaling environment\n");
      return (0);
    }
    flag = 0;
    if (cmpwrd(arg,"w2gr")) flag = 1;
    if ((arg = nxtwrd (arg)) == NULL) goto errw;
    if (pcm->xdim == 3) {
      if (adtprs(arg,&(pcm->tmin),&dtim) == NULL) goto errw;
      lon = t2gr(pcm->xgrval,&dtim);
    } else {
      if (getdbl(arg,&lon) == NULL ) goto errw;
    }
    if ((arg = nxtwrd (arg)) == NULL) goto errw;
    if (pcm->ydim == 3) {
      if (adtprs(arg,&(pcm->tmin),&dtim) == NULL) goto errw;
      lat = t2gr(pcm->ygrval,&dtim);
    } else {
      if (getdbl(arg,&lat) == NULL ) goto errw;
    }
    if (flag) {
      conv = pcm->xab2gr;
      x = lon;
      if (conv && pcm->xdim!=3) x = conv(pcm->xabval, lon);
      conv = pcm->yab2gr;
      y = lat;
      if (conv && pcm->ydim!=3) y = conv(pcm->yabval, lat);
      snprintf(pout,1255,"%s = %g  %s = %g\n", ccdims[pcm->xdim], x, ccdims[pcm->ydim], y);
      gaprnt(2,pout);
    } else {
      gxconv (lon,lat,&x,&y,2);
      snprintf(pout,1255,"X = %g  Y = %g\n",x,y);
      gaprnt(2,pout);
    }
    return (0);
    errw:
    if (flag) snprintf(pout,1255,"Query Error: Syntax is QUERY W2GR %s %s\n",
             cdims[pcm->xdim+1],cdims[pcm->ydim+1]);
    else snprintf(pout,1255,"Query Error: Syntax is QUERY W2XY %s %s\n",
             cdims[pcm->xdim+1],cdims[pcm->ydim+1]);
    gaprnt (0,pout);
    return (1);
  }

  else if (cmpwrd(arg,"gr2w") || cmpwrd(arg,"gr2xy")) {
    if (pcm->xdim<0 || pcm->ydim<0) {
      gaprnt (2,"No scaling environment\n");
      return (0);
    }
    flag = 0;
    if (cmpwrd(arg,"gr2w")) flag = 1;
    if ((arg = nxtwrd (arg)) == NULL) goto errgr;
    if (getdbl(arg,&x) == NULL ) goto errgr;
    if ((arg = nxtwrd (arg)) == NULL) goto errgr;
    if (getdbl(arg,&y) == NULL ) goto errgr;
    conv = pcm->xgr2ab;
    lon = x;
    if (conv && pcm->xdim!=3) lon = conv(pcm->xgrval, x);
    conv = pcm->ygr2ab;
    lat = y;
    if (conv && pcm->ydim!=3) lat = conv(pcm->ygrval, y);
    if (flag) {
      if (pcm->xdim==3) {
        gr2t (pcm->xgrval, lon, &dtim);
        gat2ch (&dtim, 5, lab, 20);
        snprintf(pout,1255,"%s = %s  %s = %g\n",cdims[pcm->xdim+1],lab,
                    cdims[pcm->ydim+1],lat);
        gaprnt(2,pout);
      } else if (pcm->ydim==3) {
        gr2t (pcm->ygrval, lat, &dtim);
        gat2ch (&dtim, 5, lab, 20);
        snprintf(pout,1255,"%s = %g  %s = %s\n",cdims[pcm->xdim+1],lon,
                    cdims[pcm->ydim+1],lab);
        gaprnt(2,pout);
      } else {
        snprintf(pout,1255,"%s = %g  %s = %g\n",cdims[pcm->xdim+1],lon,
                    cdims[pcm->ydim+1],lat);
        gaprnt(2,pout);
      }
    } else {
      gxconv (lon,lat,&x,&y,2);
      snprintf(pout,1255,"X = %g  Y = %g\n",x,y);
      gaprnt(2,pout);
    }
    return (0);
    errgr:
    if (flag) snprintf(pout,1255,"Query Error: Syntax is QUERY GR2W %s %s\n",
             ccdims[pcm->xdim],ccdims[pcm->ydim]);
    else snprintf(pout,1255,"Query Error: Syntax is QUERY GR2XY %s %s\n",
             ccdims[pcm->xdim],ccdims[pcm->ydim]);
    gaprnt (0,pout);
    return (1);
  }

  else if (cmpwrd(arg,"pp2xy")) {
    if ((arg = nxtwrd (arg)) == NULL) goto errpp;
    if (getdbl(arg,&x) == NULL ) goto errpp;
    if ((arg = nxtwrd (arg)) == NULL) goto errpp;
    if (getdbl(arg,&y) == NULL ) goto errpp;
    gxppvp (x, y, &x, &y);
    snprintf(pout,1255,"X = %g  Y = %g\n",x,y);
    gaprnt(2,pout);
    return (0);
    errpp:
    gaprnt (0,"Query Error: Syntax is QUERY PP2XY ppx ppy\n");
    return (1);
  }

  else if (cmpwrd(arg,"xy2w") || cmpwrd(arg,"xy2gr")) {
    flag = 0;
    if (cmpwrd(arg,"xy2gr")) flag = 1;
    if ((arg = nxtwrd (arg)) == NULL) goto errxy;
    if (getdbl(arg,&x) == NULL ) goto errxy;
    if ((arg = nxtwrd (arg)) == NULL) goto errxy;
    if (getdbl(arg,&y) == NULL ) goto errxy;
    if (pcm->xdim<0 || pcm->ydim<0) {
      gaprnt (2,"No scaling environment\n");
    } else {
      gxxy2w (x,y,&lon,&lat);
      if (flag) {
        conv = pcm->xab2gr;
        x = lon;
        if (lon>-999.0) {
          if (conv && pcm->xdim!=3) x = conv(pcm->xabval, lon);
        }
        conv = pcm->yab2gr;
        y = lat;
        if (lat>-999.0) {
          if (conv && pcm->ydim!=3) y = conv(pcm->yabval, lat);
        }
        snprintf(pout,1255,"%s = %g  %s = %g\n",ccdims[pcm->xdim],x,ccdims[pcm->ydim],y);
        gaprnt(2,pout);
      } else {
        if (pcm->xdim==3) {
          gr2t (pcm->xgrval, lon, &dtim);
          gat2ch (&dtim, 5, lab, 20);
          snprintf(pout,1255,"%s = %s  %s = %g\n",cdims[pcm->xdim+1],lab,cdims[pcm->ydim+1],lat);
          gaprnt(2,pout);
        } else if (pcm->ydim==3) {
          gr2t (pcm->ygrval, lat, &dtim);
          gat2ch (&dtim, 5, lab, 20);
          snprintf(pout,1255,"%s = %g  %s = %s\n",cdims[pcm->xdim+1],lon,cdims[pcm->ydim+1],lab);
          gaprnt(2,pout);
        } else {
          snprintf(pout,1255,"%s = %g  %s = %g\n",cdims[pcm->xdim+1],lon,cdims[pcm->ydim+1],lat);
          gaprnt(2,pout);
        }
      }
    }
    return (0);
    errxy:
    if (flag) gaprnt (0,"Query Error: Syntax is QUERY XY2GR x y\n");
    else gaprnt (0,"QUERY error: Syntax is QUERY XY2W x y\n");
    return (1);
  }

  else if (cmpwrd(arg,"ll2xy")) {
    if ((arg = nxtwrd (arg)) == NULL) goto errll;
    if (getdbl(arg,&lon) == NULL ) goto errll;
    if ((arg = nxtwrd (arg)) == NULL) goto errll;
    if (getdbl(arg,&lat) == NULL ) goto errll;
    gxconv (lon,lat,&x,&y,2);
    snprintf(pout,1255,"%g %g\n",x,y);
    gaprnt (2,pout);
    return (0);
    errll:
    gaprnt (0,"QUERY error: Syntax is QUERY LL2XY lon lat\n");
    return (1);
  }

  else if (cmpwrd(arg,"defval")) {
    i = gaqdef (arg, pcm, 0);
    return (i);
  }

  else if (cmpwrd(arg,"udft")) {
    gaprnt(2,"Warning: User Defined Functions have been replaced by User Defined Plug-ins.\n");
    gaprnt(2,"         Please read documentation at http://cola.gmu.edu/grads/gadoc/udp.html\n");
    return (0);
  }

  else if (cmpwrd(arg,"udpt")) {
    gaprntupb();
    return (0);
  }

  else if (cmpwrd(arg,"ctlinfo")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if ((arg = nxtwrd (arg)) == NULL) {
      pfi = pcm->pfid;
      fnum = pcm->dfnum;
    } else {
      if (intprs(arg,&fnum) == NULL ) {
        i = 0;
        while (*arg!=' '&&*arg!='\0'&&*arg!='\n'&&i<19) {
          lab[i] = *arg;
          arg++;
          i++;
        }
        lab[i] = '\0';
        snprintf(pout,1255,"Invalid QUERY CTLINFO argument: %s \n",lab);
        gaprnt (0,pout);
        return (1);
      }
      pfi = pcm->pfi1;
      for (i=0; i<fnum-1; i++) {
        pfi = pfi->pforw;
        if (pfi==NULL) {
          snprintf(pout,1255,"QUERY CTLINFO Error:  file %i not open\n",fnum);
          gaprnt (0,pout);
          return(1);
        }
      }
    }

    snprintf(pout,1255,"dset %s\n",pfi->name);
    gaprnt (2,pout);
    snprintf(pout,1255,"title %s\n",pfi->title);
    gaprnt (2,pout);
    snprintf(pout,1255,"undef %g\n",pfi->undef);
    gaprnt (2,pout);

    if (pfi->ncflg==1) gaprnt(2,"dtype netcdf\n");
    if (pfi->ncflg==2) gaprnt(2,"dtype hdfsds\n");
    if (pfi->type==2) {
      gaprnt (2,"dtype station\n");
      snprintf(pout,1255,"  Tsize = %i\n",pfi->dnum[3]);
      gaprnt(2,pout);
    } 
    else {
      snprintf(pout,1255,"xdef %i",pfi->dnum[0]);
      gaprnt(2,pout);
      if (pfi->linear[0]) {
        conv = pfi->gr2ab[0];
        snprintf(pout,1255," linear %g %g\n",conv(pfi->grvals[0],1.0),*(pfi->grvals[0]));
        gaprnt (2,pout);
      } 
      else {
        gaprnt(2," levels");
        conv = pfi->gr2ab[0];
        cnt = 3;
        for (i=1; i<=pfi->dnum[0]; i++) {
          snprintf(pout,1255," %g",conv(pfi->grvals[0],(gadouble)i));
          gaprnt (2,pout);
          cnt++;
          if (cnt>10 && i!=pfi->dnum[0]) {
            gaprnt (2,"\n"); cnt = 1;
          }
        }
        gaprnt (2,"\n");
      }

      snprintf(pout,1255,"ydef %i",pfi->dnum[1]);
      gaprnt(2,pout);
      if (pfi->linear[1]) {
        conv = pfi->gr2ab[1];
        snprintf(pout,1255," linear %g %g\n",conv(pfi->grvals[1],1.0),*(pfi->grvals[1]));
        gaprnt (2,pout);
      } else {
        gaprnt(2," levels");
        conv = pfi->gr2ab[1];
        cnt = 3;
        for (i=1; i<=pfi->dnum[1]; i++) {
          snprintf(pout,1255," %g",conv(pfi->grvals[1],(gadouble)i));
          gaprnt (2,pout);
          cnt++;
          if (cnt>10 && i!=pfi->dnum[1]) {
            gaprnt (2,"\n"); cnt = 1;
          }
        }
        gaprnt (2,"\n");
      }

      snprintf(pout,1255,"zdef %i",pfi->dnum[2]);
      gaprnt(2,pout);
      if (pfi->linear[2]) {
        conv = pfi->gr2ab[2];
        snprintf(pout,1255," linear %g %g\n",conv(pfi->grvals[2],1.0),*(pfi->grvals[2]));
        gaprnt (2,pout);
      } else {
        gaprnt(2," levels");
        conv = pfi->gr2ab[2];
        cnt = 3;
        for (i=1; i<=pfi->dnum[2]; i++) {
          snprintf(pout,1255," %g",conv(pfi->grvals[2],(gadouble)i));
          gaprnt (2,pout);
          cnt++;
          if (cnt>10 && i!=pfi->dnum[2]) {
            gaprnt (2,"\n"); cnt = 1;
          }
        }
        gaprnt (2,"\n");
      }

      gr2t (pfi->grvals[3],1.0,&dtim);
      if (dtim.mn==0) gat2ch (&dtim,4,lab,20);
      else gat2ch (&dtim,5,lab,20);
      if (*(pfi->grvals[3]+5)!=0) {
        snprintf(pout,1255,"tdef %i linear %s %gmo\n",pfi->dnum[3],lab,*(pfi->grvals[3]+5));
      } else {
        snprintf(pout,1255,"tdef %i linear %s %gmn\n",pfi->dnum[3],lab,*(pfi->grvals[3]+6));
      }
      gaprnt (2,pout);

      /* only write out EDEF if esize > 1*/
      if (pfi->dnum[4]>1) {
	if (pfi->ens1) {
	  /* write out a multi-line EDEF entry with all metadata */
	  snprintf(pout,1255,"edef %i \n",pfi->dnum[4]);
	  gaprnt(2,pout);
	  for (i=0; i<pfi->dnum[4]; i++) {
	    gat2ch(&(pfi->ens1[i].tinit),4,lab,20);
	    snprintf(pout,1255,"%s %d %s\n", pfi->ens1[i].name, pfi->ens1[i].length, lab);
	    gaprnt(2,pout);
	  }
	  gaprnt(2,"endedef\n");
	}
      }
	
      snprintf(pout,1255,"vars %i\n",pfi->vnum);
      gaprnt (2,pout);
      pvar = pfi->pvar1;
      for (i=0;i<pfi->vnum;i++) {
	/* print out the variable name and the number of levels */
	if (pvar->longnm[0] != '\0')
	  snprintf(pout,1255,"%s=>%s  %i  ",pvar->longnm,pvar->abbrv,pvar->levels);
	else
	  snprintf(pout,1255,"%s  %i  ",pvar->abbrv,pvar->levels);
	gaprnt(2,pout);
	/* print out the values in the units field */
	for (j=0; j<5; j++) {
	  if (pvar->units[j] != -999) {
	    if      (pvar->units[j] == -100) gaprnt(2,"x");
	    else if (pvar->units[j] == -101) gaprnt(2,"y");
	    else if (pvar->units[j] == -102) gaprnt(2,"z");
	    else if (pvar->units[j] == -103) gaprnt(2,"t");
	    else if (pvar->units[j] == -104) gaprnt(2,"e");
	    else {
	      snprintf(pout,1255,"%g",pvar->units[j]);
	      gaprnt(2,pout);
	    }
	    if (pvar->units[j+1] != -999) gaprnt(2,",");    /* add a comma if we've got more */
	  }
	}
	/* print out the description */
	snprintf(pout,1255,"  %s\n",pvar->varnm);
        gaprnt(2,pout);
        pvar++;
      }
      gaprnt (2,"endvars\n");
    }
  }
  else if (cmpwrd(arg,"file")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if ((arg = nxtwrd (arg)) == NULL) {
      pfi = pcm->pfid;
      fnum = pcm->dfnum;
    } else {
      if (intprs(arg,&fnum) == NULL ) {
        i = 0;
        while (*arg!=' '&&*arg!='\0'&&*arg!='\n'&&i<19) {
          lab[i] = *arg;
          arg++;
          i++;
        }
        lab[i] = '\0';
        snprintf(pout,1255,"Invalid QUERY FILE argument: %s \n",lab);
        gaprnt (0,pout);
        return (1);
      }
      pfi = pcm->pfi1;
      for (i=0; i<fnum-1; i++) {
        pfi = pfi->pforw;
        if (pfi==NULL) {
          snprintf(pout,1255,"QUERY FILE Error:  file %i not open\n",fnum);
          gaprnt (0,pout);
          return(1);
        }
      }
    }
    snprintf(pout,1255,"File %i : %s\n",fnum,pfi->title);
    gaprnt (2,pout);
    snprintf(pout,1255,"  Descriptor: %s\n",pfi->dnam);
    gaprnt (2,pout);
    snprintf(pout,1255,"  Binary: %s\n",pfi->name);
    gaprnt (2,pout);
    if (pfi->type==2) {
      if (pfi->bufrflg) {
	gaprnt (2,"  Type = BUFR Station Data\n");
      } else {
	gaprnt (2,"  Type = Station Data\n");
      }
      snprintf(pout,1255,"  Tsize = %i\n",pfi->dnum[3]);
      gaprnt(2,pout);
    } else {
      gaprnt (2,"  Type = Gridded\n");
      snprintf(pout,1255,"  Xsize = %i  Ysize = %i  Zsize = %i  Tsize = %i  Esize = %i\n",
         pfi->dnum[0],pfi->dnum[1],pfi->dnum[2],pfi->dnum[3],pfi->dnum[4]);
      gaprnt(2,pout);
    }
    snprintf(pout,1255,"  Number of Variables = %i\n",pfi->vnum);
    gaprnt (2,pout);

    pvar = pfi->pvar1;
    for (i=0;i<pfi->vnum;i++) {
      /* print out the variable name and the number of levels */
      snprintf(pout,1255,"     %s  %i  ",pvar->abbrv,pvar->levels);
      gaprnt(2,pout);
      /* print out the values in the units field */
      for (j=0; j<5; j++) {
	if (pvar->units[j] != -999) {
	  if      (pvar->units[j] == -100) gaprnt(2,"x");
	  else if (pvar->units[j] == -101) gaprnt(2,"y");
	  else if (pvar->units[j] == -102) gaprnt(2,"z");
	  else if (pvar->units[j] == -103) gaprnt(2,"t");
	  else if (pvar->units[j] == -104) gaprnt(2,"e");
	  else {
	    snprintf(pout,1255,"%g",pvar->units[j]);
	    gaprnt(2,pout);
	  }
	  if (pvar->units[j+1] != -999) gaprnt(2,",");    /* add a comma if we've got more */
	}
      }
      /* print out the description */
      snprintf(pout,1255,"  %s\n",pvar->varnm);
      gaprnt(2,pout);
      pvar++;
    }
  }

  else if (cmpwrd(arg,"gxout")) {
    snprintf(pout,1255,"General = %s\n",gxout0D[pcm->gout1]);
    gaprnt(2,pout);
    snprintf(pout,1255,"1D Graphic, 1 expr = %s\n",gxout1D[pcm->gout1]);
    gaprnt(2,pout);
    snprintf(pout,1255,"1D Graphic, 2 expr = %s\n",gxout1Da[pcm->gout1a]);
    gaprnt(2,pout);
    snprintf(pout,1255,"2D Graphic, 1 expr = %s\n",gxout2Da[pcm->gout2a]);
    gaprnt(2,pout);
    snprintf(pout,1255,"2D Graphic, 2 expr = %s\n",gxout2Db[pcm->gout2b]);
    gaprnt(2,pout);
    snprintf(pout,1255,"Station data = %s\n",gxoutStn[pcm->goutstn]);
    gaprnt(2,pout);
  }

  else if (cmpwrd(arg,"gxinfo")) {
    snprintf(pout,1255,"Last Graphic = %s\n",gxnms[pcm->lastgx]);
    gaprnt(2,pout);
    snprintf(pout,1255,"Page Size = %g by %g\n",pcm->xsiz,pcm->ysiz);
    gaprnt(2,pout);
    snprintf(pout,1255,"X Limits = %g to %g\n",pcm->xsiz1,pcm->xsiz2);
    gaprnt(2,pout);
    snprintf(pout,1255,"Y Limits = %g to %g\n",pcm->ysiz1,pcm->ysiz2);
    gaprnt(2,pout);
    snprintf(pout,1255,"Xaxis = %s  Yaxis = %s\n",cdims[pcm->xdim+1],
                  cdims[pcm->ydim+1]);
    gaprnt(2,pout);
    snprintf(pout,1255,"Mproj = %d\n",pcm->mproj);
    gaprnt(2,pout);
  }

  else if (cmpwrd(arg,"xinfo")) {
    if (pcm->batflg) {
      gaprnt(2,"Batch Mode\n");
    } else {
      if (dsubs->win_data (&xinf) ) {
        snprintf(pout,1255,"Window ID = %d\n",xinf.winid);
        gaprnt(2,pout);
        snprintf(pout,1255,"Window X = %d\n",xinf.winx);
        gaprnt(2,pout);
        snprintf(pout,1255,"Window Y = %d\n",xinf.winy);
        gaprnt(2,pout);
        snprintf(pout,1255,"Window Width = %d\n",xinf.winw);
        gaprnt(2,pout);
        snprintf(pout,1255,"Window Height = %d\n",xinf.winh);
        gaprnt(2,pout);
        snprintf(pout,1255,"Window Border = %d\n",xinf.winb);
        gaprnt(2,pout);
      } else {
        gaprnt(2,"Error\n");
      }
    }
  }

  else if (cmpwrd(arg,"shades")) {
    if (pcm->shdcnt<1) {
      gaprnt(2,"None\n");
    } else {
      snprintf(pout,1255,"Number of levels = %i\n",pcm->shdcnt);
      gaprnt(2,pout);
      for (i=0; i<pcm->shdcnt; i++) {
        if (i==0) 
	  snprintf(pout,1255,"%i <= %g\n",pcm->shdcls[i],pcm->shdlvs[1]);
        else if (i==pcm->shdcnt-1) 
	  snprintf(pout,1255,"%i %g >\n",pcm->shdcls[i],pcm->shdlvs[i]);
        else 
	  snprintf(pout,1255,"%i %g %g\n",pcm->shdcls[i],pcm->shdlvs[i],pcm->shdlvs[i+1]);
        gaprnt(2,pout);
      }
    }
  }

  else if (cmpwrd(arg,"contours")) {
    if (pcm->cntrcnt < 1) {
      gaprnt(2,"None\n");
    } else {
      snprintf(pout,1255,"Number of levels = %i\n",pcm->cntrcnt);
      gaprnt(2,pout);
      for (i=0; i<pcm->cntrcnt; i++) {
        snprintf(pout,1255,"%i %g\n",pcm->cntrcols[i],pcm->cntrlevs[i]);
        gaprnt(2,pout);
      }
    }
  }

  else if (cmpwrd(arg,"fgvals")) {
    snprintf(pout,1255,"Number of fgvals = %d\n",pcm->fgcnt);
    gaprnt(2,pout);
    for (i=0; i<pcm->fgcnt; i++) {
      snprintf(pout,1255,"%d %d\n",pcm->fgcols[i],pcm->fgvals[i]);
      gaprnt(2,pout);
    }
  }

  else if (cmpwrd(arg,"time")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if (pcm->tmin.mn==0) gat2ch(&(pcm->tmin),4,lab,20);
    else gat2ch (&(pcm->tmin),5,lab,20);
    if (pcm->tmax.mn==0) gat2ch(&(pcm->tmax),4,lab2,20);
    else gat2ch (&(pcm->tmax),5,lab2,20);
    snprintf(pout,1255,"Time = %s to %s",lab,lab2);
    gaprnt (2,pout);
    snprintf(pout,1255,"  %s to %s\n",dweek[dayweek(&(pcm->tmin))],dweek[dayweek(&(pcm->tmax))]);
    gaprnt (2,pout);
  }

  else if (cmpwrd(arg,"bpos") || cmpwrd(arg,"pos")) {
    i = 1;
    if ((arg = nxtwrd (arg)) != NULL) {
      if (cmpwrd(arg,"nowait")) i = 0;
    }
    dsubs->gxdbtn (i, &x, &y, &i, &etype, info, rinfo);
    if (etype<1) {
      snprintf(pout,1255,"Position = %g %g %i %i\n",
	       x,y,i,etype);
    } else if (etype==1) {
      snprintf(pout,1255,"Position = %g %g %i %i %i %i\n",
	       x,y,i,etype,*info,*(info+1));
    } else if (etype==2) {
      snprintf(pout,1255,"Position = %g %g %i %i %i %g %g\n",
	       x,y,i,etype,*info,*rinfo,*(rinfo+1));
    } else if (etype==3) {
      snprintf(pout,1255,"Position = %g %g %i %i %i %i %i %i %i %i %i %i\n",
         x,y,i,etype,*info,*(info+1),*(info+2),*(info+3),*(info+4),*(info+5),*(info+6),*(info+7));
    }
    gaprnt (2,pout);
  }

  else if (cmpwrd(arg,"define")) {
    if (pcm->pdf1==NULL) {
      gaprnt (1,"No Defined Variables\n");
      return(0);
    } 
    if ((arg = nxtwrd (arg)) == NULL) {
      /* no variable name specified, just list all defined varnames */
      pdf = pcm->pdf1;
      while (pdf) {
        pfi = pdf->pfi;
        snprintf(pout,1255,"%s %g\n",pdf->abbrv,*(pfi->rbuf));
	gaprnt(2,pout);
        pdf = pdf->pforw;
      }
    } else {
      /* parse the user-provided variable name */
      i = 0;
      while (*arg!=' '&&*arg!='\0'&&*arg!='\n'&&i<19) {
	lab[i] = *arg;
	arg++;
	i++;
      }
      lab[i] = '\0';
      /* see if it matches any defined variables */
      pdf = pcm->pdf1;
      while (pdf) {
        if (!strcmp(pdf->abbrv, lab)) {       
          pfi = pdf->pfi;
	  /* print relevant information about the defined variable */
          if (pfi->type==4) {
            snprintf (pout,1255,"Gridded Defined Variable: %s \n",pdf->abbrv); 
	    gaprnt (2,pout);
          } else if (pfi->type==5) {
	    /* a variable 'put' here by gradspy */
            snprintf (pout,1255,"Gridded Defined Variable from GradsPy: %s \n",pdf->abbrv); 
	    gaprnt (2,pout);
          } else {
	    /* one day there may be more than two types of defined variables */
            gaprnt (0,"Logic error when querying a defined variable\n");
            return(0);
	  }

	  /* print stuff about the grid */
          snprintf(pout,1255," Xsize = %i  Ysize = %i  Zsize = %i  Tsize = %i  Esize = %i\n",
		   pfi->dnum[0],pfi->dnum[1],pfi->dnum[2],pfi->dnum[3],pfi->dnum[4]);
          gaprnt(2,pout);
	  
	  snprintf(pout,1255,"  XDEF %i",pfi->dnum[0]);
	  gaprnt(2,pout);
	  if (pfi->linear[0]) {
	    conv = pfi->gr2ab[0];
	    v1=(gadouble)(1+pfi->dimoff[0]);
	    snprintf(pout,1255," linear %g %g\n",conv(pfi->grvals[0],v1),*(pfi->grvals[0]));
	    gaprnt (2,pout);
	  } 
	  else {
	    gaprnt(2," levels");
	    conv = pfi->gr2ab[0];
	    cnt = 3;
	    for (i=1; i<=pfi->dnum[0]; i++) {
	      v1=(gadouble)(i+pfi->dimoff[0]);
	      snprintf(pout,1255,"   %g",conv(pfi->grvals[0],v1));
	      gaprnt (2,pout);
	      cnt++;
	      if (cnt>10 && i!=pfi->dnum[0]) {
		gaprnt (2,"\n"); cnt = 1;
	      }
	    }
	    gaprnt (2,"\n");
	  }
	  snprintf(pout,1255,"  YDEF %i",pfi->dnum[1]);
	  gaprnt(2,pout);
	  if (pfi->linear[1]) {
	    conv = pfi->gr2ab[1];
	    v1=(gadouble)(1+pfi->dimoff[1]);
	    snprintf(pout,1255," linear %g %g\n",conv(pfi->grvals[1],v1),*(pfi->grvals[1]));
	    gaprnt (2,pout);
	  } else {
	    gaprnt(2," levels");
	    conv = pfi->gr2ab[1];
	    cnt = 3;
	    for (i=1; i<=pfi->dnum[1]; i++) {
	      v1=(gadouble)(i+pfi->dimoff[1]);
	      snprintf(pout,1255," %g",conv(pfi->grvals[1],v1));
	      gaprnt (2,pout);
	      cnt++;
	      if (cnt>10 && i!=pfi->dnum[1]) {
		gaprnt (2,"\n   "); cnt = 1;
	      }
	    }
	    gaprnt (2,"\n");
	  }
	  
	  snprintf(pout,1255,"  ZDEF %i",pfi->dnum[2]);
	  gaprnt(2,pout);
	  if (pfi->linear[2]) {
	    conv = pfi->gr2ab[2];
	    v1=(gadouble)(1+pfi->dimoff[2]);
	    snprintf(pout,1255," linear %g %g\n",conv(pfi->grvals[2],v1),*(pfi->grvals[2]));
	    gaprnt (2,pout);
	  } else {
	    gaprnt(2," levels");
	    conv = pfi->gr2ab[2];
	    cnt = 3;
	    for (i=1; i<=pfi->dnum[2]; i++) {
	      v1=(gadouble)(i+pfi->dimoff[2]);
	      snprintf(pout,1255," %g",conv(pfi->grvals[2],v1));
	      gaprnt (2,pout);
	      cnt++;
	      if (cnt>10 && i!=pfi->dnum[2]) {
		gaprnt (2,"\n   "); cnt = 1;
	      }
	    }
	    gaprnt (2,"\n");
	  }
	  
          v1=(gadouble)(1+pfi->dimoff[3]);
	  gr2t (pfi->grvals[3],v1,&dtim);
	  if (dtim.mn==0) gat2ch (&dtim,4,lab,20);
	  else gat2ch (&dtim,5,lab,20);
	  if (*(pfi->grvals[3]+5)!=0) {
	    snprintf(pout,1255,"  TDEF %i linear %s %gmo\n",pfi->dnum[3],lab,*(pfi->grvals[3]+5));
	  } else {
	    snprintf(pout,1255,"  TDEF %i linear %s %gmn\n",pfi->dnum[3],lab,*(pfi->grvals[3]+6));
	  }
	  gaprnt (2,pout);
	  
	  /* only mention EDEF if esize > 1 */
	  if (pfi->dnum[4]>1) {
	    snprintf(pout,1255,"  EDEF %i linear 1 1\n",pfi->dnum[4]);
	    gaprnt(2,pout);
            gaprnt(2,"    * Note: Defined grids always have an abstract E axis. \n");
            gaprnt(2,"    * All members span the same time axis and should be   \n");
            gaprnt(2,"    * referenced by their index value, not their names.   \n");
	  }

	  /* What kind of calendar does it have ? */
          if (pfi->calendar==1) gaprnt(2," Calendar: 365-Day \n");
	  else gaprnt(2," Calendar: Gregorian \n");

	  /* Has it been modified? */
	  if (pfi->climo==1) gaprnt(2," Climatology: Seasonal \n");
	  if (pfi->climo==2) gaprnt(2," Climatology: Diurnal \n");

	}
	pdf = pdf->pforw;
      }
    }
    return(0);
  }

  else if (cmpwrd(arg,"files")) {
    pfi = pcm->pfi1;
    if (pfi==NULL) {
      gaprnt (2,"No files open\n");
    } else {
      j = 1;
      while (pfi!=NULL) {
        snprintf(pout,1255,"File %i : %s\n",j,pfi->title);
        gaprnt (2,pout);
        snprintf(pout,1255,"  Descriptor: %s\n",pfi->dnam);
        gaprnt (2,pout);
        snprintf(pout,1255,"  Binary: %s\n",pfi->name);
        gaprnt (2,pout);
        pfi = pfi->pforw;
        j++;
      }
    }
  }

  else if (cmpwrd(arg,"attr")) {
    if (pcm->pfi1==NULL) {
      gaprnt (0,"No Files Open\n");
      return(1);
    }
    if ((arg = nxtwrd (arg)) == NULL) {
      pfi = pcm->pfid;
      fnum = pcm->dfnum;
    } else {
      if (intprs(arg,&fnum) == NULL ) {
        i = 0;
        while (*arg!=' ' && *arg!='\0' && *arg!='\n' && i<19) {
          lab[i] = *arg;
          arg++;
          i++;
        }
        lab[i] = '\0';
        snprintf(pout,1255,"Invalid QUERY ATTR argument: %s \n",lab);
        gaprnt (0,pout);
        return (1);
      }
      pfi = pcm->pfi1;
      for (i=0; i<fnum-1; i++) {
        pfi = pfi->pforw;
        if (pfi==NULL) {
          snprintf(pout,1255,"QUERY ATTR Error: file %i not open\n",fnum);
          gaprnt (0,pout);
          return(1);
        }
      }
    }
  
    /* Print attributes from the descriptor file */
    hdrflgd=1;
    if (pfi->attr) { 
      /* Print global and coordinate attributes */
      hdrflgd = prntgaattr (pfi, "global", hdrflgd, fnum);
      hdrflgd = prntgaattr (pfi, "lon",  hdrflgd, fnum);
      hdrflgd = prntgaattr (pfi, "lat",  hdrflgd, fnum);
      hdrflgd = prntgaattr (pfi, "lev",  hdrflgd, fnum);
      hdrflgd = prntgaattr (pfi, "time", hdrflgd, fnum);
      hdrflgd = prntgaattr (pfi, "ens",  hdrflgd, fnum);
      /* Print variable attributes associated with the GrADS variable name, pvar->abbrv */
      pvar = pfi->pvar1;           
      for (i=0; i<pfi->vnum; i++) {
	hdrflgd = prntgaattr (pfi, pvar->abbrv, hdrflgd, fnum);
	pvar++;
      }  
      /* Print a blank line between descriptor attrbutes and native attributes for GDS parsing */
      if (!hdrflgd) gaprnt(2,"\n");
    } 
    if (hdrflgd) {
      /* Always include the following text, even if there are no descriptor attributes */
      snprintf(pout,1255,"No Descriptor Attributes for File %i : %s \n\n",fnum,pfi->title);
      gaprnt(2,pout);
    }


    /* Print native attributes for NetCDF, HDFSDS, HDF5 data sets */
    hdrflg=1; 

    /* Global attributes for netcdf */
    if (pfi->ncflg==1) {  
#if USENETCDF == 1
      /* open each data file in template set until we find one that exists */
      if (pfi->tmplat) {
	ncid=-999; rc=0; error=0;
	gr2t(pfi->grvals[3], 1.0, &tdefi);
	tfile = gafndt(pfi->name, &tdefi, &tdefi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,1,1,&flag);
	rc = nc_open(tfile, NC_NOWRITE, &ncid);
	if (rc != NC_NOERR) error=1;
	if (!error) {
	  closethisfilelater=1;
	} else {
	  for (i=2; i<=pfi->dnum[3]; i++) {
	    ncid=-999; rc=0; error=0;
	    gr2t(pfi->grvals[3], (gadouble)i, &tdef);
	    tfile2 = gafndt(pfi->name, &tdef, &tdefi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,1,1,&flag);
	    if (strcmp(tfile,tfile2)!=0) {
	      gree(tfile,"f213");
	      tfile = tfile2;
	      rc = nc_open(tfile2, NC_NOWRITE, &ncid);
	      if (rc != NC_NOERR) error=1;
	      if (!error) {
		closethisfilelater=1;
		break;
	      }
	    }
	  }
	}
	gree(tfile,"f214");
      } 
      else {
	/* Copy the netcdf file id from the file structure to the local variable ncid */
	if (pfi->ncflg == 1) {
	  ncid = pfi->ncid;
	}
	closethisfilelater=0;
      }
      /* Retrieve netcdf global attributes */
      n_gatts = ncpattrs(ncid, "NC_GLOBAL", "global", hdrflg, fnum, pfi->title);
      if (hdrflg && n_gatts>0) hdrflg=0;
#endif
    }
    
    /* Global attributes for DTYPE HDFSDS */
    else if (pfi->ncflg == 2) {
#if USEHDF==1
      /* open each data file in template set until we find one that exists */
      if (pfi->tmplat) {
	sdid=-999;
	gr2t(pfi->grvals[3], 1.0, &tdefi);
	tfile = gafndt(pfi->name, &tdefi, &tdefi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,1,1,&flag);
	sdid = SDstart(tfile,1);
	if (sdid != -1) {
	  closethisfilelater=1;
	} else {
	  for (i=2; i<=pfi->dnum[3]; i++) {
	    sdid=-999;
	    gr2t(pfi->grvals[3], (gadouble)i, &tdef);
	    tfile2 = gafndt(pfi->name, &tdef, &tdefi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,1,1,&flag);
	    if (strcmp(tfile,tfile2)!=0) {
	      gree(tfile,"f215");
	      tfile = tfile2;
	      sdid = SDstart(tfile2,1); 
	      if (sdid != -1) {
		closethisfilelater=1;
		break;
	      }
	    }
	  }
	}
	gree(tfile,"f216");
      } 
      else {
	/* Copy the hdf file id from the file structure to the local variable sdid */
	sdid = pfi->sdid;
	closethisfilelater=0;
      }
      
      /* Retrieve HDF global attributes */
      n_gatts = hdfpattrs(sdid, "foo", "global", hdrflg, fnum, pfi->title);
      if (hdrflg && n_gatts>0) hdrflg=0;
#endif
    }
    /* Global attributes for DTYPE HDF5 */
    else if (pfi->ncflg == 3) {
#if USEHDF5==1
      /* open each data file in template set until we find one that exists */
      if (pfi->tmplat) {
	h5id=-999;
	gr2t(pfi->grvals[3], 1.0, &tdefi);
	tfile = gafndt(pfi->name, &tdefi, &tdefi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,1,1,&flag);
	fid = H5Fopen(tfile,H5F_ACC_RDONLY, H5P_DEFAULT);
	if (fid>0) {
	  h5id = (gaint)fid;
	  closethisfilelater=1;
	} else {
	  for (i=2; i<=pfi->dnum[3]; i++) {
	    h5id=-999;
	    gr2t(pfi->grvals[3], (gadouble)i, &tdef);
	    tfile2 = gafndt(pfi->name, &tdef, &tdefi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,1,1,&flag);
	    if (strcmp(tfile,tfile2)!=0) {
	      gree(tfile,"f215");
	      tfile = tfile2;
	      fid = H5Fopen(tfile2,H5F_ACC_RDONLY, H5P_DEFAULT); 
	      if (fid>0) {
		h5id = (gaint)fid;
		closethisfilelater=1;
		break;
	      }
	    }
	  }
	}
	gree(tfile,"f216");
      } 
      else {
	/* Copy the hdf5 file id from the file structure to the local variable h5id */
	h5id = pfi->h5id;
	closethisfilelater=0;
      }
      
      /* Retrieve HDF5 global attributes (nothing to do yet) */
/*       n_gatts = h5pattrs(h5id, "foo", "global", hdrflg, fnum, pfi->title); */
/*       if (hdrflg && n_gatts>0) hdrflg=0; */
#endif
    }


    /* Print netcdf coordinate attributes */
#if USENETCDF == 1
    if (pfi->ncflg == 1) {
      for (i=0;i<pfi->nsdfdims;i++) {
        n_atts = ncpattrs(pfi->ncid, pfi->sdfdimnam[i], pfi->sdfdimnam[i], hdrflg, fnum, pfi->title);
        if (hdrflg && n_atts>0) hdrflg=0;
      }
    }
#endif    
    /* Print Variable attributes for dtype netcdf and dtype hdf. */
    if ((pfi->ncflg == 1) || (pfi->ncflg == 2) || (pfi->ncflg == 3)) {  
      pvar = pfi->pvar1;                  
      for (i=0; i<pfi->vnum; i++) {
	if (pvar->longnm[0] != '\0') {
	  varnam = pvar->longnm;
	}
	else {
	  varnam = pvar->abbrv;
	}
#if USENETCDF == 1
	/* Print NetCDF variable attributes */
	if (pfi->ncflg==1) {
	  n_atts = ncpattrs(ncid, varnam, pvar->abbrv, hdrflg, fnum, pfi->title);
	  if (hdrflg && n_atts>0) hdrflg=0; 
	}
#endif
#if USEHDF ==1
	/* Print HDF variable attributes */
	if (pfi->ncflg==2) {
	  n_atts = hdfpattrs(sdid, varnam, pvar->abbrv, hdrflg, fnum, pfi->title); 
	  if (hdrflg && n_atts>0) hdrflg=0; 
	}
#endif
#if USEHDF5 ==1
	/* Print HDF5 variable attributes */
	if (pfi->ncflg==3) {
	  n_atts = h5pattrs(h5id, varnam, pvar->abbrv, hdrflg, fnum, pfi->title); 
	  if (hdrflg && n_atts>0) hdrflg=0; 
	}
#endif
	pvar++;
      }  
    }
    
    /* close the file we opened to get the attributes*/
    if (closethisfilelater) {
#if USENETCDF==1
      if (pfi->ncflg==1) nc_close(ncid);
#endif
#if USEHDF==1
      if (pfi->ncflg == 2)  SDend(sdid);
#endif
#if USEHDF5==1
      if (pfi->ncflg == 3)  H5Fclose(fid);
#endif
    }
    
    if (hdrflg) {
      snprintf(pout,1255,"No Native Attributes for File %i : %s \n",fnum,pfi->title);
      gaprnt(2,pout);
    }
    
  } /* Matches  else if (cmpwrd(arg,"attr")) { */
  
  else {
    i = 0;
    while (*arg!=' '&&*arg!='\0'&&*arg!='\n'&&i<19) {
      lab[i] = *arg;
      arg++;
      i++;
    }
    lab[i] = '\0';
    snprintf(pout,1255,"Invalid QUERY argument: %s \n",lab);
    gaprnt(0,pout);
  }
  return (0);
}


/* Handle help command */

gaint gahelp (char *cmd, struct gacmn *pcm) {

  printf ("\nFor Complete Information See:  http://cola.gmu.edu/grads\n\n");
  printf ("Basic Commands:\n");
  printf (" OPEN <descr>      opens a data file \n");
  printf (" Query             shows current status \n");
  printf (" Clear             clears graphics window \n");
  printf (" SET <args>        sets options \n");
  printf (" Display expr      displays a variable graphically \n");
  printf (" QUIT              exits the program \n");
  return (0);
}

/* Handle collect command */

gaint gacoll (char *cmd, struct gacmn *pcm) {
struct gastat *pst;
struct gaclct *clct,*clct2;
gaint rc,i,flag,clnm;
size_t sz;

  if ((cmd=nxtwrd(cmd)) == NULL) goto collerr1;
  if (intprs(cmd,&clnm) == NULL) goto collerr1;
  if (clnm<0 || clnm>31) goto collerr1;
  if ((cmd=nxtwrd(cmd)) == NULL) goto collerr1;

  if (cmpwrd("free",cmd)) {
    clct = pcm->clct[clnm];
    while (clct) {
      gasfre(clct->stn);
      clct2 = clct->forw;
      gree(clct,"f218");
      clct = clct2;
    }
    pcm->clct[clnm] = NULL;
    return (0);
  }

  /* Check environment */

  if (pcm->vdim[0]!=0 || pcm->vdim[1]!=0 || (pcm->vdim[2]==1 &&
     pcm->vdim[3]==1) || (pcm->vdim[2] == 0 && pcm->vdim[3] == 0)) {
    gaprnt (0,"Collect Error: Z or T must be the only varying dimension\n");
    return (1);
  }

  /* Evaluate expression(s)  */

  garemb (cmd);
  pst = getpst(pcm);
  if (pst==NULL) return(1);
  rc = gapars(cmd, pst, pcm);
  if (rc) goto collerr2;

  /* Make sure returned objects are station data */

  flag = 0;
  if (pcm->type[0]!=0) flag = 1;
  if (pcm->numgrd>1 && pcm->type[1]!=0) flag = 1;
  if (flag) {
    gaprnt(0,"Collect Error: Station Data Required\n");
    rc = 1;
    goto collerr2;
  }

  /* Chain up what we have collected */

  sz = sizeof(struct gaclct);
  snprintf(pout,1255,"%dclct%d",clnm,pcm->clctnm[clnm]);
  clct = (struct gaclct *)galloc(sz,pout);
  if (clct==NULL) {
    gaprnt (0,"Memory allocation error in collect\n");
    rc = 1;
    goto collerr2;
  }
  clct->forw = NULL;
  if (pcm->clct[clnm]==NULL) pcm->clct[clnm] = clct;
  else {
    /* add new collection to end of chain */
    clct2 = pcm->clct[clnm];            
    while (clct2->forw) clct2 = clct2->forw;
    clct2->forw = clct;
  }
  clct->stn = pcm->result[0].stn;
  pcm->clctnm[clnm]++;

  /* Free any possible extra stuff */

  if (pcm->numgrd > 1) {
    for (i=1; i<pcm->numgrd; i++) {
      if (pcm->type[i]==1) 
	gagfre (pcm->result[i].pgr);
      else 
	gasfre (pcm->result[i].stn);
    }
  }
  pcm->numgrd = 0;
  pcm->relnum = 0;

  gree(pst,"f219");
  return (0);

collerr1:

  gaprnt (0,"Collect command error:  Invalid Syntax\n");
  gaprnt (0,"  Format is:  collect n expr\n");
  return (1);

collerr2:
  gagrel(pcm);
  gree(pst,"f220");
  return(rc);

}

/* Handle display command */

gaint gadspl (char *cmd, struct gacmn *pcm) {
struct gastat *pst;
struct gafile *pfi;
gadouble (*conv) (gadouble *, gadouble) = NULL;
gadouble *vals,v;
gadouble xl,yl,s1,s2;
gaint llen, rcode, labsv;
gaint l,l1,l2,vcnt,i,lflg,ldim,rc;
char lab[30];
static gaint dcolor[10] = {-1, 1, 3, 7, 2, 6, 9, 10, 11, 12 };

  if (pcm->impflg) gacmd (pcm->impnam, pcm, 0);
  rcode = 1;
  if ((cmd=nxtwrd(cmd)) == NULL) {
    gaprnt (0,"Display command error:  No expression provided\n");
    return (1);
  }
  garemb (cmd);

  pst = getpst(pcm);
  if (pst==NULL) return(1);

  vcnt = 0;
  for (i=0; i<5; i++) if (pcm->vdim[i]) vcnt++;
  lflg = pcm->loopflg;
  if (vcnt>2) lflg = 1;
  pcm->pass = pcm->pass%INT_MAX;
  if (!lflg) {            /* not looping */
    pcm->mcolor = 8;
    if (pcm->ccolor==-9) {
      if (vcnt==2) 
	pcm->ccolor = dcolor[pcm->pass%10];
      else 
	pcm->ccolor = dcolor[(pcm->pass+1)%10];
    }
    if (pcm->cmark==-9) {
      pcm->cmark = pcm->pass+2;
      while (pcm->cmark>5) pcm->cmark-=5;
    }
    if (pcm->ccolor<0) pcm->mcolor = 15;
    rc = gapars(cmd, pst, pcm);
    if (rc) goto retrn;
    gaplot (pcm); 
    gagrel (pcm);
    pcm->pass++;
    pcm->ccolor = -9;
    pcm->cstyle = -9;
    pcm->cmark = -9;
    pcm->cint = 0.0;
    pcm->cflag = 0;
    pcm->ccflg = 0;
    pcm->cmin = -9.99e33;
    pcm->cmax = 9.99e33;
    pcm->blkflg = 0;
    pcm->rainmn = 0.0;
    pcm->rainmx = 0.0;
    pcm->arrflg = 0;
    pcm->ptflg = 0;
    pcm->xexflg = 0;  
    pcm->yexflg = 0;
  } else {               /* looping */
    pcm->mcolor = 15; 
    if (pcm->ccolor==-9) {
      pcm->ccolor = -1;
      if (vcnt<2) pcm->ccolor = 1;
    }
    if (pcm->cmark==-9) {
      pcm->cmark = pcm->pass+2;
      while (pcm->cmark>5) pcm->cmark-=5;
    }
    pfi = pcm->pfid;
    ldim = pcm->loopdim;
    if (pfi->type > 1 && ldim < 3) {
      gaprnt (0,"Display command error:  Invalid looping environment\n");
      gaprnt (0,"  Cannot loop on stn data through X, Y, or Z\n");
      return (1);
    }
    if (ldim==3) {                    /* loop on T */
      vals = pfi->abvals[3];
      v = t2gr(vals,&(pcm->tmin));
      l1 = (gaint)v;
      v = t2gr(vals,&(pcm->tmax));
      l2 = (gaint)(v+0.999);
    } 
    else {                         /* loop on X, Y, Z or E */
      conv = pfi->ab2gr[ldim];
      vals = pfi->abvals[ldim];
      v = conv(vals,pcm->dmin[ldim]);
      l1 = (gaint)v;
      v = conv(vals,pcm->dmax[ldim]);
      l2 = (gaint)(v+0.999);
    }
    vals = pfi->grvals[ldim];
    if (ldim!=3) conv = pfi->gr2ab[ldim];
    gxfrme (2);
    pcm->pass = 0;
    labsv = pcm->clab;
    for (l=l1;l<=l2;l++) {
      if (ldim==3) {
        gr2t (vals, (gadouble)l, &(pst->tmin));
        pst->tmax = pst->tmin;
      } 
      else {
        pst->dmin[ldim] = conv(vals,(gadouble)l);
        pst->dmax[ldim] = pst->dmin[ldim];
      }
      rc = gapars(cmd, pst, pcm);
      if (rc) goto retrn;
      pcm->clab = 0;
      if (l==l2) pcm->clab = labsv;
      gaplot (pcm);
      if (ldim==3) {
	snprintf(lab,29,"%i:%i:%i:%i",pst->tmin.yr,pst->tmin.mo,pst->tmin.dy,pst->tmin.hr);
      }
      else {
	snprintf(lab,29,"%g",pst->dmin[ldim]);
      }
      llen=0; while (lab[llen]) llen++;
      xl = pcm->xsiz - (0.11*(gadouble)(llen));
      xl -= 0.02;
      yl = 0.02;
      s1 = 0.13;
      s2 = 0.11;
      gxwide(1);
      gxchpl (lab,llen,xl,yl,s1,s2,0.0);
      gxfrme (2);
      gagrel (pcm);
      pcm->aflag = -1;
      pcm->aflag2 = -1;
    }
    pcm->dbflg = 0;
    pcm->pass = 0;
    pcm->ccolor = -9;
    pcm->cstyle = -9;
    pcm->cmark = -9;
    pcm->cint = 0.0;
    pcm->cflag = 0;
    pcm->ccflg = 0;
    pcm->cmin = -9.99e33;
    pcm->cmax = 9.99e33;
    pcm->blkflg = 0;
    pcm->rainmn = pcm->rainmx = 0.0;
    pcm->aflag = 0;
    pcm->aflag2 = 0;
    pcm->axflg = 0;
    pcm->ayflg = 0;
    pcm->grdsflg = 1;
    pcm->arrflg = 0;
    pcm->ptflg = 0;
    pcm->xexflg = 0; pcm->yexflg = 0;
  }
  gree(pst,"f103");
  return (0);

retrn:
  gree(pst,"f104");
  return(1);

}

/* Parse compound expression (multiple expressions seperated
   by ;'s), get data, hand data off pcm, return */

gaint gapars (char *cmd, struct gastat *pst, struct gacmn *pcm) {
char *pos;
char *expr;
gaint num, i, rc;
size_t sz;

  sz = (strlen(cmd) + 5) * sizeof(char);
  expr = (char *)galloc(sz,"gapars");
  strcpy (expr, cmd);

  /* Convert all the ;'s to nulls and count the number of
     sub-expressions.                                           */

  num  = 0;
  pos = expr;
  while (*pos!='\0') {
    if (*pos==';') {
      *pos = '\0';
      num++;
    }
    pos++;
  }
  num++;

  /* Evaluate all the subexpressions */
  pos = expr;
  for (i=0; i<num; i++) {
    rc = gaexpr (pos, pst);
    if (!rc) rc = gaqsig();
    if (rc) goto err;
    pcm->type[i] = pst->type;
    pcm->result[i] = pst->result;
    while (*pos!='\0') pos++;
    pos++;
  }
  pcm->numgrd = num;
  pcm->relnum = num;
  gree(expr,"f221");
  return (0);

err:
  gaprnt (0,"DISPLAY error:  Invalid expression \n");
  gaprnt (0,"  Expression = ");
  gaprnt (0,pos);
  gaprnt (0,"\n");
  pcm->numgrd = i;
  pcm->relnum = i;
  gagrel (pcm);
  gree(expr,"f222");
  return (1);
}

/* release data objects */

void gagrel (struct gacmn *pcm) { 
gaint i;

  for (i=0; i<pcm->relnum; i++) {
    if (pcm->type[i]==1) 
      gagfre (pcm->result[i].pgr);
    else 
      gasfre (pcm->result[i].stn);
  }
  pcm->numgrd = 0;
  pcm->relnum = 0;
}

/* Handle set command */

char *justs[9] = {"bl","bc","br","l","c","r","tl","tc","tr"};

gaint gaset (char *cmd, char *com, struct gacmn *pcm) {
struct dt tdef;
struct gawgds *wgds;
struct gafile *pfi;
struct gaens *ens;
struct gaattr *attr,*newattr=NULL;
#if USESHP==1
struct dbfld *fld, *newfld=NULL;
#endif
gadouble (*conv) (gadouble *, gadouble);
gadouble v1,v2,*vals,tt;
gadouble val1,val2,xlo,xhi,ylo,yhi;
gaint kwrd,i,i1,i2,num,id,itt,itt1,itt2,itmp[20];
gaint enum1,enum2;
gaint xx,yy,red,green,blue;
char *ch=NULL,*ch2=NULL,*strng,*pat,*cmd1;
char ename1[16],ename2[16];
size_t sz;
char *tileimg=NULL;
static char *kwds[130] = {"X","Y","Z","T","LON","LAT","LEV","TIME",
			  "CINT","CSTYLE","CCOLOR","LOOPDIM",
			  "LOOPING","LOOPINCR","DFILE","VRANGE",
			  "CSMOOTH","GRID","CMARK","XAXIS","YAXIS",
			  "GXOUT","BLACK","DIGNUM","DIGSIZ","CMIN",
			  "CMAX","ARRSCL","ARRSIZ","CLEVS","STID",
			  "GRADS","CLAB","MPROJ","CTERP","XSIZE",
			  "CCOLS","MPVALS","MPDSET","VPAGE","PAREA",
			  "LINE","STRING","STRSIZ","RGB","FGVALS",
			  "MAP","BARBASE","BARGAP","CTHICK","MPDRAW",
			  "POLI","DBUFF","XYREV","XFLIP","YFLIP",
			  "ANNOT","DISPLAY","BACKGROUND","RBCOLS",
			  "RBRANGE","MISSCONN","IMPRUN","ZLOG","STRMDEN",
			  "FRAME","CLIP","VRANGE2","ARROWHEAD","MDLOPTS",
			  "XLOPTS","YLOPTS","CLOPTS","XLAB","YLAB",
			  "XLEVS","YLEVS","XLINT","YLINT","MISSWARN",
			  "BUTTON","DEFVAL","BAROPTS","LFCOLS","WXCOLS",
			  "FONT","FWRITE","XLPOS","YLPOS","CLSKIP","RBAND",
			  "ARRLAB","MPT","WXOPT","XLABS","YLABS","FILL",
			  "DROPMENU","LATS","TIMELAB","WARN","STNPRINT",
			  "STAT","TLSUPP","GRIDLN","HEMPREF","PRNOPTS",
			  "DATAWARN","DIALOG","WRITEGDS","COSLAT",
			  "E","ENS","SDFWRITE","SDFATTR","GEOTIFF","KML",
			  "UNDEF","CHUNKSIZE","CACHESF","SHPOPTS",
			  "SHP","SHPATTR","LOG1D","STRMOPTS","TILE",
			  "HERSHEY","LWID","ANTIALIAS","BARBOPTS"};

  strng = NULL;
  kwrd=-1;
  tt=0;
  pfi = pcm->pfid;
  cmd1 = cmd;
  if ((cmd = nxtwrd (cmd))==NULL) {
    gaprnt (0,"SET error:  missing operand\n");
    return (1);
  }
  else if (cmpwrd("defval",cmd)) {
    i1 = gaqdef (cmd, pcm, 1);
    return (i1);
  }
  else if (cmpwrd("hempref",cmd)) {
    kwrd=105;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("auto",cmd))  pcm->hemflg = -1; 
    else if (cmpwrd("shem",cmd))  pcm->hemflg = 1; 
    else if (cmpwrd("nhem",cmd))  pcm->hemflg = 0; 
    else goto err;
  }
  else if (cmpwrd("gridln",cmd)) {
    kwrd=104;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    i = 1;
    if (cmpwrd("auto",cmd)) { pcm->gridln = -9; i=0; }
    if (cmpwrd("off",cmd)) { pcm->gridln = -1; i=0; }
    if (i) {
      if (intprs(cmd,&itt) == NULL ) goto err;
      pcm->gridln = itt;
    }
    return(0);
  }
  else if (cmpwrd("tlsupp",cmd)) {
   kwrd=103;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("yr",cmd)) pcm->tlsupp = 1;
    if (cmpwrd("year",cmd)) pcm->tlsupp = 1;
    if (cmpwrd("mo",cmd)) pcm->tlsupp = 2;
    if (cmpwrd("month",cmd)) pcm->tlsupp = 2;
    return(0);
  }
  else if (cmpwrd("baropts",cmd)) {
    kwrd=82;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("outline",cmd)) pcm->barolin = 1;
    if (cmpwrd("filled",cmd)) pcm->barolin = 0;
    return(0);
  }
  else if (cmpwrd("barbopts",cmd)) {
    kwrd=129;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("outline",cmd)) pcm->barbolin = 0;
    if (cmpwrd("filled",cmd)) pcm->barbolin = 1;
    return(0);
  }
  else if (cmpwrd("barbase",cmd)) {
    kwrd = 47;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("bottom",cmd)) {
      pcm->barflg = 0;
      return (0);
    }
    if (cmpwrd("top",cmd)) {
      pcm->barflg = -1;
      return (0);
    }
    if (getdbl(cmd,&pcm->barbase) == NULL) goto err;
    pcm->barflg = 1;
  }
  else if (cmpwrd("mdlopts",cmd)) {
    kwrd = 69;
    while ( (cmd = nxtwrd (cmd)) != NULL) {
      i1 = 0;
      if (cmpwrd("noblank",cmd)) {pcm->mdlblnk = 0; i1 = 1;}
      if (cmpwrd("blank",cmd)) {pcm->mdlblnk = 1; i1 = 1;}
      if (cmpwrd("dig3",cmd)) {pcm->mdldig3 = 1; i1 = 1;}
      if (cmpwrd("nodig3",cmd)) {pcm->mdldig3 = 0; i1 = 1;}
      if (i1==0) goto err;
    }
  }
  else if (cmpwrd("bargap",cmd)) {
    kwrd = 48;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    if (itt<0 || itt>100) {
      gaprnt(0,"SET BARGAP Error: gap must be 0 to 99\n");
      return(1);
    }
    pcm->bargap = itt;
  }
  else if (cmpwrd("lwid",cmd)) {
    kwrd = 127;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    if (itt<13 || itt>256) {
      gaprnt(0,"SET LWID Error: line thickness index number must be between 13 and 256\n");
      return(1);
    }
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&v1) == NULL ) goto err;
    gxdbsetwid(itt,v1);
  }
  else if (cmpwrd("font",cmd)) {
    kwrd = 85;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    /* check if graphics backend supports fonts */
    if (dsubs->gxdckfont()==0 || psubs->gxpckfont()==0) {
      if (itt<0 || itt>9) {
        if (dsubs->gxdckfont()==0 && psubs->gxpckfont()==0) 
  	  gaprnt(0,"SET FONT Error: Graphics plug-ins only support fonts between 0 and 9\n");
        if (dsubs->gxdckfont()==0 && psubs->gxpckfont()==1) 
  	  gaprnt(0,"SET FONT Error: Graphics display plug-in only supports fonts between 0 and 9\n");
        if (dsubs->gxdckfont()==1 && psubs->gxpckfont()==0) 
  	  gaprnt(0,"SET FONT Error: Graphics printing plug-in only supports fonts between 0 and 9\n");
	return(1);
      }
    }
    else {
      if (itt<0 || itt>99) {
	gaprnt(0,"SET FONT Error: font must be 0 to 99\n");
	return(1);
      }
    }
    gxchdf(itt);  /* change the default font in gxchpl.c */
    if ((cmd = nxtwrd (cmd)) != NULL) {
      /* check for 'file' keyword */
      if (cmpwrd("file",cmd)) {
	if ((cmd = nxtwrd (cmd)) != NULL) {
	  /* user has provided a file to define a new font */
	  if (itt<10) {
	    gaprnt(0,"SET FONT Error: Cannot define new font for font numbers less than 10\n"); 
	    return(1);
	  }
	  else {
	    /* get the length of the user-supplied filename */
	    itt2 = 0;
	    while (*(cmd+itt2)!='\n'&&*(cmd+itt2)!='\0') itt2++; /* spaces are allowed */
	    /* allocate memory for tmp filename */
	    ch = NULL;
	    ch = (char *)malloc(itt2+1);
	    if (ch==NULL) {
	      gaprnt (0,"Memory allocation error for tmp font file name\n"); goto err;
	    }
	    /* copy the user-supplied filename (mixed case) to ch */
	    i2 = cmd - cmd1;
	    for (i1=0; i1<itt2; i1++) *(ch+i1) = *(com+i1+i2);
	    *(ch+itt2)='\0';      /* make sure it is null-terminated */
	    gxdbsetfn (itt, ch); 
	    free (ch);
	  }
	}
      }
      else {
	gaprnt(0,"SET FONT Error: Missing keyword \"file\" in new font definition\n"); 
	return(1);
      }
    }
  }
  else if (cmpwrd("hershey",cmd)) {
    kwrd = 126;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) gxdbsethersh(0);  /* 0, use hershey fonts */  
    else if (cmpwrd("off",cmd)) {
      /* check if graphics backend supports fonts */
      if (dsubs->gxdckfont()==0 || psubs->gxpckfont()==0) {
        if (dsubs->gxdckfont()==0 && psubs->gxpckfont()==0) 
          gaprnt(0,"SET Error: Graphics plug-ins do not support non-Hershey fonts\n");
        if (dsubs->gxdckfont()==0 && psubs->gxpckfont()==1) 
          gaprnt(0,"SET Error: Graphics display plug-in does not support non-Hershey fonts\n");
        if (dsubs->gxdckfont()==1 && psubs->gxpckfont()==0) 
          gaprnt(0,"SET Error: Graphics printing plug-in does not support non-Hershey fonts\n");
	return(1);
      }
      else gxdbsethersh(1);  /* 1, use emulation */  
    }
    else goto err;
  }
  else if (cmpwrd("antialias",cmd)) {
    kwrd = 128;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) {
      pcm->aaflg = 0;   /* set flag in gacmn */
      gxsignal(2);      /* tell hardware and metabuffer to disable anti-aliasing */  
    }
    else if (cmpwrd("on",cmd)) {
      pcm->aaflg = 1;   /* set flag in gacmn */
      gxsignal(3);      /* tell hardware and metabuffer to enable anti-aliasing */
    }
    else goto err;
  }
  else if (cmpwrd("clip",cmd)) {
    kwrd = 66;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&xlo) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&xhi) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&ylo) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&yhi) == NULL ) goto err;
    if (xlo<0.0 || ylo<0.0) goto err;
    if (xhi>pcm->pxsize || yhi>pcm->pysize) goto err;
    if (yhi<=ylo || xhi<=xlo) goto err;
    gxclip(xlo,xhi,ylo,yhi);
  }
  else if (cmpwrd("vpage",cmd)) {
    kwrd = 39;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) {
      pcm->xsiz = pcm->pxsize;
      pcm->ysiz = pcm->pysize;
      gxvpag (pcm->xsiz, pcm->ysiz, 0.0, pcm->xsiz, 0.0, pcm->ysiz);
      gacln(pcm,1);
      return (0);
    }
    if (getdbl(cmd,&xlo) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&xhi) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&ylo) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&yhi) == NULL ) goto err;
    if (xlo<0.0 || ylo<0.0 || xhi>pcm->pxsize || yhi>pcm->pysize) {
      gaprnt (0,"SET Error: vpage values beyond real page limits\n");
      return(1);
    }
    if (yhi<=ylo || xhi<=xlo) goto err;
    if ((yhi-ylo)/(xhi-xlo) > pcm->pysize/pcm->pxsize) {
      pcm->ysiz = pcm->pysize;
      pcm->xsiz = pcm->pysize * (xhi-xlo)/(yhi-ylo);
    } else {
      pcm->xsiz = pcm->pxsize;
      pcm->ysiz = pcm->pxsize * (yhi-ylo)/(xhi-xlo);
    }
    gxvpag (pcm->xsiz, pcm->ysiz, xlo, xhi, ylo, yhi);
    snprintf(pout,1255,"Virtual page size = %g %g \n",pcm->xsiz,pcm->ysiz);
    gaprnt (2,pout);
    gacln(pcm,1);
  }
  else if (cmpwrd("mpt",cmd)) {
    kwrd = 92;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("*",cmd))  itt = -999;
    else {
      if (intprs(cmd,&itt) == NULL ) goto err;
      if (itt < 0) itt = 0;
      if (itt>255) itt = 255;
    }
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    green = -999;
    blue = -999;
    if (cmpwrd("off",cmd))  red = -9;
    else {
      if (intprs(cmd,&red) == NULL ) goto err;
      if (red < -1) red = -1;
      if ((cmd = nxtwrd (cmd)) != NULL) {
        if (intprs(cmd,&green) == NULL ) goto err;
        if (green < 1) green = 1;
        if ((cmd = nxtwrd (cmd)) != NULL) {
          if (intprs(cmd,&blue) == NULL ) goto err;
          if (blue < 1) blue = 1;
        }
      }
    }
    if (itt == -999) {
      itt1 = 0; itt2 = 256;
    } else {
      itt1 = itt; itt2 = itt+1;
    }
    for (i=itt1; i<itt2; i++) {
      pcm->mpcols[i] = red;
      if (green != -999) pcm->mpstls[i] = green;
      if (blue != -999) pcm->mpthks[i] = blue;
    }
  }
  else if (cmpwrd("rgb",cmd)) {
    kwrd = 44;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("tile",cmd)) {
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      if (intprs(cmd,&green) == NULL) goto err;
      red = -9; blue = -9;
    } else {
      if (intprs(cmd,&red) == NULL) goto err;
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      if (intprs(cmd,&green) == NULL) goto err;
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      if (intprs(cmd,&blue) == NULL) goto err;
      if ((cmd = nxtwrd (cmd)) == NULL) 
	itt2 = 255; 
      else {
        if (intprs(cmd,&itt2) == NULL)  goto err;
      }
      if (itt<16 || itt>2048) {
        gaprnt (0,"SET RGB Error:  Color number must be between 16 and 2048 \n");
        return(1);
      }
      if (red<0   || red>255   ||  
	  green<0 || green>255 ||  
	  blue<0  || blue>255) {   
        gaprnt (0,"SET RGB Error:  RGB values must be between 0 and 255 \n");
        return(1);
      }
      if (itt2<-255 || itt2>255) { 
        gaprnt (0,"SET RGB Error:  Alpha value must be between -255 and 255 \n");
        return(1);
      }
    }
    gxacol (itt,red,green,blue,itt2);
  }
  else if (cmpwrd("stat",cmd)) {
    kwrd = 102;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) pcm->statflg = 0;
    else if (cmpwrd("on",cmd)) pcm->statflg = 1;
    else goto err;
  }
  else if (cmpwrd("arrlab",cmd)) {
    kwrd = 91;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) pcm->arlflg = 0;
    else if (cmpwrd("on",cmd)) pcm->arlflg = 1;
    else goto err;
  }
  else if (cmpwrd("parea",cmd)) {
    kwrd = 40;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) {
      pcm->paflg = 0;
      return (0);
    }
    if (getdbl(cmd,&xlo) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&xhi) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&ylo) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&yhi) == NULL ) goto err;
    if (xlo<0.0 || ylo<0.0 || xhi>pcm->xsiz || yhi>pcm->ysiz) {
      gaprnt (0,"SET Error: parea values beyond page limits\n");
      return(1);
    }
    if (yhi<=ylo || xhi<=xlo) goto err;
    pcm->pxmin = xlo;
    pcm->pxmax = xhi;
    pcm->pymin = ylo;
    pcm->pymax = yhi;
    pcm->paflg = 1;
  }
  else if (cmpwrd("arrowhead",cmd)) {
    kwrd = 68;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->ahdsiz)) == NULL ) goto err;
    snprintf(pout,1255,"Arrowhead = %g \n",pcm->ahdsiz);
    gaprnt (2,pout);
  }
  else if (cmpwrd("cint",cmd)) {
    kwrd = 8;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&val1) == NULL ) goto err;
    if (val1<=0) {
      gaprnt (0,"SET Error: cint must be greater than 0.0\n");
    }
    else {
      pcm->cint = val1;
      snprintf(pout,1255,"cint = %g \n",pcm->cint);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd("xlint",cmd)) {
    kwrd = 77;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->xlint)) == NULL ) goto err;
    snprintf(pout,1255,"xlint = %g \n",pcm->xlint);
    gaprnt (2,pout);
  }
  else if (cmpwrd("ylint",cmd)) {
    kwrd = 78;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->ylint)) == NULL ) goto err;
    snprintf(pout,1255,"ylint = %g \n",pcm->ylint);
    gaprnt (2,pout);
  }
  else if (cmpwrd("xsize",cmd)) {
    kwrd = 35;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&xx) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&yy) == NULL ) goto err;
    dsubs->gxdxsz(xx,yy);
    gxfrme (9);
  }
  else if (cmpwrd("mpvals",cmd)) {
    kwrd = 37;
    i1 = 0;
    if ((ch = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",ch)) {
      pcm->mpflg = 0;
      gaprnt (2,"mpvals have been turned off\n");
    } else {
      while ((cmd = nxtwrd (cmd)) != NULL) {
        if (getdbl(cmd,&(pcm->mpvals[i1])) == NULL ) goto err;
        i1++;
        if (i1>9) goto err;
      }
      pcm->mpflg = i1;
      gaprnt (2,"mpvals have been set\n");
    }
  }
  else if (cmpwrd("fgvals",cmd)) {
    kwrd = 45;
    i1 = 0;
    while ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&(pcm->fgvals[i1])) == NULL ) goto err;
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      if (intprs(cmd,&(pcm->fgcols[i1])) == NULL ) goto err;
      i1++;
      if (i1>48) goto err;
    }
    pcm->fgcnt = i1;
    gaprnt (2,"fgvals set\n");
  }
  else if (cmpwrd("clevs",cmd)) {
    kwrd = 29;
    i1 = 0;
    while ((cmd = nxtwrd (cmd)) != NULL) {
      if (getdbl(cmd,&(pcm->clevs[i1])) == NULL ) goto err;
      i1++;
      if (i1>254) goto err; 
    }
    pcm->cflag = i1;
    snprintf(pout,1255,"Number of clevs = %i \n",i1);
    gaprnt (2,pout);
    for (i=1; i<pcm->cflag; i++) {
      if (pcm->clevs[i] <= pcm->clevs[i-1]) {
	gaprnt(1,"Warning: Contour levels are not strictly increasing\n");
	gaprnt(1,"         This may lead to errors or undesired results\n");
      }
    }
  }
  else if (cmpwrd("xlevs",cmd)) {
    kwrd = 75;
    i1 = 0;
    while ( (cmd = nxtwrd (cmd)) != NULL) {
      if (getdbl(cmd,&(pcm->xlevs[i1])) == NULL ) goto err;
      i1++;
      if (i1>49) goto err;
    }
    pcm->xlflg = i1;
    snprintf(pout,1255,"Number of xlevs = %i \n",i1);
    gaprnt (2,pout);
  }
  else if (cmpwrd("ylevs",cmd)) {
    kwrd = 76;
    i1 = 0;
    while ((cmd = nxtwrd (cmd)) != NULL) {
      if (getdbl(cmd,&(pcm->ylevs[i1])) == NULL ) goto err;
      i1++;
      if (i1>49) goto err;
    }
    pcm->ylflg = i1;
    snprintf(pout,1255,"Number of ylevs = %i \n",i1);
    gaprnt (2,pout);
  }
  else if (cmpwrd("rbcols",cmd)) {
    kwrd = 59;
    i1 = 0;
    while ((cmd = nxtwrd (cmd)) != NULL) {
      if (i1==0 && cmpwrd("auto",cmd)) break;
      if (intprs(cmd,&(pcm->rbcols[i1])) == NULL ) goto err;
      i1++;
      if (i1>255) goto err; 
    }
    pcm->rbflg = i1;
    if (i1==0) gaprnt(2,"Rainbow colors set to auto\n");
    else {
      snprintf(pout,1255,"Number of rainbow colors = %i\n",i1);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd("dropmenu",cmd)) {
    kwrd = 97;
    i1 = 0;
    while ( (cmd = nxtwrd (cmd)) != NULL) {
      if (i1>14) goto drerr;
      if (intprs(cmd,&(pcm->drvals[i1])) == NULL ) goto drerr;
      i1++;
    }
    if (i1 == 0) goto drerr;
  }
  else if (cmpwrd("ccols",cmd)) {
    kwrd = 36;
    i1 = 0;
    while ( (cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&(pcm->ccols[i1])) == NULL ) goto err;
      i1++;
      if (i1>255) goto err; 
    }
    pcm->ccflg = i1;
    snprintf(pout,1255,"Number of ccols = %i\n",i1);
    gaprnt (2,pout);
    if (pcm->cflag==0) {
      gaprnt (2,"ccols won't take effect unless clevs are set.\n");
    }
  }
  else if (cmpwrd("cmin",cmd)) {
    kwrd = 25;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->cmin)) == NULL ) goto err;
    snprintf(pout,1255,"cmin = %g \n",pcm->cmin);
    gaprnt (2,pout);
  }
  else if (cmpwrd("cmax",cmd)) {
    kwrd = 26;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->cmax)) == NULL ) goto err;
    snprintf(pout,1255,"cmax = %g \n",pcm->cmax);
    gaprnt (2,pout);
  }
  else if (cmpwrd("cmark",cmd)) {
    kwrd = 18;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&(pcm->cmark)) == NULL ) goto err;
    snprintf(pout,1255,"cmark = %i \n",pcm->cmark);
    gaprnt (2,pout);
  }
  else if (cmpwrd("mproj",cmd)) {
    kwrd = 33;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) pcm->mproj = 0;
    else if (cmpwrd("scaled",cmd))       pcm->mproj = 1;
    else if (cmpwrd("latlon",cmd))       pcm->mproj = 2;
    else if (cmpwrd("nps",cmd))          pcm->mproj = 3;
    else if (cmpwrd("sps",cmd))          pcm->mproj = 4;
    else if (cmpwrd("robinson",cmd))     pcm->mproj = 5;
    else if (cmpwrd("mollweide",cmd))    pcm->mproj = 6;
    else if (cmpwrd("orthogr",cmd))      pcm->mproj = 7;
    else if (cmpwrd("orthographic",cmd)) pcm->mproj = 7;
    else if (cmpwrd("ortho",cmd))        pcm->mproj = 7;
    else if (cmpwrd("lambert",cmd))      pcm->mproj = 13;
    else goto err;
  }
  else if (cmpwrd("xyrev",cmd)) {
    kwrd = 53;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) pcm->rotate = 0;
    else if (cmpwrd("on",cmd)) pcm->rotate = 1;
    else goto err;
  }
  else if (cmpwrd("xflip",cmd)) {
    kwrd = 54;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) pcm->xflip = 0;
    else if (cmpwrd("on",cmd)) pcm->xflip = 1;
    else goto err;
 }
  else if (cmpwrd("yflip",cmd)) {
    kwrd = 55;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) pcm->yflip = 0;
    else if (cmpwrd("on",cmd)) pcm->yflip = 1;
    else goto err;
  }
  else if (cmpwrd("writegds",cmd)) {
    wgds = pcm->wgds;
    kwrd = 109;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (*cmd=='-') {
      itt = 0;
      while (*(cmd+itt)!=' '&&*(cmd+itt)!='\n'&&*(cmd+itt)!='\0') itt++;
      sz = itt+2;
      ch = (char *)galloc(sz,"writegds");
      if (ch==NULL) {
        gaprnt (0,"Memory allocation Error\n");
        goto err;
      }
      i2 = cmd - cmd1;
      for (i1=0; i1<itt; i1++) *(ch+i1) = *(com+i1+i2);
      *(ch+i1) = '\0';
      if (wgds->opts) gree(wgds->opts,"f223");
      wgds->opts = ch;
    }
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    itt = 0;
    while (*(cmd+itt)!=' '&&*(cmd+itt)!='\n'&&*(cmd+itt)!='\0') itt++;
    sz = itt+2;
    ch = (char *)galloc(sz,"writegds2");
    if (ch==NULL) {
      gaprnt (0,"Memory allocation Error\n");
      goto err;
    }
    i2 = cmd - cmd1;
    for (i1=0; i1<itt; i1++) *(ch+i1) = *(com+i1+i2);
    *(ch+i1) = '\0';
    if (wgds->fname) gree(wgds->fname,"f224");
    wgds->fname = ch;
    if (wgds->opts) {
      snprintf(pout,1255,"WRITEGDS file name = %s  Opts = %s\n",ch,wgds->opts);
    } else {
      snprintf(pout,1255,"WRITEGDS file name = %s\n",ch);
    }
    gaprnt (2,pout);
  }

  else if (cmpwrd("shp",cmd)) {
    kwrd = 121;
#if USESHP==1
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    /* parse arguments to 'set shp' command */
    while (cmpwrd("-pt",cmd) || cmpwrd("-point",cmd) || 
	   cmpwrd("-ln",cmd) || cmpwrd("-line",cmd)  || 
           cmpwrd("-poly",cmd) || cmpwrd("-fmt",cmd)) {
      if (cmpwrd("-pt",cmd) || cmpwrd("-point",cmd)) pcm->shptype = 1;
      if (cmpwrd("-ln",cmd) || cmpwrd("-line",cmd))  pcm->shptype = 2; 
      if (cmpwrd("-poly",cmd)) pcm->shptype = 3; 
      if (cmpwrd("-fmt",cmd)) {
	if ((cmd = nxtwrd (cmd)) == NULL) goto err;
	if (intprs(cmd,&itt) == NULL ) goto err;
	pcm->dblen = itt;
	if ((cmd = nxtwrd (cmd)) == NULL) goto err;
	if (intprs(cmd,&itt) == NULL ) goto err;
	pcm->dbprec = itt;
      }
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    }
    /* Parse the shapefile output filename root */
    itt = 0;
    while (*(cmd+itt)!=' '&&*(cmd+itt)!='\n'&&*(cmd+itt)!='\0') itt++;
    sz = itt+1;
    ch = (char *)galloc(sz,"shpname");
    if (ch==NULL) {
      gaprnt (0,"Memory allocation error for shapefile name\n");
      goto err;
    }

    i2 = cmd - cmd1;
    for (i1=0; i1<itt; i1++) *(ch+i1) = *(com+i1+i2);
    *(ch+itt) ='\0';

    /* release previously set filenames for shapefile output */
    if (pcm->shpfname) gree(pcm->shpfname,"f225e");
    pcm->shpfname = ch;

    snprintf(pout,1255,"Shapefile output file name root: %s\n",pcm->shpfname);
    gaprnt (2,pout);
    if (pcm->shptype==1) gaprnt(2,"Shapefile output type: point\n");
    if (pcm->shptype==2) gaprnt(2,"Shapefile output type: line\n");
    if (pcm->shptype==3) gaprnt(2,"Shapefile output type: polygon\n");
    snprintf(pout,1255,"Shapefile format string is \%%%d.%df\n",pcm->dblen,pcm->dbprec);
    gaprnt(2,pout);
#else
    gaprnt(0,"Error: This version of GrADS does not support shapefile output\n");
    return (1);
#endif
  }

  else if (cmpwrd("kml",cmd)) {
    kwrd = 116;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    pcm->kmlflg = 1;                               /* set default value to image output */
    while (cmpwrd("-img",cmd) || cmpwrd("-image",cmd) || 
	   cmpwrd("-ln",cmd) || cmpwrd("-line",cmd) || 
	   cmpwrd("-poly",cmd)) {
      if (cmpwrd("-img",cmd) || cmpwrd("-image",cmd)) pcm->kmlflg = 1;     /* image output */
      if (cmpwrd("-ln",cmd) || cmpwrd("-line",cmd))   pcm->kmlflg = 2;     /* contour output */
      if (cmpwrd("-poly",cmd)) pcm->kmlflg = 3;     /* polygon output */
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    }
    /* make sure TIFF output is enabled */
#if GEOTIFF!=1
    if (pcm->kmlflg==1) {
      gaprnt(0,"Error: Creating TIFF images for KML output is not supported in this build.\n");
      gaprnt(0,"  Try the \'-line\' option with \'set kml\' to output contour lines in KML format instead\n");  
      return(1);
    }
#endif
    /* Parse the KML output filename root */

    /* get the length of the user-supplied filename */
    itt = 0;
    while (*(cmd+itt)!=' '&&*(cmd+itt)!='\n'&&*(cmd+itt)!='\0') itt++;
    /* allocate memory for KML filename */
    sz = itt+6;
    ch = NULL;
    ch = (char *)galloc(sz,"kmlname");
    if (ch==NULL) {
      gaprnt (0,"Memory allocation error for KML file name\n"); goto err;
    }
    /* copy the user-supplied filename to ch */
    i2 = cmd - cmd1;
    for (i1=0; i1<itt; i1++) *(ch+i1) = *(com+i1+i2);
    /* check if user already put ".kml" onto the filename */
    if (*(ch+itt-4)=='.' &&  *(ch+itt-3)=='k' &&  *(ch+itt-2)=='m' &&  *(ch+itt-1)=='l') {
      *(ch+itt)='\0';      /* make sure it is null-terminated */
      itt = itt-4;         /* shorten the length in case we need the file root for TIFF image */
    }
    else {
      /* add ".kml" to the end of the filename */
      *(ch+i1+0)='.';
      *(ch+i1+1)='k';
      *(ch+i1+2)='m';
      *(ch+i1+3)='l';
      *(ch+i1+4)='\0';           
    }
    if (pcm->kmlflg==1) {
      /* A second file will be created by GrADS containing the TIFF image */
      ch2 = (char *)galloc(sz,"tifname");
      if (ch2==NULL) {
	gaprnt (0,"Memory allocation error for KML image file name\n");	goto err;
      }
      /* copy the user-supplied filename root to ch2 and add ".tif" */
      for (i1=0; i1<itt; i1++) *(ch2+i1) = *(com+i1+i2);
      *(ch2+i1+0) ='.';
      *(ch2+i1+1) ='t';
      *(ch2+i1+2) ='i';
      *(ch2+i1+3) ='f';
      *(ch2+i1+4) ='\0';           
    }
    /* release previously set filenames for KML output */
    if (pcm->kmlname) gree(pcm->kmlname,"f225d");
    pcm->kmlname = ch;
    if (pcm->kmlflg==1) {
      if (pcm->tifname) gree(pcm->tifname,"f225c");
      pcm->tifname = ch2;
      gaprnt (2,"KML output file names: \n");
      snprintf(pout,1255,"%s (TIFF image) \n",pcm->tifname);
      gaprnt (2,pout);
      snprintf(pout,1255,"%s (KML text file) \n",pcm->kmlname);
      gaprnt (2,pout);
      gaprnt (2,"KML output type: image\n");
    }
    else {
      gaprnt(2,"KML output file name: \n");
      snprintf(pout,1255,"%s (KML text file)\n",pcm->kmlname);
      gaprnt (2,pout);
      if (pcm->kmlflg==2)
	gaprnt (2,"KML output type: contour lines\n");
      else
	gaprnt (2,"KML output type: polygons\n");
    }
  }
  else if (cmpwrd("geotiff",cmd)) {
    kwrd = 115;
#if GEOTIFF==1
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    pcm->gtifflg = 1;                             /* set default value to 4-byte depth */
    while (cmpwrd("-dbl",cmd) || cmpwrd("-flt",cmd)) {
      if (cmpwrd("-flt",cmd) ) pcm->gtifflg = 1;  /* 4-byte depth in GeoTIFF output file */
      if (cmpwrd("-dbl",cmd) ) pcm->gtifflg = 2;  /* 8-byte depth in GeoTIFF output file */
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    }
    /* parse the geotiff output filename root -- .tif will be added by GrADS */
    /* get the length of the user-supplied filename */
    itt = 0;
    while (*(cmd+itt)!=' '&&*(cmd+itt)!='\n'&&*(cmd+itt)!='\0') itt++;
    /* allocate memory for GeoTIFF filename */
    sz = itt+6;
    ch = (char *)galloc(sz,"gtifname");
    if (ch==NULL) {
      gaprnt (0,"Memory allocation error for GeoTIFF file name\n"); goto err;
    }
    /* copy the user-supplied filename to ch */
    i2 = cmd - cmd1;
    for (i1=0; i1<itt; i1++) *(ch+i1) = *(com+i1+i2);
    /* check if user already put ".tif" onto the filename */
    if (*(ch+itt-4)=='.' &&  *(ch+itt-3)=='t' &&  *(ch+itt-2)=='i' &&  *(ch+itt-1)=='f') {
      *(ch+itt)='\0';      /* make sure it is null-terminated */
    }
    else {
      /* add ".tif" to the end of the filename */
      *(ch+i1+0)='.';
      *(ch+i1+1)='t';
      *(ch+i1+2)='i';
      *(ch+i1+3)='f';
      *(ch+i1+4)='\0';           
    }
    /* reset geotiff filename */
    if (pcm->gtifname) gree(pcm->gtifname,"f225b");
    pcm->gtifname = ch;
    snprintf(pout,1255,"GeoTIFF file name = %s\n",pcm->gtifname);
    gaprnt (2,pout);
    if (pcm->gtifflg==1) gaprnt(2,"GeoTIFF format is float  \n");
    if (pcm->gtifflg==2) gaprnt(2,"GeoTIFF format is double \n");
#else
    gaprnt(0,"Error: This version of GrADS does not support GeoTIFF output\n");
    return (1);
#endif
  }

  /* Following is for the so-called 'exact fwrite' to
     workaround the bug with scaling for hires files */
  else if (cmpwrd("fwex",cmd)) {
    pcm->fwexflg = 1;
  }
  else if (cmpwrd("fwrite",cmd)) {
    kwrd = 86;
    if (pcm->ffile) {
      gaprnt (0,"SET FWrite Error:  fwrite file is open\n");
      gaprnt (0,"Use DISABLE FWRITE command to close file\n");
    } else {
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      while ( cmpwrd("-be",cmd) || cmpwrd("-le",cmd) ||
	      cmpwrd("-ap",cmd) || cmpwrd("-cl",cmd) ||
              cmpwrd("-sq",cmd) || cmpwrd("-st",cmd) ||
              cmpwrd("-ex",cmd) ) {
        if (cmpwrd("-be",cmd) ) pcm->fwenflg = 1;
        if (cmpwrd("-le",cmd) ) pcm->fwenflg = 0;
        if (cmpwrd("-sq",cmd) ) pcm->fwsqflg = 1;
        if (cmpwrd("-st",cmd) ) pcm->fwsqflg = 0;
        if (cmpwrd("-ap",cmd) ) pcm->fwappend = 1;
        if (cmpwrd("-cl",cmd) ) pcm->fwappend = 0;
        if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      }
      itt = 0;
      while (*(cmd+itt)!=' '&&*(cmd+itt)!='\n'&&*(cmd+itt)!='\0') itt++;
      sz = itt+2;
      ch = (char *)galloc(sz,"fwrite1");
      if (ch==NULL) {
        gaprnt (0,"Memory allocation Error\n");
        goto err;
      }
      i2 = cmd - cmd1;
      for (i1=0; i1<itt; i1++) *(ch+i1) = *(com+i1+i2);
      *(ch+i1) = '\0';
      if (pcm->fwname) gree(pcm->fwname,"f225");
      pcm->fwname = ch;
      snprintf(pout,1255,"FWrite file name = %s\n",ch);
      gaprnt (2,pout);
      if (pcm->fwenflg == 0) {
        gaprnt (2,"FWwrite byte order is little_endian; format is ");
      } else {
        gaprnt (2,"FWwrite byte order is big_endian; format is ");
      }
      if (pcm->fwsqflg == 1) gaprnt (2,"sequential\n");
      else gaprnt (2,"stream\n");
      if (pcm->fwappend) {
	gaprnt (2,"Fwrite appending to an existing file\n");
      } else {
	gaprnt (2,"Fwrite replacing an existing file\n");
      }
    }
  }
  else if (cmpwrd("sdfwrite",cmd)) {
    kwrd = 113;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;  
    /* options for padding the output and setting a record dimension 
       are set to 'off' with each new call to 'set sdfwrite' */
    pcm->sdfwpad = 0;   
    pcm->sdfrecdim = 0;
    /* parse any arguments to 'set sdfwrite' command */
    while (cmpwrd("-4d",cmd)  || cmpwrd("-4de",cmd) || cmpwrd("-5d",cmd) || 
	   cmpwrd("-3dz",cmd) || cmpwrd("-3dt",cmd) ||
	   cmpwrd("-flt",cmd) || cmpwrd("-dbl",cmd) ||
	   cmpwrd("-nc3",cmd) || cmpwrd("-nc4",cmd) ||
	   cmpwrd("-rt",cmd)  || cmpwrd("-re",cmd) ||
	   cmpwrd("-zip",cmd) || cmpwrd("-chunk",cmd)) {
      if (cmpwrd("-3dz",cmd))   pcm->sdfwpad = 1;   
      if (cmpwrd("-3dt",cmd))   pcm->sdfwpad = 2;   
      if (cmpwrd("-4d",cmd))    pcm->sdfwpad = 3;   
      if (cmpwrd("-4de",cmd))   pcm->sdfwpad = 4;   
      if (cmpwrd("-5d",cmd))    pcm->sdfwpad = 5;
      if (cmpwrd("-dbl",cmd))   pcm->sdfprec = 8;
      if (cmpwrd("-flt",cmd))   pcm->sdfprec = 4;
      if (cmpwrd("-nc3",cmd))   pcm->sdfwtype = 1;
      if (cmpwrd("-nc4",cmd))   pcm->sdfwtype = 2;
      if (cmpwrd("-rt",cmd))    pcm->sdfrecdim+=1;
      if (cmpwrd("-re",cmd))    pcm->sdfrecdim+=2;
      if (cmpwrd("-zip",cmd))   pcm->sdfzip = 1;
      if (cmpwrd("-chunk",cmd)) pcm->sdfchunk = 1;

      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    }
    /* parse the sdfwrite output filename */
    itt = 0;
    while (*(cmd+itt)!=' '&&*(cmd+itt)!='\n'&&*(cmd+itt)!='\0') itt++;
    sz = itt+2;
    ch = (char *)galloc(sz,"sdfwname");
    if (ch==NULL) {
      gaprnt (0,"Memory allocation error for sdfwrite file name\n");
      goto err;
    }
    i2 = cmd - cmd1;
    for (i1=0; i1<itt; i1++) *(ch+i1) = *(com+i1+i2);
    *(ch+i1) = '\0';
    /* release previously set sdfwrite filename */
    if (pcm->sdfwname) gree(pcm->sdfwname,"f225a");
    pcm->sdfwname = ch;
    snprintf(pout,1255,"SDFWrite file name = %s\n",ch);
    gaprnt (2,pout);
    gaprnt (2,"SDFWrite will replace an existing file\n");
    if (pcm->sdfwpad==0)
     gaprnt (2,"SDFwrite file will have the same number of dimensions as the defined variable\n");
    if (pcm->sdfwpad==1)
      gaprnt (2,"SDFwrite file will have 3 dimensions: lon, lat, and lev\n");
    if (pcm->sdfwpad==2)
      gaprnt (2,"SDFwrite file will have 3 dimensions: lon, lat, and time\n");
    if (pcm->sdfwpad==3) 
      gaprnt (2,"SDFwrite file will have 4 dimensions: lon, lat, lev, and time\n");
    if (pcm->sdfwpad==4)
      gaprnt (2,"SDFwrite file will have 4 dimensions: lon, lat, time, and ens\n");
    if (pcm->sdfwpad==5)
      gaprnt (2,"SDFwrite file will have 5 dimensions: lon, lat, lev, time, and ens\n");
#if HAVENETCDF4 != 1
    /* reset flags if we dont' have netcdf-4 */
    if (pcm->sdfwtype == 2) {
      gaprnt(2,"Warning: This build is not enabled to write netCDF-4 format\n");
      gaprnt(2,"         sdfwrite output will be netCDF classic format instead\n");
      pcm->sdfwtype = 1;
      if (pcm->sdfchunk==1) {
	gaprnt(2,"         without chunking\n");
	pcm->sdfchunk = 0;
      }
      if (pcm->sdfzip==1) {
	gaprnt(2,"         without compression\n");
	pcm->sdfzip = 0;
      }
    }
#endif
    /* check for two record dimensions */
    if (pcm->sdfrecdim==3 && pcm->sdfwtype==1) {
      gaprnt(0,"Only one record dimension is allowed with netCDF classic format. \n");
      gaprnt(0,"  Use the \"-nc4\" option to enable multiple record dimensions. \n");
      goto err;
    }
    if (pcm->sdfrecdim==1) 
      gaprnt (2,"SDFwrite file will have 1 record dimension: time\n");
    else if (pcm->sdfrecdim==2)
      gaprnt (2,"SDFwrite file will have 1 record dimension: ens\n");
    else if (pcm->sdfrecdim==3)
      gaprnt (2,"SDFwrite file will have 2 record dimensions: time and ens\n");
    else
      gaprnt (2,"SDFwrite file will not have a record dimension\n");

    /* If -zip, make sure we have -nc4 and -chunk */
    if (pcm->sdfzip) {
      if (pcm->sdfwtype==1) {
	gaprnt(0,"Compression is not available with netCDF classic format. \n");
	gaprnt(0,"  Use the \"-nc4\" option to enable compression or \n");
	gaprnt(0,"  reset the compression option with \"clear sdfwrite\". \n");
	goto err;
      }
      pcm->sdfwtype = 2;
      pcm->sdfchunk = 1;
    }

  }
  else if (cmpwrd("sdfattr",cmd)) {
    kwrd = 114;
    com=nxtwrd(com); /* advance past the 'sdfattr' in the mixed case version */
    if ((com = nxtwrd (com)) == NULL) goto err;
    /* parse the sdf attribute */
    newattr = parseattr(com);
    if (newattr==NULL) goto err;
    /* hang the new attribute off the gacmn structure */
    if (pcm->attr) {
      attr=pcm->attr;
      while (attr->next) attr = attr->next;      /* advance to end of chain */
      attr->next = newattr;                      /* add new link */
    }
    else {
      pcm->attr = newattr;                       /* set first link */
    } 
  }
#if USESHP==1
  else if (cmpwrd("shpattr",cmd)) {
    kwrd = 122;
    /* advance past the 'set shpattr' in the mixed case version */
    com=nxtwrd(com); 
    if ((com = nxtwrd (com)) == NULL) goto err;
    /* parse the attribute (data base field) */
    newfld = parsedbfld(com);
    if (newfld==NULL) goto err;
    /* hang the new field off the gacmn structure */
    if (pcm->dbfld) {
      fld=pcm->dbfld;
      while (fld->next) fld = fld->next;      /* advance to end of chain */
      fld->next = newfld;                     /* add new link */
    }
    else {
      pcm->dbfld = newfld;                       /* set first link */
    } 
  }
#endif
#if HAVENETCDF4 == 1
  else if (cmpwrd("chunksize",cmd)) {
    kwrd = 118;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL) goto err;
    pcm->xchunk = itt;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt) == NULL) goto err;
      pcm->ychunk = itt;
      if ((cmd = nxtwrd (cmd)) != NULL) {
	if (intprs(cmd,&itt) == NULL) goto err;
	pcm->zchunk = itt;
	if ((cmd = nxtwrd (cmd)) != NULL) {
	  if (intprs(cmd,&itt) == NULL) goto err;
	  pcm->tchunk = itt;
	  if ((cmd = nxtwrd (cmd)) != NULL) {
	    if (intprs(cmd,&itt) == NULL) goto err;
	    pcm->echunk = itt;
	  }
	}
      }
    }
  }
#endif
  else if (cmpwrd("cachesf",cmd)) {
    kwrd = 119;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL) goto err;
    pcm->cachesf = (long)itt;
    setcachesf(pcm->cachesf); /* update gaio.c */
    snprintf(pout,1255,"Global cache scale factor is %ld\n",pcm->cachesf);
    gaprnt(2,pout);
  }
  else if (cmpwrd("imprun",cmd)) {
    kwrd = 62;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) {
      if (pcm->impflg) gree(pcm->impnam,"f226");
      pcm->impflg = 0;
      gaprnt (2,"IMPrun is off\n");
    } else {
      itt = 0;
      while (*(cmd+itt)!=' '&&*(cmd+itt)!='\n'&&*(cmd+itt)!='\0') itt++;
      sz = itt+6;
      ch = (char *)galloc(sz,"imprun");
      if (ch==NULL) {
        gaprnt (0,"Memory allocation Error\n");
        goto err;
      }
      for (i1=0; i1<itt; i1++) *(ch+i1+4) = *(cmd+i1);
      *(ch+i1+4) = '\0';
      *ch='r'; *(ch+1)='u'; *(ch+2)='n'; *(ch+3)=' ';
      if (pcm->impflg) gree(pcm->impnam,"f227");
      pcm->impflg = 1;
      pcm->impnam = ch;
      snprintf(pout,1255,"Imprun file name = %s\n",ch);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd("log1d",cmd)) {
    kwrd = 123;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("loglog",cmd)) pcm->log1d = 3;
    else if (cmpwrd("logh",cmd)) pcm->log1d = 1;
    else if (cmpwrd("logv",cmd)) pcm->log1d = 2;
    else if (cmpwrd("off",cmd)) pcm->log1d = 0;
    else goto err;
  }
  else if (cmpwrd("zlog",cmd)) {
    kwrd = 63;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->zlog = 1;
    else if (cmpwrd("off",cmd)) pcm->zlog = 0;
    else goto err;
  }
  else if (cmpwrd("coslat",cmd)) {
    kwrd = 110;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->coslat = 1;
    else if (cmpwrd("off",cmd)) pcm->coslat = 0;
    else goto err;
  }
  else if (cmpwrd("missconn",cmd)) {
    kwrd = 61;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->miconn = 1;
    else if (cmpwrd("off",cmd)) pcm->miconn = 0;
    else goto err;
  }
  else if (cmpwrd("mpdraw",cmd)) {
    kwrd = 50;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->mpdraw = 1;
    else if (cmpwrd("off",cmd)) pcm->mpdraw = 0;
    else goto err;
  }
  else if (cmpwrd("dbuff",cmd)) {
    kwrd = 52;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) {
      pcm->dbflg = 1;
      gxfrme(2);
      gacln(pcm,1);
    }
    else if (cmpwrd("off",cmd)) {
      if (pcm->dbflg) {
        pcm->dbflg = 0;
        gxfrme(1);
        gacln (pcm,1);
      }
    }
    else goto err;
  }
  else if (cmpwrd("poli",cmd)) {
    kwrd = 51;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) {
      if (pcm->mpcols[1] == -9) pcm->mpcols[1] = -1;
      if (pcm->mpcols[2] == -9) pcm->mpcols[2] = -1;
    }
    else if (cmpwrd("off",cmd)) {
      pcm->mpcols[1] = -9;
      pcm->mpcols[2] = -9;
    }
    else goto err;
  }
  else if (cmpwrd("mpdset",cmd)) {
    kwrd = 38;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    for (xx=0; xx<8; xx++) {
      if (pcm->mpdset[xx]) gree(pcm->mpdset[xx],"f228");
      pcm->mpdset[xx] = NULL;
    }
    xx = 0;
    itt = 0;
    while (xx<8) {
      itt2 = itt;
      while (*(cmd+itt)!=' '&&*(cmd+itt)!='\n'&&*(cmd+itt)!='\0') itt++;
      sz = itt+2-itt2;
      ch = (char *)galloc(sz,"setmpds");
      if (ch==NULL) {
        gaprnt (0,"Memory allocation Error\n");
        goto err;
      }
      for (i1=itt2; i1<itt; i1++) *(ch+i1-itt2) = *(cmd+i1);
      *(ch+i1-itt2) = '\0';
      pcm->mpdset[xx] = ch;
      snprintf(pout,1255,"MPDSET file name = %s\n",ch);
      gaprnt (2,pout);
      while (*(cmd+itt)==' ') itt++;
      if (*(cmd+itt)=='\n'||*(cmd+itt)=='\0') break;
      xx++;
    }
  }
  else if (cmpwrd("rbrange",cmd)) {
    kwrd = 60;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&val1) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&val2) == NULL ) goto err;
    if (val1>=val2) goto err;
    pcm->rainmn = val1;
    pcm->rainmx = val2;
  }
  else if (cmpwrd("black",cmd)) {
    kwrd = 22;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("off",cmd)) pcm->blkflg = 0;
    else {
      if (getdbl(cmd,&(pcm->blkmin)) == NULL ) goto err;
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      if (getdbl(cmd,&(pcm->blkmax)) == NULL ) goto err;
      pcm->blkflg = 1;
    }
  }
  else if (cmpwrd("display",cmd)) {
    kwrd = 57;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if      (cmpwrd("color",cmd) || cmpwrd("grey",cmd) || cmpwrd("greyscale",cmd)) itt=2; /* this will be a no-op */
    else if (cmpwrd("white",cmd)) itt=1; 
    else if (cmpwrd("black",cmd)) itt=0; 
    else goto err;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if      (cmpwrd("white",cmd)) itt = 1;
      else if (cmpwrd("black",cmd)) itt = 0;
      else goto err;
    }
    if (itt==0 || itt==1) gxdbck(itt);
  }
  else if (cmpwrd("gxout",cmd)) {
    kwrd = 21;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    pcm->gout0 = 9;
    if      (cmpwrd("contour",cmd))  pcm->gout2a = 1;
    else if (cmpwrd("shaded",cmd))   pcm->gout2a = 16;
    else if (cmpwrd("shade1",cmd))   pcm->gout2a = 2;
    else if (cmpwrd("shade2",cmd))   pcm->gout2a = 16;
    else if (cmpwrd("shade2b",cmd))  pcm->gout2a = 17;
    else if (cmpwrd("grid",cmd))    {pcm->gout2a = 3; pcm->gout2b = 3;}
    else if (cmpwrd("vector",cmd))  {pcm->gout2b = 4; pcm->goutstn = 6; pcm->gout1a = 1;}
    else if (cmpwrd("scatter",cmd))  pcm->gout2b = 5;
    else if (cmpwrd("fgrid",cmd))    pcm->gout2a = 6;
    else if (cmpwrd("fwrite",cmd))   pcm->gout2a = 7;
    else if (cmpwrd("stream",cmd))   pcm->gout2b = 8;
    else if (cmpwrd("grfill",cmd))   pcm->gout2a = 10;
    else if (cmpwrd("geotiff",cmd)) {
#if GEOTIFF==1
      pcm->gout2a = 12; 
#else
      gaprnt(0,"Creating GeoTIFF files is not supported in this build\n");
      goto err;
#endif
    }
    else if (cmpwrd("kml",cmd))      pcm->gout2a = 13; 
    else if (cmpwrd("imap",cmd))     pcm->gout2a = 14; 
    else if (cmpwrd("shp",cmd)) {
#if USESHP==1
      pcm->gout2a = 15; pcm->goutstn = 9;}
#else
      gaprnt(0,"Creating shapefiles is not supported in this build\n");
      goto err;}
#endif
    else if (cmpwrd("value",cmd))    pcm->goutstn = 1;
    else if (cmpwrd("barb",cmd))    {pcm->goutstn = 2; pcm->gout2b = 9; pcm->gout1a = 2;}
    else if (cmpwrd("findstn",cmd))  pcm->goutstn = 3;
    else if (cmpwrd("model",cmd))    pcm->goutstn = 4;
    else if (cmpwrd("wxsym",cmd))    pcm->goutstn = 5;
    else if (cmpwrd("stnmark",cmd))  pcm->goutstn = 7;
    else if (cmpwrd("stnwrt",cmd))   pcm->goutstn = 8;  /* undocumented */
    else if (cmpwrd("line",cmd))    {pcm->gout1 = 1; pcm->tser = 0;}
    else if (cmpwrd("bar",cmd))      pcm->gout1 = 2;
    else if (cmpwrd("errbar",cmd))   pcm->gout1 = 3;
    else if (cmpwrd("linefill",cmd)) pcm->gout1 = 4;
    else if (cmpwrd("stat",cmd))     pcm->gout0 = 1;
    else if (cmpwrd("print",cmd))    pcm->gout0 = 2;
    else if (cmpwrd("writegds",cmd)) pcm->gout0 = 3;    /* only for internal use */
    else if (cmpwrd("tserwx",cmd))   pcm->tser = 1;
    else if (cmpwrd("tserbarb",cmd)) pcm->tser = 2;
    else goto err;
    if (pcm->gout0==9) pcm->gout0 = 0;
  }
  else if (cmpwrd("arrscl",cmd)) {
    kwrd = 27;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->arrsiz)) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) {
      pcm->arrmag = -999.0;
    } else {
      if (getdbl(cmd,&(pcm->arrmag)) == NULL ) goto err;
    }
    pcm->arrflg = 1;
  }
  else if (cmpwrd("xlabs",cmd)||cmpwrd("ylabs",cmd)) {
    if (cmpwrd("xlabs",cmd)) {kwrd=94; strng=pcm->xlabs;}
    if (cmpwrd("ylabs",cmd)) {kwrd=95; strng=pcm->ylabs;}
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    i1 = 0;
    if (cmpwrd("off",cmd)) {
      if (strng) gree(strng,"f229");
      strng = NULL;
    } else {
      com = nxtwrd(com);
      com = nxtwrd(com);
      num = 0;
      while(*(com+num)!='\0' && *(com+num)!='\n') num++;
      if (strng) gree(strng,"f230");
      sz = num+2;
      strng = (char *)galloc(sz,"xlabs");
      if (strng==NULL) {
        gaprnt(0,"Memory Allocation Error: Set XLABS/YLABS\n");
        goto err;
      }
      num = 0;
      while(*(com+num)!='\0' && *(com+num)!='\n') {
        *(strng+num) = *(com+num);
        if (*(strng+num)=='|') {
          *(strng+num) = '\0';
          i1++;
        }
        num++;
      }
      *(strng+num) = '\0';
      i1++;
    }
    if (kwrd==94) { pcm->xlabs=strng; pcm->ixlabs = i1; }
    if (kwrd==95) { pcm->ylabs=strng; pcm->iylabs = i1; }
  }
  else if (cmpwrd("clab",cmd)||cmpwrd("xlab",cmd)||cmpwrd("ylab",cmd)) {
    if (cmpwrd("clab",cmd)) {kwrd=32; strng=pcm->clstr; i1=pcm->clab;}
    if (cmpwrd("xlab",cmd)) {kwrd=73; strng=pcm->xlstr; i1=pcm->xlab;}
    if (cmpwrd("ylab",cmd)) {kwrd=74; strng=pcm->ylstr; i1=pcm->ylab;}
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) i1 = 1;
    else if (cmpwrd("off",cmd)) i1 = 0;
    else if (cmpwrd("forced",cmd)) i1 = 2;
    else if (cmpwrd("masked",cmd)) i1 = 3;
    else if (cmpwrd("auto",cmd)) {
      if (strng) gree(strng,"f231");
      strng = NULL;
    }
    else {
      com = nxtwrd(com);
      com = nxtwrd(com);
      num = 0;
      while(*(com+num)!='\0' && *(com+num)!='\n') num++;
      if (strng) gree(strng,"f232");
      sz = num+2;
      strng = (char *)galloc(sz,"clab1");
      if (strng==NULL) {
        gaprnt(0,"Memory Allocation Error: Set ?LAB\n");
        goto err;
      }
      num = 0;
      while(*(com+num)!='\0' && *(com+num)!='\n') {
        *(strng+num) = *(com+num);
        num++;
      }
      *(strng+num) = '\0';
      gaprnt (2,"Substitution string is: ");
      gaprnt (2,strng);
      gaprnt (2,"\n");
    }
    if (kwrd==32) {pcm->clstr=strng; pcm->clab=i1;}
    if (kwrd==73) {pcm->xlstr=strng; pcm->xlab=i1;}
    if (kwrd==74) {pcm->ylstr=strng; pcm->ylab=i1;}
  }
  else if (cmpwrd("prnopts",cmd)) {
    kwrd = 106;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    com = nxtwrd(com);
    com = nxtwrd(com);
    num = 0;
    while(*(com+num)!='\0' && *(com+num)!='\n' && *(com+num)!=' ') num++;
    if (pcm->prstr) gree(pcm->prstr,"f233");
    sz = num+2;
    strng = (char *)galloc(sz,"prnopts");
    if (strng==NULL) {
      gaprnt(0,"Memory Allocation Error: Set PRNOPTS\n");
      goto err;
    }
    num = 0;
    while(*(com+num)!='\0' && *(com+num)!='\n' && *(com+num)!=' ') {
      *(strng+num) = *(com+num);
      num++;
    }
    *(strng+num) = '\0';
    pcm->prstr = strng;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt) == NULL ) goto err;
      pcm->prlnum = itt;
      if ((cmd = nxtwrd (cmd)) != NULL) {
        if (intprs(cmd,&itt) == NULL ) goto err;
        pcm->prbnum = itt;
        if ((cmd = nxtwrd (cmd)) != NULL) {
          pcm->prudef = 0;
          if (*cmd=='u') pcm->prudef = 1;
        }
      }
    }
  }
  else if (cmpwrd("frame",cmd)) {
    kwrd = 65;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->frame = 1;
    else if (cmpwrd("off",cmd)) pcm->frame = 0;
    else if (cmpwrd("circle",cmd)) pcm->frame = 2;
    else goto err;
  }
  else if (cmpwrd("grid",cmd)) {
    kwrd = 17;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->grflag = 1;
    else if (cmpwrd("off",cmd)) pcm->grflag = 0;
    else if (cmpwrd("horizontal",cmd)) pcm->grflag = 2;
    else if (cmpwrd("vertical",cmd)) pcm->grflag = 3;
    else goto err;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt) == NULL ) goto err;
      else pcm->grstyl = itt;
      if ((cmd = nxtwrd (cmd)) != NULL) {
        if (intprs(cmd,&itt) == NULL ) goto err;
        else pcm->grcolr = itt;
	if ((cmd = nxtwrd (cmd)) != NULL) {
	  if (intprs(cmd,&itt) == NULL ) goto err;
	  else pcm->grthck = itt;
	}
      }
    }
    if (pcm->grflag) {
      snprintf(pout,1255,"grid is on, style %i color %i thickness %d\n",pcm->grstyl,pcm->grcolr,pcm->grthck);
      gaprnt (2,pout);
    } else {
      gaprnt (2,"grid is off\n");
    }
  }
  else if (cmpwrd("clskip",cmd)) {
    kwrd = 89;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    if (itt<1) goto err;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (getdbl(cmd,&val1) == NULL ) goto err;  
      gxclmn(val1);
    }
    pcm->clskip = itt;
  }
  else if (cmpwrd("clopts",cmd)) {
    kwrd = 72;
    itt = pcm->clcol;
    itt1 = pcm->clthck;
    val1 = pcm->clsiz;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt1) == NULL ) goto xlerr;
      else {
        if ((cmd = nxtwrd (cmd)) != NULL) {
          if (getdbl(cmd,&val1) == NULL ) goto xlerr;
        }
      }
    }
    pcm->clcol = itt;
    pcm->clthck = itt1;
    pcm->clsiz = val1;
    snprintf(pout,1255,"SET CLOPTS values:  Color = %i Thickness = %i", pcm->clcol, pcm->clthck);
    gaprnt (2,pout);
    snprintf(pout,1255," Size = %g\n",pcm->clsiz);
    gaprnt (2,pout);
  }
  else if (cmpwrd("shpopts",cmd)) {
    kwrd=120;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    pcm->fillpoly = itt;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt) == NULL ){
        gaprnt(0,"SET SHPOPTS Error: Invalid mark type\n");
      } else {
        pcm->marktype = itt;
        if ((cmd = nxtwrd (cmd)) != NULL) {
          if (getdbl(cmd,&val1) == NULL ){
            gaprnt(0,"SET SHPOPTS Error: Invalid mark size\n");
          } else {
            pcm->marksize = val1;
          }
        }
      }
    }
    snprintf(pout,1255,"SET SHPOPTS values:  polygon fill color = %i  ",pcm->fillpoly);
    gaprnt (2,pout);
    snprintf(pout,1255,"mark type = %i  ",pcm->marktype);
    gaprnt (2,pout);
    snprintf(pout,1255,"mark size = %g \n",pcm->marksize);
    gaprnt (2,pout);
  }
  else if (cmpwrd("wxopt",cmd)) {
    kwrd = 93;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    else if (cmpwrd("wxsym",cmd)) pcm->wxopt = 1;
    else if (cmpwrd("mark",cmd)) pcm->wxopt = 2;
    else if (cmpwrd("char",cmd)) pcm->wxopt = 3;
    else goto err;
  }
  else if (cmpwrd("wxcols",cmd)) {
    kwrd = 84;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,itmp) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,itmp+1) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,itmp+2) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,itmp+3) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,itmp+4) == NULL ) goto err;
    for (i1=0; i1<5; i1++) pcm->wxcols[i1] = itmp[i1];
    gaprnt (2,"New WXCOLS have been set\n");
  }
  else if (cmpwrd("lfcols",cmd)) {
    kwrd = 83;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt1) == NULL ) goto err;
    pcm->lfc1 = itt;
    pcm->lfc2 = itt1;
    snprintf(pout,1255,"LineFill Colors: Above = %i  Below = %i\n",
      pcm->lfc1, pcm->lfc2);
    gaprnt (2,pout);
  }
  else if (cmpwrd("rband",cmd)) {
    kwrd = 90;
    itt = -1;
    if ((cmd = nxtwrd (cmd)) == NULL) goto rbberr;
    if (intprs(cmd,&i1) == NULL ) goto rbberr;
    if ((cmd = nxtwrd (cmd)) == NULL) goto rbberr;
    if (*cmd=='m' && *(cmd+1)=='b') {
      cmd += 2;
      if (intprs(cmd,&(itt)) == NULL ) goto rbberr;
      if ((cmd = nxtwrd (cmd)) == NULL) goto rbberr;
      if (itt>3) itt = 3;
    }
    if (cmpwrd("box",cmd)) i2 = 1;
    else if (cmpwrd("line",cmd)) i2 = 2;
    else goto rbberr;
    if ((cmd = nxtwrd (cmd)) == NULL) goto rbberr;
    if (getdbl(cmd,&xlo) == NULL ) goto rbberr;
    if ((cmd = nxtwrd (cmd)) == NULL) goto rbberr;
    if (getdbl(cmd,&ylo) == NULL ) goto rbberr;
    if ((cmd = nxtwrd (cmd)) == NULL) goto rbberr;
    if (getdbl(cmd,&xhi) == NULL ) goto rbberr;
    if ((cmd = nxtwrd (cmd)) == NULL) goto rbberr;
    if (getdbl(cmd,&yhi) == NULL ) goto rbberr;
    dsubs->gxdrbb (i1,i2,xlo,ylo,xhi,yhi,itt);
  }
  else if (cmpwrd("button",cmd)) {
    kwrd = 80;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    pcm->btnfc = itt;
    pcm->btnftc = itt;
    if ((cmd = nxtwrd (cmd)) != NULL) {
     if (intprs(cmd,&itt) == NULL ) goto err;
     pcm->btnbc = itt;
     pcm->btnbtc = itt;
     if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt) == NULL ) goto err;
      pcm->btnoc = itt;
      pcm->btnoc2 = itt;
      pcm->btnotc = itt;
      pcm->btnotc2 = itt;
      if ((cmd = nxtwrd (cmd)) != NULL) {
       if (intprs(cmd,&itt) == NULL ) goto err;
       pcm->btnoc2 = itt;
       pcm->btnotc2 = itt;
       if ((cmd = nxtwrd (cmd)) != NULL) {
        if (intprs(cmd,&itt) == NULL ) goto err;
        pcm->btnftc = itt;
        if ((cmd = nxtwrd (cmd)) != NULL) {
         if (intprs(cmd,&itt) == NULL ) goto err;
         pcm->btnbtc = itt;
         if ((cmd = nxtwrd (cmd)) != NULL) {
          if (intprs(cmd,&itt) == NULL ) goto err;
          pcm->btnotc = itt;
          pcm->btnotc2 = itt;
          if ((cmd = nxtwrd (cmd)) != NULL) {
           if (intprs(cmd,&itt) == NULL ) goto err;
           pcm->btnotc2 = itt;
           if ((cmd = nxtwrd (cmd)) != NULL) {
            if (intprs(cmd,&itt) == NULL ) goto err;
            pcm->btnthk = itt;
           }
          }
         }
        }
       }
      }
     }
    }
    snprintf(pout,1255,"SET BUTTON values:  Fc, Bc, Oc, Oc2 = %i %i %i %i ",
      pcm->btnfc,pcm->btnbc,pcm->btnoc,pcm->btnoc2);
    gaprnt (2,pout);
    snprintf(pout,1255,"Toggle Fc, Bc, Oc, Oc2 = %i %i %i %i ",
      pcm->btnftc,pcm->btnbtc,pcm->btnotc,pcm->btnotc2);
    snprintf(pout,1255,"Thick = %i\n",pcm->btnthk);
    gaprnt (2,pout);
  }
  else if (cmpwrd("dialog",cmd)) {
    kwrd = 108;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    pcm->dlgpc = itt;    
    if ((cmd = nxtwrd (cmd)) != NULL) {
     if (intprs(cmd,&itt) == NULL ) goto err;
     pcm->dlgfc = itt;     
     if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt) == NULL ) goto err;
      pcm->dlgbc = itt;      
      if ((cmd = nxtwrd (cmd)) != NULL) {
       if (intprs(cmd,&itt) == NULL ) goto err;
       pcm->dlgoc = itt;       
       if ((cmd = nxtwrd (cmd)) != NULL) {
        if (intprs(cmd,&itt) == NULL ) goto err;
        pcm->dlgth = itt;        
        if ((cmd = nxtwrd (cmd)) != NULL) {
         if (cmpwrd("n",cmd)||cmpwrd("numeric",cmd)) pcm->dlgnu = 1;       
        } else pcm->dlgnu = 0;
       }
      }
     }
    }
    snprintf(pout,1255,"SET DIALOG values:  Pc, Fc, Bc, Oc = %i %i %i %i ",
      pcm->dlgpc,pcm->dlgfc,pcm->dlgbc,pcm->dlgoc);
    gaprnt (2,pout);
    if (pcm->dlgnu) {
      snprintf(pout,1255,"Thick = %i ",pcm->dlgth);
      gaprnt (2,pout);
      snprintf(pout,1255,"Args = numeric\n ");
      gaprnt (2,pout);
    } else {
      snprintf(pout,1255,"Thick = %i\n",pcm->dlgth);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd("xlpos",cmd)) {
    kwrd = 87;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&val1) == NULL ) goto xlerr2;
    pcm->xlpos = val1;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (cmpwrd("b",cmd)||cmpwrd("bottom",cmd)) pcm->xlside = 0;
      if (cmpwrd("t",cmd)||cmpwrd("top",cmd)) pcm->xlside = 1;
    }
    if (pcm->xlside)
      snprintf(pout,1255,"SET XLPOS values:  Offset = %g  Side = Top\n",pcm->xlpos);
    else
      snprintf(pout,1255,"SET XLPOS values:  Offset = %g  Side = Bottom\n",pcm->xlpos);
    gaprnt (2,pout);
  }
  else if (cmpwrd("ylpos",cmd)) {
    kwrd = 88;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&val1) == NULL ) goto xlerr2;
    pcm->ylpos = val1;
    pcm->ylpflg = 1;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (cmpwrd("r",cmd)||cmpwrd("right",cmd)) pcm->ylside = 1;
      if (cmpwrd("l",cmd)||cmpwrd("left",cmd)) pcm->ylside = 0;
    }
    snprintf(pout,1255,"SET YLPOS values:  Offset = %g  Side = ",tt);
    gaprnt (2,pout);
    if (pcm->ylside) gaprnt(2,"Right\n");
    else gaprnt(2,"Left\n");
  }
  else if (cmpwrd("xlopts",cmd)) {
    kwrd = 70;
    itt  = pcm->xlcol;
    itt1 = pcm->xlthck;
    val1 = pcm->xlsiz;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt1) == NULL ) goto xlerr;
      else {
        if ((cmd = nxtwrd (cmd)) != NULL) {
          if (getdbl(cmd,&val1) == NULL ) goto xlerr;
        }
      }
    }
    pcm->xlcol  = itt;
    pcm->xlthck = itt1;
    pcm->xlsiz  = val1;
    snprintf(pout,1255,"SET XLOPTS values:  Color = %i  Thickness = %i  Size = %g \n",
	     pcm->xlcol, pcm->xlthck, pcm->xlsiz);
    gaprnt (2,pout);
  }
  else if (cmpwrd("ylopts",cmd)) {
    kwrd = 71;
    itt  = pcm->ylcol;
    itt1 = pcm->ylthck;
    val1 = pcm->ylsiz;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt1) == NULL ) goto xlerr;
      else {
        if ((cmd = nxtwrd (cmd)) != NULL) {
          if (getdbl(cmd,&val1) == NULL ) goto xlerr;
        }
      }
    }
    pcm->ylcol  = itt;
    pcm->ylthck = itt1;
    pcm->ylsiz  = val1;
    snprintf(pout,1255,"SET YLOPTS values:  Color = %i  Thickness = %i   Size = %g \n",
	     pcm->ylcol, pcm->ylthck, pcm->ylsiz);
    gaprnt (2,pout);
  }
  else if (cmpwrd("annot",cmd)) {
    kwrd = 56;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    pcm->anncol = itt;
    pcm->xlcol  = itt;  /* set annot should change color of tic labels too */
    pcm->ylcol  = itt;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt) == NULL ) {
        gaprnt(0,"SET ANNOT Error: Invalid thickness value\n");
      } else {
        pcm->annthk = itt;
      }
    }
    snprintf(pout,1255,"SET ANNOT values:  Color = %i  Thickness = %i\n",
	     pcm->anncol, pcm->annthk);
    gaprnt (2,pout);
  }
  else if (cmpwrd("line",cmd)) {
    kwrd = 41;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    pcm->lincol = itt;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (intprs(cmd,&itt) == NULL ){
        gaprnt(0,"SET LINE Error: Invalid linestyle value\n");
      } else {
        pcm->linstl = itt;
        if ((cmd = nxtwrd (cmd)) != NULL) {
          if (intprs(cmd,&itt) == NULL ){
            gaprnt(0,"SET LINE Error: Invalid thickness value\n");
          } else {
            pcm->linthk = itt;
          }
        }
      }
    }
    snprintf(pout,1255,"SET LINE values:  color = %i  style = %i  thickness = %i\n",
	     pcm->lincol, pcm->linstl, pcm->linthk);
    gaprnt (2,pout);
  }
  else if (cmpwrd("map",cmd)) {
    kwrd = 46;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("auto",cmd)) {
      pcm->mapcol = -9;
    } else {
      if (intprs(cmd,&itt) == NULL ) goto err;
      pcm->mapcol = itt;
      if ((cmd = nxtwrd (cmd)) != NULL) {
        if (intprs(cmd,&itt) == NULL ){
          gaprnt(0,"SET MAP Error: Invalid linestyle value\n");
        } else {
          pcm->mapstl = itt;
          if ((cmd = nxtwrd (cmd)) != NULL) {
            if (intprs(cmd,&itt) == NULL ){
              gaprnt(0,"SET MAP Error: Invalid thickness value\n");
            } else {
              pcm->mapthk = itt;
            }
          }
        }
      }
    }
    if (pcm->mapcol < 0 ) {
      gaprnt (2,"SET MAP values:  auto\n");
    } else {
      snprintf(pout,1255,"SET MAP values:  color = %i  style = %i  thickness = %i\n",
	       pcm->mapcol, pcm->mapstl, pcm->mapthk);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd("string",cmd)) {
    kwrd = 42;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    pcm->strcol = itt;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      itt = -1;
      for (i1=0; i1<9; i1++) if (cmpwrd(justs[i1],cmd)) itt=i1;
      if (itt<0) {
        gaprnt(0,"SET STRING Error: Invalid justification value\n");
      } else {
        pcm->strjst = itt;
        if ((cmd = nxtwrd (cmd)) != NULL) {
          if (intprs(cmd,&itt) == NULL ){
            gaprnt(0,"SET STRING Error: Invalid thickness value\n");
          } else {
            pcm->strthk = itt;
            if ((cmd = nxtwrd (cmd)) != NULL) {
              if (getdbl(cmd,&val1) == NULL ){
                gaprnt(0,"SET STRING Error: Invalid rotation value\n");
              } else {
                pcm->strrot = val1;
              }
            }
          }
        }
      }
    }
    snprintf(pout,1255,"SET STRING values:  color = %i  just = %s",
      pcm->strcol, justs[pcm->strjst]);
    gaprnt (2,pout);
    snprintf(pout,1255,"  thickness = %i  rotation = %g\n",
      pcm->strthk, pcm->strrot);
    gaprnt (2,pout);
  }
  else if (cmpwrd("strsiz",cmd)) {
    kwrd = 43;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&val1) == NULL ) goto err;
    pcm->strhsz = val1;
    i1 = 1;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (getdbl(cmd,&val1) == NULL ){
        gaprnt(0,"SET STRSIZ Error:  Invalid vsize value\n");
      } else {pcm->strvsz = val1; i1 = 0;}
    }
    if (i1) pcm->strvsz = pcm->strhsz;
    snprintf(pout,1255,"SET STRSIZ values:  hsize = %g  vsize = %g\n",
      pcm->strhsz, pcm->strvsz);
    gaprnt (2,pout);
  }
  else if (cmpwrd("xaxis",cmd)) {
    kwrd = 19;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->axmin)) == NULL) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->axmax)) == NULL) goto err;
    pcm->axflg = 1;
    pcm->axint = 0.0;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (getdbl(cmd,&v1) == NULL) goto err;
      else pcm->axint = v1;
    }
    snprintf(pout,1255,"xaxis labels range %g %g incr %g \n",pcm->axmin,pcm->axmax,pcm->axint);
    gaprnt (2,pout);
  }
  else if (cmpwrd("yaxis",cmd)) {
    kwrd = 20;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->aymin)) == NULL) goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->aymax)) == NULL) goto err;
    pcm->ayflg = 1;
    pcm->ayint = 0.0;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (getdbl(cmd,&v1) == NULL) goto err;
      else pcm->ayint = v1;
    }
    snprintf(pout,1255,"yaxis labels range %g %g incr %g \n",pcm->aymin,pcm->aymax,pcm->ayint);
    gaprnt (2,pout);
  }
  else if (cmpwrd("misswarn",cmd)) {
    kwrd = 79;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) itt = 1;
    else if (cmpwrd("off",cmd)) itt=0;
    else goto err;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&id) == NULL ) goto err;
    if (pfi==NULL) {
      snprintf(pout,1255,"SET MISSWARN error:  no open files\n");
      gaprnt (0,pout);
      return(1);
    }
    else {
      for (i1=0; i1<id-1; i1++) {
	pfi = pfi->pforw;
	if (pfi==NULL) {
	  snprintf(pout,1255,"SET MISSWARN error:  file %i not open\n",id);
	  gaprnt (0,pout);
	  return(1);
	}
      }
      pfi->errflg = itt;
    }
  }
  else if (cmpwrd("undef",cmd)) {
    kwrd = 117;
    if ((cmd = nxtwrd (cmd)) == NULL) {
      gaprnt (2,"Warning: SET UNDEF argument is missing; output undef value is unchanged.\n");
    }
    else if (cmpwrd("file",cmd)) {    
      if ((cmd = nxtwrd (cmd)) == NULL) {
	gaprnt (2,"Warning: SET UNDEF FILE argument is missing; output undef value is unchanged.\n");
      } 
      else if (intprs(cmd,&id) == NULL ) {
	gaprnt (2,"Warning: SET UNDEF FILE argument is invalid; output undef value is unchanged.\n");
      }
      else {
	pfi = pcm->pfi1;
	if (pfi!=NULL) {
	  for (i1=0; i1<id-1; i1++) {
	    pfi = pfi->pforw;
	    if (pfi==NULL) {
	      break;
	    }
	  }
	}
	if (pfi==NULL) {
	  snprintf(pout,1255,"Warning: SET UNDEF FILE -- file %d not open, output undef value is unchanged.\n",id);
	  gaprnt (2,pout);
	} else {
	  pcm->undef = pfi->undef;
	  snprintf(pout,1255,"Output undef value copied from file %d : %s \n",id,pfi->dnam);
	  gaprnt (2,pout);
	}
      }
    }
    else if (cmpwrd("dfile",cmd)) {    
      id = pcm->dfnum;
      pfi = pcm->pfi1;
      for (i1=0; i1<id-1; i1++) {
	pfi = pfi->pforw;
	if (pfi==NULL) {
	  break;
	}
      }
      if (pfi==NULL) {
	snprintf(pout,1255,"Warning: SET UNDEF DFILE -- default file %i not open, output undef value is unchanged\n",id);
	gaprnt (2,pout);
      } else {
	pcm->undef = pfi->undef;
	snprintf(pout,1255,"Output undef value copied from default file %d : %s \n",id,pfi->dnam);
	gaprnt (2,pout);
      }
    }
    else {
      if (getdbl(cmd,&v1) == NULL) {
	gaprnt(2,"Warning: SET UNDEF argument is invalid, output undef value is unchanged\n");
      }
      else {
	pcm->undef = v1;
      }
    }
    snprintf(pout,1255,"Output undef value is set to %12f \n",pcm->undef);
    gaprnt (2,pout);
  }
  else if (cmpwrd("dfile",cmd)) {
    kwrd = 14;
    if (pcm->pfid==NULL) goto errf;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&id) == NULL ) goto err;
    pfi = pcm->pfi1;
    for (i1=0; i1<id-1; i1++) {
      pfi = pfi->pforw;
      if (pfi==NULL) {
        snprintf(pout,1255,"SET DFILE error:  file %i not open\n",id);
        gaprnt (0,pout);
        return(1);
      }
    }
    snprintf(pout,1255,"Default file set to: %s \n",pfi->name);
    gaprnt (2,pout);
    pcm->pfid = pfi;
    pcm->dfnum = id;
  }
  else if (cmpwrd("background",cmd)) {
    kwrd = 58;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    snprintf(pout,1255,"background = %i \n",itt);
    gaprnt (2,pout);
    gxdbck(itt);        
  }
  else if (cmpwrd("cthick",cmd)) {
    kwrd = 49;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&(pcm->cthick)) == NULL ) goto err;
    snprintf(pout,1255,"cthick = %i \n",pcm->cthick);
    gaprnt (2,pout);
  }
  else if (cmpwrd("cstyle",cmd)) {
    kwrd = 9;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&(pcm->cstyle)) == NULL ) goto err;
    if(pcm->cstyle==0) {
      snprintf(pout,1255,"WARNING: cstyle=0; no lines will be plotted; try using 1 instead\n");
    } else {
      snprintf(pout,1255,"cstyle = %i \n",pcm->cstyle);
    }
    gaprnt (2,pout);
  }
  else if (cmpwrd("digsiz",cmd) || cmpwrd("digsize",cmd)) {
    kwrd = 24;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->digsiz)) == NULL ) goto err;
    snprintf(pout,1255,"digsiz = %g \n",pcm->digsiz);
    gaprnt (2,pout);
  }
  else if (cmpwrd("dignum",cmd)) {
    kwrd = 23;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&itt) == NULL ) goto err;
    if (itt<0 || itt>8) {
      gaprnt (0,"Invalid dignum value:  must be 0 to 8\n");
    } else {
      pcm->dignum = itt;
      snprintf(pout,1255,"dignum = %i \n",pcm->dignum);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd("axlim",cmd)||cmpwrd("vrange",cmd)) {
    kwrd = 15;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("auto",cmd)) pcm->aflag = 0;
    else {
      if (getdbl(cmd,&(pcm->rmin)) == NULL ) goto err;
      pcm->aflag = 0;
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      if (getdbl(cmd,&(pcm->rmax)) == NULL ) goto err;
      pcm->aflag = -1;
      snprintf(pout,1255, "1-D axis limits set: %g to %g \n", pcm->rmin, pcm->rmax);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd("vrange2",cmd)) {
    kwrd = 67;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->rmin2)) == NULL ) goto err;
    pcm->aflag2 = 0;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (getdbl(cmd,&(pcm->rmax2)) == NULL ) goto err;
    pcm->aflag2 = -1;
    snprintf(pout,1255, "Scatter Y axis limits set: %g to %g \n", pcm->rmin2, pcm->rmax2);
    gaprnt (2,pout);
  }
  else if (cmpwrd("strmden",cmd) || cmpwrd("strmopts",cmd)) {
    kwrd = 124;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,&(pcm->strmden)) == NULL ) goto err;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      if (getdbl(cmd,&tt) == NULL ) goto err;
      if (tt>0.001) pcm->strmarrd = tt;
      if ((cmd = nxtwrd (cmd)) != NULL) {
        if (getdbl(cmd,&tt) == NULL ) goto err;
        if (tt>=0.0) pcm->strmarrsz = tt;
        if ((cmd = nxtwrd (cmd)) != NULL) {
          if (intprs(cmd,&i) == NULL ) goto err;
          if (i>0 && i<3) pcm->strmarrt = i;
        }
      }
    }
    snprintf (pout, 255, "Streamline options set to: Density %i Spacing %g Size %g Type %i\n",
        pcm->strmden, pcm->strmarrd, pcm->strmarrsz, pcm->strmarrt) ;
    gaprnt (2,pout);
  }
  else if (cmpwrd("ccolor",cmd)) {
    kwrd = 10;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("rainbow",cmd)) {
      pcm->ccolor = -1;
      gaprnt (2,"ccolor = rainbow \n");
    } else if (cmpwrd("revrain",cmd)) {
      pcm->ccolor = -2;
      gaprnt (2,"ccolor = reverse rainbow \n");
    } else {
      if (intprs(cmd,&(pcm->ccolor)) == NULL ) goto err;
      snprintf(pout,1255,"ccolor = %i \n",pcm->ccolor);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd("stid",cmd)) {
    kwrd = 30;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->stidflg = 1;
    else if (cmpwrd("off",cmd)) pcm->stidflg = 0;
    else goto err;
  }
  else if (cmpwrd("csmooth",cmd)) {
    kwrd = 16;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->csmth = 1;
    else if (cmpwrd("off",cmd)) pcm->csmth = 0;
    else if (cmpwrd("linear",cmd)) pcm->csmth = 2;
    else goto err;
  }
  else if (cmpwrd("cterp",cmd)) {
    kwrd = 34;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->cterp = 1;
    else if (cmpwrd("off",cmd)) pcm->cterp = 0;
    else goto err;
  }
  else if (cmpwrd("loopdim",cmd)) {
    kwrd = 11;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("x",cmd)) pcm->loopdim=0;
    else if (cmpwrd("y",cmd)) pcm->loopdim=1;
    else if (cmpwrd("z",cmd)) pcm->loopdim=2;
    else if (cmpwrd("t",cmd)) pcm->loopdim=3;
    else if (cmpwrd("e",cmd)) pcm->loopdim=4;
    else goto err;
  }
  else if (cmpwrd("grads",cmd)) {
    kwrd = 31;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) {
      pcm->grdsflg = 1;
    }
    else if (cmpwrd("off",cmd)) {
      pcm->grdsflg = 0;
      pcm->timelabflg = 0;
    }
    else goto err;
  }
  else if (cmpwrd("timelab",cmd)) {
    kwrd = 100;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd)) pcm->timelabflg = 1;
    else if (cmpwrd("off",cmd)) pcm->timelabflg = 0;
    else goto err;
  }
  else if (cmpwrd("stnprint",cmd)) {
    kwrd = 102;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd) ) pcm->stnprintflg = 1;
    else if (cmpwrd("off",cmd) ) pcm->stnprintflg = 0;
    else goto err;
  }
  else if (cmpwrd("warn",cmd)) {
    kwrd = 101;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd) ) mfcmn.warnflg = 1;
    else if (cmpwrd("off",cmd) ) mfcmn.warnflg = 0;
    else goto err;
  }
  else if (cmpwrd("looping",cmd)) {
    kwrd = 12;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd) ) {
      pcm->loopflg = 1;
      gaprnt (2,"Looping is on \n");
    } else if (cmpwrd("off",cmd) ) {
      pcm->loopflg = 0;
      gaprnt (2,"Looping is off \n");
    } else goto err;
  }
  else if (cmpwrd("ens",cmd)) {
    kwrd=112;
    if (pcm->pfid==NULL) goto errf;
    pfi = pcm->pfid;
    /* get the first ensemble name */
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    num = 1;
    getwrd(ename1,cmd,16);
    /* get the second ensemble name */
    if ((cmd = nxtwrd (cmd)) != NULL) {
      num = 2;
      getwrd(ename2,cmd,16);
      if (strcmp(ename1,ename2) == 0) num=1;
    }
    ens = pfi->ens1;
    i=0;
    enum1=enum2=-1;
    while (i<pfi->dnum[4]) {
      if (strcmp(ename1,ens->name) == 0) {
	enum1=i;
      }
      if (num>1) {
	if (strcmp(ename2,ens->name) == 0) enum2=i;
      }
      i++; ens++;
    }
    if (enum1<0) goto err;
    if ((num>1) && (enum2<0)) goto err; 
    if (num==1) enum2=enum1;
    pcm->vdim[4] = num-1;
    pcm->dmin[4] = enum1+1;  
    pcm->dmax[4] = enum2+1;
    snprintf(pout,1255,"E set to %g %g \n", pcm->dmin[4], pcm->dmax[4]);
    gaprnt (2,pout);    
  }
  else if (cmpwrd("e",cmd)) {
    kwrd = 111;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (pcm->pfid==NULL) goto errf;
    pfi = pcm->pfid;
    num = 1;
    if (cmpwrd("last",cmd)) {
      v1 = pfi->dnum[4];
    } else {
      if (getdbl(cmd,&v1) == NULL) goto err;
    }
    v2 = v1;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      num = 2;
      if (cmpwrd("last",cmd)) {
        v2 = pfi->dnum[4];
      } else {
        if (getdbl(cmd,&v2) == NULL) goto err;
      }
      if (v1==v2) num = 1;
    } 
    pcm->vdim[4] = num-1;
    pcm->dmin[4] = v1;  
    pcm->dmax[4] = v2;
    snprintf(pout,1255,"E set to %g %g \n", pcm->dmin[4], pcm->dmax[4]);
    gaprnt (2,pout);
  }
  else if (cmpwrd("x",cmd) || cmpwrd("y",cmd) ||
           cmpwrd("z",cmd) || cmpwrd("t",cmd) ) {
    if (*cmd=='x') kwrd=0;
    if (*cmd=='y') kwrd=1;
    if (*cmd=='z') kwrd=2;
    if (*cmd=='t') kwrd=3;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (pcm->pfid==NULL) goto errf;
    pfi = pcm->pfid;
    num = 1;
    if (kwrd==3 && cmpwrd("last",cmd)) {
      v1 = pfi->dnum[3];
    } else {
      if (getdbl(cmd,&v1) == NULL) goto err;
    }
    v2 = v1;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      num = 2;
      if (kwrd==3 && cmpwrd("last",cmd)) {
        v2 = pfi->dnum[3];
      } else {
        if (getdbl(cmd,&v2) == NULL) goto err;
      }
      if (dequal(v1,v2,1.0e-8)==0) num=1;
    }
    pcm->vdim[kwrd] = num-1;

    /* Try to save grid dims for write flag -ex */

    if (pfi->type==1 && num==2) {
      if (kwrd==0) {
        pcm->xexflg = 1;
        pcm->x1ex = (gaint)(v1+0.001);
        pcm->x2ex = (gaint)(v2+0.001);
      }
      if (kwrd==1) {
        pcm->yexflg = 1;
        pcm->y1ex = (gaint)(v1+0.001);
        pcm->y2ex = (gaint)(v2+0.001);
      }
    }

    if (pfi->type==1 && num==1) {
      v1 = floor(v1+0.5);
      pcm->vdim[kwrd] = 0;
    }
    if (kwrd==3) {
      vals = pfi->grvals[3];
      gr2t (vals,v1,&(pcm->tmin));
      if (num==1) pcm->tmax = pcm->tmin;
      else gr2t(vals,v2,&(pcm->tmax));
      gaprnt (2,"Time values set: ");
      snprintf(pout,1255,"%i:%i:%i:%i ",pcm->tmin.yr,pcm->tmin.mo,
        pcm->tmin.dy,pcm->tmin.hr);
      gaprnt (2,pout);
      snprintf(pout,1255,"%i:%i:%i:%i \n",pcm->tmax.yr,pcm->tmax.mo,
        pcm->tmax.dy,pcm->tmax.hr);
      gaprnt (2,pout);
    } else {
      if (pfi->type==1) {
        conv = pfi->gr2ab[kwrd];
        vals = pfi->grvals[kwrd];
        pcm->dmin[kwrd] = conv(vals,v1);
        if (num==1) 
	  pcm->dmax[kwrd] = pcm->dmin[kwrd];
        else 
	  pcm->dmax[kwrd] = conv(vals,v2);
      } else {
        pcm->dmin[kwrd] = v1;
        if (num==1) 
	  pcm->dmax[kwrd] = pcm->dmin[kwrd];
        else 
	  pcm->dmax[kwrd] = v2;
      }
      snprintf(pout,1255,"%s set to %g %g \n",kwds[kwrd+4], pcm->dmin[kwrd], pcm->dmax[kwrd]);
      gaprnt (2,pout);
    }
  }
  else if (cmpwrd("lon",cmd) || cmpwrd("lat",cmd) || cmpwrd("lev",cmd)) {
    if (cmpwrd("lon",cmd)) {kwrd=4; id=0;}
    if (cmpwrd("lat",cmd)) {kwrd=5; id=1;}
    if (cmpwrd("lev",cmd)) {kwrd=6; id=2;}
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (pcm->pfid==NULL) goto errf;
    num = 1;
    if (getdbl(cmd,&(pcm->dmin[id])) == NULL) goto err;
    pcm->dmax[id] = pcm->dmin[id];
    if ((cmd = nxtwrd (cmd)) != NULL) {
      num = 2;
      if (getdbl(cmd,&(pcm->dmax[id])) == NULL) goto err;
      if (dequal(pcm->dmin[id],pcm->dmax[id],1.0e-8)==0) num=1;
    }
    pcm->vdim[id] = num-1;
    pfi = pcm->pfid;
    if (pfi->type==1 && num==1) {
      pcm->vdim[id] = 0;
      conv = pfi->ab2gr[id];
      vals = pfi->abvals[id];
      v1 = conv(vals,pcm->dmin[id]);
      v1 = floor(v1+0.5);
      conv = pfi->gr2ab[id];
      vals = pfi->grvals[id];
      pcm->dmin[id] = conv(vals,v1);
      pcm->dmax[id] = pcm->dmin[id];
    }
    snprintf(pout,1255,"%s set to %g %g \n",kwds[id+4],pcm->dmin[id],pcm->dmax[id]);
    gaprnt (2,pout);
  }
  else if (cmpwrd("time",cmd)) {
    kwrd=7; id=3;
    if (pcm->pfid==NULL) goto errf;
    if ((cmd = nxtwrd(cmd)) == NULL) goto err;
    num = 1;
    tdef = pcm->tmin;
    if (adtprs(cmd,&tdef,&(pcm->tmin)) == NULL ) goto err;
    pcm->tmax = pcm->tmin;
    if ((cmd = nxtwrd (cmd)) != NULL) {
      num = 2;
      if (adtprs(cmd,&tdef,&(pcm->tmax)) == NULL ) goto err;
    }
    pcm->vdim[3] = 1;
    if (num==1) {
      pcm->vdim[3] = 0;
      pfi = pcm->pfid;
      vals = pfi->abvals[3];
      v1 = t2gr(vals,&(pcm->tmin));
      v1 = floor(v1+0.5);
      vals = pfi->grvals[3];
      gr2t (vals,v1,&(pcm->tmin));
      pcm->tmax = pcm->tmin;
    }
    gaprnt (2,"Time values set: ");
    snprintf(pout,1255,"%i:%i:%i:%i ",pcm->tmin.yr,pcm->tmin.mo,
      pcm->tmin.dy,pcm->tmin.hr);
    gaprnt (2,pout);
    snprintf(pout,1255,"%i:%i:%i:%i \n",pcm->tmax.yr,pcm->tmax.mo,
      pcm->tmax.dy,pcm->tmax.hr);
    gaprnt (2,pout);
  }
  else if (cmpwrd("datawarn",cmd)) {
    kwrd = 107;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (cmpwrd("on",cmd) ) pcm->dwrnflg = 1;
    else if (cmpwrd("off",cmd) ) pcm->dwrnflg = 0;
    else goto err;
  }
  else if (cmpwrd("tile",cmd)) {
    kwrd = 125;
    for (i=0; i<15; i++) itmp[i] = -999;
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,itmp) == NULL ) goto err;  /* tile number */
    if (*(itmp+0)<0 || *(itmp+0)>=COLORMAX) {
      snprintf (pout,1255,"Tile number must be between 0 and %d\n",COLORMAX-1); 
      gaprnt (0,pout);
      goto err;
    } 
    if ((cmd = nxtwrd (cmd)) == NULL) goto err;
    if (intprs(cmd,itmp+1) == NULL ) goto err;  /* tile type */
    if (*(itmp+1)<0 || *(itmp+1)>8) {
      snprintf (pout,1255,"Tile type must be between 0 and 8\n"); 
      gaprnt (0,pout);
      goto err;
    } 

    /* check for type 0 */
    if (*(itmp+1)==0) {
      if ((cmd = nxtwrd (cmd)) != NULL) {
	/* user has provided a file to define a new tile -- get the length */
	itt2 = 0;
	while (*(cmd+itt2)!='\n'&&*(cmd+itt2)!='\0') itt2++; /* spaces are allowed */
	/* allocate memory for tile filename */
	tileimg = (char *)malloc(itt2+1);
	if (tileimg==NULL) {
	  gaprnt (0,"Memory allocation error for tile image filename\n"); goto err;
	}
	/* copy the user-supplied filename (mixed case) to tileimg */
	i2 = cmd - cmd1;
	for (i1=0; i1<itt2; i1++) *(tileimg+i1) = *(com+i1+i2);
	/* Make sure filename ends in ".png"  */
	i = itt2-4;
	if (i>0) {
	  if (*(tileimg+i+1)!='p' || *(tileimg+i+2)!='n' || *(tileimg+i+3)!='g' ) {
	    if (*(tileimg+i+1)!='P' || *(tileimg+i+2)!='N' || *(tileimg+i+3)!='G' ) {
	      gaprnt(0,"Custom tile image file must be PNG format\n");
	      free (tileimg);
	      goto err;
	    }
	  }
	}
	*(tileimg+itt2)='\0';      /* ensure tile image filename is null-terminated */
	/* check if file exists*/
	FILE *tileimgfile=NULL;
	tileimgfile=fopen(tileimg,"rb");
	if (tileimgfile==NULL) {
	  gaprnt(0,"Unable to open tile image file\n");
	  free (tileimg);
	  goto err;
	}
	else fclose(tileimgfile);
      }
    }
    else {
      /* get rest of args for on-the-fly tile generation */
      for (i=2; i<7; i++) {  
	if ((cmd = nxtwrd (cmd)) == NULL) break;
	if (intprs(cmd,itmp+i) == NULL ) goto err;
      }
    }
    gxdbsetpatt(itmp,tileimg);
    if (tileimg) free (tileimg);
  }
  else if (cmpwrd("fill",cmd)) {
    kwrd = 96;
    pat = (char *)galloc(7,"fillpat");
    if ((cmd = nxtwrd (cmd)) == NULL) goto pterr;
    if (cmpwrd("on",cmd) ) {
      snprintf(pat,6,"%s","on");
      pcm->ptflg = 1;
    }
    else if (cmpwrd("off",cmd) ) {
      snprintf(pat,6,"%s","off");
      pcm->ptflg = 0;
    }
    else if (cmpwrd("open",cmd)) {
      pcm->ptflg = 1;
      pcm->ptopt = 0;
      snprintf(pat,6,"%s","open");
    }
    else if (cmpwrd("solid",cmd)) {
      snprintf(pat,6,"%s","solid");
      pcm->ptflg = 1;
      pcm->ptopt = 1;
    }
    else if (cmpwrd("dot",cmd)) {
      snprintf(pat,6,"%s","dot");
      pcm->ptflg = 1;
      pcm->ptopt = 2;
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      if (intprs(cmd,&num) == NULL ) goto err;
      if (num<1 || num>6)
        gaprnt (0,"Invalid ptrnden value:  must be integer 1 to 6\n");
      else
        pcm->ptden=num;
    }
    else if (cmpwrd("line",cmd)) {
      snprintf(pat,6,"%s","line");
      pcm->ptflg = 1;
      pcm->ptopt = 3;
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      if (intprs(cmd,&num) == NULL ) goto err;
      if (num<1 || num>5)
        gaprnt (0,"Invalid ptrnden value:  must be integer 1 to 5\n");
      else
        pcm->ptden=num;
      if ((cmd = nxtwrd (cmd)) == NULL) goto err;
      if (intprs(cmd,&num) == NULL ) goto err;
      if (num!=-90&&num!=90&&num!=-60&&num!=60&&num!=-30&&num!=30
	  &&num!=-45&&num!=45&&num!=0) {
        gaprnt (0,"Invalid ptrnang value:  must be -90, -60, -45 -30\n");
        gaprnt (0," 	                        0, 30. 45, 60, or 90\n");
      }
      else
        pcm->ptang=num;
    }
    else goto err;

    if (cmpwrd("line",pat) )
      snprintf(pout,1255,"SET FILL values: %s %d %d\n",pat,pcm->ptden,pcm->ptang);
    else if (cmpwrd("dot",pat) )
      snprintf(pout,1255,"SET FILL values: %s %d\n",pat,pcm->ptden);
    else
      snprintf(pout,1255,"SET FILL values: %s\n",pat);
    gaprnt (2,pout);
    gree(pat,"f234");
  }

  else {
    gaprnt (0,"SET error: Invalid operand\n");
    gaprnt (0,"  Operand = ");
    gaprnt (0,cmd);
    gaprnt (0,"\n");
    return (1);
  }
  return (0);

err:
  gaprnt (0,"SET error:  Missing or invalid arguments ");
  snprintf(pout,1255,"for %s option\n",kwds[kwrd]);
  gaprnt (0,pout);
  return (1);

errf:
  gaprnt (0,"SET Error:  No files open yet\n");
  return (1);

xlerr:
  gaprnt (0,"SET XLOPTS(YLOPTS,CLOPTS) Syntax Error");
  gaprnt (0,"  Syntax is: SET XLOPTS color thickness size");
  return (1);

xlerr2:
  gaprnt (0,"SET XLPOS(YLPOS) Syntax Error");
  gaprnt (0,"  Syntax is: SET XLPOS offset side\n");
  return (1);

rbberr:
  gaprnt (0,"SET RBAND Syntax Error.");
  gaprnt (0,"  Syntax is: SET RBAND num type xlo ylo xhi yhi\n");
  return (1);

pterr:
  gaprnt (0,"SET FILL Syntax Error.");
  gaprnt (0,"  Syntax is: SET FILL type [density] [angle]\n");
  return (1);

drerr:
  gaprnt (0,"SET DROPMENU Syntax Error.");
  gaprnt (0,"  Syntax is: SET DROPMENU fc bc oc1 oc2 tfc tbc toc1 toc2\n");
  gaprnt (0,"      bfc bbc boc1 boc2 soc1 soc2 thick\n");
  return (1);
}

/* Open a data set by reading the descriptor file for that
   data set, and create a gafile structure.  Chain the gafile
   structure on to the list anchored in the gastat.                   */

gaint  gaopen (char *name, struct gacmn *pcm) {
struct gafile *pfi, *pfio;
gaint rc;

  pfi = getpfi();
  if (pfi==NULL) {
    gaprnt (0,"Memory Allocation Error: On File Open\n");
    return (1);
  }

#if USEGADAP
  /* if name starts with http:// then it's a URL */
  if (*name=='h' && *(name+1)=='t' && *(name+2)=='t' &&
      *(name+3)=='p' && *(name+4)==':' && *(name+5)=='/' &&
      *(name+6)=='/' ) {    
    rc = dappfi (name,pfi); 
  } else {
    gaprnt (2,"Scanning description file:  ");
    gaprnt (2,name);
    gaprnt (2,"\n");
    rc = gaddes(name, pfi, 1);
  }       
#else 
  gaprnt (2,"Scanning description file:  ");
  gaprnt (2,name);
  gaprnt (2,"\n");
  rc = gaddes(name, pfi, 1);
#endif

  if (rc) {
    frepfi (pfi,0);
    return (rc);
  }
  pcm->fseq = pcm->fseq + 1;
  pfi->fseq = pcm->fseq;       /* opened files get unique sequence number */

  if (pfi->tmplat==0 && pfi->dhandle == -999) {
    if (pfi->ncflg) {
      if (pfi->ncflg==1) {    /* dtype netcdf */
#if USENETCDF == 1
	rc = gaopnc(pfi,0,1);
	if (rc) {
	  frepfi (pfi,0);
	  return (1);
	}
#endif
      }  
      else if (pfi->ncflg==2) {    /* dtype hdfsds */
#if USEHDF == 1
	rc = gaophdf(pfi,0,1);
	if (rc) {
	  frepfi (pfi,0);
	  return (1);
	}
#endif
      }
      else if (pfi->ncflg==3) {    /* dtype hdf5 */
#if USEHDF5 == 1
	rc = gaoph5(pfi,0,1);
	if (rc) {
	  frepfi (pfi,0);
	  return (1);
	}
#endif
      }

    } else if (!pfi->bufrflg) {   
      pfi->infile = fopen (pfi->name, "rb");
      if (pfi->infile==NULL) {
	gaprnt (0,"Open Error:  Can't open binary data file\n");
	gaprnt (0,"  File name = ");
	gaprnt (0,pfi->name);
	gaprnt (0,"\n");
	frepfi (pfi,0);
	return (1);
      }
    }
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

  if (pcm->fnum==1) {pcm->pfid = pcm->pfi1; pcm->dfnum = 1;}
  snprintf(pout,1255,"Data file %s is open as file %i\n",pfi->name,pcm->fnum);
  gaprnt (2,pout);

  /* If first file open, set up some default dimension ranges for the user */
  if (pcm->fnum==1) {
    if (pfi->type==2 || pfi->wrap ) gacmd ("set lon 0 360",pcm,0);
    else {
      snprintf(pout,1255,"set x 1 %i",pfi->dnum[0]);
      gacmd (pout,pcm,0);
    }
    if (pfi->type==2) {
      gacmd ("set lat -90 90",pcm,0);
      gacmd ("set lev 500",pcm,0);
    } else {
      snprintf(pout,1255,"set y 1 %i",pfi->dnum[1]);
      gacmd (pout,pcm,0);
      
      /* set z to max if x or y = 1 */
      if ((pfi->type==1 && pfi->dnum[2] >= 1)
	  && ((pfi->dnum[0] == 1) || (pfi->dnum[1] == 1))) {
	snprintf(pout,1255,"set z 1 %i",pfi->dnum[2]);
	gacmd (pout,pcm,0);
      } else {
	gacmd ("set z 1",pcm,0);
      }
    }
    gacmd ("set t 1",pcm,0);
    gacmd ("set e 1",pcm,0);
  }

  if (pfi->ppflag) {
    snprintf(pout,1255,"Notice: Implied interpolation for file %s\n",name);
    gaprnt (1,pout);
    gaprnt (1," Interpolation will be performed on any data ");
    gaprnt (1,"displayed from this file\n");

    if (pfi->ppflag==8 && pfi->pdefgnrl==0) {
      snprintf(pout,1255,"WARNING: The use of PDEF FILE in %s \n",pfi->dnam);
      gaprnt (1,pout);
      gaprnt (1," may be incorrect. Please make sure you are using it properly.\n");
      gaprnt (1," Updated documentation is at http://cola.gmu.edu/grads/gadoc/pdef.html\n");
    }
  }
  
  return (0);
}

struct gastat *getpst (struct gacmn *pcm) {
struct gastat *pst;
gaint i,vcnt,lflg,ll;
size_t sz;

  sz = sizeof(struct gastat);
  if ((pst = (struct gastat *)galloc(sz,"pst")) == NULL) {
    gaprnt(0,"Memory allocation error in getpst\n");
    return(NULL);
  }
  pst->pfi1 = pcm->pfi1;
  pst->pfid = pcm->pfid;
  pst->fnum = pcm->fnum;
  pst->pclct = (struct gaclct **)&(pcm->clct);
  for (i=0;i<5;i++) {
    if (i==3) {
      pst->tmin = pcm->tmin;
      pst->tmax = pcm->tmax;
    } 
    else {
      pst->dmin[i] = pcm->dmin[i];
      pst->dmax[i] = pcm->dmax[i];
    }
  }
  pst->type = 1;
  pst->result.pgr = NULL;
  pst->pdf1 = pcm->pdf1;

  vcnt = 0;
  for (i=0; i<5; i++) if (pcm->vdim[i]) vcnt++;
  lflg = pcm->loopflg;
  if (vcnt>2) lflg = 1;

  ll=0;
  pst->idim = -1;
  pst->jdim = -1;
  if (pcm->vdim[0]) {                    /* X is varying */
    if (pcm->dmin[0]>pcm->dmax[0]) {
      gaprnt (0,"Operation error:  Invalid dimension environment\n");
      snprintf(pout,1255,"  Min longitude > max longitude: %g %g \n",
	       pcm->dmin[0],pcm->dmax[0]);
      gaprnt (0,pout);
      goto err;
    }
    if (pcm->loopdim!=0 || !lflg) {
      pst->idim=0;
      ll++;
    }
  }
  if (pcm->vdim[1]) {                    /* Y is varying */
    if (pcm->dmin[1]>pcm->dmax[1]) {
      gaprnt (0,"Operation error:  Invalid dimension environment\n");
      snprintf(pout,1255,"  Min latitude > max latitude: %g %g \n",
	       pcm->dmin[1],pcm->dmax[1]);
      gaprnt (0, pout);
      goto err;
    }
    if (pcm->loopdim!=1 || !lflg) {
      if (ll>0) 
	pst->jdim = 1;
      else 
	pst->idim = 1;
      ll++;
    }
  }
  if (pcm->vdim[2]) {                    /* Z is varying */
    if (pcm->loopdim!=2 || !lflg) {
      if (ll>0) 
	pst->jdim = 2;
      else 
	pst->idim = 2;
      ll++;
    }
  }
  if (pcm->vdim[3]) {                    /* T is varying */
    if (pcm->loopdim!=3 || !lflg) {
      if (ll>0) 
	pst->jdim = 3;
      else 
	pst->idim = 3;
      ll++;
    }
  }
  if (pcm->vdim[4]) {                    /* E is varying */
    if (pcm->loopdim!=4||!lflg) {
      if (ll>0) 
	pst->jdim = 4;
      else 
	pst->idim = 4;
      ll++;
    }
  }
  if (lflg && (vcnt==ll) ) {
    gaprnt (0,"Operation error:  Invalid dimension environment\n");
    gaprnt (0,"  Looping dimension does not vary\n");
    goto err;
  }
  if (ll>2) {
    gaprnt (0,"Operation error:  Invalid dimension environment\n");
    gaprnt (0,"  Too many varying dimensions \n");
    goto err;
  }

  return (pst);

err:
  gree(pst,"f101");
  return(NULL);

}


void gaprnt (gaint level, char *msg) {
gaint len;

  if (msgflg) {
    len = 0;
    while (*(msg+len)) len++;
    len++;
    msgnew = (struct msgbuf *)malloc(sizeof(struct msgbuf));
    if (msgnew==NULL) {
      msgflg = 0;
      printf ("Memory allocation error:  msg buffers\n");
      return;
    }
    msgnew->msg = (char *)malloc(len);
    if (msgnew->msg==NULL) {
      msgflg = 0;
      printf ("Memory allocation error:  msg buffers\n");
      gree(msgnew,"f102");
      return;
    }
    len = 0;
    while (*(msg+len)) {
      *(msgnew->msg+len) = *(msg+len);
      len++;
    }
    *(msgnew->msg+len) = '\0';
    msgnew->len = len;
    msgnew->forw = NULL;
    if (msgstk==NULL) msgstk = msgnew;
    else msgcurr->forw = msgnew;
    msgcurr = msgnew;
  }
  if (!msgflg || level<2) {
    printf ("%s",gatxtl(msg,level));
  }
}

gaint prntgaattr (struct gafile *pfi, char *name, gaint hdrflg, gaint fnum) {
  struct gaattr *attr;
  short *sptr=NULL;
  long *lptr=NULL;
  gafloat *fptr=NULL;
  gadouble *dptr=NULL;
  gaint i;

  if (pfi->attr) {
    attr=pfi->attr;
    while (attr) {
      if (strcmp(attr->varname,name)==0) {
	if (attr->fromddf == 1) {   /* only print those that were in a descriptor file */
	  if (hdrflg) {
	    snprintf(pout,1255,"Descriptor Attributes for File %i : %s \n",fnum,pfi->title);
	    gaprnt(2,pout);
	    hdrflg=0;
	  }
	  /* print strings */
	  if (attr->nctype <= 2) {
	    snprintf(pout,1255,"%s %s %s %s\n",
		    attr->varname,attr->type,attr->name,(char*)attr->value);
	    gaprnt(2,pout);
	  } 
	  else {
	    snprintf(pout,1255,"%s %s %s ",attr->varname,attr->type,attr->name);
	    gaprnt(2,pout);
	    if      (attr->nctype == 3) sptr = (short*)attr->value;
	    else if (attr->nctype == 4) lptr = (long*)attr->value;
	    else if (attr->nctype == 5) fptr = (gafloat*)attr->value;
	    else if (attr->nctype == 6) dptr = (gadouble*)attr->value;
	    for (i=0; i<attr->len; i++) {
	      /* print numbers */
	      if (attr->nctype == 3) {
		snprintf(pout,1255,"%i",(gaint)*(sptr));
		gaprnt(2,pout);
		sptr++;
	      } else if (attr->nctype == 4) {
		snprintf(pout,1255,"%li",*(lptr));
		gaprnt(2,pout);
		lptr++;
	      } else if (attr->nctype == 5) {
		snprintf(pout,1255,"%f",*(fptr));
		gaprnt(2,pout);
		fptr++;
	      } else { 
		snprintf(pout,1255,"%g",*(dptr));
		gaprnt(2,pout);
		dptr++;
	      }
	      if (i != attr->len-1) {
		snprintf(pout,1255,",");
		gaprnt(2,pout);
	      }
	    }
	    snprintf(pout,1255,"\n");
	    gaprnt(2,pout);
	  }
	}
      }
      attr=attr->next;
    }
  }
  return (hdrflg);
}


#if READLINE==1

/* print history or repeat commands from history  */
gaint gahistory(char*cmd, char *com, struct gacmn *pcm) {

#include <readline/history.h>

  extern gaint history_length; 
  HIST_ENTRY **his_cmd;
  FILE *logid;        /* history file id */
  char *logfile;      /* history filename */
  gaint pformat=0;    /* format for printing */
  gaint i,is,ie,in=0; /* counter, history boundaries, # boundary argument */
  gaint nerror=0;     /* count error in repeated commands */
  gaint fileout=0;    /* flag: output to a file? */
  gaint retcod=0;     /* return code */
  gaint verbose=1;    /* Give feetback */
  gaint readhis=0;    /* Read history */

  is=1;
  ie=history_length;
  com=nxtwrd(com);
  if (com) {
    if (cmpwrd("-s",com) ) { /* script style: quote commands */
      pformat=1;
      com=nxtwrd(com);
    } else if (cmpwrd("-x",com) ) { /* exec style: no command number */
      pformat=2;
      com=nxtwrd(com);
    } else if (cmpwrd("-q",com) ) {
      verbose=0;
      com=nxtwrd(com);
    } else if (cmpwrd("-r",com) ) {
      readhis=1;
      com=nxtwrd(com);
    }
  }

  if(readhis) { /* read history from file */
    if(com)
      if(verbose)
	printf("Adding content of file '%s' to history.\n",com);
      if(read_history(com)) {
	printf("Error reading history from file '%s'.\n",com);
	return(1);
      }
    return(0);
  }
  if (com) {
    /* try to read requested history boundaries */
    in=sscanf(com,"%d%d",&is,&ie);
    if (in==1) { 
      if(is<0) {
	is=history_length+1+is;
	ie=is;
      } else if (is==0) {
	is=history_search_pos("quit # (End of session: ", -1,history_length)+2;
	ie=history_length;
      } else {
	ie=is;
      }
    }
    if(in==2) {
      /* look backward if is, ie < 0 */
      if(is<0) is=history_length+1+is;
      if(ie<0) ie=history_length+1+ie;
      else if(ie==0) ie=history_length;
    }
  }
  if(is>ie) {i=is;is=ie;ie=i;}
  if (ie>history_length) ie=history_length;
  for (i=1;i<=in;i++) com=nxtwrd(com);
  if(com){
   if(verbose) printf("Writing command history to file '%s'.\n",com);
    fileout=1;
    logfile=strtok(com," ");
    logid=fopen(logfile,"a");
    /* write only commands from current session to file */
    if(in==0) is=history_search_pos("quit # (End of session: ", -1,history_length)+2;
    if(in<2) ie=history_length-1;
    if (strcmp(".gs",com+strlen(com)-3)==0) {
      pformat=1;
    } else if (strcmp(".eps",com+strlen(com)-4)==0 ||
              strcmp(".EPS",com+strlen(com)-4)==0    ) {
      pformat=3;
   
    } else {
      pformat=2;
    }
  } else
    logid=stdout;
  his_cmd=history_list();
  if (cmpwrd("history",cmd) || cmpwrd("his",cmd) ) {
    /* print history */
    if (pformat==1) 
      for(i=is;i<=ie;i++) 
	fprintf(logid,"'%s'\n",his_cmd[i-1]->line);
    else if(pformat==2)
      for(i=is;i<=ie;i++) 
	fprintf(logid,"%s\n",his_cmd[i-1]->line);
    else if(pformat==3) 
      for(i=is;i<=ie;i++) 
        fprintf(logid,"%%ga> %s\n",his_cmd[i-1]->line);
    else 
      for(i=is;i<=ie;i++) 
	fprintf(logid,"  [%d] %s\n",i,his_cmd[i-1]->line);
    if (fileout) fclose(logid);
  } else {
    /* repeat history commands */
    if(ie==history_length) ie--;
    if(is==ie) {
      printf("  %s\n",his_cmd[is-1]->line);
    }
    else {
      printf("  Repeating commands\n    [%d] \"%s\" ...\n    [%d] \"%s\"\n",
	   is, his_cmd[is-1]->line, ie,his_cmd[ie-1]->line);
    }
    for(i=is;i<=ie;i++) {
      if (cmpwrdl(his_cmd[i-1]->line, "his") || 
           cmpwrdl(his_cmd[i-1]->line, "history") || 
           cmpwrdl(his_cmd[i-1]->line, "r") || 
           cmpwrdl(his_cmd[i-1]->line,"repeat" ) ) {
	printf(" [%d] \"%s\" not repeated.\n",i,his_cmd[is-1]->line);
	continue; /* skip repeat commands */
      }
      retcod = gacmd(his_cmd[i-1]->line,pcm,1);
      if (retcod) {
	++nerror;
	snprintf(pout,1255,"  Repeat:  Error in command [%d] \"%s\".\n",i,his_cmd[i-1]->line);
	gaprnt (0,pout);
	if (nerror>50) {
	  snprintf(pout,1255,"  Repeat:  Too many errors (>50 ). Repeat stopped.\n");
	  gaprnt (0,pout);
	  retcod = 1;
	  return (retcod);
	}
      }
    }
  }
  return (retcod);
}
#endif /* READLINE==1 */

/* Routine to write out attribute metadata for self-describing output file */
gaint sdfwatt (struct gacmn *pcm, gaint varid, char *varname, char *attname, char *attval) {
#if USENETCDF == 1
  gaint rc;
  struct gaattr *attr=NULL;

   /* Default attributes have name and value provided in the function call */ 
  if (attname != NULL) {
    rc = nc_put_att_text(pcm->ncwid, varid, attname, strlen(attval), attval);
    if (rc) {
      snprintf(pout,1255,"sdfwatt error from nc_put_att_text (%s %s): \n ",
	       varname, attname); 
      gaprnt (0,pout);
      handle_error(rc);
      return(1);
    }
  }
  else {
    /* Non-default user-defined attributes are chained off the gacmn structure */
    attr = pcm->attr;
    while (attr) {
      if (!strcmp(attr->varname, varname)) {       /* varname must match */
	/* NetCDF library calls depend on attribute type */
	if (attr->nctype <= 2) { 
	  rc = nc_put_att_text (pcm->ncwid, varid, attr->name, 
				attr->len, (char*)attr->value);
	  if (rc) {
	    snprintf(pout,1255,"sdfwatt error from nc_put_att_text (%s %s): \n ",
		     attr->varname, attr->name); 
	    gaprnt(0,pout);
	    handle_error(rc);
	    return(1);
	  }
	} else if (attr->nctype == 3) {
	  rc = nc_put_att_short (pcm->ncwid, varid, attr->name, 
				 attr->nctype, attr->len, (short*)attr->value);
	  if (rc) {
	    snprintf(pout,1255,"sdfwatt error from nc_put_att_short (%s %s): \n ",
		     attr->varname, attr->name); 
	    gaprnt(0,pout);
	    handle_error(rc);
	    return(1);
	  }
	} else if (attr->nctype == 4) {
	  rc = nc_put_att_long (pcm->ncwid, varid, attr->name, 
				attr->nctype, attr->len, (long*)attr->value);
	  if (rc) {
	    snprintf(pout,1255,"sdfwatt error from nc_put_att_long (%s %s): \n ",
		     attr->varname, attr->name); 
	    gaprnt(0,pout);
	    handle_error(rc);
	    return(1);
	  }
	} else if (attr->nctype == 5) {
	  rc = nc_put_att_float (pcm->ncwid, varid, attr->name, 
				 attr->nctype, attr->len, (gafloat*)attr->value);
	  if (rc) {
	    snprintf(pout,1255,"sdfwatt error from nc_put_att_float (%s %s): \n ",
		     attr->varname, attr->name); 
	    gaprnt(0,pout);
	    handle_error(rc);
	    return(1);
	  }
	} else if (attr->nctype == 6) {
	  rc = nc_put_att_double (pcm->ncwid, varid, attr->name, 
				  attr->nctype, attr->len, (gadouble*)attr->value);
	  if (rc) {
	    snprintf(pout,1255,"sdfwatt error from nc_put_att_double (%s %s): \n ",
		     attr->varname, attr->name); 
	    gaprnt(0,pout);
	    handle_error(rc);
	    return(1);
	  }
	}
      }
      attr = attr->next;
    }
  }
  return (0);
#endif
 return (0);
}

/* Routine to write out coordinate values for self-describing output file */
gaint sdfwdim (struct gafile *pfi, struct gacmn *pcm, gaint dim, gaint varid) {
#if USENETCDF == 1
struct dt tinit,time;
gadouble *axis, *axis0;
gadouble (*conv) (gadouble *, gadouble);
gaint i, rc, flag;
size_t sz;
size_t start, count;

 /* populate a local array with the coordinate values */
 axis = NULL;
 sz = sizeof(gadouble)*pfi->dnum[dim];
 axis = (gadouble *)galloc(sz,"sdfaxis");
 if (axis==NULL) { 
   gaprnt(0,"sdfwdim error: unable to allocate memory for coordinate values\n"); 
   return (1);
 }
 axis0 = axis; 
 if (dim == 3) {
   /* For T coordinate */
   flag=0;
   if (*(pfi->grvals[3]+5)>0) flag=1;   /* change time unit to months instead of minutes */
   gr2t (pfi->grvals[dim],1.0+pfi->dimoff[dim],&tinit);
   for (i=1; i<=pfi->dnum[dim]; i++) {
     gr2t (pfi->grvals[dim],(gadouble)(i+pfi->dimoff[dim]),&time);
     *(axis+i-1) = timdif(&tinit,&time,flag);   
   }
 }
 else {
   /* For X, Y, Z and E coordinates */
   conv = pfi->gr2ab[dim];
   for (i=1; i<=pfi->dnum[dim]; i++) {
     *axis = conv(pfi->grvals[dim],(gadouble)(i+pfi->dimoff[dim]));
     axis++;
   }
 }
 axis = axis0;

 /* When writing the coordinate values to the file,
    use nc_put_vara_double if it's a record dimension */

 if ((dim==4 && (pcm->sdfrecdim==2 || pcm->sdfrecdim==3)) ||
     (dim==3 && (pcm->sdfrecdim==1 || pcm->sdfrecdim==3))) {
   count = (size_t)pfi->dnum[dim];
   start = 0;
   rc = nc_put_vara_double(pcm->ncwid, varid, &start, &count, axis0);
 }
 else {
   rc = nc_put_var_double(pcm->ncwid, varid, axis0);
 }

 if (rc) {
   snprintf(pout,1255,"sdfwdim error from nc_put_var_double (dim=%d): \n ",dim);
   gaprnt(0,pout);
   handle_error(rc);
   return (1);
 }
 gree(axis,"g0");
 return (0);
#endif
 return (0);
}

/* Routine to set up a coordinate dimension and variable in a self-describing output file */
gaint sdfdefdim (gaint fileid, char *dimname, gaint dimsize, gaint *dimid, gaint *varid, 
		 gaint recflg, gaint sdfwtype) {
#if USENETCDF == 1
gaint rc;
size_t chunksize;
 /* define the dimension */
 if (recflg)
   rc = nc_def_dim(fileid, dimname, NC_UNLIMITED, dimid);
 else
   rc = nc_def_dim(fileid, dimname, dimsize, dimid);
 if (rc) {
   snprintf(pout,1255,"sdfdefdim error from nc_def_dim (%s): \n ",dimname);
   gaprnt (0,pout);
   handle_error(rc); 
   return (1);
 }
 /* define the coordinate variable */
 rc = nc_def_var(fileid, dimname, NC_DOUBLE, 1, dimid, varid);
 if (rc) {
   snprintf(pout,1255,"sdfdefdim error from nc_def_var (%s): \n ",dimname);
   gaprnt (0,pout);
   handle_error(rc); 
   return (1);
 }

#if HAVENETCDF4==1
 /* define record dimensions as chunked for netcdf-4 files */
 if (recflg && sdfwtype==2) {
   chunksize = (size_t)dimsize;
   rc = nc_def_var_chunking (fileid, *dimid, NC_CHUNKED, &chunksize);
   if (rc) {
     snprintf(pout,1255,"sdfdefdim error from nc_def_var_chunking (%s): \n ",dimname);
     gaprnt(0,pout);
     handle_error(rc);
     return (1);
   }
 }
#endif



 return (0);
#endif
 return(0);
}


/* Routine to write out a defined grid in netcdf format */
gaint ncwrite (char *cmd, struct gacmn *pcm) {
#if USENETCDF == 1
struct gadefn *pdf;
struct gafile *pfi;
struct dt tinit;
off_t pos,nelems;
gaint i,rc,nvdims=0,vdims[5],dimids[5];
size_t start[5],count[5],*chunksize=NULL;
gaint xdimid,ydimid,zdimid,tdimid,edimid;
gaint xvarid,yvarid,zvarid,tvarid,evarid,varid;
gaint padX,padY,padZ,padT,padE,recflg;
char name[20],tunit[256];
gafloat flundef,*flbuf=NULL;
char *filename={"grads.sdfwrite.nc"};


  /* Get the name of the defined grid */
  if ((cmd=nxtwrd(cmd)) == NULL) {
    gaprnt (0,"ncwrite error: name of defined grid is missing \n");
    goto err;
  }
  /* copy defined variable name */
  i=0;
  while (*(cmd+i)!=' ' && *(cmd+i)!='\n' && *(cmd+i)!='\0' && i<17) {
    name[i] = *(cmd+i);
    i++;
  }
  name[i] = '\0';

  /* See if the name is a defined grid */
  pdf = pcm->pdf1;
  while (pdf!=NULL && !cmpwrd(name,pdf->abbrv)) pdf = pdf->pforw;
  if (pdf==NULL) {
    snprintf(pout,1255,"ncwrite error: defined grid %s not found\n",name);
    gaprnt (0,pout);
    goto err;
  }
  pfi = pdf->pfi;

  /* Create the file. */
  if (pcm->sdfwname) filename = pcm->sdfwname;    /* override default filename */
  if (pcm->sdfwtype == 2) 
#if HAVENETCDF4==1
    rc = nc_create(filename, NC_CLOBBER|NC_NETCDF4, &i);
#else
    rc = nc_create(filename, NC_CLOBBER, &i);
#endif
  else
    rc = nc_create(filename, NC_CLOBBER, &i);
  if (rc) {
    snprintf(pout,1255,"ncwrite error from nc_create (filename is %s): \n",filename);
    gaprnt (0,pout);
    handle_error(rc);
    goto err;
  }
  pcm->ncwid = i;

  /* initialize the vdims array */
  vdims[0] = vdims[1] = vdims[2] = vdims[3] = vdims[4] = -999;

  /* Set up the dimensions and the coordinate variables */ 
  padX = padY = padZ = padT = padE = 0;
  if (pcm->sdfwpad) {
    padX = 1;
    padY = 1;
    if (pcm->sdfwpad==1 || pcm->sdfwpad==3 || pcm->sdfwpad==5) padZ = 1;
    if (pcm->sdfwpad>=2) padT = 1;
    if (pcm->sdfwpad>=4) padE = 1;
  }

  /* X is varying or user wants to pad the output with non-varying dims */
  if (pfi->dnum[0]>1 || padX) {
    /* define the dimension */
    if (sdfdefdim (pcm->ncwid, "lon", pfi->dnum[0], &xdimid, &xvarid, 0, pcm->sdfwtype)) goto err;
    /* assign default and user-defined attributes */
    if (sdfwatt(pcm, xvarid, "lon", "units", "degrees_east")) goto err;
    if (sdfwatt(pcm, xvarid, "lon", "long_name", "Longitude")) goto err;
    if (sdfwatt(pcm, xvarid, "lon", NULL, NULL)) goto err;
    /* increment the number of varying dimensions */
    vdims[nvdims] = 0;
    nvdims++;
  }
  else {
    xdimid = -999;
    xvarid = -999;
  }

  /* Y is varying or user wants to pad the output with non-varying dims  */
  if (pfi->dnum[1]>1 || padY) {                    
    /* define the dimension */
    if (sdfdefdim (pcm->ncwid, "lat", pfi->dnum[1], &ydimid, &yvarid, 0, pcm->sdfwtype)) goto err;
    /* assign default and user-defined attributes */
    if (sdfwatt(pcm, yvarid, "lat", "units", "degrees_north")) goto err;
    if (sdfwatt(pcm, yvarid, "lat", "long_name", "Latitude")) goto err;
    if (sdfwatt(pcm, yvarid, "lat", NULL, NULL)) goto err;
    /* increment the number of varying dimensions */
    vdims[nvdims] = 1;
    nvdims++;
  }
  else {
    ydimid = -999;
    yvarid = -999;
  }

  /* Z is varying or user wants to pad the output with non-varying dims */
  if (pfi->dnum[2]>1 || padZ) {                    
    /* define the dimension */
    if (sdfdefdim (pcm->ncwid, "lev", pfi->dnum[2], &zdimid, &zvarid, 0, pcm->sdfwtype)) goto err;
    /* assign default and user-defined attributes */
    if (sdfwatt(pcm, zvarid, "lev", "units", "millibar")) goto err;
    if (sdfwatt(pcm, zvarid, "lev", "long_name", "Level")) goto err;
    if (sdfwatt(pcm, zvarid, "lev", NULL, NULL)) goto err;
    /* increment the number of varying dimensions */
    vdims[nvdims] = 2;
    nvdims++;
  }
  else {
    zdimid = -999;
    zvarid = -999;
  }

  /* T is varying or user wants to pad the output with non-varying dims */
  if (pfi->dnum[3]>1 || padT) {                    
    /* define the dimension */
    recflg=0;
    if ((pcm->sdfrecdim==1) || (pcm->sdfrecdim==3)) recflg=1;
    if (sdfdefdim (pcm->ncwid, "time", pfi->dnum[3], &tdimid, &tvarid, recflg, pcm->sdfwtype)) goto err;
    /* assign default and user-defined attributes */
    if (sdfwatt(pcm, tvarid, "time", "long_name", "Time")) goto err;
    if (sdfwatt(pcm, tvarid, "time", NULL, NULL)) goto err;
    /* write out the time axis units and calendar attributes after the 
       user-defined attributes. This guarantees that they will be correct */
    gr2t (pfi->grvals[3],1.0+pfi->dimoff[3],&tinit);
    if ((*(pfi->grvals[3]+6) != 0))
      snprintf(tunit,255,"minutes since %04d-%02d-%02d %02d:%02d",
	    tinit.yr,tinit.mo,tinit.dy,tinit.hr,tinit.mn);
    else
      snprintf(tunit,255,"months since %04d-%02d-%02d %02d:%02d",
	    tinit.yr,tinit.mo,tinit.dy,tinit.hr,tinit.mn);
    if (sdfwatt(pcm, tvarid, "time", "units", tunit)) goto err;
    /* if necessary, assign 365_day_calendar attribute */
    if ((*(pfi->grvals[3]+6) != 0) && (pfi->calendar==1)) {
      if (sdfwatt(pcm, tvarid, "time", "calendar", "noleap")) goto err;
    }
    /* increment the number of varying dimensions */
    vdims[nvdims] = 3;
    nvdims++;
  }
  else {
    tdimid = -999;
    tvarid = -999;
  }

  /* E is varying or user wants to pad the output with non-varying dims */
  if (pfi->dnum[4]>1 || padE) {                    
    /* define the dimension */
    recflg=0;
    if (pcm->sdfrecdim>=2) recflg=1;
    if (sdfdefdim (pcm->ncwid, "ens", pfi->dnum[4], &edimid, &evarid, recflg, pcm->sdfwtype)) goto err;
    /* assign default and user-defined attributes */
    if (sdfwatt(pcm, evarid, "ens", "grads_dim", "e")) goto err;
    if (sdfwatt(pcm, evarid, "ens", "long_name", "Ensemble member")) goto err;
    if (sdfwatt(pcm, evarid, "ens", NULL, NULL)) goto err;
    /* increment the number of varying dimensions */
    vdims[nvdims] = 4;
    nvdims++;
  }
  else {
    edimid = -999;
    evarid = -999;
  }

  /* Check to make sure at least one dimension is varying */
  if (nvdims==0) {
    gaprnt(0,"ncwrite error: defined variable has no varying dimensions\n");
    goto err;
  }

  /* The dimids array is used to pass the dimids of the dimensions of the variable. 
     The unlimited dimension must come first on the list of dimids, 
     but in this case there is no unlimited dimension.  
     The first dimid is the dimension that varies slowest, the outermost dimension, E in GrADS
     The last  dimid is the dimension that varies fastest, the innermost dimension, X in GrADS 
  */
  for (i=0; i<nvdims; i++) {
    if      (vdims[nvdims-1-i] == 4) dimids[i] = edimid; 
    else if (vdims[nvdims-1-i] == 3) dimids[i] = tdimid; 
    else if (vdims[nvdims-1-i] == 2) dimids[i] = zdimid; 
    else if (vdims[nvdims-1-i] == 1) dimids[i] = ydimid; 
    else if (vdims[nvdims-1-i] == 0) dimids[i] = xdimid; 
  }

  /* define the netCDF variable */
  if (pcm->sdfprec==8) {
    rc = nc_def_var(pcm->ncwid, name, NC_DOUBLE, nvdims, dimids, &varid);
  }
  else if (pcm->sdfprec==4) {
    rc = nc_def_var(pcm->ncwid, name, NC_FLOAT, nvdims, dimids, &varid);
  }
  if (rc) {
    snprintf(pout,1255,"ncwrite error from nc_def_var (%s): \n ",name);
    gaprnt(0,pout);
    handle_error(rc);
    goto err;
  }
 
#if HAVENETCDF4 == 1
  /* If user has not set chunksizes, default chunk is a global 2D grid */
  if (pcm->sdfchunk || pcm->sdfzip) {
    /* set chunk sizes */
    chunksize = (size_t*)galloc(nvdims*sizeof(size_t),"chnksz");
    for (i=0; i<nvdims; i++) {
      if      (vdims[nvdims-1-i] == 4) chunksize[i] = pcm->echunk ? pcm->echunk : 1 ; 
      else if (vdims[nvdims-1-i] == 3) chunksize[i] = pcm->tchunk ? pcm->tchunk : 1 ; 
      else if (vdims[nvdims-1-i] == 2) chunksize[i] = pcm->zchunk ? pcm->zchunk : 1 ; 
      else if (vdims[nvdims-1-i] == 1) chunksize[i] = pcm->ychunk ? pcm->ychunk : pfi->dnum[1] ; 
      else if (vdims[nvdims-1-i] == 0) chunksize[i] = pcm->xchunk ? pcm->xchunk : pfi->dnum[0] ; 
    }
    /* define variable as chunked */
    rc = nc_def_var_chunking (pcm->ncwid, varid, NC_CHUNKED, chunksize);
    if (rc) {
      snprintf(pout,1255,"ncwrite error from nc_def_var_chunking (%s): \n ",name);
      gaprnt(0,pout);
      handle_error(rc);
      goto err;
    }
  }
  if (pcm->sdfzip) {
    /* compression settings: shuffle on, deflate level 1 */
    rc = nc_def_var_deflate (pcm->ncwid, varid, 1, 1, 1); 
    if (rc) {
      snprintf(pout,1255,"ncwrite error from nc_def_var_deflate (%s): \n ",name);
      gaprnt(0,pout);
      handle_error(rc);
      goto err;
    }
  }
#endif
  
  /* assign undef attribute to the netCDF variables. 
     The undef attribute value is taken from pcm->undef instead of pfi->undef.
     When the variable is defined, undef values are copied from pcm->undef.
   */
  if (pcm->sdfprec==8) {
    rc = nc_put_att_double(pcm->ncwid, varid, "_FillValue", NC_DOUBLE, 1, &pcm->undef);
  }
  else if (pcm->sdfprec==4) {
    flundef = (gafloat)pcm->undef;
    rc = nc_put_att_float(pcm->ncwid, varid, "_FillValue", NC_FLOAT, 1, &flundef);
  }
  if (rc) {
    snprintf(pout,1255,"ncwrite error from nc_put_att (%s): \n ",name);
    gaprnt(0,pout);
    handle_error(rc);
    goto err;
  }
  /* add any user-defined attributes */
  if (sdfwatt(pcm, varid, name, NULL, NULL)) goto err;
  if (sdfwatt(pcm, NC_GLOBAL, "global", NULL, NULL)) goto err;

  /* End define mode. */
  rc = nc_enddef(pcm->ncwid);
  if (rc) {
    gaprnt(0,"ncwrite error from nc_enddef: \n ");
    handle_error(rc);
    goto err;
  }

  /* Write the coordinate variable data */
  if (pfi->dnum[0]>1 || padX) if (sdfwdim (pfi, pcm, 0, xvarid)) goto err;
  if (pfi->dnum[1]>1 || padY) if (sdfwdim (pfi, pcm, 1, yvarid)) goto err;
  if (pfi->dnum[2]>1 || padZ) if (sdfwdim (pfi, pcm, 2, zvarid)) goto err;
  if (pfi->dnum[3]>1 || padT) if (sdfwdim (pfi, pcm, 3, tvarid)) goto err;
  if (pfi->dnum[4]>1 || padE) if (sdfwdim (pfi, pcm, 4, evarid)) goto err;

  /* set the start and count array values */
  for (i=0; i<nvdims; i++) {
    if      (vdims[nvdims-1-i] == 4) count[i] = pfi->dnum[4];
    else if (vdims[nvdims-1-i] == 3) count[i] = pfi->dnum[3];
    else if (vdims[nvdims-1-i] == 2) count[i] = pfi->dnum[2];
    else if (vdims[nvdims-1-i] == 1) count[i] = pfi->dnum[1];
    else if (vdims[nvdims-1-i] == 0) count[i] = pfi->dnum[0];
    start[i] = 0;
  }

  /* Write the variable data. */
  nelems = (off_t)pfi->dnum[0] * (off_t)pfi->dnum[1] * (off_t)pfi->dnum[2] * (off_t)pfi->dnum[3] * (off_t)pfi->dnum[4];
  if (pcm->sdfprec==8) {
    /* copy undef values into rbuf array where mask is 0 */
    for (pos=0; pos<nelems; pos++) if (*(pfi->ubuf+pos)==0) *(pfi->rbuf+pos) = pcm->undef;
    /* write the grid of doubles */
    rc = nc_put_vara_double(pcm->ncwid, varid, start, count, pfi->rbuf);
    if (rc) {
      gaprnt(0,"ncwrite error from nc_put_vara_double: \n ");
      handle_error(rc);
      goto err;    
    }
  } 
  else if (pcm->sdfprec==4) {
    /* allocate a new array for float value grid */
    flbuf = (gafloat *)galloc(nelems*sizeof(gafloat),"flbuf");
    if (flbuf==NULL) {
      gaprnt(0,"ncwrite error: unable to allocate memory for floating point buffer\n");
      goto err;
    }
    /* copy undef values into array where mask is 0 */
    for (pos=0; pos<nelems; pos++) {
      if (*(pfi->ubuf+pos)==1)
	*(flbuf+pos) = (gafloat)*(pfi->rbuf+pos);
      else
	*(flbuf+pos) = flundef;
    }
    /* write out the grid of floats */
    rc = nc_put_vara_float(pcm->ncwid, varid, start, count, flbuf);
    if (rc) {
      gaprnt(0,"ncwrite error from nc_put_vara_float: \n ");
      handle_error(rc);
      gree(flbuf,"f333");
      goto err;    
    }
    gree(flbuf,"f334");
  }

  /* Close the file. */
  if (chunksize) gree(chunksize,"f335");
  rc = nc_close(pcm->ncwid);
  if (pcm->sdfwname) 
    snprintf(pout,1255,"Wrote variable %s to %s\n",name,pcm->sdfwname);
  else 
    snprintf(pout,1255,"Wrote variable %s to grads.sdfwrite.nc\n",name);
  gaprnt(2,pout);

  return 0;


 err:
  if (chunksize) gree(chunksize,"f335");
  if (pcm->ncwid != -999) nc_close(pcm->ncwid);
  pcm->ncwid = -999; 
  return 1;
#endif
  return(0);
}


/* Routine to open a shapefile 
   1. Tries to open filename as provided by user
   2. Parses GASHP envv, which can have more than one directory, and prepends dirs to filename
   3. Prepends GADDIR directory (only one dir) to filename 
   N.B. Same code as gaopdbf routine below, make sure any mods go into both routines.
*/
#if USESHP==1
SHPHandle gaopshp(char *shparg) {
  SHPHandle id=NULL;
  gaint i,j,len,sz,ch1,ch2;
  char *sdir,*fname;

  /* get length of shapefile name provided by user (ch2) */
  ch2 = 0;
  i = 0;
  while (*(shparg+i)) { i++; ch2++; }

  /* first try just the filename */
  if ((id=SHPOpen(shparg,"rb"))!=NULL) return(id);

  /* next try the directories in the GASHP environment variable */
  sdir = getenv("GASHP");
  while (sdir!=NULL) {
    /* ch1 is length of directory harvested from GASHP */
    ch1 = 0;
    i = 0;
    /* delimters are: space, semicolon, comma, or colon. NULL is at end of GASHP string  */
    while (*(sdir+i)!=' ' && *(sdir+i)!=';' && *(sdir+i)!=',' && *(sdir+i)!=':' && *(sdir+i)!='\0') {i++; ch1++;} 
    /* build new file name: directory + shparg + extra "/" (if necessary) + "/0" */
    sz = ch1 + ch2 + 2;    
    fname = (char *)galloc(sz,"shpname");
    if (fname==NULL) {
      gaprnt (0,"Memory allocation error for shapefile name\n");
      return(NULL);
    }
    for (j=0; j<ch1; j++) *(fname+j) = *(sdir+j);            /* copy dir harvested from sdir to fname */
    len = ch1;      
    if (*(fname+ch1-1)!='/') { *(fname+ch1) = '/'; len++; }  /* add slash to fname if needed */
    for (j=0; j<ch2; j++) *(fname+len+j) = *(shparg+j);      /* copy shparg to fname */
    len += ch2;
    *(fname+len)='\0';
    /* try to open new filename */
    if ((id=SHPOpen(fname,"rb"))!=NULL) {
      gree(fname);                    
      return(id);                                   /* success! */
    } 
    gree(fname);                                    /* release filename */
    sdir+=ch1;                                      /* advance to delimiter */
    if (*(sdir)=='\0') break;                       /* end of GASHP */
    else sdir++;                                    /* advance past delimiter */
    while (*(sdir)==' ' && *(sdir)!='\0') sdir++;   /* advance past any extra spaces */
  }
  
  /* final option: try prepending GADDIR */
  if ((id=SHPOpen(gxgnam(shparg),"rb"))!=NULL) return(id);
 
  /* give up */
  snprintf(pout,1255,"Error: Unable to open shapefile \"%s\" \n",shparg);
  gaprnt(0,pout);
  return (NULL); 
}

/* open a shapefile attribute data base file 
   1. Tries to open filename as provided by user
   2. Parses GASHP envv, which can have more than one directory, and prepends dirs to filename
   3. Prepends GADDIR directory (only one dir) to filename 
   N.B. Same code as gaopshp routine above, make sure any mods go into both routines.
*/
DBFHandle gaopdbf(char *shparg) {
  DBFHandle id=NULL;
  gaint i,j,len,sz,ch1,ch2;
  char *sdir,*fname;

  /* get length of shapefile name provided by user (ch2) */
  ch2 = 0;
  i = 0;
  while (*(shparg+i)) { i++; ch2++; }

  /* first try just the filename */
  if ((id=DBFOpen(shparg,"rb"))!=NULL) return(id);

  /* next try the directories in the GASHP environment variable */
  sdir = getenv("GASHP");
  while (sdir!=NULL) {
    /* ch1 is length of directory harvested from GASHP */
    ch1 = 0;
    i = 0;
    /* delimters are: space, semicolon, comma, or colon. NULL is at end of GASHP string */
    while (*(sdir+i)!=' ' && *(sdir+i)!=';' && *(sdir+i)!=',' && *(sdir+i)!=':' && *(sdir+i)!='\0') {i++; ch1++;} 
    /* build new file name: directory + shparg + extra "/" (if necessary) + "/0" */
    sz = ch1 + ch2 + 2;    
    fname = (char *)galloc(sz,"shpname");
    if (fname==NULL) {
      gaprnt (0,"Memory allocation error for shapefile name\n");
      return(NULL);
    }
    for (j=0; j<ch1; j++) *(fname+j) = *(sdir+j);            /* copy dir harvested from sdir to fname */
    len = ch1;      
    if (*(fname+ch1-1)!='/') { *(fname+ch1) = '/'; len++; }  /* add slash to fname if needed */
    for (j=0; j<ch2; j++) *(fname+len+j) = *(shparg+j);      /* copy shparg to fname */
    len += ch2;
    *(fname+len)='\0';
    /* try to open new filename */
    if ((id=DBFOpen(fname,"rb"))!=NULL) {
      gree(fname);                    
      return(id);                                   /* success! */
    } 
    gree(fname);                                    /* release filename */
    sdir+=ch1;                                      /* advance to delimiter */
    if (*(sdir)=='\0') break;                       /* end of GASHP */
    else sdir++;                                    /* advance past delimiter */
    while (*(sdir)==' ' && *(sdir)!='\0') sdir++;   /* advance past any extra spaces */
  }
  
  /* final option: try prepending GADDIR */
  if ((id=DBFOpen(gxgnam(shparg),"rb"))!=NULL) return(id);
 
  /* give up */
  snprintf(pout,1255,"Error: Unable to open shapefile \"%s\" \n",shparg);
  gaprnt(0,pout);
  return (NULL); 
}
#endif



/* Parse a shapefile attribute (data base field). 
   Return NULL for error, pointer to dbfld structure if successful */
#if USESHP==1
struct dbfld *parsedbfld (char *ch) {
  gaint jj,len;
  char fldname[512], fldtype[512], fldvalue[512];
  void *value=NULL;
  struct dbfld *field;
  size_t sz;
  DBFFieldType dbtype;
  
  /* get the field name */
  len = 0;
  while (*(ch+len)!=' ' && *(ch+len)!='\n' && *(ch+len)!='\t' && *(ch+len)!='\0') len++;
  if (len>11) {
    gaprnt (2,"Warning: attribute name is too long, truncating to 11 characters\n");
    len = 11;
  }
  for (jj=0; jj<len; jj++) fldname[jj] = *(ch+jj);
  fldname[len] = '\0';
  
  /* get the field type */
  if ( (ch=nxtwrd(ch))==NULL ) {
    gaprnt (2,"Warning: Missing attribute type and value \n"); 
    goto err;
  } 
  len = 0;
  while (*(ch+len)!=' ' && *(ch+len)!='\n' && *(ch+len)!='\t' && *(ch+len)!='\0') len++;
  for (jj=0; jj<len; jj++) fldtype[jj] = *(ch+jj);
  fldtype[len] = '\0';
  
  /* See if field type matches accepted types */
  if (!(strncasecmp(fldtype,"string",6)) || 
      !(strncasecmp(fldtype,"char",4))) {
    dbtype = FTString; 
  } else if (!(strncasecmp(fldtype,"int", 3))) {
    dbtype = FTInteger;
  } else if (!(strncasecmp(fldtype,"double", 6))) {
    dbtype = FTDouble;
  } else {
    gaprnt (2,"Warning: Invalid attribute type \n"); 
    goto err;
  }
  
  /* get the attribute value */
  if ((ch=nxtwrd(ch))==NULL ) {
    gaprnt (2,"Warning: Missing attribute value \n"); 
    goto err;
  } 
  getstr (fldvalue, ch, 512);
  /* Get length of field value */  
  len=strlen(fldvalue);

  /* check if int and double values parse ok -- Maybe not necessary */

  /* allocate space for the field value */
  sz = (len+1) * sizeof(char);
  if ((value = (void *)galloc(sz,"valuec")) == NULL) {
    gaprnt (0,"Error: memory allocation failed for shapefile attribute \n");
    goto err;
  }
  strcpy(value,fldvalue);

  /* Everything parsed and allocated OK, so allocate and populate a dbfld structure */
  sz = sizeof(struct dbfld);
  field = (struct dbfld *) galloc(sz,"dbfield");
  if (field != NULL) {
    strcpy(field->name,fldname);
    field->type = dbtype;
    field->len = len;
    field->value = value;
    field->flag = 0;        /* static field -- value same for each shape */   
    field->next = NULL;
  } else {
    gaprnt (0,"Error: memory allocation failed for attribute metadata\n");
    goto err;
  }
  /* return the pointer to the field structure */
  return field;

 err:
  if (value) gree(value,"g184");
  return(NULL);
}
#endif
 
/* Invoked by gradspy.c, these routines are for passing data between GrADS and Python 

   Notes:  

   gadoexpr() is called by the "result" gradspy method.
   It evaluates an expression (a string provided by the Python user) and returns
   the resulting grid and other metadata, which is hung off a "pygagrid" structure.  
   The caller (gradspy.c) owns this structure. After copying the data from the
   pygagrid structure, the caller then invokes gapyfre() to free up all the GrADS storage.
   Because gadoexpr() calls gaexpr() directly, there is a maximum of two varying dimensions.  

   gadoget() is called by the "get" gradspy method.
   It takes a defined variable name (a string provided by the Python user) and copies the data 
   and metadata into Python. The defined object remains in GrADS memory, but gapyfre() is called
   to release memory allocated to store the grid coordinate values in the pygagrid structure. 

   gasetup() is called by the "put" gradspy method.
   It creates a defined object in GrADS with data and metadata passed from Python. 
   It gets everything GrADS it needs from the pygagrid structure.

   gapyfre() releases any memory that was hung off the pygagrid structure. 

   Station data are not supported in any of these routines.
*/

#include "gradspy.h"

/* function prototypes */
int   gadoexpr (char *, struct pygagrid *);
int   gadoget  (char *, struct pygagrid *);
int   gasetup  (char *, struct pygagrid *);
void  gapyfre  (struct pygagrid *);
gaint defpylev (gadouble *, struct gafile *, gaint);
gaint defpylin (gadouble, gadouble, struct gafile *, gaint);


int gadoexpr (char *expr, struct pygagrid *pypgr) {
  struct gastat *pst;
  struct gafile *pfi;
  struct gacmn *pcm;
  struct gagrid *pgr;
  struct dt dtim; 
  gadouble (*conv) (gadouble *, gadouble);
  gadouble *g, *s, mynan;
  int rc,i,vcnt,nvals;
  char *ch,*u;
  
  /* Initialize */
  mynan = strtod("nan",&ch);
  pypgr->gastatptr = NULL;
  pypgr->grid = NULL;
  pypgr->isiz = 0;
  pypgr->jsiz = 0;
  pypgr->idim = -1;
  pypgr->jdim = -1;
  pypgr->xsz = 1; 
  pypgr->ysz = 1; 
  pypgr->zsz = 1; 
  pypgr->tsz = 1; 
  pypgr->esz = 1;
  pypgr->xstrt = mynan; 
  pypgr->ystrt = mynan; 
  pypgr->zstrt = mynan;
  pypgr->xincr = mynan; 
  pypgr->yincr = mynan; 
  pypgr->zincr = mynan;
  pypgr->syr = 0; 
  pypgr->smo = 0; 
  pypgr->sdy = 0; 
  pypgr->shr = 0; 
  pypgr->smn = 0;
  pypgr->tincr = 0; 
  pypgr->ttyp = 0; 
  pypgr->tcal = 0;
  pypgr->estrt = 0;
  pypgr->xvals = NULL; 
  pypgr->yvals = NULL; 
  pypgr->zvals = NULL;

  pcm = savpcm;
  pst = NULL;
  pfi = NULL; 
  
  /* make sure a file is open */
  if (pcm->pfid==NULL) { 
    printf("Error in gadoexpr: no file open\n"); goto reterr; 
  }

  /* restrict grids to having no more than two varying dimensions */
  vcnt = 0;
  for (i=0; i<5; i++) if (pcm->vdim[i]) vcnt++;
  if (vcnt>2) { 
    printf("Error in gadoexpr: number of varying dimensions may not exceed two\n"); goto reterr; 
  }

  pst = getpst(pcm);
  if (pst==NULL) { printf("Error in gadoexpr: getpst failed\n"); goto reterr; }

  /* evaluate the expression */
  garemb (expr);
  rc = gaexpr (expr, pst);
  if (rc) { printf("Error in gadoexpr: gaexpr failed\n"); goto reterr; }

  /* result must be a grid, not a station data type */
  if (pst->type != 1) { 
    printf("Error in gadoexpr: result is not a gridded data type\n"); goto reterr; }
    
  /* Update elements in the pypgr structure */
  pypgr->gastatptr = (void *)pst;
  pgr = pst->result.pgr;
  pypgr->isiz = pgr->isiz;
  pypgr->jsiz = pgr->jsiz;
  pypgr->idim = pgr->idim;
  pypgr->jdim = pgr->jdim;

  /* update sizes of the varying dimensions */
  if (pgr->idim>=0) {  
    /* idim is varying */
    if (pgr->idim==0) pypgr->xsz = pgr->isiz; 
    if (pgr->idim==1) pypgr->ysz = pgr->isiz;
    if (pgr->idim==2) pypgr->zsz = pgr->isiz;
    if (pgr->idim==3) pypgr->tsz = pgr->isiz;
    if (pgr->idim==4) pypgr->esz = pgr->isiz;
  }
  if (pgr->jdim>=0) {
    /* jdim is varying */
    if (pgr->jdim==0) pypgr->xsz = pgr->jsiz;
    if (pgr->jdim==1) pypgr->ysz = pgr->jsiz;
    if (pgr->jdim==2) pypgr->zsz = pgr->jsiz;
    if (pgr->jdim==3) pypgr->tsz = pgr->jsiz;
    if (pgr->jdim==4) pypgr->esz = pgr->jsiz;
  }
 
  /* populate missing data values with NaN */
  u = pgr->umask;
  g = pgr->grid;
  for (i=0; i<pgr->isiz*pgr->jsiz; i++) if (*(u+i)==0) *(g+i) = mynan;

  /* set the grid pointer in the pypgr structure */
  pypgr->grid = pgr->grid;

  /* Get a suitable gafile structure so we can populate metadata in pypgr structure.
     pgr->pfile is set to NULL for functions tloop, eloop, s2g2d, and those that 
     call gagrvl (aave, mnmx, scorr), so we use the default file instead */
  if (pgr->pfile != NULL)
    pfi = pgr->pfile;
  else
    pfi = pst->pfid;

  /* Longitude */
  conv = pfi->gr2ab[0];
  pypgr->xstrt = conv(pfi->grvals[0],pgr->dimmin[0]); /* initial world coordinate value */
  if (pfi->linear[0]) 
    pypgr->xincr = *(pfi->grvals[0]);                 /* valid grid increment */
  else 
    pypgr->xincr = -1;                                /* negative grid increment */
  /* allocate memory for X coordinate values */
  nvals = 1;
  if (pypgr->xsz > 1) nvals = pypgr->xsz;
  pypgr->xvals = (double *)malloc(nvals*sizeof(double));
  if (pypgr->xvals==NULL) { 
    printf("Error in gadoexpr: memory allocation failed for xvals\n"); goto reterr; }
  /* write world coordinate values, use NaN if dimension isn't varying */
  s = pypgr->xvals;
  if (pypgr->xsz > 1) {                           
    for (i=pgr->dimmin[0]; i<=pgr->dimmax[0]; i++) {
      *s = conv(pfi->grvals[0],(gadouble)i);
      s++;
    }
  } 
  else *s = mynan;

  /* Latitude */
  conv = pfi->gr2ab[1];
  pypgr->ystrt = conv(pfi->grvals[1],pgr->dimmin[1]); /* initial world coordinate value */
  if (pfi->linear[1]) 
    pypgr->yincr = *(pfi->grvals[1]);                 /* valid grid increment */
  else 
    pypgr->yincr = -1;                                /* negative grid increment */
  /* allocate memory for Y coordinate values */
  nvals = 1;
  if (pypgr->ysz > 1) nvals = pypgr->ysz;
  pypgr->yvals = (double *)malloc(nvals*sizeof(double));
  if (pypgr->yvals==NULL) { 
    printf("Error in gadoexpr: memory allocation failed for yvals\n"); goto reterr; }
  /* write world coordinate values, use NaN if dimension isn't varying  */
  s = pypgr->yvals;
  if (pypgr->ysz > 1) {                           
    for (i=pgr->dimmin[1]; i<=pgr->dimmax[1]; i++) {
      *s = conv(pfi->grvals[1],(gadouble)i);
      s++;
    }
  } 
  else *s = mynan; 

  /* Level */
  pypgr->zvals = (double *)malloc(pypgr->zsz*sizeof(double));   
  if (pypgr->zvals==NULL) { printf("Error in gadoexpr: memory allocation failed for zvals\n"); goto reterr; }
  s = pypgr->zvals; 

  if (pypgr->zsz > 1) {
    conv = pfi->gr2ab[2];
    pypgr->zstrt = conv(pfi->grvals[2],pgr->dimmin[2]); /* initial world coordinate value */
    if (pfi->linear[2]) {
      pypgr->zincr = *(pfi->grvals[2]);                 /* valid grid increment */
    }
    else {    
      pypgr->zincr = -1;                                /* set negative grid increment */
      for (i=pgr->dimmin[2]; i<=pgr->dimmax[2]; i++) {
	*s = conv(pfi->grvals[2],(gadouble)i);
	s++;
      }
    }
  }
  else {
    pypgr->zstrt = 1;
    pypgr->zincr = 1;
    *s = mynan;
  }

  /* Time */
  gr2t (pfi->grvals[3],pgr->dimmin[3],&dtim);    /* get the initial time metadata */
  pypgr->syr = dtim.yr;
  pypgr->smo = dtim.mo;
  pypgr->sdy = dtim.dy;
  pypgr->shr = dtim.hr;
  pypgr->smn = dtim.mn;
  pypgr->tcal = pfi->calendar;
  if (*(pfi->grvals[3]+5)!=0) {
    pypgr->ttyp = 0; /* increment is months  */
    pypgr->tincr = *(pfi->grvals[3]+5); 
  }
  else {
    pypgr->ttyp = 1; /* increment is minutes */
    pypgr->tincr = *(pfi->grvals[3]+6); 
  }
  /* Ensemble */
  pypgr->estrt = pgr->dimmin[4]; 

  /* everything is ok, return code is the number of varying dimensions */
  return (vcnt);


reterr:
  /* In case of error, memory is released here; caller does not need to call gapyfre. */
  if (pypgr->xvals != NULL) { free(pypgr->xvals); pypgr->xvals=NULL; }
  if (pypgr->yvals != NULL) { free(pypgr->yvals); pypgr->yvals=NULL; }
  if (pypgr->zvals != NULL) { free(pypgr->zvals); pypgr->zvals=NULL; }
  if (pst) {
    gafree(pst);
    gree(pst,"f104");
  }
  pypgr->gastatptr = NULL;
  return(-999);

}


int gadoget (char *vnm, struct pygagrid *pypgr) {
  struct gafile *pfi;
  struct gacmn *pcm;
  struct gagrid *pgr;
  struct gadefn *pdf;
  struct dt dtim; 
  gadouble (*conv) (gadouble *, gadouble);
  gadouble *g, *s, mynan;
  int rc,i,vcnt,nvals;
  size_t gsz;
  char *ch,*u,name[20];
  
  /* Initialize */
  mynan = strtod("nan",&ch);
  pypgr->gastatptr = NULL;
  pypgr->grid = NULL;
  pypgr->isiz = 0;
  pypgr->jsiz = 0;
  pypgr->idim = -1;
  pypgr->jdim = -1;
  pypgr->xsz = 1; 
  pypgr->ysz = 1; 
  pypgr->zsz = 1; 
  pypgr->tsz = 1; 
  pypgr->esz = 1;
  pypgr->xstrt = mynan; 
  pypgr->ystrt = mynan; 
  pypgr->zstrt = mynan;
  pypgr->xincr = mynan; 
  pypgr->yincr = mynan; 
  pypgr->zincr = mynan;
  pypgr->syr = 0; 
  pypgr->smo = 0; 
  pypgr->sdy = 0; 
  pypgr->shr = 0; 
  pypgr->smn = 0;
  pypgr->tincr = 0; 
  pypgr->ttyp = 0; 
  pypgr->tcal = 0;
  pypgr->estrt = 0;
  pypgr->xvals = NULL; 
  pypgr->yvals = NULL; 
  pypgr->zvals = NULL;

  pcm = savpcm;
  pfi = NULL;

  /* copy defined variable name */
  i=0;
  while (*(vnm+i)!=' ' && *(vnm+i)!='\n' && *(vnm+i)!='\0' && i<19) {
    name[i] = *(vnm+i);
    i++;
  }
  name[i] = '\0';

  /* See if the name is a defined grid */
  pdf = pcm->pdf1;
  while (pdf!=NULL && !cmpwrd(name,pdf->abbrv)) pdf = pdf->pforw;
  if (pdf==NULL) {
    printf("Error in gadoget: defined grid %s not found\n",name);
    goto reterr;
  }

  /* Get the gafile structure for this variable */
  pfi = pdf->pfi;
  
  /* Get the number of varying dimensions, set dimension sizes */
  vcnt = 0;
  gsz = 1;
  for (i=0; i<5; i++) {
    if (pfi->dnum[i] > 1) vcnt++;
    gsz = gsz * (size_t)pfi->dnum[i];
  }

  /* update sizes of the varying dimensions */
  pypgr->xsz = pfi->dnum[0];
  pypgr->ysz = pfi->dnum[1];
  pypgr->zsz = pfi->dnum[2];
  pypgr->tsz = pfi->dnum[3];
  pypgr->esz = pfi->dnum[4];

  /* populate missing data values with NaN */
  u = pfi->ubuf;
  g = pfi->rbuf;
  for (i=0; i<gsz; i++) if (*(u+i)==0) *(g+i) = mynan;
  /* set the grid pointer in the pypgr structure */
  pypgr->grid = pfi->rbuf;

  /* Longitude */
  conv = pfi->gr2ab[0];
  pypgr->xstrt = conv(pfi->grvals[0],1.0);            /* initial world coordinate value */
  if (pfi->linear[0]) 
    pypgr->xincr = *(pfi->grvals[0]);                 /* valid grid increment */
  else 
    pypgr->xincr = -1;                                /* negative grid increment */
  /* allocate memory for X coordinate values */
  nvals = 1;
  if (pypgr->xsz > 1) nvals = pypgr->xsz;
  pypgr->xvals = (double *)malloc(nvals*sizeof(double));
  if (pypgr->xvals==NULL) { printf("Error in gadoget: memory allocation failed for xvals\n"); goto reterr; }
  s = pypgr->xvals;
  /* write world coordinate values, use NaN if dimension isn't varying */
  if (pypgr->xsz > 1) {                           
    for (i=1; i<=pypgr->xsz; i++) {
      *s = conv(pfi->grvals[0],(gadouble)(i));
      s++;
    }
  } 
  else *s = mynan;

  /* Latitude */
  conv = pfi->gr2ab[1];
  pypgr->ystrt = conv(pfi->grvals[1],1.0);            /* initial world coordinate value */
  if (pfi->linear[1]) 
    pypgr->yincr = *(pfi->grvals[1]);                 /* valid grid increment */
  else 
    pypgr->yincr = -1;                                /* negative grid increment */
  /* allocate memory for Y coordinate values */
  nvals = 1;
  if (pypgr->ysz > 1) nvals = pypgr->ysz;
  pypgr->yvals = (double *)malloc(nvals*sizeof(double));
  if (pypgr->yvals==NULL) { printf("Error in gadoget: memory allocation failed for yvals\n"); goto reterr; }
  s = pypgr->yvals;
  /* write world coordinate values, use NaN if dimension isn't varying  */
  if (pypgr->ysz > 1) {                           
    for (i=1; i<=pypgr->ysz; i++) {
      *s = conv(pfi->grvals[1],(gadouble)i);
      s++;
    }
  } 
  else *s = mynan; 

  /* Level */
  pypgr->zvals = (double *)malloc(pypgr->zsz*sizeof(double));   
  if (pypgr->zvals==NULL) { printf("Error in gadoget: memory allocation failed for zvals\n"); goto reterr; }
  s = pypgr->zvals; 
  /* write world coordinate values, use NaN if dimension isn't varying  */
  if (pypgr->zsz > 1) {
    conv = pfi->gr2ab[2];
    pypgr->zstrt = conv(pfi->grvals[2],1.0);            /* initial world coordinate value */
    if (pfi->linear[2]) {
      pypgr->zincr = *(pfi->grvals[2]);                 /* valid grid increment */
    }
    else {    
      pypgr->zincr = -1;                                /* set negative grid increment */
      for (i=1; i<=pypgr->zsz; i++) {
	*s = conv(pfi->grvals[2],(gadouble)i);
	s++;
      }
    }
  }
  else {
    pypgr->zstrt = 1;
    pypgr->zincr = 1;
    *s = mynan;
  }

  /* Time */
  gr2t (pfi->grvals[3],1.0,&dtim);    /* get the initial time metadata */
  pypgr->syr = dtim.yr;
  pypgr->smo = dtim.mo;
  pypgr->sdy = dtim.dy;
  pypgr->shr = dtim.hr;
  pypgr->smn = dtim.mn;
  pypgr->tcal = pfi->calendar;
  if (*(pfi->grvals[3]+5)!=0) {
    pypgr->ttyp = 0; /* increment is months  */
    pypgr->tincr = *(pfi->grvals[3]+5); 
  }
  else {
    pypgr->ttyp = 1; /* increment is minutes */
    pypgr->tincr = *(pfi->grvals[3]+6); 
  }
  /* Ensemble */
  pypgr->estrt = 1.0;

  /* everything is ok, return code is the number of varying dimensions */
  return (vcnt);


reterr:
  /* In case of error, memory is released here; caller does not need to call gapyfre. */
  if (pypgr->xvals != NULL) { free(pypgr->xvals); pypgr->xvals=NULL; }
  if (pypgr->yvals != NULL) { free(pypgr->yvals); pypgr->yvals=NULL; }
  if (pypgr->zvals != NULL) { free(pypgr->zvals); pypgr->zvals=NULL; }
  return(-999);

}


/* This routine is called by the Python extension code to 
   free the pst (and associated storage) once the data is copied. */

void gapyfre (struct pygagrid *pypgr) {
struct gastat *pst;

  if (pypgr->xvals != NULL) { free(pypgr->xvals); pypgr->xvals=NULL; }
  if (pypgr->yvals != NULL) { free(pypgr->yvals); pypgr->yvals=NULL; }
  if (pypgr->zvals != NULL) { free(pypgr->zvals); pypgr->zvals=NULL; }

  pst = (struct gastat *)pypgr->gastatptr;
  if (pst) {
    gafree(pst);
    gree(pst,"f104");
    pst=NULL;
  }
}

/* This routine is called by the Python extension module 'put'
   It sets up a new defined variable using data and metadata 
   that are passed through in the pygagrid structure */

gaint gasetup (char *ch, struct pygagrid *pypgr) {
struct gafile *pypfi,*pfi;
struct gadefn *pypdf,*pdf,*opdf;
struct gacmn  *pcm=NULL;
gaint i,j,gsz,rc,ev1,ev2,add;
char name[20];
gadouble *g,*tvals=NULL,*evals=NULL;
gadouble *pygrid=NULL;
char *pymask=NULL;

  pypfi = pfi = NULL;
  pcm = savpcm;
  
  /* Make sure a file is open */
  if (pcm->pfid==NULL) { 
    printf("Error: No files open. Please open a file, any file, then try again. \n");
    goto reterr; 
  }

  /* Parse the name argument */
  i=0;
  while ((*ch>='a' && *ch<='z') || (*ch>='0' && *ch<='9' )) {
    name[i] = *ch;
    ch++; i++;
    if (i>16) break;
  }
  name[i] = '\0';
  
  /* initialize a file structure for the new defined grid. 
     Use getpfi, but don't put it in the chain of open file structures */
  pypfi = getpfi();
  if (pypfi==NULL) { gaprnt (0,"Error: getpfi failed\n"); goto reterr; }
  pypfi->type = 5;  /* new type created for Python deposits */

  /* Set up the X, Y, and Z dimensions.  
     If the increment is negative and the size is > 1, the dimension is non-linear */
  
  pypfi->dnum[0] = pypgr->xsz;
  if (pypgr->xsz > 1 && pypgr->xincr < 0) 
    rc = defpylev (pypgr->xvals, pypfi, 0);
  else
    rc = defpylin (pypgr->xstrt, pypgr->xincr, pypfi, 0);
  if (rc) {
    gaprnt (0,"Error: Unable to allocate memory for X axis metadata\n");
    goto reterr; 
  }

  pypfi->dnum[1] = pypgr->ysz;
  if ( pypgr->ysz > 1 && pypgr->yincr < 0) 
    rc = defpylev (pypgr->yvals, pypfi, 1);
  else
    rc = defpylin (pypgr->ystrt, pypgr->yincr, pypfi, 1);
  if (rc) {
    gaprnt (0,"Error setting up Y axis metadata\n");
    goto reterr; 
  }

  pypfi->dnum[2] = pypgr->zsz;
  if (pypgr->zsz > 1 && pypgr->zincr < 0) 
    rc = defpylev (pypgr->zvals, pypfi, 2);
  else
    rc = defpylin (pypgr->zstrt, pypgr->zincr, pypfi, 2);
  if (rc) {
    gaprnt (0,"Error setting up Z axis metadata\n");
    goto reterr; 
  }

  /* set up T dimension */
  pypfi->dnum[3] = pypgr->tsz;
  tvals = (gadouble *)galloc(sizeof(gadouble)*8,"pyvals3");
  if (tvals == NULL) {
    gaprnt (0,"Error: Unable to allocate memory for T axis metadata\n");
    goto reterr; 
  }
  *(tvals)   = pypgr->syr;
  *(tvals+1) = pypgr->smo;
  *(tvals+2) = pypgr->sdy;
  *(tvals+3) = pypgr->shr;
  *(tvals+4) = pypgr->smn;
  if (pypgr->ttyp == 0) {
    *(tvals+5) = pypgr->tincr; /* increment is months */
    *(tvals+6) = 0;
  }
  else {
    *(tvals+5) = 0;
    *(tvals+6) = pypgr->tincr; /* increment is minutes */
  }
  *(tvals+7) = -999.9;
  pypfi->grvals[3] = tvals;
  pypfi->abvals[3] = tvals;
  pypfi->calendar = pypgr->tcal;
  pypfi->linear[3] = 1;

  /* set up E dimension */
  pypfi->dnum[4] = pypgr->esz;
  ev1 = ev2 = 1;
  rc = defpylin (ev1, ev2, pypfi, 4);
  if (rc) {
    gaprnt (0,"Error: Unable to allocate memory for E axis metadata\n");
    goto reterr; 
  }
 
  /* allocate memory for grid and undef mask */
  gsz = pypgr->xsz * pypgr->ysz * pypgr->zsz * pypgr->tsz * pypgr->esz;
  pygrid = (gadouble *)galloc(gsz*sizeof(gadouble),"pygrid");  
  if (pygrid==NULL) {
    gaprnt (0,"Error: Unable to allocate memory for data grid\n");
    snprintf(pout,1255,"       Size of request was %d grid elements\n",gsz);
    gaprnt (0,pout);
    goto reterr;
  }
  pymask = (char *)galloc(gsz*sizeof(char),"pymask");  
  if (pymask==NULL) {
    gaprnt (0,"Error: Unable to allocate memory for data mask\n");
    snprintf(pout,1255,"       Size of request was %d grid elements\n",gsz);
    gaprnt (0,pout);
    goto reterr;
  }

  /* copy grid values and set the undef mask */
  g = pypgr->grid;
  for (i=0; i<gsz; i++) {
    *(pygrid+i) = *(g+i);
    *(pymask+i) = isnan(*(g+i)) ? 0 : 1 ;
  }
  pypfi->rbuf = pygrid;
  pypfi->ubuf = pymask;

  snprintf(pout,1255,"Put memory allocation size = %ld bytes\n",(size_t)(gsz*sizeof(gadouble)));
  gaprnt (2,pout);

  /* Now we will add our new object to the chain of define blocks.
     First, check for an existing defined object with the same name */
  pdf = pcm->pdf1;
  opdf = NULL;
  while (pdf!=NULL) {
    if (cmpwrd(name,pdf->abbrv)) break;
    opdf = pdf;
    pdf = pdf->pforw;
  }
  if (pdf!=NULL) {
    /* name was found, pdf is pointing to the object we need to delete */
    snprintf(pout,1255,"Name already DEFINEd: '%s' will be deleted and replaced\n",name);
    gaprnt (2,pout);
    if (opdf==NULL) {
      /* pdf was the first in the chain, so we promote the forward link to anchor */
      pcm->pdf1 = pdf->pforw;
    }
    else {
      /* attach the forward link to the previous link */
      opdf->pforw = pdf->pforw;
    }
    /* now we can delete pdf */
    pfi = pdf->pfi;
    gree(pfi->rbuf,"g159");
    gree(pfi->ubuf,"g160");
    for (i=0; i<5; i++) {
      if (pfi->grvals[i]!=NULL) {
	gree(pfi->grvals[i],"g161");
	if (pfi->type!=5) gree(pfi->abvals[i],"g161");
      }
    }
    gree(pfi,"g163");
    gree(pdf,"g164");
  }

  /* Allocate the structure for a defined variable */
  pypdf = NULL;
  pypdf = (struct gadefn *)galloc(sizeof(struct gadefn),"pypdf");
  if (pypdf==NULL) {
    gaprnt (0,"Error: failed to allocate memory for a gadefn structure \n");
    goto reterr;
  }
  getwrd(pypdf->abbrv,name,19);  /* copy the name */
  pypdf->pfi = pypfi;            /* set the gafile pointer */
  pypdf->pforw = NULL;

  /* chain it up */
  if (pcm->pdf1==NULL) {
    pcm->pdf1 = pypdf; /* new structure is the anchor */
    pdf = pcm->pdf1;
  }
  else {
    pdf = pcm->pdf1;
    while (pdf->pforw != NULL) pdf = pdf->pforw;
    pdf->pforw = pypdf; 
  }  

  return(0);

reterr:
  if (pypdf)  gree(pypdf,"a");
  if (pygrid) gree(pygrid,"b");
  if (pymask) gree(pymask,"c");
  if (tvals)  gree(evals,"d");
  if (evals)  gree(evals,"e");
  return(1);
}

/* This is the equivalent of deflin(), without the parsing.
   Return codes:  -1 is memory allocation error, 0 for success */

gaint defpylin (gadouble v1, gadouble v2, struct gafile *pfi, gaint dim) {
gadouble *vals,temp;
size_t sz;
char name[8];

  sz = sizeof(gadouble)*6;
  sprintf(name,"pyvals%d",dim);  
  vals = (gadouble *)galloc(sz,name);
  if (vals==NULL) return (-1);
  *(vals)   = v2;
  *(vals+1) = v1 - v2;
  *(vals+2) = -999.9;
  *(vals+3) = 1.0/v2;
  *(vals+4) = -1.0 * ( (v1-v2)/v2 );
  *(vals+5) = -999.9;

  pfi->grvals[dim] = vals;
  pfi->abvals[dim] = vals+3;
  pfi->ab2gr[dim] = liconv;
  pfi->gr2ab[dim] = liconv;
  pfi->linear[dim] = 1;

  /* No harm in adding the wrap test, while we're here.
     Python defined objects can wrap, GrADS defined objects do not */
  if (dim==0) {
    temp = v1+((gadouble)(pfi->dnum[dim]))*v2;
    temp=temp-360.0;
    if (fabs(temp-v1)<0.01) pfi->wrap = 1;
  }
  return (0);
}

/* This is the equivalent of deflev(), without the parsing. 
   Return codes:  -1 is memory allocation error, 0 for success */

gaint defpylev (gadouble *pylevs, struct gafile *pfi, gaint dim) {
gadouble *vvs,*vals;
gaint i;
char name[8];
size_t sz;

  sz = (pfi->dnum[dim]+5)*sizeof(gadouble);
  sprintf(name,"pyvals%d",dim);  
  vals = (gadouble *)galloc(sz,name);
  if (vals==NULL) return (-1);
  vvs = vals;
  *vvs = (gadouble)pfi->dnum[dim]; 
  vvs++;
  for (i=0; i<pfi->dnum[dim]; i++) { 
    *vvs = *(pylevs+i); 
    vvs++; 
  }
  *vvs = -999.9;
  pfi->abvals[dim] = vals;
  pfi->grvals[dim] = vals;
  pfi->ab2gr[dim] = lev2gr;
  pfi->gr2ab[dim] = gr2lev;
  pfi->linear[dim] = 0;
  return (0);
}
