/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* Authored by B. Doty */

/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* This file contains the routines that implement the GrADS scripting
   language.  */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "grads.h"
#include "gs.h"

static char *rcdef = "rc              ";
static char *redef = "result          ";

/*  Execute a script, from one or more files. 
    Beware: various levels of recursion are used.     */

char *gsfile (char *cmd, gaint *retc, gaint impp) {
struct gsvar *parg;
struct gscmn *pcmn;
char *ch,*fname,*res;
gaint len,rc,i;

  /* Point to (and delimit) the file name from the cmd line */

  while (*cmd==' ') cmd++;
  fname = cmd;
  while (*cmd!=' '&& *cmd!='\0'&&*cmd!='\n') cmd++;
  if (*cmd=='\n') *cmd = '\0';
  else if (*cmd==' ') {
    *cmd = '\0';
    cmd++;
    while (*cmd==' ') cmd++;
  }

  /* Allocate the common structure; this anchors most allocated
     memory for executing this script */

  pcmn = (struct gscmn *)malloc(sizeof(struct gscmn));
  if (pcmn==NULL) {
    printf ("Error executing script file: --> %s <--\n",fname);
    printf ("  Memory allocation error \n");
    *retc = 1;
    return(NULL);
  }
  pcmn->ffdef = NULL;
  pcmn->lfdef = NULL;
  pcmn->frecd = NULL;
  pcmn->lrecd = NULL;
  pcmn->fvar = NULL;
  pcmn->ffnc = NULL;
  pcmn->iob = NULL;
  pcmn->gvar = NULL;
  pcmn->farg = NULL;
  pcmn->fname = fname;  /* Don't free this later */
  pcmn->fprefix = NULL;
  pcmn->ppath = NULL;
  pcmn->rres = NULL;
  pcmn->gsfflg = 0;   /* No dynamic functions by default.
                         The gsfallow function controls this. */
  res = NULL;

  /* Open, read, and scan the script file. */
  
  rc = gsgsfrd (pcmn,0,fname);

  if (rc) {
    gsfree(pcmn);
    if (rc==9) {
      if (impp) {      /* This should be handled by caller -- fix later */
        gaprnt (0,"Unknown command: ");
        gaprnt (0,fname);
        gaprnt (0,"\n");
      } else {
        printf ("Error opening script file: %s\n",fname);
      }
    }
    *retc = 1;
    return(NULL);
  }

  /* Get ready to start executing the script.
     Allocate a var block and provide arg string */

  parg = (struct gsvar *)malloc(sizeof(struct gsvar));
  if (parg==NULL) {
    printf ("Memory allocation error:  Script variable buffering\n");
    goto retrn;
  }
  parg->forw = NULL;
  ch = cmd;
  len = 0;
  while (*(ch+len)!='\0' && *(ch+len)!='\n') len++;
  parg->strng = (char *)malloc(len+1);
  if (parg->strng==NULL) {
    printf ("Memory allocation error:  Script variable buffering\n");
    free (parg);
    goto retrn;
  }
  for (i=0; i<len; i++) *(parg->strng+i) = *(ch+i);
  *(parg->strng+len) = '\0';
  pcmn->farg = parg;

  /* Execute the main function. */

  rc = gsrunf(pcmn->frecd, pcmn);
  res = pcmn->rres;
  if (rc==999) rc = -1;

  /*  We are done.  Return.  */

retrn:
  gsfree (pcmn);
  *retc = rc;
  return (res);
}

/* Free gscmn and associated storage */

void gsfree (struct gscmn *pcmn) {
struct gsfdef *pfdf, *tfdf;
struct gsrecd *precd, *trecd;
struct gsfnc *pfnc, *tfnc;
struct gsiob *piob, *tiob;

  pfdf = pcmn->ffdef;
  while (pfdf) {
    tfdf = pfdf->forw;
    if (pfdf->name) free(pfdf->name);
    if (pfdf->file) free(pfdf->file);
    free (pfdf);
    pfdf = tfdf;
  }
  gsfrev(pcmn->gvar);
  gsfrev(pcmn->fvar);
  precd = pcmn->frecd; 
  while (precd) {
    trecd = precd->forw;
    free (precd);
    precd = trecd;
  }
  pfnc = pcmn->ffnc;
  while (pfnc) {
    tfnc = pfnc->forw;
    free (pfnc);
    pfnc = tfnc;
  }
  piob = pcmn->iob;
  while (piob) {
    fclose (piob->file);
    free (piob->name);
    tiob = piob->forw;
    free (piob);
    piob = tiob;
  }
  if (pcmn->fprefix) free(pcmn->fprefix);
  if (pcmn->ppath) free(pcmn->ppath);
  free (pcmn);
}

/* Read in the main script or a script function (.gsf)
   and scan the contents, adding to the chain of 
   recd descriptors if appropriate.  
   When lflag is zero we are reading the main script; 
   when 1 we are handling a .gsf file (and the name of
   the function is provided as pfnc.

   return codes:  0:  normal
                  1:  error; message already printed
                  9:  couldn't open file; message not yet printed */
   
gaint gsgsfrd (struct gscmn *pcmn, gaint lflag, char *pfnc) {
struct gsfdef *pfdf, *tfdf; 
struct gsrecd *rectmp, *reccur=NULL;
char *sfile,*fpos,*ch;
FILE *ifile;
gaint rc,flen,len,reccnt,first;

  /* First allocate a gsfdef file, and chain it off of
     gscmn.  Gets freed at end of script execution; we are
     careful to set NULLS so things are freed properly
     if an error occurs and execution falls thru. */ 

  pfdf = (struct gsfdef *)malloc(sizeof(struct gsfdef));
  if (pfdf==NULL) {
    printf ("Memory allocation error:  script initialization\n");
    return (1);
  }

  if (pcmn->ffdef==NULL) pcmn->ffdef = pfdf;
  else {
    tfdf = pcmn->ffdef;
    while (tfdf->forw) tfdf = tfdf->forw;
    tfdf->forw = pfdf;
  }
  pfdf->forw = NULL;
  pfdf->name = NULL;
  pfdf->file = NULL;

  pcmn->lfdef = pfdf;

  /* Open the file */

  if (lflag==0) {
    ifile = gsonam(pcmn, pfdf);
  } else {
    ifile = gsogsf(pcmn, pfdf, pfnc);
  }

  if (ifile==NULL) return (9);

  /* Read in the file */

  fseek(ifile,0L,2);
  flen = ftell(ifile);
  fseek(ifile,0L,0);

  sfile = (char *)malloc(flen+1);
  if (sfile==NULL) {
    printf ("Error executing script file: %s\n",pfdf->name);
    printf ("  Unable to allocate memory for file read\n");
    return(1);
  }

  len = flen;
  fpos = sfile;
  while (len>511) {
    rc = fread(fpos,1,512,ifile);
    if (rc!=512) {
      printf ("I/O Error reading script file: %s\n",pfdf->name);
      free (sfile);
      return (1);
    }
    fpos+=512;
    len-=512;
  }
  rc = fread(fpos,1,len,ifile);
  if (rc!=len) {
    printf ("I/O Error reading script file: %s\n",pfdf->name);
    printf ("  Return code = %i, %i\n",rc, len);
    free (sfile);
    return (1);
  }
  fclose (ifile);
  *(sfile+flen) = '\0';

  /* Remove cr for PC version */

  ch = sfile;
  while (*ch!='\0') {
    if ((gaint)(*ch)==13) *ch = ' ';
    if ((gaint)(*ch)==10) fpos = ch;
    ch++;
  }
  flen = (fpos-sfile) + 1;
  *(sfile+flen) = '\0';

  /* Above for pc version */

  pfdf->file = sfile;

  /* Build link list of record descriptor blocks.
     Append to existing list if handling a .gsf */

  first = 1;
  fpos = sfile;
  reccnt = 1;
  while (fpos-sfile<flen) {
    rectmp = gsrtyp (&fpos,&reccnt,&rc);
    if (rc) return (1); 
    if (rectmp!=NULL) {
      if (pcmn->frecd==NULL) {
        pcmn->frecd = rectmp;
        pfdf->precd = rectmp;
        reccur = rectmp;
        first = 0;
      } else {
        if (first) {
          reccur = pcmn->lrecd;
          pfdf->precd = rectmp;
          first = 0;
        }
        reccur->forw = rectmp;
        reccur = rectmp;
      }
      reccur->forw = NULL;
      reccur->pfdf = pfdf;
    }
  }
  pcmn->lrecd = reccur;

  /* Resolve flow-control blocks */

  rc = gsblck (pfdf->precd, pcmn);
  if (rc) return(1);  

  return(0);
}

/* Determine what kind of record in the script file we have,
   and fill in a record descriptor block.                  */

struct gsrecd *gsrtyp (char **ppos, gaint *reccnt, gaint *rc) {
char *fpos,*pos;
struct gsrecd *recd;
char ch[20];
gaint i, eflg, cflg;

   /* Ignore comments */

   fpos = *ppos;
   if (*fpos=='*' || *fpos=='#') {
     while (*fpos!='\n') fpos++;
     fpos++;
     *ppos = fpos;
     *rc = 0;
     *reccnt = *reccnt+1;
     return (NULL);
   }

   /* Ignore blank lines */

   while (*fpos==' ') fpos++;
   if (*fpos=='\n' || *fpos==';') {
     if (*fpos=='\n') *reccnt = *reccnt+1;
     fpos++;
     *ppos = fpos;
     *rc = 0;
     return (NULL);
   }

   /* We found something, so allocate a descriptor block */

   recd = (struct gsrecd *)malloc(sizeof(struct gsrecd));
   if (recd==NULL) {
     printf ("Memory allocation error: script scan\n");
     *rc = 1;
     return(NULL);
   }
   recd->forw = NULL;
   recd->pos = fpos;
   recd->num = *reccnt;
   recd->refer = NULL;

   /* Check for assignment statement first */

   eflg = 0;
   recd->epos = NULL;
   pos = fpos;
   recd->type = -9;
   if ((*pos>='a'&&*pos<='z')||(*pos>='A'&&*pos<='Z')||*pos=='_') {
     while ( (*pos>='a' && *pos<='z') ||
             (*pos>='A' && *pos<='Z') ||
             (*pos=='.') || (*pos=='_') ||
             (*pos>='0' && *pos<='9') ) pos++;
     while (*pos==' ') pos++;
     if (*pos=='=') {
       recd->type = 2;
       fpos = pos+1;
       eflg = 1;
     }
   }

   /* Check for other keywords:  if, while, etc.  */

   if (recd->type!=2) {
     i = 0;
     while (*(fpos+i)!='\n' && *(fpos+i)!=';' && i<9) {
       ch[i] = *(fpos+i);
       i++;
     }
     ch[i] = '\0';
     lowcas(ch);

     if (cmpwrd(ch,"if")||!cmpch(ch,"if(",3)) {
       fpos+=2;
       eflg = 1;
       recd->type = 7;
     } else if (cmpwrd(ch,"else")) {
       fpos+=4;
       recd->type = 8;
     } else if (cmpwrd(ch,"endif")) {
       fpos+=5;
       recd->type = 9;
     } else if (cmpwrd(ch,"while")||!cmpch(ch,"while(",6)) {
       fpos+=5;
       eflg = 1;
       recd->type = 3;
     } else if (cmpwrd(ch,"endwhile")) {
       fpos+=8;
       recd->type = 4;
     } else if (cmpwrd(ch,"continue")) {
       fpos+=8;
       recd->type = 5;
     } else if (cmpwrd(ch,"break")) {
       fpos+=5;
       recd->type = 6;
     } else if (cmpwrd(ch,"return")||!cmpch(ch,"return(",7)) {
       fpos+=6;
       recd->type = 10;
       eflg = 1;
     } else if (cmpwrd(ch,"function")) {
       fpos+=8;
       recd->type = 11;
       eflg = 1;
     } else if (cmpwrd(ch,"say")) {
       fpos+=3;
       recd->type = 12;
       eflg = 1;
     } else if (cmpwrd(ch,"print")) {
       fpos+=5;
       recd->type = 12;
       eflg = 1;
     } else if (cmpwrd(ch,"prompt")) {
       fpos+=6;
       recd->type = 15;
       eflg = 1;
     } else if (cmpwrd(ch,"pull")) {
       fpos+=4;
       recd->type = 13;
       eflg = 1;
     } else if (cmpwrd(ch,"exit")) {
       fpos+=4;
       recd->type = 14;
       eflg = 1;
     } else {
       recd->type = 1;
       recd->epos = fpos;
     }
   }

