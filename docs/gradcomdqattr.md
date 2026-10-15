---
title: q attr
---

# **q attr**

`q attr `*`<fnum>`*

This command returns all the attribute metadata associated with file number *`fnum`* in a formatted manner. If no value for *`fnum`* is given, then the attributes for the default file are returned. The output of 'q attr' may be read by the user in the command window, or parsed by a script for the purpose of capturing the metadata and including it in the analysis or display. There are three categories of attributes: (1) global, which means the attribute is relevant for all the data in the file, (2) coordinate, which means the attribute describes one of the four dimensions (lon, lat, lev, or time), or (3) variable, which means the attribute is assoicated with a particular variable in the file. The formatting of the attribute listing is as follows:

global \| *\<dimension\>* \| *\<varname\>   \<attribute_type\>   \<attribute_name\>   \<attribute_value\>*

*\<dimension\>* is "lon", "lat", "lev", or "time"\
*\<varname\>* is one of the variables in the data set\
*\<attribute_type\>* is one of the following case-sensitive types: String, Byte, Int16, UInt16, Int32, UInt32, Float32, Float64.*\
\<attribute_name\>* is any single word or string with no spaces\
*\<attribute_value\>* is be any string up to 512 characters.

## Usage Notes

Attributes may be native to the data file (as with some HDF and NetCDF files), but they may be also be added manually in the GrADS <a href="descriptorfile.html#ATTR">descriptor file</a>. The output from 'q attr' will print all the attributes taken from the descriptor file followed by all the native attributes. Within these two categories (descriptor and native), attributes are listed in the following order:\
1. Global\
2. Coordinate -- these will have names "lon", "lat", "lev", or "time"\
3. Variable -- these will have names taken from the list of variables in the data file

This command has been fully implemented starting with version 1.9b4.

### Examples

Here's a sample descriptor file (note the attributes added after the ENDVARS statement):

dset ^hgt.%y4.nc\
dtype netcdf\
options template yrev\
title NCEP Reanalysis\
undef -999 missing_value\
unpack scale_factor add_offset\
xdef 144 linear 0 2.5\
ydef 73 linear -90 2.5\
zdef 17 levels 1000 925 850 700 600 500 400 300 250 200 150 100 70 50 30 20 10\
tdef 730 linear 00Z01JAN1989 1dy\
vars 1\
hgt 17 t,z,y,x Mean Daily Geopotential height \[m\]\
ENDVARS\
@ global String comment This is an all-purpose test file\
@ hgt String comment This describes variable hgt\
@ lev String units millibar\
@ lat String units degrees_north\
@ lon String units degrees_east\
@ time String units hours since 1-1-1 00:00:0.0\
@ lev Float32 actual_range 1000. 10.\
@ lat Float32 actual_range 90. -90.\
@ lon Float32 actual_range 0. 357.5\
@ time Int32 actual_range 17426496. 17435232.\

Here's the output from 'q attr' when the above descriptor file is opened:

ga-\> q attr\
Descriptor Attributes for File 1 : NCEP Reanalysis\
global String comment This is an all-purpose test file\
lon String units degrees_east\
lon Float32 actual_range 0. 357.5\
lat String units degrees_north\
lat Float32 actual_range 90. -90.\
lev String units millibar\
lev Float32 actual_range 1000. 10.\
time String units hours since 1-1-1 00:00:0.0\
time Int32 actual_range 17426496. 17435232.\
hgt String comment This describes variable hgt

Native Attributes for File 1 : NCEP Reanalysis\
global String title mean daily NMC reanalysis\
global Int16 base_date 1989 1 1\
global String history /home/hoop/crdc/cpreanjuke2farm/cpreanjuke2farm Wed Oct 18 03:10:49 1995 from hgt.89.nc\
global String history created 95/02/06 by Hoop (netCDF2.3)\
global String description Data is from NMC initialized reanalysis\
global String description (4x/day). It consists of most variables interpolated to\
global String description pressure surfaces from model (sigma) surfaces.\
global String platform Model\
global String Conventions COARDS\
hgt String long_name mean Daily Geopotential height\
hgt Float32 actual_range -522 32306\
hgt Float32 valid_range -700 35000\
hgt String units m\
hgt Float32 add_offset 32066\
hgt Float32 scale_factor 1\
hgt Int16 missing_value 32766\
hgt Int16 precision 0\
hgt Int16 least_significant_digit 0\
hgt Int16 GRIB_id 7\
hgt String GRIB_name HGT\
hgt String var_desc Geopotential height\
hgt String var_desc H\
hgt String dataset NCEP Reanalysis Daily Averages\
hgt String dataset AJ\
hgt String level_desc Multiple levels\
hgt String level_desc F\
hgt String statistic Mean
