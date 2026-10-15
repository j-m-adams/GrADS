---
title: Plug-ins for GrADS
---

# Plug-ins for GrADS

Plug-ins are software components that add features to an existing program.
GrADS uses plug-ins to provide user-defined functions and graphics backends.
The [User Defined Plug-in Table (UDPT)](udpt.md) records the plug-in names and
the shared libraries that implement them. See the [UDPT reference](udpt.md) for
the record syntax.

## User-defined plug-in functions

User Defined Plug-ins (UDPs) were introduced in version 2.1.1.b0 to replace the
legacy file-based user-defined-function interface. UDPs allow users to write
custom GrADS functions in the language of their choice.

Arguments and data grids are passed directly in memory rather than through
temporary files. This improves both performance and flexibility. See the
[UDP documentation](udp.md) for compilation, setup, and usage instructions.

## Graphics plug-ins

Version 2.2.0 redesigned the graphics subsystem as plug-ins. Display and
printing are independent, so users can select the backend that best fits their
workflow. The graphics plug-ins shipped with GrADS are part of the core source
code; they are not user-defined plug-ins.

The pre-Cairo PostScript and Encapsulated PostScript output engines are no
longer supported. Those formats are available only through the Cairo backend.

### Display backends

For interactive X-window display, choose between Cairo and the traditional X11
backend:

| Backend | Strengths | Limitations |
| --- | --- | --- |
| `Cairo` | Fonts, anti-aliasing, transparency, pattern filling, and improved visual quality | Does not support widgets; can be slower; some X11 platforms may show incomplete rendering |
| `X11` | Supports widgets such as buttons, drop menus, and rubber-banding | Older rendering capabilities and no Cairo font/anti-aliasing features |

### Printing backends

For image and vector output, choose Cairo or GD:

| Backend | Formats and features |
| --- | --- |
| `Cairo` | PNG, PS, EPS, PDF, and SVG; supports fonts, anti-aliasing, transparency, and pattern filling |
| `GD` | PNG, GIF, and JPEG; fast and compact, but without fonts, anti-aliasing, transparency, or pattern filling |

The `gxdummy` plug-in disables all graphics routines. It is useful when no
graphics output is required and can serve as a template for a new graphics
plug-in.

## Mixing display and printing backends

Any display backend can be combined with any printing backend. Selecting Cairo
for both generally produces output that matches the interactive display. Using
X11 with GD also matches the display apart from widgets, which are never
printed.

## Selecting graphics plug-ins

Use `-d` to select the display backend and `-h` to select the hardcopy/printing
backend. The available names are listed in the [UDPT](udpt.md).

```console
grads -d Cairo -h GD
grads -d Cairo -h Cairo
grads -d X11 -h GD
grads -d X11 -h Cairo
```

If neither option is supplied, GrADS selects `Cairo` for both display and
printing. Specify `-d` or `-h` only when a different backend is required.

## Diagnosing plug-in errors

At startup, GrADS reads plug-in records from:

1. The file named by `GAUDPT`.
2. A file named `udpt` in `GADDIR`.

Records in the `GAUDPT` file take precedence over records in the default
`GADDIR/udpt` file. Syntax errors are reported as warnings; the invalid record
is ignored and startup continues.

After parsing the available records, GrADS initializes the graphics package:

```text
GX Package Initialization
```

Messages such as:

```text
GX Package Error: Could not find a record for the printing plug-in
GX Package Error: Could not find a record for the display plug-in
```

usually indicate a missing or invalid [UDPT record](udpt.md). A message such
as:

```text
GX Package Error: dlopen failed
```

indicates a problem with the shared object itself—for example, a missing,
corrupt, inaccessible, or incorrectly linked library.

## Querying graphics plug-ins

Use these commands after startup:

```text
q udpt
q gxconfig
```

- `q udpt` lists the plug-ins found while parsing the UDPT files.
- `q gxconfig` reports the active graphics plug-ins, shared-object filenames,
  and relevant library-version information.
