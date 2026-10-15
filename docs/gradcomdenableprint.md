---
title: enable print
---

# enable print

`enable print `*`fname`*

This command opens the output file ` `*`fname`* that will contain the instructions in GrADS metacode format to create a hardcopy of the graphical display. Any existing contents of *`fname`* will be lost.The output file ` `*`fname`* is referred to as a GrADS metafile.

## Usage Notes

Creating a GrADS metafile for vector-graphic hardcopy involves four steps:

1\. Open the metafile with the <a href="gradcomdenableprint.html">`enable print`</a>` ` command\
2. Create the graphical display that you want to print\
3. Issue the <a href="gradcomdprint.html">`print`</a> command. For multiple images in your metafile, use <a href="gradcomdclear.html">`clear`</a> and then <a href="gradcomdprint.html">`print`</a> again.\
4. Close the metafile with the <a href="gradcomddisableprint.html">`disable print`</a>` ` command

After you have created a GrADS metafile, you can

1\. convert it to postscript using the external utility <a href="gradutilgxps.html">`gxps`</a>\
2. convert it to encapsulated postscript using <a href="gradutilgxeps.html">`gxeps`</a> or you can\
3. display it using the external utilities <a href="gradutilgxtran.html">`gxtran`</a> or [`gv32.exe`](https://github.com/rickedanielson/grads.lib) (for MS Windows)
