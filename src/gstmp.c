/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT for more information. */

/*  Code originally by M.Fiorino   */

/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "gs.h"

FILE *gsonam (struct gscmn *, struct gsfdef *);
int gsgsfrd (struct gscmn *);
char *gsstcp (char *);
int gsdelim (char);
char *gsstad (char *, char *);

/* test some script functions */

char *string = "mytest";

main () {
FILE *ifile;
struct gsfdef *pfdf;
struct gscmn *pcmn;
int rc;

  pcmn = (struct gscmn *)malloc(sizeof(struct gscmn));
  pfdf = (struct gsfdef *)malloc(sizeof(struct gsfdef));

  pcmn->fname = string;
  printf ("qqq p0 %s\n",pcmn->fname);
  ifile = gsonam(pcmn, pfdf);
  if (ifile!=NULL) {
    printf ("oname = -->%s<--\n",pfdf->name);
    printf ("prefix = -->%s<--\n",pcmn->fprefix);
  } else {
    printf ("null\n");
  }
}

/* Open the main script; search the path if needed 

     Rules:  When working with the name of the primary 
             script, 1st try to open the name provided, as is.
             If this fails, append .gs (if not there already)
             and try again.  If this fails, and the file 
             name provided does not start with a /, then 
             we try the directories in the GASCRP envvar, 
             both with the primary name and the .gs extension.
*/

FILE *gsonam (struct gscmn *pcmn, struct gsfdef *pfdf) {
FILE *ifile;
char *uname,*xname,*dname,*lname,*oname;
char *sdir;
int len;

  uname = NULL;  /* user provided name */
  xname = NULL;  /* user name plus extension */
  dname = NULL;  /* path dir name */
  lname = NULL;  /* path plus uname or xname */
  oname = NULL;  /* name of file that gets opened */
  
  /* First try to open by using the name provided. */
             
  uname = gsstad(pcmn->fname,"\0");
  if (uname==NULL) return(NULL);
  printf ("qqq p1 -->%s<--- \n",uname);
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
      if (sdir) printf ("qqqq -->%s<---\n",sdir);

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
    printf ("qqq p2 file opened \n");
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
 
/*  When working with a gsf, the function name
    is appended with .gsf.  Then we first try the
    same directory that the main script was found
    in.  If that fails, then we try the search path
    in GASCRP. */

/* Copy a string to a new dynamically allocated area.
   Copy until a delimiter or null is encountered.
   Caller is responsible for freeing the storage.  */

char *gsstcp (char *ch) {
char *res;
int i,len;
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

int gsdelim (char ch) {
  if (ch==' ') return (1);
  if (ch==';') return (1);
  if (ch==',') return (1);
  return (0);
}

/* Concatentate two strings and make a new string in
   a dynamically allocated area.  Caller is responsible
   for freeing the storage.  */

char *gsstad(char *ch1, char *ch2) {
char *res;
int len,len2,i,j;
  len = 0;
  while (*(ch1+len)) len++;
  len2 = 0;
  while (*(ch2+len2)) len2++;
  printf ("qqq p3 lens = %i %i\n",len,len2);
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
