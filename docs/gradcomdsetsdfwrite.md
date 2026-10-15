---
title: set sdfwrite
---

# **set sdfwrite**

This command sets the filename and other characteristics for self-describing netCDF data output. Is it used in conjunction with the <a href="gradcomdsdfwrite.html">`sdfwrite`</a> command which writes out a defined variable.

## Syntax

`set sdfwrite <-3dz or -3dt or -4d or -5d> <-rt or -re> <-flt or -dbl> <-nc3 or -nc4> <-chunk> <-zip> `*`fname`*

where

- *`fname`*`    `output filename (required)

The following six optional arguments are not enabled by default; each new call to <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> will turn them off if they are not included. The default behavior is to write out a variable with the same number of varying dimensions as the defined variable and with no record (unlimited) dimension.

- `-3dt     `forces the output data file to have at least 3 coordinate dimensions (lon, lat, and time)\
  `-3dz     `forces the output data file to have at least 3 coordinate dimensions (lon, lat, and lev)\
  `-4d      `forces the output data file to have at least 4 coordinate dimensions (lon, lat, lev, and time)\
  `-4e      `forces the output data file to have at least 4 coordinate dimensions (lon, lat, time, and ens)\
  `-5d      `forces the output data file to have 5 coordinate dimensions (lon, lat, lev, time, and ens)\
  `-rt      `sets the T axis as the record (unlimited) dimension\
  `-re      `sets the E axis as the record (unlimited) dimension\

The following six optional arguments will "stick" once they are invoked; defaults are restored with the [`reset`](https://wetterzentrale.de/grads/docs/gradcomdreset.html), [`reinit`](https://wetterzentrale.de/grads/docs/gradcomdreinit.html), or [`clear sdfwrite`](https://wetterzentrale.de/grads/docs/gradcomdclear.html) commands.

`-flt     `output data written with floating point precision\
`-dbl     `output data written with double precision (default)\
`-nc3     `output data file written in netCDF classic format (default)\
`-nc4     `output data file written in netCDF-4 format\
`-chunk   `output data will be chunked (only if `-nc4`); set chunk sizes with the <a href="gradcomdsetchunksize.html">`set chunksize`</a> command\
`-zip     `output data will be compressed (implies `-nc4` and `-chunk`)\

### Usage Notes

This command is available in version v2.0.a3+.\
The `-4d` and `-5d` options are available in version v2.0.a5+.\
The `-flt`, `-dbl`, `-nc3`,`-nc4`, `-chunk`, and `-zip` options are available in version 2.0.a8+.\
The `-3dz,``-3dt,-rt` and `-re` options are available in version 2.1.a2+.\
The -4de option is available in version 2.1.1.b0.

The options set with this command "stick" until the [`reset`](https://wetterzentrale.de/grads/docs/gradcomdreset.html), [`reinit`](https://wetterzentrale.de/grads/docs/gradcomdreinit.html), or [`clear sdfwrite`](https://wetterzentrale.de/grads/docs/gradcomdclear.html) commands are invoked -- they are not altered when <a href="gradcomdsdfwrite.html">`sdfwrite`</a> is invoked; however, there have been some changes related to which options are reset whenever a new <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> command is invoked:

- (2.0.a3) Originally, all options were designed to "stick" until a [`clear sdfwrite`](https://wetterzentrale.de/grads/docs/gradcomdclear.html) command was invoked.
- (2.0.a9) The interface was changed so that the `-5d` and `-4d` options are reset to the default (off) with each new <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> command. This means that if these options are not used, the output variable will have the same number of dimensions as the defined variable. The options that control the precision, format, and compression of the output file continue to "stick" until they are explicitly changed.
- (2.1.a2) The new options (`-3dz,``-3dt,-rt` and `-re`) have the same behavior as the `-5d` and `-4d` options -- they are reset to the default (off) with each new <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a> command.

When the `-3dz,``-3dt,-4d` or `-5d` options are used, the output file will retain information about the values of the dimensions that were fixed when the variable was defined; non-varying dimensions will appear as a coordinate variable of size 1.

The file *`fname`* will be replaced if it exists. Use the <a href="gradcomdsdfwrite.html">`sdfwrite`</a> command to create the output file in netCDF format. The <a href="gradcomdqsdfwrite.html">`query sdfwrite`</a> command returns the status of the `sdfwrite` options.

The <a href="gradcomdclear.html">`clear sdfwrite`</a> command resets these sdfwrite parameters back to their default values, and releases from memory all attribute metadata that has been defined with <a href="gradcomdsetsdfattr.html">`set sdfattr`</a>.

If you use the `-zip` option, then it is not necessary to also specify `-nc4` or `-chunk`; GrADS will automatically set these options. A compressed file is always in netCDF-4 format and is always chunked. Use the <a href="gradcomdsetchunksize.html">`set chunksize`</a> command to set the chunk size before writing out the file. Please see the documentation on <a href="compression.html">compression</a> for more details. Compression is not availabe with the classic netCDF format.

The `-rt` and `-re` options are used to assign a record (unlimited) dimension. The default is to not have a record dimension. A netCDF-4 file may have more than one record dimension -- it can be set to T or E or both. For netCDF classic format, only one dimension may be a record dimension, and it must be the outermost (slowest varying) dimension.

### Examples

Suppose you have a high resolution data set (5120x2560) and you wish to save the variable as compressed netcdf with floating point precsion.

`set x 1 5120`\
`set y 1 2560`\
`set z 1`\
`set t 1`\
`define var = var`\
`set sdfwrite -flt -zip var.nc`\
`set chunksize 512 256`\
`sdfwrite var`\
