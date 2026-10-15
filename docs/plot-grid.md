---
title: Grid-value plots
---

# Grid-value plots

Numeric values displayed at grid points. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
set x 1 8
set y 1 6
set gxout grid
display field
printim grid.png png white
```

## Relevant controls

Use [`set dignum`](gradcomdsetdignum.md), [`set digsiz`](gradcomdsetdigsiz.md), [`set ccolor`](gradcomdsetccolor.md), and [`set gridln`](gradcomdsetgridln.md) to control printed precision, text size, color, and grid lines.

![Grid-value plots](_static/plots/grid.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
