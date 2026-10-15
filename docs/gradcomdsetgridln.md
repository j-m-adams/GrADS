---
title: set gridln
---

# **set gridln**

`set gridln auto|off|`*`col#`*` `

Used with <a href="gradcomdsetgxout.html">set gxout grid</a> to control the presence and appearance of the grid lines. The options are as follows:

- `auto` (default)

  - The grid lines are drawn and are the same color as the text.

  `off`

  - The grid lines are not drawn.

  *`col#`*

  - The grid lines are drawn in the specified color. *`col#`* may be one of the <a href="16colors.html">16 GrADS default colors</a> or the number of a user-defined color created with <a href="gradcomdsetrgb.html">set rgb</a>.

## Usage Notes

1.  `set grdln` is reset by entering <a href="gradcomdclear.html">`clear`</a>.

### Examples
