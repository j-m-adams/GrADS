---
title: set hempref
---

# **set hempref**

`set hempref auto|nhem|shem`

This command controls the way wind barbs are plotted for any output where wind barbs are produced (e.g. <a href="gradcomdsetgxout.html">`set gxout barb`</a>). The options are as follows:

- `auto` (default)

  - The Northern Hemisphere convention is used for wind barbs at positive latitudes and the Southern Hemisphere convention is used for wind barbs at negative latitudes. Barbs and flags are drawn on opposite sides of the wind arrow in the different Hemispheres.

  `nhem`

  - Overrides the default behavior so that all wind barbs are plotted using the Northern Hemisphere convention.

  `shem`

  - Overrides the default behavior so that all wind barbs are plotted using the Southern Hemisphere convention.

## Usage Notes

### Examples
