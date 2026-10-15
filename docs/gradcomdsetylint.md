---
title: set ylint
---

# **set ylint**

`set ylint `*`interval`*

Specifies the *`interval`* between labeled tick marks on the Y-axis.

## Usage Notes

1.  If *`interval`* is a positive value, the labeled tick marks will 'start' at `0`, regardless of the current dimension environment. For example, if you set the *`interval`* to `3`, the labeled tick marks will be at `0, 3, 6, 9` ...
2.  If *`interval`* is a negative value, the labeled tick marks will 'start' at the axis start value, which is usually the lower limit of the Y dimension environment. If this were `30` (with an *`interval`* of `10`), then the labeled tick marks would be at `30, 40, 50, 60`...
3.  This command is overridden by the <a href="gradcomdsetylevs.html">`set ylevs`</a> command.
4.  This command will override the Y-axis tick mark interval specified with the <a href="gradcomdsetyaxis.html">`set yaxis`</a> command.
5.  Reset by <a href="gradcomdclear.html">clear</a>, but not <a href="gradcomddisplay.html">display</a>.
6.  This command does not apply to a date/time axis.
7.  `set ylint` and <a href="gradcomdsetxlint.html">`set xlint`</a> may not work as described when used to control grid lines for polar stereographic projections.

### Examples