   /* Locate expression */

   if (eflg) {
     while (*fpos==' ') fpos++;
     if (*fpos=='\n' || *fpos==';') {
       recd->epos = NULL;
     } else recd->epos = fpos;
   }

   /* Advance to end of record */

   cflg = 0;
   while (1) {
     if (!cflg && *fpos==';') break;
     if (*fpos=='\n') break;
     if (*fpos =='\'') {
       if (cflg==1) cflg = 0;
       else if (cflg==0) cflg = 1;
     } else if (*fpos=='\"') {
       if (cflg==2) cflg = 0;
       else if (cflg==0) cflg = 2;
     }
     fpos++;
   }

   /* Remove trailing blanks */

   pos = fpos-1;
   while (*pos==' ') {*pos='\0'; pos--;}

   /* Finish building rec block and return */

   if (*fpos=='\n') *reccnt = *reccnt + 1;
   *fpos = '\0';
   fpos++;
   *ppos = fpos;
   *rc = 0;
   return (recd);
}

/*  Resolve flow-control blocks.  Scan each function
    seperately.                                      */

gaint gsblck (struct gsrecd *recd, struct gscmn *pcmn) {
struct gsfnc *pfnc,*prev,*cfnc;
gaint rc,i;
char *fch;

  /* Loop looking at statements.  If a function definition, allocate a
     function block and chain it.   */

  while (recd) {
    recd = gsbkst (recd, NULL, NULL, &rc);
    if (rc) return(rc);

    /* If a function, allocate a function block */

    if (recd!=NULL && recd->type==11) {
      pfnc = (struct gsfnc *)malloc(sizeof(struct gsfnc));
      if (pfnc==NULL) {
        printf ("Error allocating memory: script scan\n");
        return(1);
      }

      /* Chain it */

      if (pcmn->ffnc==NULL) {
        pcmn->ffnc = pfnc;
      } else {
        cfnc = pcmn->ffnc;
        while (cfnc) {
          prev = cfnc;
          cfnc = cfnc->forw;
        }
        prev->forw = pfnc;
      }
      pfnc->forw = NULL;

      /* Fill it in */

      pfnc->recd = recd;
      for (i=0; i<16; i++) pfnc->name[i]=' ';
      fch = recd->epos;
      if (fch==NULL) goto err;
      if ((*fch>='a'&&*fch<='z')||(*fch>='A'&&*fch<='Z')) {
        i = 0;
        while ( (*fch>='a'&&*fch<='z') ||
                (*fch>='A'&&*fch<='Z') ||
                (*fch>='0'&&*fch<='9') ||
                 *fch=='_' ) {
          if (i>15) {
            printf ("Function name too long\n");
            goto err;
          }
          pfnc->name[i] = *fch;
          fch++; i++;
        }
      } else {
        printf ("Invalid function name\n");
        goto err;
      }
      while (*fch==' ') fch++;
      if (*fch==';'||*fch=='\0') recd->epos = NULL;
      else recd->epos = fch;
      recd = recd->forw;
    }
  }
  return (0);

err:
  printf ("Error in %s: Invalid function statement",recd->pfdf->name);
  printf (" at line %i\n",recd->num);
  printf ("  In file %s\n",recd->pfdf->name);
  return (1);
}

/*  Figure out status of a statement.  Recursively resolve
    if/then/else and while/endwhile blocks.  Return pointer
    to next statement (unless a function statement). */

struct gsrecd *gsbkst (struct gsrecd *recd, struct gsrecd *ifblk,
                                     struct gsrecd *doblk, gaint *rc) {
gaint ret;

  if (recd->type==3) {
    recd = gsbkdo(recd->forw, NULL, recd, &ret);
    if (ret) { *rc = ret; return(NULL);}
  }
  else if (recd->type==4) {
    printf ("Unexpected endwhile.  Incorrect loop nesting.\n");
    if (ifblk) {
       printf ("  Expecting endif before endwhile for ");
       printf ("if statement at line %i\n",ifblk->num);
    }
    printf ("  Error occurred scanning line %i\n",recd->num);
    printf ("  In file %s\n",recd->pfdf->name);
    *rc = 1;
    return(NULL);
  }
  else if (recd->type==5) {
    if (doblk==NULL) {
      printf ("Unexpected continue.  No associated while\n");
      printf ("  Error occurred scanning line %i\n",recd->num);
      printf ("  In file %s\n",recd->pfdf->name);
      printf ("  Statement is ignored\n");
    }
    recd->refer = doblk;
  }
  else if (recd->type==6) {
    if (doblk==NULL) {
      printf ("Unexpected break.  No associated while\n");
      printf ("  Error occurred scanning line %i\n",recd->num);
      printf ("  In file %s\n",recd->pfdf->name);
      printf ("  Statement is ignored\n");
    }
    recd->refer = doblk;
  }
  else if (recd->type==7) {
    recd = gsbkif(recd->forw, recd, doblk, &ret);
    if (ret) { *rc = ret; return(NULL);}
  }
  else if (recd->type==8) {
    printf ("Unexpected else.  Incorrect if block nesting.\n");
    printf ("  Error occurred scanning line %i\n",recd->num);
    printf ("  In file %s\n",recd->pfdf->name);
    *rc = 1;
    return(NULL);
  }
  else if (recd->type==9) {
    printf ("Unexpected endif.  Incorrect if block nesting.\n");
    printf ("  Error occurred scanning line %i\n",recd->num);
    printf ("  In file %s\n",recd->pfdf->name);
    *rc = 1;
    return(NULL);
  }
  else if (recd->type==11) {
    *rc = 0;
    return (recd);
  }
  *rc = 0;
  return (recd->forw);
}

/* Resolve an while/endwhile block.  Recursively resolve any
   nested elements. */

struct gsrecd *gsbkdo (struct gsrecd *recd, struct gsrecd *ifblk,
                                     struct gsrecd *doblk, gaint *rc) {
gaint ret;

  ret = 0;
  while (recd!=NULL && recd->type!=4 && recd->type!=11 && ret==0) {
    recd = gsbkst(recd, ifblk, doblk, &ret);
  }
  if (ret==0 && (recd==NULL || recd->type==11)) {
    printf ("Unable to locate ENDWHILE statement");
    printf (" for the WHILE statement at line %i\n",doblk->num);
    printf ("  In file %s\n",doblk->pfdf->name);
    *rc = 1;
    return(NULL);
  }
  *rc = ret;
  if (ret==0) {
    recd->refer = doblk;
    doblk->refer = recd;
    return (recd);
  } else return(NULL);
}

/*  Resolve if/else/endif block */

struct gsrecd *gsbkif (struct gsrecd *recd, struct gsrecd *ifblk,
                                     struct gsrecd *doblk, gaint *rc) {
gaint ret,eflg;
struct gsrecd *elsblk=NULL;

  eflg = 0;
  ret = 0;
  while (recd!=NULL && recd->type!=11 && recd->type!=9 && ret==0) {
    if (recd->type==8 && eflg==0) {
      elsblk = recd;
      eflg = 1;
      recd = recd->forw;
    } else recd = gsbkst(recd, ifblk, doblk, &ret);
  }
  if (ret==0 && (recd==NULL || recd->type==11)) {
    printf ("Unable to locate ENDIF statement");
    printf (" for the IF statement at line %i\n",ifblk->num);
    printf ("  In file %s\n",ifblk->pfdf->name);
    *rc = 1;
    return(NULL);
  }
  *rc = ret;
  if (ret==0) {
    recd->refer = ifblk;
    if (eflg) {
      ifblk->refer = elsblk;
      elsblk->refer = recd;
    } else {
      ifblk->refer = recd;
    }
    return (recd);
  } else return(NULL);
}

/* Execute the function pointed to by recd and with the
   arguments pointed to by farg in pcmn */

gaint gsrunf (struct gsrecd *recd, struct gscmn *pcmn) {
struct gsvar *fvar, *tvar, *avar, *nvar, *svar;
gaint i, ret, len;
char fnm[20],*ch;

  svar = pcmn->fvar;     /* Save caller's args  */
  avar = NULL;           /* Create new arg list */

  /* First two variables in var list are rc and result */

  fvar = NULL;
  fvar = (struct gsvar *)malloc(sizeof(struct gsvar));
  if (fvar==NULL) goto merr;
  fvar->forw = NULL;
  for (i=0; i<16; i++) fvar->name[i] = *(rcdef+i);
  fvar->strng = (char *)malloc(1);
  if (fvar->strng==NULL) goto merr;
  *(fvar->strng) = '\0';

  tvar = (struct gsvar *)malloc(sizeof(struct gsvar));
  if (tvar==NULL) goto merr;
  tvar->forw = NULL;
  fvar->forw = tvar;
  for (i=0; i<16; i++) tvar->name[i] = *(redef+i);
  tvar->strng = (char *)malloc(1);
  if (tvar->strng==NULL) goto merr;
  *(tvar->strng) = '\0';

  /* If the recd is a function record, check the prototype
     list to assign variables.  Add these variables to
     the variable list */

  avar = pcmn->farg;
  if (recd->type==11 && recd->epos) {
    ch = recd->epos;
    if (*ch!='(') goto argerr;
    ch++;
    while (1) {
      while (*ch==' ') ch++;
      if (*ch==')') break;
      if ((*ch>='a'&&*ch<='z') || (*ch>='A'&&*ch<='Z')) {
        len = 0;
        for (i=0; i<16; i++) fnm[i] = ' ';
        while ( (*ch>='a' && *ch<='z') ||
                (*ch>='A' && *ch<='Z') ||
                (*ch=='.') || (*ch=='_') ||
                (*ch>='0' && *ch<='9') ) {
          fnm[len] = *ch;
          len++; ch++;
          if (len>15) goto argerr;
        }
      } else goto argerr;
      if (avar) {
        nvar = avar;
        avar = avar->forw;
      } else {
        nvar = (struct gsvar *)malloc(sizeof(struct gsvar));
        if (nvar==NULL) goto merr;
        nvar->strng = (char *)malloc(len+1);
        if (nvar->strng==NULL) {
          free(nvar);
          goto merr;
        }
        for (i=0; i<len; i++) *(nvar->strng+i) = fnm[i];
        *(nvar->strng+len) = '\0';
      }
      for (i=0; i<16; i++) nvar->name[i] = fnm[i];
      tvar->forw = nvar;
      nvar->forw = NULL;
      tvar = nvar;
      while (*ch==' ') ch++;
      if (*ch==')') break;
      if (*ch==',') ch++;
    }
  }

  /* If the calling arg list was too long, discard the
     unused var blocks */

  gsfrev (avar);

  /* Execute commands until we are done.  Flow control is
     handled recursively. */

  pcmn->fvar = fvar;
  pcmn->rc = 0;
  ret = 0;
  if (recd->type==11) recd = recd->forw;
  while (recd && ret==0) {
    recd = gsruns(recd, pcmn, &ret);
  }
  if (ret==1 || ret==2) {
    printf ("Error in gsrunf:  Internal Logic Check 8\n");
    ret = 1;
  } else if (ret==3) ret=0;
  gsfrev (fvar);
  pcmn->fvar = svar;       /* Restore caller's arg list */
  return (ret);

merr:

  printf ("Error allocating variable memory\n");
  gsfrev(fvar);
  gsfrev(avar);
  pcmn->fvar = svar;
  return (99);

argerr:

  printf ("Error:  Invalid function list\n");
  printf ("  Error occurred on line %i\n",recd->num);
  printf ("  In file %s\n",recd->pfdf->name);
  gsfrev(fvar);
  gsfrev(avar);
  pcmn->fvar = svar;
  return (99);
}

/* Free a link list of variable blocks */

void gsfrev (struct gsvar *var) {
struct gsvar *nvar;

  while (var) {
    nvar = var->forw;
    if (var->strng) free (var->strng);
    free (var);
    var = nvar;
  }
}

/* Execute a statement in the scripting language */

struct gsrecd *gsruns (struct gsrecd *recd, struct gscmn *pcmn, gaint *rc) {
gaint ret, ntyp;
gaint lv;
gadouble vv;
char *res;

  if (gaqsig()) {
    *rc = 99;
    return (NULL);
  }

  /* Statement */

  if (recd->type==1) {
    *rc = gsstmt (recd, pcmn);
    return (recd->forw);
  }

