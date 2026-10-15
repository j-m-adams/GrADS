---
title: set xlevs
---

# **set xlevs**

`set xlevs `*`lev1 lev2 ... levN`*

Allows the user to specify each individual labeled tick mark for the X-axis.

## Usage Notes

1.  Reset by <a href="gradcomdclear.html">clear</a>, but not <a href="gradcomddisplay.html">display</a>.
2.  `set xlevs` will override the tick mark interval specified with the <a href="gradcomdsetxlint.html">`set xlint`</a> command.
3.  If you use <a href="gradcomdsetxaxis.html">`set xaxis`</a> to specify labeled tick marks and also invoke `set xlevs`, then the tick marks *`lev1 ... levN`* specified with `set xlevs` will appear if they fall within the *`start`* and *`end`* range specified in the <a href="gradcomdsetxaxis.html">`set xaxis`</a> command.
4.  This command does not apply to a date/time axis.

### Examples
