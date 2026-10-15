---
title: Variable Formats and Binary Data File Structure
---

# Variable Formats and Binary Data File Structure

This page explains how to refine variable declarations so GrADS can read binary
files whose storage layout or numeric type differs from the default. Before
continuing, review:

- <a href="aboutgriddeddata.html">About GrADS Gridded Data Sets</a>
- <a href="descriptorfile.html">Elements of a GrADS Data Descriptor File</a>

## Variable declaration

A descriptor-file variable record has this form:

~~~text
varname levs units description
~~~

The <a href="descriptorfile.html#VARS"><code>VARS</code></a> reference describes
the general declaration syntax. This page focuses on the <code>units</code>
field, which can activate special binary unpacking behavior.

## Default storage order

For a 3-D or 4-D data set, GrADS assumes that records are written from fastest
to slowest varying dimension in this order:

| Order | Dimension |
| --- | --- |
| 1 | Longitude (X) |
| 2 | Latitude (Y) |
| 3 | Vertical level (Z) |
| 4 | Variable (VAR) |
| 5 | Time (T) |

If the binary file uses another order, set <code>units</code> to a special
comma-delimited structure code.

## Structure codes

| Units value | Storage layout or behavior | Notes |
| --- | --- | --- |
| <code>99</code> | Default handling | Ignore special unpacking features. |
| <code>-1,10,arg</code> | VAR and Z transposed: X, Y, VAR, Z, T | Removed in GrADS 2.0. Designed for NASA GCM “phoenix” data. <code>arg=1</code> means VAR/Z transposed; <code>arg=2</code> means it is not. |
| <code>-1,20</code> | VAR and T transposed: X, Y, Z, T, VAR | All times for one variable are stored together, followed by all times for the next variable. |
| <code>-1,30</code> | X and Y transposed: X, Y data are stored as latitude, longitude | Inefficient and retained mainly for inspection and debugging. |
| <code>-1,40,arg</code> | Non-floating-point data | Values are converted to floats after reading; <code>arg</code> selects the input type. |

Use <code>-1</code> followed by the structure code and any required arguments:

~~~text
units = -1,structure[,arg]
~~~

## Non-floating-point types

For <code>units=-1,40,arg</code>, the secondary argument identifies the binary
data type:

| Units value | Input type |
| --- | --- |
| <code>-1,40,1</code> | One-byte unsigned characters (0–255). |
| <code>-1,40,2</code> | Two-byte unsigned integers. |
| <code>-1,40,-2</code> | Two-byte signed integers. |
| <code>-1,40,4</code> | Four-byte integers. |

## VAR/T-transposed data with templates

When separate files are aggregated with a template, <code>-1,20</code> needs an
additional argument giving the number of time steps in each individual file.
For example, this descriptor reads ten years of monthly wind components and
temperature data stored with VAR and T transposed:

~~~text
DSET ^monthlydata_%y4.dat
OPTIONS template
...
TDEF 120 linear jan79 1mo
VARS 3
u 18 -1,20,12 u component
v 18 -1,20,12 v component
t 18 -1,20,12 temperature
ENDVARS
~~~

The <code>12</code> argument indicates that each templated file contains twelve
time steps.

