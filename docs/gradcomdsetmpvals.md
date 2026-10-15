---
title: set mpvals
---

# **set mpvals**

`set mpvals `*`lonmin lonmax latmin latmax`*` `

Sets reference longitudes and latitudes for polar stereographic plots. By default, these are set to the current dimension environment limits. This command overrides that, and allows the data-reference to be decoupled with the map display. The polar plot will be drawn such that the region bounded by these longitudes and latitudes will be entirely included in the plot.

GrADS will plot lat/lon lines on polar plots with no labels as yet. To turn this off, `set grid off`.

## Usage Notes

### Examples
