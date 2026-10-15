dnl GA_CHECK_LIB_GRIB2 : Checks whether GrADS can be built with grib2 interface
dnl  args : 		action-if-yes, action-if-no
AC_DEFUN([GA_CHECK_LIB_GRIB2],
[
  ga_check_grib2="no"
  AC_CHECK_HEADER(grib2.h,
  [ AC_CHECK_LIB([grib2c], [main],
      [ga_grib2_library="grib2c"],
      [AC_CHECK_LIB([g2c], [main], [ga_grib2_library="g2c"],
                    [ga_grib2_library=""])])
    AS_IF([test "x$ga_grib2_library" != "x"], [
      AC_CHECK_LIB([png], [png_create_read_struct], [
      AC_CHECK_LIB([z], [compress], [
      AC_CHECK_LIB([jasper], [jas_init], [
        ga_check_grib2="yes"
        G2_LIBS="-l$ga_grib2_library -ljasper -lpng -lz"
      ])
      ])
      ])
    ])
  ])
  if test $ga_check_grib2 = "yes" ; then
     m4_if([$1], [], [:], [$1])
  else
     m4_if([$2], [], [:], [$2])
  fi

  AC_SUBST([G2_LIBS])
])
