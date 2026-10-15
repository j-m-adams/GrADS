---
title: tregr()
---

# **tregr()**

`tregr (`*`expr1, expr2, tdim1, tdim2`*`)`

This function calculates the least-squares regression between two time-dependent variables.

- *`expr1`*`   `- a valid GrADS expression that varies in time\
  *`expr2`*`   `- a valid GrADS expression that varies in time and may also vary in X and Y\
  *`tdim1`*`   `- starting time dimension expression\
  *`tdim2`*`   `- ending time dimension expression\

The result is a grid that matches the X and Y dimensions of *`expr2`* where each grid point is the temporal regression of *`expr2`* (the dependent variable) onto *`expr1`* (the independent variable). `tregr` gives the expected value of the *`expr2`* departure given a 1 unit departure in *`expr1`*.

## Usage Notes

1.  If both *`expr1`* and *`expr2`* vary only in time, the output is a single value.

2.  The regression is sensitive to the units of the input expressions. In the examples below, the variable SLP is in units of mb and the variable elnino is in units of K, so the regression coefficient of SLP upon elnino is in units of mb per K.

3.  The result of the least squares regression of Y on X is often expressed as a linear equation:

    - `Y = slope * X + intercept`

    where X is the independent variable, Y is the dependent variable, and the slope and intercept are calculated using complicated algebraic formulas. The calculation is simplified if the time means are removed. If we define x and y to be the departures from the time averages of X and Y:

    - `x = X - Xave`\
      `y = Y - Yave`

    then the regression equation becomes:

    - `y = `*`coefficient`*` * x`

    Where

    - *`coefficient`*` = (sum of x*y over time)/(sum of x*x over time)`

    This *`coefficient`* is the output from the `tregr` function. The second example below shows how to construct the regression estimate of Y based on X.

4.  Use the <a href="gradfuncsregr.html">`sregr`</a> function to do regression over the spatial domain.

### Example

1.  This example calculates the expected departure from the slp mean given a unit departure in the defined variable elnino.
    set y 1 set z 1 set t 1 100 define elnino = aave(ts, lon=-160, lon=-80, lat=-20, lat=10) set lon 0 360 set lat -90 90 set z 1 set t 1 d tregr(elnino, slp, t=1, t=100)
2.  This example builds on the previous example by calculating the regression estimate of slp based on the defined variable elnino.
```text

define coeff = tregr(elnino, slp, t=1, t=100)
define slpave = ave(slp, t=1, t=100)
define ninoave = ave(elnino, t=1, t=100)
d coeff * (elnino - ninoave) + slpave

```
