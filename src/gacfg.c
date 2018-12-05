/* Copyright (C) 1988-2018 by George Mason University. See file COPYRIGHT for more information. */

/* file: gacfg.c
 *
 *   Prints the configuration options of this build of GrADS.
 *   This function is invoked at startup and with 'q config'.
 *
 *   REVISION HISTORY:
 *
 *   09sep97   da Silva   Initial code.
 *   12oct97   da Silva   Small revisions, use of gaprnt().
 *   15apr98   da Silva    Added BUILDINFO stuff, made gacfg() void. 
 *   24jun02   K Komine   Added 64-bit mode . 
 *
 *   --
 *   (c) 1997 by Arlindo da Silva
 *
 *   Permission is granted to any individual or institution to use,
 *   copy, or redistribute this software so long as it is not sold for
 *   profit, and provided this notice is retained. 
 *
 */

/* Include ./configure's header file */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include "buildinfo.h"

#if GRIB2==1
#include "grib2.h"
#endif

#if USEHDF==1
#include "mfhdf.h"
#endif

#if USEHDF5==1
#include "hdf5.h"
#endif


#include "gatypes.h"
#include "gx.h"
static struct gxdsubs *dsubs=NULL; /* function pointers for display  */
static struct gxpsubs *psubs=NULL; /* function pointers for printing */
void getwrd (char *, char *, gaint);
void gaprnt (int, char *);

/*
 * gacfg() - Prints several configuration parameters. 
 *
 *           verbose = 0   only config string
 *                   = 1   config string + verbose description
 *                   > 1   no screen display.
 */

void gacfg(int verbose) {
char cmd[256];
#if USEHDF==1
char hdfverstr[1024];
uint32 majorv=0,minorv=0,release=0;
#endif
#if (USEHDF5==1)
unsigned vmajor=0,vminor=0,vrelease=0;
#endif

if (dsubs==NULL) dsubs = getdsubs();  /* get ptrs to the graphics display functions */
if (psubs==NULL) psubs = getpsubs();  /* get ptrs to the graphics printing functions */

snprintf(cmd,255,"Config: v%s",GRADS_VERSION);
#if BYTEORDER==1
 strcat(cmd," big-endian");
#else
 strcat(cmd," little-endian");
#endif
#if READLINE==1
 strcat(cmd," readline");
#endif
#if GRIB2==1
 strcat(cmd," grib2");
#endif
#if USENETCDF==1
 strcat(cmd," netcdf");
#endif
#if USEHDF==1
 strcat(cmd," hdf4-sds");
#endif
#if USEHDF5==1
 strcat(cmd," hdf5");
#endif
#if USEDAP==1
 strcat(cmd," opendap-grids");
#endif
#if USEGADAP==1
 strcat(cmd,",stn");
#endif
#if USEGUI==1
 strcat(cmd," athena");
#endif
#if GEOTIFF==1
 strcat(cmd," geotiff");
#endif
#if USESHP==1
 strcat(cmd," shapefile");
#endif
 strcat(cmd,"\n");
 gaprnt(verbose,cmd);
 
 if (verbose==0) {
   gaprnt(verbose,"Issue 'q config' and 'q gxconfig' commands for more detailed configuration information\n");
   return;
 }
 
 gaprnt (verbose, "Grid Analysis and Display System (GrADS) Version " GRADS_VERSION "\n");
 gaprnt (verbose, "Copyright (C) 1988-2018 by George Mason University \n");
 gaprnt (verbose, "GrADS comes with ABSOLUTELY NO WARRANTY \n");
 gaprnt (verbose, "See file COPYRIGHT for more information \n\n");
 
 gaprnt (verbose, buildinfo );
 
 gaprnt(verbose,"\n\nThis build of GrADS has the following features:\n");
 
#if BYTEORDER==1
   gaprnt(verbose," -+- Byte order is BIG ENDIAN \n");
#else 
   gaprnt(verbose," -+- Byte order is LITTLE ENDIAN \n");
#endif

#if USEGUI==1
   gaprnt(verbose," -+- Athena Widget GUI ENABLED \n");
#else
   gaprnt(verbose," -+- Athena Widget GUI DISABLED \n");
#endif
 
#if READLINE==1
   gaprnt(verbose," -+- Command line editing ENABLED \n");
#else
   gaprnt(verbose," -+- Command line editing DISABLED \n");
#endif

#if GRIB2==1
   snprintf(cmd,255," -+- GRIB2 interface ENABLED  %s \n",G2_VERSION);
   gaprnt(verbose,cmd);
#else 
   gaprnt(verbose," -+- GRIB2 interface DISABLED\n");
#endif
 
#if USENETCDF==1
   const char * nc_inq_libvers(void);
   snprintf(cmd,255," -+- NetCDF interface ENABLED  netcdf-%s \n",nc_inq_libvers());
   gaprnt(verbose,cmd);
#else 
   gaprnt(verbose," -+- NetCDF interface DISABLED\n");
#endif
 
#if USEDAP==1
   gaprnt(verbose," -+- OPeNDAP gridded data interface ENABLED\n");
#else
   gaprnt(verbose," -+- OPeNDAP gridded data interface DISABLED\n");
#endif

#if USEGADAP==1
   const char *libgadap_version(void);
   snprintf(cmd,255," -+- OPeNDAP station data interface ENABLED  %s \n", libgadap_version());
   gaprnt(verbose,cmd);
#else
   gaprnt(verbose," -+- OPeNDAP station data interface DISABLED\n");
#endif

#if USEHDF==1
   Hgetlibversion(&majorv,&minorv,&release,hdfverstr);
   snprintf(cmd,255," -+- HDF4 interface ENABLED  hdf-%d.%dr%d \n",majorv,minorv,release);
   gaprnt(verbose,cmd);
#else
   gaprnt(verbose," -+- HDF4 interface DISABLED \n");
#endif

#if USEHDF5==1
   H5get_libversion(&vmajor,&vminor,&vrelease);
   snprintf(cmd,255," -+- HDF5 interface ENABLED  hdf5-%d.%d.%d \n",vmajor,vminor,vrelease);
   gaprnt(verbose,cmd);
#else
   gaprnt(verbose," -+- HDF5 interface DISABLED \n");
#endif

   gaprnt(verbose," -+- KML contour output ENABLED\n");

#if GEOTIFF==1
   gaprnt(verbose," -+- GeoTIFF and KML/TIFF output ENABLED\n");
#else
   gaprnt(verbose," -+- GeoTIFF and KML/TIFF output DISABLED\n");
#endif
#if USESHP==1
   gaprnt(verbose," -+- Shapefile interface ENABLED\n");
#else
   gaprnt(verbose," -+- Shapefile interface DISABLED\n");
#endif

 gaprnt(verbose,"The 'q gxconfig' command returns Graphics configuration information\n");
}

