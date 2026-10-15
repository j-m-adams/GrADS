---
title: set display
---

# **set display**

`set display `*`grey|greyscale|color <black|white>`*

Sets the mode of the display.

By default, the mode is `color`, where shading and contouring is done with a rainbow of colors. When using a monochrome display, these colors may not map to greyscale in a pleasing way. When the mode is set to `greyscale`, contours are displayed using a single grey level, and shaded contours are done using a sequence of greyscales.

You may optionally set the hardware background color to `black` or `white`. The default is `black`.

Issuing: `set display grey white` gives a result on the display that is very similar to the output produced by `gxps`.

## Usage Note

1.  This command DOES NOT affect hardcopy output.

### Examples
