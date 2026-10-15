---
title: set clip
---

# **set clip**

`set clip `*`xlo xhi ylo yhi`*` `

This command sets the coordinates for clipping the plot area for display and draw commands. The four arguments *`xlo, xhi, ylo, yhi`* are the clipping coordinates in real page inches.

## Usage Notes

1.  The default clipping region is the area specified by the <a href="gradcomdsetparea.html">`set parea`</a> command. `set clip` overrides this default.
2.  Every time a <a href="gradcomddisplay.html">`display`</a> command is executed, GrADS sets the clipping area to the region specified by <a href="gradcomdsetparea.html">`set parea`</a> or `set clip`, draws the graphic, then resets the clipping region to the entire page.
3.  The clipping region is reset to the default by entering <a href="gradcomdclear.html">`clear`</a> or <a href="gradcomddisplay.html">`display`</a>.

### Examples