  /* Assignment */

  else if (recd->type==2) {
    *rc = gsassn (recd, pcmn);
    return (recd->forw);
  }

  /* While */

  else if (recd->type==3) {
    recd = gsrund (recd, pcmn, &ret);
    *rc = ret;
    return (recd);
  }

  /* Endwhile */

  else if (recd->type==4) {
    printf ("Error in gsruns:  Internal Logic Check 8\n");
    *rc = 99;
    return (NULL);
  }

  /* Continue */

  else if (recd->type==5) {
    if (recd->refer) {
      *rc = 1;
      return (NULL);
    }
  }

  /* Break */

  else if (recd->type==6) {
    if (recd->refer) {
      *rc = 2;
      return (NULL);
    }
  }

  /* If */

  else if (recd->type==7) {
    recd = gsruni (recd, pcmn, &ret);
    *rc = ret;
    return (recd);
  }

  /* Else */

  else if (recd->type==8) {
    printf ("Error in gsruns:  Internal Logic Check 12\n");
    *rc = 99;
    return (NULL);
  }

  /* Endif */

  else if (recd->type==9) {
    printf ("Error in gsruns:  Internal Logic Check 16\n");
    *rc = 99;
    return (NULL);
  }

  /* Return */

  else if (recd->type==10) {
    if (recd->epos) {
      pcmn->rres = gsexpr(recd->epos, pcmn);
      if (pcmn->rres==NULL) {
        printf ("  Error occurred on line %i\n",recd->num);
        printf ("  In file %s\n",recd->pfdf->name);
        *rc = 99;
        return (NULL);
      }
    } else pcmn->rres = NULL;
    *rc = 3;
    return (NULL);
  }

  /* Function statement (ie, implied return) */

  else if (recd->type==11) {
    pcmn->rres = NULL;
    *rc = 3;
    return (NULL);
  }

  /* 'say' command */

  else if (recd->type==12 || recd->type==15) {
    if (recd->epos) res = gsexpr(recd->epos, pcmn);
    else {
      printf ("\n");
      *rc = 0;
      return (recd->forw);
    }
    if (res==NULL) {
      printf ("Error occurred on line %i\n",recd->num);
      printf ("  In file %s\n",recd->pfdf->name);
      *rc = 99;
      return (NULL);
    }
    if (recd->type==12) printf ("%s\n",res);
    else printf ("%s",res);
    free (res);
    return (recd->forw);
  }

  /* Pull command */

  else if (recd->type==13) {
    *rc = gsassn (recd, pcmn);
    return (recd->forw);
  }

  /* Exit command */

  else if (recd->type==14) {
    if (recd->epos) {
      res = gsexpr(recd->epos, pcmn);
      if (res==NULL) {
        printf ("  Error occurred on line %i\n",recd->num);
        printf ("  In file %s\n",recd->pfdf->name);
        *rc = 99;
        return (NULL);
      }
      gsnum (res, &ntyp, &lv, &vv);
      if (ntyp!=1) {
        printf ("Error on Exit Command:  Non Integer Argument\n");
        printf ("  Error occurred on line %i\n",recd->num);
        printf ("  In file %s\n",recd->pfdf->name);
        *rc = 99;
        return (NULL);
      }
      pcmn->rc = lv;
      free (res);
    } else {
      pcmn->rc = 0;
    }
    *rc = 4;
    return (NULL);
  }

  /* Anything else? */

  else {
    printf ("Error in gsruns:  Internal Logic Check 16\n");
    *rc = 99;
    return (NULL);
  }
  return (NULL);
}

/*  Execute a while loop */

struct gsrecd *gsrund (struct gsrecd *recd, struct gscmn *pcmn, gaint *rc) {
struct gsrecd *dorec;
gaint ret;
char *rslt;


  rslt = gsexpr(recd->epos, pcmn);
  if (rslt==NULL) {
    printf ("  Error occurred on line %i\n",recd->num);
    printf ("  In file %s\n",recd->pfdf->name);
    *rc = 99;
    return (NULL);
  }
  dorec = recd;

  ret = 0;
  while (*rslt!='0' || *(rslt+1)!='\0') {
    recd = dorec->forw;
    ret = 0;
    while (ret==0 && recd->type!=4) {
      recd = gsruns (recd, pcmn, &ret);
    }
    if (ret>1) break;
    free(rslt);
    rslt = gsexpr(dorec->epos, pcmn);
    if (rslt==NULL) {
      printf ("  Error occurred on line %i\n",recd->num);
      printf ("  In file %s\n",recd->pfdf->name);
      *rc = 99;
      return (NULL);
    }
  }
  free(rslt);
  if (ret<3) ret=0;
  *rc = ret;
  recd = dorec->refer;
  return (recd->forw);
}

/*  Execute an if block */

struct gsrecd *gsruni (struct gsrecd *recd, struct gscmn *pcmn, gaint *rc) {
gaint ret;
char *rslt;

  rslt = gsexpr(recd->epos, pcmn);
  if (rslt==NULL) {
    printf ("  Error occurred on line %i\n",recd->num);
    printf ("  In file %s\n",recd->pfdf->name);
    *rc = 99;
    return (NULL);
  }

  if (*rslt=='0' && *(rslt+1)=='\0') recd = recd->refer;
  free (rslt);

  if (recd->type != 9) {
    recd = recd->forw;
    ret = 0;
    while (ret==0 && recd->type!=8 && recd->type!=9) {
      recd = gsruns (recd, pcmn, &ret);
    }
    if (ret) {
      *rc = ret;
      return (NULL);
    }
    if (recd->type==8) recd = recd->refer;
  }
  *rc = 0;
  return (recd->forw);
}

/* Execute a statement that is to be passed to the program
   environment, and get a response back. */

gaint gsstmt (struct gsrecd *recd, struct gscmn *pcmn) {
struct gsvar *pvar;
gaint rc;
char *res, *buf, *tmp;

  res = gsexpr (recd->epos, pcmn);
  if (res==NULL) {
    printf ("  Error occurred on line %i\n",recd->num);
    printf ("  In file %s\n",recd->pfdf->name);
    return(99);
  }

  /* Execute the command */

  buf = gagsdo (res, &rc);
  free (res);

  /* We want to reflect the quit command back to the scripting
     language so we really do quit.  */

  if (rc==-1) {
    if (buf) free(buf);
    return (999);
  }

  /* Put the return code and command response into the appropriate
     variables.  We ASSUME that rc and result are variables that
     are at the start of the link list. */

  pvar = pcmn->fvar;
  tmp = (char *)malloc(6);
  if (tmp==NULL) {
    printf ("Memory allocation error\n");
    if (buf) free (buf);
    return (99);
  }
  snprintf(tmp,5,"%i",rc);
  free (pvar->strng);
  pvar->strng = tmp;

  pvar = pvar->forw;
  if (buf==NULL) {
    tmp = (char *)malloc(1);
    if (tmp==NULL) {
      printf ("Memory allocation error\n");
      return (99);
    }
    *tmp = '\0';
  } else tmp = buf;
  free (pvar->strng);
  pvar->strng = tmp;
  return(0);
}

/* Execute an assignment or pull command*/

gaint gsassn (struct gsrecd *recd, struct gscmn *pcmn) {
struct gsvar *var, *pvar=NULL;
gaint rc,i,flg;
char *res, *pos;
char varnm[16];

  /* Evaluate expression or read user input */

  if (recd->type==13) {
    res = (char *)malloc(RSIZ);
    if (res==NULL) {
      printf ("Memory allocation Error\n");
      return (99);
    }
    for (i=0; i<10; i++) *(res+i) = '\0';
    fgets(res,512,stdin);
    /* Replace newline character or return character at end of user input string with null */
    for (i=0; i<512; i++) {
/*       if (*(res+i) == '\n') *(res+i)='\0';  */
      if ((*(res+i) == '\n') || (*(res+i) == '\r')) *(res+i)='\0';   
    }
  } else {
    res = gsexpr (recd->epos, pcmn);
    if (res==NULL) {
      printf ("  Error occurred on line %i\n",recd->num);
      printf ("  In file %s\n",recd->pfdf->name);
      return (99);
    }
  }

  /* Get variable name */

  for (i=0; i<16; i++) varnm[i] = ' ';
  if (recd->type==13) pos = recd->epos;
  else pos = recd->pos;
  i=0;
  while (*pos!=' ' && *pos!='=' && i<16 && *pos!='\0') {
    varnm[i] = *pos;
    pos++; i++;
  }

  /* Resolve possible compound name. */

  rc = gsrvar (pcmn, varnm, varnm);
  if (rc) {
    printf ("  Error occurred on line %i\n",recd->num);
    printf ("  In file %s\n",recd->pfdf->name);
    return(99);
  }

  /* See if this variable name already exists */

  if (varnm[0]=='_') var = pcmn->gvar;
  else var = pcmn->fvar;
  if (var==NULL) flg = 1;
  else flg = 0;
  while (var) {
    for (i=0; i<16; i++) {
      if (varnm[i] != var->name[i]) break;
    }
    if (i==16) break;
    pvar = var;
    var = var->forw;
  }

  /* If it didn't, create it.  If it did, release old value */

  if (var==NULL) {
    var = (struct gsvar *)malloc(sizeof(struct gsvar));
    if (var==NULL) {
      printf ("Error allocating memory for variable\n");
      return (99);
    }
    if (flg) {
      if (varnm[0]=='_' ) pcmn->gvar = var;
      else pcmn->fvar = var;
    } else pvar->forw = var;
    var->forw = NULL;
    for (i=0; i<16; i++) var->name[i] = varnm[i];
  } else {
    free (var->strng);
  }

  /* Assign new value */

  var->strng = res;

  return(0);
}

/* Dump stack.  Any member of the list may be passed */

void stkdmp (struct stck *stack) {

  while (stack->pback) stack = stack->pback;
  while (stack) {
    if (stack->type==0) {
      printf ("Operand: %s\n",stack->obj.strng);
    }
    else if (stack->type==1) {
      printf ("Operator: %i \n",stack->obj.op);
    }
    else if (stack->type==2) printf ("Left paren '('\n");
    else if (stack->type==3) printf ("Right paren ')'\n");
    else printf ("Type = %i \n",stack->type);
    stack = stack->pforw;
  }
}

/* Evaluate an expression in the GrADS scripting language.
   The expression must be null terminated.  The result string
   is returned, or if an error occurs, NULL is returned.    */

