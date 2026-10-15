---
title: Line graphs
---

# Line graphs

A one-dimensional line graph. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
set x 1 20
set y 1
set gxout line
display lon
printim line.png png white
```

## Relevant controls

Use [`set ccolor`](gradcomdsetccolor.md), [`set cstyle`](gradcomdsetcstyle.md), [`set cthick`](gradcomdsetcthick.md), `set cmark`, [`set vrange`](gradcomdsetvrange.md), `set xaxis`, and `set yaxis` to control line appearance and axes.

![Line graphs](_static/plots/line.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
