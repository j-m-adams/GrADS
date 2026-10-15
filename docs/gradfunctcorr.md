---
title: tcorr()
---

# **tcorr()**

`tcorr (`*`expr1, expr2, tdim1, tdim2`*`)`

This function produces a spatial map of temporal correlation coefficients.

- *`expr1`*`   `- a valid GrADS expression that varies in time\
  *`expr2`*`   `- a valid GrADS expression that varies in time and may also vary in X and Y\
  *`tdim1`*`   `- starting time dimension expression\
  *`tdim2`*`   `- ending time dimension expression\

The *`expr1`* time series is correlated to the time series at each grid point in *`expr2`*. The result is a grid of correlation coefficients that matches the X and Y dimensions of *`expr2`*.

## Usage Notes

1.  If both *`expr1`* and *`expr2`* vary only in time, the output is a single value.
2.  Use the <a href="gradfuncscorr.html">`scorr`</a> function to do correlation over the spatial domain.

### Example

1.  This example calculates the temporal correlation between sea level pressure and the defined variable elnino, an areal average of surface temperature in the equatorial Pacific.
```text

set x 1
set y 1 
set z 1 
set t 1 100
define elnino = aave(ts,lon=-160,lon=-80,lat=-10,lat=10)
set lon -180 180
set lat -90 90
set z 1
set t 1
d tcorr(elnino, slp, t=1, t=100)

```
