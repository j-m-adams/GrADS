dnl AC_CHECK_HDF : Check for hdf4
dnl args :             action-if-yes, action-if-no
AC_DEFUN([AC_CHECK_HDF4],
[
  AC_ARG_WITH([hdf4],
            [AS_HELP_STRING([--with-hdf4=ARG],[hdf4 directory])],
            [HDF4_PATH=$withval], 
            [HDF4_PATH=""])

  AC_ARG_WITH([hdf4_include],
            [AS_HELP_STRING([--with-hdf4-include=ARG],[hdf 4 include directory])],
            [HDF4_PATH_INC=$withval],
            [HDF4_PATH_INC=""])

  AC_ARG_WITH([hdf4_libdir],
            [AS_HELP_STRING([--with-hdf4-libdir=ARG],[hdf 4 library directory])],
            [HDF4_PATH_LIBDIR=$withval], 
            [HDF4_PATH_LIBDIR=""])

  dnl This is a very common location for the hdf4 code. jhrg 10/11/05
dnl  AS_IF([test -d /usr/local/hdf], [HDF4_PATH="/usr/local/hdf"])
      
  AS_IF([test "z$HDF4_PATH" != "z"],
  [
    AS_IF([test "z$HDF4_PATH_LIBDIR" = "z"],
      [HDF4_PATH_LIBDIR="$HDF4_PATH/lib"])  
    AS_IF([test "z$HDF4_PATH_INC" = "z"],
      [HDF4_PATH_INC="$HDF4_PATH/include"])  
  ])
  
  
  ac_hdf4_lib_ok='no'
  ac_hdf4_save_LDFLAGS=$LDFLAGS
  HDF4_LIBS=
  AS_IF([test "z$HDF4_PATH_LIBDIR" != "z"],
    [
      HDF4_LDFLAGS="-L$HDF4_PATH_LIBDIR"
      LDFLAGS="$LDFLAGS $HDF4_LDFLAGS"
      AC_CHECK_HDF4_LIB([ac_hdf4_lib_ok='yes'])
    ],
    [
      for ac_hdf4_libdir in "" /usr/lib64/hdf /usr/lib64 \
       /usr/local/hdf4.2r1/lib64 /opt/hdf4.2r1/lib64 \ 
       /usr/hdf4.2r1/lib64 /usr/local/lib64/hdf4.2r1 /opt/lib64/hdf4.2r1 \
       /usr/lib64/hdf4.2r1 /usr/local/hdf/lib64/ /opt/hdf/lib64 /usr/hdf/lib64 \
       /usr/local/lib64/hdf /opt/lib64/hdf \
       /usr/local/hdf4.2r1/lib /opt/hdf4.2r1/lib \ 
       /usr/hdf4.2r1/lib /usr/local/lib/hdf4.2r1 /opt/lib/hdf4.2r1 \
       /usr/lib/hdf4.2r1 /usr/local/hdf/lib/ /opt/hdf/lib /usr/hdf/lib \
       /usr/local/lib/hdf /opt/lib/hdf /usr/lib/hdf ; do
        AS_IF([test "z$ac_hdf4_libdir" = 'z'],
           [HDF4_LDFLAGS=],
           [
             AC_MSG_NOTICE([searching hdf libraries in $ac_hdf4_libdir])
             HDF4_LDFLAGS="-L$ac_hdf4_libdir"
           ])
        LDFLAGS="$LDFLAGS $HDF4_LDFLAGS" 
        AC_CHECK_HDF4_LIB([ac_hdf4_lib_ok='yes'])
        AS_IF([test $ac_hdf4_lib_ok = 'yes'],[break])
        LDFLAGS=$ac_hdf4_save_LDFLAGS
      done
    ])
  LDFLAGS=$ac_hdf4_save_LDFLAGS
  
  ac_hdf4_h='no'
  HDF4_CFLAGS=
  ac_hdf4_save_CPPFLAGS=$CPPFLAGS
  AS_IF([test "z$HDF4_PATH_INC" != "z"],
    [
       HDF4_CFLAGS="-I$HDF4_PATH_INC"
       CPPFLAGS="$CPPFLAGS $HDF4_CFLAGS"
       AC_CHECK_HEADER_NOCACHE_HDF4([mfhdf.h],[ac_hdf4_h='yes'])
    ],
    [
      for ac_hdf4_incdir in "" /usr/include/hdf \
       /usr/local/hdf4.2r1/include /opt/hdf4.2r1/include \ 
       /usr/hdf4.2r1/include /usr/local/include/hdf4.2r1 \
       /opt/include/hdf4.2r1 /usr/include/hdf4.2r1 /usr/local/hdf/include \
       /opt/hdf/include /usr/hdf/include /usr/local/include/hdf \
       /opt/include/hdf ; do
        AS_IF([test "z$ac_hdf4_incdir" = 'z'],
           [HDF4_CFLAGS=],
           [
             AC_MSG_NOTICE([searching hdf includes in $ac_hdf4_incdir])
             HDF4_CFLAGS="-I$ac_hdf4_incdir"
           ])
        CPPFLAGS="$CPPFLAGS $HDF4_CFLAGS" 
        AC_CHECK_HEADER_NOCACHE_HDF4([mfhdf.h],[ac_hdf4_h='yes'])
        AS_IF([test $ac_hdf4_h = 'yes'],[break])
        CPPFLAGS=$ac_hdf4_save_CPPFLAGS
      done
    ])
  CPPFLAGS=$ac_hdf4_save_CPPFLAGS
  
  AS_IF([test "$ac_hdf4_h" = 'yes' -a "$ac_hdf4_lib_ok" = 'yes'],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])

  AC_SUBST([HDF4_LIBS])
  AC_SUBST([HDF4_CFLAGS])
  AC_SUBST([HDF4_LDFLAGS])
])

