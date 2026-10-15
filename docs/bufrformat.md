---
title: BUFR Format
---

# BUFR Format

## Reading BUFR Files with GrADS
BUFR (Binary Universal Form for the Representation of meteorological data) is a World Meteorological Organization (WMO) standard for storing observational data (aka sequence or in-situ data). BUFR is self-describing data format and can store a large amount of data and metadata in a small amount of disk space by using look-up tables and bit-by-bit packing. 

There is a GrADS interface for BUFR, which means that BUFR data can be read directly in their native format and are handled as a GrADS station data set with all the associated display an analysis capabilities. GrADS requires a specially-formatted descriptor file to read BUFR data; the output from <a href="gradutilbufrscan.html">`bufrscan`</a>, an external GrADS utility, is used to compose the descriptor file.

Individual elements of a BUFR message are uniquely described by three numbers: F, X, and Y. F is a type indicator and may be 0, 1, 2, or 3. X is a class or category indicator and varies between 0 and 63. Y indicates an entry within an X class, and varies between 0 and 255. The F,X,Y trio provides the required unique table reference, so that a value may be retrieved for the BUFR element.

To read BUFR with GrADS, the user needs to identify which F,X,Y trios are in the BUFR file and then organize that information in a descriptor file to give "shape" to the data by identifying the appropriate time axis, vertical dimension, and number of variables. The GrADS-relevant data in a BUFR message will always have an F value of 0. For this reason, it is only necessary to put the X,Y pairs that are associated with the data or metadata variables in the BUFR descriptor file.

## BUFR Descriptor File Components
The GrADS station data interface requires a few pieces of metadata for each report: the location of the station (lat/lon), a station ID (a string no more than 8-characters long), a pressure level (if it is an upper air variable), and a time stamp. The BUFR descriptor file must provide the X,Y pairs for these metadata fields, plus a few other elements.

Descriptor file entries used for BUFR files are:

| DSET | This entry points to the BUFR data file. It is not currently recommend to use templating with BUFR data. (See Note on Templating below) |
| --- | --- |
| TITLE | It is good general practice to include a descriptive title in every GrADS descriptor file. |
| UNDEF | This is required by GrADS, but not used for undef-testing in the BUFR interface. Place an arbitrary number here that is unlikely to be confused with a good data value. |
| DTYPE | This entry should have the 'bufr' keyword. This data type must be accompanied by the XVAR, YVAR, TVAR, and STID entries. |
| STID | This required entry provides the X,Y pair for the station ID. |
| XVAR | This required entry provides the X,Y pair for the station's longitude. |
| YVAR | This required entry provides the X,Y pair for the station's latitude. |
| ZVAR | This optional entry provides the X,Y pair for the data's vertical coordinate (usually pressure). This is only required if there are level-dependent variables in the BUFR file. |
| TVAR and TOFFVAR | The time for any individual BUFR station report is the base time plus the offset time.The TVAR entry is required and provides the X,Y pairs for the base time coordinate variables. The TOFFVAR entry provides the X,Y pairs for the offset time. If the offset time is zero, the TOFFVAR entry is not required. Each time coordinate variable (year=yr, month=mo, day=dy, hour=hr, minute=mn, second=sc) is presented as a 2-letter abbreviation followed by the X,Y pair that goes with that time unit. All six base/offset time units are not required to appear in the TVAR/TOFFVAR record, only those that are in the data file. |
| TDEF | For BUFR station data, the time axis defined by the TDEF entry provides an evenly-spaced framework for the (sometimes) unevenly spaced BUFR station reports to fit into. Choose a TDEF that spans the time range of your BUFR data and has a time increment that matches the frequency of the BUFR reports. (See Note on TDEF below) |
| VARS through ENDVARS | The variable declarations in a BUFR descriptor file also have special features. The varname field may be any 15-character alphanumeric string that must start with an alphabetic character (a-z). It is not necessary for the varname in the descriptor to match the varname in the BUFR file. The levs field is 0 for surface variables, 1 for upper air variables. Exception to this rule: replicated surface variables (i.e. variables for which there may be more than one observation, such as present weather) are given a levs value of 2. The units field contains the X,Y pair for the named variable. |

The first step in generating a BUFR descriptor file is figuring out the X,Y values for the data and metadata variables that GrADS requires. Begin by perusing the header output from <a href="gradutilbufrscan.html">`bufrscan`</a> looking primarily at the numeric elements. Here is a link to some [example header output](_downloads/bufr.sample.headers) -- a subset of this is given below. 

