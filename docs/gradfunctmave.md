---
title: tmave()
---

# **tmave()**

This function does time averaging while applying a mask. The syntax is:

- `tmave(`*`maskexpr,expr,timexpr1,timexpr2`*`)`

where:

- *`maskexpr`*`    ` - the mask expression; when evaluated at a fixed time, it must give a single value\
  *`expr`*`        ` - the expression to be averaged\
  *`timexpr1,2`*`  ` - the limits of the time averaging domain\

## Usage Notes

1.  This function works similarly to the <a href="gradfuncave.html">`ave`</a> function, except for the masking. Using `tmave` is much more efficient than using <a href="gradfuncmaskout.html">`maskout`</a> with <a href="gradfuncave.html">`ave`</a>).
2.  The function loops through the specified time steps, and evaluates *`maskexpr`* at each fixed time. *`maskexpr`* must yeild a single value. If this value is the undefined/missing data value, then *`expr`* for that time is not included in the average.
3.  If *`maskexpr`* is not the undefined data value, it is used as the weight for *`expr`* in the average. So if you define *`maskexpr`* accordingly, you can use the `tmave` function to do weighted time averaging.
4.  The tricky aspect of using `tmave` is setting up *`maskexpr`*. If *`expr`* is a grid with X and/or Y and/or Z varying, then *`maskexpr`* \*MUST\* refer to either a defined variable or a file with only time varying. In general, you have to set up *`maskexpr`* in advance.

### Examples

Say you want to average `slp` over some time range but only when `sst` over some region is above some value. You can do this by:

```text

set x 1
set y 1
set t 1 last
define sstmask = aave(sst,lon=-180,lon=-90,lat=-20,lat=20)
define sstmask = const(maskout(sstmask,sstmask-25.0),1)

```

Now `sstmask` is a time series where the value is 1 when the `sst` areal average is above 25 and undefined when the value is below 25. <a href="gradfuncmaskout.html">`maskout`</a> set the values below 25 to missing; <a href="gradfuncconst.html">`const`</a> set the non-missing values to 1. We can now do our `tmave`:

```text

set lon -180 -90
set lat -20 20
set t 1
d tmave(sstmask,slp,t=1,t=last)

```

The mask could also be written to a file with all dimensions nonvarying except for time. Here is what some of the records in the data descriptor file might look like:

```text

dset 
maskfilename

xdef 1 linear 1 1
ydef 1 linear 1 1
zdef 1 linear 1 1
tdef 100 linear 
....

```
