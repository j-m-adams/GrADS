---
title: q define
---

# **q define**

`q define `*`varname`*

This command returns information about a defined variable, *`varname`*. The `'q define'` command without the variable name will return a list of defined variables. When a variable name is added, the printout includes the type, the sizes of each dimension, axis definitions for each dimension, the calendar, and whether the variable has been <a href="gradcomdmodify.html">`'modified'`</a>.

## Usage Notes

This command is available in GrADS version 2.1.1.b0.

### Example

` `

ga-\>define ps=ps\
Define memory allocation size = 8311688 bytes\
ga-\> q define ps\
Gridded Defined Variable: ps\
Xsize = 1441 Ysize = 721 Zsize = 1 Tsize = 1 Esize = 1\
XDEF 1441 linear 0 0.25\
YDEF 721 linear -90 0.25\
ZDEF 1 levels 1000\
TDEF 1 linear 12Z02FEB2017 180mn\
Calendar: Gregorian q ens\

###
