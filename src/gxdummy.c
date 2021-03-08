/* This is a dummy file that contains no-ops for all the graphics subroutines 
   gcc -g -Wall -fPIC -rdynamic -c gxdummy.c
   gcc -g -Wall -fPIC -rdynamic -shared gxdummy.o -o gxdummy.so
*/
#include <stdio.h>
#include "gatypes.h"
#include "gx.h"

void gxdcfg () {}
gaint gxdckfont () {return 0;}
void gxdfb (char *a) {}
gaint gxdacol (gaint a, gaint b, gaint c, gaint d, gaint e) {return 0;}
void gxdbat () {}
void gxdbgn (gadouble a, gadouble b) {}
void gxdbtn (gaint a, gadouble *b, gadouble *c, gaint *d, gaint *e, gaint *f, gadouble *g) {}
gadouble gxdch (char a, gaint b, gadouble c, gadouble d, gadouble e, gadouble f, gadouble g) {return 0;}
void gxdclip (gadouble a, gadouble b, gadouble c, gadouble d) {}
void gxdcol (int a) {}
void gxddbl () {}
void gxddrw (gadouble a, gadouble b) {}
void gxdend () {}
void gxdfil (gadouble *a, gaint b) {}
void gxdfrm (int a) {}
void gxdgeo (char *a) {}
void gxdgcoord (gadouble a, gadouble b, gaint *c, gaint *d) {}
void gxdimg (gaint *a, gaint b, gaint c, gaint d, gaint e) {}
char* gxdlg (struct gdlg *a) {return NULL;}
void gxdmov (gadouble a, gadouble b) {}
void gxdopt (gaint a) {}
void gxdpbn (int a, struct gbtn *b , int c, int d, int e) {}
void gxdptn (int a, int b, int c) {}
gadouble gxdqchl (char a, gaint b, gadouble c) {return 0;}
void gxdrbb (gaint a, gaint b, gadouble c, gadouble d, gadouble e, gadouble f, gaint g) {}
void gxdrec (gadouble a, gadouble b, gadouble c, gadouble d) {}
void gxdrmu (int a, struct gdmu *b, int c, int d) {}
void gxdsfr (int a) {}
void gxdsgl () {}
void gxdsignal (gaint a) {}
void gxdssh (int a) {}
void gxdssv (int a) {}
void gxdswp () {}
void gxdwid (int a) {}
void gxdXflush () {}
void gxdxsz (int a, int b) {}
void gxrs1wd (int a, int b) {}
void gxsetpatt (gaint a) {}
gaint win_data (struct xinfo *a) {return 0;}

void gxpcfg () {}
gaint gxpckfont () {return 0;}
void gxpbgn (gadouble a, gadouble b) {}
void gxpinit (gadouble a, gadouble b) {}
void gxpend () {}
gaint gxprint (char *a, gaint b, gaint c, gaint d, gaint e, char *f, char *g, gaint h, gadouble i) {return 0;}
void gxpcol (gaint a) {}
void gxpacol (gaint a) {}
void gxpwid (gaint a) {}
void gxprec (gadouble a, gadouble b, gadouble c, gadouble d) {}
void gxpbpoly () {}
gaint gxpepoly (gadouble *a, gaint b) {return 0;}
void gxpmov (gadouble a, gadouble b) {}
void gxpdrw (gadouble a, gadouble b) {}
void gxpflush () {}
void gxpsignal (gaint a) {}
void gxpclip (gadouble a, gadouble b, gadouble c, gadouble d) {}
gadouble gxpch (char a, gaint b, gadouble c, gadouble d, gadouble e, gadouble f, gadouble g) {return 0;}
gadouble gxpqchl (char a, gaint b, gadouble c) {return 0;}
