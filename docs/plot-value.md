---
title: Station values
---

# Station values

Values displayed at station locations. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_station_fixture.ctl
set gxout value
display t
printim value.png png white
```

## Relevant controls

Use [`set lon`](gradcomdsetlon.md), [`set lat`](gradcomdsetlat.md), [`set t`](gradcomdsett.md), [`set dignum`](gradcomdsetdignum.md), [`set digsiz`](gradcomdsetdigsiz.md), [`set ccolor`](gradcomdsetccolor.md), and [`set stid`](gradcomdsetstid.md) to control station selection and value labels.

![Station values](_static/plots/value.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), [station descriptor](_downloads/grads-plot-fixtures/test_station_fixture.ctl), [station data](_downloads/grads-plot-fixtures/test_station_fixture.dat), and [station map](_downloads/grads-plot-fixtures/test_station_fixture.map).
