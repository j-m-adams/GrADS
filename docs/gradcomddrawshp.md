---
title: draw shp
---

# **draw shp**

`draw shp `*`shapefile <n> <m>`*` `

Draws the contents of a shapefile. The arguments are:

*`shapefile`*`  `The name of the shapefile. It is not necessary to include the file extension (.shp)\
*`n`*`          `Element number (if you want to draw only one)\
*`m`*`          `Use *`n`*  and *`m`* if you want to draw a range of elements

If *`n`*  and *`m`* are omitted, all the elements in the shapefile will be drawn.\

## Usage Notes

This command is available with GrADS version 2.0.a8 or later.

If you put the three shapefile components (\*.shp, \*.shx, and \*.dbf) in the GrADS data directory (pointed to by the <a href="gradcomdgrads.html#env">GADDIR environment variable</a>), then it is not necessary to include the full path in *`shapefile`*.

Before invoking this command, it is necessary to draw a plot first in order to establish the dimensions and scaling of the display. Shapefiles contain 2-dimensional spatial features, so your plot must be varying in the X-Y (lon/lat) domain. A shapefile may contain one of three kinds of graphical elements: points, lines, or polygons.

- For shapefiles that contain points, GrADS will draw a mark at each point location. The mark type and size are controlled by the <a href="gradcomdsetshpopts.html">`set shpopts`</a> command, and the color is controlled by the <a href="gradcomdsetline.html">`set line`</a> command.
- For shapefiles that contain lines, GrADS will draw the line elements using the color, style, and thickness settings that are controlled by the <a href="gradcomdsetline.html">`set line`</a> command.
- For shapefiles that contain polygons, the default behavior of GrADS is draw only the perimeter of each polygon element. Use the <a href="gradcomdsetshpopts.html">`set shpopts`</a> command to draw filled polygons and set the fill color. The color, style, and thickness of the polygon perimeters are controlled by the <a href="gradcomdsetline.html">`set line`</a> command.

Filled polygons preserve all rings in a shape. Interior rings are rendered as
holes using the even-odd fill rule, including after map projection and
clipping.

Please see the documentation page on <a href="shapefiles.html">shapefiles</a> for more details.

### Examples
