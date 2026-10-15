---
title: set xlint
---

# **set xlint**

`set xlint `*`interval`*

Specifies the *`interval`* between labeled tick marks on the X-axis.

## Usage Notes

1.  If *`interval`* is a positive value, the labeled tick marks will 'start' at `0`, regardless of the current dimension environment. For example, if you set the *`interval`* to `3`, the labeled tick marks will be at `0, 3, 6, 9` ...
2.  If *`interval`* is a negative value, the labeled tick marks will 'start' at the axis start value, which is usually the lower limit of the X dimension environment. If this were `30` (with an *`interval`* of `10`), then the labeled tick marks would be at `30, 40, 50, 60`...
3.  This command is overridden by the <a href="gradcomdsetxlevs.html">`set xlevs`</a> command.
4.  This command will override the X-axis tick mark interval specified with the <a href="gradcomdsetxaxis.html">`set xaxis`</a> command.
5.  Reset by <a href="gradcomdclear.html">clear</a>, but not <a href="gradcomddisplay.html">display</a>.
6.  This command does not apply to a date/time axis.
7.  `set xlint` and <a href="gradcomdsetylint.html">`set ylint`</a> may not work as described when used to control grid lines for polar stereographic projections.

### Examples
