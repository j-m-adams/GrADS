---
title: Using Map Projections in GrADS
---

# Using Map Projections in GrADS

Map projections occur in two different places in a GrADS workflow:

| Projection type | What is projected | Main configuration |
| --- | --- | --- |
| Data projection | The source data are already stored on a projected grid. | A <code>PDEF</code> entry in the descriptor file. |
| Display projection | GrADS grid values are transformed for rendering on a map. | <a href="gradcomdsetmproj.html"><code>set mproj</code></a>. |

GrADS supports regular or Gaussian longitude/latitude grids and
preprojected grids. The distinction matters because a preprojected data grid
and the map projection used to draw the display are independent.

## Preprojected data

Preprojected data are already defined on a map projection. When the file is
opened, GrADS calculates interpolation constants and maps the source grid to
an internal longitude/latitude grid defined by <code>XDEF</code> and
<code>YDEF</code>. The internal grid can have a different resolution or region
from the source grid, and multiple descriptor files can point to the same
source data.

Use a <code>PDEF</code> entry to describe the source projection. For example:

~~~text
PDEF 103 69 lcc 30 -88 51.5 34.5 20 40 -88 90000 90000
XDEF 180 linear -180 1.0
YDEF 100 linear -10 1.0
ZDEF 16 levels 1000 925 850 700 500 400 300 250 200 150 100 70 50 30 20 10
TDEF 1 linear 00z1jan1994 12hr
VARS 1
t 16 0 temperature
ENDVARS
~~~

In this example, <code>PDEF</code> describes the native Lambert conformal
grid, while <code>XDEF</code> and <code>YDEF</code> define the internal grid
used for interpolation and display.

See <a href="preprojectedgrids.html">Using Preprojected Grids</a> for
projection-specific PDEF syntax, parameter tables, wind-rotation limitations,
and common pitfalls.

### Interpolation and vectors

| Data type | Behavior |
| --- | --- |
| Scalar fields | Bilinearly interpolated to the internal longitude/latitude grid. Increase X/Y resolution when more interpolation precision is needed. |
| Vector fields | May require rotation from grid-relative to Earth-relative components. Support depends on the projection. |
| Analysis functions | Operate on the interpolated internal grid, so functions such as <a href="gradfuncaave.html"><code>aave</code></a> and <a href="gradfunchcurl.html"><code>hcurl</code></a> remain available. |

To inspect the source i/j grid without the geographic map, use a descriptor
with the internal map drawing disabled:

~~~text
set mpdraw off
~~~

## Projection families for preprojected data

| Projection | Typical use | Status |
| --- | --- | --- |
| Northern polar stereographic (<code>nps</code>) | NMC model data. | Supported. |
| Southern polar stereographic (<code>sps</code>) | NMC model data. | Supported. |
| Lambert conformal (<code>lcc</code>) | Navy NORAPS and related limited-area models. | Supported. |
| NMC Eta (<code>eta.u</code>) | Unstaggered Eta grids. | Supported; wind rotation included. |
| Eccentric polar stereographic (<code>pse</code>) | High-resolution SSM/I scalar fields. | Scalar fields only; wind rotation unavailable. |
| Oblique polar stereographic (<code>ops</code>) | CSU RAMS model grids. | Experimental; wind rotation unavailable. |

## Display projections

For display projections, GrADS first calculates graphics in its internal i/j
grid, converts i/j to longitude/latitude, and then transforms those coordinates
to screen x/y.

| Display projection | Configuration |
| --- | --- |
| Longitude/latitude (spherical) | Default projection. |
| Northern polar stereographic | <a href="gradcomdsetmproj.html"><code>set mproj nps</code></a> |
| Southern polar stereographic | <a href="gradcomdsetmproj.html"><code>set mproj sps</code></a> |
| Robinson | Set <code>lon</code> to -180/180, <code>lat</code> to -90/90, then use <code>set mproj robinson</code>. |

Lambert conformal display projection is not currently implemented.

For example:

~~~text
set lon -180 180
set lat -90 90
set mproj robinson
display slp
~~~

## Choosing the right approach

| Situation | Recommended approach |
| --- | --- |
| Data are already on a projected model grid | Use a complete descriptor with <code>PDEF</code>; GrADS interpolates to its internal longitude/latitude grid. |
| Data are on a regular longitude/latitude grid | Use the normal XDEF/YDEF descriptor and choose a display projection with <code>set mproj</code>. |
| You need to compare fields from different source grids | Open each file, select a common world-coordinate region, and display them on the same internal grid. |
| You need to inspect projected i/j values directly | Use a descriptor with <code>set mpdraw off</code>. |

The two projection mechanisms can be combined: a preprojected data set is first
mapped to the internal grid, and that internal grid can then be rendered with a
display projection.
