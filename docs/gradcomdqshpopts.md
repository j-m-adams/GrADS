---
title: q shpopts
---

# **q shpopts**

`q shpopts `` `

Lists the current settings for drawing and writing out shapefiles.\

## Usage Notes

This command is available with GrADS version 2.0.a9 or later.

The <a href="gradcomdclear.html">`clear shp`</a> command releases all user-defined shapefile attributes from memory, and resets the output filename root and shapefile type to their default values. The <a href="gradcomdreset.html">`reset`</a> and <a href="gradcomdreinit.html">`reinit`</a> commands will do the same thing -- use <a href="gradcomdclear.html">`clear shp`</a> if you do not want to reset all the other user settings.

Please see the documentation page on <a href="shapefiles.html">shapefiles</a> for more details.

 

### Examples

This example shows the default values:\

```text
ga-> 
reinit

ga-> 
q shpopts

Settings for drawing shapefiles:
 polygon fill color: -1 
 mark type: 3 
 mark size: 0.05 
Settings for writing shapefiles:
 output filename root: grads
 output type: line
 format string: %12.6f

```

This example shows how to set and query shapefile options:\

```text
ga-> 
set shpopts
 15
ga-> 
set shp
 -pt -fmt 8 4 pointshp
ga-> 
set shpattr
 Author string JMA
ga-> 
set shpattr
 Date string 2010-05-23 
ga-> 
set shpattr
 pi double 3.14159
ga-> 
q shpopts

Settings for drawing shapefiles:
 polygon fill color: 15 
 mark type: 3 
 mark size: 0.05 
Settings for writing shapefiles:
 output filename root: pointshp
 output type: point
 format string: %8.4f
 attributes:
  Author: JMA
  Date: 2010-05-23
  pi: 3.1416  

```
