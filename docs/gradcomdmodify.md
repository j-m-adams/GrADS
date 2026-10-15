---
title: modify
---

# modify

modify *varname type*

This command defines a climatological variable, which is year-independent. *varname* is a defined grid. There are two options for *type*:

- `seasonal   ` - For creating monthly or multi-monthly climatologies\
  `diurnal    ` - For creating climatologies over a time period less than a day\
  

## Usage Notes

### Example

Say you have a 50-year timeseries of monthly mean sea surface temperatures (a variable named sst with 600 time steps) and you want to create a climatology and then look at the monthly anomalies. First, set the time range for 1 to 12, to span a complete year. Second, define the variable "sstclim" which will contain the January mean in the first time step, the February mean in the second time set, etc. Then use 'modify' to turn 'sstclim' into a climatological variable. This means that the calendar year associated with 'sstclim' (the first year in the original sst data set) becomes a wild card. Then you can define the anomaly by subtracting the climatology from the original time series. The commands are as follows:

'set t 1 12'\
'define sstclim = ave(sst, t+0, t=600, 12)'\
'modify sstclim seasonal'\
'set t 1 last'\
'define anom = sst - sstclim'\