char *gsexpr (char *expr, struct gscmn *pcmn) {
struct stck *curr, *snew, *sold;
char *pos;
gaint state, uflag, i, flag;

  /* First element on stack is artificial left paren.  We
     will match with artificial right paren at end of expr
     to force final expression evaluation.  */

  curr = (struct stck *)malloc(sizeof(struct stck));
  if (curr==NULL) goto err2;
  curr->pback = NULL;
  curr->pforw = NULL;
  curr->type = 2;

  /* Initial state */

  state = 1;
  uflag = 0;
  pos = expr;

  /* Loop while parsing expression.  Each loop iteration deals with
     the next element of the expression.  Each expression element
     is pushed onto the expression stack.  When a right paren is
     encountered, the stack is evaluated back to the matching left
     paren, with the intermediate result restacked.                 */

  while (1) {

    /* Allocate next link list item so its ready when we need it  */

    snew = (struct stck *)malloc(sizeof(struct stck));
    if (snew==NULL) goto err2;
    curr->pforw = snew;
    sold = curr;
    curr = snew;
    curr->pforw = NULL;
    curr->pback = sold;
    curr->type = -1;

    /* Advance past any imbedded blanks */

    while (*pos==' ') pos++;

    /* End of expr?  If so, leave loop.  */

    if (*pos=='\0') break;

    /*  The state flag determines what is expected next in the
        expression.  After an operand, we would expect an operator,
        for example -- or a ')'.  And after an operator, we would
        expect an operand, among other things.                      */

    if (state) {                     /* Expect oprnd, unary op, '(' */

      /*  Handle a left paren. */

      if (*pos=='(') {
        curr->type = 2;
        pos++;
        uflag = 0;
      }

      /* Unary minus */

      else if (*pos=='-') {
        if (uflag) goto err1;
        curr->type = 1;
        curr->obj.op = 15;
        pos++;
        uflag = 1;
      }

      /* Unary not */

      else if (*pos=='!') {
        if (uflag) goto err1;
        curr->type = 1;
        curr->obj.op = 14;
        pos++;
        uflag = 1;
      }

      /*  Handle a constant   */

      else if (*pos=='\"' || *pos=='\'' ||
               (*pos>='0' && *pos<='9') ) {
        curr->type = 0;
        curr->obj.strng = gscnst(&pos);
        if (curr->obj.strng==NULL) goto err3;
        state = 0;
        uflag = 0;
      }

      /*  Handle a variable or function call */

      else if ( (*pos>='a' && *pos<='z') ||
                (*pos>='A' && *pos<='Z') ||
                (*pos=='_')) {
        curr->type = 0;
        curr->obj.strng = gsgopd(&pos, pcmn);
        if (curr->obj.strng==NULL) goto err3;
        state = 0;
        uflag = 0;
      }

      /*  Anything else is an error.  */

      else {
        goto err1;
      }

    } else {                         /* Expect operator or ')'      */

      uflag = 0;

      /*  Handle right paren.   */

      if (*pos==')') {
        curr->type = 3;
        pos++;
        snew = gseval(curr);
        if (snew==NULL) goto err3;
        curr = snew;
      }

      /*  Handle implied concatenation - check for operand */

      else if (*pos=='\"' || *pos=='\'' || *pos=='_' ||
                (*pos>='0' && *pos<='9') ||
                (*pos>='a' && *pos<='z') ||
                (*pos>='A' && *pos<='Z') ) {
        curr->type = 1;
        curr->obj.op = 9;
        state = 1;
      }

      /*  Handle operator   */

      else {
        flag = -1;
        for (i=0; i<13; i++) {
          if (*pos != *(opchars[i])) continue;
          if (*(opchars[i]+1) && (*(pos+1)!=*(opchars[i]+1))) continue;
          flag = opvals[i];
          break;
        }
        if (flag<0) goto err1;
        curr->type = 1;
        curr->obj.op = flag;
        state = 1;
        if (i<3) pos += 2;
        else pos++;
      }
    }
  }

  /*  We get here when the end of the expression is reached.
      If the last thing stacked wasn't an operand or a closing
      paren, then an error.    */

  if (sold->type!=0 && sold->type!=3) goto err1;

  /*  Put an artificial right paren at the end of the stack
      (to match the artificial opening paren), then do a
      final evaluation of the stack.  If the result doesn't
      resolve to one operand, then unmatched parens or something */

  curr->type = 3;
/*
  stkdmp(curr);
*/
  snew = gseval(curr);
  if (snew==NULL) goto err3;
  curr = snew;
  if (curr->pback != NULL) goto err4;
  if (curr->pforw != NULL) goto err4;
/*
  stkdmp (curr);
*/

  /*  The expression has been evaluated without error.
      Free the last stack entry and return the result.     */

  pos = curr->obj.strng;
  free (curr);
  return (pos);

  /* Handle errors.  Issue error messages, free stack and
     associated memory.  */

  err1:

  printf ("Syntax Error\n");
  goto err3;

  err2:

  printf ("Memory Allocation Error\n");
  goto err3;

  err4:

  printf ("Unmatched parens\n");
  goto err3;

  err3:

  while (curr->pback) curr = curr->pback;
  while (curr!=NULL) {
    if (curr->type==0) free (curr->obj.strng);
    sold = curr;
    curr = curr->pforw;
    free (sold);
  }
  return (NULL);
}

/*  Evaluate the stack between opening and closing parentheses.
    This is done by making multiple passes at decreasing
    precedence levels, and evaluating all the operators at that
    precedence level.  When the final result is obtained, it is
    placed on the end of the stack without the parens.           */

struct stck *gseval (struct stck *curr) {
struct stck *sbeg, *srch, *stmp;
gaint i;

  /* Locate matching left paren. */

  sbeg = curr;
  while (sbeg) {
    if (sbeg->type==2) break;
    sbeg = sbeg->pback;
  }
  if (sbeg==NULL) {
    printf ("Unmatched parens\n");
    return (NULL);
  }

  /* Make a pass between the parens at each precedence level.  */

  for (i=0; i<7; i++) {
/*
    stkdmp(sbeg);
*/
    srch = sbeg;
    while (srch != curr) {
       if (srch->type==1 &&
           srch->obj.op>=opmins[i] && srch->obj.op<=opmaxs[i]) {
         srch = gsoper(srch);
         if (srch==NULL) return(NULL);
       }
      srch = srch->pforw;
    }
  }

  /* Make sure we are down to one result.  If not, we are in
     deep doodoo */

  srch = sbeg->pforw;
  srch = srch->pforw;
  if (srch != curr) {
    printf ("Logic error 8 in gseval \n");
    return (NULL);
  }

  /* Remove the parens from the linklist */

  srch = sbeg->pforw;
  srch->pforw = curr->pforw;
  srch->pback = sbeg->pback;
  stmp = sbeg->pback;
  if (stmp) stmp->pforw = srch;
  stmp = curr->pforw;
  if (stmp) stmp->pback = srch;
  free(sbeg);
  free(curr);

  return (srch);
}

/* Perform an operation.  Unstack the operator and operands,
   and stack the result in their place.  Return a pointer to
   the link list element representing the result.           */

struct stck *gsoper (struct stck *soper) {
struct stck *sop1, *sop2, *stmp;
gaint op, ntyp1, ntyp2, ntype=0, comp=0, len;
gadouble v1, v2, v;
gaint iv1, iv2, iv;
char *s1, *s2, *ch, *res, buf[25];

  /* Get pointers to the operands.  If a potentially numeric
     operation, do string to numeric conversion.             */

  op = soper->obj.op;
  sop1 = soper->pback;
  sop2 = soper->pforw;
  if (optyps[op-1]) {
    gsnum (sop2->obj.strng, &ntyp2, &iv2, &v2);
    if (op<14) gsnum (sop1->obj.strng, &ntyp1, &iv1, &v1);
    else ntyp1 = ntyp2;
    if (ntyp1==1 && ntyp2==1) ntype = 1;
    else if (ntyp1==0 || ntyp2==0) ntype = 0;
    else ntype = 2;
  }

  /* If an op that requires numbers, check to make sure we
     can do it.  */

  if (optyps[op-1]==2 && ntype == 0 ) {
    printf ("Non-numeric args to numeric operation\n");
    return (NULL);
  }

  /* Perform actual operations. */

  /* Logical or, and */

  if (op==1 || op==2) {
    s1 = sop1->obj.strng;
    s2 = sop2->obj.strng;
    res = malloc(2);
    if (res==NULL) {
      printf ("Memory allocation error\n");
      return (NULL);
    }
    *(res+1) = '\0';
    if (op==1) {
      if ( (*s1=='0' && *(s1+1)=='\0') &&
           (*s2=='0' && *(s2+1)=='\0')  ) *res = '0';
      else *res = '1';
    } else {
      if ( (*s1=='0' && *(s1+1)=='\0') ||
           (*s2=='0' && *(s2+1)=='\0') ) *res = '0';
      else *res = '1';
    }
  }

  /* Logical comparitive */

  else if (op>2 && op<9) {
    res = malloc(2);
    if (res==NULL) {
      printf ("Memory allocation error\n");
      return (NULL);
    }
    *(res+1) = '\0';

    /* Determine relationship between the ops */

    if (ntype==2) {
      if (v1<v2) comp = 1;
      else if (v1==v2) comp = 3;
      else comp = 2;
    } else if (ntype==1) {
      if (iv1<iv2) comp = 1;
      else if (iv1==iv2) comp = 3;
      else comp = 2;
    } else {
      s1 = sop1->obj.strng;
      s2 = sop2->obj.strng;
      while (*s1 && *s2) {
        if (*s1<*s2) {
          comp = 1;
          break;
        }
        if (*s1>*s2) {
          comp = 2;
          break;
        }
        s1++; s2++;
      }
      if (*s1=='\0'&&*s2=='\0') comp = 3;
      else if (*s1=='\0') comp = 1;
      else if (*s2=='\0') comp = 2;
    }

    /* Apply relationship to specific op */

    if (op==3) {
      if (comp==3) *res = '1';
      else *res = '0';
    } else if (op==4) {
      if (comp!=3) *res = '1';
      else *res = '0';
    } else if (op==5) {
      if (comp==2) *res = '1';
      else *res = '0';
    } else if (op==6) {
      if (comp==2 || comp==3) *res = '1';
      else *res = '0';
    } else if (op==7) {
      if (comp==1) *res = '1';
      else *res = '0';
    } else {
      if (comp==1 || comp==3) *res = '1';
      else *res = '0';
    }
  }

  /* String concatenation */

  else if (op==9) {
    s1 = sop1->obj.strng;
    s2 = sop2->obj.strng;
    len = strlen(s1) + strlen(s2);
    res = malloc(len+1);
    if (res==NULL) {
      printf ("Memory allocation error\n");
      return(NULL);
    }
    ch = res;
    while (*s1) {
      *ch = *s1;
      s1++; ch++;
    }
    while (*s2) {
      *ch = *s2;
      s2++; ch++;
    }
    *ch = '\0';
  }

  /*  Handle arithmetic operator */

  else if (op<14 && op>9) {
/*
    if (ntype==1) {
      if (op==10) iv = iv1+iv2;
      else if (op==11) iv = iv1-iv2;
      else if (op==12) iv = iv1*iv2;
      else {
        if (iv2==0) {
          printf ("Divide by zero\n");
          return (NULL);
        }
        iv = iv1 / iv2;
      }
      snprintf(buf,24,"%i",iv);
    } else {
      if (op==10) v = v1+v2;
      else if (op==11) v = v1-v2;
      else if (op==12) v = v1*v2;
      else {
        if (v2==0.0) {
          printf ("Divide by zero\n");
          return (NULL);
        }
        v = v1 / v2;
      }
      snprintf(buf,24,"%.15g",v);
    }
*/
      if (op==10) v = v1+v2;
      else if (op==11) v = v1-v2;
      else if (op==12) v = v1*v2;
      else {
        if (v2==0.0) {
          printf ("Divide by zero\n");
          return (NULL);
        }
        v = v1 / v2;
      }
      snprintf(buf,24,"%.15g",v);
/**/
    len = strlen(buf) + 1;
    res = malloc(len);
    if (res==NULL) {
      printf ("Memory allocation error\n");
      return (NULL);
    }
    strcpy(res,buf);
  }

  /*  Do unary not operation */

  else if (op==14) {
    res = malloc(2);
    if (res==NULL) {
      printf ("Memory allocation error\n");
      return (NULL);
    }
    *(res+1) = '\0';
    s2 = sop2->obj.strng;
    if (*s2=='\0' || (*s2=='0' && *(s2+1)=='\0')  ) *res = '1';
    else *res = '0';
  }

  /* Do unary minus operation */

  else if (op==15) {
    if (ntype==1) {
      iv = -1 * iv2;
      snprintf(buf,24,"%i",iv);
    } else {
      v = -1.0 * v2;
      snprintf(buf,24,"%.15g",v);
    }
    len = strlen(buf) + 1;
    res = malloc(len);
    if (res==NULL) {
      printf ("Memory allocation error\n");
      return (NULL);
    }
    strcpy(res,buf);
  }

  else {
    printf ("Logic error 12 in gsoper\n");
    return (NULL);
  }

  /* Rechain, Free stuff and return */

  free (sop2->obj.strng);
  if (op<14) free(sop1->obj.strng);
  sop2->obj.strng = res;
  if (op<14) {
    sop2->pback = sop1->pback;
    stmp = sop1->pback;
    stmp->pforw = sop2;
    free (sop1);
  } else {
    sop2->pback = soper->pback;
    stmp = soper->pback;
    stmp->pforw = sop2;
  }
  free (soper);
  return (sop2);
}

/* Obtain the value of an operand.  This may be either a
   variable or a function.                                */

