/* Copyright (C) 1988-2020 by George Mason University. See file COPYRIGHT for more information. */

/* This is the source file for the GrADS Python extension. 
   It gets compiled with the command 'python setup.py install'
   Originally authored by Brian Doty and Jennifer Adams in April 2018.

   http://python3porting.com/cextensions.html
   https://docs.python.org/3/howto/cporting.html
*/


#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>
#include <dlfcn.h>
#include <stdio.h>
#include "gradspy.h"

static int gapyerror;
static int gapystart;

/* This method starts GrADS */

static PyObject* start(PyObject* self, PyObject *args) {
PyObject *item;
Py_ssize_t tupsiz;
int i,rc,siz;
char *ganame = "gradspy";
char *arglist[50];

    if (gapystart) {
      PyErr_SetString(PyExc_TypeError, "start error: this method has already been invoked");
      return NULL;
    }

    if (gapyerror) {
      PyErr_SetString(PyExc_TypeError, "start error: prior initialization error");
      return NULL;
    }

    tupsiz = PyTuple_Size(args);
    siz = (int)tupsiz;

    arglist[0] = ganame;
    for (i=0; i<siz; i++) {
      item = PyTuple_GetItem(args,i);

      if (PyUnicode_Check(item) != 1) {
         PyErr_SetString(PyExc_TypeError, "start error: args must be strings");
         return NULL;
      } 
      arglist[i+1] = (char*)PyUnicode_AsUTF8(item);
    }
    rc = (*pgainit)(siz+1,arglist); /* Call gamain */
    if (rc == 0) gapystart = 1;     /* If all ok, set the flag */

    return Py_BuildValue("i", rc);  
}

/* The 'cmd' method executes a grads command.
   It calls subroutine gagsdo in gauser.c and returns any resulting text */

static PyObject* cmd(PyObject* self,PyObject *args) {
  char *str, *s;
  PyObject *resstr;
  int rc;
  if (gapyerror) {
    PyErr_SetString(PyExc_TypeError, "cmd error: prior initialization error");
    return NULL;
  }
  if (!gapystart) {
    PyErr_SetString(PyExc_TypeError, "cmd error: start method failed or not called");
    return NULL;
  }
  
  if (!PyArg_ParseTuple (args,"s", &s)) return NULL;
  str = (*pdocmd)(s,&rc);
  
  if (rc<0) exit(0);
  resstr = Py_BuildValue("s", str);
  free(str);
  return resstr;
}

/* The 'result' method evaluates a grads expression and returns the result in a Python tuple.

   The returned tuple has seven elements (one integer and six PyObjects):
   1. The return code contains the number of varying dimensions (rank) 
      of the result grid; if it is negative, an error occurred.
   2. 2D NumPy array containing the result grid (with NaN for missing data values) 
   3. 1D NumPy array of X coordinate values (NaN if X is not varying) 
   4. 1D NumPy array of Y coordinate values (NaN if Y is not varying)
   5. 1D NumPy array of Z coordinate values (NaN if Z is not varying)
   6. 1D NumPy array of grid metadata (longs)
   7. 1D NumPy array of grid metadata (doubles) 
*/

