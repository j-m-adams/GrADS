---
title: set line
---

# **set line**

`set line `*`color# <style> <thickness>`*

Sets current `line` attributes.

*`color#:`*`    `one of the <a href="colorcontrol.html">16 GrADS default colors</a> or a new color defined with <a href="gradcomdsetrgb.html">`set rgb`</a>\
*`style:`*`     1 `- solid\
`           2 `- long dash\
`           3 `- short dash\
`           4 `- long dash, short dash\
`           5 `- dotted\
`           6 `- dot dash\
`           7 `- dot dot dash\
*`thickness:`*` `values range from `1` to `6`

## Usage Notes

1.  Line thicknesses 1-5 will take effect only in laser printed output; on the display screen thicknesses 1-5 will all look the same. For screen display, only thickness 6 will appear as a thick line.

### Examples
