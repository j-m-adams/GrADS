---
title: Bar graphs
---

# Bar graphs

A bar chart for one-dimensional data. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
set x 1 20
set y 1
set gxout bar
display lon
printim bar.png png white
```

## Relevant controls

Use [`set barbase`](gradcomdsetbarbase.md), [`set bargap`](gradcomdsetbargap.md), [`set baropts`](gradcomdsetbaropts.md), [`set ccolor`](gradcomdsetccolor.md), and [`set vrange`](gradcomdsetvrange.md) to control the baseline, spacing, appearance, and value range.

![Bar graphs](_static/plots/bar.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
