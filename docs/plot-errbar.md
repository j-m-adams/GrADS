---
title: Error bars
---

# Error bars

Bars with error ranges. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
set x 1 20
set y 1
set gxout errbar
display lon;lon+5
printim errbar.png png white
```

## Relevant controls

Use [`set ccolor`](gradcomdsetccolor.md), [`set cthick`](gradcomdsetcthick.md), [`set vrange`](gradcomdsetvrange.md), and [`set vrange2`](gradcomdsetvrange2.md) to control error-bar appearance and the two value axes.

![Error bars](_static/plots/errbar.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
