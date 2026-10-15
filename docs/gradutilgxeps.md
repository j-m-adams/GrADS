---
title: Gradutilgxeps
---

GrADS Utilities: gxeps

# 
(gxeps)=
gxeps

`gxeps [ -`*`acdklrsv -i <infile> -o <outfile>`*` ] [`*`<infile>`*`]`

Converts the GrADS metacode format file to a PostScript file. Where:

- `-i `*`fname`*`    ` identifies input GrADS metacode file \
  `-o `*`fname`*`    ` identifies output postscript file \
  `-b          ` prints black and white plot\
  `-c          ` prints color plot (default)\
  `-r          ` prints on a black background\
  `-d          ` appends CTRL-D to the file, useful if printing on a HP1200C/PS color printer\
  `-1          ` use PostScript Level 1 (default)\
  `-2          ` use PostScript Level 2 \
  `-a          ` Page size A4 \
  `-l          ` Page size US-Letter \
  `-L          ` Ask for a label to be printed on the plot \
  `-n          ` Ask for a note to include in PostScript file header \
  `-s          ` Add a file and time stamp on the plot.\
  `-v          ` Verbose mode.

**Usage Notes**

1.   The default behaviour of `gxeps`is to create a grayscale plot on a white background. The GrADS default rainbow colors (color numbers 2 to 14) are converted into appropriate grey shades. User-defined colors (numbers above 15) are translated to greyscale intensity based on their *green* content only. 
2.   For more information, see the section in the User's Guide on <a href="imageoutput.html">Producing Image Output from GrADS</a>. 

**Examples**

1.  `gxeps -a`\
    Prompts user for the input and output file names and convert to greyscale on white background with page size A4. 
2.  `gxeps -rc -i mytest.mf -o mytest.ps`\
    Converts GrADS metacode format file *`mytest.mf`* to a color plot on black background and outputs the result to PostScript file *`mytest.ps`*.
