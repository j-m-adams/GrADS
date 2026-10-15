---
title: set stat
---

# **set stat**

`set stat on | off`

If `on`, GrADS will print statistical output to the terminal every time the <a href="gradcomddisplay.html">display</a> command is executed. The statistical information is the same as that which is output with <a href="gradcomdsetgxout.html">set gxout stat</a>.

## Usage Notes

1.  If you `"set stat on"`, GrADS will continue to display the output statistics with every <a href="gradcomddisplay.html">display</a> until you `"set stat off"`.
2.  Use this command if you are overlaying plots and want to guarantee that the 2nd plot will be drawn with the same range of values. See example \#2.

### Examples

1.  Here is an example of the statistical output from a horizontal temperature plot:
```text

Data Type = grid
Dimensions = 0 1
I Dimension = 1 to 73 Linear 0 5
J Dimension = 1 to 46 Linear -90 4
Sizes = 73 46 3358
Undef value = -2.56e+33
Undef count = 1763  Valid count = 1595
Min, Max = 243.008 302.818
Cmin, cmax, cint = 245 300 5
Stats[sum,sumsqr,root(sumsqr),n]:     452778 1.29046e+08 11359.8 1595
Stats[(sum,sumsqr,root(sumsqr))/n]:     283.874 80906.7 284.441
Stats[(sum,sumsqr,root(sumsqr))/(n-1)]: 284.052 80957.4 284.53
Stats[(sigma,var)(n)]:     17.9565 322.437
Stats[(sigma,var)(n-1)]:   17.9622 322.64
Contouring: 245 to 300 interval 5 

```
2.  Here is an example showing how to overlay plots with the same axis range:
```text

* display a time series
'set stat on'
'd t(lon=180)'

* get the yaxis range
range = sublin(result,9)
cmin = subwrd(range,5)
cmax = subwrd(range,6)
cint = subwrd(range,7)

* draw an overlay
'set vrange 'cmin' 'cmax
'd t(lon=0)'
'set stat off'

```
