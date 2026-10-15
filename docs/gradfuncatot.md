---
title: atot()
---

GrADS Function: atot

# **atot()**

Thie `atot` function (in GrADS version 2.0.2+) duplicates the summing capability of <a href="gradfuncasum.html">`asum`</a> but adds the latitude-weighting feature of <a href="gradfuncaave.html">`aave`</a>. The syntax is:

`atot(`*`expr, xdim1, xdim2, ydim1, ydim2`*`)`

## Usage Notes

Please see the reference page for <a href="gradfuncasum.html">`asum`</a> for details on the command syntax and usage notes.\

### Examples

1.  An instructive sanity check is to calculate the total area over globe on any grid of data that has a constant value of 1 and no missing values:\
    \
    `ga-> d atot(const(temp,1,-a),global)`\
    `Result value = 12.5664 `\
    \
    The answer we get is 4\*pi, and the unit of the result is steradians.

2.  Suppose you have a data variable sea ice concentration (named 'sic') which is given as a % in each grid box, and you would like to calculate the total area in the Arctic covered by sea ice. To calculate the sea ice coverage in millions of km^2, we multiply the `atot` result by the square of the radius of the Earth in km:

    `ga-> d atot(sic/100,lon=0,lon=360,lat=40,lat=90)*6371*6371*1e-6`\
    `Result value = 11.289`
