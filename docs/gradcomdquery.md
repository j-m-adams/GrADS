---
title: query
---

# **query**

The `query` command allows the user to get information about a variety of aspects of the current GrADS session. Configuration, plot characteristics, graphics specifics, and file structure are some examples. The use of `query pos` combined with the coordinate transformations is the basis of many interactive applications with buttons and drop menus. The `query` command may be shortened to simply `q`. The syntax is:

- `query `*`<option>`*\
  or\
  `q `*`<option>`*

When given without an *`option`*, the `query` command returns a list of the possible options. These are:

> |  |  |
> |----|----|
> | <a href="gradcomdqattr.html">attr</a> n | Returns all attributes for file n (or default file if n is omitted) |
> | cache n | Returns netcdf4/hdf5 cache size for file n (or default file if n is omitted) (2.0.a8+) |
> | cachesf | Returns netcdf4/hdf5 cache scale factor (2.0.a8+) |
> | calendar | Returns calendar mode: unset, 365-day, or standard |
> | config | Returns GrADS configuration information |
> | contours | Returns colors and levels of a line contours (2.0.a8+) |
> | ctlinfo | Returns contents of data descriptor file |
> | <a href="gradcomdqdbf.html">dbf</a> | Lists the contents of a shapefile attribute database (2.0.a8+) |
> | define | Lists currently defined variables |
> | defval v1 i j | Returns the value of defined variable *`v1`* at point *`i,j`* |
> | <a href="gradcomdqdialog.html">dialog</a> args | Launches a dialog box that prompts for text or numeric data entry |
> | dims | Returns current dimension environment |
> | <a href="gradcomdqens.html">ens</a> | Returns ensemble metadata |
> | fgvals | Returns values and colors specified for gxout fgrid (2.1.a2+) |
> | file n | Returns info on file number *`n`*. Uses the default file if *`n`* is not given. |
> | files | Lists open files |
> | font n | Returns info on font number *`n`*. Uses the default font if *`n`* is not given. (2.2.0) |
> | <a href="gradcomdqfwrite.html">fwrite</a> | Returns status and characteristics of fwrite ouput file |
> | gxconfig | Returns configuration information about the graphics plug-ins (2.2.0) |
> | gxinfo | Returns graphics environment info |
> | gxout | Returns current gxout settings |
> | lats | Returns the status of the GrADS-LATS interface |
> | <a href="gradcomdqpos.html">pos</a> | Waits for mouse click, then returns position plus additional widget information |
> | <a href="gradcomdqsdfwrite.html">sdfwrite</a> | Returns the status of the sdfwrite options |
> | <a href="gradcomdqshades.html">shades</a> | Lists colors and levels of shaded contours |
> | <a href="gradcomdqshp.html">shp</a> | Lists the contents of a shapefile (2.0.a8+) |
> | <a href="gradcomdqshpopts.html">shpopts</a> | Returns settings for drawing and writing shapefiles (2.0.a9+) |
> | string str | Returns the width of string *`str`* in virtual page inches |
> | time | Returns info about time settings |
> | udft | Returns the user defined function table |
> | undef | Returns the output undefined value |
> | xinfo | Returns characteristics of the graphics display window |
> | xy2w  v1 v2 | Converts XY coords to world coords |
> | xy2gr v1 v2 | Converts XY coords to grid coords |
> | w2xy  v1 v2 | Converts world coords to XY coords |
> | w2gr  v1 v2 | Converts world coords to grid coords |
> | gr2w  v1 v2 | Converts grid coords to world coords |
> | gr2xy v1 v2 | Converts grid coords to XY coords |
> | pp2xy v1 v2 | Converts virtual page XY coords to real page XY coords |

` `\

## Usage Notes

For more information on the use of the `query` command, see the section of the User's Guide on <a href="script.html#commands">commands that complement the scripting language</a>.

### Examples
