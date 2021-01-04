dnl AC_CHECK_UDUNITS : Check for udunits
dnl args :             action-if-yes, action-if-no
AC_DEFUN([AC_CHECK_UDUNITS2],
[
  AC_ARG_WITH([udunits2],
            [AS_HELP_STRING([--with-udunits2=ARG],[udunits2 directory])],
            [UDUNITS2_PATH=$withval], 
            [UDUNITS2_PATH=""])

  AC_ARG_WITH([udunits2_include],
            [AS_HELP_STRING([--with-udunits2-include=ARG],[udunits2 include directory])],
            [UDUNITS2_PATH_INC=$withval],
            [UDUNITS2_PATH_INC=""])

  AC_ARG_WITH([udunits2_libdir],
            [AS_HELP_STRING([--with-udunits2-libdir=ARG],[udunits2 library directory])],
            [UDUNITS2_PATH_LIBDIR=$withval], 
            [UDUNITS2_PATH_LIBDIR=""])

  AS_IF([test "z$UDUNITS2_PATH" != "z"],
  [
    AS_IF([test "z$UDUNITS2_PATH_LIBDIR" = "z"],
      [UDUNITS2_PATH_LIBDIR="$UDUNITS2_PATH/lib"])  
    AS_IF([test "z$UDUNITS2_PATH_INC" = "z"],
      [UDUNITS2_PATH_INC="$UDUNITS2_PATH/include"])  
  ])
  
 
  ac_udunits2_lib_ok='no'
  ac_udunits2_save_LDFLAGS=$LDFLAGS
  UDUNITS2_LIBS=
  AS_IF([test "z$UDUNITS2_PATH_LIBDIR" != "z"],
    [
      UDUNITS2_LDFLAGS="-L$UDUNITS2_PATH_LIBDIR"
      LDFLAGS="$LDFLAGS $UDUNITS2_LDFLAGS"
      AC_CHECK_UDUNITS2_LIB([ac_udunits2_lib_ok="yes"])
    ],
    [
      for ac_udunits2_libdir in "" /usr/lib64 /usr/lib /usr/local/lib \
       /usr/libudunits/lib64 /usr/local/lib64/libudunits \
       /usr/local/udunits/lib /usr/local/libudunits/lib \ 
       /usr/udunits/lib /usr/local/lib/udunits /opt/lib/udunits \
       /usr/libudunits/lib /usr/local/lib/libudunits /opt/lib/libudunits \
       /usr/lib/udunits /usr/lib/libudunits ; do
        AS_IF([test "z$ac_udunits2_libdir" = "z"],
           [UDUNITS2_LDFLAGS=],
           [
             AC_MSG_NOTICE([searching udunits2 libraries in $ac_udunits2_libdir])
             UDUNITS2_LDFLAGS="-L$ac_udunits2_libdir"
           ])
        LDFLAGS="$LDFLAGS $UDUNITS2_LDFLAGS" 
        AC_CHECK_UDUNITS2_LIB([ac_udunits2_lib_ok="yes"])
        AS_IF([test $ac_udunits2_lib_ok = "yes"],[break])
        LDFLAGS=$ac_udunits2_save_LDFLAGS
      done
    ])
  LDFLAGS=$ac_udunits2_save_LDFLAGS
  
  ac_udunits2_h='no'
  UDUNITS2_CFLAGS=
  ac_udunits2_save_CPPFLAGS=$CPPFLAGS
  AS_IF([test "z$UDUNITS2_PATH_INC" != "z"],
    [
       UDUNITS2_CFLAGS="-I$UDUNITS2_PATH_INC"
       CPPFLAGS="$CPPFLAGS $UDUNITS2_CFLAGS"
       AC_CHECK_HEADER_NOCACHE_UDUNITS2([udunits2.h],[ac_udunits2_h="yes"])
    ],
    [
      for ac_udunits2_incdir in /usr/include \
       /usr/local/udunits/include /opt/udunits/include \ 
       /usr/udunits/include /usr/local/include/udunits \
       /opt/include/udunits /usr/include/udunits /usr/local/libudunits/include \
       /opt/libudunits/include /usr/libudunits/include /usr/local/include/libudunits \
       /opt/include/libudunits /usr/include/libudunits ; do
        AS_IF([test "z$ac_udunits2_incdir" = "z"],
           [UDUNITS2_CFLAGS=],
           [
             AC_MSG_NOTICE([searching udunits2 includes in $ac_udunits2_incdir])
             UDUNITS2_CFLAGS="-I$ac_udunits2_incdir"
           ])
        CPPFLAGS="$CPPFLAGS $UDUNITS2_CFLAGS"
        AC_CHECK_HEADER_NOCACHE_UDUNITS2([udunits2.h],[ac_udunits2_h="yes"])
        AS_IF([test $ac_udunits2_h = "yes"],[break])
        CPPFLAGS=$ac_udunits2_save_CPPFLAGS
      done
    ])
  CPPFLAGS=$ac_udunits2_save_CPPFLAGS
  
  AS_IF([test "$ac_udunits2_h" = "yes" -a "$ac_udunits2_lib_ok" = "yes"],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])

  AC_SUBST([UDUNITS2_LIBS])
  AC_SUBST([UDUNITS2_CFLAGS])
  AC_SUBST([UDUNITS2_LDFLAGS])
])