static PyObject* result(PyObject* self, PyObject *args) {
  PyArrayObject *res,*xvals,*yvals,*zvals,*iinfo,*dinfo,*junk;
  PyObject *rval;
  struct pygagrid pygr;
  double *r,*s,*t;
  char *expr,*ch;
  int i,j,rc;
  long *lr,*ls;
  npy_intp dims[2],dim[1]; 
  int nd;
  
  if (gapyerror) {
    PyErr_SetString(PyExc_TypeError, "result error: prior initialization error");
    return NULL;
  }
  if (!gapystart) {
    PyErr_SetString(PyExc_TypeError, "result error: start method failed or not called");
    return NULL;
  }

  /* Evaluate the expression. GrADS will populate the pygr structure with data and metadata */
  PyArg_ParseTuple (args,"s",&expr); 
  rc = (*pdoexpr)(expr,&pygr);

  /* Check for an error */
  if (rc<0) {
    /* something went wrong, so we return the tuple with a negative return code, 
       and the remaining elements contain a single value: nan */
    dims[0] = 1;
    dims[1] = 1;
    nd = 2;
    res = (PyArrayObject *) PyArray_SimpleNew(nd,dims,NPY_DOUBLE);
    r = (double *)PyArray_DATA(res);
    *r = strtod("nan",&ch);
    dim[0] = 1;
    nd = 1;
    junk = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
    r = (double *)PyArray_DATA(junk);
    *r = strtod("nan",&ch);
    rval =  Py_BuildValue("iNNNNNN",rc,res,junk,junk,junk,junk,junk); 
    return(rval);
  }
  
  /* set up a PyArray for the result grid, copy data from pygr structure */
  dims[0] = pygr.jsiz; 
  dims[1] = pygr.isiz;
  nd = 2;
  res = (PyArrayObject *) PyArray_SimpleNew(nd,dims,NPY_DOUBLE);
  r = (double *)PyArray_DATA(res);
  s = r; 
  t = pygr.grid;
  for (j=0; j<pygr.jsiz; j++) {
    for (i=0; i<pygr.isiz; i++) {
      *s = *(t+j*pygr.isiz+i);
      s++; 
    }
  }

  /* set up a PyArray for the X coordinate values, copy data from pygr structure */
  dim[0] = pygr.xsz;
  nd = 1;
  xvals = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
  r = (double *)PyArray_DATA(xvals);
  s = r; 
  t = pygr.xvals;
  for (i=0; i<pygr.xsz; i++) {
    *s = *(t+i);
    s++; 
  } 
  
  /* set up another PyArray for the Y coordinate values, copy data from pygr structure */
  dim[0] = pygr.ysz; 
  yvals = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
  r = (double *)PyArray_DATA(yvals);
  s = r; 
  t = pygr.yvals;
  for (i=0; i<pygr.ysz; i++) {
    *s = *(t+i);
    s++; 
  } 
  
  /* set up a PyArray for the Z coordinate values, copy data from pygr structure */
  dim[0] = pygr.zsz; 
  zvals = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
  r = (double *)PyArray_DATA(zvals);
  s = r; 
  t = pygr.zvals;
  for (i=0; i<pygr.zsz; i++) {
    *s = *(t+i);
    s++; 
  } 

  /* Set up a PyArray for metadata: 14 integers copied from pygr, cast to longs  */
  dim[0] = 14;
  iinfo = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_LONG); 
  lr = (long *)PyArray_DATA(iinfo);
  ls = lr; 
  *(ls+0)  = (long)pygr.xsz;      /* X (lon)  size (1 if not varying) */
  *(ls+1)  = (long)pygr.ysz;      /* Y (lat)  size (1 if not varying) */
  *(ls+2)  = (long)pygr.zsz;      /* Z (lev)  size (1 if not varying) */
  *(ls+3)  = (long)pygr.tsz;      /* T (time) size (1 if not varying) */  
  *(ls+4)  = (long)pygr.esz;      /* E (ens)  size (1 if not varying) */
  *(ls+5)  = (long)pygr.syr;      /* T start time -- year   */			  
  *(ls+6)  = (long)pygr.smo;      /* T start time -- month  */
  *(ls+7)  = (long)pygr.sdy;      /* T start time -- day    */
  *(ls+8)  = (long)pygr.shr;      /* T start time -- hour   */
  *(ls+9)  = (long)pygr.smn;      /* T start time -- minute */	  
  *(ls+10) = (long)pygr.tincr;    /* T increment */		  
  *(ls+11) = (long)pygr.ttyp;     /* type of T increment (0==months, 1==minutes) */
  *(ls+12) = (long)pygr.tcal;     /* T calendar type (0==normal, 1==365-day) */
  *(ls+13) = (long)pygr.estrt;    /* E start (E increment is always 1) */


  /* Set up another PyArray for more metadata: 6 doubles copied from pygr */
  dim[0] = 6;
  dinfo = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
  r = (double *)PyArray_DATA(dinfo);
  s = r; 
  *(s+0) = pygr.xstrt;      /* X start value (if linear) */
  *(s+1) = pygr.xincr;      /* X increment (negative if non-linear) */
  *(s+2) = pygr.ystrt;      /* Y start value (if linear) */
  *(s+3) = pygr.yincr;      /* Y increment (negative if non-linear) */
  *(s+4) = pygr.zstrt;      /* Z start value (if linear) */
  *(s+5) = pygr.zincr;      /* Z increment (negative if non-linear) */
  

  /* We're done copying data, so we can release the pygr structure on the GrADS side */
  (*pyfre)(&pygr);
  
  /* Returns the tuple containing the return code, the result, and the metadata back to Python. 
     This routine passes "ownership" of the object reference to our result back to Python
     by decrementing the reference count of the numpy result object */
  rval =  Py_BuildValue("iNNNNNN",rc,res,xvals,yvals,zvals,iinfo,dinfo);
  return(rval);
}


