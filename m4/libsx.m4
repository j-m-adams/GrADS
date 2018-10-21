dnl GA_CHECK_LIBSX : Checks whether GrADS can be built with libsx
dnl  		      	enabled.
dnl  args : 		action-if-yes, action-if-no
AC_DEFUN([GA_CHECK_LIBSX],
[
# Check libs and headers for GUI widgets
  GA_LIBSX_LIBS=
  ac_save_LDFLAGS=$LDFLAGS
  ac_save_LIBS=$LIBS
  ac_save_CFLAGS=$CFLAGS

  GA_CHECK_XAW([ga_xaw_found='yes'],[ga_xaw_found='no'])

  LDFLAGS="$LDFLAGS -L$ga_supplib_dir/lib $X_LIBS"
  CFLAGS="$CFLAGS -I$ga_supplib_dir/include/libsx"
  ga_use_libsx='no'
  ga_libsx_header='no'
  ga_libsx_freq_header='no'

  if test "z$ga_xaw_found" = "zyes"; then
    LIBS="$LIBS $XAW_LIBS $XAW_XLIBS"
    CFLAGS="$CFLAGS $X_CFLAGS $XAW_CFLAGS"
  
    AC_CHECK_HEADER([libsx.h],
    [ AC_CHECK_HEADER([freq.h],
      [
         ga_libsx_freq_header='yes'
      ])
      ga_libsx_header='yes'
    ])
  
    if test "z$ga_libsx_header" = "zyes"; then
      if test "z$ga_libsx_freq_header" = "zyes"; then 
        AC_CHECK_LIB([freq],[main],
        [ AC_CHECK_LIB([sx],[GetFile],
          [ ga_use_libsx='freq'
            GA_LIBSX_LIBS="-lsx -lfreq $XAW_LIBS"
          ])
        ])
      fi
      if test "z$ga_use_libsx" = "zno"; then
         AC_CHECK_LIB([sx],[GetFile],
         [  ga_use_libsx='yes'
            GA_LIBSX_LIBS="-lsx $XAW_LIBS"
         ])
      fi
      if test "z$ga_use_libsx" != "zno"; then
         AC_CHECK_FUNCS([SimpleGetFile])
         ga_getfile_short_prototype=no
         AC_MSG_CHECKING([if GetFile has a short prototype])
         AC_LANG_PUSH(C)
         if test "z$ga_use_libsx" = "zfreq"; then
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <libsx.h>
#include <freq.h>]],
[[GetFile("/path/to/file")]])],[ga_getfile_short_prototype=yes])
         else
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <libsx.h>]],
[[GetFile("/path/to/file")]])],[ga_getfile_short_prototype=yes])
         fi
         if test $ga_getfile_short_prototype = 'yes'; then
           AC_DEFINE([GETFILE_SHORT_PROTOTYPE],[],[Define if GetFile has a short prototype])
           AC_MSG_RESULT([yes])
         else
           AC_MSG_RESULT([no])
         fi
         AC_LANG_POP
      fi
    fi
  fi

  if test "z$ga_use_libsx" = "zfreq" ; then
      m4_if([$1], [], [:], [$1])
  else
      if test "z$ga_use_libsx" = "zyes" ; then
         m4_if([$2], [], [:], [$2])
      else
         m4_if([$3], [], [:], [$3])
      fi
  fi
  AC_SUBST([GA_LIBSX_LIBS])
  CFLAGS=$ac_save_CFLAGS
  LIBS=$ac_save_LIBS
  LDFLAGS=$ac_save_LDFLAGS
])