AC_DEFUN([AC_CHECK_UDUNITS2_LIB],
[
  UDUNITS2_LIBS=
  ac_udunits2_save_LIBS=$LIBS

  ac_udunits2_lib='no'
  AC_CHECK_LIB_NOCACHE_UDUNITS2([udunits2],[main],
  [ ac_udunits2_lib="yes"
    UDUNITS2_LIBS="-ludunits2 $UDUNITS2_LIBS"
  ])
  LIBS=$ac_udunits2_save_LIBS
  
  AS_IF([test "$ac_udunits2_lib" = "yes"],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])
])

AC_DEFUN([AC_CHECK_LIB_NOCACHE_UDUNITS2],
[
  AS_TR_SH([ac_check_lib_nocache_ok_$1_$2])='no'
  AS_TR_SH([ac_check_lib_nocache_$1_$2_LIBS])=$LIBS
  LIBS="-l$1 $5 $LIBS"
  AC_MSG_CHECKING([for $2 in -l$1])
  AC_LINK_IFELSE([AC_LANG_CALL([], [$2])],
  [ 
    AS_TR_SH([ac_check_lib_nocache_ok_$1_$2])="yes" 
    AC_MSG_RESULT([yes])
  ],[ 
    AC_MSG_RESULT([no])
  ])
  LIBS=$AS_TR_SH([ac_check_lib_nocache_$1_$2_LIBS])
  AS_IF([test $AS_TR_SH([ac_check_lib_nocache_ok_$1_$2]) = "yes"],
  [m4_if([$3], [], [:], [$3])],
  [m4_if([$4], [], [:], [$4])])
])
 
AC_DEFUN([AC_CHECK_HEADER_NOCACHE_UDUNITS2],
[
  AS_TR_SH([ac_check_header_nocache_compile_$1])='no'
  AS_TR_SH([ac_check_header_nocache_preproc_$1])='no'
  AC_MSG_CHECKING([for $1 with compiler])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <$1>]])],
    [
      AC_MSG_RESULT([yes])
      AS_TR_SH([ac_check_header_nocache_compile_$1])="yes"
    ],
    [
      AC_MSG_RESULT([no])
    ])
  AC_MSG_CHECKING([for $1 with preprocessor])
  AC_PREPROC_IFELSE([AC_LANG_SOURCE([[#include <$1>]])],
    [
      AC_MSG_RESULT([yes])
      AS_TR_SH([ac_check_header_nocache_preproc_$1])="yes"
    ],
    [
      AC_MSG_RESULT([no])
      AS_IF([test "$AS_TR_SH([ac_check_header_nocache_compile_$1])" = "yes"],
        [AC_MSG_WARN([trusting compiler result, ignoring preprocessor error])])
    ])
  AS_IF([test "$AS_TR_SH([ac_check_header_nocache_compile_$1])" = "yes"],
  [m4_if([$2], [], [:], [$2])],
  [m4_if([$3], [], [:], [$3])])
])
