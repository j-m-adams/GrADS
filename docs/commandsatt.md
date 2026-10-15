---
title: GrADS Commands by Category
---

# GrADS Commands by Category

Commands are grouped by purpose. For a alphabetic overview, expand the menu on the left. Each command links to its detailed reference page.

## Animation

| Command | Description |
| --- | --- |
| [set loopdim](gradcomdsetloopdim.md) | Sets dimension to animate |
| [set loopincr](gradcomdsetloopincr.md) | Sets looping increment |
| [set looping](gradcomdsetlooping.md) | Turns on animation when fewer than three dimesions are varying |
| [set dbuff](gradcomdsetdbuff.md) | Turns on/off double buffer mode |
| [swap](gradcomdswap.md) | Swaps buffers when in double buffer mode |

## Axis Labeling

| Command | Description |
| --- | --- |
| [draw xlab](gradcomddrawxlab.md) | Draws an X-axis label |
| [draw ylab](gradcomddrawylab.md) | Draws a Y-axis label |
| [set vrange](gradcomdsetvrange.md) | Sets the range of values for Y-axis scaling |
| [set vrange2](gradcomdsetvrange2.md) | Sets the range of values for X-axis scaling |
| [set xaxis](gradcomdsetxaxis.md) | Specifies where the labeled tick marks will be placed on the X-axis |
| [set xflip](gradcomdsetxflip.md) | Flips the order of the horizontal axis |
| [set xlab](gradcomdsetxlab.md) | Controls the format of X-axis tick mark labels |
| [set xlabs](gradcomdsetxlabs.md) | Gives specific text for X-axis labels |
| [set xlevs](gradcomdsetxlevs.md) | Specifies each individual labeled tick mark for the X-axis |
| [set xlint](gradcomdsetxlint.md) | Specifies the interval between labeled tick marks on the X-axis |
| [set xlopts](gradcomdsetxlopts.md) | Controls X-axis label options |
| [set xlpos](gradcomdsetxlpos.md) | Controls position of X-axis labels |
| [set xyrev](gradcomdsetxyrev.md) | Reverses the X and Y axes on a plot |
| [set yaxis](gradcomdsetyaxis.md) | Specifies where the labeled tick marks will be placed on the Y-axis |
| [set yflip](gradcomdsetyflip.md) | Flips the order of the vertical axis |
| [set ylab](gradcomdsetylab.md) | Controls the format of Y-axis tick mark labels |
| [set ylabs](gradcomdsetylabs.md) | Gives specific text for Y-axis labels |
| [set ylevs](gradcomdsetylevs.md) | Specifies each individual labeled tick mark for the Y-axis |
| [set ylint](gradcomdsetylint.md) | Specifies the interval between labeled tick marks on the Y-axis |
| [set ylopts](gradcomdsetylopts.md) | Controls Y-axis label options |
| [set ylpos](gradcomdsetylpos.md) | Controls position of Y-axis labels |
| [set zlog](gradcomdsetzlog.md) | Sets log scaling of the Z dimension |

## Bar Graphs

| Command | Description |
| --- | --- |
| [set barbase](gradcomdsetbarbase.md) | Sets the reference point for bar graphs |
| [set bargap](gradcomdsetbargap.md) | Sets the gap between bars for bar graphs |
| [set baropts](gradcomdsetbaropts.md) | Sets characteristics of bars for bar graphs |
| [set gxout bar](gradcomdsetgxout.md) | Graphics output type for bar graphs |
| [set gxout errbar](gradcomdsetgxout.md) | Graphics output type to show error bars |

## Color Control

