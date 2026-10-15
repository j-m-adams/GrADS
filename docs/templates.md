---
title: Using Templates
---

# Using Templates

GrADS allows you use a single data descriptor file to aggregate multiple data files and handle them as if they were one individual file. The individual data files must be identical in the X, Y, and Z dimensions and have the same list of variables. The time range of each individual file must be indicated it its filename. Beginning with version 2.0, data files may also be aggregated in the ensemble dimension.

First, the DSET entry has a substitution template instead of a filename. See below for a description of all the possible components of the template. Second, the OPTIONS entry contains the `template` keyword. Third, the TDEF entry describes the time range for the entire set of data files.

Templating works on any GrADS data type for which you can write a descriptor file. If you specify any additional OPTIONS keywords in the data descriptor file, make sure the options apply equally to each file included in the template.

You can use the <a href="gradcomdsetmisswarn.html">`set misswarn`</a> command to alert you if any of the data files in the templated set is missing.

## Templating over TIME

Valid components of the substitution template for the TIME axis are:

| Token | Meaning |
| --- | --- |
| `%x1`, `%x3` | One- or three-digit decade. |
| `%y2`, `%y4` | Two- or four-digit year. |
| `%m1`, `%m2` | One-/two-digit month, or two-digit month with a leading zero. |
| `%mc` | Three-character month abbreviation. |
| `%d1`, `%d2` | One-/two-digit day, or two-digit day with a leading zero. |
| `%h1`, `%h2`, `%h3` | One-, two-, or three-digit hour. `%h3` produces values such as `120` or `012`. |
| `%n2` | Two-digit minute, padded with a leading zero. |
| `%f2`, `%f3` | Two- or three-digit forecast hour, padded with zeros as needed; additional digits are added beyond 99 or 999. |
| `%fn2` | Forecast minutes, padded to two digits; additional digits are added beyond 99 (GrADS 2.0.a9+). |
| `%fhn` | Forecast time as hours and minutes (`hhnn`); minutes remain 0–59 and hours increase indefinitely. Values below 10 are zero-padded (GrADS 2.0.a9+). |
| `%fdhn` | Forecast time as days, hours, and minutes (`ddhhnn`); hours remain 0–23 and minutes 0–59. Values below 10 are zero-padded (GrADS 2.0.a9+). |
| `%j3` | Three-digit Julian day (day of year) (GrADS 2.0.a7+). |
| `%t1`–`%t6` | One- through six-digit positive time index. The filename sequence begins with `1`, `01`, `001`, `0001`, `00001`, or `000001` respectively (GrADS 2.0.a7/2.0.a8+). |
| `%tm1`–`%tm6` | One- through six-digit zero-based time index. The filename sequence begins with `0`, `00`, `000`, `0000`, `00000`, or `000000` respectively (GrADS 2.0.a7/2.0.a8+). |

When specifying the initial time (e.g., NWP model output), use these substitutions:

| Token | Meaning |
| --- | --- |
| `%ix1`, `%ix3` | Initial one- or three-digit decade. |
| `%iy2`, `%iy4` | Initial two- or four-digit year. |
| `%im1`, `%im2` | Initial one-/two-digit month, or two-digit month with a leading zero. |
| `%imc` | Initial three-character month abbreviation. |
| `%id1`, `%id2` | Initial one-/two-digit day, or two-digit day with a leading zero. |
| `%ih1`, `%ih2`, `%ih3` | Initial one-, two-, or three-digit hour. |
| `%in2` | Initial two-digit minute with a leading zero when needed. |

### Templating over ENSEMBLE

