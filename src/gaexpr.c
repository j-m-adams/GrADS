/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT for more information. */

/* Authored by B. Doty */

/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "grads.h"

static char pout[1256];     /* Build error msgs here */
static gaint pass=0;  /* Internal pass number */

/* Debugging routine to print the current stack */

/*
void stkdmp (struct smem *, gaint);

void stkdmp (struct smem *stack, gaint curr) {
struct gagrid *pgr;
gaint i;

  printf ("Stack: %i\n",curr);
  for (i=0;i<=curr;i++) {
    printf ("->");
    if (stack[i].type==0) {
      pgr = stack[i].obj.pgr;
      printf ("  Grid %g \n",*pgr->grid);
    } else if (stack[i].type==1) {
      printf ("  Oper %i \n",stack[i].obj.op);
    } else if (stack[i].type==2) {
      printf ("  Right Paren \n");
    } else if (stack[i].type==3) {
      printf ("  Left Paren \n");
    } else printf ("  Unknown %i \n",stack[i].type);
  }
}
*/

/* Evaluate a GrADS expression.  The expression must have
   blanks removed.  The expression is evaluated with respect
   to the environment defined in the status block (pst).
   This routine is invoked recursively from functions in order
   to evaluate sub-expressions.                                  */

gaint gaexpr (char *expr, struct gastat *pst) {
struct gagrid *pgr;
struct gastn *stn;
struct smem *stack;
char *ptr, *pos;
gadouble val;
gaint cmdlen,i,j,rc,curr;
gaint state,cont,err;
gaint size;

  if (gaqsig()) return(1);
  pass++;

  cmdlen = strlen(expr);
  size = sizeof(struct smem[cmdlen+10]);
  stack = (struct smem *)galloc(size,"stack");
  if (stack==NULL) {
    gaprnt (0,"Memory Allocation Error:  parser stack\n");
    return (1);
  }

  state=1;
  curr = -1;
  pos = expr;

  cont=1; err=0;
  while (cont) {                     /* Loop while parsing exprssn  */

    if (state) {                     /* Expect operand or '('       */

      if (*pos=='(') {               /* Handle left paren           */
        curr++;
        stack[curr].type = 2;
        pos++;
      }

      else if (*pos=='-') {          /* Handle unary '-' operator   */
        curr++;
        stack[curr].type = -1;
        stack[curr].obj.pgr = gagrvl(-1.0);
        curr++;
        stack[curr].type=1;  stack[curr].obj.op = 0;
        pos++;
      }

      else if (*pos>='0' && *pos<='9') {  /* Handle numeric value   */
        if ((ptr=getdbl(pos,&val))==NULL) {
          cont=0; err=1;
          i = 1 + pos - expr;
          gaprnt (0,"Syntax Error:  Invalid numeric value\n");
        } else {
          curr++;
          stack[curr].type = -1;
          stack[curr].obj.pgr  = gagrvl(val);
   /*     stkdmp(stack,curr);  */
          rc = eval(0, stack, &curr);
          if (rc) {
            err=1; cont=0;
          }
          state = 0;
          pos=ptr;
        }
      }

      else if (*pos>='a' && *pos<='z') {  /* Handle variable        */
        if ((ptr=varprs(pos, pst))==NULL) {
          cont=0; err=1;
        } else {
          curr++;
          if (pst->type) {
            stack[curr].type = -1;
            stack[curr].obj.pgr  = pst->result.pgr;
          } else {
            stack[curr].type = -2;
            stack[curr].obj.stn  = pst->result.stn;
          }
  /*      stkdmp(stack,curr);  */
          rc = eval(0, stack, &curr);
          if (rc) {
            err=1; cont=0;
          }
          state = 0;
          pos=ptr;
        }
      }

      else {
        gaprnt (0,"Syntax Error:  Expected operand or '('\n");
        cont=0; err=1;
      }

    } else {                         /* Expect operator or ')'      */

      if (*pos==')') {               /* Handle right paren          */
        curr++;
        stack[curr].type = 3;
        pos++;
        rc = eval(0, stack, &curr);  /* Process stack         */
        if (rc) {
          err=1; cont=0;
          pos--;
        }
      }

                                     /* Handle operator             */

      else if ( (*pos=='*')||(*pos=='/')||(*pos=='+')||(*pos=='-') ) {
        curr++;
        stack[curr].type=1;
        if (*pos=='*') stack[curr].obj.op=0;
        if (*pos=='/') stack[curr].obj.op=1;
        if (*pos=='+') stack[curr].obj.op=2;
        if (*pos=='-') {
          stack[curr].obj.op=2;
          curr++;
          stack[curr].type = -1;
          stack[curr].obj.pgr = gagrvl(-1.0);
          curr++;
          stack[curr].type=1;  stack[curr].obj.op = 0;
        }
   /*   stkdmp(stack,curr);  */
        pos++;
        state=1;
      }

      /* logical operator */

      else if ( (*pos=='=')||(*pos=='!')||(*pos=='<')||(*pos=='>')
                       ||(*pos=='|')||(*pos=='&') ) {
        if (*(pos+1)=='=') {
          curr++;
          stack[curr].type=1;
          if ( (*pos=='=')&&(*(pos+1)=='=') ) stack[curr].obj.op=20;
          if ( (*pos=='<')&&(*(pos+1)=='=') ) stack[curr].obj.op=23;
          if ( (*pos=='>')&&(*(pos+1)=='=') ) stack[curr].obj.op=24;
          if ( (*pos=='!')&&(*(pos+1)=='=') ) stack[curr].obj.op=25;
          pos+=2;
          state=1;
        } else if ((*pos=='|')&&(*(pos+1)=='|')) {
          curr++;
          stack[curr].type=1;
	  stack[curr].obj.op=26; 
          pos+=2;
          state=1;
        } else if ((*pos=='&')&&(*(pos+1)=='&')) {
          curr++;
          stack[curr].type=1;
	  stack[curr].obj.op=27; 
          pos+=2;
          state=1;
        } else {
          curr++;
          stack[curr].type=1;
          if (*pos=='=') stack[curr].obj.op=20;
          if (*pos=='<') stack[curr].obj.op=21;
          if (*pos=='>') stack[curr].obj.op=22;
          if (*pos=='|') stack[curr].obj.op=26;
          if (*pos=='&') stack[curr].obj.op=27;
          pos++;
          state=1;
        }
      }

      else {
        gaprnt (0,"Syntax Error:  Expected operator or ')'\n");
        cont=0; err=1;
      }
    }
    if (*pos=='\0'||*pos=='\n') cont=0;
  }

  if (!err) {
    rc = eval(1, stack, &curr);
/*  stkdmp(stack,curr);  */
    if (rc) {
      err=1;
    } else {
      if (curr==0) {
        if (stack[0].type == -1) {
          pst->type = 1;
          pst->result.pgr = stack[0].obj.pgr;
        } else if (stack[0].type == -2) {
          pst->type = 0;
          pst->result.stn = stack[0].obj.stn;
        } else {
          gaprnt (0,"GAEXPR Logic Error Number 29\n");
          err=1;
        }
      }
      else {
        gaprnt (0,"Syntax Error:  Unmatched Parens\n");
        err=1;
      }
    }
  }

  if (err) {
    if (pass==1) {
      i = 1 + pos - expr;
      snprintf(pout,1255,"  Error ocurred at column %i\n",i);
      gaprnt (0,pout);
    }

/*  release any memory still hung off the stack  */
    for (i=0; i<=curr; i++) {
      if (stack[i].type==-1) {
        pgr = stack[i].obj.pgr;
        gagfre(pgr);
        pst->result.pgr=NULL; 
      } else if (stack[i].type==-2) {
        stn = stack[i].obj.stn;
        for (j=0; j<BLKNUM; j++) {
          if (stn->blks[j] != NULL) gree(stn->blks[j],"f172");
        }
        gree(stn);
        pst->result.stn=NULL; 
      }
    }
  }
  gree(stack);
  pass--;
  return (err);
}


