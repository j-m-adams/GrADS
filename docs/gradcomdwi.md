---
title: wi
---

# **wi**

N.B.: This command was deprecated long ago and is not included in GrADS version 1.9 or later.

The `wi` (write image) command dumps an exact copy of the contents of the GrADS graphics screen directly to an image file. This feature is implemented by means of an interface to the ImageMagick library. The syntax is:

- `wi `*`filename.fmt`*

where *`filename`* can be any meaningful file name,and *`fmt`* is one of the ImageMagick supportedformats: gif, bmp, cgm, eps, fax, ico, jpeg, pcx, hdf, andmany more.

## Usage Notes

1.  Because it requires a connection to an X-server, `wi`does not work when running GrADS in batch mode (-b option). A similarcommand that *does* work in batch mode is `printim`.
2.  Some ImageMagick formats (tiff, png, mpeg, et al.) are not yetsupported by GrADS. If a specified format is not recognized orsupported, the image will be saved in MIFF, ImageMagick's nativeformat. If a file name extension is *not* specified, GIF is the default.
3.  Make sure no other window is on top of your GrADS graphics windowswhen issuing a `wi` command, because `wi` takesa snapshot of the current state of this window. If another window ispartially covering your graphics window, your image file will containthe combination of these 2 windows.

### Examples

To try out `wi`, display something on the graphics screen,and then at the ga-\> prompt enter:

- `wi test.gif ` for writing a GIF file\
  `wi test.jpeg` for writing a JPEG file\
  `wi test.bmp ` for writing a Windows BMP file\

### Bugs for Win32 GrADS Users

1.  On Windows 95 systems, GradsHDF, the version built with the NCSAMFHDF library, has a subtle bug when reading certain 16-bit (short)packed NetCDF files (e.g. NCEP re-analysis files converted toNetCDF at CIRES/CDC). Although GrADS is able to produce a plot, itmistankenly produces very large values for a few gridpoints. Work around: use GradsNC in such cases. This problem has notbeen reported on Windows NT systems.\
2.  X Windows widgets built with gs scripts may miss/delay some Xevents (keyboard/mouse clicks). This problem appears to occur only onWindows 95; it has not been reported on Windows NT.\
3.  Athena widgets built with gui scripts: menu items become invisiblewhen selected (MI/X Server only); some mouse delays on Windows 95.\
4.  `wi` has been reported to produce black and white GIFoutput on VGA systems with more than 256 colors. The workaround is towrite a JPEG image, or to change your Windows display to 256 colors.
