/* Copyright (C) 1988-2020 by George Mason University. See file COPYRIGHT for more information. */

/* Function prototypes for GD interface. 
   The interactive interface (X windows) is managed by the routines in gxX11.c
   Hardcopy output is managed in gxprintGD.c  */

/* function prototypes */
void  gxGDacol (gaint);
void  gxGDbpoly (void);
void  gxGDcol (gaint);
void  gxGDdrw (gadouble, gadouble);
gaint gxGDend (char *, char *, gaint, gaint);
gaint gxGDepoly (gadouble *, gaint);
gaint gxGDfgimg (char *);
void  gxGDflush (void);
gaint gxGDinit (gadouble, gadouble, gaint, gaint, gaint, char *);
void  gxGDmov (gadouble, gadouble);
void  gxGDpoly (gaint, gaint, gaint);
void  gxGDrec (gadouble, gadouble, gadouble, gadouble);
void  gxGDwid (gaint);
void  gxGDcfg (void);