/* Evaluate the stack.  If the state is zero, then don't go
   past an addition operation unless enclosed in parens.
   If state is one, then do all operations to get the result.
   If we hit an error, set up the stack pointer to insure
   everything gets released properly.                                   */

gaint eval (gaint state, struct smem *stack, gaint *cpos) {
gaint cont,op,pflag,err,rc;
gaint curr,curr1,curr2;

  curr = *cpos;
  err = 0;
  cont = 1;
  pflag = 0;
  while (curr>0 && cont) {

    /* Handle an operand in the stack.  An operand is preceded by
       either a left paren, or an operator.  We will do an operation
       if it is * or /, or if it is enclosed in parens, or if we
       have hit the end of the expression.                           */

    if (stack[curr].type<0) {          /* Operand?                   */
      curr2 = curr;                    /* Remember operand           */
      curr--;                          /* Look at prior item         */
      if (stack[curr].type==2) {       /* Left paren?                */
        if (pflag) {                   /* Was there a matching right?*/
          stack[curr].type = stack[curr2].type; /* Yes, restack oprnd*/
          stack[curr].obj = stack[curr2].obj;
          pflag=0;                     /* Have evaluated parens      */
        } else {                       /* If not,                    */
          cont = 0;                    /*  stop here,                */
          curr++;                      /*  leaving operand on stack  */
        }
      } else if (stack[curr].type==1) {  /* Prior item an operator?  */
        op = stack[curr].obj.op;       /* Remember operator          */
        curr--;                        /* Get other operand          */
        if (stack[curr].type>0) {      /* Better be an operand       */
          cont = 0;                    /* If not then an error       */
          err = 1;
          gaprnt (0,"Internal logic check 12 \n");
        } else {                       /* Is an operand...           */
          curr1 = curr;                /* Get the operand            */
          if ( op<2 || pflag || state ) {             /* Operate?    */
            rc = gaoper(stack,curr1,curr2,curr,op);   /* Yes...      */
            if (rc) {                  /* Did we get an error?       */
              cont = 0; err = 1;       /* Yes...  don't continue     */
              curr+=2;                 /* Leave ptr so can free ops  */
            }
          } else {                     /* Don't operate...           */
            curr+=2;                   /* Leave stuff stacked        */
            cont = 0;                  /* Won't continue             */
          }
        }
      } else {                         /* Prior item invalid         */
        gaprnt (0,"Internal logic check 14 \n");
        cont = 0; err = 1;
      }

    } else if (stack[curr].type==3) {  /* Current item right paren?  */
      pflag=1;                         /* Indicate we found paren    */
      curr--;                          /* Unstack it                 */

    } else { cont=0; err=1; }          /* Invalid if not op or paren */
  }
  *cpos = curr;
  return (err);
}

/* Perform an operation on two data objects.  Determine what class
   of data we are working with and call the appropriate routine     */

gaint gaoper (struct smem *stack, gaint c1, gaint c2, gaint c, gaint op) {
struct gagrid *pgr=NULL;
struct gastn *stn;

  /* If both grids, do grid-grid operation */
  if (stack[c1].type==-1 && stack[c2].type==-1) {
    pgr = NULL; 
    pgr = gagrop(stack[c1].obj.pgr, stack[c2].obj.pgr, op, 1);
    if (pgr==NULL) return (1);
    stack[c].type = -1;
    stack[c].obj.pgr = pgr;
    return (0);
  }

  /* If both stns, do stn-stn operation */

  if (stack[c1].type==-2 && stack[c2].type==-2) {
    stn = NULL; 
    stn = gastop(stack[c1].obj.stn, stack[c2].obj.stn, op, 1);
    if (stn==NULL) return (1);
    stack[c].type = -2;
    stack[c].obj.stn = stn;
    return (0);
  }

  /* Operation between grid and stn is invalid -- unless the grid
     is really a constant.  Check for this.  */

  if (stack[c1].type == -1) pgr=stack[c1].obj.pgr;
  if (stack[c2].type == -1) pgr=stack[c2].obj.pgr;
  if (pgr->idim == -1 && pgr->jdim == -1) {
    if (stack[c1].type == -2) {
      stn = gascop (stack[c1].obj.stn, pgr->rmin, op, 0);
    } else {
      stn = gascop (stack[c2].obj.stn, pgr->rmin, op, 1);
    }
    if (stn==NULL) return (1);
    gagfre (pgr);
    stack[c].type = -2;
    stack[c].obj.stn = stn;
  } else {
    gaprnt (0,"Operation Error: Incompatable Data Types\n");
    gaprnt (0,"  One operand was stn data, other was grid\n");
    return (1);
  }
  return (0);
}

/* Perform the operation between two grid data objects.
   Varying dimensions must match.  Grids with fewer varying dimensions
   are 'expanded' to match the larger grid as needed.                 */

