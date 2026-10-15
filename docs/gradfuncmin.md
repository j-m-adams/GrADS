---
title: min()
---

# **min()**

`min(`*`expr, dim1, dim2 <,tinc>`*`)`

Returns the minimum of *`expr`* over the specified dimension range. If the specified dimension is time, an optional time increment *`tincr`* may be specified.

- *`expr`*`    `- any valid GrADS expression\
  *`dim1`*`    `- the starting dimension expression\
  *`dim2`*`    `- the ending dimension expression\
  *`tinc`*`    `- optional time increment\

*`dim1`* and *`dim2`* are standard GrADS dimension expressions whose dimensions must match.

## Usage Notes

1.  Related functions are: <a href="gradfuncminloc.html">`minloc`</a>, <a href="gradfuncmax.html">`max`</a>, and <a href="gradfuncmaxloc.html">`maxloc`</a>.\
    These functions will only work with GrADS version 1.8 or later.

### Examples
