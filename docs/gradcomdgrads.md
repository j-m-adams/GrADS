---
title: Gradcomdgrads
---

**grads \| gradsdap    \
gradsc \| gradsnc \| gradshdf \| gradsdods**   
-----------------------------------------------

**Installation:** use the conda-forge package: `mamba create -n grads -c conda-forge grads`, followed by `mamba activate grads`. The package includes the data files and graphics plug-ins. Its activation hook configures `GADDIR`, `GAUDPT`, and `GAGPY`, so users should not set these variables or library paths manually.

<table width="700" data-border="0">
<colgroup>
<col style="width: 100%" />
</colgroup>
<tbody>
<tr>
<td><p>GrADS is an interactive desktop tool for the analysis and display of earth science data. GrADS is used worldwide and [freely available](../downloads) over the internet.</p>
<p>GrADS implements two data models: a 5-Dimensional gridded data model, and a station data model. In the gridded data model, the dimensions are presumed to be latitude, longitude, level, time, and <a href="ensembles.html">ensemble</a>. In the station data model, data exist at arbitrary locations in space and time. Four dimensions (longitude, latitude, level, and time) are used as a framework in the station data model to guide which station reports are to be examined. Each data set is placed within a 4- or 5-Dimensional space by the use of a data descriptor file. Both gridded and station data may be described. Gridded data may be non-linearly spaced; gaussian grids and variable resolution ocean model grids are directly supported. The internal data format in a file may be binary, GRIB1, GRIB2, BUFR, NetCDF, HDF4-SDS, or HDF5.</p>
<p>Operations may be performed on the data directly, and interactively, by entering expressions at the command line. The expression syntax allows complex operations that range over very large amounts of data to be performed with simple expressions. A rich set of built-in <a href="functions.html">functions</a> are provided. In addition, users may add their own functions as <a href="udp.html">user-defined plug-ins</a> written in any programming language.</p></td>
</tr>
</tbody>
</table>

The syntax for running GrADS is:

- *`GrADS_executable`*` < `*`options`*` >`

<table class="plaintext" width="680" data-border="0" data-cellpadding="5" data-cellspacing="5">
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<tbody>
<tr data-bgcolor="c5d5ff">
<td colspan="2" data-bgcolor="e0f0ff">Beginning with GrADS version 2.0.a8, there is only one choice for <em><code>GrADS_executable,</code></em><code> </code> a single, fully-featured build:</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="75" data-bgcolor="c5d5ff"><code>grads</code></td>
<td>Reads GRIB (version 1 and 2), gridded binary, BUFR, GrADS station data, NetCDF (classic and NetCDF-4), HDF4-SDS, HDF5, and OPeNDAP (grids and station data)<br />
Writes binary, NetCDF (classic and NetCDF-4), GeoTIFF, KML<br />
Draws shapefiles</td>
</tr>
</tbody>
</table>

<table class="plaintext" width="680" data-border="0" data-cellpadding="5" data-cellspacing="5">
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<tbody>
<tr data-bgcolor="c5d5ff">
<td colspan="2" data-bgcolor="e0f0ff">For GrADS versions 2.0.a0 through 2.0.a7, the choices for <em><code>GrADS_executable</code></em> are:</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="75" data-bgcolor="c5d5ff"><code>grads</code></td>
<td>Reads GRIB (version 1 and 2), gridded binary, BUFR, GrADS station data, NetCDF, HDF4-SDS<br />
Writes binary, NetCDF, (starting with 2.0.a5) GeoTIFF, KML</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="75" data-bgcolor="c5d5ff"><code>gradsdap</code></td>
<td>Reads GRIB (version 1 and 2), gridded binary, BUFR, GrADS station data, NetCDF, HDF4-SDS, OPeNDAP<br />
Writes binary, NetCDF, (starting with 2.0.a5) GeoTIFF, KML</td>
</tr>
</tbody>
</table>

