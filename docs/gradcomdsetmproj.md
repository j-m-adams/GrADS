---
title: set mproj
---

# **set mproj**

`set mproj `*`proj`*

Sets current map projection. Options for *`proj`* are:

- `latlon     ` Lat/lon projection with aspect ratio maintained (default)\
  `scaled     ` Lat/lon aspect ratio is not maintained; plot fills entire plotting area\
  `nps        ` North polar stereographic\
  `sps        ` South polar stereographic\
  `lambert    ` Lambert conformal conic projection\
  `mollweide  ` Mollweide projection\
  `orthogr    ` Orthographic projection\
  `robinson   ` Robinson projection, requires `set lon -180 180, set lat -90 90`\
  `off        ` No map is drawn; axis labels are not interpreted as lat/lon\

## Usage Notes

### Examples
