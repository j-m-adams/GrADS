---
title: pdefwrite
---

# **pdefwrite**

When GrADS opens a descriptor with a <a href="pdef.html">`PDEF`</a> entry that contains options such as `lcc`, `lccr`, `nps`, `sps`, et. al, it calculates the interpolation weights internally and stores them in memory for later use. These calculations can take some time for high resolution grids. There is a significant performance advantage gained from reading in the interpolation weights from an external file instead of calculating them every time you open the descriptor. Introduced in GrADS version 2.1.0, the `pdefwrite` command will write out the interpolation weights that have been calculated internally for these special types of <a href="pdef.html">`PDEF`</a> entries. The file it creates can be used with a ‘<a href="pdef.html">`PDEF`</a>` bilin`’ entry instead.

## Syntax

- `pdefwrite `*`filename`*` `

where:

- *`filename`*`  `The name of the output file. If this file exists, it will be replaced.\
  `           `\
  \

### Usage Notes

1.  `pdefwrite` works with GrADS version 2.1.0+.
2.  The `pdefwrite` command will return an error message if you use it with a default file that does not use <a href="pdef.html">`PDEF`</a> or already has an external <a href="pdef.html">`PDEF`</a> file. If there are no errors, `pdefwrite` will return a complete <a href="pdef.html">`PDEF`</a> entry that points to the file it just created.

### Example

1.  Suppose you have a descriptor that contains the following three entries:
    - `pdef 4736 3000 lcc 23.00 -120 1 1 40.0 40.0 -100 1016.2360 1016.150`\
      `xdef 6650 linear -130.0 0.01`\
      `ydef 3500 linear 20.0 0.01 `
2.  Open this descriptor, then invoke pdefwrite with a file name as an argument.
    - `ga-> pdefwrite myfile.pdef`\
3.  GrADS will return the syntax for the new <a href="pdef.html">`PDEF`</a> entry:
    - `pdef 4736 3000 bilin stream binary-little ^myfile.pdef `
4.  Rewrite your descriptor to use this new <a href="pdef.html">`PDEF`</a> entry instead. Don’t change the <a href="descriptorfile.html#XDEF">`XDEF`</a> and <a href="descriptorfile.html#YDEF">`YDEF`</a> statements — those match the <a href="pdef.html">`PDEF`</a> file you created.
5.  Keep the <a href="pdef.html">`PDEF`</a> file (`myflile.pdef`) and the descriptor file together in the same directory -- this is important because of the ^ before the <a href="pdef.html">`PDEF`</a> file's name in the <a href="pdef.html">`PDEF`</a> entry. If you want to put the descriptor and <a href="pdef.html">`PDEF`</a> files in separate locations, change the ^ in the entry to the <a href="pdef.html">`PDEF`</a> file's full path, like this:
    - `pdef 4736 3000 bilin stream binary-little /your/path/to/myfile.pdef`