struct gagrid *gagrop (struct gagrid *pgr1, struct gagrid *pgr2,
                       gaint op, gaint rel) {

gadouble *val1, *val2;
gaint dnum1,dnum2;
struct gagrid *pgr;
gaint incr,imax,omax;
gaint i,i1,i2,swap;
char *uval1,*uval2;

  /* Figure out how many varying dimensions for each grid.            */

  val1 = pgr1->grid;
  uval1 = pgr1->umask;
  dnum1 = 0;
  if (pgr1->idim > -1) dnum1++;
  if (pgr1->jdim > -1) dnum1++;

  val2 = pgr2->grid;
  uval2 = pgr2->umask;
  dnum2 = 0;
  if (pgr2->idim > -1) dnum2++;
  if (pgr2->jdim > -1) dnum2++;

  /* Force operand 1 (pgr1, dnum1, etc.) to have fewer varying dims.  */
  swap = 0;
  if (dnum2<dnum1) {
    pgr = pgr1;
    pgr1 = pgr2;
    pgr2 = pgr;
    val1 = pgr1->grid;
    val2 = pgr2->grid;
    uval1 = pgr1->umask;
    uval2 = pgr2->umask;
    swap = 1;
    i = dnum1; dnum1 = dnum2; dnum2 = i;
  }

  /* Check the validity of the operation (same dimensions varying;
     same absolute dimension ranges.    First do the case where there
     are the same number of dimensions varying (dnum1=dnum2=0,1,2).   */

  if (dnum1==dnum2) {
    if (pgr1->idim != pgr2->idim || pgr1->jdim!=pgr2->jdim) goto err1;
    i = pgr1->idim;
    if (dnum1>0 && gagchk(pgr1,pgr2,pgr1->idim)) goto err2;
    i = pgr1->jdim;
    if (dnum1>1 && gagchk(pgr1,pgr2,pgr1->jdim)) goto err2;
    incr = 0;
    imax = pgr1->isiz * pgr1->jsiz;

  /* Case where dnum1=0, dnum2=1 or 2.  */

  } else if (dnum1==0) {
    incr = pgr2->isiz * pgr2->jsiz;
    imax = 1;

  /* Case where dnum1=1, dnum2=2.  */

  } else {
    i = pgr1->idim;
    if (gagchk(pgr1,pgr2,pgr1->idim)) goto err2;
    if (pgr1->idim==pgr2->idim) {
      incr = 0;
      imax = pgr1->isiz;
    } else if (pgr1->idim==pgr2->jdim) {
      incr = pgr2->isiz;
      imax = pgr1->isiz;
    } else goto err1;
  }
  omax = pgr2->isiz * pgr2->jsiz;

  /* Perform the operation.  Put the result in operand 2 (which is
     always the operand with the greater number of varying
     dimensions).  The smaller grid is 'expanded' by using incrementing
     variables which will cause the values in the smaller grid to be
     used multiple times as needed.                                   */

  i1 = 0; i2 = 0;
  for (i=0; i<omax; i++) {
    if (*uval1==0 || *uval2==0) {
      *uval2=0;
    }
    else {
      if (op==2) *val2 = *val1 + *val2;
      else if (op==0) *val2 = *val1 * *val2;
      else if (op==1) {
        if (swap) {
          if (dequal(*val1,0.0,1e-08)==0) {
	    *uval2 = 0;
	  }
          else {
	    *val2 = *val2 / *val1;
	  }
        } else {
          if (dequal(*val2,0.0,1e-08)==0) *uval2 = 0;
          else *val2 = *val1 / *val2;
        }
      } else if (op==10) {
        if (swap) {
	  if (isnan(pow(*val2,*val1))) *uval2 = 0;
	  else *val2 = pow(*val2,*val1);
	}
        else {
	  if (isnan(pow(*val1,*val2))) *uval2 = 0;
	  else *val2 = pow(*val1,*val2);
	}
      } else if (op==11)  *val2 = hypot(*val1,*val2);
      else if (op==12) {
        if (*val1==0.0 && *val2==0.0) *val2 = 0.0;
        else {
        if (swap) *val2 = atan2(*val2,*val1);
        else *val2 = atan2(*val1,*val2);
        }
      } else if (op==13) {
        if (swap) {
          if (*val1<0.0) *uval2 = 0;
        } else {
          if (*val2<0.0) *uval2 = 0;
          else *val2 = *val1;
        }
      } else if (op==14) {   /* for if function.  pairs with op 15 */
        if (swap) {
          if (*val2<0.0) *val2 = 0.0;
          else *val2 = *val1;
        } else {
          if (*val1<0.0) *val2 = 0.0;
        }
      } else if (op==15) { 
        if (swap) {
          if (*val2<0.0) *val2 = *val1;
          else *val2 = 0.0;
        } else {
          if (!(*val1<0.0)) *val2 = 0.0;
        }
      } else if (op>=21 && op<=24 ) {
        if (swap) {
          if (op==21) {
            if (*val2 < *val1) *val2 = 1.0;
            else *val2 = -1.0;
          }
          if (op==22) {
            if (*val2 > *val1) *val2 = 1.0;
            else *val2 = -1.0;
          }
          if (op==23) {
            if (*val2 <= *val1) *val2 = 1.0;
            else *val2 = -1.0;
          }
          if (op==24) {
            if (*val2 >= *val1) *val2 = 1.0;
            else *val2 = -1.0;
          }
        } else {
          if (op==21) {
            if (*val1 < *val2) *val2 = 1.0;
            else *val2 = -1.0;
          }
          if (op==22) {
            if (*val1 > *val2) *val2 = 1.0;
            else *val2 = -1.0;
          }
          if (op==23) {
            if (*val1 <= *val2) *val2 = 1.0;
            else *val2 = -1.0;
          }
          if (op==24) {
            if (*val1 >= *val2) *val2 = 1.0;
            else *val2 = -1.0;
          }
        }
      } else if (op==20) {
        if (*val1 == *val2) *val2 = 1.0;
        else *val2 = -1.0;
      } else if (op==25) {
        if (*val1 != *val2) *val2 = 1.0;
        else *val2 = -1.0;
      } else if (op==26) {
        if ( (*val1<0.0)&&(*val2<0.0) ) *val2 = -1.0;
        else *val2 = 1.0;
      } else if (op==27) {
        if ( (*val1>=0.0)&&(*val2>=0.0) ) *val2 = 1.0;
        else *val2 = -1.0;
      }

      else {
        gaprnt (0,"Internal logic check 17: invalid oper value\n");
        return (NULL);
      }
    }
    val2++; uval2++; i2++;
    if (i2>=incr) {i2=0; val1++; uval1++; i1++;}        /* Special increment for*/
    if (i1>=imax) {i1=0; val1=pgr1->grid; uval1=pgr1->umask;}     /*   the smaller grid   */
  }

  /* If requested, release the storage for operand 1 (which does not
     contain the result).  Note that this refers to operand 1 AFTER
     the possible grid swap earlier in the routine.                   */

  if (rel) {
    gagfre(pgr1);
  }

  return (pgr2);

  err1:
    gaprnt (0,"Operation error:  Incompatable grids \n");
    gaprnt (1,"   Varying dimensions are different\n");
    snprintf(pout,1255,"  1st grid dims = %i %i   2nd = %i %i \n",
            pgr1->idim, pgr2->idim, pgr1->jdim, pgr2->jdim);
    gaprnt (2,pout);
    return (NULL);

  err2:
    gaprnt (0,"Operation error:  Incompatable grids \n");
    snprintf(pout,1255,"  Dimension = %i\n",i);
    gaprnt (2, pout);
    snprintf(pout,1255,"  1st grid range = %i %i   2nd = %i %i \n",
            pgr1->dimmin[i],pgr1->dimmax[i],
            pgr2->dimmin[i],pgr2->dimmax[i]);
    gaprnt (2,pout);
    return (NULL);
}


