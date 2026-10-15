---
title: sdfwrite
---

# **sdfwrite**

`sdfwrite `*`varname`*` `

This command will write out a defined variable *`varname`*` `into a NetCDF formatted data file.

-  

## Usage Notes

The `sdfwrite` command was initially implemented in GrADS version 2.0.a3. Additional features added in subsequent releases are as follows:

- (2.0.a5) NetCDF output may be forced to have 4 or 5 dimensions.
- (2.0.a8) NetCDF output may be floating point or double precision. NetCDF output may also be version 4, with chunking and compression enabled.
- (2.0.a9) If the output is not forced to have 4 or 5 dimensions, it will have the same number of dimensions as the defined variable being written out to file.
- (2.1.a1) New options were added to set a record (unlimited) dimension and to force the output to have 3 dimensions.

Options to control the name and format of the output file are implemented with the <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> command. The <a href="gradcomdqsdfwrite.html">`q sdfwrite`</a> command returns the status of the `sdfwrite` options.
The name of the output file will be `grads.sdfwrite.nc` unless specified otherwise with the <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> command. If the file exists, it will be replaced. It is not possible to append variables to an existing file.

The dimensions of the variable written to file correspond to the dimension environment that was set when the variable was defined. The dimension environment that is set when the 'sdfwrite' command is invoked is ignored. Note this behavior is different from the fwrite command.

By default, the output file will have a coordinate variable only for varying dimensions in the defined variable; non-varying dimensions will not appear as a coordinate variable with a size of 1. However, options have been added to the <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> command to force the variable in the output file to have at least 3, 4, or all 5 dimensions. When either of these options to <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> are used, the output file will retain information about the values of the dimensions that were fixed when the variable was defined; non-varying dimensions will appear as a coordinate variable of size 1. For example, if your defined variable is 500mb height on14may2002 (a 2D variable that varies only in lon and lat), and you use the -4d option, the output file with show height as a 4D variable with a Z dimension of size 1 called 'lev' with a value "500 mb", and a T dimension of size 1 called 'time' with a value of "0 minutes since 2002-05-14 00:00".

The coordinate variables will be type 'double' and will have two default attributes ("units" and "long_name") with values that are based on the GrADS 5-dimensional gridded data model. The data variable will also be type 'double' by default; beginning with version 2.0.a8, you can also write out data of type 'float' if you use the `-flt` option with <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> command. Data variables will have a "missing \_value" or "\_FillValue" attribute for undefined gridpoints. The undefined value is controlled with the <a href="gradcomdsetundef.html">`set undef`</a> command.

For example, if *`varname`* is called "testvar" and varies in X, Y, Z, T, and E, then the output file might have a NetCDF header that looks like this:

`netcdf foo {`\
`dimensions:`\
`  longitude = 9 ;`\
`  latitude = 9 ;`\
`  level = 9 ;`\
`  time = 9 ;`\
`  ensemble = 9 ;`\
`variables:`\
`  double longitude(longitude) ;`\
`     longitude:units = "degrees_east" ;`\
`     longitude:long_name = "Longitude" ;`\
`  double latitude(latitude) ;`\
`     latitude:units = "degrees_north" ;`\
`     latitude:long_name = "Latitude" ;`\
`  double level(level) ;`\
`     level:units = "millibar" ;`\
`     level:long_name = "Level" ;`\
`  double time(time) ;`\
`     time:units = "minutes since 01-01-0001 00:00" ;`\
`     time:long_name = "Time" ;`\
`  double ensemble(ensemble) ;`\
`     ensemble:grads_dim = "e" ;`\
`     ensemble:long_name = "Ensemble member" ;`\
`  double testvar(ensemble, time, level, latitude, longitude) ;`\
`     testvar:missing_value = -888. ;`\
`} `

The time axis units will always be "minutes since ..." and the date of the time axis origin will correspond to the initial time of the defined variable.

If the variable has an ensemble dimension, the attribute "grads_dim" with the value "e" will always be present so that the resulting output file can be opened with GrADS using the 'sdfopen' command.

To supplement or override the default attributes of the output file, use the <a href="gradcomdsetsdfattr.html">`set sdfattr`</a> command.

Beginning with version 2.0.a8, the output file may also be a compressed netCDF file. Use the `-zip` option with the <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> command to enable compression. Please see the documentation on <a href="compression.html">compression</a> for more details.

### Examples

The following commands produce a regional subset of a global precip forecast :

```text
open gfs.ctl
set lon -111.3 -103.8 
set lat 40.9 45.0 
set t 1 last
define precip = ptot
set sdfwrite wyoming_precip.nc
sdfwrite precip
```

