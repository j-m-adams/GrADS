---
title: display
---

# **display**

`display `*`expression`*\
or\
`d `*`expression`*

The `display` command is how you actually display data via the graphics output window.

## Usage Notes

If you `display` when all dimensions are fixed, you get a single value which is typed out in the command window. If you `display` when one dimension varies, you get a 1-D line graph (by default). If you `display` when two dimensions are varying, you get a 2-D contour plot (by default).

GrADS will automatically overlay the output from each successive display command.\
To clear the display, enter:

`clear` (or just `c`)

### Examples

The simplest example of an expression to display would be a variable named by the default data file.\
