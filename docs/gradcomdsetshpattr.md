---
title: set shpattr
---

GrADS Command: set dbfield

# **set shpattr**

`set shpattr `*`name type value`*

Sets attribute metadata to be included in the shapefile created with the <a href="gradcomdsetgxout.html">`set gxout shp`</a> command.

- *`name`*`    `Name of the data base field (attribute), must be 11 characters or less.\
  *`type`*`    `May be one of three data types: "string" ("char"), "int", or "double".\
  *`value`*`   `May be a numeric value (for types "int" and "double") or any string as long as the length of the entire entry does not exceed 512 characters.\
                          

## Usage Notes

This command is available in GrADS v2.0.a9 or higher.

Numerical attribute values (integers and doubles) are written to the shapefile in text format. The <a href="gradcomdsetshp.html">`set shp`</a> command controls the formatting of the numbers, by specifiying the length of the number (total number of columns) and the precision (number of places to the right of the decimal place, which is only meaningful for attributes of type double).

The <a href="gradcomdreset.html">`reset`</a> or <a href="gradcomdreinit.html">`reinit`</a> commands will release all the user-specified shapefile attributes from memory. To do this without resetting all the other user-specified options, use the <a href="gradcomdclear.html">`clear shp`</a> command.

Use the <a href="gradcomdqshpopts.html">`q shpopts`</a> commandto see the current settings for drawing and writing shapefiles.

Please see the documentation page on <a href="shapefiles.html">shapefiles</a> for more information.

### Examples

`set shpattr Author string Put_Your_Name_Here`
