dnl AC_CHECK_GEOTIFF : Check for geotiff
dnl args :             action-if-yes, action-if-no
AC_DEFUN([AC_CHECK_GEOTIFF],
[
# set up help strings and parse user-provided paths
  AC_ARG_WITH([geotiff],
            [AS_HELP_STRING([--with-geotiff=ARG],[geotiff directory])],
            [GEOTIFF_PATH=$withval], 
            [GEOTIFF_PATH=""])

  AC_ARG_WITH([geotiff_include],
            [AS_HELP_STRING([--with-geotiff-include=ARG],[geotiff include directory])],
            [GEOTIFF_PATH_INC=$withval],
            [GEOTIFF_PATH_INC=""])

  AC_ARG_WITH([geotiff_libdir],
            [AS_HELP_STRING([--with-geotiff-libdir=ARG],[geotiff library directory])],
            [GEOTIFF_PATH_LIBDIR=$withval], 
            [GEOTIFF_PATH_LIBDIR=""])

# add /lib and /include to user-provided path      
  AS_IF([test "z$GEOTIFF_PATH" != "z"],
  [
    AS_IF([test "z$GEOTIFF_PATH_LIBDIR" = "z"],
      [GEOTIFF_PATH_LIBDIR="$GEOTIFF_PATH/lib"])  
    AS_IF([test "z$GEOTIFF_PATH_INC" = "z"],
      [GEOTIFF_PATH_INC="$GEOTIFF_PATH/include"])  
  ])
  
# initialize, save existing $LDFLAGS  
  ac_geotiff_lib_ok='no'
  ac_geotiff_save_LDFLAGS=$LDFLAGS
  GEOTIFF_LIBS=

# if the lib directory exists, call function to look for geotiff library
  AS_IF([test -d "$GEOTIFF_PATH_LIBDIR"],
    [
      GEOTIFF_LDFLAGS="-L$GEOTIFF_PATH_LIBDIR"
      LDFLAGS="$LDFLAGS $GEOTIFF_LDFLAGS"
      AC_CHECK_GEOTIFF_LIB([ac_geotiff_lib_ok='yes'])
    ],
    [
      for ac_geotiff_libdir in "" /usr/lib64 /usr/geotiff/lib64 /usr/local/lib64/geotiff \
       /usr/libgeotiff/lib64 /usr/local/lib64/libgeotiff \
       /opt/lib64/geotiff /opt/lib64/libgeotiff \
       /opt/geotiff/lib64 /usr/lib64/geotiff /usr/local/geotiff/lib64 \
       /opt/libgeotiff/lib64 /usr/lib64/libgeotiff /usr/local/libgeotiff/lib64 \
       /usr/local/geotiff/lib /opt/geotiff/lib \ 
       /usr/local/libgeotiff/lib /opt/libgeotiff/lib \ 
       /usr/geotiff/lib /usr/local/lib/geotiff /opt/lib/geotiff \
       /usr/libgeotiff/lib /usr/local/lib/libgeotiff /opt/lib/libgeotiff \
       /usr/lib/geotiff /usr/lib/libgeotiff ; do
        AS_IF([test ! -d "$ac_geotiff_libdir"],
           [GEOTIFF_LDFLAGS=],
           [
             AC_MSG_NOTICE([searching geotiff libraries in $ac_geotiff_libdir])
             GEOTIFF_LDFLAGS="-L$ac_geotiff_libdir"
           ])
        LDFLAGS="$LDFLAGS $GEOTIFF_LDFLAGS" 
        AC_CHECK_GEOTIFF_LIB([ac_geotiff_lib_ok='yes'])
        AS_IF([test $ac_geotiff_lib_ok = 'yes'],[break])
        LDFLAGS=$ac_geotiff_save_LDFLAGS
      done
    ])
  LDFLAGS=$ac_geotiff_save_LDFLAGS
  
  ac_geotiff_h='no'
  GEOTIFF_CFLAGS=
  ac_geotiff_save_CPPFLAGS=$CPPFLAGS
  AS_IF([test -d "$GEOTIFF_PATH_INC"],
    [
       GEOTIFF_CFLAGS="-I$GEOTIFF_PATH_INC"
       CPPFLAGS="$CPPFLAGS $GEOTIFF_CFLAGS"
       AC_CHECK_HEADER_NOCACHE_GEOTIFF([geotiffio.h],[ac_geotiff_h='yes'])
    ],
    [
      for ac_geotiff_incdir in /usr/include \
       /usr/local/geotiff/include /opt/geotiff/include \ 
       /usr/geotiff/include /usr/local/include/geotiff \
       /opt/include/geotiff /usr/include/geotiff /usr/local/libgeotiff/include \
       /opt/libgeotiff/include /usr/libgeotiff/include /usr/local/include/libgeotiff \
       /opt/include/libgeotiff /usr/include/libgeotiff ; do
        AS_IF([test ! -d "$ac_geotiff_incdir"],
           [GEOTIFF_CFLAGS=],
           [
             AC_MSG_NOTICE([searching geotiff includes in $ac_geotiff_incdir])
             GEOTIFF_CFLAGS="-I$ac_geotiff_incdir"
             CPPFLAGS="$CPPFLAGS $GEOTIFF_CFLAGS"
             AC_CHECK_HEADER_NOCACHE_GEOTIFF([geotiffio.h],[ac_geotiff_h='yes'])
             AS_IF([test $ac_geotiff_h = 'yes'],[break])
             CPPFLAGS=$ac_geotiff_save_CPPFLAGS
           ])
      done
    ])
  CPPFLAGS=$ac_geotiff_save_CPPFLAGS
  
  AS_IF([test "$ac_geotiff_h" = 'yes' -a "$ac_geotiff_lib_ok" = 'yes'],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])

  AC_SUBST([GEOTIFF_LIBS])
  AC_SUBST([GEOTIFF_CFLAGS])
  AC_SUBST([GEOTIFF_LDFLAGS])
])

# the subroutine to check for 'main' in libtiff and libgeotiff
AC_DEFUN([AC_CHECK_GEOTIFF_LIB],
[
  GEOTIFF_LIBS=
  ac_geotiff_lib='no'
  ac_geotiff_save_LIBS=$LIBS
  AC_CHECK_LIB_NOCACHE_GEOTIFF([tiff], [main],
  [ AC_CHECK_LIB_NOCACHE_GEOTIFF([geotiff],[main],
    [ ac_geotiff_lib="yes"
      GEOTIFF_LIBS="-lgeotiff -ltiff $GEOTIFF_LIBS"
    ],[],[-ltiff])
  ])
  LIBS=$ac_geotiff_save_LIBS
  
  AS_IF([test "$ac_geotiff_lib" = 'yes'],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])
])

AC_DEFUN([AC_CHECK_LIB_NOCACHE_GEOTIFF],
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
 
AC_DEFUN([AC_CHECK_HEADER_NOCACHE_GEOTIFF],
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
