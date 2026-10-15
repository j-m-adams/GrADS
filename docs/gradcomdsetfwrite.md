---
title: set fwrite
---

# **set fwrite**

`set fwrite <-be or -le> <-sq or -st> <-ap or -cl> `*`fname`*

Sets the filename for data output as well as byte ordering and data format.

- *`fname`*`   `output filename (default = `grads.fwrite`)\
  *`-be`*`     `output data byte ordering is big endian\
  *`-le`*`     `output data byte ordering is little endian\
  *`-sq`*`     `output data format is sequential\
  *`-st`*`     `output data format is stream (default)\
  *`-ap`*`     `output data is appended to existing file\
  *`-cl`*`     `output data replaces existing file if it exists (default)

  \

## Usage Notes

The <a href="gradcomdqfwrite.html">`q fwrite`</a> commandreturns the status of the `fwrite` options.

### Examples
