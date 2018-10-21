dnl GA_CHECK_LIB_SHP : Checks whether GrADS can be built with shapefile interface
dnl  args : 		action-if-yes, action-if-no
AC_DEFUN([GA_CHECK_LIB_GADAP],
[
  ga_check_gadap="no"
  AC_CHECK_HEADER(gadap.h,
  [ AC_CHECK_LIB(gadap, main, [
      ga_check_gadap="yes"
      GADAP_LIBS="-lgadap"
    ])
  ])
  if test $ga_check_gadap = "yes" ; then
     m4_if([$1], [], [:], [$1])
  else
     m4_if([$2], [], [:], [$2])
  fi

  AC_SUBST([GADAP_LIBS])
])
