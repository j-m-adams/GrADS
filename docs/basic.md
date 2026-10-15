---
title: GrADS Basics
---

# GrADS Basics

This page introduces the basic GrADS workflow: open data, select what and
where to display, and control how the result is presented.

## The GrADS work areas

After GrADS has started, you work with two windows:

- a terminal window containing the GrADS prompt, `ga->`, where commands are
  entered;
- a graphics window, resizable and black by default, where plots are drawn.

GrADS commands are entered in the terminal window. The response is either
graphics in the graphics window or text in the terminal window.

## The three fundamental commands

Most GrADS work begins with these three commands:

| Command | Purpose |
| --- | --- |
| [`open`](gradcomdopen) | Open or make a gridded or station data file available to GrADS. |
| [`d` (display)](gradcomddisplay) | Display a GrADS expression, such as a slice of data. |
| [`set`](commandsatt.md) | Control what is displayed, where it is displayed, and how it looks. |

For example, after opening a data file, you can display a variable with:

```text
ga-> d slp
```

## Expressions: what to display

The expression supplied to `d` specifies what you want to display. It may be
as simple as a variable from the opened data file:

```text
ga-> d slp
```

Expressions can also use arithmetic or GrADS intrinsic functions:

```text
ga-> d slp/100
ga-> d mag(u,v)
```

Here, `mag` is an intrinsic function that calculates the magnitude of its two
arguments.

## The dimension environment: where to display

The *dimension environment* defines which part, or hyperslab, of the five-
dimensional geophysical space is displayed:

```text
longitude, latitude, level, time, ensemble
```

Use the [`set` command's dimension-environment options](commandsatt.md)
to select dimensions in either grid coordinates (`x`, `y`, `z`, `t`, and `e`)
or world coordinates (`lon`, `lat`, `lev`, `time`, and `ens`).

## Display settings: how to display

The [`set` command](commandsatt.md) also controls how data is displayed. Its
options include graphics methods such as contours and streamlines, as well as
output destinations such as writing data to a file.

GrADS graphics can be written to a metacode file with [`enable
print`](gradcomdenableprint) and [`print`](gradcomdprint). The resulting file
can then be converted to PostScript for printing or to other image formats.

GrADS also provides graphic primitives, including lines and circles, and basic
labeling through the [`draw` command family](commandsatt.md).

## Querying the session

Use [`q` or `query`](gradcomdquery) to obtain information about the current
GrADS session, including which files are open and available statistics:

```text
ga-> q file
```

See the [full command reference](commandsatt.md) for the complete set of commands
and options.
