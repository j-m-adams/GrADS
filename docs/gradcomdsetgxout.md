---
title: set gxout
---

# **set gxout**

`set gxout `*`graphics_type`*

Where *`graphics_type`* can be one of the following:

- `bar       `Bar chart (The <a href="library.html">library</a> contains an [example script](https://github.com/rickedanielson/grads.lib) demonstrating how to use gxout bar and errbar)\
  `barb      `Wind barbs\
  `contour   `Contour plot\
  `errbar    `Error bars (The <a href="library.html">library</a> contains an [example script](https://github.com/rickedanielson/grads.lib) demonstrating how to use gxout bar and errbar)\
  `geotiff   ` (2.0.a5+) Generates a GeoTIFF format data file; options specified by <a href="gradcomdsetgeotiff.html">`set geotiff`</a>\
  `grfill    `Shaded grid boxes\
  `fgrid     `Shaded grid boxes with values specified by <a href="gradcomdsetfgvals.html">`set fgvals`</a>\
  `fwrite    `Writes data to file instead of drawing a plot; options specified by <a href="gradcomdsetfwrite.html">`set fwrite`</a>\
  `grid      `Grid boxes with printed values\
  `imap      `(2.0.a8+) Quickly drawn shaded grid boxes. No metafile, so no hard copy; image output only with <a href="gradcomdoutxwd.html">`outxwd`</a>.\
  `kml       ` (2.0.a5+) Generates a TIFF image file and a KML text file; options specified by <a href="gradcomdsetkml.html">`set kml`</a>\
  `line      `Line Graph\
  `linefill  `Color fill between two lines\
  `print     `Generates ascii output for anything displayed; controlled by<a href="gradcomdsetprnopts.html">`set prnopts`</a>\
  `scatter   `Scatter plot\
  `shaded    `Shaded contour plot, alias for original algorithm (shade1)\
  `shade1    `(2.0.0+) Shaded contour plot, original algorithm\
  `shade2    `(2.0.0+) Shaded contour plot, new algorithm, polygons merged to be larger and fewer in number (faster to render)\
  `shade2b   `(2.0.0+) Shaded contour plot, new algorithm, polygons all on sub-grid scale (slower to render)\
  `shp       `(2.0.a9+) Generates a shapefile; options specified by <a href="gradcomdsetshp.html">`set shp`</a>\
  `stream    `Wind streamlines\
  `stat      `Prints statistical output to terminal instead of drawing a plot\
  `vector    `Wind vector arrows\

For station data, these additional graphics output types are also available:

- `findstn   `Finds nearest station\
  `model     `Plots station model\
  `stnmark   `Plots a mark at station location that is colorized by data value\
  `tserbarb  `Time series of wind barbs at a point (1-D)\
  `tserwx    `Time series of weather symbols at a point (1-D)\
  `value     `Plots station values\
  `wxsym     `Plots weather symbols at station, controlled by <a href="gradcomdsetwxopt.html">`set wxopt`</a>\

## Usage Notes

1.  
2.  For the graphics output types `vector, stream,` and `barb`, the plotting routines need two result grids, where the first result grid is treated as the U component, and the second result grid is treated as the V component. These two result grids are provided with the <a href="gradcomddisplay.html">display</a> command by entering two expressions separated by a semicolon:
    - ` display u ; v`\
      `display ave(u,t=1,t=10) ; ave(v,t=1,t=10)`
3.  For the graphics output types `vector` and `stream`, you can specify a third result grid that will be used to colorize the vectors or streamlines:
    - ` display u ; v ; mag(u,v)`\
      `display u ; v ; hcurl(u,v)`
4.  To draw a 1-D time series with graphics output types vector or barb, set the dimension environment so that only time is varying, then :
    - ` display const(u,0); u ; v `
5.  For the graphics output type `wxsym`, each value at a station location is assumed to be a wx symbol code number. To see a chart of all available wx symbols and their corresponding code numbers, run the sample script `wxsym.gs`.
6.  The graphics output type `findstn` requires three arguments to be provided with the `display` command. The first argument is a station data expression. The 2nd and 3rd arguments are the X and Y screen coordinates of the of the desired search location. GrADS will find the station closest to the specified X and Y position, and print its stid, lon, and lat. This graphics output type should only be used when X and Y are the varying dimensions and AFTER a regular display command (that results in graphics output) is entered.
7.  For the graphics output type `stnmark`, the size and style of the mark are controled by <a href="gradcomdsetdigsize.html">`set digsiz`</a> and <a href="gradcomdsetcmark.html">`set cmark`</a>. To turn off rainbow colorizing and use a single color instead, use
    `set ccolor `*`color#`*.

### Examples

The <a href="library.html">library</a> contains an [example script](https://github.com/rickedanielson/grads.lib) demonstrating how to use gxout bar and errbar
