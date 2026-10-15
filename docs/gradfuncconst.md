---
title: const()
---

GrADS Function: const

# **const()**

`const (`*`expr, value,`*` <-u|-a>)`

This is a powerful function that allows the user to change the missing values of a variable, set all the non-missing values of a variable to a constant, or set all possible values of a variable (both valid and missing) to a constant.

- *`expr`*`   `a valid GrADS expression\
  *`value`*`  `a constant, either an integer or floating point value\
  `-u     `all missing data are set to *`value`*; non-missing data are unchanged\
  `-a     `all data are set to *`value`*, both missing and non-missing\

Default behaviour is to set all non-missing data equal to *`value`*; missing data are unchanged\

## Usage Notes

1.  The `const` function operates on both gridded and station data.
2.  If *`value`* is given as an integer, it will still be treated as as floating point.

### Examples

1.  The `const` function assigns a new value to missing data, so that missing data may participate in operations:
    - `const(z, 0, -u)`

2.  The `const` function is used with the <a href="gradcomdsetgxout.html">set gxout linefill</a> graphics output option to define a straight horizontal line:
    - ` set lon -90`\
      `set lat -90 90`\
      `set gxout linefill`\
      `set lev 500`\
      `d const(t, -20);t-273`\

3.  In this example, `const` is used to calculate a daily timeseries of the fraction of the globe convered by precipitation greater than `10mm/day`:

    - ` set lon 0 360`\
      `set lat -90 90`\
      `set t 1 last`\
      `define ones = const(const(maskout(p,p-10),1),0,-u)`\
      `set x 1`\
      `set y 1`\
      `display tloop(aave(ones,lon=0,lon=360,lat=0,lat=360))`\

    Notes: The <a href="gradcomddefine.html">defined</a> variable `"ones"` contains `1` wherever the precip value is greater than `10`, and `0` whever the precip value is less than `10`. This is done via nested functions; first <a href="gradfuncmaskout.html">`maskout`</a> sets all values less than `10` to missing, then `const` sets all non-missing values to `1`, then `const` is used with the `-u` flag to set all the missing data values to `0`. The <a href="gradfuncaave.html">`aave`</a> function calculates an area weighted average. Since we are averaging zeros and ones, the result is the fraction of the area where there are ones. See the <a href="gradfunctloop.html">`tloop`</a> function for a description of how to perform time series of areal averages.
