---
title: set ylab
---

# **set ylab**

`set ylab `*`option`*

Controls the format of Y-axis tick mark labels. The *`option`* argument may be one of the following:

- `on       ` labeled tick marks are drawn with default characteristics\
  `off      ` labeled tick marks are not drawn\
  *`format`*`   ` gives a C-language template for conversion of the label value to a string\

## Usage Notes

1.  If the Y axis represents latitude, then the default behavior is to label the equator with "EQ" and append either "N" or "S" to all other tick mark labels depending on the hemisphere.
2.  Changes to the Y-axis tick mark labels are reset by <a href="gradcomdclear.html">clear</a>, but not <a href="gradcomddisplay.html">display</a>.

### Examples

This command would cause all Y-axis labels to have 2 digits after the decimal point:

- `set ylab %.2f `
