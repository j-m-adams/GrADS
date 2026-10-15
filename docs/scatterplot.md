---
title: Scatter plots
---

# Scatter plots

Scatter plots show one grid expression against another. The first expression
provides the X coordinate and the second provides the Y coordinate. A third
expression can optionally color the points (GrADS 2.0.2 and later).

## Syntax

```text
set gxout scatter
display exp1;exp2[;exp3]
```

| Expression | Description |
| --- | --- |
| `exp1` | Grid expression used for the X coordinate. |
| `exp2` | Grid expression used for the Y coordinate. |
| `exp3` | Optional grid expression used to color the points. |

## Controls

When colorizing points, use [`set clevs`](gradcomdsetclevs.md) and
[`set ccols`](gradcomdsetccols.md) to define the value ranges and colors. A
negative color number suppresses points in that range.

After drawing a scatter plot, GrADS reports the minimum and maximum X values
followed by the minimum and maximum Y values. Override these automatic ranges
with [`set vrange`](gradcomdsetvrange.md) and
[`set vrange2`](gradcomdsetvrange2.md), respectively.

The example below uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
define field = sin(lon/20) + cos(lat/20)
set x 1 20
set y 1
set gxout scatter
display lon;field
printim scatter.png png white
```

![Scatter plots](_static/plots/scatter.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
