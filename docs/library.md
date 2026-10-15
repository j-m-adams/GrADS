---
title: GrADS Script Library
---

# GrADS Script Library

The scripts below are supplementary examples and utilities for GrADS. Download
the scripts you need and place them in a directory on your GrADS script path.

| Script | Description |
| --- | --- |
| [basemap.gs](_downloads/grads-scripts/basemap.gs) | Overlays a land or ocean mask that fits the coastal outlines. It requires [`lpoly_lowres.asc`](_downloads/grads-scripts/lpoly_lowres.asc), [`lpoly_mres.asc`](_downloads/grads-scripts/lpoly_mres.asc), [`lpoly_hires.asc`](_downloads/grads-scripts/lpoly_hires.asc), [`opoly_lowres.asc`](_downloads/grads-scripts/opoly_lowres.asc), [`opoly_mres.asc`](_downloads/grads-scripts/opoly_mres.asc), and [`opoly_hires.asc`](_downloads/grads-scripts/opoly_hires.asc). Use [`lpoly_US.asc`](_downloads/grads-scripts/lpoly_US.asc) to mask out non-US areas. |
| [box_and_whisker.gs](_downloads/grads-scripts/box_and_whisker.gs) | Demonstrates `gxout bar` and `gxout errbar` for a box-and-whisker plot. |
| [cbar.gs](_downloads/grads-scripts/cbar.gs)<br>[cbarn.gs](_downloads/grads-scripts/cbarn.gs)<br>[cbarm.gs](_downloads/grads-scripts/cbarm.gs) | Draws a rectangular colour legend next to shaded plots. `cbar.gs` draws filled rectangles with labels; `cbarn.gs` adds outlines and triangular endpoints; `cbarm.gs` is suited to plots using 30 or more colours. |
| [cbarc.gs](_downloads/grads-scripts/cbarc.gs) | Draws a small fan-shaped colour legend in the corner of a shaded plot. |
| [cbar_l.gs](_downloads/grads-scripts/cbar_l.gs)<br>[cbar_line.gs](_downloads/grads-scripts/cbar_line.gs)<br>[cbar_line2.gs](_downloads/grads-scripts/cbar_line2.gs) | Draws legends for line graphs. |
| [cmap.gs](_downloads/grads-scripts/cmap.gs) | Creates a colour table. See the [colour control documentation](colorcontrol.md). |
| [connect_the_dots.gs](_downloads/grads-scripts/connect_the_dots.gs) | Draws a line connecting the user's mouse clicks. |
| [define_colors.gs](_downloads/grads-scripts/define_colors.gs) | Defines colours using the [`set rgb`](gradcomdsetrgb.md) command. |
| [defval_demo.gs](_downloads/grads-scripts/defval_demo.gs) | Demonstrates the [`q defval`](gradcomdquery.md) and [`set defval`](gradcomdsetdefval.md) commands. |
| [draw_pdsi.gs](_downloads/grads-scripts/draw_pdsi.gs) | Demonstrates the shapefile interface by drawing Palmer Drought Severity Index values for US climate divisions. |
| [font.gs](_downloads/grads-scripts/font.gs) | Displays all characters in a font set. |
| [isen.gs](_downloads/grads-scripts/isen.gs) | Displays a field interpolated to a specified isentropic level. |
| [lats4d.gs](_downloads/grads-scripts/lats4d.gs) | Writes NetCDF, HDF-SDS, or GRIB files from GrADS. See the additional [lats4d documentation](http://dao.gsfc.nasa.gov/software/grads/lats4d/). |
| [makebg.gs](_downloads/grads-scripts/makebg.gs) | Creates a background map image with topographic texture. It requires a DODS-enabled GrADS build and the ImageMagick `combine` utility. |
| [map.gs](_downloads/grads-scripts/map.gs) | Automates settings for useful map projections. |
| [mconv.gs](_downloads/grads-scripts/mconv.gs) | Calculates moisture convergence. |
| [meteogram_subset_GDS.gs](_downloads/grads-scripts/meteogram_subset_GDS.gs) | Downloads data from the GrADS Data Server for the 10-day meteogram example. The companion `meteogram_GDS.gs` script is no longer included. |
| [narropen.gs](_downloads/grads-scripts/narropen.gs) | Uses command-line arguments to build a descriptor file for pre-projected (Lambert conformal) NARR NetCDF data. |
| [panels.gsf](_downloads/grads-scripts/panels.gsf)<br>[panels_demo.gs](_downloads/grads-scripts/panels_demo.gs) | Creates global variables containing `set vpage` commands for multi-panel plots and demonstrates [dynamic script-function loading](gsf.md). |
| [script_math_demo.gs](_downloads/grads-scripts/script_math_demo.gs) | Demonstrates mathematical functions in the GrADS scripting language. |
| [pinterp.gs](_downloads/grads-scripts/pinterp.gs) | Displays a field interpolated to a specified pressure level. |
| [plotskew.gs](_downloads/grads-scripts/plotskew.gs) | Draws a Skew-T/Log-P diagram. |
| [rgb255.gs](_downloads/grads-scripts/rgb255.gs) | Defines rainbow colours and draws a demonstration plot. It uses [`cbarm.gs`](_downloads/grads-scripts/cbarm.gs) to draw the colour bar. |
| [sdfopen365.gs](_downloads/grads-scripts/sdfopen365.gs) | Creates an `xdfopen`-style descriptor for files using a 365-day calendar. |
| [sdfopent.gs](_downloads/grads-scripts/sdfopent.gs) | Simulates the old three-argument `sdfopen` command for templating. |
| [stack.gs](_downloads/grads-scripts/stack.gs) | Delays display while a command sequence is entered, then executes it all at once. |
| [string.gs](_downloads/grads-scripts/string.gs) | Draws a string at the position of the user's mouse click. |
| [sweat_index.gs](_downloads/grads-scripts/sweat_index.gs) | Calculates the SWEAT index from relative humidity, temperature, and wind components. |
| [traj.gs](_downloads/grads-scripts/traj.gs) | Draws forward and backward trajectories in the horizontal plane. |
| [use.gs](_downloads/grads-scripts/use.gs) | Similar to [`open`](gradcomdopen.md), but reuses an already opened file instead of opening it again. |
| [wxsym.gs](_downloads/grads-scripts/wxsym.gs) | Displays available weather symbols. |
| [xanim.gs](_downloads/grads-scripts/xanim.gs) | Controls an animated display. |
| [zinterp.gs](_downloads/grads-scripts/zinterp.gs) | Displays a field interpolated to a specified height level. |
| [zoom.gs](_downloads/grads-scripts/zoom.gs) | Provides a simple way to zoom into a plot. |
