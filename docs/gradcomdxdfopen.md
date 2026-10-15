---
title: xdfopen
---

# **xdfopen**

`xdfopen `*`filename`*

GrADS requires a certain amount of metadata in order to understand howto read a NetCDF/HDF-SDS data file, also called a self-describing file(SDF). The <a href="gradcomdsdfopen.html">`sdfopen`</a>command assumes all the metadata is internal to the self-describingfile, whereas the `xdfopen` command allows the user to supplementor replace any internal metadata via a data descriptor file. In thisway, `xdfopen` provides access to some self-describing filesthat do not comply with the [COARDS conventions](http://ferret.wrc.noaa.gov/noaa_coop/coop_cdf_profile.html).

*`filename`* is the name of the data descriptor file thatcontains the supplemental metadata. It has a syntax very similar tothe <a href="descriptorfile.html">regular data descriptor files</a> that are used with the <a href="gradcomdopen.html">`open`</a> command. The few differences are noted below:

1.  <a href="descriptorfile.html#DSET">`DSET`</a>` `*`SDF_filename`*

    This is the only required entry. *`SDF_filename`* may be either the name of a netCDF or HDF-SDS file or a substitution template for a collection ofnetCDF or HDF-SDS files.

    Other than <a href="descriptorfile.html#DSET">`DSET`</a>,the only other data descriptor file entries that are supported are <a href="descriptorfile.html#UNDEF">`UNDEF`</a>,<a href="descriptorfile.html#TITLE">`TITLE`</a>,<a href="descriptorfile.html#XDEF">`XDEF`</a>,<a href="descriptorfile.html#YDEF">`YDEF`</a>,<a href="descriptorfile.html#ZDEF">`ZDEF`</a>,<a href="descriptorfile.html#TDEF">`TDEF`</a>, <a href="descriptorfile.html#EDEF">`EDEF`</a>, <a href="descriptorfile.html#OPTIONS">`OPTIONS`</a>, <a href="descriptorfile.html#VARS">`VARS`</a>, and <a href="descriptorfile.html#ENDVARS">`ENDVARS`</a>. Valid arguments for the <a href="descriptorfile.html#OPTIONS">`OPTIONS`</a> entry are: `yrev, zrev, template,` and `365_day_calendar`.

2.  <a href="descriptorfile.html#XDEF">`XDEF`</a>,<a href="descriptorfile.html#YDEF">`YDEF`</a>,<a href="descriptorfile.html#ZDEF">`ZDEF`</a>, <a href="descriptorfile.html#TDEF">`TDEF`</a>, and<a href="descriptorfile.html#EDEF">`EDEF`</a>:

    Each of these entries requires an additional argument,*`SDF_dimension_name`*, which comes before all theother arguments. The *`SDF_dimension_name`* is used toachieve dimension order independence, so it must be a real dimensionin the SDF. The *`SDF_dimension_name`* string may bemixed case and should appear exactly as it is listed in the outputfrom ncdump.

    If the coordinate variables in the SDF file exist and have therequired metadata, then *`SDF_dimension_name`* is theonly argument needed for the corresonding axis definition entry(`XDEF, YDEF, ZDEF,` `TDEF, and EDEF`) in the datadescriptor file. If you need to supplement or override the coordinate metadata in the SDF file, you can fill out the axis definition entries in the descriptor file with the remaining arguments describing the size, linearity, start, and increment. For EDEF, there is support for three variations on the compact syntax of the EDEF entry:\
    `edef <`*`SDF_dimension_name`*`>`\
    `edef <`*`SDF_dimension_name`*`> <`*`size`*`>`\
    `edef <`*`SDF_dimension_name`*`> <`*`size`*`> names <`*`list of names`*`> `

3.  The first argument ("*`varname`*") of the variabledefinition lines that appear between `VARS` and `ENDVARS` has the following syntax:\
    *`SDF_varname`*`=>`*`grads_varname`*
    *`SDF_varname`* is the name of the variable as it appearsin the output from the NetCDF utility ncdump. It may be of mixed case. If it includes blanks, substitute "~" for the blanks. If everything up to and including the `"=>"` is omitted,then *`grads_varname`* must be identical to*`SDF_varname`*. This syntax (when"*`SDF_varname`*`=>`" is omitted) will only workproperly in GrADS if *`SDF_varname`* is less than 15characters and does not contain any upper case letters.As it was with the coordinate variables, if the data variables in theSDF file have the required metadata, then*`SDF_varname`*`=>`*`grads_varname`* is the onlyargument needed for the corresonding variable definition entry in thedata descriptor file.

4.  The order of the variable definition lines between VARSand ENDVARS is not important.

## Usage Notes

1.  If *`filename`* contains only the DSET entry, then `xdfopen` devolves into working just like <a href="gradcomdsdfopen.html">`sdfopen`</a>.
2.  *`filename`* does not need to be a full datadescriptor file, it only needs to contain whatever metadata the SDFfile lacks. Anything not specified in *`filename`*will be looked for in the file's internal metadata.
3.  The *`SDF_dimension_name`* parameter in the XDEF,YDEF, ZDEF, TDEF, and EDEF entries and the first parameter of the VARIABLE definition lines are the only parts of the data descriptor file that aren't converted to lower case before they are interpreted.
4.  For further information on the COARDS conventions, check out Conventionsfor the standardization of NetCDF files.
5.  (GrADS version 2.0.a7.1+) The <a href="descriptorfile.html#CHSUB">CHSUB</a> entry will work with xdfopen.
### Examples
This example shows the data descriptor file that would be requiredin order to open a self-describing file that is missing much of therequired metadata. Below is the sample data descriptor file for theNetCDF file moisture.nc. Follow [this link](_downloads/xdfsample1.txt)to see output from ncdump for this file.
```text
DSET ^moisture.nc
TITLE This is a sample 
UNDEF 99999.0
XDEF dimension1 144 LINEAR 0.0 2.5
YDEF dimension2  73 LINEAR 0.0 2.5
TDEF dimension3 365 LINEAR 0Z01JAN1979 1DY
VARS 1
Moisture=>moisture 1 99 Moisture
ENDVARS

```

This second example comes from a real-world HDF-SDS file from the DataAssimilation Office at NASA Goddard Space Flight Center. The datadescriptor file is shown below, and [thislink](_downloads/xdfsample2.txt) shows the output from running the HDF version of ncdump on`DAOE054A.hdf`. (Note that the output has been annotated withexplanatory comments -- they are preceded with "//")

```text

DSET ^DAOE054A.hdf
TITLE This is only a test
OPTIONS YREV
UNDEF 1.0E15
XDEF XDim:DAOgrid 144 LINEAR -180.0 2.5
YDEF YDim:DAOgrid  91 LINEAR  -90.0 2.0
ZDEF HGHT18DIMS:DAOgrid 18 LEVELS 1000 850 700 500 400 300 250 200 150 100 70 50 30 10 5 2 1 0.4
TDEF TIME4DIMS:DAOgrid 4 LINEAR 0Z31JUL1993 6HR
VARS 3
GEOPOTENTIAL_HEIGHT=>hgt 18 99 geopotential height
SPECIFICHUMIDITY=>shum 18 99 specific humidity
TEMPERATURE=>temp 18 99 temperature
ENDVARS

```
