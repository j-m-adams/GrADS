---
title: set strsiz
---

# **set strsiz**

`set strsiz `*`hsiz <vsiz>`*

This command sets the string character size, where*`hsiz`* is the width of the characters,*`vsiz`* is the height of the characters, in virtual pageinches. If *`vsiz`* is not specified, it will be set thethe same value as *`hsiz`*.

## Usage Notes

Beginning with GrADS version 2.1.a0, additional fonts are available with the Cairo graphics library. Please see the documentation on <a href="fontcontrol.html">Font Control</a> for more information. This command controls the font size for both Hershey and Cairo fonts. Because the Hershey fonts are vector-based, you can give different scaling factors for the width and height of the letters and the font will stretch accordingly. For Cairo fonts, only one scaling factor (the vertical size) is appropriate -- the fonts do not deform in the X or Y dimension, they simply get bigger or smaller. If` `*`vsiz`*` `is not given, *`hsiz`* will be used instead.

### Examples