/* Perform operation on two stn data items.  The operation is done
   only when the varying dimensions are equal.  Currently, only
   three station data dimension environments are supported:
   X,Y varying (X,Y plot), T varying (time series), and Z
   varying (vertical profile).  This routine will probably need to
   be rewritten at some point.                                     */

struct gastn *gastop (struct gastn *stn1, struct gastn *stn2,
                      gaint op, gaint rel) {
struct gastn *stn;
struct garpt *rpt1,*rpt2;
gaint swap,i,j,flag,dimtyp;

  /* Verify dimension environment */

  if (stn1->idim==0 && stn1->jdim==1 && 
      stn2->idim==0 && stn2->jdim==1) 
    dimtyp = 1;                                 /* X and Y are varying */
  else if (stn1->idim==2 && stn1->jdim==-1 && 
	   stn2->idim==2 && stn2->jdim==-1) 
    dimtyp = 2;                                 /* Z is varying */
  else if (stn1->idim==3 && stn1->jdim==-1 && 
	   stn2->idim==3 && stn2->jdim==-1) 
    dimtyp = 3;                                 /* T is varying */
  else {
    gaprnt (0,"Invalid dimension environment for station data");
    gaprnt (0," operation\n");
    return (NULL);
  }

  /* Set it up so first stn set has fewer stations */

  swap=0;
  if (stn1->rnum > stn2->rnum) {
    stn=stn1;
    stn1=stn2;
    stn2=stn;
    swap=1;
  }

  /* Loop through stations of 1st station set.  Find matching
     stations in 2nd station set.  If a match, perform operation.
     Any duplicates in the 2nd station set get ignored.      */

  rpt1 = stn1->rpt;
  for (i=0; i<stn1->rnum; i++,rpt1=rpt1->rpt) {
    if (rpt1->umask == 0) continue;
    flag = 0;
    rpt2 = stn2->rpt;
    for (j=0; j<stn2->rnum; j++,rpt2=rpt2->rpt) {
      if (rpt2->umask == 0) continue;
      if (dimtyp==1 && dequal(rpt1->lat,rpt2->lat,1e-08)!=0) continue;
      if (dimtyp==1 && dequal(rpt1->lon,rpt2->lon,1e-08)!=0) continue;
      if (dimtyp==2 && dequal(rpt1->lev,rpt2->lev,1e-08)!=0) continue;
      if (dimtyp==3 && dequal(rpt1->tim,rpt2->tim,1e-08)!=0) continue;
      if (op==2) 
	rpt1->val = rpt1->val + rpt2->val;
      else if (op==0) 
	rpt1->val = rpt1->val * rpt2->val;
      else if (op==1) {
        if (swap) {
          if (dequal(rpt1->val,0.0,1e-08)==0) rpt1->umask = 0;
          else rpt1->val = rpt2->val / rpt1->val;
        } else {
          if (dequal(rpt2->val,0.0,1e-08)==0) rpt1->umask = 0;
          else rpt1->val = rpt1->val / rpt2->val;
        }
      } 
      else if (op==10) {
        if (swap) {
	  if (isnan(pow(rpt2->val,rpt1->val))) rpt1->umask = 0;
	  else rpt1->val = pow(rpt2->val,rpt1->val);
	}
        else {
	  if (isnan(pow(rpt1->val,rpt2->val))) rpt1->umask = 0;
	  else rpt1->val = pow(rpt1->val,rpt2->val);
	}
      } 
      else if (op==11)  
	rpt1->val = hypot(rpt1->val,rpt2->val);
      else if (op==12) {
        if ((dequal(rpt1->val,0.0,1e-08)==0) && (dequal(rpt2->val,0.0,1e-08)==0)) 
	  rpt1->val = 0.0;
        else rpt1->val = atan2(rpt1->val,rpt2->val);
      } 
      else if (op==13) {
        if (swap) {
          if (rpt1->val<0.0) rpt1->umask = 0;
          else rpt1->val = rpt2->val;
        } else {
          if (rpt2->val<0.0) rpt1->umask = 0;
        }
      }
      else {
        gaprnt (0,"Internal logic check 57: invalid oper value\n");
        return (NULL);
      }
      flag=1;
      break;
    }
    if (!flag) rpt1->umask = 0;
  }

  /* Release storage if requested then return */

  if (rel) {
    for (i=0; i<BLKNUM; i++) {
      if (stn2->blks[i] != NULL) gree(stn2->blks[i],"f168");
    }
    gree(stn2,"f169");
  }
  return (stn1);
}

/* Perform operation between a stn set and a constant   */

struct gastn *gascop (struct gastn *stn, gadouble val, gaint op, gaint swap) {
struct garpt *rpt;
gaint i;

  /* Loop through stations.  Perform operation.              */

  rpt = stn->rpt;
  for (i=0; i<stn->rnum; i++,rpt=rpt->rpt) {
    if (rpt->umask == 0) continue;
    if (op==2) 
      rpt->val = rpt->val + val;
    else if (op==0) 
      rpt->val = rpt->val * val;
    else if (op==1) {
      if (swap) {
        if (dequal(rpt->val,0.0,1e-08)==0) rpt->umask = 0;
        else rpt->val = val / rpt->val;
      } else {
        if (dequal(val,0.0,1e-08)==0) rpt->umask = 0;
        else rpt->val = rpt->val / val;
      }
    } 
    else if (op==10) {
      if (swap) {
	if (isnan(pow(val,rpt->val))) rpt->umask = 0;
	else rpt->val = pow(val,rpt->val);
      }
      else {
	if (isnan(pow(rpt->val,val))) rpt->umask = 0;
	else rpt->val = pow(rpt->val,val);
      }
    } 
    else if (op==11)  
      rpt->val = hypot(rpt->val,val);
    else if (op==12) {
      if (dequal(rpt->val,0.0,1e-08)==0 && dequal(val,0.0,1e-08)==0) 
	rpt->val = 0.0;
      else {
        if (swap) rpt->val = atan2(val,rpt->val);
        else rpt->val = atan2(rpt->val,val);
      }
    } 
    else if (op==13) {
      if (rpt->val<0.0) rpt->umask = 0;
    }
    else {
      gaprnt (0,"Internal logic check 57: invalid oper value\n");
      return (NULL);
    }
  }
  return (stn);
}

/* Put a constant value into a grid.  We will change this at
   some point to have three data types (grid, stn, constant) but
   for now it is easier to keep the constant grid concept.     */

