---
title: amin()
---

# **amin()**

This function returns an area minimum -- the minimum value in a grid spanning an X-Y region. The syntax is:

`amin(`*`expr, xdim1, xdim2, ydim1, ydim2`*`)`

where:

- *`expr`*`    `any valid GrADS grid expression\
  *`xdim1`*`   `starting X or LON dimension expression\
  *`xdim2`*`   `ending X or LON dimension expression\
  *`ydim1`*`   `starting Y or LAT dimension expression\
  *`ydim2`*`   `ending Y or LAT dimension expression\

For global minimum, a shorthand may be used:

- `amin(`*`expr`*`, global)` or\
  `amin(`*`expr`*`, g)`

is the same as

- `amin(`*`expr`*`, lon=0, lon=360, lat=-90, lat=90)`

## Usage Notes

1.  This function will only work with GrADS version 2.0.2 or later.
2.  This function is more efficient that using nested <a href="gradfuncmin.html">`min`</a> functions.
3.  Related functions <a href="gradfuncaminlocx.html">`aminlocx`</a> and <a href="gradfuncaminlocy.html">`aminlocy`</a> will return the grid location (X or Y) of the minimum value. If more than one grid box contains the minimum value, the location returned will be the first one encountered as the grid is scanned. The grid is scanned by rows from south to north, and each row is scanned from west to east.
4.  A similar set of functions exists for finding the maximum over an area: <a href="gradfuncamax.html">`amax`</a>, <a href="gradfuncamaxlocx.html">`amaxlocx`</a>, and <a href="gradfuncamaxlocy.html">`amaxlocy`</a>.

### Examples

1.  Get the minimum value of the variable `ps` over a specified grid domain:\
    `d amin(ps,x=10,x=120,y=15,y=45)`\
    \
2.  Get the minimum value of the variable `sstanom` over the nino3.4 domain:\
    `d amin(sstanom,lon=-170,lon=-120,lat=-5,lat=5)`\
    \
3.  Get the minimum value of the variable `slp` over the global domain, and also get the grid location of that minimum. Check results.\
```text

ga-> d amin(slp,g)     
Result value = 94569 
ga-> d aminlocx(slp,g)
Result value = 523 
ga-> d aminlocy(slp,g)
Result value = 51 
ga-> set x 523
LON set to 261 261 
ga-> set y 51
LAT set to -65 -65 
ga-> d slp
Result value = 94569 
  
```
