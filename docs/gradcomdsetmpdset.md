---
title: set mpdset
---

# **set mpdset**

`set mpdset <`*`dataset`*`> [`*`dataset`*` ...]`

Selects one or more map background data sets. Up to eight data set names may be given; when more than one is specified, GrADS draws them in the order listed.

`lowres` is the legacy default and remains selected unless this command is issued. The other legacy data sets are `mres`, `hires`, and `nmap`; `mres` and `hires` have state and country outlines, while `nmap` covers only North America.

The Natural Earth data sets included with GrADS are:

- `ne110m` — 1:110 million, suited to global maps
- `ne50m` — 1:50 million, suited to continental maps
- `ne10m` — 1:10 million, suited to regional maps

All three contain coastlines, country boundaries, and first-order state/province boundaries. See the <a href="naturalearthmaps.html">Natural Earth map data set notes</a> for provenance, contents, and examples.

Settings stay the same until changed by new `set` commands.

## Usage Notes

The named files are normally found in the GrADS data directory. A full path may also be used. Map types can be selected and styled with <a href="gradcomdsetmpt.html">`set mpt`</a>; political boundaries can be toggled with <a href="gradcomdsetpoli.html">`set poli`</a>.

### Examples

```text

set mpdset ne110m
set mpdset ne50m
set mpdset ne10m
set mpdset ne110m ne50m

```
