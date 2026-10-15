---
title: Drawing Data Plots
---

# Drawing Data Plots

- [Drawing data plots](#ddp)
- [Clearing the display](#ctd)
- [Graphics output types](#got)
- <a href="advdisplay.html">Advanced display options</a>

(ddp)=
## Displaying data

The <a href="gradcomddisplay.html"><code>display</code></a> command evaluates an
expression and sends the result to the graphics output window:

~~~text
display expression
d expression
~~~

The simplest expression is a variable abbreviation.

| Varying dimensions | Default result |
| --- | --- |
| None | A single value is printed as text. |
| One | A one-dimensional line graph. |
| Two | A two-dimensional contour plot. |
| Three or more | A sequence of two-dimensional slices. |

Other plot types are available through <code>set gxout</code>.

(ctd)=
## Clearing the display

GrADS overlays output from successive display commands. Clear the graphics
window with <a href="gradcomdclear.html"><code>clear</code></a>, or its shortcut
<code>c</code>:

~~~text
clear
c
~~~

Without a qualifier, <code>clear</code> also resets many internal settings.
Qualifiers limit the scope:

| Command | Effect |
| --- | --- |
| <code>c events</code> | Flush the events buffer, such as mouse clicks. |
| <code>c graphics</code> | Clear graphics but keep widgets. |
| <code>c hbuff</code> | Clear the display buffer in double-buffer mode. |

If the syntax of a <code>clear</code> command is invalid, GrADS performs the
full clear.

(got)=
## Graphics output types

Set the plot type with:

~~~text
set gxout graphics_type
~~~

By default, one varying dimension produces a line graph and two varying
dimensions produce a contour plot. Common graphics types include:

| Type | Typical output |
| --- | --- |
| <code>contour</code> | Contour lines. |
| <code>shaded</code> | Filled contours. |
| <code>grid</code> | Grid-point values. |
| <code>bar</code> | Bar chart. |
| <code>vector</code> | Wind vectors. |
| <code>stream</code> or <code>streamline</code> | Streamlines. |
| <code>barb</code> | Wind barbs. |
| <code>wxsym</code> | Weather symbols at station locations. |

See the <a href="gradcomdsetgxout.html"><code>set gxout</code> reference</a> for
the complete list and additional options.

### Vector, streamline, and barb plots

These plot types require two result grids. The first expression supplies the U
component and the second supplies the V component, separated by a semicolon:

~~~text
display u ; v
display ave(u,t=1,t=10) ; ave(v,t=1,t=10)
~~~

For <code>vector</code> and <code>stream</code>, a third expression can colorize
the vectors or streamlines:

~~~text
display u ; v ; mag(u,v)
display u ; v ; hcurl(u,v)
~~~

### Weather symbols

For <code>wxsym</code>, each station value is interpreted as a weather-symbol
code. Run the <code>wxsym.gs</code> sample script to see the available codes
and their corresponding symbols.

