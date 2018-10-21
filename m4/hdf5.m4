dnl AC_CHECK_HDF5 : Check for hdf5
dnl args :             action-if-yes, action-if-no
AC_DEFUN([AC_CHECK_HDF5],
[
  AC_ARG_WITH([hdf5],
            [AS_HELP_STRING([--with-hdf5=ARG],[hdf5 directory])],
            [HDF5_PATH=$withval], 
            [HDF5_PATH=""])

  AC_ARG_WITH([hdf5_include],
            [AS_HELP_STRING([--with-hdf5-include=ARG],[hdf5 include directory])],
            [HDF5_PATH_INC=$withval],
            [HDF5_PATH_INC=""])

  AC_ARG_WITH([hdf5_libdir],
            [AS_HELP_STRING([--with-hdf5-libdir=ARG],[hdf5 library directory])],
            [HDF5_PATH_LIBDIR=$withval], 
            [HDF5_PATH_LIBDIR=""])

  AS_IF([test "z$HDF5_PATH" != "z"],
  [
    AS_IF([test "z$HDF5_PATH_LIBDIR" = "z"],
      [HDF5_PATH_LIBDIR="$HDF5_PATH/lib"])  
    AS_IF([test "z$HDF5_PATH_INC" = "z"],
      [HDF5_PATH_INC="$HDF5_PATH/include"])  
  ])
  
  
  ac_hdf5_lib_ok='no'
  ac_hdf5_save_LDFLAGS=$LDFLAGS
  HDF5_LIBS=
  AS_IF([test "z$HDF5_PATH_LIBDIR" != "z"],
    [
      HDF5_LDFLAGS="-L$HDF5_PATH_LIBDIR"
      LDFLAGS="$LDFLAGS $HDF5_LDFLAGS"
      AC_CHECK_HDF5_LIB([ac_hdf5_lib_ok='yes'])
    ],
    [
      for ac_hdf5_libdir in "" /usr/local/hdf5/lib64/ /opt/hdf5/lib64 /usr/hdf5/lib64 \
       /usr/local/lib64/hdf5 /opt/lib64/hdf5 /usr/lib64/hdf5 /usr/lib64 \
       /usr/local/hdf5/lib/ /opt/hdf5/lib /usr/hdf5/lib \
       /usr/local/lib/hdf5 /opt/lib/hdf5 /usr/lib/hdf5 /usr/lib ; do
        AS_IF([test "z$ac_hdf5_libdir" = 'z'],
           [HDF5_LDFLAGS=],
           [
             AC_MSG_NOTICE([searching hdf5 libraries in $ac_hdf5_libdir])
             HDF5_LDFLAGS="-L$ac_hdf5_libdir"
           ])
        LDFLAGS="$LDFLAGS $HDF5_LDFLAGS" 
        AC_CHECK_HDF5_LIB([ac_hdf5_lib_ok='yes'])
        AS_IF([test $ac_hdf5_lib_ok = 'yes'],[break])
        LDFLAGS=$ac_hdf5_save_LDFLAGS
      done
    ])
  LDFLAGS=$ac_hdf5_save_LDFLAGS
  
  ac_hdf5_h='no'
  HDF5_CFLAGS=
  ac_hdf5_save_CPPFLAGS=$CPPFLAGS
  AS_IF([test "z$HDF5_PATH_INC" != "z"],
    [
       HDF5_CFLAGS="-I$HDF5_PATH_INC"
       CPPFLAGS="$CPPFLAGS $HDF5_CFLAGS"
       AC_CHECK_HEADER_NOCACHE_HDF5([hdf5.h],[ac_hdf5_h='yes'])
    ],
    [
      for ac_hdf5_incdir in "" /usr/include /usr/local/hdf5/include \
       /opt/hdf5/include /usr/hdf5/include /usr/local/include/hdf5 \
       /opt/include/hdf5 /usr/include/hdf5 ; do
        AS_IF([test "z$ac_hdf5_incdir" = 'z'],
           [HDF5_CFLAGS=],
           [
             AC_MSG_NOTICE([searching hdf5 includes in $ac_hdf5_incdir])
             HDF5_CFLAGS="-I$ac_hdf5_incdir"
           ])
        CPPFLAGS="$CPPFLAGS $HDF5_CFLAGS" 
        AC_CHECK_HEADER_NOCACHE_HDF5([hdf5.h],[ac_hdf5_h='yes'])
        AS_IF([test $ac_hdf5_h = 'yes'],[break])
        CPPFLAGS=$ac_hdf5_save_CPPFLAGS
      done
    ])
  CPPFLAGS=$ac_hdf5_save_CPPFLAGS
  
  AS_IF([test "$ac_hdf5_h" = 'yes' -a "$ac_hdf5_lib_ok" = 'yes'],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])

  AC_SUBST([HDF5_LIBS])
  AC_SUBST([HDF5_CFLAGS])
  AC_SUBST([HDF5_LDFLAGS])
])

AC_DEFUN([AC_CHECK_HDF5_LIB],
[
  HDF5_LIBS=
  ac_hdf5_save_LIBS=$LIBS

dnl -lsz is not required because due to licencing it may not be present
dnl nor required everywhere. GrADS will do without szip.
dnl  AC_CHECK_LIB_NOCACHE_HDF5([sz], [main],
dnl  [
dnl      LIBS="$LIBS -lsz"
dnl      HDF5_LIBS='-lsz'
dnl  ])

  ac_hdf5_lib='no'
  AC_CHECK_LIB_NOCACHE_HDF5([z],[compress],
  [ AC_CHECK_LIB_NOCACHE_HDF5([jpeg],[main],
    [ AC_CHECK_LIB_NOCACHE_HDF5([hdf5],[H5Fopen],
      [ ac_hdf5_lib="yes"
          HDF5_LIBS="-lhdf5 -ljpeg -lz $HDF5_LIBS"
      ],[],[-lhdf5 -ljpeg -lz])
    ])
  ])
  LIBS=$ac_hdf5_save_LIBS
  
  AS_IF([test "$ac_hdf5_lib" = 'yes'],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])
])

dnl lib check 
AC_DEFUN([AC_CHECK_LIB_NOCACHE_HDF5],
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
 
dnl header check
AC_DEFUN([AC_CHECK_HEADER_NOCACHE_HDF5],
[
  AS_TR_SH([ac_check_header_nocache_compile_$1])='no'
  AC_MSG_CHECKING([for $1 with compiler])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <$1>]])],
    [
      AC_MSG_RESULT([yes])
      AS_TR_SH([ac_check_header_nocache_compile_$1])='yes'
    ],
    [
      AC_MSG_RESULT([no])
    ])
  AS_IF([test "$AS_TR_SH([ac_check_header_nocache_compile_$1])" = 'yes'],
  [m4_if([$2], [], [:], [$2])],
  [m4_if([$3], [], [:], [$3])])
])