| Command | Description |
| --- | --- |
| [set fgvals](gradcomdsetfgvals.md) | Assigns a color to a particular value; used with `set gxout fgrid |
| [set lfcols](gradcomdsetlfcols.md) |  |
| [set rbcols](gradcomdsetrbcols.md) | Specifies a new rainbow color sequence |
| [set rbrange](gradcomdsetrbrange.md) | Assigns a range of values to rainbow colors |
| [set rgb](gradcomdsetrgb.md) | Defines a new color |
| [set wxcols](gradcomdsetwxcols.md) | Controls color of weather symbols |

## Contour Settings

| Command | Description |
| --- | --- |
| [set annot](gradcomdsetannot.md) | Sets the color and thickness for the axis border, axis labels, and tickmarks. |
| [set black](gradcomdsetblack.md) | Specifies a range of values for which no contours will be drawn |
| [set ccolor](gradcomdsetccolor.md) | Specifies the color of the plotted contours |
| [set ccols](gradcomdsetccols.md) | Assigns specific colors for each contour level |
| [set cint](gradcomdsetcint.md) | Sets the contour interval |
| [set clab](gradcomdsetclab.md) | Controls contour labels |
| [set clevs](gradcomdsetclevs.md) | Sets specific contour levels |
| [set clopts](gradcomdsetclopts.md) | Contour label options |
| [set clskip](gradcomdsetclskip.md) | Sets the number of contour lines to skip when labelling |
| [set cmax](gradcomdsetcmax.md) | Contours not drawn above this value |
| [set cmin](gradcomdsetcmin.md) | Contours not drawn below this value |
| [set csmooth](gradcomdsetcsmooth.md) | Interpolates the grid to a finer resolution before contouring |
| [set cstyle](gradcomdsetcstyle.md) | Sets the contour line style |
| [set cterp](gradcomdsetcterp.md) | Turns on/off spline smoothing |
| [set cthick](gradcomdsetcthick.md) | Sets contour line thickness |

## Display Controls

| Command | Description |
| --- | --- |
| [clear](gradcomdclear.md) | Clears the display window and resets many graphics options |
| [display](gradcomddisplay.md) | Draws a plot |
| [reset](gradcomdreset.md) | Returns GrADS settings to default state with some exceptions |
| [set background](gradcomdsetbackground.md) | Sets background color |
| [set clip](gradcomdsetclip.md) | Sets the coordinates for clipping the plot area |
| [set cmark](gradcomdsetcmark.md) | Sets the type of line marker |
| [set display](gradcomdsetdisplay.md) | Sets the mode of display |
| [set frame](gradcomdsetframe.md) | Draws a frame around plot borders |
| [set gridln](gradcomdsetgridln.md) | Controls appearance of grid lines; used with `set gxout grid |
| [set gxout](gradcomdsetgxout.md) | Sets a graphics output type |
| [set line](gradcomdsetline.md) | Sets line attributes |
| [set missconn](gradcomdsetmissconn.md) | Connects plots lines over missing data |
| [set parea](gradcomdsetparea.md) | Specifies the area for plotting contour plots, maps, or line graphs |
| [set vpage](gradcomdsetvpage.md) | Sets dimensions of the virtual page |
| [set xsize](gradcomdsetxsize.md) | Resizes the graphics display window |

## Data and Image Output

| Command | Description |
| --- | --- |
| [disable fwrite](gradcomddisablefwrite.md) | Closes output file containing gridded data |
| [disable print](gradcomddisableprint.md) | Closes output file containing images in metacode format |
| [enable print](gradcomdenableprint.md) | Opens output file containing images in metacode format |
| [gxeps](gradutilgxeps.md) | Converts GrADS metacode format image files into postscript |
| [gxps](gradutilgxps.md) | Converts GrADS metacode format image files into postscript |
| [gxtran](gradutilgxtran.md) | Displays GrADS metacode format image files |
| [outxwd](gradcomdoutxwd.md) | Copies the contents of the display window into a file in XWD format |
| [print](gradcomdprint.md) | Copies the contents of display window to a file in a metacode format |
| [printim](gradcomdprintim.md) | Copies the contents of display window to a file in PNG or GIF format |
| [set fwrite](gradcomdsetfwrite.md) | Sets filename, byte ordering, and format for data output |
| [set gxout fwrite](gradcomdsetgxout.md) | Graphics output type for writing data to file |
| [wi](gradcomdwi.md) | Dumps the contents of the display window into a file in a variety of formats |

