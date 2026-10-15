---
title: set xlab
---

# **set xlab**

`set xlab `*`option`*

Controls the format of X-axis tick mark labels. The *`option`* argument may be one of the following:

- `on       ` labeled tick marks are drawn with the default characteristics\
  `off      ` labeled tick marks are not drawn\
  *`format`*`   ` gives a C-language template for conversion of the label value to a string\

## Usage Notes

1.  If the X axis represents longitude, then the default behavior is to append an "E" or "W" to all tick mark labels depending on the hemisphere.
2.  Changes to the X-axis tick mark labels are reset by <a href="gradcomdclear.html">clear</a>, but not <a href="gradcomddisplay.html">display</a>.

### Examples

This command would cause all X-axis labels to have 2 digits after the decimal point:

- `set xlab %.2f `
