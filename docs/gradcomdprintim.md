---
title: printim
---

# **printim**

Beginning with GrADS version 2.1, the `printim` command has been deprecated and is an alias for <a href="gradcomdgxprint.html">`gxprint`</a>.

The `printim` command will produce a PNG, GIF, or JPG formatted image file based on the current contents of the metabuffer, which is usually the stuff displayed on the screen, minus any widgets. The syntax is:

- `printim `*`filename options`*

where:

- *`filename:`*`  `The name of the output file. If this file exists, it will be replaced.\
  `            `If the filename ends with ".png" or ".PNG" then GrADS will automatically create the image in PNG format\
  `           `If the filename ends with ".gif" or ".GIF" then GrADS will automatically create the image in GIF format\
  `           `If the filename ends with ".jpg" or ".JPG" then GrADS will automatically create the image in JPEG format\

  *`options:`*`   png ` - produce PNG output (default)\
  `           gif ` - produce GIF output\
  `           jpg ` - produce JPEG output\
  `           black ` - use black background (default is current display setting)\
  `           white ` - use white background (default is current display setting)\
  `           xNNN ` - horizontal size in NNN pixels\
  `           yNNN ` - vertical size in NNN pixels\
  `           -t NN ` - color number NN is transparent\
  `           -b `*`bgImage`*` ` - Image file *`bgImage`* s included in the background of the output.\
  `           -f `*`fgImage`*` ` - Image file *`fgImage`* is included in the foreground of the output.\

One or more *`options`* may be given in any order. *`bgImage`* and *`fgImage`* must be PNG format.

## Usage Notes

1.  `printim` works with GrADS version 1.8 (or higher). Beginning with version 2.1, printim is an alias for <a href="gradcomdgxprint.html">`gxprint`</a>.
2.  `printim` can be used while in batch mode.
3.  The option for JPEG formatted output became 'official' with version 2.0.a5.
4.  The file names for *`bgImage`* and *`fgImage`* should not contain any upper case letters.

### Examples

The following command produces a 1000x800 PNG image into file out.png:

- `printim out.png x1000 y800`

This command produces a 800x600 GIF image, with a white background, into file gifimage.out:

- `printim gifimage.out gif x800 y600 white`

This command produces a GIF image with transparent color 0 and a background image mybkg.png. This means that in image.gif, the background image mybkg.png will be seen wherever the color 0 (background) appeared in the current display.

- `printim image.gif -b mybkg.png -t 0 `\