/* The 'put' method takes a Python tuple as an argument and creates a defined object within GrADS.

   The tuple has seven elements (one string and six PyObjects):
   0. The variable's name (alphanumeric, lowercase, starts with a letter, <=16 chars)
   1. The NumPy array containing the data grid (with NaN for missing data values) 
   2. 1D NumPy array of X coordinate values (NaN if X is not varying) 
   3. 1D NumPy array of Y coordinate values (NaN if Y is not varying)
   4. 1D NumPy array of Z coordinate values (NaN if Z is not varying)
   5. 1D NumPy array of grid metadata (14 integers)
   6. 1D NumPy array of grid metadata (6 doubles) 
*/

static PyObject* put(PyObject* self, PyObject *args) {

  PyArrayObject *grid,*xvals,*yvals,*zvals,*iinfo,*dinfo;
  PyObject *rval;
  struct pygagrid pygr;
  double mynan;
  char *name,*ch;
  int i,gsz,rc;
  long *idata;
  double *ddata,*gdata,*xdata,*ydata,*zdata;
  double *buf=NULL,*xlevs=NULL,*ylevs=NULL,*zlevs=NULL;

  
  if (gapyerror) {
    PyErr_SetString(PyExc_TypeError, "put error: prior initialization error");
    rc=1; goto rtrn;
  }
  if (!gapystart) {
    PyErr_SetString(PyExc_TypeError, "put error: start method failed or not called");
    rc=1; goto rtrn;
  }

  /* Evaluate the input tuple, returns true on success */
  rc = PyArg_ParseTuple(args,"(sOOOOOO)",&name,&grid,&xvals,&yvals,&zvals,&iinfo,&dinfo);

  if (!rc) {
    rc=1; goto rtrn;
  }
 
  /* Initialize the pygr structure*/
  mynan = strtod("nan",&ch);
  pygr.gastatptr = NULL;
  pygr.grid = NULL;
  pygr.isiz = 0;
  pygr.jsiz = 0;
  pygr.idim = -1;
  pygr.jdim = -1;
  pygr.xsz = 1; 
  pygr.ysz = 1; 
  pygr.zsz = 1; 
  pygr.tsz = 1; 
  pygr.esz = 1;
  pygr.xstrt = mynan; 
  pygr.ystrt = mynan; 
  pygr.zstrt = mynan;
  pygr.xincr = mynan; 
  pygr.yincr = mynan; 
  pygr.zincr = mynan;
  pygr.syr = 0; 
  pygr.smo = 0; 
  pygr.sdy = 0; 
  pygr.shr = 0; 
  pygr.smn = 0;
  pygr.tincr = 0; 
  pygr.ttyp = 0; 
  pygr.tcal = 0;
  pygr.estrt = 0;
  pygr.xvals = NULL; 
  pygr.yvals = NULL; 
  pygr.zvals = NULL;
  
  /* check the args */
  gdata = (double*)PyArray_DATA(grid);
  xdata = (double*)PyArray_DATA(xvals);
  ydata = (double*)PyArray_DATA(yvals);
  zdata = (double*)PyArray_DATA(zvals);
  idata =   (long*)PyArray_DATA(iinfo);
  ddata = (double*)PyArray_DATA(dinfo);

  /* Copy 14 integers from idata into pygr */
  pygr.xsz   = (int)idata[0] ;     /* X (lon)  size (1 if not varying) */
  pygr.ysz   = (int)idata[1] ;     /* Y (lat)  size (1 if not varying) */
  pygr.zsz   = (int)idata[2] ;     /* Z (lev)  size (1 if not varying) */
  pygr.tsz   = (int)idata[3] ;     /* T (time) size (1 if not varying) */
  pygr.esz   = (int)idata[4] ;     /* E (ens)  size (1 if not varying) */
  pygr.syr   = (int)idata[5] ;     /* T start time -- year   */
  pygr.smo   = (int)idata[6] ;     /* T start time -- month  */
  pygr.sdy   = (int)idata[7] ;     /* T start time -- day    */
  pygr.shr   = (int)idata[8] ;     /* T start time -- hour   */
  pygr.smn   = (int)idata[9] ;     /* T start time -- minute */
  pygr.tincr = (int)idata[10];     /* T increment */
  pygr.ttyp  = (int)idata[11];     /* type of T increment (0==months, 1==minutes) */
  pygr.tcal  = (int)idata[12];     /* T calendar type (0==normal, 1==365-day) */
  pygr.estrt = (int)idata[13];     /* E start (E increment is always 1) */

  /* Copy 6 doubles from dinfo into pygr */
  pygr.xstrt = *(ddata+0);      /* X start value (if linear) */
  pygr.xincr = *(ddata+1);      /* X increment (negative if non-linear) */
  pygr.ystrt = *(ddata+2);      /* Y start value (if linear) */
  pygr.yincr = *(ddata+3);      /* Y increment (negative if non-linear) */
  pygr.zstrt = *(ddata+4);      /* Z start value (if linear) */
  pygr.zincr = *(ddata+5);      /* Z increment (negative if non-linear) */
  
  /* Allocate memory to copy the grid values */
  gsz = pygr.xsz * pygr.ysz * pygr.zsz * pygr.tsz * pygr.esz; 
  buf = (double *)malloc(gsz*sizeof(double));
  if (buf==NULL) {
    PyErr_SetString(PyExc_TypeError, "put error: unable to allocate memory for data grid");
    rc=1; goto rtrn;
  }
  pygr.grid = buf;

  for (i=0; i<gsz; i++) *(buf+i) = *(gdata+i);

  /* if dimension increment is negative and size is > 1, copy array of levels for X, Y, and Z coordinates */
  if (pygr.xincr < 0 && pygr.xsz > 1) {
    xlevs = (double *)malloc(pygr.xsz*sizeof(double));
    if (xlevs==NULL) {
      PyErr_SetString(PyExc_TypeError, "put error: unable to allocate memory for X coordinate values");
      rc=1; goto rtrn;
    }
    pygr.xvals = xlevs;
    for (i=0; i<pygr.xsz; i++) *(xlevs+i) = *(xdata+i);
  }
  
  if (pygr.yincr < 0 && pygr.ysz > 1) {
    ylevs = (double *)malloc(pygr.ysz*sizeof(double));
    if (ylevs==NULL) {
      PyErr_SetString(PyExc_TypeError, "put error: unable to allocate memory for Y coordinate values");
      rc=1; goto rtrn;
    }
    pygr.yvals = ylevs;
    for (i=0; i<pygr.ysz; i++) *(ylevs+i) = *(ydata+i);
  }
  
  if (pygr.zincr < 0 && pygr.zsz > 1) {
    zlevs = (double *)malloc(pygr.zsz*sizeof(double));
    if (zlevs==NULL) {
      PyErr_SetString(PyExc_TypeError, "put error: unable to allocate memory for Z coordinate values");
      rc=1; goto rtrn;
    }
    pygr.zvals = zlevs;
    for (i=0; i<pygr.zsz; i++) *(zlevs+i) = *(zdata+i);
  }

  /* Hand the data and metadata to GrADS */
  rc = (*psetup)(name,&pygr);

rtrn:
  if (buf) free(buf);
  if (xlevs) free(xlevs);
  if (ylevs) free(ylevs);
  if (zlevs) free(zlevs);
  rval =  Py_BuildValue("i",rc); 
  return(rval);
}