struct gagrid *gagrvl (gadouble val) {
struct gagrid *pgr;
gaint i;
size_t sz;

  /* Allocate memory */ 
  sz = sizeof(struct gagrid);
  pgr = (struct gagrid *)galloc(sz,"pgr1");
  if (pgr==NULL) {
    gaprnt (0,"Unable to allocate memory for grid structure \n");
    return (NULL);
  }
  /* Fill in gagrid variables */
  pgr->pfile = NULL;
  pgr->undef = -9.99e8;
  pgr->pvar  = NULL;
  pgr->idim  = -1;
  pgr->jdim  = -1;
  pgr->alocf = 0;
  for (i=0;i<5;i++) {
    pgr->dimmin[i]=0;
    pgr->dimmax[i]=0;
  }
  pgr->rmin = val;
  pgr->rmax = val;
  pgr->grid = &pgr->rmin;
  pgr->umin = 1;
  pgr->umask = &pgr->umin;
  pgr->isiz = 1;
  pgr->jsiz = 1;
  pgr->exprsn = NULL;
  return (pgr);
}

/* Handle a variable or function call.  If successful, we return
   a data object (pointed to by the pst) and a ptr to the first
   character after the variable or function name.  If an error
   happens, we return a NULL pointer.                                */

char *varprs (char *ch, struct gastat *pst) {
struct gagrid *pgr,*pgr2=NULL;
struct gafile *pfi;
struct gavar  *pvar, *pvar2, vfake;
gadouble (*conv) (gadouble *, gadouble);
gadouble dmin[5],dmax[5],d1,d2;
gadouble *cvals,*r,*r2;
gafloat wrot;
gaint i,fnum,ii,jj,rc,dotflg,idim,jdim,dim,sbu;
gaint id[5],toff=0;
gaint size,j,dotest,defined;
char *ru, *r2u, name[20], vnam[20], *pos;
size_t sz;

  /* Get the variable or function name.  It must start with a
     letter, and consist of letters or numbers or underscore.  */
  i=0;
  while ( (*ch>='a' && *ch<='z') || (*ch>='0' && *ch<='9' ) || (*ch == '_') ) {
    name[i] = *ch;
    vnam[i] = *ch;
    ch++; i++;
    if (i>16) break;
  }
  name[i] = '\0';
  vnam[i] = '\0';  /* Save 'i' for next loop */

  /* Check for the data set number in the variable name.  If there,
     then this has to be a variable name.                            */

  fnum = pst->fnum;
  dotflg=0;
  if (*ch == '.') {
    dotflg=1;
    ch++;
    pos = intprs(ch,&fnum);
    if (pos==NULL || fnum<1) {
      snprintf(pout,1255,"Syntax error: Bad file number for variable %s \n",name);
      gaprnt (0,pout);
      return (NULL);
    }
    vnam[i] = '.';
    i++;
    while (ch<pos) {
      vnam[i] = *ch;
      ch++; i++;
    }
    vnam[i] = '\0';
  }

  /* Check for a predefined data object. */
  pfi = NULL;
  pvar = NULL;
  defined=0;
  if (!dotflg) {
    pfi = getdfn(name,pst);
    if (pfi!=NULL) defined=1;
  }

  /* If not a defined grid, get a pointer to a file structure    */
  if (pfi==NULL) {
    if (!dotflg) {
      pfi = pst->pfid;
    }
    else {
      pfi = pst->pfi1;
      for (i=1; i<fnum && pfi!=NULL; i++) pfi = pfi->pforw;
      if (pfi==NULL) {
        gaprnt (0,"Data Request Error:  File number out of range \n");
        snprintf(pout,1255,"  Variable = %s \n",vnam);
        gaprnt (0,pout);
        return (NULL);
      }
    }

    /* Check here for predefined variable name: lat,lon,lev */
    if ( cmpwrd(name,"lat") ||
         cmpwrd(name,"lon") ||
         cmpwrd(name,"lev") ) {
      pvar = &vfake;
      vfake.levels = -999;
      vfake.vecpair = -999;
      if (cmpwrd(name,"lon")) {vfake.offset = 0; snprintf(vfake.abbrv,5,"lon");}
      if (cmpwrd(name,"lat")) {vfake.offset = 1; snprintf(vfake.abbrv,5,"lat");}
      if (cmpwrd(name,"lev")) {vfake.offset = 2; snprintf(vfake.abbrv,5,"lev");}
      if (pfi->type==2 || pfi->type==3) {
        snprintf(pout,1255,"Data Request Error:  Predefined variable %s\n", vnam);
        gaprnt (0,pout);
        gaprnt (0,"   is only defined for grid type files\n");
        snprintf(pout,1255,"   File %i is a station file\n",fnum);
        gaprnt (0,pout);
        return (NULL);
      }
    } 
    else {
      /* See if this is a variable name.  
	 If not, give an error message (if a file number was specified) 
	 or check for a function call via rtnprs.   */
      pvar = pfi->pvar1;
      for (i=0; (i<pfi->vnum)&&(!cmpwrd(name,pvar->abbrv)); i++) pvar++;
      if (i>=pfi->vnum) {
        if (dotflg) {
          gaprnt (0,"Data Request Error:  Invalid variable name \n");
          snprintf(pout,1255,"  Variable '%s' not found in file %i\n",vnam,fnum);
          gaprnt (0,pout);
          return (NULL);
        } else {
          ch = rtnprs(ch,name,pst);              /* Handle function call */
          return (ch);
        }
      }
    }
  }

  /* It wasn't a function call (or we would have returned).
     If the variable is to a stn type file, call the parser
     routine that handles stn requests.                         */
  if (pfi->type==2 || pfi->type==3) {
    ch = stnvar (ch, vnam, pfi, pvar, pst);
    return (ch);
  }

  /* We are dealing with a grid data request.  We handle this inline.
     Our default dimension limits are defined in gastat.  These
     may be modified by the user (by specifying the new settings
     in parens).  First get grid coordinates of the limits, then
     figure out if user modifies these.        */

  /* Convert world coordinates in the status block to grid
     dimensions using the file scaling for this variable.  */

  for (i=0;i<5;i++) {
    if (i==3) {
      dmin[i] = t2gr(pfi->abvals[i],&(pst->tmin));
      dmax[i] = t2gr(pfi->abvals[i],&(pst->tmax));
    }
    else {
      conv  = pfi->ab2gr[i];
      cvals = pfi->abvals[i];
      dmin[i] = conv(cvals,pst->dmin[i]);
      dmax[i] = conv(cvals,pst->dmax[i]);
    }
  }

  /* Round varying dimensions 'outwards' to integral grid units. */
  for (i=0;i<5;i++) {
    if (i==pst->idim || i==pst->jdim) {
      dmin[i] = floor(dmin[i]+0.0001);
      dmax[i] = ceil(dmax[i]-0.0001);
      if (dmax[i]<=dmin[i]) {
        gaprnt (0,"Data Request Error: Invalid grid coordinates\n");
        snprintf(pout,1255,"  Varying dimension %i decreases: %g to %g\n",i,dmin[i],dmax[i]);
        gaprnt (0,pout);
        snprintf(pout,1255,"  Error ocurred getting variable '%s'\n",vnam);
        gaprnt (0,pout);
        return (NULL);
      }
    }
  }

  /* Check for user provided dimension expressions */
  if (*ch=='(') {
    ch++;
    for (i=0;i<5;i++) id[i] = 0;
    while (*ch!=')') {
      pos = dimprs(ch, pst, pfi, &dim, &d1, 1, &rc);
      if (pos==NULL) {
        snprintf(pout,1255,"  Variable name = %s\n",vnam);
        gaprnt (0,pout);
        return (NULL);
      }
      if (id[dim]) {
        gaprnt (0,"Syntax Error: Invalid dimension expression\n");
        gaprnt (0,"  Same dimension specified multiple times ");
        snprintf(pout,1255,"for variable = %s\n",vnam);
        gaprnt (0,pout);
        return (NULL);
      }
      id[dim] = 1;
      if ( dim==pst->idim || dim==pst->jdim) {
        gaprnt (0,"Data Request Error: Invalid dimension expression\n");
        gaprnt (0,"  Attempt to set or modify varying dimension\n");
        snprintf(pout,1255,"  Variable = %s, Dimension = %i \n",vnam,dim);
        gaprnt (0,pout);
        return (NULL);
      }
      dmin[dim] = d1;
      dmax[dim] = d1;
      /* check if we need to set flag for time offset */
      if (rc>1) {
	if (defined==1) {
	  gaprnt (0,"Error: The \"offt\" dimension expression is \n");
          gaprnt (0,"       not supported for defined variables. \n");
	  return (NULL);
	}
	else toff=1;
      }
      ch = pos;
      if (*ch == ',') ch++;
    }
    ch++;
  }

  /* If request from a defined grid, ignore fixed dimensions
     in the defined grid */

  if (pfi->type==4 || pfi->type==5) {
    for (i=0; i<5; i++) {
      if (pfi->dnum[i]==1) {
        dmin[i] = 0.0;
        dmax[i] = 0.0;
      }
    }
  }

  /* All the grid level coordinates are set.  Insure they
     are integral values, otherwise we can't do it.   The varying
     dimensions will be integral (since we forced them to be
     earlier) so this is only relevent for fixed dimensions. */

  for (i=0; i<5; i++) {
    if (dmin[i]<0.0) 
      ii = (gaint)(dmin[i]-0.1);
    else 
      ii = (gaint)(dmin[i]+0.1);
    d1 = ii;
    if (dmax[i]<0.0) 
      ii = (gaint)(dmax[i]-0.1);
    else 
      ii = (gaint)(dmax[i]+0.1);
    d2 = ii;
    /* ignore z test if variable has no levels */
    dotest=1;
    if(pvar) {
      if(!pvar->levels && i == 2) dotest=0;
    }
    if (( dequal(dmin[i],d1,1e-8)!=0 || dequal(dmax[i],d2,1e-8)!=0) && dotest ) {
      gaprnt (0,"Data Request Error: Invalid grid coordinates\n");
      gaprnt (0,"  World coordinates convert to non-integer");
      gaprnt (0,"  grid coordinates\n");
      snprintf(pout,1255,"    Variable = %s  Dimension = %i \n",vnam,i);
      gaprnt (0,pout);
      return (NULL);
    }
  }
  /* Variable has been parsed and is valid, and the ch pointer is
     set to the first character past it.  We now need to set up
     the grid requestor block and get the grid.  */

  pgr = NULL; 
  sz = sizeof(struct gagrid);
  pgr = (struct gagrid *)galloc(sz,"gpgr");
  if (pgr==NULL) {
    gaprnt (0,"Memory Allocation Error:  Grid Request Block\n");
    return (NULL);
  }

  /* Fill in gagrid variables */

  idim = pst->idim; 
  jdim = pst->jdim;
  pgr->alocf = 0;
  pgr->pfile = pfi;
  pgr->undef = pfi->undef;
  pgr->pvar  = pvar;
  pgr->idim  = idim;
  pgr->jdim  = jdim;
  pgr->iwrld = 0;  
  pgr->jwrld = 0;
  pgr->toff  = toff;
  for (i=0;i<5;i++) {
    if (dmin[i]<0.0) {
      pgr->dimmin[i] = (gaint)(dmin[i]-0.1);
    }
    else {
      pgr->dimmin[i] = (gaint)(dmin[i]+0.1);
    }
    if (dmax[i]<0.0) {
      pgr->dimmax[i] = (gaint)(dmax[i]-0.1);
    }
    else {
      pgr->dimmax[i] = (gaint)(dmax[i]+0.1);
    }
  }
  pgr->exprsn = NULL;
  pgr->ilinr = 1;
  pgr->jlinr = 1;
  if (idim>-1 && idim!=3) {  
    pgr->igrab = pfi->gr2ab[idim];
    pgr->iabgr = pfi->ab2gr[idim];
  }
  if (jdim>-1 && jdim!=3) {
    pgr->jgrab = pfi->gr2ab[jdim];
    pgr->jabgr = pfi->ab2gr[jdim];
  }
  if (idim>-1 && jdim<=4) {    /* qqqqq xxxxx fix this later ? */
    pgr->ivals  = pfi->grvals[idim];
    pgr->iavals = pfi->abvals[idim];
    pgr->ilinr  = pfi->linear[idim];
  }
  if (jdim>-1 && jdim<=4) {    /* qqqqq xxxxx fix this later ? */
    pgr->jvals  = pfi->grvals[jdim];
    pgr->javals = pfi->abvals[jdim];
    pgr->jlinr  = pfi->linear[jdim];
  }
  pgr->grid = NULL;

  if (pfi && pvar && pfi->ppflag && pfi->ppwrot && pvar->vecpair>0) {
    pgr2 = NULL; 
    sz = sizeof(struct gagrid);
    pgr2 = (struct gagrid *)galloc(sz,"gpgr2");
    if (pgr2==NULL) {
      gaprnt (0,"Memory allocation error: Data I/O \n");
      gagfre(pgr);
      return (NULL);
    }
    *pgr2 = *pgr;
  }

  /* Get grid */
  rc = gaggrd (pgr);
  if (rc>0) {
    snprintf(pout,1255,"Data Request Error:  Error for variable '%s'\n", vnam);
    gaprnt (0,pout);
    gagfre(pgr);
    return (NULL);
  }
  if (rc<0) {
    snprintf(pout,1255,"  Warning issued for variable = %s\n",vnam);
    gaprnt (2,pout);
  }

  /* Special test for auto-interpolated data, when the
     data requested is U or V.  User MUST indicate variable unit
     number in the descriptor file for auto-rotation to take place */

  if (pfi && pvar && pfi->ppflag && pfi->ppwrot && pvar->vecpair>0) {

    /* Find the matching vector component */
    if (pvar->isu) sbu=0;    /* if pvar is u, then matching component should not be u */
    else sbu=1;              /* pvar is v, so matching component should be u */
    pvar2 = pfi->pvar1;
    i = 0;
    while (i<pfi->vnum) {
      if ((pvar2->vecpair == pvar->vecpair) && 
	  (pvar2->isu     == sbu)) break;
      pvar2++; i++;
    }
    if (i>=pfi->vnum) { /* didn't find a match */
      ru = pgr->umask;
      size = pgr->isiz*pgr->jsiz;
      for (i=0; i<size; i++) {*ru=0; ru++;}
    } else {
      /* get the 2nd grid */
      pgr2->pvar = pvar2;
      rc = gaggrd (pgr2);
      if (rc>0) {
        snprintf(pout,1255,"Data Request Error:  Error for variable '%s'\n", vnam);
        gaprnt (0,pout);
        gagfre(pgr);
        gagfre(pgr2);
        return (NULL);
      }
      /* r is u component, r2 is v component */
      if (pvar2->isu) { 
        r = pgr2->grid;
        r2 = pgr->grid;
        ru = pgr2->umask;
        r2u = pgr->umask;
      } else {
        r = pgr->grid;
        r2 = pgr2->grid;
        ru = pgr->umask;
        r2u = pgr2->umask;
      }
      ii = pgr->dimmin[0];
      jj = pgr->dimmin[1];
      for (j=0; j<pgr->jsiz; j++) {
        if (pgr->idim == 0) ii = pgr->dimmin[0];
        if (pgr->idim == 1) jj = pgr->dimmin[1];
        for (i=0; i<pgr->isiz; i++) {
         if (*ru==0 || *r2u==0) {  /* u or v is undefined */
            *ru = 0;
            *r2u = 0;
          } else {
            if (ii<1 || ii>pfi->dnum[0] ||
                jj<1 || jj>pfi->dnum[1]) {   /* outside file's grid dimensions */
              *ru = 0;
              *r2u = 0;
            } else {
 	      /* get wrot value for grid element */
	      wrot = *(pfi->ppw + (jj-1)*pfi->dnum[0] + ii - 1);
              if (wrot < -900.0) {
                *ru = 0;
                *r2u = 0;
              }
              else if (wrot != 0.0) {
                if (pvar2->isu) {
 		  *r2 = (*r)*sin(wrot) + (*r2)*cos(wrot); /* display variable is v */
		  *r2u = 1;
		}
                else {
 		  *r = (*r)*cos(wrot) - (*r2)*sin(wrot); /* display variable is u */
		  *ru = 1;
		}
              }
            }
          }
          r++; r2++; ru++; r2u++;
          if (pgr->idim == 0) ii++;
          if (pgr->idim == 1) jj++;
        }
        if (pgr->jdim == 1) jj++;
      }
      gagfre(pgr2);
    }
  }

  pst->result.pgr = pgr;
  pst->type = 1;
  return (ch);
}

