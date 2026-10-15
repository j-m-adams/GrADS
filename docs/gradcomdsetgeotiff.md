---
title: set geotiff
---

# **set geotiff**

`set geotiff <-flt | -dbl> `*`fnameroot`*

Sets the filename root and other characteristics for GeoTIFF output.

- *`fnameroot`*`    `Output filename root -- GrADS will append ".tif" to the end\
                            (The default output file name is `gradsgeo.tif`)\
  `-flt         `Data will be written to file using floating-point precision (default)\
  `-dbl         `Data will be written to file using double precision\

## Usage Notes

This command is available in GrADS v2.0.a5 or later.

This command is used in conjunction with the <a href="gradcomdsetgxout.html">`set gxout geotiff`</a>` ` command which sets the graphics output type; the <a href="gradcomddisplay.html">`display`</a> command will then create the output file instead of drawing a plot. The output data file will be in GeoTIFF format, which is a georeferenced raster image. The GeoTIFF output is a grid of data values, similar to the 'grfill' graphics output type, except the pixels (grid elements) in the image can be tied to a geographic location.

The file *`fnameroot`*`.tif` will be replaced if it exists.

If the `-flt` or `-dbl` options are not given, the output will be written using floating-point precision. No compression algorithm is used when creating the GeoTIFF file.

### Examples

`set geotiff my_name`\
`set gxout geotiff`\
`d my_var`\

Note: The GrADS expression `my_var` must be a 2-dimensional grid that varies in X and Y (longitude and latitude).
