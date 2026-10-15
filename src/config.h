/* src/config.h.  Generated from config.h.in by configure.  */
/* src/config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* little_endian machine */
#define BYTEORDER 0

/* Enabled geotiff */
#define GEOTIFF 1

/* Machine is a Cray */
#define GRADS_CRAY 0

/* Machine is 64-bit HP */
#define GRADS_HP64 0

/* GrADS version */
#define GRADS_VERSION "2.3"

/* Enable grib2 */
#define GRIB2 1

/* Enable netcdf4 */
#define HAVENETCDF4 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the <hdf4_netcdf.h> header file. */
/* #undef HAVE_HDF4_NETCDF_H */

/* Define to 1 if you have the <history.h> header file. */
/* #undef HAVE_HISTORY_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define if you have a readline compatible library */
/* #undef HAVE_LIBREADLINE */

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <readline.h> header file. */
/* #undef HAVE_READLINE_H */

/* Define if your readline library has \`add_history' */
/* #undef HAVE_READLINE_HISTORY */

/* Define to 1 if you have the <readline/history.h> header file. */
/* #undef HAVE_READLINE_HISTORY_H */

/* Define to 1 if you have the <readline/readline.h> header file. */
/* #undef HAVE_READLINE_READLINE_H */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/file.h> header file. */
#define HAVE_SYS_FILE_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define if hdf prefixes netcdf symbols by sd */
/* #undef HDF_HAVE_NETCDF */

/* A macro that append sd_ to netcdf symbols if needed */
/* #undef HDF_NETCDF_NAME */

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "jennifer.m.adams@nasa.gov"

/* Define to the full name of this package. */
#define PACKAGE_NAME "GrADS"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "GrADS 2.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "grads"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.3"

/* Enabled command line editing */
#define READLINE 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Enable cairo */
#define USECAIRO 1

/* Enable OPeNDAP for grids */
#define USEDAP 1

/* Enable OPeNDAP for station data */
/* #undef USEGADAP */

/* Enable gd */
#define USEGD 1

/* Enable hdf4 */
#define USEHDF 1

/* Enable hdf5 */
#define USEHDF5 1

/* Enable netcdf */
#define USENETCDF 1

/* Enable shapefile */
#define USESHP 1

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */
