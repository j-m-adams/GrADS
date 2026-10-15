---
title: Gradutilgxtran
---

GrADS Utilities: gxtran

# 
(gxtran)=
gxtran

`gxtran [ -`*`airg <infile>`*` ]`

This utility is used to display GrADS meta files containing single frames or animations (multi frames).

The options are:

- `-i `*`fname`*`    ` identifies input GrADS metacode file\
  `-a          ` animates the frames without user hitting return in the command window\
  `-r          ` reverses background/foreground colors\
  `-g `*`geom`*`     ` sets the geometry of the X window as in any other X application.\
  `            ` *`geom`* has the form WWWxHHH+X+Y, giving a display window\
  `            ` WWW pixels wide and HHH pixels tall starting at point X,Y on the screen.\

**Usage Notes**

Without any options, `gxtran` prompts the user for the name of the GrADS meta file. To quit gxtran, hit return in the command window.

**Examples**

`gxtran -a -g 800x600+0 -i mytest.mf`

This command would open a window of size 800x600 starting at the upper left corner of the screen and animate all frames (plots) in the file `mytest.mf`.
