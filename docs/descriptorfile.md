---
title: Data descriptor file
---

(TOP)=
# Components of a GrADS Data Descriptor File

A GrADS data descriptor file describes the organization and metadata of a data set. Each entry is documented below.

| Group | Entries |
| --- | --- |
| Data files and type | [DSET](#DSET), [CHSUB](#CHSUB), [DTYPE](#DTYPE), [INDEX](#INDEX), [STNMAP](#STNMAP), [TITLE](#TITLE), [UNDEF](#UNDEF), [UNPACK](#unpack), [CACHESIZE](#CACHESIZE), [OPTIONS](#OPTIONS) |
| Binary-file layout | [FILEHEADER](#FILEHEADER), [THEADER](#THEADER), [HEADERBYTES](#HEADERBYTES), [TRAILERBYTES](#TRAILERBYTES), [XYHEADER](#XYHEADER), [XYTRAILER](#XYTRAILER) |
| BUFR station coordinates | [XVAR](#XVAR), [YVAR](#YVAR), [ZVAR](#ZVAR), [STID](#STID), [TVAR](#TVAR), [TOFFVAR](#TOFFVAR) |
| Grid dimensions | [PDEF](#PDEF), [XDEF](#XDEF), [YDEF](#YDEF), [ZDEF](#ZDEF), [TDEF](#TDEF), [EDEF](#EDEF) |
| Variables and metadata | [VECTORPAIRS](#VECT), [VARS](#VARS), [ENDVARS](#ENDVARS), [attribute metadata](#ATTR), [comments](#COMMENT) |

(DSET)=
## `DSET`

```text
DSET data_filename
```

This entry specifies the filename of the data file being described. If the data and the descriptor file are not in the same directory, then *data_filename* must include a full path. If a ^ character is placed in front of *data_filename*, then *data_filename* is assumed to be relative to the path of the descriptor file. If you are using the ^ character in the DSET entry, then the descriptor file and the data file may be moved to a new directory without changing any entries in the data descriptor file, provided their relative paths remain the same. For example:

For example, if the descriptor and binary data files are
`/data/wx/grads/sa.ctl` and `/data/wx/grads/sa.dat`, respectively, the
descriptor may use a relative path:

```text
DSET ^sa.dat
```

instead of the full path:

```text
DSET /data/wx/grads/sa.dat
```

If *data_filename* does not include a full path or a ^, then GrADS will only look for data files in the directory where you are running GrADS.

GrADS allows you to use a single DSET entry to aggregate multiple data files and handle them as if they were one individual file. The individual data files must be identical in all dimensions except time, and the time range of each individual file must be indicated in its filename. To accomplish this, the DSET entry has a substitution template instead of a filename. See the section on [Using Templates](templates.md) for a description of all the possible components of the template. Second, the [OPTIONS](#OPTIONS) entry must contain the template keyword.

(CHSUB)=
## `CHSUB`

```text
CHSUB t1 t2 string
```

(GrADS version 1.9b4) This entry is used with a templating option that allows any user-specified string substitution instead of only date-string substitution. This is useful when none of the standard template options match the time ranges in the files you wish to aggregate, or when files are located on different disks. When the [`DSET`](#DSET) entry contains the `%ch` template, the descriptor file must also contain one or more `CHSUB` entries. The *string* is substituted for `%ch` for the time steps beginning with *t1* and ending with *t2*. See [Using Templates](templates.md) for examples.

(DTYPE)=
## `DTYPE`

```text
DTYPE keyword
```

The `DTYPE` entry specifies the type of data being described. If `DTYPE` is omitted, GrADS assumes the file contains [gridded binary data](aboutgriddeddata.md).

| Keyword | Description |
| --- | --- |
| bufr | (GrADS version 1.9) Data file is a BUFR station data file. This data type must be accompanied by the following special entries: [XVAR](#XVAR), [YVAR](#YVAR), [TVAR](#TVAR), [STID](#STID). Optional special entries are: [ZVAR](#ZVAR), [TOFFVAR](#TOFFVAR). |
| grib | Data file is an indexed GRIB (version 1) file. This data type requires a secondary entry in the descriptor file: [INDEX](#INDEX). The [INDEX](#INDEX) entry provides the filename (including the full path or a ^) for the GRIB index file. The index file is created by the [gribmap](gradutilgribmap.md) utility. You must run [gribmap](gradutilgribmap.md) and create the index file before you can display the GRIB data in GrADS. |
| grib2 | (GrADS version 2.0) Data file is an indexed GRIB2 file. This data type requires an [`INDEX`](#INDEX) entry containing the GRIB2 index filename, including a full path or `^`-relative path. Run [gribmap](gradutilgribmap.md) to create the index before displaying the data. |
| hdfsds | (GrADS version 1.9) Data file is an HDF Scientific Data Set (SDS). Although HDF-SDS files are self-describing and may be read automatically using the [sdfopen](gradcomdsdfopen.md)/[xdfopen](gradcomdxdfopen.md) commands, this DTYPE gives you the option of overriding the file's own metadata and creating a descriptor file for some or all of the variables in the file. This DTYPE may also be used if the metadata in the HDF-SDS file is insufficient or is not coards-compliant. This data type requires a special entry in the *units* field of the [variable declaration.](#VARS) The [undef](#UNDEF) and [unpack](#unpack) entries contain special options for this dtype. |
| hdf5_grid | (GrADS version 2.0.a7+) Data file is HDF5 gridded format. The HDF5 format is extremely general and is designed to store a variety of data types. The GrADS interface is only for grids, and requires a complete descriptor file -- there is no sdfopen/xdfopen interface for HDF5. |
| netcdf | (GrADS version 1.9) Data file is NetCDF. Although NetCDF files are self-describing and may be read automatically using the [sdfopen](gradcomdsdfopen.md)/[xdfopen](gradcomdxdfopen.md) commands, this DTYPE gives you the option of overriding the file's own metadata and creating a descriptor file for some or all of the variables in the file. This DTYPE may also be used if the metadata in the NetCDF file is insufficient or is not coards-compliant. This data type requires a special entry in the *units* field of the [variable declaration.](#VARS) The [undef](#UNDEF) and [unpack](#unpack) entries contain special options for this dtype. |
| station | Data file is in GrADS station data format. This data type requires a secondary entry in the descriptor file: STNMAP. The STNMAP entry provides the filename (including the full path or a ^) for the station data map file. The map file is created by the [stnmap](stnmap.md) utility. You must run [stnmap](stnmap.md) and create the map file before you can display the station data in GrADS. |

(INDEX)=
## `INDEX`

```text
INDEX filename
```

This entry specifies the name of the GRIB map file. It is required when using `DTYPE grib` or `DTYPE grib2`. Generate the file with [gribmap](gradutilgribmap.md). Filename conventions are the same as for [`DSET`](#DSET).

(STNMAP)=
## `STNMAP`

```text
STNMAP filename
```

This entry specifies the name of the station map file. It is required when using the [DTYPE](#DTYPE) station entry to read GrADS-formatted station data. The file is generated by running the external utility [stnmap](stnmap.md). Filenaming conventions are the same as those described for the [DSET](#DSET) entry.

(TITLE)=
## `TITLE`

```text
TITLE string
```

This entry gives a brief description of the contents of the data set. *String* is included in the output from a [query](gradcomdquery.md) command. It may also be exposed by services that publish the data set, so use a meaningful title and avoid double quotation marks (`"`).

(UNDEF)=
## `UNDEF`

```text
UNDEF value [undef_attribute_name [secondary_undef_attribute_name]]
```

This entry specifies the undefined or missing data value. `UNDEF` is required even if the data contain no undefined values. GrADS operations and graphics routines ignore data with this value.

(GrADS version 1.9b4) An optional second argument has been added for data sets of [DTYPE](#DTYPE) netcdf or hdfsds -- it is the name of the attribute that contains the undefined value. This should be used when individual variables in the data file have different undefined values. After data I/O, the missing values in the grid are converted from the variable undef to the file-wide undef (the numerical value in the first argument of the UNDEF record). Then it appears to GrADS that all variables have the same undef value, even if they don't in the original data file. If the data require a transformation using the attributes named in the [UNPACK](#unpack) entry, GrADS assumes the variable undef value corresponds to the data values as they appear in the file, i.e., *before* they are transformed using a scale factor and offset. Missing packed data values are thus assigned the file-wide undef value and are never unpacked. Attribute names are case sensitive, and it is assumed that the name is identical for all variables in the netcdf or hdfsds data file. If the name given does not match any attributes, or if no name is given, the file-wide undef value will be used.

(GrADS version 2.1.0) An optional third argument has been added for data sets of [DTYPE](#DTYPE) netcdf or hdfsds -- it is the name of a secondary attribute that contains another undefined value.

```text
UNDEF 1e+33 missing_value _FillValue
```

(unpack)=
## `UNPACK`

```text
UNPACK scale_factor_attribute_name [add_offset_attribute_name]
```

(GrADS version 1.9) This entry is used with [DTYPE](#DTYPE) netcdf, hdfsds, or hdf5_grid (GrADS version 2.0.a7+) for packed, non-floating-point data that must be converted to floating point:

```text
y = x * scale_factor + add_offset
```

If your self-describing file does not have an offset attribute, the 2nd argument may be omitted, and the offset will be assigned the default value of 0.0. If your self-describing file has an offset attribute, but not a scale factor, use "NULL" for the *scale_factor_attribute_name*. (This "NULL" option is in GrADS version 2.0.0+). Attribute names are case sensitive, and it is assumed that the names are identical for all variables in the netcdf or hdfsds data file. If the names given do not match any attributes, the scale factor will be assigned a value of 1.0 and the offset will be assigned a value of 0.0. The transformation of packed data is done after the undef test has been applied.

```text
UNPACK scale_factor add_offset
UNPACK NULL add_offset
UNPACK Slope Intercept
```

(FILEHEADER)=
## `FILEHEADER`

```text
FILEHEADER length
```

This optional entry tells GrADS that your data file has a header record of *length* bytes that precedes the data. GrADS will skip past this header, then treat the remainder of the file as though it were a normal GrADS binary file after that point. This optional descriptor file entry is only valid for GrADS gridded data sets.

(THEADER)=
## `THEADER`

```text
THEADER length
```

These two equivalent optional entries tell GrADS that the data file has a header record of *length* bytes preceding each time block of binary data. Use one or the other but not both. These entries are only valid for GrADS gridded data sets. See the <a href="aboutgriddeddata.html#structure">structure of a gridded binary data file</a> for more information.

(HEADERBYTES)=
## `HEADERBYTES`

```text
HEADERBYTES length
```

`HEADERBYTES` is an alias for [`THEADER`](#THEADER). Use one of these entries, not both.

(TRAILERBYTES)=
## `TRAILERBYTES`

```text
TRAILERBYTES length
```

This optional entry tells GrADS that the data file has a trailer record of *length* bytes following each time block of binary data. This entry is only valid for GrADS gridded data sets. See the <a href="aboutgriddeddata.html#structure">structure of a gridded binary data file</a> for more information.

(XYHEADER)=
## `XYHEADER`

```text
XYHEADER length
```

This optional entry tells GrADS that the data file has a header record of *length* bytes preceding each horizontal grid (XY block) of binary data. This entry is only valid for GrADS gridded data sets. See the <a href="aboutgriddeddata.html#structure">structure of a gridded binary data file</a> for more information.

(XYTRAILER)=
## `XYTRAILER`

```text
XYTRAILER length
```

(GrADS version 2.1.1.b0+) This optional entry tells GrADS that the data file has a trailer record of *length* bytes following each horizontal grid (XY block) of binary data. This entry is only valid for GrADS gridded data sets. See the <a href="aboutgriddeddata.html#structure">structure of a gridded binary data file</a> for more information.

(XVAR)=
## `XVAR`

```text
XVAR x,y
```

(GrADS version 1.9) This entry provides the x,y pair for the station's longitude. This entry is required for [DTYPE](#DTYPE) bufr.

(YVAR)=
## `YVAR`

```text
YVAR x,y
```

(GrADS version 1.9) This entry provides the x,y pair for the station's latitude. This entry is required for [DTYPE](#DTYPE) bufr.

(ZVAR)=
## `ZVAR`

```text
ZVAR x,y
```

(GrADS version 1.9) This entry provides the x,y pair for the station data's vertical coordinate (e.g., pressure). This is an optional entry for [DTYPE](#DTYPE) bufr.

(STID)=
## `STID`

```text
STID x,y
```

(GrADS version 1.9) This entry provides the x,y pair for the station ID. This entry is required for [DTYPE](#DTYPE) bufr.

(TVAR)=
## `TVAR`

```text
TVAR yr x,y mo x,y dy x,y hr x,y mn x,y sc x,y
```

(GrADS version 1.9) This entry provides the x,y pairs for all the **base time** coordinate variables. Each time unit (year=yr, month=mo, day=dy, hour=hr, minute=mn, second=sc) is presented as a 2-letter abbreviation followed by the x,y pair that goes with that time unit. The time for any individual station report is the base time plus the offset time (see [TOFFVAR](#TOFFVAR)). All six base time units are not required to appear in the TVAR record, only those that are in the data file. This entry is required for [DTYPE](#DTYPE) bufr.

(TOFFVAR)=
## `TOFFVAR`

```text
TOFFVAR yr x,y mo x,y dy x,y hr x,y mn x,y sc x,y
```

(GrADS version 1.9) This entry provides the x,y pairs for all the **offset time** coordinate variables. The syntax is the same as [TVAR](#TVAR). The time for any individual station report is the base time plus the offset time. All six offset time units are not required to appear in the TOFFVAR record, only those that are in the data file. This is an optional entry for [DTYPE](#DTYPE) bufr.

(CACHESIZE)=
## `CACHESIZE`

```text
CACHESIZE bytes
```

(GrADS version 2.0.a8+) This entry overrides the default size of the cache for reading HDF5 or NetCDF4 files. It is not relevant for other data types. It should not be necessary to set the cache size explicitly unless the data file has especially large chunks. Please see the documentation on [compression](compression.md).

(OPTIONS)=
## `OPTIONS`

```text
OPTIONS keyword [keyword ...]
```

This entry controls various aspects of the way GrADS interprets the raw data file. It replaces the old FORMAT record. The *keyword* argument may be one or more of the following:

| Keyword | Description |
| --- | --- |
| pascals | (GrADS version 2.0) (For DTYPE grib2 only) Indicates that pressure values that appear in the descriptor file (in the ZDEF entry and in the GRIB2 codes in the variable declarations) are given in units of Pascals. The [gribmap](gradutilgribmap.md) utility requires pressure to be given in Pascals. If this keyword is present, the pressure level values will be converted to millibars after the gribmap index is generated and the descriptor file is opened with GrADS. If this keyword is omitted, pressure levels will remain in Pascals, and many of the internal functions (which assume a vertical dimension in units of millibars) will not work properly. |
| yrev | Indicates that the Y dimension (latitude) in the data file has been written in the reverse order from what GrADS assumes. An important thing to remember is that GrADS still presents the view that the data goes from south to north. The YDEF statement does not change; it still describes the transformation from a grid space going from south to north. The reversal of the Y axis is done as the data is read from the data file. |
| zrev | Indicates that the Z dimension (pressure) in the data file has been written from top to bottom, rather than from bottom to top as GrADS assumes. The same considerations as noted above for yrev also apply. |
| template | Indicates that a template for multiple data files is in use. For more information, see the section on [Using Templates](templates.md). |
| sequential | Indicates that the file was written in sequential unformatted I/O. This keyword may be used with either station or gridded data. If your gridded data is written in sequential format, then each record must be an X-Y varying grid. If you have only one X and one Y dimension in your file, then each record in the file will be one element long (it may not be a good idea to write the file this way). |
| 365_day_calendar | Indicates the data file was created with perpetual 365-day years, with no leap years. This is used for some types of model output. |
| byteswapped | Indicates the binary data file is in reverse byte order from the normal byte order of your machine. Putting this keyword in the OPTIONS record of the descriptor file tells GrADS to swap the byte order as the data is being read. May be used with gridded or station data. |
| big_endian | Indicates the data file contains 32-bit IEEE floats created on a big endian platform (e.g., sun, sgi) |
| little_endian | Indicates the data file contains 32-bit IEEE floats created on a little endian platform (e.g., iX86, and dec) |
| cray_32bit_ieee | Indicates the data file contains 32-bit IEEE floats created on a cray. |

For hardware-independent descriptor files, specify the source platform with
`big_endian`, `little_endian`, or `cray_32bit_ieee`. GrADS can then use the
same descriptor after data are moved between machines with different byte
orders.

(PDEF)=
## `PDEF`

```text
PDEF ...
```

PDEF is so powerful it has [its own documentation page](pdef.md).

(XDEF)=
## `XDEF`

```text
XDEF xnum mapping [additional_arguments]
```

This entry defines the grid point values for the X dimension, or longitude. The first argument, *xnum*, specifies the number of grid points in the X direction. *xnum* must be an integer \>= 1. *mapping* defines the method by which longitudes are assigned to X grid points. There are two options for *mapping*:

| Mapping | Meaning |
| --- | --- |
| `LINEAR` | Longitudes are generated from a starting value and increment. |
| `LEVELS` | Longitudes are specified individually. |

The LINEAR mapping method requires two additional arguments: *start* and *increment*. *start* is a floating point value that indicates the longitude at grid point X=1. Negative values indicate western longitudes. *increment* is the spacing between grid point values, given as a positive floating point value.

The LEVELS mapping method requires one additional argument, *value-list*, which explicitly specifies the longitude value for each grid point. *value-list* should contain *xnum* floating point values. It may continue into the next record in the descriptor file, but note that records may not have more than 255 characters. There must be at least 2 levels in *value-list*; otherwise use the LINEAR method.

Here are some examples:

| Entry | Count | Mapping | Arguments |
| --- | --- | --- | --- |
| XDEF | 144 | LINEAR | 0.0 2.5 |
| XDEF | 72 | LINEAR | 0.0 5.0 |
| XDEF | 12 | LEVELS | 0 30 60 90 120 150 180 210 240 270 300 330 |
| XDEF | 12 | LEVELS | 15 45 75 105 135 165 195 225 255 285 315 345 |

(YDEF)=
## `YDEF`

```text
YDEF ynum mapping [additional_arguments]
```

This entry defines the grid point values for the Y dimension, or latitude. The first argument, *ynum*, specifies the number of grid points in the Y direction. *ynum* must be an integer \>= 1. *mapping* defines the method by which latitudes are assigned to Y grid points. There are several options for *mapping*:

| Mapping | Meaning |
| --- | --- |
| `LINEAR` | Latitudes are generated from a starting value and increment. |
| `LEVELS` | Latitudes are specified individually. |
| `GAUST62` | Gaussian T62 latitudes. |
| `GAUSR15` | Gaussian R15 latitudes. |
| `GAUSR20` | Gaussian R20 latitudes. |
| `GAUSR30` | Gaussian R30 latitudes. |
| `GAUSR40` | Gaussian R40 latitudes. |

The LINEAR mapping method requires two additional arguments: *start* and *increment*. *start* is a floating point value that indicates the latitude at grid point Y=1. Negative values indicate southern latitudes. *increment* is the spacing between grid point values in the Y direction. It is assumed that the Y dimension values go from south to north, so *increment* is always positive.

The LEVELS mapping method requires one additional argument, *value-list*, which explicitly specifies the latitude for each grid point, from south to north. *value-list* should contain *ynum* floating point values. It may continue into the next record in the descriptor file, but note that records may not have more than 255 characters. There must be at least 2 levels in *value-list*; otherwise use the LINEAR method.

The Gaussian mapping methods require one additional argument: start. This argument indicates the first gaussian grid number. If the data span all latitudes, start would be 1, indicating the southernmost gaussian grid latitude.

Here are some examples:

| Entry | Count | Mapping | Arguments |
| --- | --- | --- | --- |
| YDEF | 73 | LINEAR | -90 2.5 |
| YDEF | 180 | LINEAR | -90 1.0 |
| YDEF | 18 | LEVELS | -85 -75 -65 -55 -45 -35 -25 -15 -5 5 15 25 35 45 55 65 75 85 |
| YDEF | 94 | GAUST62 | 1 |
| YDEF | 20 | GAUSR40 | 15 |

The NCEP/NCAR Reanalysis surface variables are on the GAUST62 grid.

The final example shows that there are 20 Y dimension values which start at Gaussian Latitude 15 (64.10 south) on the Gaussian R40 grid

(ZDEF)=
## `ZDEF`

```text
ZDEF znum mapping [additional_arguments]
```

This entry defines the grid point values for the Z dimension. The first argument, *znum*, specifies the number of pressure levels. *znum* must be an integer \>= 1. *mapping* defines the method by which level values are assigned to Z grid points. There are two options for *mapping*:

| Mapping | Meaning |
| --- | --- |
| `LINEAR` | Levels are generated from a starting value and increment. |
| `LEVELS` | Pressure levels are specified individually. |

The LINEAR mapping method requires two additional arguments: *start* and *increment*. *start* is a floating point value that indicates the level value at grid point Z=1. *increment* is the spacing between grid point values in the Z direction, or from lower to higher. *increment* must be non-zero and non-negative.

The LEVELS mapping method requires one additional argument, *value-list*, which explicitly specifies the pressure level for each grid point in ascending order. *value-list* should contain *znum* floating point values. It may continue into the next record in the descriptor file, but note that records may not have more than 255 characters.

Here are some examples:

| Entry | Count | Mapping | Values |
| --- | --- | --- | --- |
| ZDEF | 7 | LEVELS | 1000 850 700 500 300 200 100 |
| ZDEF | 17 | LEVELS | 1000 925 850 700 600 500 400 300 250 200 150 100 70 50 |

(GrADS version 2.0) (For DTYPE grib2 only) If your Z axis is pressure, the [gribmap](gradutilgribmap.md) utility requires the level values to be given in units of Pascals instead of millibars. Use the "options pascals" keyword to convert the unit of the level values to millibars after the gribmap index is generated and when the descriptor file is opened with GrADS. Pressure level values may remain in Pascals, but then many of the internal functions (which assume a vertical dimension in units of millibars) will not work properly.

(TDEF)=
## `TDEF`

```text
TDEF tnum LINEAR start increment
```

This entry defines the grid point values for the T dimension. The first argument, *tnum*, specifies the number of time steps. *tnum* must be an integer \>= 1. The method by which times are assigned to T grid points is always LINEAR.

*start* indicates the initial time value at grid point T=1. *start* must be specified in the GrADS absolute date/time format:

- *hh*:*mm*Z*ddmmmyyyy*

where:

| Component | Equals | Meaning |
| --- | --- | --- |
| hh | = | hour (two digit integer) |
| mm | = | minute (two digit integer) |
| dd | = | day (one or two digit integer) |
| mmm | = | 3-character month |
| yyyy | = | year (may be a two or four digit integer; 2 digits implies a year between 1950 and 2049) |

If not specified, *hh* defaults to 00, *mm* defaults to 00, and *dd* defaults to 1. The month and year must be specified. No intervening blanks are allowed in the GrADS absolute date/time format.

*increment* is the spacing between grid point values in the T direction. *increment* must be specified in the GrADS absolute time increment format:

- *vvkk*

where:

| Component | Equals | Meaning |
| --- | --- | --- |
| vv | = | an integer number, 1 or 2 digits |
| kk | = | mn (minute)<br>hr (hour)<br>dy (day)<br>mo (month)<br>yr (year) |

Here are some examples:

| Entry | Count | Mapping | Start and increment |
| --- | --- | --- | --- |
| TDEF | 60 | LINEAR | 00Z31dec1999 1mn |
| TDEF | 73 | LINEAR | 3jan1989 5dy |
| TDEF | 730 | LINEAR | 00z1jan1990 12hr |
| TDEF | 12 | LINEAR | 1jan2000 1mo |
| TDEF | 365 | LINEAR | 12Z1jan1959 1dy |
| TDEF | 40 | LINEAR | 1jan1950 1yr |

(EDEF)=
## `EDEF`

```text
EDEF enum NAMES name1 name2 ... nameN
```

The expanded form is:

```text
EDEF enum
ensemble_record_1
ensemble_record_2
...
ensemble_record_enum
ENDEDEF
```

(GrADS version 2.0) This entry defines the ensemble dimension. All ensemble members must have identical X, Y, and Z dimensions, the same list of variables, and the same time axis increment. There are two different syntaxes for the EDEF entry: the first is simpler and requires only the names for each ensemble member, the second expanded form contains a name, individual time axis information, and optional GRIB2 codes.

Both EDEF syntaxes begin with the *enum* argument, an integer \>=1 which specifies the number of ensemble members.

If all of the ensemble members have an identical time axis (i.e. length, initial time, and increment are the same for each one), then it is only necessary to distinguish the ensembles by their names, and the simplified EDEF syntax with the NAMES keyword may be used. A simple space-delimited list of names is all that is required. Ensemble names must have between 1 and 15 alphanumeric characters, lower case only. (In version 2.0.0 and later, mixed case ensemble names are allowed). Some examples are:

| Entry | Count | Mode | Names |
| --- | --- | --- | --- |
| EDEF | 10 | NAMES | 1 2 3 4 5 6 7 8 9 10 |
| EDEF | 12 | NAMES | m01 m02 m03 m04 m05 m06 m07 m08 m09 m10 m11 ensm |
| EDEF | 7 | NAMES | e1 e2 e3 e4 e5 e6 e7 |

When the [OPTIONS](#OPTIONS) TEMPLATE entry is used with EDEF, the ensemble names are used in the %e substitution template to generate the file name. See [Using Templates](templates.md) for more details.

If the ensemble members do not have identical time axes (i.e., their lengths or initial times are not the same), or if you need to include the GRIB2 codes, then you must use the expanded EDEF syntax: a collection of records framed by EDEF and ENDEDEF. The format of the ensemble records is as follows:

```text
ensname length start [grib2_codes]
```

The *ensname* is the 1-15 character "name" for the ensemble member. The *length* is the size of the time axis of the ensemble, which must be less than or equal to the *tnum* argument in the TDEF entry. (The time axis described by [TDEF](#TDEF) must span all the ensemble members.) The *start* argument is the initial time of the ensemble member and must be given in GrADS absolute date/time format. (See [TDEF](#TDEF) for details).

The *grib2 codes* are required if (1) the DTYPE is grib2 and (2) there is more than one ensemble member (*enum* \> 1). The expanded form of the EDEF entry must be used when *grib2 codes* are required, even if the length and start times are the same for all members. For GRIB2 ensembles, support currently exists for four different Product Definition Template (PDT) numbers: 1, 2, 11, and 12. These are grouped into two types: individual ensemble forecasts (PDT 1 and 11) or derived forecasts based on all ensemble members (PDT 2 and 12). For individual ensemble forecasts (PDT 1 and 11), two comma-delimited *grib2 codes* are required: the ensemble type and perturbation number. For derived forecasts based on all ensemble members (PDT 2 and 12), only one *grib2 code* is required: the derived forecast. Clarification of all the GRIB2 nomenclature may be found in the documentation at [WMO](http://www.wmo.ch/web/www/DPS/grib-2.html) and [NCEP](http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc.shtml). Two examples are given below.

The first example illustrates ensemble members with different lengths and start times:

```text
TDEF 591 LINEAR 12z09dec1980 12hr
EDEF 16
```

| Member | Length | Start |
| --- | --- | --- |
| ensm | 591 | 12z09dec1980 |
| m01 | 591 | 12z09dec1980 |
| m02 | 589 | 12z10dec1980 |
| m03 | 587 | 12z11dec1980 |
| m04 | 585 | 12z12dec1980 |
| m05 | 583 | 12z13dec1980 |
| m06 | 571 | 12z19dec1980 |
| m07 | 569 | 12z20dec1980 |
| m08 | 567 | 12z21dec1980 |
| m09 | 565 | 12z22dec1980 |
| m10 | 563 | 12z23dec1980 |
| m11 | 549 | 12z30dec1980 |
| m12 | 547 | 12z31dec1980 |
| m13 | 545 | 12z01jan1981 |
| m14 | 543 | 12z02jan1981 |
| m15 | 541 | 12z03jan1981 |

The declaration ends with `ENDEDEF`.

The second example illustrates the use of GRIB2 codes:

```text
TDEF 31 LINEAR 00z24apr2007 12hr
EDEF 23
```

| Member | Length | Start | GRIB2 codes |
| --- | --- | --- | --- |
| p01 | 31 | 00z24apr2007 | 3,1 |
| p02 | 31 | 00z24apr2007 | 3,2 |
| p03 | 31 | 00z24apr2007 | 3,3 |
| p04 | 31 | 00z24apr2007 | 3,4 |
| p05 | 31 | 00z24apr2007 | 3,5 |
| p06 | 31 | 00z24apr2007 | 3,6 |
| p07 | 31 | 00z24apr2007 | 3,7 |
| p08 | 31 | 00z24apr2007 | 3,8 |
| p09 | 31 | 00z24apr2007 | 3,9 |
| p10 | 31 | 00z24apr2007 | 3,10 |
| p11 | 31 | 00z24apr2007 | 3,11 |
| p12 | 31 | 00z24apr2007 | 3,12 |
| p13 | 31 | 00z24apr2007 | 3,13 |
| p14 | 31 | 00z24apr2007 | 3,14 |
| p15 | 31 | 00z24apr2007 | 3,15 |
| p16 | 31 | 00z24apr2007 | 3,16 |
| p17 | 31 | 00z24apr2007 | 3,17 |
| p18 | 31 | 00z24apr2007 | 3,18 |
| p19 | 31 | 00z24apr2007 | 3,19 |
| p20 | 31 | 00z24apr2007 | 3,20 |
| c00 | 31 | 00z24apr2007 | 1,0 |
| avg | 31 | 00z24apr2007 | 0 |
| spr | 31 | 00z24apr2007 | 2 |

The declaration ends with `ENDEDEF`.

(VECT)=
(VECTORPAIRS)=
## `VECTORPAIRS`

```text
VECTORPAIRS u_component,v_component [u_component,v_component ...]
```

(GrADS version 1.9b4) This entry explicitly identifies vector component pairs. It is needed when data use a native projection other than latitude/longitude (with [`PDEF`](#PDEF)) and winds must be <a href="pdef.html#rotation">rotated</a> from grid-relative to Earth-relative coordinates. GrADS must retrieve both components to perform the rotation.

Using this entry replaces the old technique of putting 33 (for U) or 34 (for V) in the first element of the units field in the variable declaration. The *U-component* and *V-component* arguments should be variable names that appear in the [VARS](#VARS) list. They are separated by a comma, with no spaces. More than one pair of components may be listed; in this case, the pairs should be separated by a space. For example:

```text
VECTORPAIRS u,v u10,v10 uflx,vflx
```

(VARS)=
## `VARS`

```text
VARS varnum
```

`varnum` is the number of variable records between `VARS` and `ENDVARS`.
The records have these forms:

```text
varname levs units description
varname levs additional_codes units description
```

The second form applies to GrADS 2.0.2 and later when additional GRIB2 codes
are required. The syntax of `varname`, `levs`, and `units` depends on the data
type and is described below.

### `varname`

This is a 1-15 character "name" or abbreviation for the data variable. *varname* may contain alphabetic and numeric characters but it must start with an alphabetic character (a-z).

### `varname` (DTYPE netcdf, hdfsds, or hdf5_grid)

(GrADS version 1.9+) For [DTYPE](#DTYPE) netcdf or hdfsds, *varname* may have a different syntax. This syntax is required when the name of the data variable in the SDF does not conform to the GrADS naming conventions (see below for list of criteria), but it may also be used to shorten or change the variable name to make it easier to work with inside GrADS. The syntax is:

```text
SDF_varname=>grads_varname
```

SDF_varname is the name the data variable was given when the SDF file was originally created. For NetCDF files, this name appears in the output from ncdump. It is important that SDF_varname exactly matches the variable name in the data file. SDF_varname may contain uppercase letters and non-alpha-numeric characters.

The classic *varname* syntax (when `SDF_varname=>` is omitted) may be used if SDF_varname meets the criteria for GrADS variable names: it must be less than 16 characters, start with an alphabetic character, and cannot contain any uppercase letters or non-alphanumeric characters.

(GrADS version 2.0.a3+) If the SDF_varname contains spaces, substitute "~" for each space -- the spaces in the variable name string will be swapped back in later after the descriptor file has been parsed.

(GrADS version 2.0.a7+) For dtype hdf5_grid, the SDF_varname may be particularly long since it must contain the names of all the nested groups (separated by "/") to which the data set belongs. For example:

```text
/HDFEOS/GRIDS/EarthSurfaceReflectanceClimatology/Data~Fields/MonthlySurfaceReflectance=>msr
```

### `levs`

This is an integer that specifies the number of vertical levels the variable contains. *levs* may not exceed *znum* as specified in the ZDEF statement. If *levs* is 0, the variable does not correspond to any vertical level. Surface variables (e.g. sea level pressure) have a *levs* value of 0.

For [DTYPE](#DTYPE) station or bufr, surface variables have a *levs* value of 0 and upper air variables have a *levs* value of 1. (Exception to this rule for bufr data: replicated surface variables are given a levs value of 2).

### levs (DTYPE grib2)

(GrADS version 2.0) This is a comma-delimited list of numbers that provide information about the vertical dimension of a variable. The first number in the list is the number of vertical levels the variable contains or zero if the variable doesn't vary in Z. The remaining numbers are the GRIB2 parameters that specify the vertical level or layer. The levs field may contain up to five comma-delimited numbers:

```text
NLEVS,LTYPE,LVAL,LVAL2,LTYPE2
```

where

| Code | Equals | Meaning |
| --- | --- | --- |
| NLEVS | = | The number of vertical levels, or 0 if not Z-varying (Required) |
| LTYPE | = | The level type indicator (Required) |
| LVAL | = | The value of the 1st level (Not Required for all level types) |
| LVAL2 | = | The value of the 2nd level (Only Required for layers between 2 fixed levels) |
| LTYPE2 | = | The level type indicator for the 2nd level (Only required if different from LTYPE) |

If NLEVS \> 0 and is followed only by the LTYPE, the values for LVAL will be determined by the [ZDEF](#ZDEF) entry. If a variable has an NLEVS entry that is \> 0 but less than the number of levels declared in the [ZDEF](#ZDEF) entry, then the values for LVAL will correspond to the first NLEVS values of the Z axis. If LTYPE is 100 (the GRIB2 code for an isobaric surface), the units of LVAL must be Pascals. If the values of LVAL are taken from the ZDEF entry, use [OPTIONS](#OPTIONS) *pascals* to convert the vertical coordinate to millibars once the descriptor file is opened with GrADS. Some level types such as "mean sea level" or "tropopause" do not require an LVAL. In this case, LVAL may be omitted (see the "slp" example below). If two LTYPE entries are required but LVAL and LVAL2 are not, then the LVAL entries may be omitted, with adjacent commas used to indicate missing values (see the "cloud" example below).

Examples:

| Variable | Levels | Units | Description |
| --- | --- | --- | --- |
| hgt | 26,100 | 0,3,5 | Geopotential Height \[gpm\] |
| hgt500 | 0,100,50000 | 0,3,5 | Geopotential Height at 500mb \[gpm\] |
| slp | 0,101 | 0,3,1 | Sea Level Pressure \[Pa\] |
| t2m | 0,103,2 | 0,0,0 | 2-meter Temperature \[K\] |
| soilt1 | 0,106,0,0.1 | 0,0,0 | Soil Temp, 0-0.10m below surface \[K\] |
| cloud | 0,1,,,8 | 0,6,1 | Total Cloud Cover, from surface to TOA \[%\] |

The external utilities [grib2scan](gradutilgrib2scan.md) and [wgrib2](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/index.html) are quite useful in determining what the values for the *levs* field should be for a GRIB2 data file.

### additional_codes (DTYPE grib2) (optional)

(GrADS version 2.0.2+) This field specifies any additional GRIB2 codes that are required to uniquely identify the record when the elements in the levs and units fields are not sufficient. It is only required for certain Product Definition Templates: the Probability Forecasts (PDT 5 and 9), Percentiles (PDT 6 and 10), and the Optical Properties of Aerosol (PDT 48). The *additional_codes* field always begins with the letter "a" (for "additional") followed by a set of comma-delimited numbers. The quantity and meaning of the numbers depends on the product.

### additional_codes (PDT 5 or 9)

For the Probability forecasts (PDT 5 or 9), the *additional_codes* field has 2 or 3 comma-delimited numbers, preceded by the letter "a" :

```text
aPTYPE,LIMIT,LIMIT2
```

where | Code | Equals | Meaning |
| --- | --- | --- |
| PTYPE | = | The probability type indicator |
| LIMIT | = | The value of the limit (for probabilities above or below the given limit) |
| LIMIT2 | = | The value of the 2nd limit (for probabilities between the 2 given limits)<br>(only needed for PTYPE=2) |

Examples:

| Variable | Levels | Additional codes | Units | Description |
| --- | --- | --- | --- | --- |
| pt2m273 | 0,103,2 | a0,273 | 0,0,0 | Prob. of 2-m Temp below 273 |
| pcape250 | 0,1,0 | a1,250 | 0,7,6 | Prob. of CAPE above 250 |
| pcape500 | 0,1,0 | a1,500 | 0,7,6 | Prob. of CAPE above 500 |
| pcape1000 | 0,1,0 | a1,1000 | 0,7,6 | Prob. of CAPE above 1000 |
| pcsnow1 | 0,1,0 | a2,1,1 | 0,1,195 | Prob. of categor. snow between 1 and 1 |

The external utilities [grib2scan](gradutilgrib2scan.md) (with the -v option) and [wgrib2](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/index.html) are quite useful in determining what the values for the *additional_codes* field should be for a GRIB2 data file.

### additional_codes (PDT 6 or 10)

(Version 2.1.a3+) For the Percentile forecasts (PDT 6 or 10), the *additional_codes* field has only one number preceded by the letter "a" :

```text
aPCTLE
```

where | Code | Equals | Meaning |
| --- | --- | --- |
| PCTLE | = | The percentile value |

Examples:

| Variable | Levels | Additional codes | Units | Description |
| --- | --- | --- | --- | --- |
| t75 | 0,103,2 | a75 | 0,0,0 | 75th percentile of 2-m Temperatures |
| t90 | 0,103,2 | a90 | 0,0,0 | 90th percentile of 2-m Temperatures |

The external utilities [grib2scan](gradutilgrib2scan.md) (with the -v option) and [wgrib2](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/index.html) are quite useful in determining what the values for the *additional_codes* field should be for a GRIB2 data file.

### additional_codes (PDT 48)

For the Aerosol Forecasts (PDT 48), the *additional_codes* field may have up to 7 comma-delimited numbers, preceded by the letter "a":

```text
aATYPE,STYPE,S1,S2,WTYPE,W1,W2
```

where | Code | Equals | Meaning |
| --- | --- | --- |
| ATYPE | = | The aerosol type indicator |
| STYPE | = | The type of interval for the first and second size |
| S1 | = | The first size (in meters) |
| S2 | = | The second size (in meters) |
| WTYPE | = | The type of interval for the first and second wavelength |
| W1 | = | The first wavelength (in meters) |
| W2 | = | The second wavelength (in meters) |

The ATYPE code is always required. If the STYPE is non-missing, then the trio of numbers STYPE,S1,S2 must be included in the *additional_codes* field. Similarly, if WTYPE is non-missing, then the trio of numbers WTYPE,W1,W2 must be included in the *additional_codes* field. If both STYPE and WTYPE are non-missing, then all six codes must be present in the order listed above.

Examples:

| Variable | Levels | Additional codes | Units | Description |
| --- | --- | --- | --- | --- |
| aotk | 0,10,0 | a62000,0,2e-5,0,7,5.45e-7,5.65e-7 | 0,0,0 | \*desc1 |
| aemflx | 0,10,0 | a62001,0,2e-05,0 | 0,20,3 | \*desc2 |

\*desc1=Total Aerosol Optical Thickness, size \< 2e-5, wavelength \>= 5.45e-7 and \<= 5.65e-7
\*desc2=Atmosphere Emission Mass Flux for Dry Dust, size is \< 2e-5

The external utilities [grib2scan](gradutilgrib2scan.md) (with the -v option) and [wgrib2](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/index.html) are quite useful in determining what the values for the *additional_codes* field should be for a GRIB2 data file.

### `units`

For flat binary files containing 4-byte floating-point data that are not pre-projected, this field is ignored but must be included. Put in a value of 99.

### units (DTYPE bufr)

(GrADS version 1.9) For [DTYPE](#DTYPE) bufr files, this field contains the x,y pair for the named variable.

### units (DTYPE grib)

For [DTYPE](#DTYPE) grib, the *units* field specifies the GRIB parameters of the variable. This information is used by the [gribmap](gradutilgribmap.md) utility for mapping the variables listed in the descriptor file to the data records in the GRIB files. This parameter may contain up to four comma-delimited numbers:

```text
VV,LTYPE,LVAL,TRI
VV,LTYPE,LVAL,LVAL2
```

where,

| Code | Equals | Meaning |
| --- | --- | --- |
| VV | = | The GRIB parameter number (Required) |
| LTYPE | = | The level type indicator (Required) |
| LVAL | = | The value of the 1st level (Required if NLEVS=0) |
| LVAL2 | = | The value of the 2nd level (Optional) |
| TRI | = | The "time range indicator" (Optional) |

The external utilities [gribscan](gradutilgribscan.md) and [wgrib](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib.html) are quite useful in determining what the values for the *units* field should be for a GRIB data file. Examples:

| Variable | Levels | Units | Description |
| --- | --- | --- | --- |
| u | 39 | 33,100 | U Winds \[m/s\] |
| t | 39 | 11,100 | Temperature \[K\] |
| ts | 0 | 11,1 | Surface Temperature \[K\] |
| tb | 0 | 11,116,60,30 | Temperature, 30-60mb above surface \[K\] |
| dpt | 0 | 17,100,1000 | Dew Point Temperature at 1000 mb \[K\] |

### units (DTYPE grib2)

(GrADS version 2.0) This is a comma-delimited list of values that identify a GRIB2 parameter (variable):

```text
DISC,CAT,NUM,SP,SP2
```

where,

| Code | Equals | Meaning |
| --- | --- | --- |
| DISC | = | The parameter Discipline (Required) |
| CAT | = | The parameter Category (Required) |
| NUM | = | The parameter Number (Required) |
| SP | = | The Statistical Process used to derive the parameter<br>(May be required if parameter is not an instantaneous value) |
| SP2 | = | The Spatial Process used to interpolate the parameter<br>(Required only for Product Definition Template 4.15) |

Some examples are:

| Variable | Levels | Units | Description |
| --- | --- | --- | --- |
| u | 26,100 | 0,2,2 | U-Component of Wind \[m/s\] |
| v | 26,100 | 0,2,3 | V-Component of Wind \[m/s\] |
| t2max | 0,103,2 | 0,0,5 | 2-meter Temperature Maximum \[K\] (NCEP) |
| t2max | 0,103,2 | 0,0,0,2 | 2-meter Temperature Maximum \[K\] (TIGGE) |
| soilm1 | 0,106,0,0.1 | 2,0,192 | Soil Moisture, 0-0.10m below surface \[K\] |
| catave | 10,100 | 0,19,22,0,3 | Spatial Avg. of Clear Air Turbulence \[%\] |
| catmax | 10,100 | 0,19,22,2,3 | Spatial Max of Clear Air Turbulence \[%\] |

### units (DTYPE netcdf, hdfsds, or hdf5_grid)

(GrADS version 1.9) For [DTYPE](#DTYPE) netcdf or hdfsds or hdf5_grid (GrADS version 2.0.a7+) , the *units* field is a comma-delimited list of the varying dimensions of the variable. Dimensions expressed as x, y, z, or t correspond to the four axes defined by XDEF, YDEF, ZDEF and TDEF. For example, a surface variable such as sea level pressure might look like this:

```text
presSFC=>psfc 0 y,x Surface Pressure
```

A time-varying atmospheric variable such as geopotential height might look like this:

```text
Height=>hght 17 t,z,y,x Geopotential Height (m)
```

The order of the dimensions listed in the *units* field does matter. They must describe the shape of the variable as it was written to the SDF data file. For NetCDF files, this information appears in the output from ncdump next to the variable name.

If your data file contains a variable that also varies in a non-world-coordinate dimension (e.g. histogram interval, spectral band, ensemble number) then you can put a non-negative integer in the list of varying dimensions that will become the array index of the extra dimension. For example:

```text
VAR=>hist0 0 0,y,x First histogram interval for VAR
VAR=>hist1 0 1,y,x Second histogram interval for VAR
VAR=>hist2 0 2,y,x Third histogram interval for VAR
```

Another option in this example would be to fill the unused Z axis with the histogram intervals:

```text
ZDEF 3 LINEAR 1 1
VAR=>hist 3 z,y,x VAR Histogram
```

In this case, it would appear to GrADS that variable 'hist' varies in Z, but the user would have to remember that the Z levels correspond to histogram intervals. The latter technique makes it easier to slice through the data, but is not the most accurate representation. And if you don't have an unused world-coordinate axis available, then you still have a way to access your data*.*

### units (non-standard binary)

For non-standard binary files, the *units* field tells GrADS how to read files that do not conform to the <a href="aboutgriddeddata.html#structure">default structure</a> or do not contain 4-byte floating-point data. GrADS assumes the order, from fastest- to slowest-varying dimension, is longitude (X), latitude (Y), vertical level (Z), variable (VAR), and time (T). For another dimension sequence, use *units* to describe how the data must be unpacked.

For these non-standard binary files, the *units* field is a series of one or more comma-delimited numbers, the first of which is always -1. The syntax is as follows:

```text
-1,structure[,arg]
```

There are four options for *structure*, outlined below. Some of these options have additional attributes which are specified with *arg*.

| Units value | Meaning |
| --- | --- |
| -1,10,*arg* | (GrADS 1.9 or earlier) This option indicates that "VAR" and "Z" have been transposed in the dimension sequence. The order is: longitude (X), latitude (Y), variable (VAR), vertical level (Z), time(T). Thus, all variables are written out one level at a time. This feature was designed to be used with NASA GCM data in the "phoenix" format. The upper air *prognostic* variables were transposed, but the *diagnostic* variables were not. Thus an *arg* of 1 means the variable has been var-z transposed, and an *arg* of 2 means the variable has not. |
| -1,20 | This option indicates that "VAR" and "T" have been transposed in the dimension sequence. The order is: longitude (X), latitude (Y), vertical level (Z), time(T), variable (VAR). Thus, all times for one variable are written out in order followed by all times for the next variable, etc. Data files for which "VAR" and "T" have been transposed may not be templated together. |
| -1,30 | (GrADS 1.9 or earlier) This option handles the cruel and unusual case where X and Y dimensions are transposed and the horizontal grids are (lat,lon) as opposed to (lon,lat) data. This option causes GrADS to work very inefficiently. However, it is useful for initial inspection and debugging. |
| -1,40,*arg* | This option handles non-float data. If there are multiple variables in the same file, they must all be the same type. The dimension sequence is assumed to be the default. The secondary *arg* tells GrADS what type of data values are in the binary file:<br>- *units* = -1,40,1     = 1-byte unsigned chars (0-255)<br>*units* = -1,40,2     = 2-byte unsigned integers<br>*units* = -1,40,2,-1 = 2-byte signed integers<br>*units* = -1,40,4     = 4-byte integers |

### units (pre-projected wind components)

For pre-projected vector components that require [`PDEF`](pdef.md) and [wind rotation](pdef.md#how-pdef-wind-rotation-works), GrADS must retrieve both the U and V component. The recommended way to match components is the [`VECTORPAIRS`](#VECT) entry. Before version 1.9b4, the variable record's *units* field was used instead: U had a value of 33 and V a value of 34, with secondary values distinguishing multiple pairs.

### `description`

This is text description or long name for the variable. Max 140 characters.

(ENDVARS)=
## `ENDVARS`

```text
ENDVARS
```

`ENDVARS` terminates the variable declarations and must be the final record in the descriptor file.

(ATTR)=
## Attribute metadata

```text
@ varname attribute_type attribute_name attribute_value
```

(GrADS version 1.9b4) To supplement the metadata in your descriptor file, use attribute comments. The first two characters of the attribute comment must be "@" followed by a space -- this distinguishes it from an ordinary comment (see below). Attribute comments may appear anywhere in the descriptor file, and they will be ignored if used with older versions of GrADS.

All file attributes may be retrieved with the [`query attr`](gradcomdqattr.md) command.

| Field | Meaning |
| --- | --- |
| `varname` | Use `global` for data-set attributes, `lon`, `lat`, `lev`, or `time` for coordinate attributes, or a declared GrADS variable name. For an aliased variable, use the GrADS name rather than the native SDF name. |
| `attribute_type` | One of the case-sensitive types `String`, `Byte`, `Int16`, `UInt16`, `Int32`, `UInt32`, `Float32`, or `Float64`. |
| `attribute_name` | A single word without spaces, such as `units` or `minimum_value`. |
| `attribute_value` | Any value for which the complete attribute entry remains no longer than 512 characters. |

```text
@ precip String units mm/day
@ global String documentation http://put.your.documentation.url.here
```

(COMMENT)=
## Comments

```text
* comment
```

Begin an ordinary comment with `*`. Use `@` for formatted attribute metadata as described above.
