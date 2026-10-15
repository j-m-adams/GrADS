---
title: Handling GRIB in GrADS
---

# Handling GRIB in GrADS

## Contents

- [What is GRIB?](#what-is-grib)
- [How to read GRIB data with GrADS](#how-to-read-grib-data-with-grads)
- [GRIB code tables and parameters](#grib-code-tables-and-parameters)
- [An example](#an-example-for-grib1)
- [Comments on GRIB2](#comments-on-grib2)
- [Summary](#summary)

## Introduction

One of the most powerful features of GrADS is its ability to workdirectly with GRIB data, versions 1 and 2. Grads version 2.0 is required to handle GRIB2. The interfaces for GRIB and GRIB2 are similar, but not identical -- they are treated as separate data types. This documentation page will attempt to provide the required understanding to useGRIB data in GrADS.

## What is GRIB?

GRIB (General Regularly-distributed Information in Binary form) is an international, public, binary format for the efficient storage of meteorological/oceanographic variables and the metadata that describe them. GRIB2 is similar to GRIB, but has a more complex set of header fields for the metadata, and also offers data compression that can significantly reduce file size. A GRIB data file typically consists of a collection of records. Each GRIB record contains a 2-D (lon,lat) grid of data at a particular time and vertical level. A 4-D GRIB data set is a collection of 2-D records that span a range of times and vertical levels. GRIB2 records may also contain ensemble information, creating a 5-D data set. A GRIB record is a self-describing data object -- each record contains not only the data, but also the metadata to describe the spatial grid, the valid time, the vertical level, and any ensemble metadata (for GRIB2 only). GRIB records may be concatenated together to form a single data set, but because each record is self-describing, the order in which they may be merged is arbitrary.

## How to read GRIB data with GrADS

In order to display GRIB data in GrADS, the collection of records must be sorted and placed into the internal 4- or 5-D gridded data model. This is accomplished by the use of a GrADS data descriptor file and a separate index file, which maps the position of the GRIB records in the data file into their proper place in the 4- or 5-D grid environment. The general idea is to scan through the metadata in all the records, collecting information about the lat/lon grid, the list of vertical levels at which the records are defined, the list of times at which the records are valid, and (if GRIB2) any ensemble information. The next step is to create a descriptor file which describes the complete grid and the list of variables. The final step is to create the index file by running gribmap, which takes the name of your descriptor file as an argument. The name of the index file that gribmap creates is included in the descriptor file in the INDEX entry. Once you have created a descriptor file and run gribmap, you are ready to open the descriptor file with GrADS and begin displaying the data.

The scanning of the metadata in each record is done by the several external utilities: <a href="gradutilgribscan.html">gribscan</a> and <a href="gradutilgrib2scan.html">grib2scan</a> (which are supported by COLA, as part of GrADS) and [wgrib](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib.html) and [wgrib2](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/) (which are supported by Wesley Ebisuzaki at NOAA). The scanning utilities will give you the information you need to describe the 4- or 5-D grid in your descriptor file, so you can manually create the XDEF, YDEF, ZDEF, TDEF, and EDEF (if ensembles are present) entries, plus the list of variable declarations, with the appropriate codes in the levels and units fields to describe each variable uniquely. If you don't feel up to doing all that work manually, there are two user-friendly routines that do all the scanning and create a descriptor file for you: [grib2ctl](http://www.cpc.ncep.noaa.gov/products/wesley/grib2ctl.html) and [g2ctl](http://www.cpc.ncep.noaa.gov/products/wesley/g2ctl.html) (also supported by Wesley Ebisuzaki at NOAA).

## GRIB code tables and parameters

GRIB messages store metadata fields as numeric codes. The meaning of a code
depends on the GRIB edition and, for some products, on the centre-specific
local table used to create the message. Always check the edition, table
version, centre, and product definition together.

The [ECMWF ecCodes GRIB reference](https://codes.ecmwf.int/grib/) provides an
online index of the WMO GRIB definitions:

| Use | GRIB 1 | GRIB 2 |
| --- | --- | --- |
| Edition overview | [GRIB 1 keys](https://codes.ecmwf.int/grib/format/grib1/) | [GRIB 2 keys](https://codes.ecmwf.int/grib/format/grib2/) |
| Code tables | [GRIB 1 code tables](https://codes.ecmwf.int/grib/format/grib1/ctables/) | [GRIB 2 code tables](https://codes.ecmwf.int/grib/format/grib2/ctables/) |
| Parameter names and units | [ECMWF parameter database](https://codes.ecmwf.int/grib/param-db/) | [ECMWF parameter database](https://codes.ecmwf.int/grib/param-db/) |
| Flags and bit fields | GRIB 1 code tables | [GRIB 2 flags tables](https://codes.ecmwf.int/grib/format/grib2/ftables/) |

For GRIB 1, `kpds5`, `kpds6`, and `kpds7` identify the parameter, level type,
and level value. Their meanings come primarily from Code Table 2 (parameter)
and Code Table 3 (fixed levels or layers). For GRIB 2, the equivalent
information is distributed across sections and product-definition templates:
discipline, parameter category and number, fixed-surface type, scaled values,
and statistical-processing fields must be interpreted together. The [GRIB 2
code-table index](https://codes.ecmwf.int/grib/format/grib2/ctables/) lists
these fields by section and table number.

The [parameter database](https://codes.ecmwf.int/grib/param-db/) is useful for
names, units, and centre-specific definitions, but it does not replace
checking the complete message metadata. Use `wgrib`, `wgrib2`, `gribscan`, or
`grib2scan` to inspect the codes in your files before writing `VARS` entries in
a GrADS descriptor.

(example)=
## An Example (for GRIB1)

```text
# wgrib sample.grib
1:0:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=1000:TR=10:P1=0:P2=0:TimeU=1:1000 mb:anl:NAve=0
2:81534:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=1000:TR=10:P1=0:P2=0:TimeU=1:1000 mb:anl:NAve=0
3:154922:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=850:TR=10:P1=0:P2=0:TimeU=1:850 mb:anl:NAve=0
4:236456:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=850:TR=10:P1=0:P2=0:TimeU=1:850 mb:anl:NAve=0
5:317990:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=500:TR=10:P1=0:P2=0:TimeU=1:500 mb:anl:NAve=0
6:399524:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=500:TR=10:P1=0:P2=0:TimeU=1:500 mb:anl:NAve=0
7:489202:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=200:TR=10:P1=0:P2=0:TimeU=1:200 mb:anl:NAve=0
8:578880:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=200:TR=10:P1=0:P2=0:TimeU=1:200 mb:anl:NAve=0
9:660414:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=1000:TR=10:P1=0:P2=6:TimeU=1:1000 mb:6hr fcst:NAve=0
10:741948:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=1000:TR=10:P1=0:P2=6:TimeU=1:1000 mb:6hr fcst:NAve=0
11:815336:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=850:TR=10:P1=0:P2=6:TimeU=1:850 mb:6hr fcst:NAve=0
12:896870:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=850:TR=10:P1=0:P2=6:TimeU=1:850 mb:6hr fcst:NAve=0
13:978404:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=500:TR=10:P1=0:P2=6:TimeU=1:500 mb:6hr fcst:NAve=0
14:1059938:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=500:TR=10:P1=0:P2=6:TimeU=1:500 mb:6hr fcst:NAve=0
15:1141472:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=200:TR=10:P1=0:P2=6:TimeU=1:200 mb:6hr fcst:NAve=0
16:1231150:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=200:TR=10:P1=0:P2=6:TimeU=1:200 mb:6hr fcst:NAve=0
17:1312684:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=1000:TR=10:P1=0:P2=12:TimeU=1:1000 mb:12hr fcst:NAve=0
18:1394218:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=1000:TR=10:P1=0:P2=12:TimeU=1:1000 mb:12hr fcst:NAve=0
19:1467606:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=850:TR=10:P1=0:P2=12:TimeU=1:850 mb:12hr fcst:NAve=0
20:1549140:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=850:TR=10:P1=0:P2=12:TimeU=1:850 mb:12hr fcst:NAve=0
21:1630674:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=500:TR=10:P1=0:P2=12:TimeU=1:500 mb:12hr fcst:NAve=0
22:1712208:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=500:TR=10:P1=0:P2=12:TimeU=1:500 mb:12hr fcst:NAve=0
23:1793742:d=04040200:UGRD:kpds5=33:kpds6=100:kpds7=200:TR=10:P1=0:P2=12:TimeU=1:200 mb:12hr fcst:NAve=0
24:1883420:d=04040200:VGRD:kpds5=34:kpds6=100:kpds7=200:TR=10:P1=0:P2=12:TimeU=1:200 mb:12hr fcst:NAve=0
```

The wgrib output reveals that the sample.grib file contains 24 records. There are two variables: UGRD and VGRD; there are four pressure levels: 1000, 850, 500, and 200; there are three times: 00hr analysis, 6hr forecast, and 12hr forecast, with the initial reference time of 00z 2 April 2004 ("04040200"). We still need to know about the lon/lat grid, so we use wgrib again, this time with the verbose option (-V) and just for the first record (-d 1). We are assuming that all the records in the file share the same lon/lat grid, but this may not necessarily be the case.

```text
# wgrib -V -d 1 sample.grib
rec 1:0:date 2004040200 UGRD kpds5=33 kpds6=100 kpds7=1000 levels=(3,232) grid=3 1000 mb anl:
UGRD=u wind [m/s]
timerange 10 P1 0 P2 0 TimeU 1 nx 360 ny 181 GDS grid 0 num_in_ave 0 missing 0
center 7 subcenter 0 process 81 Table 2
latlon: lat 90.000000 to -90.000000 by 1.000000 nxny 65160
long 0.000000 to -1.000000 by 1.000000, (360 x 181) scan 0 mode 128 bdsgrid 1
min/max data -24.8 29.3 num bits 10 BDS_Ref -248 DecScale 1 BinScale 0
```

Now we can use this information to put together the descriptor file ([sample.ctl](_downloads/sample.ctl)):

```text
 dset ^sample.grib
index ^sample.idx
title sample grib file
dtype grib
options yrev
undef 9.999E+20
XDEF 360 linear 0.0 1.0
YDEF 181 linear -90.0 1
ZDEF 4 levels 1000 850 500 200
TDEF 3 linear 00Z02apr2004 6hr
VARS 2
u  4  33,100  u wind [m/s]
v  4  34,100  v wind [m/s]
ENDVARS
```

Notes:\
The name of the index file name can be anything at all, here we choose a filename similar to the data file, but with a .idx extension. The options yrev is needed because the grid is written from north to south (90 to -90), but the GrADS default is the opposite, so we need to tell GrADS to turn the grid upside down. For GRIB, the undef value is arbitrary, but required by GrADS. The XDEF and YDEF entries are based on the info from the verbose wgrib output. The ZDEF and TDEF entries are based on the level and time info from wgrib: 4 pressure levels, and 3 time steps with a 6-hour increment. In the variable declarations, the numbers in the units field (33,100 and 34,100) are the grib codes that appear in the kpds5 and kpds6 fields of the wgrib output. These are the codes that identify the variable and the level type; the kpds7 field give the level values, which in this case are the pressure levels. The name of the variable is arbitrary -- it is a good practice to keep is short but meaningful.

The final step is to run gribmap to create the index file. Gribmap looks at the metadata in each and every record in the GRIB file and compares it to the information in the descriptor file. If the record contains a variable at a vertical level at a time that fits into the grid described by the descriptor file, then it is declared a "MATCH" and a the file position of that record is recorded:

```text
# gribmap -v -i sample.ctl
grib1map: opening GRIB file: sample.grib
!!!!! MATCH:  1   81534 0 3 1 0 33 100 1000      79 0 btim: 2004040200:00 tau:  0 dtim: 2004040200:00
!!!!! MATCH:  2  154922 0 3 1 0 34 100 1000   81613 0 btim: 2004040200:00 tau:  0 dtim: 2004040200:00
!!!!! MATCH:  3  236456 0 3 1 0 33 100 850   155001 0 btim: 2004040200:00 tau:  0 dtim: 2004040200:00
!!!!! MATCH:  4  317990 0 3 1 0 34 100 850   236535 0 btim: 2004040200:00 tau:  0 dtim: 2004040200:00
!!!!! MATCH:  5  399524 0 3 1 0 33 100 500   318069 0 btim: 2004040200:00 tau:  0 dtim: 2004040200:00
!!!!! MATCH:  6  489202 0 3 1 0 34 100 500   399603 0 btim: 2004040200:00 tau:  0 dtim: 2004040200:00
!!!!! MATCH:  7  578880 0 3 1 0 33 100 200   489281 0 btim: 2004040200:00 tau:  0 dtim: 2004040200:00
!!!!! MATCH:  8  660414 0 3 1 0 34 100 200   578959 0 btim: 2004040200:00 tau:  0 dtim: 2004040200:00
!!!!! MATCH:  9  741948 0 3 1 0 33 100 1000  660493 0 btim: 2004040200:00 tau:  6 dtim: 2004040206:00
!!!!! MATCH: 10  815336 0 3 1 0 34 100 1000  742027 0 btim: 2004040200:00 tau:  6 dtim: 2004040206:00
!!!!! MATCH: 11  896870 0 3 1 0 33 100 850   815415 0 btim: 2004040200:00 tau:  6 dtim: 2004040206:00
!!!!! MATCH: 12  978404 0 3 1 0 34 100 850   896949 0 btim: 2004040200:00 tau:  6 dtim: 2004040206:00
!!!!! MATCH: 13 1059938 0 3 1 0 33 100 500   978483 0 btim: 2004040200:00 tau:  6 dtim: 2004040206:00
!!!!! MATCH: 14 1141472 0 3 1 0 34 100 500  1060017 0 btim: 2004040200:00 tau:  6 dtim: 2004040206:00
!!!!! MATCH: 15 1231150 0 3 1 0 33 100 200  1141551 0 btim: 2004040200:00 tau:  6 dtim: 2004040206:00
!!!!! MATCH: 16 1312684 0 3 1 0 34 100 200  1231229 0 btim: 2004040200:00 tau:  6 dtim: 2004040206:00
!!!!! MATCH: 17 1394218 0 3 1 0 33 100 1000 1312763 0 btim: 2004040200:00 tau: 12 dtim: 2004040212:00
!!!!! MATCH: 18 1467606 0 3 1 0 34 100 1000 1394297 0 btim: 2004040200:00 tau: 12 dtim: 2004040212:00
!!!!! MATCH: 19 1549140 0 3 1 0 33 100 850  1467685 0 btim: 2004040200:00 tau: 12 dtim: 2004040212:00
!!!!! MATCH: 20 1630674 0 3 1 0 34 100 850  1549219 0 btim: 2004040200:00 tau: 12 dtim: 2004040212:00
!!!!! MATCH: 21 1712208 0 3 1 0 33 100 500  1630753 0 btim: 2004040200:00 tau: 12 dtim: 2004040212:00
!!!!! MATCH: 22 1793742 0 3 1 0 34 100 500  1712287 0 btim: 2004040200:00 tau: 12 dtim: 2004040212:00
!!!!! MATCH: 23 1883420 0 3 1 0 33 100 200  1793821 0 btim: 2004040200:00 tau: 12 dtim: 2004040212:00
!!!!! MATCH: 24 1973098 0 3 1 0 34 100 200  1883499 0 btim: 2004040200:00 tau: 12 dtim: 2004040212:00
grib1map: reached end of files
grib1map: writing the map...
```

Success! Each record in the GRIB file has been mappedto a variable, level, and time in the descriptor file. Note that failure to match will not lead to an error in GrADS; if a grib record at a particular time or level is missing, GrADS will return a gridwith "undefined" values on display.

## Comments on GRIB2

The procedure for handling GRIB2 in GrADS is essentially the same as it is for GRIB1. However, GRIB2 is treated as a separate data type, so you need to change the DTYPE entry in your descriptor file. There are some keywords for the OPTIONS entry that are only valid for GRIB2 (e.g. "pascals"), and the "yrev" option is not necessary with GRIB2, since the north-south orientation of the grid is contained in the header metadata, so GrADS can figure that one out on its own. In addition, the GRIB2 codes required in the variable declarations are much more numerous -- please see the reference page on <a href="descriptorfile.html">descriptor file elements</a> for details on variable declarations for GRIB2. The <a href="gradutilgrib2scan.html">grib2scan</a> utility may also be helpful in determining what the grib codes should be, as well as the output from gribmap (with the verbose option enabled "-v"). For GRIB2 ensemble data sets, the required ensemble codes are included in the <a href="gradutilgrib2scan.html">grib2scan</a> and gribmap output.

### `alt_g2ctl` and `alt_gmp`

[alt_g2ctl](https://www.ftp.cpc.ncep.noaa.gov/wd51we/alt_gmp/alt_g2ctl) and [alt_gmp](https://www.ftp.cpc.ncep.noaa.gov/wd51we/alt_gmp/alt_gmp) are alternative Perl utilities for large or complex GRIB2 collections. They use the extended variable names from `wgrib2` to create more flexible field identifiers than the traditional `g2ctl`/`gribmap` pair. The [CPC download page](https://www.cpc.ncep.noaa.gov/products/wesley/alt_g2ctl_gmp.html) describes the format and available options. Both scripts require a recent `wgrib2`, which can be readily installed from conda-forge:
```bash
conda install wgrib2
```

The basic workflow is:

```text
alt_g2ctl [options] TEMPLATE [INDEX] > FILE.CTL
alt_gmp [-v] [-v] [-v] [-i FILE.CTL]
```

`TEMPLATE` is a GRIB2 filename or a filename template such as `forecast_%y4%m2%d2.grb`. `alt_g2ctl` writes the GrADS control file to standard output; the optional `INDEX` names the index file that `alt_gmp` will create. Run `alt_gmp FILE.CTL` (the `-i` form is also accepted) to scan the GRIB2 files with `wgrib2` and generate the index. Repeat `-v` up to three times for more diagnostic output. Options that control the scan, such as `-update`, `-nthreads N`, `-match`, `-not`, and time-selection options such as `-0t`, are supplied to `alt_g2ctl`; they are recorded in the control file and therefore do not need to be repeated when running `alt_gmp`.

For example:

```text
alt_g2ctl forecast_%y4%m2%d2.grb forecast.idx > forecast.ctl
alt_gmp forecast.ctl
```

The resulting `forecast.ctl` and index file can be opened in GrADS in the usual way. `alt_gmp` can use cached compressed `wgrib2` inventories in update mode, which avoids rescanning unchanged files in large templated datasets. The alt format is compatible with GrADS but its control files are intended for `alt_gmp`, not the standard `gribmap` program.

## Summary

1.  Scan the GRIB files (with [wgrib](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib.html) / [wgrib2](http://www.cpc.ncep.noaa.gov/products/wesley/wgrib2/) / <a href="gradutilgribscan.html">gribscan</a> / <a href="gradutilgrib2scan.html">grib2scan</a>) to see what's in them.
2.  Construct a descriptor using [alt_g2ctl](https://www.ftp.cpc.ncep.noaa.gov/wd51we/alt_gmp/alt_g2ctl),  [grib2ctl](http://www.cpc.ncep.noaa.gov/products/wesley/grib2ctl.html) / [g2ctl](http://www.cpc.ncep.noaa.gov/products/wesley/g2ctl.html)).
3.  Run [alt_gmp](https://www.ftp.cpc.ncep.noaa.gov/wd51we/alt_gmp/alt_gmp) / <a href="gradutilgribmap.html">gribmap</a> to map the GRIB records to the 4- or 5-D gridstructure in the descriptor file and create the index file.

The work required for step \#2 is not necessarily easy. But the advantage of reading GRIB data directly without converting it to another format is worth the effort. The output from the tools above is usually a good first guess for your descriptor file, but may need some tweaking (by examinging the `wgrib` output) to make sure you match every record. Of course, there's also the option to describe only those variables (or levels) that are of interest to you, in which case you are not required to match every record.