<table class="plaintext" width="680" data-border="0" data-cellpadding="5" data-cellspacing="5">
<colgroup>
<col style="width: 50%" />
<col style="width: 50%" />
</colgroup>
<tbody>
<tr data-bgcolor="c5d5ff">
<td colspan="2" data-bordercolor="#F4F4F4" data-bgcolor="#E0F0FF">For GrADS version 1.9 and earlier, the choices for <em><code>GrADS_executable</code></em> are:</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>gradsc</code></td>
<td>Reads GRIB1, gridded binary, BUFR, GrADS station data<br />
Writes binary, GRIB1</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>gradsnc</code></td>
<td>Reads GRIB1, gridded binary, BUFR, GrADS station data, NetCDF<br />
Writes binary, GRIB1, NetCDF</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>gradshdf</code></td>
<td><p>Reads GRIB1, gridded binary, BUFR, GrADS station data, NetCDF, HDF4-SDS<br />
Writes binary, GRIB1, HDF-SDS</p></td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>gradsdods</code></td>
<td>Reads GRIB1, gridded binary, BUFR, GrADS station data, NetCDF, OPeNDAP (aka DODS)<br />
Writes binary, GRIB1, NetCDF</td>
</tr>
<tr>
<td> </td>
<td> </td>
</tr>
<tr>
<td colspan="2">Command line <em><code>options</code></em> are:</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>-help</code></td>
<td>Prints the command line options.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-a </code><em><code>ratio</code></em></td>
<td><p>(GrADS 2.0.a9+) Specifies the aspect ratio of the real page inside GrADS. A valid <em><code>ratio</code></em> is the X size divided by the Y size and must be greater than 0.2 and less than 5.0. Page size is scaled so the longer side will always be 11 inches. On startup, GrADS will provide the page dimensions with the message that begins with "<code>GX Package Initialization: Size = </code>"</p></td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-b</code></td>
<td>Runs GrADS in batch mode. No graphics output window is opened.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-c </code><em><code>'command'</code></em></td>
<td>Executes the supplied <em><code>command</code></em> after GrADS has started.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-C N</code></td>
<td>(GrADS 2.0.a9+) Enables colorization of text displayed in the GrADS command window. N can be 0, 1, or 2. If -C is invoked but N is not provided, color scheme 0 will be used.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-d </code><em><code>gxdname</code></em></td>
<td>(GrADS 2.2.0+) Specifies the name of the <a href="plugins.html">graphics display plug-in</a> (default is "Cairo")</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-E</code></td>
<td>Disables command line editing</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-g </code><em><code>geometry</code></em></td>
<td><p>Specifies the size of the graphics output window, which is a representation on your computer screen of the real page and may be any size at all. The size of the real page in GrADS is controlled by the -l or -p or -a options. The <em><code>geometry</code></em> argument has the syntax <code>W</code>x<code>H</code>+<code>X</code>+<code>Y</code> , where <code>W</code> is the width of window in pixels, <code>H</code> is the height of window in pixels, <code>X</code> is the starting pixel point in x, and <code>Y</code> is the starting pixel point in y. Note that <code>X</code> and <code>Y</code> may be negative.</p></td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-h </code><em><code>gxpname</code></em></td>
<td>(GrADS 2.2.0+) Specifies the name of the <a href="plugins.html">graphics printing plug-in</a> (default is "Cairo")</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-H </code><em><code>filename</code></em></td>
<td>Enables command line logging to <em><code>filename</code></em>. If <em><code>filename</code></em> is not provided, command history written to file $HOME/.grads.log</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-l</code></td>
<td>Runs GrADS in landscape mode, sets the "real" page size to 11 x 8.5.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-m NNN</code></td>
<td>Sets metafile buffer size to NNN, which must be an integer. Default value is 1000000.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-p</code></td>
<td>Runs GrADS in portrait mode, sets the "real" page size to 8.5 x 11.<br />
If neither the <code>-l</code> or <code>-p</code> options are used, GrADS will prompt the user for a preferred mode.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>-u</code></td>
<td>Unbuffers output, needed for IPC mode</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="100"><code>-x</code></td>
<td>Generally used with the -c option, causes GrADS to automatically quit after the supplied <em><code>command</code></em> has been executed.</td>
</tr>
</tbody>
</table>

Options that do not require arguments may be concatenated. Some examples follow:
` grads -pb`\
`grads -lbxc "myscript.gs"`\
`grads -Ca 1.7778`\
`grads -C 2 -a 1.7778`\
`grads -pHm 5000000 -g 1100x850+70+0`\
`grads -pH mysession.log -m 5000000 -g 1100x850+70+0 `

 

# 
(env)=
Environment Variables

**Conda-forge installations:** activating the environment automatically sets `GADDIR`, `GAUDPT`, and `GAGPY` to locations within that environment and restores any previous values on deactivation. The manual settings below are intended for source builds and other custom installations. Stale values that name a different GrADS installation can cause data or graphics plug-ins from the wrong installation to be loaded.

<table class="plaintext" width="680" data-border="0" data-cellpadding="5" data-cellspacing="5">
<tbody>
<tr>
<td colspan="2">Some environment variables must be set before starting GrADS. </td>
</tr>
<tr data-bgcolor="c5d5ff">
<td width="55"><code>GADDIR</code>  </td>
<td width="590"> Points to the directory containing the supplemental font and map files in the GrADS release package. If GADDIR is not set, GrADS will look in the default location, /usr/local/lib/grads/. </td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>GASCRP</code> </td>
<td> Points to a list of directories containing GrADS utility scripts and user scripts. If more than one directory is specified, acceptable delimiters are a space, a semi-colon, colon, or a comma.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>GASHP</code></td>
<td>(GrADS version 2.0.0+) Points to a list of directories containing shapefiles. Put your shapefiles in those directories, and then it won't be necessary to use the full path when drawing or querying the shapefiles. If more than one directory is specified, acceptable delimiters are a space, a semi-colon, colon, or a comma.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>GAUDFT</code> </td>
<td>(GrADS version 1.9 or earlier) Points to the user defined function table. If this variable is not set, the function table will not be read.</td>
</tr>
<tr data-bgcolor="c5d5ff">
<td><code>GAUDPT</code> </td>
<td>(GrADS version 2.1.1+) This variable identifies the <a href="udpt.html">user defined plug-in table</a>, and should contain a single filename with its full path. GrADS will also look in the directory named by GADDIR for a file named "udpt". </td>
</tr>
</tbody>
</table>

For example:

*C-shell* 

`example% setenv GADDIR /ford1/local/lib/grads`\
`example% setenv GASHP $HOME/grads/shapefiles`\
`example% setenv GASCRP "$HOME/grads/scripts /opt/local/share/grads/library"`\
`example% setenv GAUDPT $HOME/grads/udpt`\

*Bourne shell*

`example% GADDIR=/ford1/local/lib/grads; export GADDIR`\
`example% GASCRP=$HOME/grads/scripts; export GASCRP`\
`example% GAUDPT=$HOME/grads/udpt; export GAUDPT`\
