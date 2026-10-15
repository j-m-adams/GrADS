---
title: set kml
---

# **set kml**

`set kml <-`*`type`*`> `*`fnameroot`*

Sets the filename root and output options for Keyhole Markup Language (KML) output.

Where *`type`* may be one of the following:

- `img | image      `(default) For writing out an image file and complementary KML file\
  `ln  | line       `For writing out contour lines in a KML file\
  `poly             `For writing out polygons from the shaded contouring algorithm in KML format.\

When using the `-img` or `-image` option (the default), two output files will be created:

- *`fnameroot`*`.tif    `A TIFF image file with a pixel for each gridbox in the domain (default is `grads.tif`)\
  *`fnameroot`*`.kml    `A text file in the Keyhole Markup Language (KML) that contains\
  `                  `the georeferencing information for the TIFF image (default is `grads.kml`)\

Otherwise, only one file is written:

- *`fnameroot`*`.kml    `A text file in the KML format that contains the color, thickness, and\
  `                  `georeferencing information for the contour lines or polygons (default is `grads.kml`)

##  

### Usage Notes

This command is available in version 2.0.a5 or later. The `-ln` option for writing out contours in KML format is available in version 2.0.a9 or later. The `-poly` option for writing out polygons in KML format is available in version 2.0.0 or later. In order to use the `-img` option, GrADS must be linked with the TIFF and GeoTIFF libraries. Check the output from 'q config' for the message "GeoTIFF and KML/TIFF output ENABLED". The `-ln` and `-poly` options do not depend on the TIFF library, and are always enabled.

This command is used in conjunction with the <a href="gradcomdsetgxout.html">`set gxout kml`</a>` ` command which sets the graphics output type; the <a href="gradcomddisplay.html">`display`</a> command will then create the output file(s) instead of drawing a plot. The display expression must be a 2-dimensional grid that varies in X and Y (longitude and latitude), and the projection must be latlon.

The files *`fnameroot`*`.tif` and *`fnameroot`*`.kml` will be replaced if they exist. If ` `*`fnameroot`*` `provided by the user ends in ".kml", GrADS will not append an additional ".kml", and will change the extension to ".tif" for the image file (if necessary).

The output in KML format is intended for use with GIS tools such as [Google Earth](http://earth.google.com/). Please note that Google Earth prefers longitudes to range from -180 to 180 instead of 0 to 360.

### Examples

```text
clear
set gxout kml
set kml myimg
d var

clear
set gxout kml
set kml -ln mycntr
d var
 

```
