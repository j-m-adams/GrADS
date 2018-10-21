dnl example of use
dnl AC_CHECK_NETCDF(
dnl   [
dnl       LIBS="$LIBS $NC_LIBS"
dnl       LDFLAGS="$LDFLAGS $NC_LDFLAGS"
dnl       CPPFLAGS="$CPPFLAGS $NC_CFLAGS"
dnl   ],
dnl   [
dnl       echo "*** Use --with-netcdf for the root netcdf directory."
dnl       echo "*** Otherwise use --with-netcdf-include switch for includes directory"
dnl       echo "*** and --with-netcdf-libdir switch for libraries directory."
dnl       AC_MSG_ERROR([netcdf library and netcdf headers are required.])
dnl   ]
dnl )

# Check for the netcdf library.
# AC_CHECK_NETCDF([ACTION-IF-FOUND],[ACTION-IF-NOT-FOUND],[INTERFACE-NR])
# if interface number is given, check for a specific interface
# sets NC_LDFLAGS, NC_LIBS, and, by calling other macros
# NC_CFLAGS and maybe NC_NETCDF_3_CFLAGS
AC_DEFUN([AC_CHECK_NETCDF],
[
  AC_ARG_WITH([netcdf],
            [AS_HELP_STRING([--with-netcdf=ARG],[netcdf directory])],
            [NC_PATH=$withval], 
            [NC_PATH=""])

  AC_ARG_WITH([netcdf_include],
            [AS_HELP_STRING([--with-netcdf-include=ARG],[netcdf include directory])],
            [NC_PATH_INC=$withval], 
            [NC_PATH_INC=""])

  AC_ARG_WITH([netcdf_libdir],
            [AS_HELP_STRING([--with-netcdf-libdir=ARG],[netcdf library directory])],
            [NC_PATH_LIBDIR=$withval], 
            [NC_PATH_LIBDIR=""])

  AS_IF([test "z$NC_PATH" != "z"],
  [
    AS_IF([test "z$NC_PATH_LIBDIR" = "z"],[NC_PATH_LIBDIR="$NC_PATH/lib"])
    AS_IF([test "z$NC_PATH_INC" = "z"],[NC_PATH_INC="$NC_PATH/include"])
  ])

  ac_netcdf_ok='no'
  NC_LIBS=
  NC_LDFLAGS=
  ac_nc_save_LDFLAGS=$LDFLAGS
  ac_nc_save_LIBS=$LIBS
  ac_check_nc_func_checked='ncopen'
  ac_check_nc_interface=
dnl the interface number isn't quoted with "" otherwise a newline 
dnl following the number isn't stripped.
  m4_if([$3],[],[ac_check_nc_interface=2],[ac_check_nc_interface=$3])
  AS_IF([test "z$ac_check_nc_interface" = 'z3'],
    [ac_check_nc_func_checked='nc_open'])
  AS_IF([test "z$NC_PATH_LIBDIR" != "z"],
    [
      NC_LDFLAGS="-L$NC_PATH_LIBDIR"
      LDFLAGS="$LDFLAGS $NC_LDFLAGS"
dnl the autoconf internal cache isn't avoided because we really check for
dnl libnetcdf, other libraries that implement the same api have other names
dnl  AC_LINK_IFELSE([AC_LANG_CALL([],[$ac_check_func_checked])],
      AC_CHECK_LIB([netcdf],[$ac_check_nc_func_checked],
        [
          NC_LIBS='-lnetcdf'
          ac_netcdf_ok='yes'
        ])
    ],
    [
      for ac_netcdf_libdir in "" \
       /usr/lib64 \
       /usr/local/netcdf-${ac_check_nc_interface}/lib64 \
       /opt/netcdf-${ac_check_nc_interface}/lib64 \
       /usr/netcdf-${ac_check_nc_interface}/lib64 \
       /usr/local/lib64/netcdf-${ac_check_nc_interface} \
       /opt/lib64/netcdf-${ac_check_nc_interface} \
       /usr/lib64/netcdf-${ac_check_nc_interface} \
       /usr/local/netcdf/lib64 /opt/netcdf/lib64 \
       /usr/netcdf/lib64 /usr/local/lib64/netcdf /opt/lib64/netcdf \
       /usr/lib64/netcdf \
       /usr/local/netcdf-${ac_check_nc_interface}/lib \
       /opt/netcdf-${ac_check_nc_interface}/lib \
       /usr/netcdf-${ac_check_nc_interface}/lib \
       /usr/local/lib/netcdf-${ac_check_nc_interface} \
       /opt/lib/netcdf-${ac_check_nc_interface} \
       /usr/lib/netcdf-${ac_check_nc_interface} \
       /usr/local/netcdf/lib /opt/netcdf/lib \
       /usr/netcdf/lib /usr/local/lib/netcdf /opt/lib/netcdf \
       /usr/lib/netcdf ; do
        AS_IF([test "z$ac_netcdf_libdir" = 'z'],
          [NC_LDFLAGS=],
          [
            AC_MSG_CHECKING([for netcdf libraries in $ac_netcdf_libdir])
            NC_LDFLAGS="-L$ac_netcdf_libdir"
          ])
        LDFLAGS="$LDFLAGS $NC_LDFLAGS"
        LIBS="$LIBS -lnetcdf"
dnl we have to avoid the autoconf internal cache in that case
        AC_LINK_IFELSE([AC_LANG_CALL([],[$ac_check_nc_func_checked])],
          [
            NC_LIBS='-lnetcdf'
            ac_netcdf_ok='yes'
            AS_IF([test "z$ac_netcdf_libdir" != 'z'],[AC_MSG_RESULT([yes])])
          ],
          [
            AS_IF([test "z$ac_netcdf_libdir" != 'z'],[AC_MSG_RESULT([no])])
          ])
        AS_IF([test $ac_netcdf_ok = 'yes'],[break])
        LDFLAGS=$ac_nc_save_LDFLAGS
        LIBS=$ac_nc_save_LIBS
      done
    ])
  LDFLAGS=$ac_nc_save_LDFLAGS
  LIBS=$ac_nc_save_LIBS

  AC_SUBST([NC_LDFLAGS])
  AC_SUBST([NC_LIBS])
  ac_netcdf_header='no'

  NC_CFLAGS=
  AS_IF([test "z$NC_PATH_INC" != "z"],
    [
      AC_CHECK_NETCDF_HEADER([$NC_PATH_INC],
        [ac_netcdf_header='yes'],
        [ac_netcdf_header='no'],
        [$ac_check_nc_interface])
    ],
    [
      for ac_netcdf_incdir in "" \
       /usr/local/netcdf-${ac_check_nc_interface}/include \
       /opt/netcdf-${ac_check_nc_interface}/include \ 
       /usr/netcdf-${ac_check_nc_interface}/include \
       /usr/local/include/netcdf-${ac_check_nc_interface} \
       /opt/include/netcdf-${ac_check_nc_interface} \
       /usr/include/netcdf-${ac_check_nc_interface} \
       /usr/local/netcdf/include \
       /opt/netcdf/include /usr/netcdf/include /usr/local/include/netcdf \
       /opt/include/netcdf /usr/include/netcdf ; do
        AC_MSG_NOTICE([searching netcdf includes in $ac_netcdf_incdir])
        AC_CHECK_NETCDF_HEADER([$ac_netcdf_incdir],[ac_netcdf_header='yes'],
          [ac_netcdf_header='no'],[$ac_check_nc_interface])
        AS_IF([test $ac_netcdf_header = 'yes'],
        [
           AS_IF([test "z$ac_netcdf_incdir" != "z"],
             [NC_CFLAGS="-I$ac_nc_include_dir"])
           break
        ])
      done
    ])
  AC_SUBST([NC_CFLAGS])

  AS_IF([test "$ac_netcdf_ok" = 'no' -o "$ac_netcdf_header" = 'no'],
    [m4_if([$2], [], [:], [$2])],
    [m4_if([$1], [], [:], [$1])])
])
