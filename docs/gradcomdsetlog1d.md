---
title: set log1d
---

# set log1d

`set log1d `*`arg`*

Where *`arg`* can be one of the following:

- `logh     `make the horizontal axis logarithmic\
  `logv     `make the vertical axis logarithmic\
  `loglog   `make both axes logarithmic\
  `off      `disable for both axes


## Usage Notes

This command is available in GrADS version 2.0 and higher (it is not in the 2.0.a\* versions).

This setting is intended for use with 1-D plots of gridded data ONLY.

To override the axis labeling, use the "set xlevs" or "set ylevs" command.

If axis values approach zero, grads will automatically extend the axis to zero. And then the display command will fail, with an error message. In this case, it is necessary to use the "set vrange" command to force the axis range to be positive valued.

### Examples

```text
'open gfs.ctl'
'set lat 40'
'set lev 500'
'set lon 1 10'
'set vrange 1 1050'
'set ylevs 1 2 5 10 20 50 100 200 500 1000'
'set log1d loglog'
'set mproj off'
'd pow(2,lon)'

```
