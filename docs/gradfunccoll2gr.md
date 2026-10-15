---
title: coll2gr()
---

# **coll2gr()**

This function creates a grid from a collection of station data. The syntax is:

`coll2gr(`*`cnum <,num>`*`)`

- *`cnum`*`    ` collection number\
  *`num`*`     ` number of vertical levels in the output grid (default is 10)\

If *`num`* is given as `"-u"`, then the vertical levels will be a union of all the levels in the collection of station data.

## Usage Notes

1.  Before using `coll2gr`, you must create the collection of stations using the GrADS command <a href="gradcomdcollect.html">`collect`</a>.
2.  `coll2gr` does not yet support time slices; currently, it will only work when the collection of stations is a collection of vertical profiles.
3.  `coll2gr` produces an output grid that varies in X and Z; the dimension environment used when `coll2gr` is invoked must also be X and Z varying. The X axis of the output grid will contain the equally spaced station profiles and will span the range of the current X dimension environment. The Z axis of the output grid will span the range of the current Z dimension environment and will have either the specified number of levels or a union of the levels. Data points outside of the range of levels will be used for interpolating to within the range if appropriate.
4.  The `"-u"` option will only work for data in pressure vertical coordinates. If your vertical coordinate is height, *`num`* should be the actual number of vertical levels in the collection of profiles. Note: This restriction has been removed as of version 2.1.0.
5.  See the section of the User's Guide on <a href="usingstationdata.html#xsection">Arbitrary Cross Sections</a> for more information.

### Examples
