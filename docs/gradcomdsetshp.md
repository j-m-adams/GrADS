---
title: set shp
---

# **set shp**

`set shp <-`*`type`*`> <-fmt `*`n m`*` > `*`fnameroot`*

Sets the filename root for shapefile output and has additional arguments to control the type of shapefile created and the formatting of numerical values in the dBase file. This command is used in conjunction with <a href="gradcomdsetgxout.html">`set gxout shp`</a> and <a href="gradcomddisplay.html">`display`</a> to create four complementary files:

- *`fnameroot`*`.shp    `The main binary file in which each record describes a shape with a list of vertices (default is `grads.shp`)\
  *`fnameroot`*`.shx    `The index file contains the offsets for each record in the main file (default is `grads.shx`)\
  *`fnameroot`*`.dbf    `A dBase table that contains feature attributes for each record in the main file (default is `grads.dbf`)\
  *`fnameroot`*`.prj    `A text file that describes the default projection: the WGS84 lon-lat spheroid (default is `grads.prj`)

Where *`type`* may be one of the following:

`ln | line        `(default) Shapefile output will contain shapes of the type PolyLineM (measured contour lines)\
`pt | point       `Shapefile output will contain shapes of the type PointM (measured points)\
`poly             `Shapefile output will contain polygons of the type PolygonM (measured polygons)

`-fmt `*`n m`*`         `Optional controls for the formatting of the numerical values in the dBase file.\
`                 `*`n`*` `is the maximum number of digits to be used when formatting integers and doubles in the data base entry (default is 12)\
`                 `*`m`*` `is the number of precsion decimal places to be used when formatting doubles in the data base entry (default is 6)\
`                 Both `*`m`*` `and *n* must be provided when using the -fmt option

 

## Usage Notes

This command is available in GrADS v2.0.a9 or later. The -poly option is available in version 2.0.0 or later.

The files *`fnameroot`*`.* `will be replaced if they exist.

When creating a shapefile with contour lines, the <a href="gradcomdsetcterp.html">`cterp`</a> setting is ignored -- contours are written out as if ` `<a href="gradcomdsetcterp.html">`cterp`</a> is set to `off` (i.e. no spline fitting).

Use the <a href="gradcomdqshpopts.html">`q shpopts`</a> command to get information on the status of the shapefile options; use the <a href="gradcomdclear.html">`clear shp`</a> command to reset shapefile options to their default values.

The output in shapefile format is intended for use with GIS tools, and can also be drawn with GrADS. Please see the documentation page on <a href="shapefiles.html">shapefiles</a> for more information.

### Examples

`set gxout shp`\
`set shp -ln -fmt 8 3 my_shapefile_name`\
`d my_var`\
`set shp -poly mypolyfile`\
`set shpattr author string your_name`\
`d my_var`\

Note: The GrADS expression `my_var` must be a 2-dimensional grid that varies in X and Y (longitude and latitude). In this example, GrADS will use the format string `%8.3f` to write out double precision numbers to the dBase file.\
