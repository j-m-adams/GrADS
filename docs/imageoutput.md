---
title: Producing Hardcopy and Image Output from GrADS
---

# **Producing Hardcopy and Image Output from GrADS**

## Generating Image Files from GrADS

There are several GrADS commands that will convert thecontents of the graphics window into an image file. The differencesbetween them are the image formats they support and the way they areimplemented in GrADS.

***printim***

The <a href="gradcomdprintim.html">`printim`</a> commandwas introduced in version 1.8. It produces a PNG, GIF, or JPG formatted image file based on the currentcontents of the GrADS metabuffer, which is the stuff displayed in thegraphics window, minus any widgets. `printim` will work inbatch mode.

***outxwd***

The <a href="gradcomdoutxwd.html">`outxwd`</a> command draws the contents of the graphics display window to a file in XWD (X window dump) format. It does not work in batch mode.

***wi***

The <a href="gradcomdwi.html">`wi`</a> command was deprecated long ago and is not included in version 1.9 or later.

### Generating GrADS metafiles

***1. Set-up the GrADS metafile***

The first step in creating hardcopy image output is to invoke the <a href="gradcomdenableprint.html">`enable print`</a> command -- this opens the output file*` `*and enablesGrADS to direct image information to that file. If the file exists, it will be overwritten.

***2. Display the image***

The next step is to display the graphic that you want to print. Whenyou have finished, issue the <a href="gradcomdprint.html">`print`</a>command. GrADS copies the vector instructions used to create the currentdisplay into the output file in a GrADS metacode format. For multiple images in your metafile, use <a href="gradcomdclear.html">`clear`</a>, create the new image, and then <a href="gradcomdprint.html">`print`</a> again.

***3. Close the GrADS metafile***

There are three way to close the output file:\
<a href="gradcomddisableprint.html">`disable print`</a>\
<a href="gradcomdreinit.html">`reinit`</a>\
<a href="gradcomdquit.html">`quit`</a>\

### Converting GrADS Metafiles to Postscript

GrADS metacode files may be translated into postscript using the GrADSexternal utilities <a href="gradutilgxps.html">`gxps`</a> and `gxeps`. Both utilities willprompt for input and output filenames, unless theyare provided on the command line. The input filename should be thefile created by the <a href="gradcomdenableprint.html">`enableprint`</a> command. The output filename can be anything, but a".ps" extension is conventional. Any existing file with this name willbe overwritten. Once the output file is created, you may print itusing UNIX print commands. Please consult the references pages for <a href="gradutilgxps.html">`gxps`</a> and <a href="gradutilgxeps.html">`gxeps`</a> to see all the command line arguments and options.

<a href="gradutilgxps.html">`gxps`</a> and `gxeps` are notGrADS commands. They must be executed from the UNIX command line, orpreceded by a <a href="gradcomdshell.html">`!`</a> andexecuted as a shell command from the GrADS command line.

### Generating Postscript from within GrADS

There is a shortcut for creating an encapsulated postscript (EPS) file directly from within a GrADS session: use the <a href="gradcomdprint.html">`print`</a> command without invoking the <a href="gradcomdenableprint.html">`enable print`</a> command first. This shortcut allows the user to skip the steps of creating the GrADS metafile and invoking the external utility `gxeps`. However, using this shortcut means there can only be one image per file, and none of the options available when invoking `gxeps` directly can be used.

### Displaying GrADS Metafiles

GrADS metacode files may be displayed using the GrADS external utility<a href="gradutilgxtran.html">`gxtran`</a>. The inputfilename should be the file created by the `enable print`command. If the GrADS metafile contains more than one image, `gxtran` will animate them.The animation can be automatic or controlled by the user with carriagereturns. Please consult the gxtran reference page tosee all the command line arguments and options.