## (setdimension)=

| Command | Description |
| --- | --- |

## Dimension Environment

| Command | Description |
| --- | --- |
| [set x](gradcomdsetxyzte.md) | Specifies the X-dimension in grid coordinates |
| [set y](gradcomdsetxyzte.md) | Specifies the Y-dimension in grid coordinates |
| [set z](gradcomdsetxyzte.md) | Specifies the Z-dimension in grid coordinates |
| [set t](gradcomdsetxyzte.md) | Specifies the T-dimension in grid coordinates |
| [set e](gradcomdsetxyzte.md) | Specifies the E-dimension in grid coordinates |
| [set lon](gradcomdsetlatlonlevtimeens.md) | Specifies the X-dimension in world coordinates |
| [set lat](gradcomdsetlatlonlevtimeens.md) | Specifies the Y-dimension in world coordinates |
| [set lev](gradcomdsetlatlonlevtimeens.md) | Specifies the Z-dimension in world coordinates |
| [set time](gradcomdsetlatlonlevtimeens.md) | Specifies the T-dimension in world coordinates |
| [set ens](gradcomdsetlatlonlevtimeens.md) | Specifies the E-dimension in world coordinates |

## File I/O

| Command | Description |
| --- | --- |
| [close](gradcomdclose.md) | Closes a GrADS data file |
| [open](gradcomdopen.md) | Opens a GrADS data file |
| [reinit](gradcomdreinit.md) | Returns GrADS to its initial state |
| [exec](gradcomdexec.md) | Executes the list of GrADS commands contained in a file |
| [run](gradcomdrun.md) | Runs a GrADS script |
| [sdfopen](gradcomdsdfopen.md) | Opens a NetCDF of HDF-SDS file that conforms to the COARDS conventions |
| [xdfopen](gradcomdxdfopen.md) | Opens a NetCDF of HDF-SDS file that does not conform to the COARDS conventions |
| [set dfile](gradcomdsetdfile.md) | Changes default file |
| [set imprun](gradcomdsetimprun.md) | Sets up automatic script execution before every display command |

## GrADS-User Interface

| Command | Description |
| --- | --- |
| [grads](gradcomdgrads.md) | Starts the GrADS program |
| [help](gradcomdhelp.md) | Lists a few basic GrADS commands |
| [query](gradcomdquery.md) | Returns information about a variety of aspects of the current GrADS session |
| [quit](gradcomdquit.md) | Quits GrADS |
| [set datawarn](gradcomdsetdatawarn.md) | Prints "Entire Grid Undefined" in display window if all data are missing |
| [set stat](gradcomdsetstat.md) | Turns on/off printing of statistical information for each display |
| [set warn](gradcomdsetwarn.md) | Turns on/off messages about the progress of certain mathematical operations |
| [!shell](gradcomdshell.md) | Sends a command to the shell |

## Graphical Elements

| Command | Description |
| --- | --- |
| [draw line](gradcomddrawline.md) | Draws a line |
| [draw mark](gradcomddrawmark.md) | Draws a mark |
| [draw polyf](gradcomddrawpolyf.md) | Draws a filled polygon |
| [draw rec](gradcomddrawrec.md) | Draws a rectangle |
| [draw recf](gradcomddrawrecf.md) | Draws a filled rectangle |
| [draw wxsym](gradcomddrawwxsym.md) | Draws a weather symbol |

## GRIB Utilities

| Command | Description |
| --- | --- |
| [gribmap](gradutilgribmap.md) | Creates a map of data sets in GRIB format |
| [gribscan](gradutilgribscan.md) | Extracts grid info from data sets in GRIB format |

## Map Settings

