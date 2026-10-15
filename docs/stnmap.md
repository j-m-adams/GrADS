---
title: stnmap
---

# stnmap

`stnmap [ -`*`i fname`*` ]`

## Usage Notes

`stnmap` is a GrADS utility that writes out a hash table and/or link list information for station data that allows GrADS to access the report data more efficiently.

If you change the data file - perhaps by appending another time group - you will also have to change the descriptor file to reflect the changes - the new number of times for example - and then rerun the `stnmap` utility.

Note the difference between station descriptor file and a grid descriptor file:

- `DTYPE`     specify a data type of: station

  `STNMAP`     gives the file name of the station mapping file.

  `XDEF, YDEF, ZDEF`     not specified.

  `TDEF`     describes the time grouping interval and the number of the time groups in the file.

  `VAR`     surface variables must come first, and are given a zero for the number-of-levels field. Level dependent variables are listed after the surface variables, and are given a one in the number-of-levels field.

### Examples

When a data set, for example `ua.reps` is written, and the descriptor file `stat.ctl`, including the information records for the station data file `ua.reps` is created, you should then create the station map file `ua.map` (named in the `STNMAP` record of the descriptor file; see below) by running the `stnmap` utility.

Example descriptor file: `stat.ctl`

- ` DSET ^ua.reps`\
  `DTYPE station`\
  `STNMAP ^ua.map`\
  `UNDEF -999.0`\
  `TITLE Real Time Upper air obs`\
  `TDEF 10 linear 12z18jan1992 12hr`\
  `VARS 12`\
  `  slp 0 99 SLP`\
  `  ts 0 99 Temps`\
  `  us 0 99 U Winds`\
  `  vs 0 99 V Winds`\
  `  z 1 99 Heightsa`\
  `  t 1 99 Temps`\
  `  u 1 99 U Winds`\
  `  v 1 99 V WInds`\
  `ENDVARS`

Running the `stnmap` utility:

`stnmap -i stat.ctl`

The station map file `ua.map` is a binary file, which includes the hash table and/or link list information.