/* The 'get' method takes the name of a defined variables and returns the data and metadata in a Python tuple.

   The returned tuple has seven elements (one integer and six PyObjects):
   1. The return code contains the number of varying dimensions (rank) 
      of the result grid; if it is negative, an error occurred.
   2. 2D NumPy array containing the result grid (with NaN for missing data values) 
   3. 1D NumPy array of X coordinate values (NaN if X is not varying) 
   4. 1D NumPy array of Y coordinate values (NaN if Y is not varying)
   5. 1D NumPy array of Z coordinate values (NaN if Z is not varying)
   6. 1D NumPy array of grid metadata (integers)
   7. 1D NumPy array of grid metadata (doubles) 
*/

static PyObject* get(PyObject* self, PyObject *args) {
  PyArrayObject *res,*xvals,*yvals,*zvals,*iinfo,*dinfo,*junk;
  PyObject *rval;
  struct pygagrid pygr;
  double *r,*s,*t;
  char *vname,*ch;
  off_t gsz,ig;
  int i,rc;
  long *lr,*ls;
  npy_intp dims[5],dim[1];
  int nd,pydim,gadims[5];
  
  if (gapyerror) {
    PyErr_SetString(PyExc_TypeError, "get error: prior initialization error");
    return NULL;
  }
  if (!gapystart) {
    PyErr_SetString(PyExc_TypeError, "get error: start method failed or not called");
    return NULL;
  }

  /* Calls GrADS routine gadoget() to copy defined variable data and metadata into pygr */
  PyArg_ParseTuple (args,"s",&vname); 
  rc = (*pdoget)(vname,&pygr);

  /* Check for an error */
  if (rc<0) {
    /* something went wrong, so we return the tuple with a negative return code, 
       and the remaining elements contain a single value: nan */
    dims[0] = 1;
    dims[1] = 1;
    nd = 2;
    res = (PyArrayObject *) PyArray_SimpleNew(nd,dims,NPY_DOUBLE);
    r = (double *)PyArray_DATA(res);
    *r = strtod("nan",&ch);
    dim[0] = 1;
    nd = 1;
    junk = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
    r = (double *)PyArray_DATA(junk);
    *r = strtod("nan",&ch);
    rval =  Py_BuildValue("iNNNNNN",rc,res,junk,junk,junk,junk,junk); 
    return(rval);
  }

  /* set up a PyArray for the result grid, copy data from pygr structure */
  /* this is where the order of dimensions gets reversed */
  
  gadims[0] = pygr.xsz; 
  gadims[1] = pygr.ysz;
  gadims[2] = pygr.zsz;
  gadims[3] = pygr.tsz;
  gadims[4] = pygr.esz;
  pydim=0;
  gsz = 1;
  for (i=4; i>=0; i--) {
    if (gadims[i] > 1) {
      dims[pydim] = gadims[i];
      gsz = gsz * dims[pydim];
      pydim++;
    }
  }
  nd = rc;  /* number of varying dims */

  res = (PyArrayObject *) PyArray_SimpleNew(nd,dims,NPY_DOUBLE);
  r = (double *)PyArray_DATA(res);
  s = r; 
  t = pygr.grid;
  for (ig=0; ig<gsz; ig++) {
    *s = *(t+ig);
    s++;
  }

  /* set up a PyArray for the X coordinate values, copy data from pygr structure */
  dim[0] = pygr.xsz;
  nd = 1;
  xvals = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
  r = (double *)PyArray_DATA(xvals);
  s = r; 
  t = pygr.xvals;
  for (i=0; i<pygr.xsz; i++) {
    *s = *(t+i);
    s++; 
  } 
  
  /* set up another PyArray for the Y coordinate values, copy data from pygr structure */
  dim[0] = pygr.ysz; 
  yvals = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
  r = (double *)PyArray_DATA(yvals);
  s = r; 
  t = pygr.yvals;
  for (i=0; i<pygr.ysz; i++) {
    *s = *(t+i);
    s++; 
  } 
  
  /* set up a PyArray for the Z coordinate values, copy data from pygr structure */
  dim[0] = pygr.zsz; 
  zvals = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
  r = (double *)PyArray_DATA(zvals);
  s = r; 
  t = pygr.zvals;
  for (i=0; i<pygr.zsz; i++) {
    *s = *(t+i);
    s++; 
  } 

  /* Set up a PyArray for metadata: 14 integers copied from pygr, cast to longs */
  dim[0] = 14;
  iinfo = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_LONG);
  lr = (long *)PyArray_DATA(iinfo);
  ls = lr; 
  *(ls+0)  = (long)pygr.xsz;      /* X (lon)  size (1 if not varying) */
  *(ls+1)  = (long)pygr.ysz;      /* Y (lat)  size (1 if not varying) */
  *(ls+2)  = (long)pygr.zsz;      /* Z (lev)  size (1 if not varying) */
  *(ls+3)  = (long)pygr.tsz;      /* T (time) size (1 if not varying) */  
  *(ls+4)  = (long)pygr.esz;      /* E (ens)  size (1 if not varying) */
  *(ls+5)  = (long)pygr.syr;      /* T start time -- year   */			  
  *(ls+6)  = (long)pygr.smo;      /* T start time -- month  */
  *(ls+7)  = (long)pygr.sdy;      /* T start time -- day    */
  *(ls+8)  = (long)pygr.shr;      /* T start time -- hour   */
  *(ls+9)  = (long)pygr.smn;      /* T start time -- minute */	  
  *(ls+10) = (long)pygr.tincr;    /* T increment */		  
  *(ls+11) = (long)pygr.ttyp;     /* type of T increment (0==months, 1==minutes) */
  *(ls+12) = (long)pygr.tcal;     /* T calendar type (0==normal, 1==365-day) */
  *(ls+13) = (long)pygr.estrt;    /* E start (E increment is always 1) */


  /* Set up another PyArray for more metadata: 6 doubles copied from pygr */
  dim[0] = 6;
  dinfo = (PyArrayObject *) PyArray_SimpleNew(nd,dim,NPY_DOUBLE);
  r = (double *)PyArray_DATA(dinfo);
  s = r; 
  *(s+0) = pygr.xstrt;      /* X start value (if linear) */
  *(s+1) = pygr.xincr;      /* X increment (negative if non-linear) */
  *(s+2) = pygr.ystrt;      /* Y start value (if linear) */
  *(s+3) = pygr.yincr;      /* Y increment (negative if non-linear) */
  *(s+4) = pygr.zstrt;      /* Z start value (if linear) */
  *(s+5) = pygr.zincr;      /* Z increment (negative if non-linear) */
  

  /* We're done copying data, so we can release the pygr structure on the GrADS side.
     In the 'get' method, pygr->gastatptr is NULL, so we won't delete the defined grid,
     only the three arrays of xvals, yvals, and zvals that were allocated in gadoget(). */
  (*pyfre)(&pygr);
  
  /* Returns the tuple containing the return code, the result, and the metadata back to Python. 
     This routine passes "ownership" of the object reference to our result back to Python
     by decrementing the reference count of the numpy result object */
  rval =  Py_BuildValue("iNNNNNN",rc,res,xvals,yvals,zvals,iinfo,dinfo);
  return(rval);
}


