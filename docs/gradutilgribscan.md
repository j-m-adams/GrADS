---
title: gribscan
---

# **gribscan**

The `gribscan` utility is used for extracting grid info from GRIB data files. Its features include grid/product information, gridded output in ASCII, binary, and/or grib format, plus automatic "scanning" for GRIB records so that you don't have to know the physical layout of the data to scan it.

The command sytax is:

- `gribscan [-i `*`ifname`*`] [-o `*`ofname`*`] [-`*`file options`*`] [-`*`processing options`*`] [-`*`display options`*`]`

Where:

- *`ifname`*

  - This is the input grib file name. If `-i `*`ifname`*` ` is omitted, `gribscan` will prompt the user for a file name.

  *`ofname`*

  - This is the output file name WITHOUT an extension. If `-o `*`ofname`* is omitted, a default file name of `zy0x1w2.`*`type`* is created where *`type`* is:\
    `asc` - ascii\
    `grb` - GRIB\
    `dat` - a stream of floats (GrADS format)

File Options:

- `-og`

  - `gribscan` will return output in GRIB format

  `-oa`

  - `gribscan` will return output in ASCII format (%8g in C-language syntax)

  `-of`

  - `gribscan` will return output as a stream of floats. This is machine dependent and is 64-bit on Crays and 32-bit elsewhere.

Processing Options:

- `-s`*`NNN`*

  - Specifies the max number *`NNN`* of bytes between GRIB messages in the file. The default is 500 and it is assumed that you want to ignore junk (e.g., comm stuff) between data.

  `-sp`*`NNN`*

  - Selects parameter \# *`NNN`* (e.g.,`-sp11` for temperature)

  `-sl`*`NNN`*

  - Selects level \# *`NNN`* (e.g., `-sp500` to get 500 mb fields)

  `-st`*`NNN`*

  - Selects tau \# *`NNN`* (e.g., `-st12` to get t=12 forecasts)

  *`-h`*`NNN`**

  - Specifies a fixed file header of *`NNN`* bytes. If omitted, the default is to seek the first GRIB message automatically, but if you know *`NNN`*, it is more efficient to specify it.

  *Special note to NMC users:* The once "standard" 81-byte header in an NMC GRIB file contained the string "GRIB". Unfortunately, the same string is part of the GRIB indicator section itself! Thus, an automatic scan for GRIB to demark the start of the data will fail if the 81-byte header is present! When in doubt (or failure) try using the `-h81` option.

  *Note:* These processing options can be used simultaneously to output a very narrow set of fields.

Display options:

- `-q    ` Quick output to extract stuff GrADS gribmap cares about\
  `-q1   ` One-line quick output\
  `-d    ` Comma delimited mode\
  `-v    ` Verbose mode for diagnostics\
  `-bd   ` Binary data section info\
  `-gv   ` Uses the NMC GRIB variable table to output mnemonic, title, and units\
  `-gd   ` Output info from the grid defn sec\
  `-S    ` Silent mode; NO standard output\

## Examples

1.  A "quick" scan to get the info GrADS cares about:

    `gribscan -q -i eta.T12Z.PGrbF48 | grep 184`

    Gives the result:

    `184,F,135,108,100,0,100,0,1e+09,T,1994,8,29,12,0,1,48,0,G,104,BDTG,94082912`

    Where:

    - `184     ` field \# in the file\
      `F       ` field data\
      `135     ` param \#\
      `108     ` level indicator\
      `100     ` level\
      `0       ` l1 byte 1 of level\
      `100     ` l2 byte 2 of level\
      `0       ` time range indicator\
      `1e+09   ` decimal scale factor\
      `T       ` time data follows\
      `1994    ` year\
      `8       ` month\
      `29      ` day\
      `12      ` hour\
      `0       ` min\
      `1       ` forecast time unit (hour)\
      `48      ` t=48 h forecast\
      `G       ` grid param follows\
      `104     ` NMC grid \#104\
      `BDTG    ` Base date-time-group (yymmddhh) follows

2.  Comma delimited output:

    `gribscan -d -i eta.T12Z.PGrbF48 | grep 184 `

    Gives the same results as the previous example but arranged differently:

    `PDS,184,104,135,108,100,0,100,1994,8,29,12,0,1,48,0,0,1e+09`

3.  A full listing:

    `gribscan -d -gv -bd -gd -i eta.T12Z.PGrbF48 | grep 184 `

    Gives the following results:

    - ` PDS,184,104,135,108,100,0,100,1994,8,29,12,0,1,48,0,0,1e+09,mconv,Horizontal moisture divergence,[kg/kg/s],GDS,5,147,110,-139.475,90.755,0.354,-0.268,-105.000,33536.000,0,1,0,BDS,12,-646.844,16170,4825059,26366 `

    Where:

    `104         ` grid id\
    `param #135   ` mconv,Horizontal moisture divergence,\[kg/kg/s\]\
    `BDS         ` binary data section\
    `646.844     ` ref value 16170 - \# of points\
    `4825059     ` starting byte of the data\
    `26366       ` length of the grib message\

    Note that eliminating the `-d` option would result in a fixed-column type output.

4.  Output a selected few fields in GRIB:

    `gribscan -og -sp135 -q -i eta.T12Z.PGrbF48 -o eta.135`

    Writes out all GRIB message containing the 135 parameter to the file `eta.135.grb`. A subsequent execution of `gribscan` on `eta.135.grb` would return:

    `1, F ,135,108,100,0,100,0,1e+09, T,1994,8,29,12,0,1,48,0, G ,104, BDTG, 94082912 2, F,135,108,21860,85,100,0,1e+09, T ,1994,8,29,12,0,1,48,0, G ,104, BDTG, 94082912`
