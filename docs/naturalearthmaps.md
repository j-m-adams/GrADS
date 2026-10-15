---
title: Natural Earth Map Data Sets
---

# **Natural Earth Map Data Sets**

GrADS includes three optional map backgrounds derived from [Natural Earth](https://www.naturalearthdata.com/) version 5.1.2: `ne110m` (1:110 million), `ne50m` (1:50 million), and `ne10m` (1:10 million). The names can be used with <a href="gradcomdsetmpdset.html">`set mpdset`</a>. The legacy `lowres` map remains the GrADS default.

The source is the Natural Earth 5.1.2 vector GeoPackage archive. Its SHA-256 is `b9e2e7b3d5c2c59593f6eafc19be2ba87fb04788a90c57ccd6cf32a587b6ecc3`. Natural Earth data are in the public domain; see the [Natural Earth terms of use](https://www.naturalearthdata.com/about/terms-of-use/).

The generated GrADS files have these checksums:

| File | Bytes | SHA-256 |
|----|----|----|
| `ne110m` | 58,596 | `1173c5840a849016efc219cc92479218f5c768ad11bcba14ce8f620ad9a899e9` |
| `ne50m` | 592,938 | `725508f80866ab9386b0ffd6e2f8805007fa2eba586f6a1b5d1b8954c4855aa0` |
| `ne10m` | 5,288,370 | `5bfad17a26c93b07565d7b92549524c568207743b667c00da937ae2736f4dda4` |

## Land and ocean masks

Each resolution also includes standards-compliant land and ocean shapefiles:

| Map data set | Land mask | Ocean mask |
|---|---|---|
| `ne110m` | `ne110m_land` | `ne110m_ocean` |
| `ne50m` | `ne50m_land` | `ne50m_ocean` |
| `ne10m` | `ne10m_land` | `ne10m_ocean` |

Use the packaged `nebasemap.gs` script to draw them:

```text
set mpdset ne50m
display sst
nebasemap O 15 0 50m
```

The mask shapefiles are generated from the corresponding
`ne_<scale>_land` and `ne_<scale>_ocean` GeoPackage layers. Their component
sizes and SHA-256 checksums are recorded in
`data/natural_earth_masks.json`.

Maintainers can regenerate them with GDAL:

```bash
python tools/natural_earth_to_shapefiles.py \
  tools/natural_earth_vector.gpkg.zip data
```

GDAL is required only for regeneration. Installed GrADS builds read the
generated files with Shapelib.

## Included Layers

The same mapping is used at each of the three scales:

| GrADS map type | Natural Earth layer                         |
|----------------|---------------------------------------------|
| 0              | `ne_<scale>_coastline`                      |
| 1              | `ne_<scale>_admin_0_boundary_lines_land`    |
| 2              | `ne_<scale>_admin_1_states_provinces_lines` |

Here, `<scale>` is `110m`, `50m`, or `10m`. The selected layers use Natural Earth's unfiltered default worldview. Separate Natural Earth layers for minor-island coastlines, lakes, rivers, maritime boundaries, and disputed boundaries are not included.

### Controlling Boundaries

Coastlines only:

```text

set mpdset ne10m
set poli off

```

Coastlines plus all political boundaries:

```text

set mpdset ne10m
set poli on

```

Coastlines and country boundaries, without state/province boundaries:

```text

set mpdset ne10m
set mpt * off
set mpt 0 -1
set mpt 1 -1

```

The value `-1` tells GrADS to use the current <a href="gradcomdsetmap.html">`set map`</a> attributes for that map type. See <a href="gradcomdsetmpt.html">`set mpt`</a> for per-type color, line-style, and thickness controls.
