---
title: set lat\|lon\|lev\|time\|ens
---

# **set lat\|lon\|lev\|time\|ens**

`set lat|lon|lev|time|ens `*`val1 <val2>`*

This set command sets one dimension of the dimension environment usingworld coordinates.

## Usage Notes

When you enter dimensions in grid coordinates, they arealways converted to world coordinates. This conversion requires someknowledge of what scaling is in use for grid to world conversions. Thescaling that is used in all cases (except one) is the scaling of theDEFAULT FILE. The exception is when you supply a dimension expressionwithin a variable specification, which will be covered later.

### Examples

1.  `set lon -180 0` (sets longitude to vary from 180W to 0).\
2.  `set lat 0 90` (sets latitude to vary from the equator to90N)\
3.  `set lev 500` (sets the level to 500mb - a fixed dimension)
4.  `set ens cntrl` (sets the ensemble to cntrl)\
