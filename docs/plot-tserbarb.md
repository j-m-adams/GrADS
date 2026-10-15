---
title: Time-series wind barbs
---

# Time-series wind barbs

Wind barbs for station time series. This minimal example uses the shared deterministic plotting fixture.

## Example

```text
open test_station_fixture.ctl
set lon 4
set lat 52
set t 1 3
set gxout tserbarb
display const(u,0);u;v
printim tserbarb.png png white
```

## Relevant controls

Use [`set t`](gradcomdsett.md), [`set lon`](gradcomdsetlon.md), [`set lat`](gradcomdsetlat.md), [`set arrscl`](gradcomdsetarrscl.md), [`set barbopts`](gradcomdsetbarbopts.md), [`set vrange`](gradcomdsetvrange.md), and [`set ccolor`](gradcomdsetccolor.md) to control the time window, station point, and barb style.

![Time-series wind barbs](_static/plots/tserbarb.png)

Download the [example script](_downloads/grads-scripts/test_plot.gs), [station descriptor](_downloads/grads-plot-fixtures/test_station_fixture.ctl), [station data](_downloads/grads-plot-fixtures/test_station_fixture.dat), and [station map](_downloads/grads-plot-fixtures/test_station_fixture.map).
