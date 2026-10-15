---
title: set shpopts
---

# **set shpopts**

`set shpopts `*`fillpoly <marktype> <marksize>`*

Sets current attributes for drawing shapefiles.

*`fillpoly`*`   `color number to use when drawing filled polygon elements of a shapefile. Default is -1, which draws polygons unfilled.\
*`marktype`*`   `Mark type to use when drawing point elements of a shapefile. Please see the <a href="gradcomddrawmark.html">`draw mark`</a> command for the list of options.\
*`marksize`*`   `Mark size of use when drawing point elements of a shapefile.

## Usage Notes

This command is available with GrADS version 2.0.a8 or later.

When drawing shapefiles that contain points, the default mark type is a closed circle (type 3) and the default size is 0.05.

When drawing shapefiles that contain polygons, the default behavior is to draw only the perimeter of each polygon element. Use the *`fillpoly`* option with <a href="gradcomdsetshpopts.html">`set shpopts`</a> to draw filled polygons and set the fill color. The polygon perimeters will also be drawn when the *`fillpoly`* option is used. The color, style, and thickness of the polygon perimeters are controlled by the <a href="gradcomdsetline.html">`set line`</a> command.

Use the <a href="gradcomdqshpopts.html">`q shpopts`</a> commandto see the current settings for drawing and writing shapefiles.

Please see the documentation page on <a href="shapefiles.html">shapefiles</a> for more details.

### Examples

`set shpopts 15 `