char *gsgopd (char **ppos, struct gscmn *pcmn) {
char *pos, *res;
char name[16];
gaint i,pflag;

  pos = *ppos;
  for (i=0; i<16; i++) name[i]=' ';
  i = 0;
  pflag = 0;
  while ( (*pos>='a' && *pos<='z') ||
          (*pos>='A' && *pos<='Z') ||
          (*pos=='.') || (*pos=='_') ||
          (*pos>='0' && *pos<='9')   ) {
    if (*pos=='.') pflag = 1;
    if (i>15) {
      printf ("Variable name too long - 1st 16 chars are: ");
      for (i=0; i<16; i++) printf ("%c",name[i]);
      printf ("\n");
      return (NULL);
    }
    name[i] = *pos;
    pos++; i++;
  }
  while (*pos==' ') pos++;

  /* Handle a function call -- this is a recursive call all the
     way back to gsrunf.   */

  if (*pos=='(') {
    if (pflag) {
      printf ("Invalid function name: ");
      for (i=0; i<16; i++) printf ("%c",name[i]);
      printf ("\n");
      return (NULL);
    }
    pos = gsfunc(pos, name, pcmn);
    if (pos==NULL) return(NULL);
    *ppos = pos;
    res = pcmn->rres;
    if (res==NULL) {
      res = (char *)malloc(1);
      if (res==NULL) {
        printf ("Memory allocation error\n");
        return (NULL);
      }
      *res = '\0';
    }
    pcmn->rres = NULL;
    return(res);
  }

  *ppos = pos;
  res = gsfvar(name, pcmn);
  return (res);
}

/* Call a function.  */

char *gsfunc (char *pos, char *name, struct gscmn *pcmn) {
struct gsfnc *pfnc;
struct gsvar *avar, *nvar, *cvar=NULL;
char *astr, *res;
gaint len, rc, i, cflg, pcnt;

  avar = NULL;

  /*  Get storage for holding argument expressions */

  len = 0;
  while (*(pos+len)) len++;
  astr = (char *)malloc(len);
  if (astr==NULL) {
    printf ("Memory allocation error \n");
    return (NULL);
  }

  /*  Evaluate each argument found.  Allocate a gsvar block
      for each one, and chain them together */

  pos++;
  pcnt = 0;
  while (!(*pos==')'&&pcnt==0)) {
    cflg = 0;
    len = 0;
    while (*pos) {
      if (!cflg && (*pos==',' || (*pos==')'&&pcnt==0))) break;
      if (!cflg) {
        if (*pos=='(') pcnt++;
        if (*pos==')') pcnt--;
        if (pcnt<0) break;
      }
      if (*pos =='\'') {
        if (cflg==1) cflg = 0;
        else if (cflg==0) cflg = 1;
      } else if (*pos=='\"') {
        if (cflg==2) cflg = 0;
        else if (cflg==0) cflg = 2;
      }
      *(astr+len) = *pos;
      pos++; len++;
    }
    if (*pos=='\0') {
      printf ("Unmatched parens on function call\n");
      pos = NULL;
      goto retrn;
    }
    *(astr+len) = '\0';
    res = gsexpr(astr, pcmn);
    if (res==NULL) {
      printf ("Error occurred processing function arguments\n");
      pos = NULL;
      goto retrn;
    }
    nvar = (struct gsvar *)malloc(sizeof(struct gsvar));
    if (nvar==NULL) {
      printf ("Memory allocation error\n");
      pos = NULL;
      goto retrn;
    }
    nvar->strng = res;
    if (avar==NULL) avar = nvar;
    else cvar->forw = nvar;
    cvar = nvar;
    cvar->forw = NULL;
    if (*pos==',') pos++;
  }
  pos++;

  /*  We are all set up to invoke the function.  So now we need
      to find the function.  Look for internal functions first */

  pcmn->farg = avar;
  if (cmpwrd(name,"substr")) rc = gsfsub(pcmn);
  else if (cmpwrd(name,"subwrd")) rc = gsfwrd(pcmn);
  else if (cmpwrd(name,"sublin")) rc = gsflin(pcmn);
  else if (cmpwrd(name,"wrdpos")) rc = gsfpwd(pcmn);
  else if (cmpwrd(name,"strlen")) rc = gsfsln(pcmn);
  else if (cmpwrd(name,"valnum")) rc = gsfval(pcmn);
  else if (cmpwrd(name,"read")) rc = gsfrd(pcmn);
  else if (cmpwrd(name,"write")) rc = gsfwt(pcmn);
  else if (cmpwrd(name,"close")) rc = gsfcl(pcmn);
  else if (cmpwrd(name,"sys")) rc = gsfsys(pcmn);
  else if (cmpwrd(name,"gsfallow")) rc = gsfallw(pcmn);
  else if (cmpwrd(name,"gsfpath")) rc = gsfpath(pcmn);
  else if (cmpwrd(name,"math_log")) rc = gsfmath(pcmn,1);
  else if (cmpwrd(name,"math_log10")) rc = gsfmath(pcmn,2);
  else if (cmpwrd(name,"math_cos")) rc = gsfmath(pcmn,3);
  else if (cmpwrd(name,"math_sin")) rc = gsfmath(pcmn,4);
  else if (cmpwrd(name,"math_tan")) rc = gsfmath(pcmn,5);
  else if (cmpwrd(name,"math_atan")) rc = gsfmath(pcmn,6);
  else if (cmpwrd(name,"math_atan2")) rc = gsfmath(pcmn,7);
  else if (cmpwrd(name,"math_sqrt")) rc = gsfmath(pcmn,8);
  else if (cmpwrd(name,"math_abs")) rc = gsfmath(pcmn,9);
  else if (cmpwrd(name,"math_acosh")) rc = gsfmath(pcmn,10);
  else if (cmpwrd(name,"math_asinh")) rc = gsfmath(pcmn,11);
  else if (cmpwrd(name,"math_atanh")) rc = gsfmath(pcmn,12);
  else if (cmpwrd(name,"math_cosh")) rc = gsfmath(pcmn,13);
  else if (cmpwrd(name,"math_sinh")) rc = gsfmath(pcmn,14);
  else if (cmpwrd(name,"math_exp")) rc = gsfmath(pcmn,15);
  else if (cmpwrd(name,"math_fmod")) rc = gsfmath(pcmn,16);
  else if (cmpwrd(name,"math_pow")) rc = gsfmath(pcmn,17);
  else if (cmpwrd(name,"math_sinh")) rc = gsfmath(pcmn,18);
  else if (cmpwrd(name,"math_tanh")) rc = gsfmath(pcmn,19);
  else if (cmpwrd(name,"math_acos")) rc = gsfmath(pcmn,20);
  else if (cmpwrd(name,"math_asin")) rc = gsfmath(pcmn,21);
  else if (cmpwrd(name,"math_format")) rc = gsfmath(pcmn,22);
  else if (cmpwrd(name,"math_nint")) rc = gsfmath(pcmn,23);
  else if (cmpwrd(name,"math_int")) rc = gsfmath(pcmn,24);
  else if (cmpwrd(name,"math_mod")) rc = gsfmath(pcmn,25);
  else if (cmpwrd(name,"math_strlen")) rc = gsfmath(pcmn,26);

  /*  Not an intrinsic function.  See if it is a function
      within the file we are currently working on.  */

  else {
    pfnc = pcmn->ffnc;
    while (pfnc) {
      if (!cmpch(pfnc->name,name,16)) break;
      pfnc = pfnc->forw;
    }

    /* If not found, try to load it, assuming this is 
       currently allowed */

    if (pfnc==NULL && pcmn->gsfflg!=0) {
      rc = gsgsfrd (pcmn, 1, name);     /* Load function file */
      if (rc==0) {                      /* Now look again */
        pfnc = pcmn->ffnc;
        while (pfnc) {
          if (!cmpch(pfnc->name,name,16)) break;
          pfnc = pfnc->forw;
        }
        if (pfnc==NULL) {
          printf ("Loaded function file %s\n",pcmn->lfdef->name);
          printf ("  But... ");
        }
      } else if (rc!=9) {        /* An error occurred */
        printf ("Error while loading function: ");
        for (i=0; i<16; i++) printf("%c",name[i]);
        printf ("\n");
        pos = NULL;
        goto retrn;
      }                         /* File not found (rc==9) just */
    }                           /*   fall thru and give msg below */
    if (pfnc) {
      rc = gsrunf(pfnc->recd, pcmn);
    } else {
      printf ("Function not found: ");
      for (i=0; i<16; i++) printf("%c",name[i]);
      printf ("\n");
      pos = NULL;
      goto retrn;
    }
  }
  if (rc>0) pos = NULL;
  avar = NULL;

retrn:
  gsfrev(avar);
  free (astr);
  return(pos);
}

/* Find the value of a variable */

char *gsfvar (char *iname, struct gscmn *pcmn) {
struct gsvar *var;
char *ch, *src, name[16];
gaint len,i;

  /* Resolve possible compound name. */

  i = gsrvar (pcmn, iname, name);
  if (i) return(NULL);

  /* See if this variable name already exists */

  if (name[0]=='_') var = pcmn->gvar;
  else var = pcmn->fvar;

  while (var) {
    for (i=0; i<16; i++) {
      if (name[i] != var->name[i]) break;
    }
    if (i==16) break;
    var = var->forw;
  }

  /* If it didn't, use var name.  If it did, use current value */

  if (var==NULL) {
    len = 0;
    while (name[len]!=' ' && len<16) len++;
    src = name;
  } else {
    len = 0;
    while (*(var->strng+len)) len++;
    src = var->strng;
  }
  ch = malloc(len+1);
  if (ch==NULL) {
    printf ("Error allocating memory for variable \n");
    return (NULL);
  }
  for (i=0; i<len; i++) *(ch+i) = *(src+i);
  *(ch+len) = '\0';
  return (ch);
}

/*  Resolve compound variable name */

gaint gsrvar (struct gscmn *pcmn, char *name, char *oname) {
struct gsvar *var;
char rname[16],sname[16];
gaint len,pos,tpos,cnt,i;

  for (i=0; i<16; i++) rname[i] = ' ';
  pos = 0;
  len = 0;
  while (pos<16) {
    if (len>15) {
      if (*(name+pos)!=' ') {
        printf ("Compound variable name too long: ");
        for (i=0; i<16; i++) printf ("%c",*(name+i));
        return (1);
      }
      break;
    }
    rname[len] = *(name+pos);
    len++;
    if (*(name+pos)=='.') {                  /* Split off sub name */
      pos++;
      cnt = 0;
      tpos = pos;
      for (i=0; i<16; i++) sname[i] = ' ';
      while (*(name+tpos)!='.' && *(name+tpos)!=' ' && tpos<16) {
        sname[cnt] = *(name+tpos);
        tpos++; cnt++;
      }
      if (cnt>0) {                          /* See if it's a var  */
        if (*sname=='_') var = pcmn->gvar;
        else var = pcmn->fvar;
        while (var) {
          for (i=0; i<16; i++) {
            if (*(sname+i) != var->name[i]) break;
          }
          if (i==16) break;
          var = var->forw;
        }
        if (var!=NULL) {                    /* If so, use value   */
          cnt = 0;
          while (len<16 && *(var->strng+cnt)!='\0') {
            rname[len] = *(var->strng+cnt);
            cnt++; len++;
          }
          if (len==16 && *(var->strng+cnt)=='\0') {
            printf ("Compound variable name too long: ");
            for (i=0; i<16; i++) printf ("%c",*(name+i));
            return (1);
          }
          pos = tpos;                       /* Advance pointer    */
        }
      }
    } else pos++;
  }

  for (i=0; i<16; i++) *(oname+i) = rname[i];
/*
  for (i=0; i<16; i++) printf ("%c",*(oname+i));
  printf ("\n");
*/
  return (0);
}


/*  Retreive a constant */

char *gscnst (char **ppos) {
char *pos, *ch, *cpos, delim;
gaint len, i, dflg, eflg;

  pos = *ppos;

  /* Handle integer constant */

  if (*pos>='0'&&*pos<='9') {
    len = 0;
    dflg = 1;
    while ( (*pos>='0'&&*pos<='9') ||
            (dflg && *pos=='.')  ) {
      if (*pos=='.') dflg = 0;
      pos++;
      len++;
    }
    eflg = 0;
    if ( (*pos=='e' || *pos=='E') )  {
       if ( *(pos+1)>='0' && *(pos+1)<='9' ) eflg = 1;
       else if ( *(pos+1)=='-' || *(pos+1)=='+' ) {
         if ( *(pos+2)>='0' && *(pos+2)<='9' ) eflg = 2;
       }
    }
    if (eflg) {
      pos += eflg;    /* Skip past 'e' and exponent sign */
      len += eflg;
      while ( *pos>='0'&&*pos<='9' ) { 
        pos++;
        len++;
      }
    }
      
    ch = malloc(len+1);
    if (ch==NULL) {
      printf ("Memory allocation error \n");
      return (NULL);
    }
    pos = *ppos;
    for (i=0; i<len; i++) *(ch+i) = *(pos+i);
    *(ch+len) = '\0';
    *ppos = pos+len;
    return (ch);
  }

  /* Handle string constant */

  delim = *pos;
  len = 0;
  pos++;
  while (*pos) {
    if (*pos==delim && *(pos+1)==delim) {
      len++;
      pos += 2;
      continue;
    }
    if (*pos==delim) break;
    len++;
    pos++;
  }
  if (*pos=='\0') {
    printf ("Non-terminated constant\n");
    return (NULL);
  }
  ch = malloc(len+1);
  if (ch==NULL) {
    printf ("Memory allocation error \n");
    return (NULL);
  }

  pos = *ppos;
  cpos = ch;
  pos++;
  while (1) {
    if (*pos==delim && *(pos+1)==delim) {
      *cpos = *pos;
      cpos++;
      pos += 2;
      continue;
    }
    if (*pos==delim) break;
    *cpos = *pos;
    pos++; cpos++;
  }
  *cpos = '\0';
  *ppos = pos+1;
  return (ch);
}

