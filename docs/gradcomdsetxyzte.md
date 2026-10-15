---
title: set x\|y\|z\|t\|e
---

# **set x\|y\|z\|t\|e**

`set x|y|z|t|e `*`val1 <val2>`*

This sets one dimension of the <a href="dimenv.html">dimension environment</a> using gridcoordinates. You may use whatever coordinates are convenient to you.Issuing `set lon` is equivalent to issuing `set x`,both set the `x` dimension. The difference is only the unitsyou wish to use when entering the command.

When you enter just one value, that dimension is said to be "fixed". Whenyou enter two values, that dimension is said to be "varying". Thecombination of fixed and varying dimensions defines the dimensionenvironment.

## Usage Notes

The 'set e' command is only available in GrADS version 2.0+, when the <a href="ensembles.html">ensemble dimension</a> was introduced.

An important note: When you enter dimensions in grid coordinates, they are always converted to world coordinates. This conversion requires some knowledge of what scaling is in use for grid to world conversions. The scaling that is used in all cases (except one) is the scaling of the DEFAULT FILE. The exception is when you supply a dimension expression within a <a href="variable.html">variable specification</a>.

If you are setting the T or E dimension using grid coordinates, the value given in *`val1`*  or *`val2`*  may be the word "last" to set the dimension to the max value of that grid coordinate.

### Examples

`set t 1`\
This sets time to the first time in the data set -- usinggrid coordinates. T is now a fixeddimension.

`set z 1 4`\
This sets the vertical dimension to vary between the first and the fourth levels in the data set -- usinggrid coordinates. Z is now a varyingdimension.

`set t 1 last`\
This sets time to vary between the first and the last time in the data set -- usinggrid coordinates. T is now a varyingdimension.

`set e last `\
This sets the ensemble to the final member in the data set. E is now a fixeddimension.
