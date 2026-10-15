---
title: collect
---

# **collect**

This command provides a way for station data profiles and time series to be saved in memory as a set. The command format is:

- `collect `*`cnum expr`*

where *`cnum`* is the collection number, a value from 0 to 31, and *`expr`* is any GrADS expression that gives a station data result. If `"free"` is given for *`expr`*, the collection is emptied and all storage is freed.

## Usage Notes

1.  The `collect` command will only work when one dimension is varying, either Z or T. Each time `collect` is issued for a particular collection number, the station data is added to the collection of stations. The order in which stations are added is important.
2.  After a group of station profiles or time series have been saved as a collection, use the <a href="gradfunccoll2gr.html">coll2gr</a> function to convert the collection into a grid.
3.  See the section of the User's Guide on <a href="usingstationdata.html#xsection">Arbitrary Cross Sections</a> for more information.
