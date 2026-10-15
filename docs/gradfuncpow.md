---
title: pow
---

# **pow**

`pow(`*`expr1,expr2`*`)`

The `pow` function raises the values of *`expr1`* to the power of *`expr2`*. No error checking is performed for invalid values in the operands. This function works on both gridded and station data.

## Usage Notes

### Examples

1.  To square some value:
    `pow(expr,2) `
2.  To duplicate the operation of the mag function:
    `sqrt(pow(u,2)+pow(v,2)) `
