---
title: set poli
---

# **set poli**

`set poli `*`on|off`*

Selects whether political boundaries are drawn for map data sets that provide them, including `mres`, `hires`, `ne110m`, `ne50m`, and `ne10m`. The default is `on`.

For the Natural Earth maps, `set poli off` hides map types 1 (country boundaries) and 2 (state/province boundaries), leaving map type 0 (coastlines) visible. `set poli on` enables both political map types. Use <a href="gradcomdsetmpt.html">`set mpt`</a> when the two political types must be controlled separately.

Settings stay the same until changed by new `set` commands.

## Usage Notes

### Examples

```text

set mpdset ne10m
set poli off

```
