---
title: Expressions
---

# Expressions

A GrADS expression consists of operators, operands, and parentheses. Parentheses are used to control the order of operation -- this is especially important when using the logical operators.

## Operators

| Operator | Operation |
| --- | --- |
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division |

## Logical operators

Logical operators were introduced in GrADS version 2.1.1.b0 for gridded data
only.

| Operator | Operation |
| --- | --- |
| `=` or `==` | Equal |
| `!=` | Not equal |
| `>` | Greater than |
| `>=` | Greater than or equal |
| `<` | Less than |
| `<=` | Less than or equal |
| `&` or `&&` | And |
| `\|` or `\|\|` | Or |

Operands are <a href="variable.html"><code>variable specifications</code></a>,
<a href="functions.html"><code>functions</code></a>, and constants.

## Usage Notes

Operations between two variables are done on equivalent grid points in each grid. Missing data values in either grid give a result of a missing data value at that grid point. Dividing by zero gives a result of a missing data value at that grid point.

Operations cannot be done between grids that have different scaling in their varying dimensions -- i.e., grids that have different rules for converting the varying dimensions from grid space to world coordinate space. This can only be encountered when you are attempting operations between grids from different files that have different scaling rules.

If one grid has more varying dimensions than the other, the grid with fewer varying dimensions is 'expanded' and the operation is performed.

Expression evaluation in GrADS is recursive, so that multiple expressions may be nested together.

(GrADS version 2.0.a7+) Variable specifications can include a dimension expression to set time as an <a href="offt.html">offset</a> from the variable's initial time.

(GrADS version 2.1.1.b0+) The result of a logical operation is boolean -- an answer to a yes/no question. If the expression is true the result is 1, if the expression is false the answer is -1 (instead of zero). This is slightly different from the usual convention, but it is implemented this way in GrADS to make it easier to use logical operators with the <a href="gradfuncmaskout.html">`maskout()`</a> function. An <a href="gradfuncif.html">`if()`</a> function has also been implemented to use logical operators in expressions of the form if-then-else. Note: the logical operators have not yet been implemented for station data.

## Examples

| Expression | Result |
| --- | --- |
| `slp/100` | Convert sea-level pressure from hPa to mb. |
| `z-z(t-1)` | Height change over one time step. |
| `z-z(offt=0)` | Height change since the initial time. |
| `t(lev=500)-t(lev=850)` | Temperature difference between 500 and 850 mb. |
| `ave(z,t=1,t=5)` | Average of `z` over the first five times in the file. |
| `sum(prec(offt+0),t=1,t=4)` | Accumulated precipitation (the sum of the second through fifth time steps). |
| `z-ave(z,lon=0,lon=360,-b)` | Remove the zonal mean. |
| `tloop(aave(p,global))` | Time series of globally averaged precipitation. |

## Examples Using Logical Operators

| Expression | Result |
| --- | --- |
| `tsfc>=0` | Surface temperatures greater than or equal to zero. |
| `maskout(tsfc,tsfc>=0)` | Set negative surface temperatures to undefined. |
| `(tsfc>30)\|(tsfc<-30)` | Select surface-temperature extremes. |
| `(lat>=-5)&(lat<=5)&(lon>=190)&(lon<=240)` | Select the latitude/longitude bounding box for Niño 3.4. |
| `maskout(tsfc,(lat>=-5)&(lat<=5)&(lon>=190)&(lon<=240))` | Keep surface temperature defined only within the latitude/longitude box. |
