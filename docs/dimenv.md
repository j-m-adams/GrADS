---
title: Dimension Environment
---

# Dimension Environment

GrADS views every data set as a generalized five-dimensional array in physical
space:

```text
longitude, latitude, level, time, ensemble
```

Variables add a sixth conceptual dimension. A data set may contain only a
subset of these dimensions, but GrADS uses the same model for evaluating
expressions and producing displays.

The **dimension environment** specifies which part of the data set is being
used. Expressions are evaluated relative to it, and the display geometry is
determined by it.

## Selecting dimensions

Use world or grid coordinates to set each dimension:

| Coordinate type | Command form | Dimensions | Meaning |
| --- | --- | --- | --- |
| World coordinates | `set lon\|lat\|lev\|time\|ens val1 [val2]` | `lon`, `lat`, `lev`, `time`, `ens` | Select physical coordinate values. See the <a href="gradcomdsetlatlonlevtimeens.html"><code>set lon/lat/lev/time/ens</code> reference</a>. |
| Grid coordinates | `set x\|y\|z\|t\|e val1 [val2]` | `x`, `y`, `z`, `t`, `e` | Select array indices. See the <a href="gradcomdsetxyzte.html"><code>set x/y/z/t/e</code> reference</a>. |

For example, `set lon` and `set x` both select the X dimension; the
difference is whether the value is supplied in world or grid units.

## Fixed and varying dimensions

A dimension is **fixed** when one value is supplied and **varying** when two
values are supplied:

| Input | State | Example | Result |
| --- | --- | --- | --- |
| One value | Fixed | `set lev 500` | Select one level. |
| Two values | Varying | `set lon -180 0` | Select a longitude range. |

The combination of fixed and varying dimensions determines the kind of object
being addressed:

| Varying dimensions | GrADS interpretation |
| --- | --- |
| None | A single data point. |
| One | A one-dimensional slice. |
| Two | A two-dimensional slice. |
| Three or more | A sequence of two-dimensional slices. |

## Example

The following commands select longitude from 180°W to 0°, latitude from the
equator to 90°N, level 500 mb, and the first time step:

```text
set lon -180 0
set lat 0 90
set lev 500
set t 1
```

The first three dimensions use world coordinates; `set t 1` uses a grid
coordinate and fixes time at the first index.

## Grid-to-world scaling

When a dimension environment is specified with grid coordinates, GrADS converts
those coordinates to world coordinates. This requires a grid-to-world scaling
defined by the descriptor file.

| Situation | Scaling used |
| --- | --- |
| Normal dimension selection | Scaling of the default file, usually the first file opened. |
| Grid coordinate inside a variable specification | Scaling of the file named in that variable specification. |

The second case is the exception to the default-file rule. It is relevant when a
variable specification contains a local dimension expression; see the
<a href="variable.html">variable specification</a> documentation for details.
