# Check for the netcdf header.
# AC_CHECK_NETCDF_HEADER([INCLUDE-DIR],[ACTION-IF-FOUND],
# [ACTION-IF-NOT-FOUND],[INTERFACE-NR])
# if interface number is given, check for a specific interface
# sets maybe NC_NETCDF_3_CPPFLAG
AC_DEFUN([AC_CHECK_NETCDF_HEADER],
[
  ac_netcdf_h='no'
  ac_netcdf_h_compile='no'
  ac_netcdf_h_preproc='no'
  ac_nc_include_dir=
  ac_nc_header_interface=
  
  ac_nc_save_CPPFLAGS=$CPPFLAGS
  m4_if([$1],[],[:],[
    ac_nc_include_dir="$1"
    AS_IF([test "z$ac_nc_include_dir" != "z"],
       [CPPFLAGS="$CPPFLAGS -I$ac_nc_include_dir"])
  ])
  m4_if([$4],[],[:],[ac_nc_header_interface=$4])
dnl dont use AC_CHECK_HEADERS to avoid autoconf internal caching
  AC_MSG_CHECKING([for netcdf.h with compiler])
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <netcdf.h>]])],
    [
      AC_MSG_RESULT([yes])
      ac_netcdf_h_compile='yes'
    ],
    [
      AC_MSG_RESULT([no])
      ac_netcdf_h_compile='no'
    ])
    AC_MSG_CHECKING([for netcdf.h with preprocessor])
    AC_PREPROC_IFELSE([AC_LANG_SOURCE([[#include <netcdf.h>]])],
    [
      AC_MSG_RESULT([yes])
      ac_netcdf_h_preproc='yes'
    ],
    [
      AC_MSG_RESULT([no])
      ac_netcdf_h_preproc='no'
    ])
  CPPFLAGS="$ac_nc_save_CPPFLAGS"
  AS_IF([test $ac_netcdf_h_compile = 'yes'],
    [ac_netcdf_h='yes'
    AS_IF([test "z$ac_nc_header_interface" = 'z3'],
      [AC_CHECK_NETCDF_3_HEADER([$1],
         [ac_netcdf_h='yes'],[ac_netcdf_h='no'])])
    ])

  AS_IF([test "$ac_netcdf_h" = 'yes'],
    [
      m4_if([$2], [], [:], [$2])
    ],
    [m4_if([$3], [], [:], [$3])])
])

AC_DEFUN([AC_CHECK_NETCDF_3_HEADER],
[
  NC_NETCDF_3_CPPFLAG=
  ac_check_netcdf_3_include=
  ac_check_netcdf_3_header='no'
  ac_nc_save_CPPFLAGS=$CPPFLAGS
  AC_MSG_CHECKING([for netcdf 3 interface])
  m4_if([$1],[],[:],[
    ac_check_netcdf_3_include="$1"
  ])
  AS_IF([test "z$ac_check_netcdf_3_include" != "z"],
    [CPPFLAGS="$CPPFLAGS -I$ac_check_netcdf_3_include"])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <netcdf.h>]],
    [[int status;
int ncid;
char vernum;
status = nc_open("foo.nc", 0, &ncid);
vernum = *nc_inq_libvers();]])],
    [
      AS_IF([test "z$ac_check_netcdf_3_include" != "z"],
        [NC_NETCDF_3_CPPFLAG="-I$ac_check_netcdf_3_include"])
      ac_check_netcdf_3_header='yes'
    ],[ac_check_netcdf_3_header='no'])
  CPPFLAGS=$ac_nc_save_CPPFLAGS
  AS_IF([test "$ac_check_netcdf_3_header" = 'yes'],
    [
      AC_MSG_RESULT([yes])
      m4_if([$2], [], [:], [$2])
    ],
    [
      AC_MSG_RESULT([no])
      m4_if([$3], [], [:], [$3])
    ])
  
  AC_SUBST([NC_NETCDF_3_CPPFLAG])
])
