---
title: set cstyle
---

# **set cstyle**

`set cstyle `*`style`*

Sets the contour linestyle. *`style`* options are:

- `0 `- no contours\
  `1 `- solid\
  `2 `- long dash\
  `3 `- short dash\
  `4 `- long dash, short dash\
  `5 `- dotted\
  `6 `- dot dash\
  `7 `- dot dot dash\

## Usage Notes

1.  `set cstyle` is reset by entering <a href="gradcomdclear.html">`clear`</a> or <a href="gradcomddisplay.html">`display`</a>.
2.  `set cstyle 0` will generate the following message: `"WARNING cstyle = 0 ; no lines will be plotted; I suggest 1 ..."`. However, it may be used with <a href="gradcomdsetcmark.html">`set cmark`</a> for drawing a 'line' plot with marks at all the data points that are not connected by a line.

### Examples
