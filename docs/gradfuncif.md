---
title: if()
---

GrADS Function: if

# **if()**

This function performs an if/then/else expression evaluation. It is available starting with GrADS version 2.1.1.b0.

## Syntax

`if (`*`logical_expr, then_expr, else_expr`*`)`

where:

- *`logical_expr`*`   `- any valid logical expression that has a boolean (yes/no) result\
  *`then_expr`*`      `- the result expression if *`logical_expr`* is true\
  *`else_expr   `*`   `- the result expression if *`logical_expr`* is false\

All the arguments must be expressions for gridded data -- the logical operators and the if() function have not yet been implemented for station data.\

### Usage Notes

1.  The *`logical_expr`* should include one or more of the logical operators:` =, !=, >, >=, <, <=, |, &`\
    \
    The result of a logical operation is boolean -- an answer to a yes/no question. If the expression is true the result is 1, and if the expression is false the answer is -1. The `if()` function will evaluate *`logical_expr`* and wherever the result is \>0 it will place the value of *`then_expr`*, and wherever the result is \<0 it will place the value of *`else_expr`*.\
    \
2.  The arguments*`then_expr`* and *`else_expr`* may be any GrADS expression, including a constant. If you want *`then_expr`* or *`else_expr`* to be undefined, then use <a href="gradfuncmaskout.html">`maskout()`</a> instead of the <a href="gradfuncif.html">`if()`</a> function.

### Examples

Here is a script sample to find the minimum and maximum 2-meter temperature at each grid point over a 12-month period:

` 'define tmin = const(t2m,1e9)'`\
`'define tmax = const(t2m,-1e9)'`\
`t = 1`\
`while (t <= 12)`\
`  'set t 't`\
`  'define tmin = if(t2m<tmin,t2m,tmin)'`\
`  'define tmax = if(t2m>tmax,t2m,tmax)'`\
`  t = t + 1`\
`endwhile `
