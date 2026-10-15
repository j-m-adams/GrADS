---
title: skip()
---

# **skip()**

`skip (`*`expr, skipi, skipj`*`)`

Sets alternating values of *`expr`* to the missingdata value.

- *`expr`*`     `A valid grid expression that may have 1 or 2 varying dimensions\
  *`skipi`*`    `Skip factor in the I dimension of *`expr`*\
  *`skipj`*`    `Skip factor in the J dimension of *`expr`*\

## Usage Notes

1.  This function is often used while displaying wind `arrows` or `barbs` to thin the number of arrows orbarbs. It is not necessary to use the `skip` function onboth the U and V wind components; it is sufficient to populate only onecomponent with missing data values to suppress the plotting of the wind`arrow` or `barb`.

### Examples

1.  Suppose you have a time series of 3-hourly data, but you want to display values at 6-hourly time steps:\
    \
    - `set x 1`\
      `set y 1`\
      `set t 1 last`\
      `d skip(var,2)`\

    \
2.  To display every other grid point in both the X and Y direction:\
    \
    - `d skip(u,2,2);v`

    \
3.  To display every grid point in the Y direction, but every 5th gridpoint in the X direction:\
    \
    - `d skip(u,5,1);v`

    \
4.  This example script "d_uv.gs" written by Wesley Ebisuzakiautomatically sets the skip factor based on the plot dimensions.\
    \*\* This function does a d skip(ugrd,n);v\* where n is automatically set to an appropriate value\*\* usage: d_uv ugrd vgrd \*\* v1.1 w. ebisuzaki\* v1.2 4/6/98 revised empirical formula for skip\*function duv(arg)u = subwrd(arg,1)v = subwrd(arg,2)\* get lat/lon info'query dims'lons = sublin(result,2)lats = sublin(result,3)dx = subwrd(lons,13) - subwrd(lons,11)dy = subwrd(lats,13) - subwrd(lats,11)\* Determine skip factor dn = dxif (dy \> dx) ; dn = dy ; endifskip = dn / 50 + 0.5if (skip \< 1) ; skip=1 ; endif\* Display the plot'd skip('u','skip');'v

```text

*
* This function does a d skip(ugrd,n);v
* where n is automatically set to an appropriate value
*
* usage: d_uv ugrd vgrd 
*
* v1.1 w. ebisuzaki
* v1.2 4/6/98 revised empirical formula for skip
*
function duv(arg)
u = subwrd(arg,1)
v = subwrd(arg,2)

* get lat/lon info
'query dims'
lons = sublin(result,2)
lats = sublin(result,3)
dx = subwrd(lons,13) - subwrd(lons,11)
dy = subwrd(lats,13) - subwrd(lats,11)

* Determine skip factor 
dn = dx
if (dy > dx) ; dn = dy ; endif
skip = dn / 50 + 0.5
if (skip < 1) ; skip=1 ; endif

* Display the plot
'd skip('u','skip');'v

```
