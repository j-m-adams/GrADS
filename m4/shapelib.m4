dnl GA_CHECK_LIB_SHP : Checks whether GrADS can be built with shapefile interface
dnl  args : 		action-if-yes, action-if-no
AC_DEFUN([GA_CHECK_LIB_SHP],
[
  ga_check_shp="no"
  AC_CHECK_HEADER(shapefil.h,
  [ AC_CHECK_LIB(shp, main, [
      ga_check_shp="yes"
      SHP_LIBS="-lshp"
    ])
  ])
  if test $ga_check_shp = "yes" ; then
     m4_if([$1], [], [:], [$1])
  else
     m4_if([$2], [], [:], [$2])
  fi

  AC_SUBST([SHP_LIBS])
])
