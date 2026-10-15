---
title: Time-series weather symbols
---

# Time-series weather symbols

Weather symbols for station time series. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_station_fixture.ctl
set lon 4
set lat 52
set t 1 3
set gxout tserwx
display wx
printim tserwx.png png white
```

## Relevant controls

Use [`set t`](gradcomdsett.md), [`set lon`](gradcomdsetlon.md), [`set lat`](gradcomdsetlat.md), [`set wxopt`](gradcomdsetwxopt.md), [`set digsiz`](gradcomdsetdigsiz.md), and [`set ccolor`](gradcomdsetccolor.md) to control the time window, station point, symbol type, size, and color.

![Time-series weather symbols](_static/plots/tserwx.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), [station descriptor](_downloads/grads-plot-fixtures/test_station_fixture.ctl), [station data](_downloads/grads-plot-fixtures/test_station_fixture.dat), and [station map](_downloads/grads-plot-fixtures/test_station_fixture.map).
