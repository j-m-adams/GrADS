---
title: gxprint
---

# **gxprint**

Beginning with GrADS version 2.1, the <a href="gradcomdgxprint.html">`gxprint`</a> command will produce all hardcopy output in both image and vector graphic formats. The output is based on the current contents of the metabuffer, and it works in batch mode as well as interactive mode. It replaces the <a href="gradcomdprint.html">`print`</a>, <a href="gradutilgxps.html">`gxps`</a>, <a href="gradutilgxeps.html">`gxeps`</a>, and <a href="gradcomdprintim.html">`printim`</a> commands.

## Syntax

- `gxprint `*`filename options`*

where:

- *`filename:`*`  `The name of the output file. If this file exists, it will be replaced.\
  `            `If the filename ends with ".eps", ".ps", ".pdf", ".svg", ".png", ".gif", or ".jpg", then GrADS will automatically create the output in the corresponding format.\
  \

  *`options:`*\
  `           eps         `Produce output in EPS format\
  `           ps          `Produce output in PS format\
  `           pdf         `Produce output in PDF format\
  `           svg         `Produce output in SVG format\
  `           png         `Produce image output in PNG format\
  `           gif         `Produce image output in GIF format (not available with Cairo)\
  `           jpg         `Produce image output in JPG format (not available with Cairo)\
  `           black       `Use black background (default is current display setting)\
  `           white       `Use white background (default is current display setting)\
  `           xNN         `Horizontal image size in `NN` pixels (for image output only)\
  `           yNN         `Vertical image size in `NN` pixels (for image output only)\
  `           -e NN       `The blank edge around the plot will be `NN` inches wide (for PS, EPS, and PDF output only)\
  `           -t NN       `Color number `NN` is fully transparent\
  `           -b `*`bgImage `*` `Image file *`bgImage`* is included in the background of the output\
  `           -f `*`fgImage`*`  `Image file *`fgImage`* is included in the foreground of the output\

One or more *`options`* may be given in any order.

### Usage Notes

1.  `gxprint` works with GrADS version 2.1+.
2.  *`bgImage`* and *`fgImage`* must be PNG format, and the file names should not contain any upper case letters.
3.  The GIF and JPG image formats are not support by the Cairo library. These options will work only when using the GD library for rendering image output.
4.  The default width of the edge around the PS, EPS, and PDF output files is 0.15625 inches. An edge width smaller than the default may result in the plot getting cropped by the printer.
5.  In GrADS version 2.1+, it is also possible to achieve color transparency (full or partial) using the <a href="gradcomdsetrgb.html">`set rgb`</a> command.
6.  Multi-page output is not yet supported with `gxprint`.

### Examples

The following command produces a 1024x768 PNG image into file test.png:

- `gxprint test.png x1024 y768`

This command produces a GIF image with transparent color 0 and a background image mybkg.png. This means that in test.png, the background image mybkg.png will be seen wherever the color 0 (background) appeared in the current display.

- `gxprint test.png -b mybkg.png -t 0 `\