With the introduction of the extra grid dimension for ensembles in version 2.0, support was also added for file templating over E. The sole substitution template is `%e` and the substitution string is the ensemble name, which is provided in the EDEF entry in the descriptor file. Note that the ensemble names are limited to 15 characters -- keep this limit in mind when designing your data directory structure and file naming conventions (or use symbolic links to create short aliases for longer filenames). If you are templating over the ensemble dimension, there can be only one ensemble member per file. If your data set has an ensemble dimension, and you are using templating over T but not E (i.e., there is no %e in the DSET entry), then all ensemble members are presumed to have identical time axes, and all members must be contained in the data file for a given time. Templating over T but not E is not supported for data sets in flat binary or GRIB1 formats.

### String Substitution

The `%ch` template option, introduced in version 1.9b4, allows for *any* user-specified string substitution, not just date strings. This is useful when none of the above template options match the time ranges in the files you wish to aggregate, or if the files are located on different disk pathnames. The syntax is as follows:

- `%ch   `substitute string

If you put the `%ch` template in your DSET entry, then you also need to put additional <a href="descriptorfile.html#CHSUB">CHSUB</a> entries in the descriptor file that contain two integers (t1 and t2) followed by a string which will be substituted for `%ch` in the data file names for the time steps beginning with
t1 and ending with t2. The CHSUB descriptor file entries have the following syntax:

- `CHSUB  `*`t1`*`  `*`t2`*`  `*`string`*

Version 2.1.a3 adds a new feature to the string substitution template: the *`string`* provided in the `CHSUB` entry may contain time-based template components. GrADS will do the `CHSUB` string substitution *before* the complete filename is generated by resolving all the other template substitution components. An application of this strategy might be to merge a reanalyis and forecast into one seamless time series, but the reanalyes and forecasts have different file naming conventions. See example \#6 below.

### Examples

1. A set of binary files spans one month, with each day's hourly data in a
   separate file:

   ```text
   1may92.dat
   2may92.dat
   ...
   31may92.dat
   ```

   The descriptor file must include the following records; `TDEF` covers the
   entire month:

   ```text
   DSET ^%d1may92.dat
   OPTIONS template
   TDEF 744 linear 0z1may1992 1hr
   ```

2. If the hourly files extend across multiple months and years, include month
   and year substitutions and extend `TDEF`:

   ```text
   1jun92.dat
   2jun92.dat
   ...
   1jan93.dat

   DSET ^%d1%mc%y2.dat
   OPTIONS template
   TDEF 6624 linear 0z1may1992 1hr
   ```

3. For decade-long monthly NetCDF files such as `pr.1880_1889.nc` through
   `pr.1940_1949.nc`, use decade substitutions:

   ```text
   DSET ^pr.%x30_%x39.nc
   OPTIONS template
   DTYPE netcdf
   TDEF 840 linear jan1880 1mo
   ```

4. For files covering different time ranges, use `%ch` and `CHSUB`:

   ```text
   DSET ^pr.%ch.nc
   CHSUB 1 600 1851-1900
   CHSUB 601 1800 1901-2000
   OPTIONS template
   DTYPE netcdf
   TDEF 1800 linear jan1851 1mo
   ```

   If the files are on different disks, substitute their full paths instead:

   ```text
   DSET %ch
   CHSUB 1 600 /disk1/pr.1851-1900.nc
   CHSUB 601 1800 /disk2/pr.1901-2000.nc
   ```

5. For forecast files named `MMOUT_DOMAIN1_00`, `MMOUT_DOMAIN1_01`, and so on,
   use a zero-based time-index substitution:

   ```text
   DSET ^MMOUT_DOMAIN1_%tm2
   ```

6. `%ch` strings can contain other time substitutions. For example:

   ```text
   DSET /data/projects/ops/pub/%ch
   CHSUB 1 1898 das/Y%y4/M%m2/D%d2/GEOS.fp.asm.inst3_2d_smp_Nx.%y4%m2%d2_%h2%n2.V01.nc4
   CHSUB 1899 1909 forecast/Y2014/M10/D15/H06/GEOS.fp.fcst.inst3_2d_smp_Nx.20141015_06+%y4%m2%d2_%h2%n2.V01.nc4
   OPTIONS template
   ```