dnl check for the netcdf 2 interface provided by hdf
dnl it defines the C preprocessor macro HDF_NETCDF_NAME(name) which
dnl prepends sd_ to name if needed and otherwise keep the name
dnl as is.
dnl
dnl args            action-if-found,
dnl                 action-if-found with sd_ appended to netcdf symbols,
dnl                 action-if-no-found
dnl
dnl in case it is detected that sd_ should be appended, the C preprocessor
dnl symbol HDF_HAVE_NETCDF is defined.

AC_DEFUN([AC_CHECK_HDF4_NETCDF],
[
  ac_hdf4_netcdf_lib='no'
  ac_hdf4_sd_netcdf_lib='no'
  ac_hdf4_netcdf_h='no'
  AC_CHECK_HDF4([
    ac_hdf4_netcdf_save_LDFLAGS=$LDFLAGS
    ac_hdf4_netcdf_save_LIBS=$LIBS
    LIBS="$LIBS $HDF4_LIBS"
    LDFLAGS="$LDFLAGS $HDF4_LDFLAGS"
    AC_MSG_CHECKING([for sd_ncopen])
    AC_LINK_IFELSE([AC_LANG_CALL([],[sd_ncopen])],
      [
        AC_MSG_RESULT([yes])
        ac_hdf4_sd_netcdf_lib='yes'
      ],
      [
        AC_MSG_RESULT([no])
        ac_hdf4_sd_netcdf_lib='no'
      ])
    AS_IF([test "$ac_hdf4_sd_netcdf_lib" = 'no'],
    [
      AC_MSG_CHECKING([for ncopen with hdf link flags])
      AC_LINK_IFELSE([AC_LANG_CALL([],[ncopen])],
      [
        AC_MSG_RESULT([yes])
        ac_hdf4_netcdf_lib='yes'
      ],
      [
        AC_MSG_RESULT([no])
        ac_hdf4_netcdf_lib='no'
      ])
    ])
    LDFLAGS=$ac_hdf4_netcdf_save_LDFLAGS
    LIBS=$ac_hdf4_netcdf_save_LIBS

    ac_hdf4_netcdf_save_CPPFLAGS=$CPPFLAGS
dnl not needed anymore
dnl    ac_hdf4_netcdf_save_NC_CFLAGS=$NC_CFLAGS
    CPPFLAGS="$CPPFLAGS $HDF4_CFLAGS"
    AC_CHECK_HEADERS([hdf4_netcdf.h],[ac_hdf4_netcdf_h='yes'])
    AC_CHECK_NETCDF_HEADER([],[ac_hdf4_netcdf_h='yes'])
    CPPFLAGS=$ac_hdf4_netcdf_save_CPPFLAGS
dnl    NC_CFLAGS=$ac_hdf4_netcdf_save_NC_CFLAG
  ])

  AH_TEMPLATE([HDF_NETCDF_NAME],[A macro that append sd_ to netcdf symbols if needed])
  AS_IF([test $ac_hdf4_netcdf_h = 'yes' -a $ac_hdf4_sd_netcdf_lib = 'yes'],
  [
     AC_DEFINE([HDF_HAVE_NETCDF],[],[Define if hdf prefixes netcdf symbols by sd])
     AC_DEFINE([HDF_NETCDF_NAME(name)], [sd_ ## name])
     m4_if([$2], [], [:], [$2])
  ],
  [
    AC_DEFINE([HDF_NETCDF_NAME(name)], [name])
    AS_IF([test $ac_hdf4_netcdf_h = 'yes' -a $ac_hdf4_netcdf_lib = 'yes'],
    [m4_if([$1], [], [:], [$1])],
    [m4_if([$3], [], [:], [$3])])
  ])
])

AC_DEFUN([AC_CHECK_HDF4_LIB],
[
  HDF4_LIBS=
  ac_hdf4_save_LIBS=$LIBS
  AC_CHECK_LIB_NOCACHE_HDF4([sz], [SZ_BufftoBuffCompress],
  [
      LIBS="$LIBS -lsz"
      HDF4_LIBS='-lsz'
  ])

dnl -lsz is not required because due to licencing it may not be present
dnl nor required everywhere
  ac_hdf4_lib='no'
  AC_CHECK_LIB_NOCACHE_HDF4([z],[deflate],
  [ AC_CHECK_LIB_NOCACHE_HDF4([jpeg],[jpeg_start_compress],
    [ AC_CHECK_LIB_NOCACHE_HDF4([df],[Hopen],
      [ AC_CHECK_LIB_NOCACHE_HDF4([mfhdf],[SDstart],
        [ ac_hdf4_lib="yes"
          HDF4_LIBS="-lmfhdf -ldf -ljpeg -lz $HDF4_LIBS"
        ],[],[-ldf -ljpeg -lz])
      ],[],[-ljpeg -lz])
    ])
  ])
  LIBS=$ac_hdf4_save_LIBS
  
  AS_IF([test "$ac_hdf4_lib" = 'yes'],
  [m4_if([$1], [], [:], [$1])],
  [m4_if([$2], [], [:], [$2])])
])

AC_DEFUN([AC_CHECK_LIB_NOCACHE_HDF4],
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
 
AC_DEFUN([AC_CHECK_HEADER_NOCACHE_HDF4],
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
