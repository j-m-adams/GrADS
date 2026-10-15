---
title: oabin()
---

# **oabin()**

`oabin (`*`gexpr, sexpr <,-flag>`*`)`

This function bins station observations into grid cells based on their location. If more than one station is located within a grid box, the observations are averaged together to produce the analyzed value.

- *`gexpr   `*a valid grid expression\
  *`sexpr   `*a valid station data expression\

The optional *`flag`* may be one of the following:

- `-f      `set the value to the first observation that gets binned\
  `-c      `output is the \# of stations located within each grid cell

## Usage Notes

1.  The -f flag should be used when summing is not appropriate -- e.g. surface type classification.
2.  See the related function <a href="gradfuncoacres.html">`oacres`</a>.

### Example

```text

open grd_var.ctl
open stn_var.ctl

set gxout grfill
d oabin(grd_var.1,stn_var.2)

set gxout grid
d oabin(grd_var.1,stn_var.2,-c)

```
