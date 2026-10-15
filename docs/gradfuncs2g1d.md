---
title: s2g1d
---

# **s2g1d**

`s2g1d(`*`expr`*`)`

This function converts a station data time series into a 1-dimensional grid. This allows more graphics and analytical operations.

*`expr`*            A valid GrADS 1D station data expression in which time is the only varying dimension.\

## Usage Notes

The s2g1d function only works with 'display' and will not work with 'define'. You have to use 'set gxout fwrite' and write the data to a file if you want to save and reuse the 1D gridded time series.

### Examples

`'set gxout bar'`\
`'set barbase 0'`\
`'set bargap 10'`\
`'``display s2g1d(precip(stid=kdca))'`

`'set gxout contour'`\
`'``display smth9(s2g1d(t(stid=kord)))`'

`'set gxout vector'`\
`'display const(s2g1d(u(stid=ksux)),0);s2g1d(u(stid=ksux));s2g1d(v(stid=ksux))'`
