---
title: Line-fill graphs
---

# Line-fill graphs

A filled region between two one-dimensional lines. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
set x 1 20
set y 1
set gxout linefill
display lon;lon+5
printim linefill.png png white
```

## Relevant controls

Use [`set ccols`](gradcomdsetccols.md), [`set ccolor`](gradcomdsetccolor.md), [`set cstyle`](gradcomdsetcstyle.md), [`set cthick`](gradcomdsetcthick.md), [`set vrange`](gradcomdsetvrange.md), and [`set vrange2`](gradcomdsetvrange2.md) to control fill and line appearance.

![Line-fill graphs](_static/plots/linefill.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