| Command | Description |
| --- | --- |
| [draw map](gradcomddrawmap.md) | Draws a map outline |
| [set grid](gradcomdsetgrid.md) | Sets characteristics of displayed grid lines |
| [set map](gradcomdsetmap.md) | Sets map background characteristics |
| [set mpdraw](gradcomdsetmpdraw.md) | Turns on/off drawing of map background |
| [set mpdset](gradcomdsetmpdset.md) | Sets the resolution of the coastal outline |
| [set mproj](gradcomdsetmproj.md) | Sets current map projection |
| [set mpt](gradcomdsetmpt.md) | Controls map background characteristics |
| [set mpvals](gradcomdsetmpvals.md) | Sets reference longitudes and latitudes for polar stereographic plots |
| [set poli](gradcomdsetpoli.md) | Turns on/off the drawing of political boundaries |

## Plot Annotation and Labeling

| Command | Description |
| --- | --- |
| [draw string](gradcomddrawstring.md) | Draws a string anywhere on the page |
| [draw title](gradcomddrawtitle.md) | Draws a title centered over a plot |
| [set dignum](gradcomdsetdignum.md) | Sets the number of significant digits after the decimal point |
| [set digsiz](gradcomdsetdigsize.md) | Sets the size of plotted numbers |
| [set font](gradcomdsetfont.md) | Selects the font for text display |
| [set grads](gradcomdsetgrads.md) | Turns on/off the GrADS logo in each plot |
| [set string](gradcomdsetstring.md) | Sets string drawing attributes |
| [set strsiz](gradcomdsetstrsiz.md) | Sets the string character size |
| [set timelab](gradcomdsettimelab.md) | Turns on/off display of the time label |
| [set tlsupp](gradcomdsettlsupp.md) | Suppresses the annotation of the year and/or month in date/time labels |

## Station Data

| Command | Description |
| --- | --- |
| [collect](gradcomdcollect.md) | Saves station data in memory as a set |
| [set mdlopts](gradcomdsetmdlopts.md) |  |
| [set wxopt](gradcomdsetwxopt.md) | Controls weather symbol output; used with `set gxout wxsym |
| [set stid](gradcomdsetstid.md) | Turns on/off display of the station ID next to the data values |
| [set stnprint](gradcomdsetstnprint.md) | Controls printing of station data values; used with `set gxout stat |
| [stnmap](gradutilstnmap.md) | Writes out a hash table and/or link list information for station data |

## Variables

| Command | Description |
| --- | --- |
| [define](gradcomddefine.md) | Creates a new GrADS variable that is loaded into memory |
| [modify](gradcomdmodify.md) | Defines a climatological variable |
| [set defval](gradcomdsetdefval.md) | Interactively modifies grid point values for 2-D defined variables |
| [undefine](gradcomdundefine.md) | Frees the memory used by a defined variable |

## Vectors

| Command | Description |
| --- | --- |
| [set arrlab](gradcomdsetarrlab.md) | Toggles drawing the vector arrow label |
| [set arrowhead](gradcomdsetarrowhead.md) | Sets the size of the vector arrowhead |
| [set arrscl](gradcomdsetarrscl.md) | Specifies arrow length scaling |
| [set gxout vector](gradcomdsetgxout.md) | Graphics output type for vector plots |
| [set hempref](gradcomdsethempref.md) | Controls wind barb drawing conventions |
| [set strmden](gradcomdsetstrmden.md) | Sets density of streamlines; used with `set gxout stream |

## Widgets

| Command | Description |
| --- | --- |
| [draw button](gradcomddrawbutton.md) | Draws a button widget |
| [redraw button](gradcomdredrawbutton.md) | Resets a button widget on/off |
| [draw dropmenu](gradcomddrawdropmenu.md) | Draws a dropmenu widget |
| [set button](gradcomdsetbutton.md) | Specifies the color characteristics of a button widget |
| [set dialog](gradcomdsetdialog.md) | Sets color properties of dialog box widgets |
| [set dropmenu](gradcomdsetdropmenu.md) | Sets color properties of dropmenu widgets |
| [set rband](gradcomdsetrband.md) | Sets characteristics for the 'rubber band' widget |

(draw)=
(set)=

```{toctree}
:hidden:
:maxdepth: 1

