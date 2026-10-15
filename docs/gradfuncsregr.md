---
title: sregr()
---

# **sregr()**

This function calculates the linear least-squares regression between two variables over an X-Y domain. It returns a single number. The syntax is:

- `sregr(`*`expr1, expr2, xdim1, xdim2, ydim1, ydim2`*`)`

where:

- *`expr1`*`   `- a valid GrADS expression varying in X and Y\
  *`expr2`*`   `- a valid GrADS expression varying in X and Y\
  *`xdim1`*`   `- starting X dimension expression\
  *`xdim2`*`   `- ending X dimension expression\
  *`ydim1`*`   `- starting Y dimension expression\
  *`ydim2`*`   `- ending Y dimension expression\

To do the regression over the global domain, a shorthand may be used:

- `sregr(`*`expr1, expr2`*`, global)` or\
  `sregr(`*`expr1, expr2`*`, g)`

is the same as

- `sregr(`*`expr1, expr2`*`, lon=0, lon=360, lat=-90, lat=90)`

The result from `sregr` is the expected value of the *`expr2`* departure given a 1 unit departure in *`expr1`*.

## Usage Notes

1.  *`expr1`* is the independent variable and *`expr2`* is the dependent variable.

2.  The regression is sensitive to the units of the input expressions. In the examples below, the sensible heat flux (shtfl) is in units of W m^-2 and the surface temperature (tsfc) is in units of K, so the regression coefficient of shtfl on tsfc is in units of W m^-2 K^-1.

3.  `sregr` may be used in conjunction with <a href="gradfunctloop.html">`tloop`</a> or <a href="gradcomddefine.html">`define`</a> to create time series or time/height plots.

4.  `sregr` assumes that the world coordinates are longitude in the X dimension and latitude in the Y dimension, and does weighting in the latitude dimension by the delta of the sin of the latitudes. Weighting is also performed appropriately for unequally spaced grids.

5.  The result of the least squares regression of Y on X is often expressed as a linear equation:

    - `Y = slope * X + intercept`

    where X is the independent variable, Y is the dependent variable, and the slope and intercept are calculated using complicated algebraic formulas. The calculation is simplified if the means are removed. If we define x and y to be the departures from the areal averages of X and Y:

    - `x = X - Xave`\
      `y = Y - Yave`

    then the regression equation becomes:

    - `y = `*`coefficient`*` * x`

    Where

    - *`coefficient`*` = (sum of x*y over area)/(sum of x*x over area)`

    This *`coefficient`* is the output from the `sregr` function. The second example below shows how to construct the regression estimate of Y based on X.

6.  Use the <a href="gradfunctregr.html">`tregr`</a> function to do regression over the time domain.

### Example

1.  This example calculates the expected departure from the mean of the sensible heat flux (shtfl) in the North Pacific given a unit departure from the mean surface temperature (tsfc). The units are W m^-2 K^-1.
```text

set lon 120 250
set lat 15 60
define ivar = tsfc   ;* surface temperautre
define dvar = shtfl  ;* sensible heat flux
set z 1
set t 1 
d sregr(ivar, dvar, lon=120, lon=250, lat=15, lat=60)

```
2.  This example builds on the previous example by calculating the regression estimate of sensible heat flux based on the surface temperature.
```text

define coeff = sregr(ivar, dvar, lon=120, lon=250, lat=15, lat=60)
define dvarave = aave(dvar, lon=120, lon=250, lat=15, lat=60)
define ivarave = aave(ivar, lon=120, lon=250, lat=15, lat=60)
d coeff * (ivar - ivarave) + dvarave

```
