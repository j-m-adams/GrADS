---
title: eloop
---

eloop function

# **eloop**

`eloop(`*`expr`*`)`

The `eloop` function is similar to the <a href="gradfunctloop.html">`tloop`</a> function. When displaying a GrADS expression (*`expr`*) with the E (Ensemble) dimension varying, the`eloop` function will evaluate the *`expr`* with each designated ensemble member fixed, then reassemble the grids to obtain a final resultthat is E-varying. The `eloop` function is provided as a way to obtain E-varying resultsfrom functions or expressions that are unable to operatewhen E is a varying dimension.

## Examples

1.  Suppose you have an ensemble forecast with 15 ensemble members, and you want to evaluate the accuracy of each ensemble member by comparing it to reanalysis. You need to calculate the difference between an E-varying data set (the forecast) with a non-E-varying data set (reanalysis).
    - `open fcst.ctl`\
      `open rean.ctl`\
      `set lon -77`\
      `set lat 39`\
      `set e 1 15`\
      `set t 1 last`\
      `d eloop(t2m.1-t2m.2(e=1))`\
