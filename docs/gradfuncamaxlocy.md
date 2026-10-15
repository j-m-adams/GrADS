---
title: amaxlocy()
---

# **amaxlocy()**

This function returns the Y location of maximum grid value over a spatial domain. The syntax is:

`amaxlocy(`*`expr, xdim1, xdim2, ydim1, ydim2`*`)`

where:

- *`expr`*`    `any valid GrADS grid expression\
  *`xdim1`*`   `starting X or LON dimension expression\
  *`xdim2`*`   `ending X or LON dimension expression\
  *`ydim1`*`   `starting Y or LAT dimension expression\
  *`ydim2`*`   `ending Y or LAT dimension expression\

For the Y location of the global maximum, a shorthand may be used:

- `amaxlocy(`*`expr`*`, global)` or\
  `amaxlocy(`*`expr`*`, g)`

is the same as

- `amaxlocy(`*`expr`*`, lon=0, lon=360, lat=-90, lat=90)`

## Usage Notes

1.  This function will only work with GrADS version 2.0.2 or later.
2.  This function is more efficient that using nested <a href="gradfuncmaxloc.html">`maxloc`</a> and <a href="gradfuncmax.html">`max`</a> functions.
3.  Related function <a href="gradfuncamaxlocx.html">`amaxlocx`</a> will return the X location of the maximum grid value.
4.  Use the <a href="gradfuncamax.html">`amax`</a> function to retrieve the maximum value over the grid.
5.  If more than one grid box contains the maximum value, the location returned will be the first one encountered as the grid is scanned. The grid is scanned by rows from south to north, and each row is scanned from west to east.
6.  A similar set of functions exists for finding the minimum over an area: <a href="gradfuncamin.html">`amin`</a>, <a href="gradfuncaminlocx.html">`aminlocx`</a>, and <a href="gradfuncaminlocy.html">`aminlocy`</a>.

### Examples

Please see the documentation page for <a href="gradfuncamax.html">`amax`</a>.
