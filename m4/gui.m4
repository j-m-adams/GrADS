dnl GA_CHECK_GUI : Checks whether GrADS can be built with GUI features 
dnl  		      	enabled.
dnl  args : 		action-if-yes, action-if-no
AC_DEFUN([GA_CHECK_GUI],
[
  # Check libs and headers for GUI widgets
  ga_check_gui="no"
  GA_SET_FLAGS([X11/neXtaw libsx],[$X_CFLAGS], [$X_LIBS],[$X_PRE_LIBS -lX11 $X_EXTRA_LIBS])
dnl  GA_SET_FLAGS([X11/neXtaw libsx])
  AC_CHECK_LIB(Xext, main, [gui_libs_Xext="-lXext"])  
  AC_CHECK_LIB(Xt, main,
    [ AC_CHECK_LIB(Xmu,main,
      [ AC_CHECK_LIB(Xpm, main,
        [ AC_CHECK_LIB(Xaw, main,
          [ AC_CHECK_LIB(sx, main,
            [ ga_check_gui="yes" ])
          ])
        ])
      ])
    ])
  GA_UNSET_FLAGS
  if test $ga_check_gui = "yes" ; then
    $1 
    true #dummy command
  else
    $2
    true #dummy command
  fi

])
