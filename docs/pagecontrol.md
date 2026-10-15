---
title: Page Control in GrADS
---

# Page Control in GrADS

(real)=
## Real and virtual pages

### The real page

The **real page** is an 8.5 × 11 inch page in either landscape or portrait
orientation. GrADS asks for the orientation when you first start
[GrADS](gradcomdgrads.md).

The graphics output window represents the real page on your screen. It can be
any size. Set its dimensions explicitly in pixels with
[`set xsize`](gradcomdsetxsize.md), or resize the window with the mouse. When
the graphics window is printed, its pixel coordinates are scaled so the
printed graphics retain the same proportions on the real page.

### The virtual page

The **virtual page** is a page within the real page. By default, it has the
same dimensions as the real page, so real-page and virtual-page coordinates
are identical. All graphics are drawn on the virtual page.

Change the virtual-page limits with:

```text
set vpage xmin xmax ymin ymax
```

This is the syntax for [`set vpage`](gradcomdsetvpage.md). The four values are
specified in inches. After the command, GrADS reports the virtual-page size.

For example, to draw a plot in the lower-left quadrant of the real page:

```text
set vpage 0 5.5 0 4.25
```

GrADS reports:

```text
Virtual page size = 11 8.5
```

The virtual page has the same aspect ratio as the real page, but represents a
smaller 11 × 8.5 inch coordinate system in that quadrant. A centered square
virtual page can be defined with:

```text
set vpage 4 7 2.75 5.75
```

GrADS reports:

```text
Virtual page size = 8.5 8.5
```

The plot occupies a 3 inch square on the real page while using an 8.5 inch
coordinate system on the virtual page. Any graphics command that accepts
page coordinates in inches uses virtual-page coordinates.

Restore the default virtual page with:

```text
set vpage off
```

(plotarea)=
## Controlling the plot area

Use [`set parea`](gradcomdsetparea.md) to control the area within the virtual
page where GrADS draws contour plots, maps, and line graphs:

```text
set parea xmin xmax ymin ymax
```

The plot area does not include axis labels, titles, color bars, or other
annotations, so leave enough space for those elements. Its coordinates are
in virtual-page inches.

GrADS chooses a default plot area based on the graphics output type. Restore
that default with:

```text
set parea off
```

Line graphs and contour plots without a map are scaled to fill the plot area.
Plots containing a map projection are scaled to preserve the correct
latitude/longitude aspect ratio, so the map may not fill the entire area for
every geographic extent. To disable this behavior, use the `scaled` map
projection; see [`set mproj`](gradcomdsetmproj.md) for details.

(multipanel)=
## Drawing multi-panel plots

Use [`set vpage`](gradcomdsetvpage.md) to define several virtual pages within
the real page. Virtual pages may overlap. The example
[`panels_demo.gs`](_downloads/grads-scripts/panels_demo.gs) demonstrates how
to arrange a specified number of rows and columns. It uses the dynamically
loaded script function [`panels.gsf`](_downloads/grads-scripts/panels.gsf).

Labels and other graphic elements are positioned in virtual-page coordinates.
Those coordinates remain unchanged when switching between panels, making it
easy to align or offset annotations consistently.

Do not use [`set parea`](gradcomdsetparea.md) to place multiple plots on one
page. `parea` controls the plotting area; `vpage` is the appropriate command
for panel layouts.
