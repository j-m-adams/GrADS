---
title: set rgb
---

# **set rgb**

This command allows the user to define a new color within GrADS. The syntax is:

`set rgb `*`color# R G B`*

where:

- *`color#`*`  `is the color number; it can range from 16 to 99 (or 255 -- see Usage Note \#1) \
  *`R`*`      ` is the Red value (0-255)\
  *`G`*`      ` is the Green value (0-255)\
  *`B`*`      ` is the Blue value 0-255)

The new color is referred to by its *`color#`*in any GrADS command that allows specification of colors.

## Usage Notes

1.  The *`color#`* must be a value between `16` and `99` -- <a href="colorcontrol.html">`0` to `15` are predefined</a>.\
    As of GrADS version 2.0.a6, the maximum number of colors increased from 99 to 255.\
    \
    
2.  The GrADS metafile-to-postscript translator `gxps` will make use of anynew color settings although the output colors will beprinter-dependent and should be checked for the desiredrendering. When converting new color settings into grayscales, `gxps` will translate theGREEN intensity ONLY into a new greyscale value. Note that `gxps` does have a predefinedmapping between color values from 0 to 15 such that the predefined rainbow sequence is rendered into afairly pleasing greyscale gradation; this cannot be done for newlydefined colors.\
    \
3.  For more details on using defined colors, see the section in theUser's Guide on <a href="colorcontrol.html">controlling colors inGrADS</a>.

### Examples

`set rgb 50 200 200 200 `

Defines a new color number, `50`, andassign a color to it. In this case, the color would be a light gray.
