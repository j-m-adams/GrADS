---
title: Specific-value grid fill
---

# Specific-value grid fill

Grid fill with explicit value-to-colour mappings. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
define field = sin(lon/20) + cos(lat/20)
set fgvals 0 2 1 3 2 4 3 5 4 6
set x 1 20
set y 1 15
set gxout fgrid
display field
printim fgrid.png png white
```

## Relevant controls

Use [`set fgvals`](gradcomdsetfgvals.md) for explicit value/color pairs; use [`set ccols`](gradcomdsetccols.md), [`set clevs`](gradcomdsetclevs.md), and [`set rbcols`](gradcomdsetrbcols.md) for palette and range control.

![Specific-value grid fill](_static/plots/fgrid.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
