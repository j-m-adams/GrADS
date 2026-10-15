---
title: clear
---

GrADS Command: clear

# **clear**

`clear` (or just `c`)

Issued without parameters, the `clear` command does pretty heavy duty clearing of many of the GrADS internal settings. Parameters can be added to limit what is cleared when using more advanced features.

## Usage Notes

### Examples

`clear norset        `clears without resetting user options\
`clear events        `flushes the events buffer (e.g., mouse clicks)\
`clear graphics      `clears the graphics, but **not** the widgets\
`clear hbuff         `clears the buffered display and turns off double buffer mode\
`clear button `*`num`*`    `clears button *`num`*\
`clear rband `*`num`*`     `clears rband *`num`*\
`clear dropmenu `*`num`*`  `clears dropmenu *`num`*\
`clear sdfwrite      `clears sdfwrite file name and attributes without resetting other user options (2.0.a3+)\
`clear mask          `clears contour label mask, which is enabled with the command `'`<a href="gradcomdsetclab.html">`set clab`</a>` masked'`(2.0.a7+)\
`clear shp           `clears shapefile output file name and attributes without resetting other user options (2.0.a9+)\
