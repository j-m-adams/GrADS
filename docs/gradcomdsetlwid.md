---
title: set lwid
---

# **set lwid**

This command, which is available in GrADS version 2.1+, allows the user to define a new line thickness setting with a specified width.

## Syntax

`set lwid `*`thickness width`*

The *`thickness`* argument must be an integer in the range of` 13 to 256. ` Line thickness settings between 1 and 12 are pre-defined and cannot be changed. The *`width`* argument must be a real number \> 0, but line widths greater than 3.0 are not recommended.

### Usage Notes

The default line widths for *`thickness`* settings between 1 and 12 are:` 0.6, 0.8, 1.0, 1.25, 1.5, 1.75, 2.0, 2.2, 2.4, 2.6, 2.8, 3.0. ` The default line width for *`thickness`* settings greater than 12 is 1.0. The units (pixels or points) will depend on the surface being drawn to.

Once a new thickness setting has been defined, it can be used with any command that controls thickness:` `<a href="gradcomdsetannot.html">`set annot`</a>`, `<a href="gradcomdsetclopts.html">`set clopts`</a>`, `<a href="gradcomdsetcthick.html">`set cthick`</a>`, `<a href="gradcomdsetgrid.html">`set grid`</a>`, `<a href="gradcomdsetline.html">`set line`</a>`, `<a href="gradcomdsetmap.html">`set map`</a>`, `<a href="gradcomdsetmpt.html">`set mpt`</a>`, `<a href="gradcomdsetstring.html">`set string`</a>`, `<a href="gradcomdsetxlopts.html">`set xlopts`</a>`, `<a href="gradcomdsetylopts.html">`set ylopts`</a>`.` The thickness contols when applied to strings (contour labels, axis labels, etc.) will only take effect when Hershey fonts are in use.

### Example

```text
set lwid 13 1.6
set cthick 13



```

###
