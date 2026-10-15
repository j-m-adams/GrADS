---
title: set ylevs
---

# **set ylevs**

`set ylevs `*`lev1 lev2 ... levN`*

Allows the user to specify each individual labeled tick mark for the Y-axis.

## Usage Notes

1.  Reset by <a href="gradcomdclear.html">clear</a>, but not <a href="gradcomddisplay.html">display</a>.
2.  `set ylevs` will override the tick mark interval specified with the <a href="gradcomdsetylint.html">`set ylint`</a> command.
3.  If you use <a href="gradcomdsetyaxis.html">`set yaxis`</a> to specify labeled tick marks and also invoke `set ylevs`, then the tick marks *`lev1 ... levN`* specified with `set ylevs` will appear if they fall within the *`start`* and *`end`* range specified in the <a href="gradcomdsetyaxis.html">`set yaxis`</a> command.
4.  This command does not apply to a date/time axis.

### Examples
