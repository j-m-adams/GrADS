---
title: oacres()
---

# **oacres()**

`oacres (`*`gexpr, sexpr <,radii>`*`)`

A Cressman objective analysis is performed on the station data to arrive at a gridded result that represents the station data.

- *`gexpr`*`   `a valid grid expression\
  *`sexpr`*`   `a valid station data expression\
  *`radii`*`   `optional radii of influence. Defaults are: 10,7,4,2,1\

The Cressman Analysis scheme is described in a 1959 paper in Monthly Weather Review. Multiple passes are made through the grid with increasingly smaller radii of influence. At each pass, a new value is calculated for each grid point based on a correction factor that is determined by looking at each station within the radius of influence.

For each such station, an error is defined as the difference between the station value and a value arrived by interpolation from the grid to that station. The correction factor is based on a distance weighted formula applied to all such errors within the radius of influence. The correction factors are applied to each grid point before the next pass is made.

Any grid boxes that do not have stations within the third specified radius are set to the missing data value.

## Usage Notes

1.  The `oacres` function can be quite slow to execute, depending on grid and station data density.
2.  The actual values of the gridded expression are ignored, but the grid itself is used as a template to perform the analysis. The scaling of the grid must be linear in lat-lon.
3.  The Cressman Analysis scheme can be unstable if the grid density is substantially higher than the station data density (ie, far more grid points than station data points). In such cases, the analysis can produce extrema in the grid values that are not realistic. It is thus suggested that you examine the results of `oacres` and compare them to the station data to insure they meet your needs.
4.  Objective analysis is a complex topic, and many schemes for doing it have been developed over the years. The `oacres` function is more of a quick-look function rather than a rigorous analysis scheme. If you have specific analysis requirements, consider doing your objective analysis outside of GrADS with a <a href="udp.html">user-defined plug-in function</a>.
5.  See the related function <a href="gradfuncoabin.html">`oabin`</a>.

### Examples

1.  The simplest case:
    `oacres(ts,ts.2)`
2.  To specify your own radii of influence:
    `oacres(ts,ts.2,12,8,5,4,3,2,1)`
