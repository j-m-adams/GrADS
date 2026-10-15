---
title: draw wxsym
---

# **draw wxsym**

`draw wxsym `*`symbol x y size<color <thickness>>`*

Draws the specified `wx` symbol at the specified location.where:

- *`symbol`*               isan integer specifying what symbol todraw\
  *`x`*                         xlocation, in plotter inches\
  *`y`*                         ylocation\
  *`size`*                   sizeof the symbol\
  *`color`*                 colorof symbol. Use `-1` (the default) to getstandard colors (red for storm, blue for snow, etc)\
  *`thickness`*         linethickness of the symbol (default is 3)

## Usage Notes

1.  To see what symbols are available, run the script [`wxsym.gs`](_downloads/grads-scripts/wxsym.gs) to see how to issue the `wxsym`command.
2.  To change the default colors of the weather symbols, use <a href="gradcomdsetwxcols.html">`set wxcols`</a>\

### Examples
