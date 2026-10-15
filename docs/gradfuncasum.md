---
title: asum()
---

GrADS Function: asum

# **asum()**

This function takes a sum over an X-Y region. The syntax is:

`asum(`*`expr, xdim1, xdim2, ydim1, ydim2`*`)`

where:

- *`expr`*`    `- any valid GrADS expression\
  *`xdim1`*`   `- starting X dimension expression\
  *`xdim2`*`   `- ending X dimension expression\
  *`ydim1`*`   `- starting Y dimension expression\
  *`ydim2`*`   `- ending Y dimension expression\

For global summing, a shorthand may be used:

- `asum(`*`expr`*`, global)` or\
  `asum(`*`expr`*`, g)`

is the same as

- `asum(`*`expr`*`, lon=0, lon=360, lat=-90, lat=90)`

## Usage Notes

1.  The `asum` function always does its average to the exact boundaries specified, in world coordinates. If the boundaries specified via the dimension expressions do not fall on grid boundaries, then the boundary values are weighted appropriately in the sum.

2.  When grid coordinates are used in the dimensions expressions, then they are converted to world coordinates for the boundary to be determined. This conversion is done using the scaling of the default file. Note that the conversion is done using the outside grid boundary, rather than the grid center. For example:

    `asum(expr,x=1,x=72,y=1,y=46)`

    Here the boundary would be determined by using the grid values 0.5, 72.5, 0.5, and 46.5 which would be converted to world coordinates. If we assume that `x=1` is 0 degrees longitude and `x=72` is 355 degrees longitude, then the averaging boundary would be -2.5 to 357.5 degrees, which would cover the earth. In the Y dimension, when the boundary is beyond the pole, the `asum` function recognizes this and weights appropriately. To calculate a sum without any weighting, use the <a href="gradfuncasumg.html">`asumg`</a> function.

### Examples
