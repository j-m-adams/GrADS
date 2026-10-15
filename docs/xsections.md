---
title: Drawing Arbitrary Cross Sections
---

# Drawing Arbitrary Cross Sections

An arbitrary vertical cross section is created by converting scattered station
profiles into a regular grid. The resulting grid can then use the normal GrADS
display and analysis functions.

## Workflow

| Step | Operation | Tool |
| --- | --- | --- |
| 1 | Create a collection of one-dimensional profiles with Z or T varying. | <a href="gradcomdcollect.html"><code>collect</code></a> |
| 2 | Convert the station collection into an X/Z grid. | <a href="gradfunccoll2gr.html"><code>coll2gr</code></a> |
| 3 | Display or analyze the resulting grid. | Normal GrADS grid commands |

Profiles may be real station data or gridded data converted to station data with
<a href="gradfuncgr2stn.html"><code>gr2stn</code></a>.

## The coll2gr output

<a href="gradfunccoll2gr.html"><code>coll2gr</code></a> currently supports
collections of vertical profiles, not time slices. The dimension environment
must be X and Z varying when it is called.

| Output axis | Description |
| --- | --- |
| X | Artificial, equally spaced station-profile positions spanning the current X range. It does not represent longitude. |
| Z | Spans the current Z range and contains either the requested number of levels or the union of profile levels. |

Data points outside the requested level range may be used when interpolating
within the range.

## Labeling the cross section

Because the output X axis is artificial, use explicit labels to show the
original positions:

~~~text
set xlabs lab1 | lab2 | lab3 ...
set ylabs lab1 | lab2 | lab3 ...
~~~

Labels are placed at equal intervals. Insert blank labels to create spacing:

~~~text
set xlabs | | | | lab1 | ...
~~~

See the <a href="gradcomdsetxlabs.html"><code>set xlabs</code></a> and
<a href="gradcomdsetylabs.html"><code>set ylabs</code></a> command references
for syntax details.

## Example script

This script samples a PV field along a line between two geographic points,
collects the profiles, and displays the resulting cross section:

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