/* Determine if an operand is a numeric.  Numerics must not
   have any leading or trailing blanks, and must be
   either integer or floating values.  */

void gsnum (char *strng, gaint *type, gaint *ival, gadouble *val) {
char *ch;
gaint dflg,eflg,len;

  ch = strng;
  len = 0;

  dflg = 0;  /* we found a decimal point */
  eflg = 0;  /* we found an exponent */

  if (*ch=='\0') {
    *type = 0;
    return;
  }
  if (*ch<'0' || *ch>'9') { 
    if (*ch=='+' || *ch=='-') {
      ch++; len++;
      if (*ch=='.') {
        dflg = 1;
        ch++; len++;
      }
    } else if (*ch=='.') {
      dflg = 1;
      ch++; len++;
    } else {
      *type = 0;
      return;
    }
  }
  if (*ch<'0' || *ch>'9') {  /* should be a number at this point */ 
    *type = 0;
    return;
  }
  while (*ch) {
    if (*ch<'0' || *ch>'9') {
      if (*ch=='.') {
        if (dflg) break;
        dflg = 1;
      } else break;
    }
    ch++;
    len++;
  }

  if (*ch=='E'||*ch=='e') {
    eflg = 1;
    ch++; len++;
    if (*ch=='+' || *ch=='-') {
      ch++; len++;
    }
    if (*ch<'0' || *ch>'9') {
      *type = 0;
      return;
    }
    while (*ch>='0' && *ch<='9') {
      ch++; len++;
    }
  }
  if (*ch) {
    *type = 0;
    return;
  }
  if (!dflg&&!eflg&&len<10) {
    *ival = atol(strng);
    *val = (gadouble)(*ival);
    *type = 1;
  } else {
    *val = atof(strng);
    *type = 2;
  }
  return;
}

/*  Intrinsic functions.  */

/* Sys function. Expects one arg, the command to execute.
   Uses popen to execute the command and read the stdout from it.  
   Note that popen has limitations. */

gaint gsfsys (struct gscmn *pcmn) {
FILE *pipe;
struct gsvar *pvar;
char *cmd,*res,*buf;
gadouble v;
gaint ret,len,siz,incr,pos,ntype;

  pcmn->rres = NULL;

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error from script function: sys:  1st argument missing\n");
    ret = 1;
    goto retrn;
  }
  cmd = pvar->strng;

  incr = 1000;
  pvar = pvar->forw;
  if (pvar!=NULL) {
    gsnum (pvar->strng, &ntype, &ret, &v);
    if (ntype!=1 || ret<5000) {
      printf ("Warning from script function: sys: 2nd arg invalid, ignored\n");
    } else incr = ret;
  }

  /* Call popen to execute the command. */

  pipe = popen(cmd, "r");
  if (pipe==NULL) {
    printf ("Error from script function: sys:  popen error\n");
    ret = 1;
    goto retrn;
  }

  /* Allocate storage for the result and read the result */

  siz = incr;
  res = NULL;
  pos = 0;
  while (1) {
    buf = (char *)realloc(res,siz+10);
    if (buf==NULL) {
      printf ("Error from script function: sys:  Memory allocation error\n");
      printf ("Error from script function: sys:  Attempted size %i\n",siz);
      free (res);
      pclose(pipe);
      ret = 1;
      goto retrn;
    }
    res = buf;
    len = fread(res+pos,sizeof(char),incr,pipe); 
    pos += len;
    if (len<incr) break;
    if (siz>10000 && incr<5000) incr=5000;
    if (siz>100000 && incr<10000) incr=10000;
    if (siz>1000000 && incr<100000) incr=100000;
    siz += incr;
  }
  *(res+pos) = '\0';
  pclose(pipe);

  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/* Substring function.  Expects three args:  string, start, length */

