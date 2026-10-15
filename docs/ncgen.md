---
title: ncgen
---

# ncgen

`ncgen [-b] [-c] [-f] [-n] [-o `*`output_file`*`] `*`input_file`*

Where:

- `-b` Create a (binary) netCDF file. If the `-o` option is absent, a default file name will be constructed from the netCDF name (specified after the netcdf keyword in the input) by appending the `.nc` extension. If a file already exists with the specified name, it will be overwritten.

  `-c` Generate C source code that will create a netCDF file matching the netCDF specification. The C source code is written to standard output.

  `-f` Generate Fortran source code that will create a netCDF file matching the netCDF specification. The Fortran source code is written to standard output.

  `-o `*`outputfile`*\

  - Name for the netCDF file created. If this option is specified, it implies the `-b` option. (This option is necessary because netCDF files cannot be written directly to standard output, since standard output is not seekable.)

  `-n` Like `-b` option, except creates netCDF file with the obsolete `.cdf` extension instead of the `.nc` extension, in the absence of an output filename specified by the `-O` option. This option is only supported for backward compatibility.

## Examples

1.  Check the syntax of the CDL file `foo.cdl`:
    `ncgen foo.cdl`
2.  From the CDL file `foo.cdl`, generate an equivalent binary netCDF file named `x.nc`:
    `ncgen -o x.nc foo.cdl`
3.  From the CDL file `foo.cdl`, generate a C program containing the netCDF function invocations necessary to create an equivalent binary netCDF file named `x.nc`:
    `ncgen -c -o x.nc foo.cdl`
