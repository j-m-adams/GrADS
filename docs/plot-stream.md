---
title: Streamlines
---

# Streamlines

Streamlines from paired U and V fields. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_plot_fixture.ctl
set x 1 20
set y 1 15
set gxout stream
display u;v
printim stream.png png white
```

## Relevant controls

Use [`set strmden`](gradcomdsetstrmden.md), [`set strmopts`](gradcomdsetstrmopts.md), [`set ccolor`](gradcomdsetccolor.md), and [`set cthick`](gradcomdsetcthick.md) to control streamline density, arrows, color, and width.

![Streamlines](_static/plots/stream.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), the [descriptor](_downloads/grads-plot-fixtures/test_plot_fixture.ctl), and the [binary fixture](_downloads/grads-plot-fixtures/test_plot_fixture.bin).
