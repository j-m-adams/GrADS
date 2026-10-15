---
title: print
---

# print

`print `*`<fname>`*` `

If a GrADS metafile has already been opened with the <a href="gradcomdenableprint.html">`enable print`</a> command, then the `print` command copies the contents of the current display into the metafile.

If a GrADS metafile is not open, then the `print` command will create an encapsulated postscript (EPS) file based on the contents of the current display. The optional argument *`fname`* is the output filename (`grads.eps` by default).

## Usage Notes

For additional guidance on how to use the print command to create a GrADS metafile, see the notes in the documentation page for <a href="gradcomdenableprint.html">`enable print`</a>.

Using the `print` command without opening a metafile first is essentially a quick shortcut for creating an EPS file from within GrADS. It allows the user to skip the steps of creating the GrADS metafile and invoking the external utility <a href="gradutilgxeps.html">`gxeps`</a>. However, using this shortcut means there can only be one image per file, and none of the options available when invoking <a href="gradutilgxeps.html">`gxeps`</a> directly can be used -- you get the default behavior of <a href="gradutilgxeps.html">`gxeps`</a> and are only allowed to specify the output filename.

###