gaint gsfsub (struct gscmn *pcmn) {
struct gsvar *pvar;
char *ch, *res;
gaint ret, ntype, strt, len, i;
gaint lstrt,llen;
gadouble v;

  pcmn->rres = NULL;

  /* Attempt to convert 2nd and thrd args to integer */

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in substr:  1st argument missing\n");
    ret = 1;
    goto retrn;
  }
  pvar = pvar->forw;
  if (pvar==NULL) {
    printf ("Error in substr:  2nd argument missing\n");
    ret = 1;
    goto retrn;
  }
  gsnum (pvar->strng, &ntype, &lstrt, &v);
  strt = lstrt;
  if (ntype!=1 || strt<1) {
    printf ("Error in substr:  2nd argument invalid.\n");
    ret = 1;
    goto retrn;
  }
  pvar = pvar->forw;
  if (pvar==NULL) {
    printf ("Error in substr:  3rd argument missing\n");
    ret = 1;
    goto retrn;
  }
  gsnum (pvar->strng, &ntype, &llen, &v);
  len = llen;
  if (ntype!=1 || len<1) {
    printf ("Error in substr:  3rd argument invalid.\n");
    ret = 1;
    goto retrn;
  }

  /* Allocate storage for the result */

  res = (char *)malloc(len+1);
  if (res==NULL) {
    printf ("Error:  Storage allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Move the desired substring.  NULL return is possible. */

  pvar = pcmn->farg;
  i = 1;
  ch = pvar->strng;
  while (*ch && i<strt) {ch++; i++;}  /* Don't start past end of string */
  i = 0;
  while (*ch && i<len) {
    *(res+i) = *ch;
    ch++;
    i++;
  }
  *(res+i) = '\0';

  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/*  Routine to get specified word in a string */

gaint gsfwrd (struct gscmn *pcmn) {
struct gsvar *pvar;
char *ch, *res;
gaint ret, ntype, wnum, i, len;
gaint lwnum;
gadouble v;

  pcmn->rres = NULL;

  /* Attempt to convert 2nd arg to integer. */

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in subwrd:  1st argument missing\n");
    ret = 1;
    goto retrn;
  }
  pvar = pvar->forw;
  if (pvar==NULL) {
    printf ("Error in subwrd:  2nd argument missing\n");
    ret = 1;
    goto retrn;
  }
  gsnum (pvar->strng, &ntype, &lwnum, &v);
  wnum = lwnum;
  if (ntype!=1 || wnum<1) {
    printf ("Error in subwrd:  2nd argument invalid.\n");
    ret = 1;
    goto retrn;
  }

  /* Find the desired word in the string */

  pvar = pcmn->farg;
  ch = pvar->strng;
  i = 0;
  while (*ch) {
    if (*ch==' '||*ch=='\n'||*ch=='\t'||i==0) {
      while (*ch==' '||*ch=='\n'||*ch=='\t') ch++;
      if (*ch) i++;
      if (i==wnum) break;
    } else ch++;
  }


  /* Get length of returned word. */

  len = 0;
  while (*(ch+len)!='\0' && *(ch+len)!=' '
         && *(ch+len)!='\t' && *(ch+len)!='\n') len++;

  /* Allocate storage for the result */

  res = (char *)malloc(len+1);
  if (res==NULL) {
    printf ("Error:  Storage allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Deliver the result and return */

  for (i=0; i<len; i++) *(res+i) = *(ch+i);
  *(res+len) = '\0';

  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/*  Routine to get specified line in a string */

gaint gsflin (struct gscmn *pcmn) {
struct gsvar *pvar;
char *ch, *res;
gaint ret, ntype, lnum, i, len;
gaint llnum;
gadouble v;

  pcmn->rres = NULL;

  /* Attempt to convert 2nd arg to integer. */

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in sublin:  1st argument missing\n");
    ret = 1;
    goto retrn;
  }
  pvar = pvar->forw;
  if (pvar==NULL) {
    printf ("Error in sublin:  2nd argument missing\n");
    ret = 1;
    goto retrn;
  }
  gsnum (pvar->strng, &ntype, &llnum, &v);
  lnum = llnum;
  if (ntype!=1 || lnum<1) {
    printf ("Error in sublin:  2nd argument invalid.\n");
    ret = 1;
    goto retrn;
  }

  /* Find the desired line in the string */

  pvar = pcmn->farg;
  ch = pvar->strng;
  i = 1;
  while (*ch && i<lnum) {
    if (*ch=='\n') i++;
    ch++;
  }

  /* Get length of returned line. */

  len = 0;
  while (*(ch+len)!='\0' && *(ch+len)!='\n') len++;

  /* Allocate storage for the result */

  res = (char *)malloc(len+1);
  if (res==NULL) {
    printf ("Error:  Storage allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Deliver the result and return */

  for (i=0; i<len; i++) *(res+i) = *(ch+i);
  *(res+len) = '\0';

  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/* Read function.  Expects one arg: the file name.
   Returnes a two line result -- an rc, and the record read */

gaint gsfrd (struct gscmn *pcmn) {
FILE *ifile;
struct gsvar *pvar;
struct gsiob *iob,*iobo;
char *res,*name,rc,*ch;
gaint ret,n;

  pcmn->rres = NULL;
  res = (char *)malloc(RSIZ);
  if (res==NULL) {
    printf ("Memory allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Get file name */

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in read:  File name missing\n");
    ret = 1;
    goto retrn;
  }
  name = pvar->strng;
  if (*name=='\0') {
    printf ("Error in read:  NULL File Name\n");
    ret = 1;
    goto retrn;
  }

  /* Check to see if the file is already open */

  iob = pcmn->iob;
  iobo = iob;
  while (iob) {
    if (!strcmp(name,iob->name)) break;
    iobo = iob;
    iob = iob->forw;
  }

  /* If it was not open, open it and chain a new iob */

  if (iob==NULL) {
    ifile = fopen(name,"r");
    if (ifile==NULL) {
      rc = '1';
      goto rslt;
    }
    iob = (struct gsiob *)malloc(sizeof(struct gsiob));
    if (iob==NULL) {
      printf ("Memory allocation error\n");
      ret = 1;
      goto retrn;
    }
    if (pcmn->iob==NULL) pcmn->iob = iob;
    else iobo->forw = iob;
    iob->forw = NULL;
    iob->file = ifile;
    iob->name = name;
    iob->flag = 1;
    pvar->strng = NULL;
  } else {
    if (iob->flag!=1) {
      rc = '8';
      printf ("Error in read:  attempt to read a file open for write\n");
      printf ("  File name = %s\n",iob->name);
      goto rslt;
    }
    ifile = iob->file;
  }

  /* Read the next record into the buffer area */

  ch = fgets(res+2, RSIZ-3, ifile);
  if (ch==NULL) {
    if (feof(ifile)) rc = '2';
    else rc = '9';
    goto rslt;
  }
  rc = '0';
  /* Remove cr for PC/cygwin version */
  ch = res+2;
  n=strlen(ch);
  if ( n > 1 ) {
    if ( (gaint)ch[n-2] == 13 ) {
       ch[n-2] = ch[n-1];
       ch[n-1] = '\0';
     }
   }
  /* Complete return arg list */

rslt:

  *res = rc;
  *(res+1) = '\n';
  ret = 0;
  pcmn->rres = res;
  res = NULL;

  /* Release arg storage and return */

retrn:

  if (res) free(res);
  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/* Write function.  Expects two or three args:  file name,
   output record, and optional append flag.  Returns a return code. */

gaint gsfwt (struct gscmn *pcmn) {
FILE *ofile;
struct gsvar *pvar, *pvars;
struct gsiob *iob, *iobo;
char *res, *name, rc, *orec;
gaint ret,appflg,len;

  pcmn->rres = NULL;
  res = (char *)malloc(2);
  if (res==NULL) {
    printf ("Memory allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Get file name */

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in write:  File name missing\n");
    ret = 1;
    goto retrn;
  }
  name = pvar->strng;
  if (*name=='\0') {
    printf ("Error in write:  NULL File Name\n");
    ret = 1;
    goto retrn;
  }
  pvars = pvar;

  /* Get output record */

  pvar = pvar->forw;
  if (pvar==NULL) {
    printf ("Error in write:  Output Record arg is missing\n");
    ret = 1;
    goto retrn;
  }
  orec = pvar->strng;

  /* Check for append flag */

  pvar = pvar->forw;
  if (pvar==NULL) appflg = 0;
  else appflg = 1;

  /* Check to see if the file is already open */

  iob = pcmn->iob;
  iobo = iob;
  while (iob) {
    if (!strcmp(name,iob->name)) break;
    iobo = iob;
    iob = iob->forw;
  }

  /* If it was not open, open it and chain a new iob */

  if (iob==NULL) {
    if (appflg) ofile = fopen(name,"a+");
    else ofile = fopen(name,"w");
    if (ofile==NULL) {
      rc = '1';
      goto rslt;
    }
    iob = (struct gsiob *)malloc(sizeof(struct gsiob));
    if (iob==NULL) {
      printf ("Memory allocation error\n");
      ret = 1;
      goto retrn;
    }
    if (pcmn->iob==NULL) pcmn->iob = iob;
    else iobo->forw = iob;
    iob->forw = NULL;
    iob->file = ofile;
    iob->name = name;
    iob->flag = 2;
    pvars->strng = NULL;
  } else {
    if (iob->flag!=2) {
      rc = '8';
      printf ("Error in write: attempt to write a file open for read\n");
      printf ("  File name = %s\n",iob->name);
      goto rslt;
    }
    ofile = iob->file;
  }

  /* Write the next record */

  len = 0;
  while (*(orec+len)) len++;
  *(orec+len) = '\n';
  len++;
  fwrite (orec,1,len,ofile);
  rc = '0';

  /* Complete return arg list */

rslt:

  *res = rc;
  *(res+1) = '\n';
  ret = 0;
  pcmn->rres = res;
  res = NULL;

  /* Release arg storage and return */

retrn:

  if (res) free(res);
  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/* Close function.  Expects one arg:  file name.
   Returns a return code:  0, normal, 1, file not open */

gaint gsfcl (struct gscmn *pcmn) {
struct gsvar *pvar;
struct gsiob *iob, *iobo;
char *name, *res, rc;
gaint ret;

  pcmn->rres = NULL;
  res = (char *)malloc(2);
  if (res==NULL) {
    printf ("Memory allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Get file name */

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in close:  File name missing\n");
    ret = 1;
    goto retrn;
  }
  name = pvar->strng;
  if (*name=='\0') {
    printf ("Error in close:  NULL File Name\n");
    ret = 1;
    goto retrn;
  }

  /* Check to see if the file is already open */

  iob = pcmn->iob;
  iobo = iob;
  while (iob) {
    if (!strcmp(name,iob->name)) break;
    iobo = iob;
    iob = iob->forw;
  }

  /* If it was not open, print message and return */

  if (iob==NULL) {
    rc = '1';
    printf ("Error in close:  file not open\n");
    printf ("  File name = %s\n",name);
  } else {
    fclose (iob->file);
    if (iob==pcmn->iob) pcmn->iob = iob->forw;
    else iobo->forw = iob->forw;
    free (iob);
    rc = '0';
  }

  /* Complete return arg list */
  *res = rc;
  *(res+1) = '\0';
  ret = 0;
  pcmn->rres = res;
  res = NULL;

  /* Release arg storage and return */

retrn:

  if (res) free(res);
  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/*  Routine to return position of specified word in a string */

gaint gsfpwd (struct gscmn *pcmn) {
struct gsvar *pvar;
char *ch, *res;
gaint ret, ntype, wnum, i, pos;
gaint lwnum;
gadouble v;

  pcmn->rres = NULL;

  /* Attempt to convert 2nd arg to integer. */

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in wrdpos:  1st argument missing\n");
    ret = 1;
    goto retrn;
  }
  pvar = pvar->forw;
  if (pvar==NULL) {
    printf ("Error in wrdpos:  2nd argument missing\n");
    ret = 1;
    goto retrn;
  }
  gsnum (pvar->strng, &ntype, &lwnum, &v);
  wnum = lwnum;
  if (ntype!=1 || wnum<1) {
    printf ("Error in wrdpos:  2nd argument invalid.\n");
    ret = 1;
    goto retrn;
  }

  /* Find the desired word in the string */

  pvar = pcmn->farg;
  ch = pvar->strng;
  i = 0;
  while (*ch) {
    if (*ch==' '||*ch=='\n'||*ch=='\t'||i==0) {
      while (*ch==' '||*ch=='\n'||*ch=='\t') ch++;
      if (*ch) i++;
      if (i==wnum) break;
    } else ch++;
  }

  /* Calculcate position of the desired word */

  if (*ch=='\0') pos = 0;
  else pos = 1 + ch - pvar->strng;

  /* Allocate storage for the result */

  res = (char *)malloc(12);
  if (res==NULL) {
    printf ("Error:  Storage allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Deliver the result and return */

  snprintf(res,11,"%i",pos);
  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/*  Routine to return the length of a string */

gaint gsfsln (struct gscmn *pcmn) {
struct gsvar *pvar;
char *res;
gaint ret, len;

  pcmn->rres = NULL;

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in strlen:  Argument missing\n");
    ret = 1;
    goto retrn;
  }
 
  len = 0;
  while (*(pvar->strng+len)) {
    len++;
    if (len==9999999) break;
  }

  /* Allocate storage for the result */

  res = (char *)malloc(12);
  if (res==NULL) {
    printf ("Error:  Storage allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Deliver the result and return */

  snprintf(res,11,"%i",len);
  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/*  Routine to check if a string is a valid numeric */

gaint gsfval (struct gscmn *pcmn) {
struct gsvar *pvar;
char *res;
gaint ret, ntype;
gaint lwnum;
gadouble v;

  pcmn->rres = NULL;

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in valnum:  Argument missing\n");
    ret = 1;
    goto retrn;
  }

  gsnum (pvar->strng, &ntype, &lwnum, &v);

  /* Allocate storage for the result */

  res = (char *)malloc(12);
  if (res==NULL) {
    printf ("Error:  Storage allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Deliver the result and return */

  snprintf(res,11,"%i",ntype);
  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/*  Routine to control gsf loading.  */

gaint gsfallw (struct gscmn *pcmn) {
struct gsvar *pvar;
char *res;
gaint ret, i;

  pcmn->rres = NULL;

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in gsfallow:  Argument missing\n");
    ret = 1;
    goto retrn;
  }

  i = 999;
  if (cmpwrd(pvar->strng, "on")) i = 1;
  if (cmpwrd(pvar->strng, "On")) i = 1;
  if (cmpwrd(pvar->strng, "ON")) i = 1;
  if (cmpwrd(pvar->strng, "off")) i = 0;
  if (cmpwrd(pvar->strng, "Off")) i = 0;
  if (cmpwrd(pvar->strng, "OFF")) i = 0;
  if (i<900) pcmn->gsfflg = i;

  /* Allocate storage for the result */

  res = (char *)malloc(12);
  if (res==NULL) {
    printf ("Error:  Storage allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Deliver the result and return */

  snprintf(res,11,"%i",i);
  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/*  Routine to set gsf private path  */

gaint gsfpath (struct gscmn *pcmn) {
struct gsvar *pvar;
char *res;
gaint ret, i, j;

  pcmn->rres = NULL;

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in gsfpath:  Argument missing\n");
    ret = 1;
    goto retrn;
  }

  /* Copy the path to the gscmn area */

  i = 0;
  while (*(pvar->strng+i)) i++;
  if (pcmn->ppath) free (pcmn->ppath);
  pcmn->ppath = (char *)malloc(i+1);
  if (pcmn->ppath==NULL) {
    printf ("Error in gsfpath:  Memory Allocation\n");
    ret = 1;
    goto retrn;
  }
  j = 0;
  while (j<i) { *(pcmn->ppath+j) = *(pvar->strng+j); j++; }
  *(pcmn->ppath+i) = '\0';

  /* Allocate storage for the result */

  res = (char *)malloc(3);
  if (res==NULL) {
    printf ("Error:  Storage allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Deliver the result and return */

  *res = '1';
  *(res+1) = '\0';
  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/*  Routine to do libmf math */

gaint gsfmath (struct gscmn *pcmn, gaint mathflg) {
struct gsvar *pvar;
char *res, buf[25],vformat[15];
char *mathmsg1 = "log";
char *mathmsg2 = "log10";
char *mathmsg3 = "cos";
char *mathmsg4 = "sin";
char *mathmsg5 = "tan";
char *mathmsg6 = "atan";
char *mathmsg7 = "atan2";
char *mathmsg8 = "sqrt";
char *mathmsg9 = "abs";
char *mathmsg10 = "acosh";
char *mathmsg11 = "asinh";
char *mathmsg12 = "atanh";
char *mathmsg13 = "cosh";
char *mathmsg14 = "sinh";
char *mathmsg15 = "exp";
char *mathmsg16 = "fmod";
char *mathmsg17 = "pow";
char *mathmsg18 = "sinh";
char *mathmsg19 = "tanh";
char *mathmsg20 = "acos";
char *mathmsg21 = "asin";
char *mathmsg22 = "format";
char *mathmsg23 = "nint";
char *mathmsg24 = "int";
char *mathmsg25 = "mod";
char *mathmsg26 = "strlen";
char *mathmsg=NULL;
gaint ret, ntype, i, len;
gaint lwnum;
gadouble v,v2;

  pcmn->rres = NULL;

  if (mathflg==1) mathmsg = mathmsg1;
  if (mathflg==2) mathmsg = mathmsg2;
  if (mathflg==3) mathmsg = mathmsg3;
  if (mathflg==4) mathmsg = mathmsg4;
  if (mathflg==5) mathmsg = mathmsg5;
  if (mathflg==6) mathmsg = mathmsg6;
  if (mathflg==7) mathmsg = mathmsg7;
  if (mathflg==8) mathmsg = mathmsg8;
  if (mathflg==9) mathmsg = mathmsg9;
  if (mathflg==10) mathmsg = mathmsg10;
  if (mathflg==11) mathmsg = mathmsg11;
  if (mathflg==12) mathmsg = mathmsg12;
  if (mathflg==13) mathmsg = mathmsg13;
  if (mathflg==14) mathmsg = mathmsg14;
  if (mathflg==15) mathmsg = mathmsg15;
  if (mathflg==16) mathmsg = mathmsg16;
  if (mathflg==17) mathmsg = mathmsg17;
  if (mathflg==18) mathmsg = mathmsg18;
  if (mathflg==19) mathmsg = mathmsg19;
  if (mathflg==20) mathmsg = mathmsg20;
  if (mathflg==21) mathmsg = mathmsg21;
  if (mathflg==22) mathmsg = mathmsg22;
  if (mathflg==23) mathmsg = mathmsg23;
  if (mathflg==24) mathmsg = mathmsg24;
  if (mathflg==25) mathmsg = mathmsg25;
  if (mathflg==26) mathmsg = mathmsg26;

  pvar = pcmn->farg;
  if (pvar==NULL) {
    printf ("Error in math_%s:  Argument missing\n",mathmsg);
    ret = 1;
    goto retrn;
  }

  if( !(mathflg == 22 || mathflg == 26) ) {
    gsnum (pvar->strng, &ntype, &lwnum, &v);

    if (ntype==0) {
      printf ("Error in math_%s:  Argument not a valid numeric\n",mathmsg);
      ret = 1;
      goto retrn;
    }

  } else {
    if(mathflg == 22) {
      if(strlen(pvar->strng) < 15) {
	strcpy(vformat,pvar->strng);
      } else {
	printf ("Error in math_%s:  argument: %s  too long < 15\n",mathmsg,pvar->strng);
	ret = 1;
	goto retrn;
      }
    }
  }
  if (v<=0.0 && (mathflg == 1 || mathflg == 2) ) {  
    printf ("Error in math_%s:  Argument less than or equal to zero\n",mathmsg);
    ret = 1;
    goto retrn;
  }
 
  if(mathflg == 16) {
    pvar = pvar->forw;
    if (pvar==NULL) {
      printf ("Error in fmod:  2rd argument missing\n");
      ret = 1;
      goto retrn;
    }

    gsnum (pvar->strng, &ntype, &lwnum, &v2);
    
    if (ntype == 0) {
      printf ("Error in fmod:  2rd argument invalid.\n");
      ret = 1;
      goto retrn;
    }
  }

  if(mathflg == 17) {
    pvar = pvar->forw;
    if (pvar==NULL) {
      printf ("Error in pow:  2rd argument missing\n");
      ret = 1;
      goto retrn;
    }

    gsnum (pvar->strng, &ntype, &lwnum, &v2);
    
    if (ntype == 0) {
      printf ("Error in pow:  2rd argument invalid.\n");
      ret = 1;
      goto retrn;
    }
  }

  if(mathflg == 7) {
    pvar = pvar->forw;
    if (pvar==NULL) {
      printf ("Error in atan2:  2rd argument missing\n");
      ret = 1;
      goto retrn;
    }

    gsnum (pvar->strng, &ntype, &lwnum, &v2);
    
    if (ntype == 0) {
      printf ("Error in atan2:  2rd argument invalid.\n");
      ret = 1;
      goto retrn;
    }
  }

  if(mathflg == 25) {
    pvar = pvar->forw;
    if (pvar==NULL) {
      printf ("Error in mod:  2rd argument missing\n");
      ret = 1;
      goto retrn;
    }

    gsnum (pvar->strng, &ntype, &lwnum, &v2);
    
    if (ntype == 0) {
      printf ("Error in mod:  2rd argument invalid.\n");
      ret = 1;
      goto retrn;
    }
  }

  if(mathflg == 22) {
    pvar = pvar->forw;
    if (pvar==NULL) {
      printf ("Error in format:  2rd argument missing\n");
      ret = 1;
      goto retrn;
    }

    gsnum (pvar->strng, &ntype, &lwnum, &v);
    
    if (ntype == 0) {
      printf ("Error in format:  2rd argument invalid.\n");
      ret = 1;
      goto retrn;
    }
  }



  /* Get result */

  if (mathflg==1) v = log(v);
  if (mathflg==2) v = log10(v);
  if (mathflg==3) v = cos(v);
  if (mathflg==4) v = sin(v);
  if (mathflg==5) v = tan(v);
  if (mathflg==6) v = atan(v);
  if (mathflg==7) v = atan2(v,v2);
  if (mathflg==8) v = sqrt(v);
  if (mathflg==9) v = fabs(v);
  if (mathflg==10) v = fabs(v);
  if (mathflg==11) v = asinh(v);
  if (mathflg==12) v = atanh(v);
  if (mathflg==13) v = cosh(v);
  if (mathflg==14) v = sinh(v);
  if (mathflg==15) v = exp(v);
  if (mathflg==16) v = fmod(v,v2);
  if (mathflg==17) v = pow(v,v2);
  if (mathflg==18) v = sinh(v);
  if (mathflg==19) v = tanh(v);
  if (mathflg==20) v = acos(v);
  if (mathflg==21) v = asin(v);


  if(mathflg == 23) {
    v=floor(v+0.5);
  } else if(mathflg == 24) {
    v=floor(v);
  } else if(mathflg == 25) {
    v=floor(fmod(v,v2));
  } else if(mathflg == 26) {
    v=strlen(pvar->strng);
  }

  if(mathflg==22) {
    snprintf(buf,24,vformat,v);
  } else {
    snprintf(buf,24,"%.15g",v);
  }



  len = 0;
  while (buf[len]) len++;
  len++;

  /* Allocate storage for the result */

  res = (char *)malloc(len);
  if (res==NULL) {
    printf ("Error:  Storage allocation error\n");
    ret = 1;
    goto retrn;
  }

  /* Deliver the result and return */

  for (i=0; i<len; i++) *(res+i) = buf[i];
  ret = 0;
  pcmn->rres = res;

  /* Release arg storage and return */

retrn:

  gsfrev (pcmn->farg);
  pcmn->farg = NULL;
  return (ret);
}

/* Following functions are related to reading the script file
   into memory based on the file name and path specification */

/* Open the main script; search the path if needed 

     Rules:  When working with the name of the primary 
             script, 1st try to open the name provided, as is.
             If this fails, append .gs (if not there already)
             and try again.  If this fails, and the file 
             name provided does not start with a /, then 
             we try the directories in the GASCRP envvar, 
             both with the primary name and the .gs extension.

     Code originally by M.Fiorino   */

FILE *gsonam (struct gscmn *pcmn, struct gsfdef *pfdf) {
FILE *ifile;
char *uname,*xname,*dname,*lname,*oname;
char *sdir;
gaint len;

  uname = NULL;  /* user provided name */
  xname = NULL;  /* user name plus extension */
  dname = NULL;  /* path dir name */
  lname = NULL;  /* path plus uname or xname */
  oname = NULL;  /* name of file that gets opened */
  
  /* First try to open by using the name provided. */
             
  uname = gsstad(pcmn->fname,"\0");
  if (uname==NULL) return(NULL);
  ifile = fopen(uname,"rb");

  /* If that failed, then try adding a .gs extension,
     but only if one is not already there */ 

  if (ifile==NULL) {
    xname = NULL;
    len = 0;
    while (*(uname+len)) len++;
    if (*(uname+len-1)!='s' || *(uname+len-2)!='g' || 
        *(uname+len-3)!='.' ) { 
      xname = gsstad(uname,".gs");
      if (xname==NULL) return(NULL);
      ifile = fopen(xname,"rb");
      if (ifile!=NULL) {
        oname = xname;
        xname = NULL;
      }
    }

    /* If that didn't work, search in the GASCRP path --
       the path contains blank-delimited directory names */

    if (ifile == NULL && *(uname)!='/' ) {

      sdir = getenv("GASCRP");

      while (sdir!=NULL) {
        while (gsdelim(*sdir)) sdir++;
        if (*sdir=='\0') break;
        dname = gsstcp(sdir);
        if (dname==NULL) return(NULL);
        len = 0;              /* add slash to dir name if needed */
        while (*(dname+len)) len++;
        if (*(dname+len-1)!='/') {
          lname = gsstad(dname,"/");
          if (lname==NULL) return(NULL);
          free(dname);
          dname = lname;
        }
        lname = gsstad(dname,uname);    /* try uname plus dirname */
        if (lname==NULL) return(NULL);
        ifile = fopen(lname,"rb");
        if (ifile!=NULL) {
          oname = lname;
          lname = NULL;
          break;
        } else {                        /* try xname plus dirname */
          free (lname);
          lname = NULL;
          if (xname) {
            lname = gsstad(dname,xname);
            if (lname==NULL) return(NULL);
            ifile = fopen(lname,"rb");
            if (ifile!=NULL) {
              oname = lname;
              lname = NULL;
              break;
            } else { 
              free(lname); 
              lname = NULL;
            }
          }
        }
        while (*sdir!=' ' && *sdir!='\0') sdir++;   /* Advance */
        free(dname);
        dname = NULL;
      }
    } 
  } else {
    oname = uname; 
    uname = NULL;
  }

  if (uname) free(uname);  /* Hopefully set  */
  if (xname) free(xname);  /*   to null        */
  if (dname) free(dname);  /*     if assigned    */
  if (lname) free(lname);  /*       to oname       */

  /* If we opened a file, figure out the prefix */

  if (ifile) {
    pfdf->name = oname;
    xname = gsstad(oname,"\0");
    len = 0;
    while (*(xname+len)) len++;
    while (len>0 && *(xname+len)!='/') len--;
    if (len>0) *(xname+len+1) = '\0';
    else *(xname) = '\0';
    pcmn->fprefix = xname;
  }
  return (ifile);
}
 
/*  When working with a .gsf, the function name
    is appended with .gsf.  Then we first try the
    same directory that the main script was found
    in.  If that fails, then we try the search path
    in GASCRP. */

FILE *gsogsf (struct gscmn *pcmn, struct gsfdef *pfdf, char *pfnc) {
FILE *ifile;
char *fname,*tname,*dname,*sdir;
gaint len,i;
char nname[20];

  /* Function name is not null terminated -- make a copy that is */

  for (i=0; i<16; i++) nname[i] = *(pfnc+i);
  nname[16] = ' ';
  i = 0;
  while (nname[i] != ' ') i++;
  nname[i] = '\0';

  fname = gsstad(nname,".gsf");
  if (fname == NULL) return (NULL);

  /* First try the prefix directory */

  tname = gsstad(pcmn->fprefix,fname);
  if (tname == NULL) return (NULL);
  ifile = fopen(tname,"rb");
  if (ifile) {
    free (fname);
    pfdf->name = tname;
    return (ifile); 
  }
  free (tname);

  /* Next try the private path.  The file names are constructed
     as the prefix plus the private path plus the function name
     plus the .gsf */

  sdir = pcmn->ppath;

  while (sdir!=NULL) {
    while (gsdelim(*sdir)) sdir++;
    if (*sdir=='\0') break;
    dname = gsstcp(sdir);
    if (dname==NULL) return(NULL);
    len = 0;              /* add slash to dir name if needed */
    while (*(dname+len)) len++;
    if (*(dname+len-1)!='/') {
      tname = gsstad(dname,"/");
      if (tname==NULL) return(NULL);
      free(dname);
      dname = tname;
    }
    tname = gsstad(dname,fname);  
    free (dname);
    dname = gsstad(pcmn->fprefix,tname);  
    free (tname);
    ifile = fopen(dname,"rb");
    if (ifile) {
      pfdf->name = dname;
      free (fname);
      return (ifile);
    }
    free (dname);
    while (*sdir!=' ' && *sdir!='\0') sdir++;   /* Advance */
  }

  /* If we fall thru, next try the GASCRP path */

  sdir = getenv("GASCRP");

  while (sdir!=NULL) {
    while (gsdelim(*sdir)) sdir++;
    if (*sdir=='\0') break;
    dname = gsstcp(sdir);
    if (dname==NULL) return(NULL);
    len = 0;              /* add slash to dir name if needed */
    while (*(dname+len)) len++;
    if (*(dname+len-1)!='/') {
      tname = gsstad(dname,"/");
      if (tname==NULL) return(NULL);
      free(dname);
      dname = tname;
    }
    tname = gsstad(dname,fname);
    free(dname);
    ifile = fopen(tname,"rb");
    if (ifile) {
      pfdf->name = tname;
      free (fname);
      return (ifile);
    }
    free (tname);
    while (*sdir!=' ' && *sdir!='\0') sdir++;   /* Advance */
  }
  
  /* If we fall thru, we didn't find anything.  */

  free (fname);
  return (NULL);
}

/* Copy a string to a new dynamically allocated area.
   Copy until a delimiter or null is encountered.
   Caller is responsible for freeing the storage.  */

char *gsstcp (char *ch) {
char *res;
gaint i,len;
  len = 0;
  while (!gsdelim(*(ch+len)) && *(ch+len)!='\0') len++;
  res = (char *)malloc(len+1);
  if (res==NULL) {
    printf ("Memory Allocation Error:  Script initialization\n");
    return (NULL);
  }
  i = 0;
  while (i<len) {
    *(res+i) = *(ch+i);
    i++;
  }
  *(res+len) = '\0';
  return (res);
}

/* Determine if a character is a delimiter for seperating the
   directory names in the GASCRP path.    To add new 
   delimiters, put them here. */

gaint gsdelim (char ch) {
  if (ch==' ') return (1);
  if (ch==';') return (1);
  if (ch==',') return (1);
  if (ch==':') return (1);
  return (0);
}

/* Concatentate two strings and make a new string in
   a dynamically allocated area.  Caller is responsible
   for freeing the storage.  */

char *gsstad(char *ch1, char *ch2) {
char *res;
gaint len,len2,i,j;
  len = 0;
  while (*(ch1+len)) len++;
  len2 = 0;
  while (*(ch2+len2)) len2++;
  res = (char *)malloc(len+len2+1);
  if (res==NULL) {
    printf ("Memory Allocation Error:  Script initialization\n");
    return (NULL);
  }
  i = 0;
  while (i<len) {
    *(res+i) = *(ch1+i);
    i++;
  }
  j = 0;   
  while (j<len2) {
    *(res+i) = *(ch2+j);
    i++; j++;
  }
  *(res+i) = '\0';
  return (res);
}