/* Check that a specific dimension range is equivalent
   between two grids.  The dimension range is defined in the
   grid descriptor block in terms of grid coordinates, so
   conversions are made to absolute coordinates to insure
   equivalence in an absolute sense.

   A true result means the grids don't match.                       */

gaint gagchk (struct gagrid *pgr1, struct gagrid *pgr2, gaint dim) {
gadouble gmin1,gmax1,gmin2,gmax2,fuz1,fuz2,fuzz;
gadouble (*conv1) (gadouble *, gadouble);
gadouble (*conv2) (gadouble *, gadouble);
gadouble *vals1, *vals2;
gaint i1,i2,i,siz1,siz2,rc;
struct dt dtim1,dtim2;

  if (dim<0) return(0);

  if (dim==pgr1->idim) {
    conv1 = pgr1->igrab;
    vals1 = pgr1->ivals;
    i1 = pgr1->ilinr;
    siz1 = pgr1->isiz;
  } else if (dim==pgr1->jdim) {
    conv1 = pgr1->jgrab;
    vals1 = pgr1->jvals;
    i1 = pgr1->jlinr;
    siz1 = pgr1->jsiz;
  } else return (1);

  if (dim==pgr2->idim) {
    conv2 = pgr2->igrab;
    vals2 = pgr2->ivals;
    i2 = pgr2->ilinr;
    siz2 = pgr2->isiz;
  } else if (dim==pgr2->jdim) {
    conv2 = pgr2->jgrab;
    vals2 = pgr2->jvals;
    i2 = pgr2->jlinr;
    siz2 = pgr2->jsiz;
  } else return (1);

  if (siz1 != siz2) {
    gaprnt(0,"Error in gagchk: axis sizes are not the same\n");
    return(1);
  }

  gmin1 = pgr1->dimmin[dim];
  gmax1 = pgr1->dimmax[dim];
  gmin2 = pgr2->dimmin[dim];
  gmax2 = pgr2->dimmax[dim];

  if (dim==3) {                         /* Dimension is time.      */
    rc=0;
    gr2t (vals1, gmin1, &dtim1);
    gr2t (vals2, gmin2, &dtim2);
    if (dtim1.yr != dtim2.yr) rc=1;
    if (dtim1.mo != dtim2.mo) rc=1;
    if (dtim1.dy != dtim2.dy) rc=1;
    if (dtim1.hr != dtim2.hr) rc=1;
    if (dtim1.mn != dtim2.mn) rc=1;
    gr2t (vals1, gmax1, &dtim1);
    gr2t (vals2, gmax2, &dtim2);
    if (dtim1.yr != dtim2.yr) rc=1;
    if (dtim1.mo != dtim2.mo) rc=1;
    if (dtim1.dy != dtim2.dy) rc=1;
    if (dtim1.hr != dtim2.hr) rc=1;
    if (dtim1.mn != dtim2.mn) rc=1;
    if (rc) {
      gaprnt(0,"Error in gagchk: time axis endpoint values are not equivalent\n");
      return (1);
    }
    return (0);
  }

  /* Check endpoints.  If unequal, then automatic no match.        */

  fuz1=fabs(conv1(vals1,gmax1)-conv1(vals1,gmin1))*FUZZ_SCALE;
  fuz2=fabs(conv2(vals2,gmax2)-conv2(vals2,gmin2))*FUZZ_SCALE;
  fuzz=(fuz1+fuz2)*0.5;
  
  rc=0;
  if ( fabs((conv1(vals1,gmin1)) - (conv2(vals2,gmin2))) > fuzz ) rc=1;
  if ( fabs((conv1(vals1,gmax1)) - (conv2(vals2,gmax2))) > fuzz ) rc=1;
  if (rc) {
    gaprnt(0,"Error in gagchk: axis endpoint values are not equivalent\n");
    return (1);
  }
  if (i1!=i2) {
    gaprnt(0,"Error in gagchk: one axis is linear and the other is non-linear\n");
    return (1);
  }
  if (i1) return (0);                   /* If linear then matches  */

  /* Nonlinear, but endpoints match.  Check every grid point for a
     match.  If any non-matches, then not a match.     */

  for (i=0; i<siz1; i++) {
    if (fabs((conv1(vals1,gmin1+(gadouble)i)) - (conv2(vals2,gmin2+(gadouble)i))) > fuzz ) {
      gaprnt(0,"Error in gagchk: axis values are not all the same\n");
      return (1);
    }
  }
  return (0);
}

