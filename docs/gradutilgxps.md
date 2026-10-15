---
title: Gradutilgxps
---

GrADS Utilities: gxps

# 
(gxps)=
gxps

`gxps [ -`*`crd -i <infile> -o <outfile>`*` ] ]`

`gxps` is a UNIX utility that converts the GrADS metacode format file to aPostScript file. Command line arguments and switches are:

- `-i `*`fname`*`    `identifies input GrADS metacode file\
  `-o `*`fname`*`    `identifies output postscript file\
  `-c          `prints color plot\
  `-r          `prints on a black background\
  `-d          `does not append CTRL-D to the file, useful if printing on a HP1200C/PS color printer\

**Usage Notes**

1.  The default behaviour of `gxps` is to create a grayscaleplot on a white background. The GrADS default rainbow colors (colornumbers 2 to 14) are converted into appropriate greyshades. User-defined colors (numbers above 15) are translated togreyscale intensity based on their *green* content only.
2.  For more information, see the section in the User's Guide on <a href="imageoutput.html">Producing Image Output from GrADS</a>.

**Example**

- `gxps -cr -i mytest.mf -o mytest.ps`

Convert GrADS metacode format file `mytest.mf` to a colorplot on a black background and outputs the result to PostScript file`mytest.ps`.
