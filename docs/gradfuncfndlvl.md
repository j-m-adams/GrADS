---
title: fndlvl()
---

# **fndlvl()**

`fndlvl (`*`expr, expr_to_find, lev1, lev2`*`)`

Given two gridded variables, *`expr`* and *`expr_to_find`*, this function finds the first vertical level at which the *`expr_to_find`* value occurs in *`expr`*. *`lev1`* and *`lev2`* specify the range of levels over which to search. The result is a grid of pressure values.

## Usage Notes

1.  The dimensions of *`expr`* and *`expr_to_find`*   must match.

### Examples

1.  This example finds the pressure levels of the 240 degree isotherm:
    - `d fndlvl (t, const(t,240), lev=1000, lev=100)`

2.  The *`expr_to_find`* doesn't have to be a constant; it can change with location. This example returns the pressure level of the tropopause, given the variable `'ttrop'` that contains the temperature at the tropopause:
    - `d fndlvl (t, ttrop, lev=1000, lev=100)`

3.  This example illustrates the limitations of `fndlvl`. It returns the level of the surface pressure variable `psfc` in the coordinate system of `lev`, a GrADS <a href="variable.html">pre-defined variable</a>.

    - `d fndlvl (lev, psfc, lev=1000, lev=100)`

    The display should be exactly the same as `psfc` except in locations where `psfc` is greater than 1000 mb, the maximum value of `lev``. `
