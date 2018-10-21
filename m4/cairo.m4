# Autoconf check that uses pkg-config to look for Cairo Graphics Library.
# Usage:
#   GA_CHECK_LIB_CAIRO(action-if-found, action-if-not-found)

AC_DEFUN([GA_CHECK_LIB_CAIRO],
[

  cairo_pkgconfig=no
  PKG_CHECK_MODULES([CAIRO],[cairo],[cairo_pkgconfig=yes])

  if test $cairo_pkgconfig = 'yes'; then
     m4_if([$1], [], [:], [$1])
  else
     m4_if([$2], [], [:], [$2])
  fi

])

