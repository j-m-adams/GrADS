---
title: Reading NetCDF and HDF Files with GrADS
---

# Reading NetCDF and HDF Files with GrADS

NetCDF and HDF files are called self-describing files (SDF) because the data
and metadata are packaged together. GrADS can read NetCDF and HDF files when
the data fit its internal five-dimensional grid: longitude, latitude, level,
time, and ensemble.

GrADS supports NetCDF, HDF4 Scientific Data Sets, and some HDF5 files. The
interface requires enough metadata to map the file into the GrADS grid. There
are three ways to provide that metadata.

1. [`sdfopen`](gradcomdsdfopen) is the simplest option: provide a filename or
   OPeNDAP URL and GrADS reads the metadata automatically. The file must follow
   the [COARDS conventions](http://ferret.wrc.noaa.gov/noaa_coop/coop_cdf_profile.html).
   The `sdfopen` interface does not support HDF5.
2. [`xdfopen`](gradcomdxdfopen) uses a supplemental descriptor file to add to
   or replace metadata in the SDF. It supports more non-standard files than
   `sdfopen`, but does not support HDF5.
3. [`open`](gradcomdopen) uses a complete GrADS descriptor file to override
   the file metadata. This is the recommended interface when templating many
   files, reading a native (non-lat/lon) projection, handling variables with
   different undefined values, or applying non-standard unpacking. It is the
   only interface that reads HDF5 files.

## Descriptor-file components

The descriptor file is free format. Components on each record are separated
by spaces and may appear in any order. Leading spaces are removed before
parsing, and a record may contain no more than 255 characters. Each record
starts with an entry name followed by its arguments or keywords.

The entries used for NetCDF, HDF-SDS, and HDF5 files are summarized below.

| Entry | Description |
| --- | --- |
| `DSET` | Points to the data file. See the <a href="descriptorfile.html#DSET"><code>DSET</code> reference</a>. |
| `DTYPE` | Use `netcdf` or `hdfsds`; for HDF5 use `hdf5_grid` (GrADS 2.0.a7 and later). |
| `TITLE` | A descriptive title for the descriptor file. |
| `UNDEF` | Sets the file-wide missing-data value; an optional second argument names the SDF attribute containing variable-specific missing values. |
| `UNPACK` | Converts packed integer data using scale and offset attributes. See [UNPACK](#unpack). |
| `OPTIONS` | Valid keywords are `yrev`, `zrev`, `template`, and `365_day_calendar`. |
| `CACHESIZE` | Overrides the HDF5 or NetCDF4 read cache size (GrADS 2.0.a8 and later). It is not relevant to other data types. See [compressed data sets](compression). |
| `PDEF` | Describes an SDF on a native projection such as Lambert conformal or polar stereographic. See the <a href="pdef.html">PDEF documentation</a>. |
| `XDEF`, `YDEF`, `ZDEF`, `TDEF`, `EDEF` | Describe the coordinate dimensions. Their syntax is the same as for binary files. `ncdump -c` can help identify the coordinate dimensions. See the <a href="descriptorfile.html#DSET">descriptor-file reference</a>. |
| `VECTORPAIRS` | Identifies wind-component pairs when a native-projection grid requires rotation to Earth-relative winds. |
| `VARS` through `ENDVARS` | Defines the SDF variables that GrADS should expose. Only variables that you want to read need declarations. |

### `UNDEF`

The first argument is the file-wide undefined value. The optional second
argument is the case-sensitive name of the SDF attribute containing the
variable's missing value. This is useful when variables have different
undefined values: after I/O, missing values are converted to the file-wide
value used by GrADS. If the attribute name is omitted or does not exist, the
file-wide value is used.

```text
UNDEF -9.99e8 _FillValue
```

### `UNPACK`

Use `UNPACK` for packed, non-floating-point variables. GrADS applies:

```text
y = x * scale_factor + add_offset
```

The scale-factor attribute is required; the offset attribute may be omitted,
in which case the offset defaults to `0.0`. Attribute names are case
sensitive. If an attribute cannot be found, the scale factor defaults to
`1.0` and the offset to `0.0`. Unpacking occurs after the undefined-value test.

```text
UNPACK scale_factor add_offset
UNPACK Slope Intercept
```

### `VECTORPAIRS`

`VECTORPAIRS` is needed only when the data use a native projection (`PDEF`) and
wind components must be rotated from grid-relative to Earth-relative
coordinates. List each U/V pair as comma-separated variable names with no
spaces; separate multiple pairs with spaces:

```text
VECTORPAIRS u,v u10,v10 uflx,vflx
```

### `VARS` through `ENDVARS`

SDF variable declarations have a few additional rules. The source variable
name can be aliased to a valid GrADS name with:

```text
SDF_name=>grads_name
```

`SDF_name` must exactly match the variable name in the SDF and may contain
uppercase letters, punctuation, or HDF5 group paths. `grads_name` must be fewer
than 16 characters, begin with a letter, and contain only lowercase letters,
digits, and underscores. The alias can be omitted when the source name already
meets these rules. For `hdf5_grid`, include all nested groups in `SDF_name`,
separated by `/`.

Each declaration contains four fields:

| Field | Meaning |
| --- | --- |
| Variable name | The SDF name, optionally followed by `=>` and its GrADS alias. |
| `levs` | Number of vertical levels. Use `0` for variables without a Z dimension; otherwise use the `znum` value from `ZDEF`. |
| `units` | Comma-separated varying dimensions (`x`, `y`, `z`, `t`, and `e`) in the same order as they are stored in the SDF. NetCDF `ncdump` output and HDF5 `h5dump` dataspace output show this order. |
| Description | Human-readable variable description, optionally including units. |

Examples:

```text
Height=>hgt 17 t,z,y,x Geopotential Height (m)
/HDFEOS/GRIDS/ColumnAmountNO2/Data~Fields/CloudFraction=>cf 15 z,y,x Cloud Fraction
```

## Usage notes

1. GrADS handles NetCDF `short`, `long`, and `float` types. For HDF-SDS it
   handles 8-bit integers (`int8` and `uint8`), 16-bit integers (`int16` and
   `uint16`), 32-bit integers (`int32` and `uint32`), and floats. These types
   are converted to float after I/O.
2. The `sdfopen`/`xdfopen` interfaces automatically unpack NetCDF data when
   the packed type is `short`, the scale and offset constants are `float`, and
   the attribute names are `scale_factor` or `slope` and `add_offset` or
   `intercept`. Otherwise use `open` with a complete descriptor file and name
   the attributes in `UNPACK`. In that case the attribute type may be `short`,
   `long`, `float`, or `double`.
3. For packed data transformed by `UNPACK`, the variable's undefined value is
   interpreted in the original, pre-transformation data units. Missing packed
   values are assigned the file-wide undefined value and are not unpacked.
4. A non-world-coordinate dimension, such as a histogram interval, spectral
   band, or ensemble number, can be represented by a non-negative integer in
   the varying-dimension list:

   ```text
   VAR=>hist0 0 0,y,x First histogram interval for VAR
   VAR=>hist1 0 1,y,x Second histogram interval for VAR
   VAR=>hist2 0 2,y,x Third histogram interval for VAR
   ```

   Alternatively, use an unused world-coordinate axis:

   ```text
   ZDEF 3 linear 1 1
   ...
   VAR=>hist 3 z,y,x VAR Histogram
   ```

   The latter makes slicing easier, but the Z levels represent histogram
   intervals rather than pressure levels.
5. GrADS can handle only one four-dimensional grid per data file. If an SDF
   contains variables on different coordinate axes, write separate descriptor
   files for the different grids.

## Examples

The [sample `ncdump` output](_downloads/sample.ncdump) describes an ocean model
file with eight coordinate dimensions and nine data variables. Five descriptor
files are provided for the different variable groups:

- velocity components [`u` and `v`](_downloads/sample_uv.ctl);
- velocity component [`w`](_downloads/sample_w.ctl);
- potential [temperature](_downloads/sample_temp.ctl);
- wind-stress components [`taux` and `tauy`](_downloads/sample_tau.ctl);
- surface variables [`hflx`, `sflx`, and `eta`](_downloads/sample_sfc.ctl).

The [WRF model](http://www.wrf-model.org/) can produce NetCDF output on native,
non-lat/lon grids. GrADS can read these files with a complete descriptor file
containing a <a href="pdef.html"><code>PDEF</code> entry</a>. The native-grid parameters can be extracted
from WRF global attributes and grid-point longitude/latitude variables. WRF
uses staggered grids, so the examples are intended as a guide rather than a
universal template:

- [edited WRF `ncdump` output](_downloads/wrf.ncdump);
- descriptor file for [native grid longitude and latitude](_downloads/wrfgrid.ctl);
- descriptor file for [four WRF data variables](_downloads/wrfvars.ctl).

The grid descriptor uses abstract X and Y increments rather than longitude and
latitude. In the example, grid point `(1,1)` has values `(-125.898, 26.9628)`
and is used as the reference point in the `PDEF` entry.
