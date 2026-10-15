---
title: amax()
---

# **amax()**

This function returns an area maximum -- the maximum value in a grid spanning an X-Y region. The syntax is:

`amax(`*`expr, xdim1, xdim2, ydim1, ydim2`*`)`

where:

- *`expr`*`    `any valid GrADS grid expression\
  *`xdim1`*`   `starting X or LON dimension expression\
  *`xdim2`*`   `ending X or LON dimension expression\
  *`ydim1`*`   `starting Y or LAT dimension expression\
  *`ydim2`*`   `ending Y or LAT dimension expression\

For global maximum, a shorthand may be used:

- `amax(`*`expr`*`, global)` or\
  `amax(`*`expr`*`, g)`

is the same as

- `amax(`*`expr`*`, lon=0, lon=360, lat=-90, lat=90)`

## Usage Notes

1.  This function will only work with GrADS version 2.0.2 or later.
2.  This function is more efficient that using nested <a href="gradfuncmax.html">`max`</a> functions.
3.  Related functions <a href="gradfuncamaxlocx.html">`amaxlocx`</a> and <a href="gradfuncamaxlocy.html">`amaxlocy`</a> will return the grid location (X or Y) of the maximum value. If more than one grid box contains the maximum value, the location returned will be the first one encountered as the grid is scanned. The grid is scanned by rows from south to north, and each row is scanned from west to east.
4.  A similar set of functions exists for finding the minimum over an area: <a href="gradfuncamin.html">`amin`</a>, <a href="gradfuncaminlocx.html">`aminlocx`</a>, and <a href="gradfuncaminlocy.html">`aminlocy`</a>.

### Examples

1.  Get the maximum value of the variable `ps` over a specified grid domain:\
    `d amax(ps,x=10,x=120,y=15,y=45)`\
    \
2.  Get the maximum value of the variable `sstanom` over the nino3.4 domain:\
    `d amax(sstanom,lon=-170,lon=-120,lat=-5,lat=5)`\
    \
3.  Get the maximum value of the variable `slp` over the global domain, and also get the grid location of that minimum. Check results.\
```text

ga-> d amax(slp,g)
Result value = 105732 
ga-> d amaxlocx(slp,g)
Result value = 168 
ga-> d amaxlocy(slp,g) 
Result value = 13 
ga-> set x 168
LON set to 83.5 83.5 
ga-> set y 13
LAT set to -84 -84 
ga-> d slp
Result value = 105732 

  
```