!shell-command <gradcomdshell>
clear <gradcomdclear>
close <gradcomdclose>
collect <gradcomdcollect>
define <gradcomddefine>
disable fwrite <gradcomddisablefwrite>
disable print <gradcomddisableprint>
display <gradcomddisplay>
draw button <gradcomddrawbutton>
draw dropmenu <gradcomddrawdropmenu>
draw line <gradcomdddrawline>
draw line <gradcomddrawline>
draw map <gradcomddrawmap>
draw mark <gradcomddrawmark>
draw polyf <gradcomddrawpolyf>
draw rec <gradcomddrawrec>
draw recf <gradcomddrawrecf>
draw shp <gradcomddrawshp>
draw string <gradcomddrawstring>
draw title <gradcomddrawtitle>
draw wxsym <gradcomddrawwxsym>
draw xlab <gradcomddrawxlab>
draw ylab <gradcomddrawylab>
enable print <gradcomdenableprint>
exec <gradcomdexec>
flush <gradcomdflush>
Gradcomdgrads <gradcomdgrads>
Gradcomdstat <gradcomdstat>
Gradcomdtserbarb <gradcomdtserbarb>
Gradcomdtserwx <gradcomdtserwx>
gxprint <gradcomdgxprint>
help <gradcomdhelp>
modify <gradcomdmodify>
open <gradcomdopen>
outxwd <gradcomdoutxwd>
pdefwrite <gradcomdpdefwrite>
print <gradcomdprint>
printim <gradcomdprintim>
q attr <gradcomdqattr>
q dbf <gradcomdqdbf>
q define <gradcomdqdefine>
q dialog <gradcomdqdialog>
q ens <gradcomdqens>
q fwrite <gradcomdqfwrite>
q pos <gradcomdqfile>
q pos <gradcomdqpos>
q sdfwrite <gradcomdqsdfwrite>
q shades <gradcomdqshades>
q shp <gradcomdqshp>
q shpopts <gradcomdqshpopts>
query <gradcomdquery>
quit <gradcomdquit>
redraw button <gradcomdredrawbutton>
reinit <gradcomdreinit>
reset <gradcomdreset>
run <gradcomdrun>
screen <gradcomdscreen>
sdfopen <gradcomdsdfopen>
sdfwrite <gradcomdsdfwrite>
set annot <gradcomdsetannot>
set antialias <gradcomdsetantialias>
set arrlab <gradcomdsetarrlab>
set arrowhead <gradcomdsetarrowhead>
set arrscl <gradcomdsetarrscl>
set background <gradcomdsetbackground>
set barbase <gradcomdsetbarbase>
set barbopts <gradcomdsetbarbopts>
set bargap <gradcomdsetbargap>
set baropts <gradcomdsetbaropts>
set black <gradcomdsetblack>
set button <gradcomdsetbutton>
set cachesf <gradcomdsetcachesf>
set ccolor <gradcomdsetccolor>
set ccols <gradcomdsetccols>
set chunksize <gradcomdsetchunksize>
set cint <gradcomdsetcint>
set clab <gradcomdsetclab>
set clevs <gradcomdsetclevs>
set clip <gradcomdsetclip>
set clopts <gradcomdsetclopts>
set clskip <gradcomdsetclskip>
set cmark <gradcomdsetcmark>
set cmax <gradcomdsetcmax>
set cmin <gradcomdsetcmin>
set coslat <gradcomdsetcoslat>
set csmooth <gradcomdsetcsmooth>
set cstyle <gradcomdsetcstyle>
set cterp <gradcomdsetcterp>
set cthick <gradcomdsetcthick>
set datawarn <gradcomdsetdatawarn>
set dbuff <gradcomdsetdbuff>
set defval <gradcomdsetdefval>
set dfile <gradcomdsetdfile>
set dialog <gradcomdsetdialog>
set dignum <gradcomdsetdignum>
set digsiz <gradcomdsetdigsize>
set display <gradcomdsetdisplay>
set dropmenu <gradcomdsetdropmenu>
set fgvals <gradcomdsetfgvals>
set font <gradcomdsetfont>
set frame <gradcomdsetframe>
set fwrite <gradcomdsetfwrite>
set geotiff <gradcomdsetgeotiff>
set grads <gradcomdsetgrads>
set grid <gradcomdsetgrid>
set gridln <gradcomdsetgridln>
set gxout <gradcomdsetgxout>
set hempref <gradcomdsethempref>
set hershey <gradcomdsethershey>
set imprun <gradcomdsetimprun>
set kml <gradcomdsetkml>
set lat\|lon\|lev\|time\|ens <gradcomdsetlatlonlevtimeens>
set lats <gradcomdsetlats>
set lfcols <gradcomdsetlfcols>
set line <gradcomdsetline>
set log1d <gradcomdsetlog1d>
set loopdim <gradcomdsetloopdim>
set loopincr <gradcomdsetloopincr>
set looping <gradcomdsetlooping>
set lwid <gradcomdsetlwid>
set map <gradcomdsetmap>
set mdlopts <gradcomdsetmdlopts>
set missconn <gradcomdsetmissconn>
set misswarn <gradcomdsetmisswarn>
set mpdraw <gradcomdsetmpdraw>
set mpdset <gradcomdsetmpdset>
set mproj <gradcomdsetmproj>
set mpt <gradcomdsetmpt>
set mpvals <gradcomdsetmpvals>
set parea <gradcomdsetparea>
set poli <gradcomdsetpoli>
set prnopts <gradcomdsetprnopts>
set rband <gradcomdsetrband>
set rbcols <gradcomdsetrbcols>
set rbrange <gradcomdsetrbrange>
set rgb <gradcomdsetrgb>
set sdfattr <gradcomdsetsdfattr>
set sdfwrite <gradcomdsetsdfwrite>
set shp <gradcomdsetshp>
set shpattr <gradcomdsetshpattr>
set shpopts <gradcomdsetshpopts>
set stat <gradcomdsetstat>
set stid <gradcomdsetstid>
set stnprint <gradcomdsetstnprint>
set string <gradcomdsetstring>
set strmden <gradcomdsetstrmden>
set strsiz <gradcomdsetstrsiz>
set tile <gradcomdsettile>
set timelab <gradcomdsettimelab>
set tlsupp <gradcomdsettlsupp>
set undef <gradcomdsetundef>
set vpage <gradcomdsetvpage>
set vrange <gradcomdsetvrange>
set vrange2 <gradcomdsetvrange2>
set warn <gradcomdsetwarn>
set wxcols <gradcomdsetwxcols>
set wxopt <gradcomdsetwxopt>
set x\|y\|z\|t\|e <gradcomdsetxyzte>
set xaxis <gradcomdsetxaxis>
set xflip <gradcomdsetxflip>
set xlab <gradcomdsetxlab>
set xlabs <gradcomdsetxlabs>
set xlevs <gradcomdsetxlevs>
set xlint <gradcomdsetxlint>
set xlopts <gradcomdsetxlopts>
set xlpos <gradcomdsetxlpos>
set xsize <gradcomdsetxsize>
set xyrev <gradcomdsetxyrev>
set yaxis <gradcomdsetyaxis>
set yflip <gradcomdsetyflip>
set ylab <gradcomdsetylab>
set ylabs <gradcomdsetylabs>
set ylevs <gradcomdsetylevs>
set ylint <gradcomdsetylint>
set ylopts <gradcomdsetylopts>
set ylpos <gradcomdsetylpos>
set zlog <gradcomdsetzlog>
swap <gradcomdswap>
undefine <gradcomdundefine>
wi <gradcomdwi>
xdfopen <gradcomdxdfopen>
```
