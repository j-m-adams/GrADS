---
title: sum()
---

# **sum()**

`sum (`*`expr, dim1, dim2, <,tinc>`*` <,-b>)`

Sums the result of *`expr`* over the specified dimension range. If the summing dimension is time, an optional time increment *`tincr`* may be specified.

- *`expr`*`    `- any valid GrADS expression\
  *`dim1`*`    `- the start point for the sum\
  *`dim2`*`    `- the end point for the sum\
  *`tinc`*`    `- optional increment for time summing\
  `-b      `- use exact boundaries\

*`dim1`* and *`dim2`* are standard GrADS dimension expressions whose dimensions must match.

## Usage Notes

1.  The limits and intervals of the summing are set according to the grid coordinates of the default file. If *`dim1`* and *`dim2`* are specified in world coordinates, the coordinates are converted to the nearest integer grid coordinates based on the scaling of the default file.
2.  The end points are given normal weighting, unless the `-b` boundary flag is specified. The boundry flag indicates that the sum should be taken to the exact boundaries specified by *`dim1`* and *`dim2`*, rather than the nearest grid points.
3.  In the Y dimension, when the boundary is beyond the pole, the `asum` function recognizes this and weights appropriately. To calculate an sum without any weighting, use the <a href="gradfuncsumg.html">`sumg`</a> function.

### Examples
