---
title: gr2stn()
---

# **gr2stn()**

This function is a grid-to-station interpolator. It creates "station"data result by sampling a gridded data set andinterpolating to a given location. That location may be provided byspecifying a longitude and a latitude, or by providing a stationexpression.

## Syntax

- `gr2stn(`*`grid_expr, stn_expr,`*` <-n>)`\
     or\
  `gr2stn(`*`grid_expr, lon, lat`*`, <-n>)`\

where:

- *`grid_expr`* is a GrADS expressionthat gives a grid result. The interpolation will be done on thisdata. The *`grid_expr`*may be a 2-D grid that varies in X and Y, or a 1-D grid that varies in Z or T.

  *`stn_expr`* is a GrADS expression that gives a station data result. The interpolation will be done to the station *locations*, the station data values are not used.

  *`lon,lat`* may be used instead of *`stn_expr`* to specify the location to which the gridded data will be interpolated (see Usage Note \#3.)

  (Version 2.0.a6 or later) The `-n` option was added to return the nearest neighbor to the station locationinstead of the bi-linear interpolation of the four surrounding grid points.

   

### Usage Notes

1.  The result of the function is station data.
2.  If *`grid_expr`* is a 2-D grid that varies in X and Y, then *`stn_expr`* should also be a 2-D expression that has multiple stations in the lat/lon domain. The result will be a station data set, with values interpolated from *`grid_expr`* to the station locations.
3.  If *`grid_expr`* is a 1-D grid, then only Z or T can be the varying dimension. In this case, *`stn_expr`* should be an expression that has a single location, such as "temp(stid=kdca)". Alternatively, you may provide exact longitude and latitude values.
4.  By default, the interpolation is done bi-linearly within the grid space. No weighting is done to account for real-world coordinate systems. If any of the four grid points around the station location are missing, the result will also be missing.
5.  As of version 2.0.a6, the -n option may be used to return the nearest grid point value instead of the bi-linearly interpolated value.
6.  See the section of the User's Guide on Arbitrary Cross Sections for more information on applications of `gr2stn`.

### Examples

1.  To examine the difference between an analysis (ie, gridded data) andthe original observations, one could:\
    \
    `d t.3-gr2stn(t.1,t.3)`
    where file 1 is gridded data, and file 3 is station data. The result woulddisplay as differences at the station locations.
2.  If one wanted to display the difference calculated in Example 1 as acontour field, one can use the `oacres` function to do a quickanalysis of the station values:\
    \
    `d oacres(t.1,t.3-gr2stn(t.1,t.3)) `