/* Check for defined data object. If found, make copy and return descriptor. */

struct gafile *getdfn (char *name, struct gastat *pst) {
struct gadefn *pdf;

  /* See if the name is a defined grid */
  pdf = pst->pdf1;
  while (pdf!=NULL && !cmpwrd(name,pdf->abbrv)) pdf = pdf->pforw;
  if (pdf==NULL) return (NULL);
  return (pdf->pfi);
}

/* Handle a station data request variable.                      */

char *stnvar (char *ch, char *vnam, struct gafile *pfi,
              struct gavar *pvar, struct gastat *pst) {
struct gastn *stn;
gadouble dmin[5],dmax[5],d,radius;
gaint id[6],dim,i,rc,rflag,sflag;
char *pos;
char stid[10];
size_t sz;

  rflag = 0;
  sflag = 0;
  radius = 0;

  /* We want to finish parsing the variable name by looking at
     any dimension settings by the user.  First initialize the
     request environment to that found in the pst.             */

  for (i=0;i<3;i++) {
    dmin[i] = pst->dmin[i];
    dmax[i] = pst->dmax[i];
  }
  dmin[3] = t2gr(pfi->abvals[3],&(pst->tmin));
  dmax[3] = t2gr(pfi->abvals[3],&(pst->tmax));

  /* Check for user provided dimension expressions */
  if (*ch=='(') {
    ch++;
    for (i=0;i<6;i++) id[i] = 0;
    while (*ch!=')') {
      if (!cmpch(ch,"stid=",5)) {   /* special stid= arg */
        for (i=0; i<8; i++) stid[i] = ' ';
        stid[8] = '\0';
        pos = ch+5;
        i=0;
        while (*pos!=',' && *pos!=')' && i<8) {
          stid[i] = *pos;
          pos++;
          i++;
        }
        if (i==0) {
          gaprnt (0,"Dimension Expression Error: No stid provided\n");
          pos=NULL;
        }
        if (i>8) {
          gaprnt (0,"Dimension Expression Error: stid too long\n");
          pos=NULL;
        }
	dim=11; 
      } else {
        pos = dimprs(ch, pst, pfi, &dim, &d, 0, &rc);
      }
      if (pos==NULL) {
        snprintf(pout,1255,"  Variable name = %s\n",vnam);
        gaprnt (0,pout);
        return (NULL);
      }
      if (dim<6 && id[dim]>1) {
        gaprnt (0,"Syntax Error: Invalid dimension expression\n");
        gaprnt (0,"  Same dimension specified more than twice ");
        snprintf(pout,1255,"for variable = %s\n",vnam);
        gaprnt (0,pout);
        return (NULL);
      }
      if ( dim==pst->idim || dim==pst->jdim ||
           ( dim>3 && (pst->idim==0 || pst->idim==1 || pst->jdim==1))) {
        gaprnt (0,"Data Request Error: Invalid dimension expression\n");
        gaprnt (0,"  Attempt to set or modify varying dimension\n");
        snprintf(pout,1255,"  Variable = %s, Dimension = %i \n",vnam,dim);
        gaprnt (0,pout);
        return (NULL);
      }
      if (dim==10) {
        rflag = 1;
        radius = d;
      } else if (dim==11) {
        sflag = 1;
      } else {
        if (id[dim]==0) dmin[dim] = d;
        dmax[dim] = d;
      }
      ch = pos;
      if (*ch == ',') ch++;
      id[dim]++;
    }
    ch++;
  }

  /* Verify that dmin is less than or equal to dmax for all our dims */
  for (i=0; i<4; i++) {
    if ((i!=2 && dmin[i]>dmax[i]) || (i==2 && dmax[i]>dmin[i])) {
      gaprnt (0,"Data Request Error: Invalid grid coordinates\n");
      snprintf(pout,1255,"  Varying dimension %i decreases: %g to %g \n",i,dmin[i],dmax[i]);
      gaprnt (0,pout);
      snprintf(pout,1255,"  Error ocurred getting variable '%s'\n",vnam);
      gaprnt (0,pout);
      return (NULL);
    }
  }

  /* Looks like the user specified good stuff, and we are ready to
     try to get some data.  Allocate and fill in a gastn block.     */

  sz = sizeof(struct gastn);
  stn = (struct gastn *)galloc(sz,"stn");
  if (stn==NULL) {
    gaprnt (0,"Memory Allocation Error:  Station Request Block \n");
    return (NULL);
  }
  stn->rnum = 0;
  stn->rpt = NULL;
  stn->pfi = pfi;
  stn->idim = pst->idim;
  stn->jdim = pst->jdim;
  stn->undef = pfi->undef;
  stn->tmin = dmin[3];
  stn->tmax = dmax[3];
  stn->ftmin = dmin[3];
  stn->ftmax = dmax[3];
  stn->pvar = pvar;
  for (i=0; i<3; i++) {
    stn->dmin[i] = dmin[i];
    stn->dmax[i] = dmax[i];
  }
  stn->rflag = rflag;
  stn->radius = radius;
  stn->sflag = sflag;
  if (sflag) {
    for (i=0; i<8; i++) stn->stid[i] = stid[i];
  }
  sz = sizeof(gadouble)*8;
  stn->tvals = (gadouble *)galloc(sz,"stntvals");
  if (stn->tvals==NULL) {
    gree(stn,"f170");
    gaprnt (0,"Memory Allocation Error:  Station Request Block \n");
    return (NULL);
  }
  for (i=0; i<8; i++) *(stn->tvals+i) = *(pfi->grvals[3]+i);

  rc = gagstn (stn);

  if (rc) {
    snprintf(pout,1255,"Data Request Error:  Variable is '%s'\n",vnam);
    gaprnt (0,pout);
    gree(stn,"f171");
    return (NULL);
  }
  pst->result.stn = stn;
  pst->type = 0;
  return (ch);
}
