---
title: Using GrADS Station Data
---

# Using GrADS Station Data

This guide covers commands and functions for analyzing and displaying station
data. See <a href="aboutstationdata.html">About Station Data</a> for the
station-file format and descriptor-file requirements.

## At a glance

- [Operating on station data](#operating)
- [Plotting station models](#model)
- [Drawing arbitrary cross sections](#xsection)

(operating)=
## Operating on station data

Station-data operations and displays support three dimension environments:

| Varying dimensions | Typical result |
| --- | --- |
| X and Y | Horizontal station plot. |
| Z | Vertical profile. |
| T | Time series. |

Operations on station data work like operations on gridded data, but operations
between a grid and station data are not supported. Operations between two
station data sets are performed only for reports with exactly matching values
of all varying dimensions. Duplicate reports are ignored after the first
matching occurrence.

For example, with T as the only varying dimension:

~~~text
display ts-ds
~~~

This produces a time series from reports that have matching times.

### Station identifiers

When X and Y are fixed, a variable specification can use a station identifier
to override both latitude and longitude:

~~~text
varname(stid=ident)
~~~

Station identifiers are case-insensitive.

### Functions that do not support station data

| Function | Function | Function |
| --- | --- | --- |
| <a href="gradfunchdivg.html"><code>hdivg</code></a> | <a href="gradfunchcurl.html"><code>hcurl</code></a> | <a href="gradfuncvint.html"><code>vint</code></a> |
| <a href="gradfuncmaskout.html"><code>maskout</code></a> | <a href="gradfuncave.html"><code>ave</code></a> | <a href="gradfuncaave.html"><code>aave</code></a> |
| <a href="gradfunctloop.html"><code>tloop</code></a> |  |  |

### Displaying station values

When X and Y vary, station values are displayed as numbers centered at their
locations. With two expressions, for example <code>display ts;ds</code>, one
value appears above and one below each station.

| Command | Purpose |
| --- | --- |
| <a href="gradcomdsetccolor.html"><code>set ccolor color</code></a> | Set the station-value color. |
| <a href="gradcomdsetdignum.html"><code>set dignum digits</code></a> | Set the number of displayed digits. |
| <a href="gradcomdsetdigsize.html"><code>set digsiz size</code></a> | Set the displayed number size. |
| <a href="gradcomdsetstid.html"><code>set stid on/off</code></a> | Show or hide station identifiers. |

(model)=
## Plotting station models

Enable station-model output with:

~~~text
set gxout model
~~~

Display the model fields in this order:

~~~text
display u;v;t;d;slp;delta;cld;wx;vis
~~~

| Field | Meaning |
| --- | --- |
| <code>u</code>, <code>v</code> | Wind components. A wind barb is drawn; if either is missing, no model is plotted. |
| <code>t</code>, <code>d</code>, <code>slp</code>, <code>delta</code> | Numeric values plotted around the station model. |
| <code>cld</code> | Center symbol. Values 1–9 select marker types; values 20–25 select cloud-cover symbols. |
| <code>wx</code> | Weather symbol value, plotted using <a href="gradcomddrawwxsym.html"><code>draw wxsym</code></a>. |
| <code>vis</code> | Visibility, plotted as a whole number and fraction. |

Cloud-cover values are:

| Value | Meaning |
| --- | --- |
| 20 | Clear |
| 21 | Scattered |
| 22 | Broken |
| 23 | Overcast |
| 24 | Obscured |
| 25 | Missing (M is plotted) |

If a field other than <code>u</code> or <code>v</code> is missing, the model is
drawn without that element. Use a constant to represent a globally missing
field. For example, omit <code>delta</code> with:

~~~text
display u;v;t;d;slp;0.0;cld
~~~

Station models respond to the usual display settings:

<a href="gradcomdsetdigsize.html"><code>set digsiz</code></a>,
<a href="gradcomdsetdignum.html"><code>set dignum</code></a>,
<a href="gradcomdsetcthick.html"><code>set cthick</code></a>, and
<a href="gradcomdsetccolor.html"><code>set ccolor</code></a>.

<a href="gradcomdsetmdlopts.html"><code>set mdlopts</code></a> can display the
SLP value as a three-digit number. Enable <code>dig3</code> and display
<code>slp*10</code> to use the standard three-digit sea-level pressure format.

(xsection)=
## Drawing arbitrary cross sections

An arbitrary vertical cross section is created by converting scattered station
profiles into a grid so that GrADS grid display and analysis functions can be
used.

### Workflow

1. Create a collection of one-dimensional profiles (Z or T varying) with
   <a href="gradcomdcollect.html"><code>collect</code></a>. Profiles may be
   station data or gridded data converted with
   <a href="gradfuncgr2stn.html"><code>gr2stn</code></a>.
2. Convert the collection to an X/Z grid with
   <a href="gradfunccoll2gr.html"><code>coll2gr</code></a>.
3. Display or analyze the resulting grid.

<code>coll2gr</code> currently supports vertical profiles, not time slices. The
input dimension environment must be X and Z varying. The output X axis contains
equally spaced profiles over the current X range; its Z axis spans the current
Z range and uses either the requested number of levels or the union of all
levels. Points outside the requested level range may be used for interpolation.

The output X axis is artificial and does not represent longitude. Label it with:

~~~text
set xlabs lab1 | lab2 | lab3 ...
set ylabs lab1 | lab2 | lab3 ...
~~~

Labels are equally spaced along the axes. Add blank labels to create spacing:

~~~text
set xlabs | | | | lab1 | ...
~~~

### Example script

The following script collects a cross section between two longitude/latitude
points and displays the result:

~~~text
'open pv.ctl'
'set grads off'
'set zlog on'
'set x 1'
'set y 1'
'set lev 1000 100'

lon1 = -95.0
lon2 = -90.0
lat1 = 55.0
lat2 = 15.0
lon = lon1

'collect 1 free'
while (lon <= lon2)
  lat = lat1 + (lat2-lat1)*(lon-lon1) / (lon2-lon1)
  'collect 1 gr2stn(pv,'lon','lat')'
  lon = lon + 1
endwhile

'set x 14 16'
'set xaxis 'lon1' 'lon2
'set clab on'
'set gxout shaded'
'set clevs 0 .5 15'
'set ccols 0 0 7 0'
'd coll2gr(1,-u)'

'set gxout contour'
'set cint .5'
'd coll2gr(1,-u)'
~~~
