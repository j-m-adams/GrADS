---
title: set antialias
---

# **set antialias**

`set antialias on` \| `off`

This command, which is available in GrADS version 2.1+, allows the user to disable or enable anti-aliasing.

## Usage Notes

When using GrADS version 2.1+ with the Cairo graphics library, [anti-aliasing](http://en.wikipedia.org/wiki/Spatial_anti-aliasing) is enabled by default. This command does not "stick" -- anti-aliasing resets to "`on`" with <a href="gradcomdclear.html">`clear`</a>, <a href="gradcomdreset.html">`reset`</a>, and <a href="gradcomdreinit.html">`reinit`</a>. For some applications it may be desireable to disable this feature, but in general anti-aliasing improves the appearance of the graphics and should be left on.