static PyMethodDef gradspy_funcs[] = {
    {"start",  (PyCFunction)start,  METH_VARARGS, "Start GrADS with desired switches and arguments"},
    {"cmd",    (PyCFunction)cmd,    METH_VARARGS, "Issue a command to GrADS"},
    {"result", (PyCFunction)result, METH_VARARGS, "Retrieve a grid using a GrADS expression"},
    {"put",    (PyCFunction)put,    METH_VARARGS, "Create a defined grid object in GrADS"},
    {"get",    (PyCFunction)get,    METH_VARARGS, "Retrieve a defined grid object from GrADS"},
    {NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "gradspy",                             /* m_name */
    "GrADS extension modules for Python",  /* m_doc */
    -1,                                    /* m_size */
    gradspy_funcs,                         /* m_methods */
    NULL,                                  /* m_reload */
    NULL,                                  /* m_traverse */
    NULL,                                  /* m_clear */
    NULL,                                  /* m_free */
};



/* This routine gets called when gradspy is imported into Python */

/* In Python 3 the value of PyMODINIT_FUNC is a PyObject*. In Python 2 it was void.
   We return NULL if an error happened or return the module object if initialization succeeded.  */

PyMODINIT_FUNC PyInit_gradspy(void) {

  PyObject *m;
  void *handle;
  const char *error;

  gapyerror = 0;

  m = PyModule_Create(&moduledef);
  if (m == NULL) goto err;
  
  /* Is there a more elegant way to handle the different shared object file names for linux and macOS? --JMA */
  handle = dlopen ("libgradspy.so",    RTLD_LAZY | RTLD_GLOBAL );
  /* handle = dlopen ("libgradspy.dylib", RTLD_LAZY | RTLD_GLOBAL ); */
  if (!handle) {
    fputs (dlerror(), stderr);
    fputs ("\n", stderr);
    goto err; 
  } 
  else {
    pgainit = dlsym(handle, "gamain");    /* starts GrADS */
    if ((error = dlerror()) != NULL)  {
      fputs(error, stderr);
      fputs ("\n", stderr);
      goto err;
    } 
    pdocmd = dlsym(handle, "gagsdo");     /* executes a command */
    if ((error = dlerror()) != NULL)  {
      fputs(error, stderr);
      fputs ("\n", stderr);
      goto err;
    }
    pdoexpr = dlsym(handle, "gadoexpr");  /* evaluates an expression */
    if ((error = dlerror()) != NULL)  {
      fputs(error, stderr);
      fputs ("\n", stderr);
      goto err;
    }
    psetup = dlsym(handle, "gasetup");    /* creates a defined grid object */
    if ((error = dlerror()) != NULL)  {
      fputs(error, stderr);
      fputs ("\n", stderr);
      goto err;
    }
    pdoget = dlsym(handle, "gadoget");    /* retreives a defined variable */
    if ((error = dlerror()) != NULL)  {
      fputs(error, stderr);
      fputs ("\n", stderr);
      goto err;
    }
    pyfre = dlsym(handle, "gapyfre");     /* releases memory after data is copied to Python */
    if ((error = dlerror()) != NULL)  {
      fputs(error, stderr);
      fputs ("\n", stderr);
      goto err;
    }
    /* This call makes sure that the module which implements the array
       type has been imported, and initializes a pointer array through
       which the NumPy functions are called. It is a bit of a black box */
    import_array();

  }
  return m;

 err:
  gapyerror = 1;
  return NULL;
}
