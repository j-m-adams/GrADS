---
title: maskout
---

# **maskout**

`maskout(`*`expr,mask`*`)`

Wherever the *`mask`* values are less than zero, the values in *`expr`* are set to the missing data value.

Works with gridded or station data. Where *`mask`* values are positive, the *`expr`* values are not modified. Thus the result of `maskout` is data with a possibly increased number of missing data values. The `maskout` function, in spite of its apparent simplicity, is extremely useful.

## Usage Notes

### Examples

See the Examples for the <a href="gradfuncconst.html">`const`</a> function for a description of using `maskout` to calculate the percentage of the globe covered by precipitation.

The `maskout` function can be used to cause part of the data to be ignored while doing another calculation. For example, if we have a land-sea mask, where sea values are negative, and we want to take some areal average of a quantity only over land:

`d aave(maskout(p,mask.2),lon=0,lon=360,lat=0,lat=90) `

People frequently have trouble using a mask grid, because it is often put into a seperate file, and given some arbitrary date/time and level. Thus, it is often necessary to *locally override* the dimension environment while using the mask grid:

`d aave(maskout(p,mask.2(t=1)),lon=0,lon=360,lat=0,lat=90) `

would probably be how Example 2 would have to be expressed in order to work, with the local override of `t=1` specified on the mask data. See the documentation on how GrADS evaluates expressions within the dimension environment for more information.
