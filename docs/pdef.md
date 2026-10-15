---
title: Working with pre-projected data
---

# Working with pre-projected data

## Use PDEF for displaying pre-projected data with GrADS

- [Display pre-projected data with PDEF](#display-pre-projected-data-with-pdef)
- [PDEF syntax](#pdef-syntax)
- [How grid interpolation works](#how-pdef-grid-interpolation-works)
- [How wind rotation works](#how-pdef-wind-rotation-works)
- [PDEF BILIN option](#the-pdef-bilin-option)
- [PDEF GENERAL option](#the-pdef-general-option-and-the-pdef-file-option)
- [PDEF FILE option](#the-pdef-general-option-and-the-pdef-file-option)


## Display pre-projected data with PDEF

Gridded data mapped onto a particular map projection are called *pre-projected*. An example is a weather forecast model output mapped onto a north polar stereographic grid.

### Native and rectilinear grids

A descriptor file containing a `PDEF` entry describes two grids:

- The native grid, described by `PDEF`, containing the pre-projected data.
- The rectilinear latitude/longitude grid, described by `XDEF` and `YDEF`.

`PDEF` describes the native grid dimensions and how to map its `i/j` coordinates to latitude/longitude values. GrADS uses this information to interpolate the native data onto the rectilinear grid. Displays and analyses operate on the interpolated grid, so built-in functions such as [aave](/grads/gadoc/gradfuncaave) and [hcurl](/grads/gadoc/gradfunchcurl) work with pre-projected data. The trade-off is that the displayed data are interpolated.

### Viewing the native grid

To view pre-projected data on its native grid, omit `PDEF` and use `XDEF` and `YDEF` to describe the native grid. Draw the data in `i/j` space without a map projection:

```text
set mpdraw off
```

### Vector fields

When displaying a pre-projected vector field with [display](/grads/gadoc/gradcomddisplay), determine whether the vector components are grid-relative or Earth-relative. Grid-relative components must be rotated to Earth-relative coordinates before interpolation. See the notes for each projection below.

## PDEF syntax


```text
PDEF isize jsize NPS ipole jpole lonref gridinc
```

```text
PDEF isize jsize SPS ipole jpole lonref gridinc
```

**Example**

```text
PDEF 53 45 nps 27 49 -105 190.5
```

**Arguments**

| Argument | Description |
|---|---|
| `isize` | The size of the native grid in the x direction |
| `jsize` | The size of the native grid in the y direction |
| `ipole` | the i coordinate of the pole referenced from the lower left corner, assumed to be at (1,1) |
| `jpole` | the j coordinate of the pole referenced from the lower left corner, assumed to be at (1,1) |
| `lonref` | reference longitude |
| `gridinc` | distance between grid points in km |


**Notes**

Polar stereographic projections (N and S) are defined as at NCEP. Wind rotation has also been added so that vector data will be properly displayed.


 
 


```text
PDEF isize jsize LCCR latref lonref iref jref Struelat Ntruelat slon dx dy
```

```text
PDEF isize jsize LCC latref lonref iref jref Struelat Ntruelat slon dx dy 
```

**Example**

```text
PDEF 103 69 lccr 30 -88 51.5 34.5 20 40 -88 90000 90000
```

**Arguments**

| Argument | Description |
|---|---|
| `isize` | The size of the native grid in the x direction |
| `jsize` | The size of the native grid in the y direction |
| `latref` | reference latitude |
| `lonref` | reference longitude (in degrees, E is positive, W is negative) |
| `iref` | i of ref point |
| `jref` | j of ref point |
| `Struelat` | S true lat |
| `Ntruelat` | N true lat |
| `slon` | standard longitude |
| `dx` | grid X increment in meters |
| `dy` | grid Y increment in meters |


**Notes**

Starting with version 1.9b4, the LCCR option supplements the use of PDEF with data on the Lambert Conformal projection. With LCCR, wind rotation has been implemented for data files with grid-relative winds instead of Earth-relative winds. Use LCC if your vector components are already Earth-relative.


 
 
 


```text
PDEF isize jsize ETA.U lonref latref dlon dlat
```

**Example**

```text
PDEF 181 136 eta.u -97.0 41.0 0.38888888 0.37
```

**Arguments**

| Argument | Description |
|---|---|
| `isize` | The size of the native grid in the x direction |
| `jsize` | The size of the native grid in the y direction |
| `lonref` | reference longitude (in degrees, E is positive, W is negative) |
| `latref` | reference latitude |
| `dlon` | grid longitude increment in degrees |
| `dlat` | grid latitude increment in degrees |


**Notes**

The eta model native grid is awkward to work with because the variables are on staggered and non-rectangular grids. NCEP created "unstaggered" eta model fields, in which the variables are placed on a common rectangular grid. Wind rotation has also been added so that vector data will be properly displayed.


 
 
 


```text
PDEF isize jsize PSE slat slon ipole jpole dx dy sign
```

**Example**

 


**Arguments**

| Argument | Description |
|---|---|
| `isize` | The size of the native grid in the x direction |
| `jsize` | The size of the native grid in the y direction |
| `slat` | absolute value of the standard latitude |
| `slon` | absolute value of the standard longitude |
| `ipole` | the i coordinate of the pole referenced from the lower left corner, assumed to be at (0,0) |
| `jpole` | the j coordinate of the pole referenced from the lower left corner, assumed to be at (0,0) |
| `dx` | grid X increment in km |
| `dy` | grid Y increment in km |
| `sign` | 1 for NH; -1 for SH |


**Notes**

The polar stereo projection used by the original NMC models is not very precise because it assumes the earth is round (eccentricity = 0). While this approximation was reasonable for coarse resolution NWP models, it is inadequate to work with higher resolution data such as SSM/I. Wind rotation has not been implemented!!! Use only for scalar fields.


 
 
 


```text
PDEF isize jsize OPS latref lonref xoff yoff iref jref dx dy
```

**Example**

```text
PDEF 26 16 ops 40.0 -100.0 90000.0 90000.0 14.0 9.0 180000.0 180000.0
```

**Arguments**

| Argument | Description |
|---|---|
| `isize` | The size of the native grid in the x direction |
| `jsize` | The size of the native grid in the y direction |
| `latref` | reference latitude |
| `lonref` | reference longitude (in degrees, E is positive, W is negative) |
| `xoff` | lonref offset in meters |
| `yoff` | latref offset in meters |
| `iref` | the i coordinate of the reference point |
| `jref` | the j coordinate of the reference point |
| `dx` | grid X increment in km |
| `dy` | grid Y increment in km |
| `dy` | grid Y increment in km |


**Notes**

The CSU RAMS model uses an oblique polar stereo projection. Wind rotation has not been implemented!!! Use only for scalar fields.


 
 


```text
PDEF isize jsize ROTLL lonpol latpol dlon dlat lonll latll
```

```text
PDEF isize jsize ROTLLR lonpol latpol dlon dlat lonll latll
```

**Example**

```text
PDEF 500 330 rotllr -170.0 43.0 0.02 0.02 -5.5 -3.8
```

**Arguments**

| Argument | Description |
|---|---|
| `isize` | The size of the native grid in the x direction |
| `jsize` | The size of the native grid in the y direction |
| `lonpol` | Longitude of the rotated pole in degrees |
| `latpol` | Latitude of the rotated pole in degrees |
| `dlon` | grid spacing in longitudinal direction of the rotated grid in degrees |
| `dlat` | grid spacing in latitudinal direction of the rotated grid in degrees |
| `lonll` | longitude of the lower left corner, given in rotated space in degree |
| `latll` | latitude of the lower left corner, given in rotated space in degree |


**Notes**

(GrADS version 2.0) The rotated lat/lon grid projection is described in the [COSMO documentation](http://www.cosmo-model.org/content/model/documentation/core/cosmo_userguide_5.04.pdf), Chapter 3.3. The lower left corner, i.e. the first element in the data array, has to be the southwest corner. It is not possible to use a mirrored grid by setting dlon or dlat to a negative value.


 
 


```text
PDEF isize jsize BILIN format byteorder fname
```

**Example**

```text
PDEF 100 100 BILIN sequential binary-big ^mygrid.interp.values
```

**Arguments**

| Argument | Description |
|---|---|
| `isize` | The size of the native grid in the x direction |
| `jsize` | The size of the native grid in the y direction |
| `format` | Must be either STREAM (direct access) or SEQENTIAL (fortran formatted) |
| `byteorder` | If set to BINARY, byte odering is assumed to be same as local machine |
| `If set to BINARY-BIG, byte ordering is assumed to be big-endian` | If set to BINARY-LITTLE, byte ordering is assumed to be little-endian |
| `fname` | The name of the supplementary file |


**Notes**

The supplementary file contains three lat-lon floating-point grids: i values, j values, and wind rotation values. The native grid is assumed to have a corner (i,j) value of (1,1). The size of these grids must match the XDEF and YDEF entries in the descriptor file.


 
 


```text
PDEF isize jsize GENERAL num format byteorder fname
```

**Example**

```text
PDEF 182 149 general 4 sequential binary-big ^mygrid.interp.values
```

```text
PDEF 15238 1 general 1 stream binary ^gtd.filepdef
```

**Arguments**

| Argument | Description |
|---|---|
| `isize` | The size of the native grid in the x direction |
| `jsize` | The size of the native grid in the y direction |
| `num` | number of sets of interpolation grids supplied |
| `format` | Must be either STREAM (direct access) or SEQENTIAL (fortran formatted) |
| `byteorder` | If set to BINARY, byte odering is assumed to be same as local machine |
| `If set to BINARY-BIG, byte ordering is assumed to be big-endian` | If set to BINARY-LITTLE, byte ordering is assumed to be little-endian |
| `fname` | The name of the supplementary file; it may be mixed case. |


**Notes**

(GrADS version 2.0.a3 and later) The syntax and behavior of PDEF GENERAL is exactly like PDEF FILE, except that the convention for the native grid offset values in the pdef file is the same for all data formats. The offsets should be 1-based; the first grid point is assumed to have (i,j) values of (1,1), and valid offset values are > 0 and <= isize*jsize .
Native grid offset values of -999 indicate not to use an input point for that portion of the calculation (thus you can apply less than the "num" number of interpolation points for some of the points).
See additional notes in the paragraphs below.


 


```text
PDEF isize jsize FILE num format byteorder fname
```

**Example**

```text
PDEF 182 149 file 4 sequential binary-big ^mygrid.interp.values
```

```text
PDEF 15238 1 file 1 stream binary ^gtd.filepdef
```

**Arguments**

| Argument | Description |
|---|---|
| `isize` | The size of the native grid in the x direction |
| `jsize` | The size of the native grid in the y direction |
| `num` | number of sets of interpolation grids supplied |
| `format` | Must be either STREAM (direct access) or SEQENTIAL (fortran formatted) |
| `byteorder` | If set to BINARY, byte odering is assumed to be same as local machine |
| `If set to BINARY-BIG, byte ordering is assumed to be big-endian` | If set to BINARY-LITTLE, byte ordering is assumed to be little-endian |
| `fname` | The name of the supplementary file; it may be mixed case. |


**Notes**

For GrADS v2.0.a2 and earlier, jsize was fixed to be 1, and isize was the size of the native grid expressed as a vector (i.e., all gridpoints in the x-y grid). This mode for describing the native grid will continue to work with v2.0.a3+, but only if the native grid is in GRIB or binary format. For NetCDF and HDF formats, the isize and jsize args must match the X and Y dimensions of the native grid.
WARNING: The use of PDEF FILE may be incorrect! A long-standing bug and incomplete documentation has led to different conventions for the native grid offset values in the pdef file for GRIB and non-GRIB data formats. For GRIB (1&2), the offsets must be 0-based; the first grid point is assumed to have (i,j) values of (0,0). For all other data types, the offsets must be 1-based; the first grid point is assumed to have (i,j) values of (1,1). Thus:

     for GRIB format: valid offset values are >= 0 and < isize*jsize

     for other formats: valid offset values are > 0 and <= isize*jsize

To maintain backward compatibility, the bug will remain in GrADS as a feature, but the use of PDEF FILE has been deprecated as of version 2.0.a3 and a warning message will be displayed when a data set is opened that uses PDEF FILE. Note that if you use [grib2ctl](http://www.cpc.ncep.noaa.gov/products/wesley/grib2ctl.html) or [g2ctl](http://www.cpc.ncep.noaa.gov/products/wesley/g2ctl.html) to generate your pdef file, the offsets are correct.
Native grid offset values of -999 indicate not to use an input point for that portion of the calculation (thus you can apply less than the "num" number of interpolation points for some of the points).
See additional notes in the paragraphs below.


## How PDEF grid interpolation works

To illustrate how the data is interpolated from the native grid to the rectilinear grid, let's consider an example. Here are a set of relevant records from a descriptor file:

```text
PDEF 100 100 nps ...
XDEF 181 linear -180 1
YDEF 90 linear 0 1
```

These three entries describe data on a native 100x100 North Polar stereographic projection and a rectilinear lat/lon grid that is 181 by 90 and has an interval of 1 degree in both lat and lon. Consider one point within the rectilinear grid, the point -90,40. GrADS calls an internal routine to calculate the i and j values in the native grid that correspond to this lat/lon point. Let's say we get i,j values of 31.24 and 67.88. To do the interpolation to the lat/lon point -90,40, GrADS uses the data values from the following four native grid points: 31,67 - 31,68 - 32,67 - 32,68. Bi-linear interpolation is used within this grid box to get down to the position 31.24,67.88. The interpolation is linear within the i,j grid. When a descriptor file is opened that contains a PDEF record, GrADS calculates the i/j values in the native grid that correspond to the lat/lon pair for each gridpoint in the rectilinear grid. 


## How PDEF wind rotation works

There is a third value calculated for every lat/lon point, and that is the wind rotation value. With some "pre-projected" or native grids, the winds are given relative to the i/j space and not the lat/lon space. To insure correct interpolation, the winds must be rotated to lat/lon space. This is done by determining a rotation amount for each lat/lon point. When u or v wind components are displayed, the values are not just interpolated but also rotated. 
To do the wind rotation properly, GrADS requires both the u and v components. Even if you are just displaying u, GrADS has to retrieve (internally) both the u and v component in order to do the rotation calculation. GrADS determines how to match u and v variables by checking the units field of the variable record in the descriptor file. The u variable must have a units value of 33, and the v variable must have a units value of 34. (This is the GRIB convention). If there are more than one u/v pairs, secondary units values are used. For example:


```text
u
18
33,100
U-Wind Components on Pressure Levels


v
18
34,100
V-Wind Components on Pressure Levels


u10
0
33,105
10 Meter U Wind


v10
0
34,105
10 Meter V Wind

```

might be some variable records in the descriptor file. If wind rotation is called for, u and v would be paired, and u10 and v10 would be paired (since the secondary values would be checked, ie, the 105,100 values).


## The PDEF BILIN option

When a descriptor file is opened that contains a PDEF record, we have explained that GrADS internally generates three grids, each one the size of the rectilinear lat/lon grid. The first two grids contain the i and j values (respectively) from the native grid that correspond to each grid point in the rectilinear grid; the third grid contains wind rotation values. But this only works for a small set of well-defined native grids. GrADS will generate these three internal grids automatically for polar stereographic, Lambert conformal, and some eta grids. If the native grid for your data is not one of the predefined projections, it is still possible for GrADS to handle the data. All you have to do is supply these three grids to GrADS with a supplementary data file and use the bilin option in your PDEF record. 
The supplementary file will contains three lat-lon floating-point grids sized according to the XDEF and YDEF records in the descriptor file.The three grids contain: i values, j values, and wind rotation values. A value of -999 in the i-value grid indicates not to interpolate to that lat-lon point (will end up missing on output) and a value of -999 in the wind-rotation grid indicates not to do wind rotation for that point. If the wind-rotation grid is all -999 values, no rotation is ever done and a flag is set not to even attempt rotation.


## The PDEF GENERAL option and the PDEF FILE option

All of the PDEF examples discussed so far involve the same method for grid interpolation: a grid point value in the rectilinear grid is calculated by finding the four neighboring grid points in the native grid and averaging them, with weights applied bi-linearly according to their proximity to the to rectilinear grid point. The PDEF GENERAL option and the PDEF FILE option generalize this method so that an arbitrary number of native grid point values and their weights are averaged to generate the interpolated rectilinear grid point values. The index values for the native grid values that are to be used and their weights are specified by the user in a supplementary data file (fname). The FILE and GENERAL options are identical except for one detail: they have different conventions for the native grid offset values in the supplementary file (see the "Notes" in the syntax tables above for specifics).


The num argument in the PDEF FILE entry specifies the number of native grid points that will be used to calculate each interpolated rectilinear grid point value. For each num, the supplementary data file will contain two grids -- both will be the size of the rectilinear grid (as defined by XDEF and YDEF). The first grid contains the index or offset values that point to the native grid value that will be used in the interpolation; the second grid contains the weights for those native grid values. The first grid contains integer values, the second grid contains floating-point values. Finally, the supplementary data file must also contain one grid of floating-point wind rotation values. Thus if num equals 1, there will be 3 grids in fname. If num equals 3, there will be 7 grids in fname (3 sets of 2 grids plus the wind rotation grid).
To do the grid interpolation, GrADS will first read the data in the native grid (vector) along with the values in the supplementary grids. To calculate the interpolated value for a particular lat-lon point, GrADS will get num native grid point values, multiply each by their weight, sum over all the weighted native grid points, and divide by the sum of the weights.
An Example:
The original data are set up as a vector of land points only, taken from a 1-degree lat/lon grid. There are 15238 land points in the native grid (vector). We use the PDEF FILE option to repopulate a rectilinear lat/lon grid with the land values, leaving the ocean grid points as missing. In this case, there is no interpolation done. The PDEF option is used simply to convert a vector of land points into a 2D grid with ocean points missing. The descriptor file looks like this:

 ```text
DSET ^gswp_vegetation_parameters.nc
DTYPE netcdf
TITLE Monthly Vegetation parameters at 1x1 degree
UNDEF 1.e+20
PDEF 15238 1 file 1 stream binary ^gswp.filepdef
XDEF 360 linear -179.5 1
YDEF 150 linear -59.5 1
ZDEF 1 linear 1 1
TDEF 204 linear 00Z01jan1982 1mo
VARS 1
NDVI=>ndvi 0 t,x Monthly vegetation index
ENDVARS
```

The supplementary file gtd.filepdef contains three grids -- the first contains the index values that associate each location in the lat/lon grid with it's point in the vector. All of the ocean points will have a missing value of -999. The second grid will contain the weights, which will be 1 for land points, 0 for ocean points. The third grid will contain all missing values since wind rotation is not a issue in this example. Here is a descriptor file for the supplementary data file (a useful strategy for making sure you've got everything written out correctly):

```text
DSET ^gswp.filepdef
TITLE PDEF file for GSWP Vegetation Parameters
UNDEF -999
XDEF 360 linear -179.5 1
YDEF 150 linear -59.5 1
ZDEF 1 linear 1 1
TDEF 1 linear 00z01jul1982 3hr
VARS 3
i 0 -1,40,4 Index Values
w 0 99 Weights
r 0 99 Wind Rotation Values
ENDVARS
```

 
