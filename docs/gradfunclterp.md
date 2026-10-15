---
title: lterp
---

lterp

# **lterp**

The lterp function performs bi-linear intepolation between two grids. It is a built-in function as of version 2.0 -- in earlier versions it was implemented as a user defined function. The syntax is:

`lterp(`*`source,dest`*`)`

The data values in the *`source`* expression are interpolated to the grid in the *`dest`* expression. The data values in the *`dest`* expression are ignored. The returned result is on the same grid as *`dest`*.

## Usage Notes

Works only with gridded data. The *`source`* and *`dest`* expressions must have the same varying dimensions, which may be X, Y, or T. Interpolation is not performed in the Z or E dimension. The *`source`* and *`dest`* expressions may vary in 1 or 2 dimensions.

If the domain of *`source`* is larger than the domain of *`dest`*, the returned result will have an expanded grid to cover the requested domain.

For interpolation in the time dimension, you may interpolate (A) between monthly and yearly time axes, or (B) between minute, hourly, and daily time axes.

### Examples

\* This script interpolates a 1-D timeseries of hourly station data to a 3hourly grid\
open hourly_station_data.ctl\
open 3hourly_grid_data.ctl\
set x 1\
set y 1\
set time 00z1jan 00z1feb\
d lterp(s2g1d(var.1(stid=kbwi)),var.2(lon=-77,lat=39))

\* This script interpolates 2-D lat/lon grids\
'open obs.ctl'\
'open model.ctl'\
\* define the destination grid\
'set dfile 1'\
'q file'\
line5 = sublin(result,5)\
ix = subwrd(line5,3)\
iy = subwrd(line5,6)\
'set x 1 'ix\
'set y 1 'iy\
'set t 1'\
'define grid = obs'\
\* define the source grid\
'set dfile 2'\
'q file'\
line5 = sublin(result,5)\
ix = subwrd(line5,3)\
iy = subwrd(line5,6)\
'set x 1 'ix\
'set y 1 'iy\
'set t 1'\
'define data = model'\
\* interpolate model data to obs grid\
'd lterp(data,grid)'
