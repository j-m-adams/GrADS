---
title: Using Preprojected Grids
---

# Using Preprojected Grids

**Preprojected data** are already defined on a map projection. GrADS reads the
projected data, calculates interpolation constants, and displays the result on
an internal latitude/longitude grid defined by the <code>XDEF</code> and
<code>YDEF</code> cards in the descriptor file.

## Supported preprojected grids

| Grid or projection | Typical source |
| --- | --- |
| Northern polar stereographic | NMC model projection |
| Southern polar stereographic | NMC model projection |
| Lambert conformal | U.S. Navy NORAPS model |
| NMC Eta (unstaggered) | NMC Eta model |
| High-accuracy polar stereographic | High-resolution SSM/I data |
| Oblique polar stereographic | Colorado State University RAMS model |

The internal GrADS grid is independent of the source grid. You can create
multiple descriptor files with different X/Y resolutions or regions that point
to the same projected data file.

When a field is displayed, it is bilinearly interpolated from the projected
grid to the internal latitude/longitude grid. Scalar fields such as 500-mb
height generally display well; increasing the X/Y resolution improves the
interpolation precision. GrADS analysis functions continue to work because
they operate on the internal grid.

The projection is selected with a <code>PDEF</code> card in the descriptor file.

## Polar stereographic data

For NMC polar stereographic GRIB data, a typical PDEF record is:

~~~text
PDEF isize jsize nps ipole jpole lonref gridinc
PDEF 53 45 nps 27 49 -105 190.5
~~~

| Argument | Meaning |
| --- | --- |
| <code>isize</code>, <code>jsize</code> | Number of points in X and Y. |
| <code>nps</code> | Northern polar stereographic projection; use <code>sps</code> for southern polar stereo. |
| <code>ipole</code>, <code>jpole</code> | Grid indices of the pole, referenced from the lower-left point (1,1). |
| <code>lonref</code> | Reference longitude. |
| <code>gridinc</code> | Grid spacing in kilometres. |

## Lambert conformal data

Lambert conformal support was originally implemented for U.S. Navy NORAPS
data. A typical record is:

~~~text
PDEF 103 69 lcc 30 -88 51.5 34.5 20 40 -88 90000 90000
~~~

| Field | Meaning |
| --- | --- |
| 103 | Number of X points. |
| 69 | Number of Y points. |
| <code>lcc</code> | Lambert conformal projection. |
| 30, -88 | Latitude and longitude of the reference point (east positive, west negative). |
| 51.5, 34.5 | X and Y indices of the reference point. |
| 20, 40 | Southern and northern true latitudes. |
| -88 | Standard longitude. |
| 90000, 90000 | X and Y grid spacing in metres. |

## NMC Eta model

The native Eta grid is staggered and non-rectangular. GrADS uses the
unstaggered Eta grid, in which variables share a common rectangular mass-point
grid. Wind rotation is included for this projection.

~~~text
PDEF 181 136 eta.u -97.0 41.0 0.38888888 0.37037037
~~~

| Argument | Meaning |
| --- | --- |
| 181, 136 | Number of X and Y points. |
| <code>eta.u</code> | Unstaggered Eta grid. |
| -97.0, 41.0 | Longitude and latitude of the reference point. |
| 0.38888888, 0.37037037 | Longitude and latitude grid increments in degrees. |

## High-accuracy polar stereo for SSM/I

The original NMC polar stereo assumes a spherical Earth and is not precise
enough for high-resolution SSM/I data. The eccentric polar stereo projection is
described with:

~~~text
PDEF ni nj pse slat slon polei polej dx dy sgn
~~~

| Argument | Meaning |
| --- | --- |
| <code>ni</code>, <code>nj</code> | Number of X and Y points. |
| <code>pse</code> | Eccentric polar stereographic projection. |
| <code>slat</code>, <code>slon</code> | Absolute standard latitude and longitude. |
| <code>polei</code>, <code>polej</code> | Pole index positions, using (0,0) for the first point. |
| <code>dx</code>, <code>dy</code> | Grid spacing in kilometres. |
| <code>sgn</code> | 1 for northern polar stereo, -1 for southern polar stereo. |

Wind rotation is not implemented for this projection; use it for scalar fields
only.

## CSU RAMS oblique polar stereo

The CSU RAMS model uses an oblique polar stereographic projection:

~~~text
PDEF 26 16 ops 40.0 -100.0 90000.0 90000.0 14.0 9.0 180000.0 180000.0
~~~

| Argument | Meaning |
| --- | --- |
| 26, 16 | Number of X and Y points. |
| <code>ops</code> | Oblique polar stereographic projection. |
| 40.0, -100.0 | Latitude and longitude of the reference point. |
| 90000.0, 90000.0 | X and Y reference offsets in metres. |
| 14.0, 9.0 | X and Y indices of the reference point. |
| 180000.0, 180000.0 | X and Y grid spacing in metres. |

Wind rotation is not implemented for this projection; use it for scalar fields
only.

## Common pitfalls

| Issue | Guidance |
| --- | --- |
| Wind-component declarations | The <code>u</code> and <code>v</code> variable declarations must use units 33 and 34, respectively; for example, <code>u 15 33</code> and <code>v 15 34</code>. |
| Wind rotation | Rotation is supported for northern and southern polar stereo, but not Lambert conformal, where Navy data are already Earth-relative. |
| Experimental projections | <code>eta.u</code> and <code>ops</code> remain experimental. |
| Viewing the source grid | A separate descriptor can open the data as i/j values with <a href="gradcomdsetmpdraw.html"><code>set mpdraw off</code></a>, omitting the geographic map. |

## GrADS display projections

Preprojected data and display projections are separate concepts. GrADS first
calculates graphics in internal i/j grid space, converts i/j to latitude and
longitude, and then maps those coordinates to screen x/y.

The supported display projections are:

| Projection | How to select it |
| --- | --- |
| Latitude/longitude (spherical) | Default display projection. |
| Northern polar stereo | <a href="gradcomdsetmproj.html"><code>set mproj nps</code></a> |
| Southern polar stereo | <a href="gradcomdsetmproj.html"><code>set mproj sps</code></a> |
| Robinson | Set longitude to -180/180, latitude to -90/90, and use <code>set mproj robinson</code>. |

Lambert conformal display projection is not currently implemented.

## Summary

GrADS handles map projections in two ways:

1. **Preprojected data**: fields are already on a projection and are
   interpolated to the internal latitude/longitude grid using <code>PDEF</code>.
2. **Display projections**: graphics calculated in grid space are transformed
   to a map projection for rendering.

Currently supported display projections are latitude/longitude, polar stereo,
and Robinson. The preprojected interface supports the projection types
described above, with Eta and CSU RAMS oblique polar stereo remaining
experimental.

