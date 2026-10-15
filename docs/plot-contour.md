---
title: Contour plots
---

# Contour plots

Line contours for a two-dimensional field. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
set x 1 20
set y 1 15
set gxout contour
display field
printim contour.png png white
```

## Relevant controls

Use [`set clevs`](gradcomdsetclevs.md), [`set cint`](gradcomdsetcint.md), [`set ccols`](gradcomdsetccols.md), [`set clab`](gradcomdsetclab.md), `set clskip`, [`set ccolor`](gradcomdsetccolor.md), and [`set cthick`](gradcomdsetcthick.md) to control levels, labels, colors, and contour lines.

![Contour plots](_static/plots/contour.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