These lines give the base time for the report, the station identifier, the location and elevation of the station, plus some of the observed variables:
0 04 001 (numeric) YEAR YEAR
0 04 002 (numeric) MNTH MONTH
0 04 003 (numeric) DAYS DAY
0 04 004 (numeric) HOUR HOUR
0 04 005 (numeric) MINU MINUTES
0 01 198 (text) RPID REPORT IDENTIFIER
0 06 002 (numeric) CLON LONGITUDE (COARSE ACCURACY)
0 05 002 (numeric) CLAT LATITUDE (COARSE ACCURACY)
0 07 001 (numeric) SELV HEIGHT OF STATION
0 11 001 (numeric) WDIR WIND DIRECTION
0 11 002 (numeric) WSPD WIND SPEED
0 12 101 (numeric) TMDB TEMPERATURE/DRY BULB TEMPERATURE
0 12 103 (numeric) TMDP DEW POINT TEMPERATURE
0 10 051 (numeric) PMSL PRESSURE REDUCED TO MSL
0 10 061 (numeric) 3HPC 3 HOUR PRESSURE CHANGE

The corresponding descriptor file entries would look like this (N.B. This is not a complete descriptor file):
TVAR yr 4,1 mo 4,2 dy 4,3 hr 4,4 mn 4,5
STID 1,198
XVAR 6,2
YVAR 5,2
VARS 9
slon 0 6,2 Station longitude
slat 0 6,2 Station latitude
selv 0 7,1 Station elevation
wdir 0 11,001 Wind direction
wspd 0 11,002 Wind speed
temp 0 12,101 Temperature
dewpt 0 12,103 Dew point temperature
mslp 0 10,51 Mean sea level pressure
dp 0 10,004 3-hour pressure change
ENDVARS

The internal GrADS variables "lat", "lon", and "lev" do not exist for station data, so it's a good idea to put them in the variable list in case you need them for any calculations (BUFR variables can be both coordinate and data variables at the same time). Just be careful not to assign the names "lat", "lon" or "lev", as this will confuse GrADS and you'll get the message that the predefined variable is only for grid type files.

The second step is to figure out what to use for a TDEF entry. You may be aware ahead of time that your BUFR file contains hourly data covering a known 6-hour period, in which case you are done (`TDEF 6 `*`start_of_period`*` 1hr`). But if you have no idea what's in your BUFR file, then you need to examine the data output from <a href="gradutilbufrscan.html">`bufrscan`</a> looking for the F-X-Y triplets that appear in your TVAR entry. Here is a link to some [example data output](_downloads/bufr.sample.data) -- a subset of this is given below.
```text
7 (0) \[-001\] 0-04-001 2004
7 (0) \[-001\] 0-04-002 4
7 (0) \[-001\] 0-04-003 22
7 (0) \[-001\] 0-04-004 15
7 (0) \[-001\] 0-04-005 0
```

These lines indicate an appropriate TDEF might be:
`TDEF 1 linear 15z22apr2004 1hr`
If you found more occurrences of 0-04-004 with values other than 15, change your TDEF:
`TDEF 24 linear 00z22apr2004 1hr`
If you found occurrences of 0-04-003 with values equal to 23 as well as 22, change your TDEF again:
`TDEF 48 linear 00z22apr2004 1hr`

## Note on TDEF:
As mentioned above, the time axis you describe with TDEF provides an evenly-spaced framework for the station reports to fit into. A display request at a specific time will return all station reports whose time stamp is within a range of times equal to the specific time plus or minus one half of the time axis interval. Let's say you set TDEF to be hourly, set the time dimension to 12Z, and then do a display request. GrADS will sift through every report in the BUFR file and display only those which fall between 11:30Z and 12:30Z. If you change TDEF to be six-hourly, set the time dimension to12Z, and then do a display request, GrADS will show you all reports that fall between 09Z and 15Z. GrADS displays the reports that fit between the grid points based on the time axis and time dimension environment you describe. In BUFR, there's no reason to expect the messages to be ordered in terms of time or place, and there's no index file to help navigate the data file, so for each display request GrADS has to sift through the whole file.

## Note on Templating:
In GrADS, file templating (aggregation) is only done in the the time dimension. It's handled by matching substrings in the file names with time dimension values. After a display request, GrADS determines which data files need to be opened in order to get the data the user requested, based on the current dimension environment settings. Templating in this manner should be avoided with BUFR. Since there's no index file to help GrADS navigate the data file, a BUFR file containing 10 reports is treated exactly the same way as a BUFR file with 10,000 -- the whole file is read into memory when the user submits the first display request. For each display request, GrADS sifts through the entire collection of reports to find the ones that match the current dimension environment. Since the memory usage is so extreme, it's better to keep BUFR files small and only access one at a time. When necessary and/or appropriate, it's possible to aggregate BUFR files by simply concatenating them together.
