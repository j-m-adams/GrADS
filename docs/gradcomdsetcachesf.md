---
title: set cachesf
---

GrADS command: set cachesf

# **set cachesf**

`set cachesf `*`num`*

Use this command to change the scale factor for setting the default cache size (in bytes), which is calculated according to this formula:\
X grid size \* Ygrid size \* 8 \* cachesf

The default cache scale factor is 1.\

## Usage Notes

This command is available with GrADS version 2.0.a8 or later.

The cache size is only relevant when reading HDF5 and NetCDF4 data types.

Cache size is set on a per-file basis; the cache size will be different for each file opened in GrADS. After a file is opened, a new cache is allocated for each variable that gets displayed in the file. Be careful

Current value of the cache size may be discovered with the <a href="gradcomdquery.html">`query cache`</a> command.

Please see the documentation on <a href="compression.html">compression</a> for more details.

### Examples
