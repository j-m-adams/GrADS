/* Copyright (C) 1988-2020 by George Mason University. See file COPYRIGHT for more information. */

/* This is the include file for the GrADS-Python interface. 
   Originally authored by Brian Doty and Jennifer Adams in April 2018.
*/

struct pygagrid {
  void *gastatptr;             /* gastat                                */
  double *grid;                /* result grid values                    */
  int isiz,jsiz;               /* size of result grid                   */
  int idim,jdim;               /* dimensions used for row and column    */
  int xsz,ysz,zsz,tsz,esz;     /* size of XYZTE dims (1 if not varying) */
  double xstrt,ystrt,zstrt;    /* XYZ dim start values                  */
  double xincr,yincr,zincr;    /* XYZ dim increments (<0 if non-linear) */
  int syr,smo,sdy,shr,smn;     /* T start date                          */
  int tincr,ttyp,tcal;         /* T increment, type (mo/mn), calendar   */
  int estrt;                   /* E start (increment is always 1)       */
  double *xvals,*yvals,*zvals; /* XYZ world coordinate values           */
};

static char *(*pdocmd)(char *,int *);
static int   (*pgainit)(int,char **);
static int   (*pdoexpr)(char *,struct pygagrid *);
static int   (*pdoget)(char *,struct pygagrid *);
static int   (*psetup)(char *,struct pygagrid *);
static void  (*pyfre)(struct pygagrid *);
