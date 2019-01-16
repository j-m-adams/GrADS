dnl AC_CHECK_UDUNITS : Check for udunits
dnl args :             action-if-yes, action-if-no
AC_DEFUN([AC_CHECK_UDUNITS],
[
  AC_ARG_WITH([udunits],
            [AS_HELP_STRING([--with-udunits=ARG],[udunits directory])],
            [UDUNITS_PATH=$withval], 
            [UDUNITS_PATH=""])

  AC_ARG_WITH([udunits_include],
            [AS_HELP_STRING([--with-udunits-include=ARG],[udunits include directory])],
            [UDUNITS_PATH_INC=$withval],
            [UDUNITS_PATH_INC=""])

  AC_ARG_WITH([udunits_libdir],
            [AS_HELP_STRING([--with-udunits-libdir=ARG],[udunits library directory])],
            [UDUNITS_PATH_LIBDIR=$withval], 
            [UDUNITS_PATH_LIBDIR=""])

  AS_IF([test "z$UDUNITS_PATH" != "z"],
  [
    AS_IF([test "z$UDUNITS_PATH_LIBDIR" = "z"],
      [UDUNITS_PATH_LIBDIR="$UDUNITS_PATH/lib"])  
    AS_IF([test "z$UDUNITS_PATH_INC" = "z"],
      [UDUNITS_PATH_INC="$UDUNITS_PATH/include"])  
  ])
  
 
  ac_udunits_lib_ok='no'
  ac_udunits_save_LDFLAGS=$LDFLAGS
  UDUNITS_LIBS=
  AS_IF([test "z$UDUNITS_PATH_LIBDIR" != "z"],
    [
      UDUNITS_LDFLAGS="-L$UDUNITS_PATH_LIBDIR"
      LDFLAGS="$LDFLAGS $UDUNITS_LDFLAGS"
      AC_CHECK_UDUNITS_LIB([ac_udunits_lib_ok="yes"])
    ],
    [
      for ac_udunits_libdir in "" /usr/lib64 /usr/lib /usr/local/lib \
       /usr/libudunits/lib64 /usr/local/lib64/libudunits \
       /usr/local/udunits/lib /usr/local/libudunits/lib \ 
       /usr/udunits/lib /usr/local/lib/udunits /opt/lib/udunits \
       /usr/libudunits/lib /usr/local/lib/libudunits /opt/lib/libudunits \
       /usr/lib/udunits /usr/lib/libudunits ; do
        AS_IF([test "z$ac_udunits_libdir" = "z"],
           [UDUNITS_LDFLAGS=],
           [
             AC_MSG_NOTICE([searching udunits libraries in $ac_udunits_libdir])
             UDUNITS_LDFLAGS="-L$ac_udunits_libdir"
           ])
        LDFLAGS="$LDFLAGS $UDUNITS_LDFLAGS" 
        AC_CHECK_UDUNITS_LIB([ac_udunits_lib_ok="yes"])
        AS_IF([test $ac_udunits_lib_ok = "yes"],[break])
        LDFLAGS=$ac_udunits_save_LDFLAGS
      done
    ])
  LDFLAGS=$ac_udunits_save_LDFLAGS
  
  ac_udunits_h='no'
  UDUNITS_CFLAGS=
  ac_udunits_save_CPPFLAGS=$CPPFLAGS
  AS_IF([test "z$UDUNITS_PATH_INC" != "z"],
    [
       UDUNITS_CFLAGS="-I$UDUNITS_PATH_INC"
       CPPFLAGS="$CPPFLAGS $UDUNITS_CFLAGS"
       AC_CHECK_HEADER_NOCACHE_UDUNITS([udunits.h],[ac_udunits_h="yes"])
    ],
    [
      for ac_udunits_incdir in /usr/include \
       /usr/local/udunits/include /opt/udunits/include \ 
       /usr/udunits/include /usr/local/include/udunits \
       /opt/include/udunits /usr/include/udunits /usr/local/libudunits/include \
       /opt/libudunits/include /usr/libudunits/include /usr/local/include/libudunits \
       /opt/include/libudunits /usr/include/libudunits ; do
        AS_IF([test "z$ac_udunits_incdir" = "z"],
           [UDUNITS_CFLAGS=],
           [
             AC_MSG_NOTICE([searching udunits includes in $ac_udunits_incdir])
             UDUNITS_CFLAGS="-I$ac_udunits_incdir"
           ])
        CPPFLAGS="$CPPFLAGS $UDUNITS_CFLAGS"
        AC_CHECK_HEADER_NOCACHE_UDUNITS([udunits.h],[ac_udunits_h="yes"])
        AS_IF([test $ac_udunits_h = "yes"],[break])
        CPPFLAGS=$ac_udunits_save_CPPFLAGS
      done
    ])
  CPPFLAGS=$ac_udunits_save_CPPFLAGS
  
  AS_IF([test "$ac_udunits_h" = "yes" -a "$ac_udunits_lib_ok" = "yes"],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])

  AC_SUBST([UDUNITS_LIBS])
  AC_SUBST([UDUNITS_CFLAGS])
  AC_SUBST([UDUNITS_LDFLAGS])
])

AC_DEFUN([AC_CHECK_UDUNITS_LIB],
[
  UDUNITS_LIBS=
  ac_udunits_save_LIBS=$LIBS

  ac_udunits_lib='no'
  AC_CHECK_LIB_NOCACHE_UDUNITS([udunits],[main],
  [ ac_udunits_lib="yes"
    UDUNITS_LIBS="-ludunits $UDUNITS_LIBS"
  ])
  LIBS=$ac_udunits_save_LIBS
  
  AS_IF([test "$ac_udunits_lib" = "yes"],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])
])

AC_DEFUN([AC_CHECK_LIB_NOCACHE_UDUNITS],
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
 
AC_DEFUN([AC_CHECK_HEADER_NOCACHE_UDUNITS],
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