***Appending Variables to a File***\
The sdfwrite command does not allow you to write out more than one variable to a file. The way to do achieve this is to write out each variable in a separate file, then merge them with the [NetCDF Operators (NCO)](http://nco.sourceforge.net/nco.html). Use the [<code>ncks</code>](http://nco.sourceforge.net/nco.html#ncks) command to append variables defined on the same grid. The GrADS commands might look like this:

```text

open gfs.ctl
set x 1 720
set y 1 361
set t 1 last
define slp = mslp
define precip = ptot
set sdfwrite -flt slp.nc
sdfwrite slp
sdt sdfwrite -flt precip.nc
sdfwrite precip

```

Then, outside GrADS, or using the ! in front of the command to send it to the shell from within GrADS, merge these two files with [<code>ncks</code>](http://nco.sourceforge.net/nco.html#ncks):

[ncks](http://nco.sourceforge.net/nco.html#ncks) -h -A precip.nc slp.nc

The -A is for appending to the output file, and the -h does not add anything to the output file's global 'history' attribute. Please see the [NCO User's Guide](http://nco.sourceforge.net/nco.html) for more information.

***Concatenating Files***\
Suppose you want to write out a long time series with the sdfwrite command, but your system does not have enough memory to define the whole grid. Use GrADS to write out the data in smaller time segments (e.g. one year, or one month, or one day), and then concatenate them together with the [NetCDF Operators (NCO)](http://nco.sourceforge.net/nco.html). Concatenation in the time dimension only works properly if time is defined to be a record (unlimited) dimension. If you are using GrADS version 2.1.a2+, you can use the `-rt` option to set the time axes as a record dimension; otherwise, you must use the [<code>ncks</code>](http://nco.sourceforge.net/nco.html#ncks) command to change the time dimension to a record dimension. When the inidividual files have been created, use the [<code>ncrcat</code>](http://nco.sourceforge.net/nco.html#ncrcat) command to concatenate them together. The GrADS commands might look like this:


```text

open long_time_series.ctl
set x 1 720
set y 1 361
set time 01jan2001 31dec2001
define slp = mslp
set sdfwrite -flt slp_2001.nc
sdfwrite slp
set time 01jan2002 31dec2002
define slp = mslp
set sdfwrite -flt slp_2002.nc
sdfwrite slp
set time 01jan2003 31dec2003
define slp = mslp
set sdfwrite -flt slp_2003.nc
sdfwrite slp

```
    set time 01jan2002 31dec2002
    define slp = mslp
    set sdfwrite -flt slp_2002.nc
    sdfwrite slp
    set time 01jan2003 31dec2003
    define slp = mslp
    set sdfwrite -flt slp_2003.nc
    sdfwrite slp

sdfwrite slp

If you are using GrADS version 2.1.a2+, the 'set sdfwrite' commands should look like this instead:\
`set sdfwrite -flt -rt slp_2001.nc`\
`set sdfwrite -flt -rt slp_2002.nc`\
`set sdfwrite -flt -rt slp_2003.nc`

If you are using an earlier verison of GrADS, you must modify the time dimension with [<code>ncks</code>](http://nco.sourceforge.net/nco.html#ncks) -- do this outside of GrADS, or use the ! in front of the command to send it to the shell from within GrADS (the --mk_rec_dmn option modifies the time dimension):

[ncks](http://nco.sourceforge.net/nco.html#ncks) -O -h --mk_rec_dmn time slp_2001.nc [ncks](http://nco.sourceforge.net/nco.html#ncks) -O -h --mk_rec_dmn time slp_2002.nc [ncks](http://nco.sourceforge.net/nco.html#ncks) -O -h --mk_rec_dmn time slp_2003.nc

```text
ncrcat 
-O -h --cnk_dmn lev,1 --cnk_dmn lat,361 --cnk_dmn lon,720 slp_2001.nc slp_2002.nc slp_2003.nc slp.nc
```

The -O is for overwriting existing output file, and the -h does not add anything to the output file's global 'history' attribute. Please see the [NCO User's Guide](http://nco.sourceforge.net/nco.html) for more information -- there are shortcuts for [<code>ncrcat</code>](http://nco.sourceforge.net/nco.html#ncrcat) to specify the long list of files to concatenate.

N.B. (For compressed NetCDF-4 files): If the NetCDF files you create with GrADS are compressed netCDF-4 format, then you must use NCO version 4.0.4 (or higher) in order to maintain the compression in the output files. The chunk sizes may also be altered by ncks or ncrcat. Use the --cnk_dmn option to specify the name and chunk size of each dimension to manually override the defaults. For example:

[ncrcat](http://nco.sourceforge.net/nco.html#ncrcat) -O -h --cnk_dmn lev,1 --cnk_dmn lat,361 --cnk_dmn lon,720 slp_2001.nc slp_2002.nc slp_2003.nc slp.nc
