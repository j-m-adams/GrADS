dnl GA_CHECK_XAW : Check for Xaw 
dnl args :             action-if-yes, action-if-no
dnl sets XAW_CFLAGS, XAW_LIBS, and XAW_XLIBS. XAW_XLIBS is set to the 
dnl necessary X library flags determined by AC_PATH_XTRA, or is empty
dnl if all the dependencies are already in XAW_XLIBS, which is the case
dnl when pkgconfig is used
AC_DEFUN([GA_CHECK_XAW],
[
  AC_REQUIRE([AC_PATH_XTRA])

  XAW_LIBS=
  XAW_XLIBS=
  XAW_CFLAGS=
  ac_pkgconfig_xaw=no

  ac_pkgconfig_xaw7=yes
  PKG_CHECK_MODULES([XAW7],[xaw7],,[ac_pkgconfig_xaw7=no])
  ac_save_LDFLAGS=$LDFLAGS
  ac_save_LIBS=$LIBS
  ac_save_CFLAGS=$CFLAGS
  LDFLAGS="$LDFLAGS $X_LIBS"
  LIBS="$LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
  CFLAGS="$CFLAGS $X_CFLAGS"
  ga_xaw_flag=''
  ga_xaw_libs='-lXmu -lXt'
  AC_CHECK_LIB([Xt],[main],
  [ 
    AC_CHECK_LIB([Xmu],[main],
    [
      # we add Xext if found. Not sure which platform needs it
      AC_CHECK_LIB([Xext],[main],
      [
         ga_xaw_libs="$ga_xaw_libs -lXext"
      ])
      # we add Xpm if found, and we don't check for neXtaw if no Xpm
      AC_CHECK_LIB([Xpm],[main],
      [ 
        ga_xaw_libs="$ga_xaw_libs -lXpm"
        AC_CHECK_LIB([neXtaw],[main],
        [ ga_xaw_flag='-lneXtaw'],,
        [])
      ])
      if test z"$ga_xaw_flag" = 'z'; then
        AC_CHECK_LIB([Xaw3d],[main],
        [  ga_xaw_flag='-lXaw3d' ],,
        [])
      fi
      if test z"$ga_xaw_flag" = 'z'; then
        if test $ac_pkgconfig_xaw7 = 'yes'; then
          ac_pkgconfig_xaw=xaw7
          ga_use_xaw=yes
          XAW_LIBS=$XAW7_LIBS
          XAW_CFLAGS=$XAW7_CFLAGS
          XAW_XLIBS=
        else
          AC_CHECK_LIB([Xaw],[main],
          [  ga_xaw_flag='-lXaw' ],,
          [$ga_xaw_libs])
        fi
      fi
      if test z"$ga_xaw_flag" != 'z'; then
        XAW_LIBS="$ga_xaw_flag $ga_xaw_libs"
        XAW_XLIBS="$X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
        ga_use_xaw=yes
      fi
       
    ])
  ])

  CFLAGS=$ac_save_CFLAGS
  LIBS=$ac_save_LIBS
  LDFLAGS=$ac_save_LDFLAGS
  if test "z$ga_use_xaw" = "zyes"; then
     m4_if([$1], [], [:], [$1])
  else
     m4_if([$2], [], [:], [$2])
  fi
  AC_SUBST([XAW_LIBS])
  AC_SUBST([XAW_XLIBS])
  AC_SUBST([XAW_CFLAGS])
])
