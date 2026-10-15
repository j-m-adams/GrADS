---
title: tloop
---

# **tloop**

`tloop(`*`expr`*`)`

When time is a varying dimension in the dimension environment, the `tloop` function evaluates the *`expr`* at fixed times, then reconstructs the time series to obtain a final result that is time varying. The `tloop` function is required due to the implementation of the GrADS expression evaluation rules, and the implementation of certain other functions. The `tloop` function can also improve performance for certain calculations.

The `tloop` function is provided as a way to obtain time series from functions that themselves are not implemented to be able to operate when time is a varying dimension. See the examples below.

## Usage Notes

1.  The `tloop` function loops through time based on the time increment of the default file; it is thus important to have the default file set appropriately.
2.  The `tloop` function and the <a href="gradcomddefine.html">`define`</a> command work very similarly. In many cases, the `define` command can be used to obtain the same result as using `tloop`. In fact, the `define` command can be even more useful along those lines, since it also loops through the Z dimension, in effect creating a `zloop` function. See the define command for more information.

### Examples

1.  A typical application of the `tloop` function is to obtain a time series of areal averages using the <a href="gradfuncaave.html">`aave`</a> function. Since the `aave` function will not work when time is a varying dimension, the useof `tloop` is required:

    `set x 1 `

    set y 1

    set t 1 31

    d tloop(aave(ts,lon=0,lon=360,lat=-90,lat=90))

    Note that the dimension environment is set up to reflect the kind of plot desired, namely a `line` plot where time is the varying dimension. Thus it is necessary to fix the X and Y dimensions; the values of those dimensions in this case are not relevent.

2.  The `tloop` function can be used to smooth in time:

    `set lon -180 0 `

    set lat 40

    set lev 500

    set t 3 28

    d tloop(ave(z,t-2,t+2))

    In this example, we are plotting a time-longitude cross section, where each time is a 5 time period mean centered at that time.

3.  If we wanted to display a time-longitude cross section (X and T varying), with the data being averaged over latitude, the 'standard' way to do this might be:

    `set lon -180 0 `

    set lat 40

    set lev 500

    set t 1 31

    d ave(z,lat=20,lat=40)

    This calculation could be fairly time consuming, since to perform the average, a longitude-time section is obtained at each latitude. If the time period is long, then this would be a very inneficient operation, due to the ordering of data in a typical GrADS data set. The `tloop` function might substantially improve the performance of this calculation:

    `d tloop(ave(z,lat=20,lat=40)) `

    since the average is then done at each fixed time, and is thus just an average of X varying data over Y. Thus the `tloop` function here is simply being used to force a different ordering to the calculation, although the result is the same.
