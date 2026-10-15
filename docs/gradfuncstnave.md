---
title: stnave
---

# **stnave**

`stnave(`*`expr,dexpr1,dexpr2<,-m cnt>`*`)`

Takes an average of station data over time:

*`expr`*            A valid GrADS expression that gives a station data result.\
*`dexpr1`*        A dimension expression giving the starting time for the average.\
*`dexpr2`*        A dimension expression giving the ending time for the average.\
*`-m cnt`*        Optional minimal data count for the average to be taken. If, in the time series, there are fewer available data points for a particular station than the cnt value, then the result for that station is the missing data value. The default cnt value is 1 (ie, even 1 valid station in a time series of even thousands of points would give a valid result for that station).

## Usage Notes

1.  The times are looped through based on the time interval of the default file. It is thus very important to set the default file to that of the station data file, or a file with the same time interval, or not all station reports will be included in the average.
2.  If there is more than one report per station for a particular time, those reports are averaged equally to arrive at a single value for that time. The final average consists of each report for each time being averaged, with missing times not included in the average.
3.  Reports from different times are considered to be for the same station when the station id, the latitude, and the longitude all match exactly.

### Examples

1.  A typical usage of the stnave function would be:

    `stnave(ts,t=1,t=20,-m 10) `

    Here an average is taken over 20 times, and if there are fewer than 10 reports for a station then that station will be missing in the final result.
