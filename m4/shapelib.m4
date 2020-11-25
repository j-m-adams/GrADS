dnl GA_CHECK_LIB_SHP : Checks whether GrADS can be built with shapefile interface
dnl  args : 		action-if-yes, action-if-no
AC_DEFUN([GA_CHECK_LIB_SHP],
[
# set up help strings and parse user-provided paths
  AC_ARG_WITH([shp],
            [AS_HELP_STRING([--with-shp=ARG],[shapefile directory])],
            [SHP_PATH=$withval], 
            [SHP_PATH=""])

  AC_ARG_WITH([shp_include],
            [AS_HELP_STRING([--with-shp-include=ARG],[shapefile include directory])],
            [SHP_PATH_INC=$withval],
            [SHP_PATH_INC=""])

  AC_ARG_WITH([shp_libdir],
            [AS_HELP_STRING([--with-shp-libdir=ARG],[shapefile library directory])],
            [SHP_PATH_LIBDIR=$withval], 
            [SHP_PATH_LIBDIR=""])

# add /lib and /include to user-provided path      
  AS_IF([test "z$SHP_PATH" != "z"],
  [
    AS_IF([test "z$SHP_PATH_LIBDIR" = "z"],
      [SHP_PATH_LIBDIR="$SHP_PATH/lib"])  
    AS_IF([test "z$SHP_PATH_INC" = "z"],
      [SHP_PATH_INC="$SHP_PATH/include"])  
  ])
  
# initialize, save existing $LDFLAGS  
  ac_shp_lib_ok='no'
  ac_shp_save_LDFLAGS=$LDFLAGS
  SHP_LIBS=

# look for the library
  AS_IF([test -d "$SHP_PATH_LIBDIR"],
    [
      SHP_LDFLAGS="-L$SHP_PATH_LIBDIR"
      LDFLAGS="$LDFLAGS $SHP_LDFLAGS"
      AC_CHECK_SHP_LIB([ac_shp_lib_ok='yes'])
    ],
    [
      for ac_shp_libdir in "" /usr/lib64 /usr/local/lib \
       /usr/lib /usr/local/lib64 ; do
        AS_IF([test ! -d "$ac_shp_libdir"],
           [SHP_LDFLAGS=],
           [
             AC_MSG_NOTICE([searching for shapefile library in $ac_shp_libdir])
             SHP_LDFLAGS="-L$ac_shp_libdir"
           ])
        LDFLAGS="$LDFLAGS $SHP_LDFLAGS" 
        AC_CHECK_SHP_LIB([ac_shp_lib_ok='yes'])
        AS_IF([test $ac_shp_lib_ok = 'yes'],[break])
        LDFLAGS=$ac_shp_save_LDFLAGS
      done
    ])
  LDFLAGS=$ac_shp_save_LDFLAGS
  
# look for the include file 
  ac_shp_h='no'
  SHP_CFLAGS=
  ac_shp_save_CPPFLAGS=$CPPFLAGS
  AS_IF([test -d "$SHP_PATH_INC"],
    [
       SHP_CFLAGS="-I$SHP_PATH_INC"
       CPPFLAGS="$CPPFLAGS $SHP_CFLAGS"
       AC_CHECK_HEADER_NOCACHE_SHP([shapefil.h],[ac_shp_h='yes'])
    ],
    [
      for ac_shp_incdir in /usr/include \
       /usr/local/include /usr/local/include/libshp ; do
        AS_IF([test ! -d "$ac_shp_incdir"],
           [SHP_CFLAGS=],
           [
             AC_MSG_NOTICE([searching for shapefil.h in $ac_shp_incdir])
             SHP_CFLAGS="-I$ac_shp_incdir"
             CPPFLAGS="$CPPFLAGS $SHP_CFLAGS"
             AC_CHECK_HEADER_NOCACHE_SHP([shapefil.h],[ac_shp_h='yes'])
             AS_IF([test $ac_shp_h = 'yes'],[break])
             CPPFLAGS=$ac_shp_save_CPPFLAGS
           ])
      done
    ])
  CPPFLAGS=$ac_shp_save_CPPFLAGS

  AC_MSG_NOTICE([ac_shp_h is $ac_shp_h and ac_shp_lib_ok is $ac_shp_lib_ok ])
  AS_IF([test "$ac_shp_h" = 'yes' -a "$ac_shp_lib_ok" = 'yes'],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])

  AC_SUBST([SHP_LIBS])
  AC_SUBST([SHP_CFLAGS])
  AC_SUBST([SHP_LDFLAGS])
])


# the subroutines to check for 'main' in libshp
AC_DEFUN([AC_CHECK_SHP_LIB],
[
  SHP_LIBS=
  ac_shp_lib='no'
  ac_shp_save_LIBS=$LIBS
  AC_CHECK_LIB_NOCACHE_SHP([shp],[main],
  [ 
    ac_shp_lib="yes"
    SHP_LIBS="-lshp $SHP_LIBS" 
  ])
  LIBS=$ac_shp_save_LIBS
  
  AS_IF([test "$ac_shp_lib" = 'yes'],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])
])

AC_DEFUN([AC_CHECK_LIB_NOCACHE_SHP],
[
  AS_TR_SH([ac_check_lib_nocache_ok_$1_$2])='no'
  AS_TR_SH([ac_check_lib_nocache_$1_$2_LIBS])=$LIBS
  LIBS="-l$1 $5 $LIBS"
  AC_MSG_CHECKING([for $2 in -l$1])
  AC_LINK_IFELSE([AC_LANG_CALL([], [$2])],
  [ 
    AS_TR_SH([ac_check_lib_nocache_ok_$1_$2])='yes' 
    AC_MSG_RESULT([yes])
  ],[ 
    AC_MSG_RESULT([no])
  ])
  LIBS=$AS_TR_SH([ac_check_lib_nocache_$1_$2_LIBS])
  AS_IF([test $AS_TR_SH([ac_check_lib_nocache_ok_$1_$2]) = 'yes'],
  [m4_if([$3], [], [:], [$3])],
  [m4_if([$4], [], [:], [$4])])
])


# subroutine to look for the header file
AC_DEFUN([AC_CHECK_HEADER_NOCACHE_SHP],
[
  AS_TR_SH([ac_check_header_nocache_compile_$1])='no'
  AS_TR_SH([ac_check_header_nocache_preproc_$1])='no'
  AC_MSG_CHECKING([for $1 with compiler])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <$1>]])],
    [
      AC_MSG_RESULT([yes])
      AS_TR_SH([ac_check_header_nocache_compile_$1])='yes'
    ],
    [
      AC_MSG_RESULT([no])
    ])
  AC_MSG_CHECKING([for $1 with preprocessor])
  AC_PREPROC_IFELSE([AC_LANG_SOURCE([[#include <$1>]])],
    [
      AC_MSG_RESULT([yes])
      AS_TR_SH([ac_check_header_nocache_preproc_$1])='yes'
    ],
    [
      AC_MSG_RESULT([no])
      AS_IF([test "$AS_TR_SH([ac_check_header_nocache_compile_$1])" = 'yes'],
        [AC_MSG_WARN([trusting compiler result, ignoring preprocessor error])])
    ])
  AS_IF([test "$AS_TR_SH([ac_check_header_nocache_compile_$1])" = 'yes'],
  [m4_if([$2], [], [:], [$2])],
  [m4_if([$3], [], [:], [$3])])
])

