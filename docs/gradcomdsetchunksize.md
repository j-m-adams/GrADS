---
title: set chunksize
---

# **set chunksize**

`set chunksize `*`Xsize Ysize <Zsize> <Tsize> <Esize>`*

*`Xsize`*`  ` Size of the chunk in the X dimension. Default value is the grid size in X.\
*`Ysize`*`  ` Size of the chunk in the Y dimension. Default value is the grid size in Y.\
*`Zsize`*`  ` Size of the chunk in the Z dimension. Default value is 1.\
*`Tsize`*`  ` Size of the chunk in the T dimension. Default value is 1.\
*`Esize`*`  ` Size of the chunk in the E dimension. Default value is 1.\

It is not necessary to specify the chunk size for the Z, T, or E dimensions if you wish to use the default values.

## Usage Notes

*Chunking* refers to the partitioning of a dataset into fixed-size multi-dimensional chunks. Chunks are treated as atomic objects; disk I/O, cacheing, and compression are always done on a per-chunk basis. Chunking can significantly improve I/O performance, provided the chunk size is set appropriately for the data set in question.

A chunk has the same number of dimensions as the original data set. The size of a chunk in any dimension should be equal to or less than the size of the data set's grid dimensions. For data sets with more than 2 dimensions, DO NOT set the chunk sizes equal to the grid dimension sizes, in which case the entire data set would be one chunk, and DO NOT set all the chunk sizes equal to 1 (one), in which case a chunk would be a single value.

The default values in GrADS set the chunk size equal to the grid dimension in X and Y, and 1 (one) for the Z, T, and E dimensions -- a chunk is a single 2D lat/lon grid. However, if your data set is of sufficiently high resolution (e.g., if the grid size is less than 1.0 degree latitude/longitude), then you should set the chunk size smaller than the grid size in X and Y -- divide by 2, or 5, or 10 as necessary to keep the chunk in the ballpark of 512 kbytes.It is recommended that you keep the chunk size equal to 1 for the Z, T, and E dimensions.

Please see the documentation on <a href="compression.html">compression</a> for more details.

### Examples

Suppose you have a high resolution data set (5120x2560) and you wish to save the variable as compressed netcdf.

`set x 1 5120`\
`set y 1 2560`\
`set z 1`\
`set t 1`\
`define var = var`\
`set sdfwrite -flt -nc4 -chunk -zip var.nc`\
`set chunksize 512 256`\
`sdfwrite var`\
