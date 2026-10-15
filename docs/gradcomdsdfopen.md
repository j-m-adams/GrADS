---
title: sdfopen
---

# **sdfopen**

`sdfopen `*`filename <template #timesteps>`*

Opens a NetCDF or HDF-SDS format file that conforms to the [COARDS conventions](http://ferret.wrc.noaa.gov/noaa_coop/coop_cdf_profile.html). The sdfopen command does not support the HDF5 format, but is does support netcdf4. The arguments and options are as follows:

- *`filename`* 
  -  The name of the COARDS-compliant NetCDF or HDF-SDS file. 

  *`template`* 
  -  This optional argument is used when you want to aggregate multiple data files and handle them as if they were one individual file. The individual data files must be identical in all dimensions except time. *`template`* has a similar structure to the substitution template in a GrADS data descriptor file. See <a href="templates.html">Using Templates</a> for details. 

  *`#timesteps`* 
  -  This argument must be included whenever *`template`* is invoked. The *`#timesteps`* is the sum of the timestep counts in all the files to be examined, not the count in any one file. 

## Usage Notes

1.  The template option with sdfopen was removed in version 2.0. If you want to aggregate multiple files together use the <a href="gradcomdxdfopen.html">xdfopen</a> command with the 'options template" keyword and a complete TDEF entry.
2.  Here's a brief summary of the metadata that sdfopen is looking for when it tries to open a self-describing file. As it goes through the list of dimension variables in the file, it checks each one for attributes named "units", "axis", and "grads_dim". Acceptable values for these attributes and the GrADS dimension the coordinate variable maps to are outlined in this table:\
    \
    <table width="763" data-border="0">
    <colgroup>
    <col style="width: 50%" />
    <col style="width: 50%" />
    </colgroup>
    <tbody>
    <tr>
    <td width="150" data-bgcolor="cccccc"><div data-align="center">
    GrADS Dimension
    </div></td>
    <td width="603" data-bgcolor="cccccc"><div data-align="center">
    Acceptable Attribute Values
    </div></td>
    </tr>
    <tr>
    <td data-bgcolor="b8c8d7"><div data-align="center">
    X
    </div></td>
    <td data-bgcolor="b8c8d7">units: degrees_east, degree_east, degrees_E, degree_E<br />
    axis: x, X</td>
    </tr>
    <tr>
    <td data-bgcolor="ccdceb"><div data-align="center">
    Y
    </div></td>
    <td data-bgcolor="ccdceb">unit: degrees_north, degree_north, degrees_N, degree_N<br />
    axis: y, Y</td>
    </tr>
    <tr>
    <td data-bgcolor="b8c8d7"><div data-align="center">
    Z (pressure)
    </div></td>
    <td data-bgcolor="b8c8d7">units: mb, millibar, hybrid_sigma_pressure<br />
    </td>
    </tr>
    <tr>
    <td data-bgcolor="ccdceb"><div data-align="center">
    Z (not pressure)
    </div></td>
    <td data-bgcolor="ccdceb">units: sigma_level, degreesk, degrees_k, level, layer, layers<br />
    axis: z, Z</td>
    </tr>
    <tr>
    <td data-bgcolor="b8c8d7"><div data-align="center">
    T
    </div></td>
    <td data-bgcolor="b8c8d7">units: yyyymmddhhmmss, yymmddhh, or a Udunits-acceptable time unit<br />
    axis: t, T</td>
    </tr>
    <tr>
    <td data-bgcolor="ccdceb"><div data-align="center">
    E
    </div></td>
    <td data-bgcolor="ccdceb">grads_dim: e<br />
    axis: e, E</td>
    </tr>
    </tbody>
    </table>

    \
3.  If the sdfopen command fails to open your self-describing file, try using the <a href="gradcomdxdfopen.html">`xdfopen`</a> command which requires a special descriptor file to supplement or override the metadata in the file, or use the <a href="gradcomdopen.html">open</a> command with a <a href="SDFdescriptorfile.html">complete descriptor file</a>.

### Examples

1.  If you had daily U-Wind data in two files, `uwnd.1989.nc` and `uwnd.1990.nc`, you could access them both as one GrADS data set by entering:

    `sdfopen /data/uwnd.1989.nc uwnd.%y4.nc 730`
