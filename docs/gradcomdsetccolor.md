---
title: set ccolor
---

# **set ccolor**

- `set ccolor rainbow`\
  `set ccolor `*`color#`*

This command sets the color of the plotted contours. If `ccolor` is set to `rainbow`, then GrADS will use the <a href="colorcontrol.html">default rainbow sequence</a> to assign colors to individual contours, resulting in a pleasing color palette that spans the range of the plotted values. To draw all contours with the same color, use the second option, where *`color#`* may be one of the <a href="colorcontrol.html">16 GrADS default colors</a> or the number of a user-defined color created with <a href="gradcomdsetrgb.html">set rgb</a>.

## Usage Notes

1.  `set ccolor` is reset by entering <a href="gradcomdclear.html">`clear`</a> or <a href="gradcomddisplay.html">`display`</a>.
2.  The color sequence for the <a href="colorcontrol.html">default rainbow palette</a> is: `9 14 4 11 5 13 3 10 7 12 8 2 6`. This may be overridden by using the <a href="gradcomdsetrbcols.html">set rbcols</a> command.
3.  If the number of colors in the rainbow palette is smaller than the number of contour levels, then some contour colors will be repeated -- i.e. not every contour color will be unique. Likewise, if the number of colors in the rainbow palette is larger than the number of contour levels, then some colors will be omitted.
4.  When overlaying contour plots, GrADS will automatically change the color of the contours for each successive plot in order to distinguish between different variables. The default order is: rainbow, foreground, 3 (green), 7 (yellow), 2 (red), 6 (magenta), 9 (purple), ....

### Examples
