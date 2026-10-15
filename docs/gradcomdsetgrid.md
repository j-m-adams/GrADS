---
title: set grid
---

# **set grid**

`set grid `*`status <linestyle> <color#>`*

Specifies the characteristics of the displayed grid lines. Default is to draw horizontal and vertical grid lines with color number 15 (grey) and linestyle 5 (dotted).

Options for *`status`* are:
- `on         ` - both latitude and longitude lines are drawn\
  `off        ` - no grid lines lines are drawn\
  `horizontal ` - only latitude grid lines are drawn\
  `vertical   ` - only longigude grid lines are drawn\

Options for *`linestyle`* are:
- `1 `- solid\
  `2 `- long dash\
  `3 `- short dash\
  `4 `- long dash, short dash\
  `5 `- dotted\
  `6 `- dot dash\
  `7 `- dot dot dash\

*`color#`* may be one of the <a href="colorcontrol.html">16 GrADS default colors</a> or a new color defined with <a href="gradcomdsetrgb.html">`set rgb`</a>.

## Usage Notes

1.  Changes to the grid display characteristics 'stick' until reset by a new execution of `set grid`.
2.  You cannot specify a *`color#`* without also specifying a *`linestyle`*.
3.  Grid lines are aligned with the labeled tick marks on the X and Y axes. GrADS chooses appropriate defaults for spacing of the labeled tick marks, but these defaults may be overridden by the commands <a href="gradcomdsetxaxis.html">set xaxis</a>, <a href="gradcomdsetyaxis.html">set yaxis</a>, <a href="gradcomdsetxlint.html">set xlint</a>, <a href="gradcomdsetylint.html">set ylint</a>, <a href="gradcomdsetxlevs.html">set xlevs</a>, and <a href="gradcomdsetylevs.html">set ylevs</a>. Please consult these reference pages for more information.

### Examples
