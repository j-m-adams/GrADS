---
title: GrADS Variables
---

# GrADS Variables

- [Variable names](#names)
- [Defining new variables](#new)
- [Undefining variables](#undefine)

(names)=
## Variable names

A variable specification has the following form:

```text
abbrev.file#(dimexpr,dimexpr,...)
```

| Component | Meaning |
| --- | --- |
| `abbrev` | The variable abbreviation from the descriptor file. |
| `file#` | The number of the file containing the variable. If omitted, GrADS uses the default file. |
| `dimexpr` | A local dimension expression that overrides the current dimension environment for this variable. Multiple expressions are comma-separated. |

Only fixed dimensions can be modified with a `dimexpr`. Grid (`x`, `y`, `z`, `t`, `e`) and world (`lon`, `lat`, `lev`, `time`, `ens`) coordinates may be used. Grid coordinates are normally converted to world coordinates using the default file's scaling. The exception is a grid coordinate supplied inside a variable specification with an explicit `file#`; that coordinate uses the scaling of the selected file.

### Dimension-expression forms

| Form | Purpose | Examples |
| --- | --- | --- |
| `dimension = value` | Absolute expression; replaces the current dimension value. | `x=1`, `lat=-10`, `lev=500`, `t=1`, `time=02feb1982`, `e=1`, `ens=spr` |
| `dimension +/- offset` | Relative expression; applies an offset to the current value. | `x+1`, `lat+30`, `y-3`, `t+0`, `time+12hr` |
| `offt =/+/- offset` | Offset from the variable's initial time (GrADS 2.0.a7+). | `offt=0`, `offt+4`, `offt-1` |

See the <a href="offt.html">offset-time expressions</a> documentation for more information about `offt`.

These commands all display the second time step of `ps`:

```text
set t 2
d ps

set t 1
d ps(t+1)

set t 1
d ps(t=2)

set t 1
d ps(offt=1)

set t 1
d ps(offt+0)
```

An offset of `0` returns the first time step; an offset of `1` returns the second, and so on.

An `offt` expression is particularly useful when ensemble members have different start times. This displays the first time step of every member, even when those times occur at different positions in the file:

```text
set t 1
set e 1 last
d ps(offt=0)
```

### Complete variable specifications

| Specification | Meaning |
| --- | --- |
| `z.3(lev=500)` | Variable `z` from file 3 at an absolute level. |
| `tv.1(time-12hr)` | Variable `tv` from file 1 with a relative time expression. |
| `rh` | `rh` from the default file. |
| `q.2(t-1,lev=850)` | `q` from file 2 with two dimension expressions. |
| `z(t+0)` | `z` at the current time offset. |

### Predefined variables

GrADS provides these implicit variables for every opened gridded file:

| Variable | Value |
| --- | --- |
| `lat` | Latitude at each grid point. |
| `lon` | Longitude at each grid point. |
| `lev` | Vertical coordinate at each grid point. |

They use the scaling of the appropriate file. For example, `lat.2` returns latitudes on the grid of the second opened data set.

(new)=
## Defining new variables

The <a href="gradcomddefine.html"><code>define</code></a> command creates a variable in memory:

```text
define varname = expression
```

Defined variables can be used by later `define` or `display` commands, but are not written to disk. Avoid defining variables over unnecessarily large dimension ranges. A definition may have zero through four varying dimensions; `define` is the only GrADS operation for which four varying dimensions are valid.

When Z or T varies, `define` evaluates the expression by stepping through Z and T with those dimensions temporarily fixed. After defining a variable, reduce the number of varying dimensions before displaying it.

For example:

```text
set lon -180 0
set lat 0 90
set lev 1000 100
set t 1 10
define temp = rh

set t 5
set lev 500
d temp
```

This displays a two-dimensional slice at time 5 and level 500.

### Fixed dimensions and wildcards

Fixed dimensions in a defined variable behave like wildcards when the variable is accessed later. In this example, `zave` is defined at 500 mb but can be used with a different current level because the fixed dimensions of the definition remain part of the variable:

```text
set lon -180 0
set lat 0 90
set lev 500
set t 10
define zave = ave(z,t=1,t=30)

set t 1
set lev 200
d zave
```

When a dimension varies in the defined variable but is fixed in the current environment, GrADS retrieves the corresponding slice:

```text
set lon -180 0
set lat 0 90
set lev 500
set t 10
define temp = z
set lat 40
d temp

set lat -40
d temp
```

Values outside the dimensions used to define the variable are returned as missing. A local dimension override can be used for a varying dimension:

```text
d temp(lat=50)
```

An override of a dimension that was fixed when the variable was defined is ignored, for example `d temp(t=15)` when `temp` has fixed T. The `define` command currently supports grids only.

### Climatological defined variables

Use the <a href="gradcomdmodify.html"><code>modify</code></a> command to treat a defined variable's time axis as climatological:

```text
modify varname seasonal
modify varname diurnal
```

`seasonal` indicates monthly or multi-month means. Daily and multi-day means are not supported. `diurnal` indicates means over a period shorter than one day.

For example, this creates a 12-month climatology from ten years of monthly means and subtracts the December climatology from the final December:

```text
set lon -180 180
set lat -90 90
set lev 500
set t 1 12
define zave = ave(z,t+0,t=120,1yr)

modify zave seasonal
set t 120
d z - zave
```

(undefine)=
## Undefining variables

Each defined variable consumes system resources. Release those resources with the <a href="gradcomdundefine.html"><code>undefine</code></a> command when the variable is no longer needed:

```text
undefine p
```

The variable `p` is no longer available after it is undefined.
