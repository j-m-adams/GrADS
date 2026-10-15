---
title: Grid-fill plots
---

# Grid-fill plots

Filled grid cells using `grfill`. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
set x 1 20
set y 1 15
set gxout grfill
display field
printim grfill.png png white
```

## Relevant controls

Use [`set ccols`](gradcomdsetccols.md), [`set rbcols`](gradcomdsetrbcols.md), [`set gridln`](gradcomdsetgridln.md), [`set missconn`](gradcomdsetmissconn.md), and [`set black`](gradcomdsetblack.md) to control cell colors, grid lines, missing data, and map background.

![Grid-fill plots](_static/plots/grfill.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
