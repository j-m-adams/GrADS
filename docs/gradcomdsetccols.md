---
title: set ccols
---

# **set ccols**

`set ccols `*`col1 col2 col3 ... colN`*

Sets specific color numbers for contour levels specified by `set clevs`.

## Usage Notes

1.  Contour colors are reset with every execution of `clear` or `display`.
2.  User-specified ccols won't take effect unless clevs are also set.
3.  If any of the color numbers is \< 0, it is set to 0 (the background color).
4.  (Version 2.0.0+) When using 'gxout shade2' or 'gxout shade2b', if any of the color numbers is \< 0, the contour is not drawn (i.e., it is effectively transparent).
5.  See section of User's Guide on <a href="colorcontrol.html">controlling colors in Grads</a> for more details.

### Examples
