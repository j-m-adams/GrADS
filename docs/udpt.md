---
title: The User Defined Plug-in Table (UDPT)
---

# The User Defined Plug-in Table (UDPT)

The UDPT is a stand-alone text file containing the information GrADS needs to
load [user-defined plug-ins](udp.md) and graphics plug-ins. Each non-comment
line describes one plug-in.

## Record format

A record contains three blank-separated fields, plus an optional fourth field
for function aliases:

```text
Type  Name  Filename  [Alias]
```

| Field | Description |
| --- | --- |
| `Type` | One of `function`, `gxdisplay`, or `gxprint`. |
| `Name` | The name used to invoke or select the plug-in. |
| `Filename` | The full path to the shared object or dynamic library. It must not contain spaces. |
| `Alias` | Optional actual routine name in the shared object (available since version 2.2.1). |

Comments begin with `*` or `#` and may appear on their own line.

## Record types

| Type | Purpose |
| --- | --- |
| `function` | A user-defined function invoked in a GrADS expression. See the [UDP documentation](udp.md). |
| `gxdisplay` | A graphics display plug-in selected with `-d`. |
| `gxprint` | A graphics printing plug-in selected with `-h`. |

## Naming rules

### Function plug-ins

For a `function` record, `Name` is the function name used in a GrADS
expression. It must:

- begin with a letter;
- contain no more than 15 characters;
- contain only letters, numbers, and underscores; and
- use lowercase letters.

Use the optional `Alias` field when the routine name in the source code does
not meet these rules. The name is used in GrADS; the alias identifies the
routine exported by the shared object.

### Graphics plug-ins

For `gxdisplay` and `gxprint` records, `Name` is a case-sensitive nickname
passed with `-d` or `-h`. The same character restrictions apply, except that
mixed-case names are allowed.

### Shared-object filename

`Filename` must be an absolute path to the shared object or dynamic library,
such as a `.so` file on Linux. GrADS loads the library with `dlopen` and finds
the named routine with `dlsym`.

## Table locations and precedence

GrADS reads UDPT records from:

1. The user-specified table named by the `GAUDPT` environment variable.
2. The default table named `udpt` in the directory specified by `GADDIR`.

Entries in `GAUDPT` take precedence over entries in the default table when both
have the same `Type` and `Name`. See the [GrADS environment documentation](gradcomdgrads.md)
for these variables.

## Example

The following table defines display, printing, and user-defined function
plug-ins:

```text
# Type       Name      Full path to shared object file
# ---------  --------  -----------------------------------------------
gxdisplay   Cairo     /usr/local/lib/grads/libgxdCairo.so
gxdisplay   X11       /usr/local/lib/grads/libgxdX11.so
gxdisplay   gxdummy   /usr/local/lib/grads/libgxdummy.so
*
gxprint     Cairo     /usr/local/lib/grads/libgxpCairo.so
gxprint     GD        /usr/local/lib/grads/libgxpGD.so
gxprint     gxdummy   /usr/local/lib/grads/libgxdummy.so
*
function    dothis    /home/username/grads/udp/dothis.so
```

## Querying the UDPT

Run `q udpt` to display the complete list of records parsed during startup:

```text
ga-> q udpt
gxdisplay   Cairo    /usr/local/lib/grads/libgxdCairo.so
gxdisplay   X11      /usr/local/lib/grads/libgxdX11.so
gxdisplay   gxdummy  /usr/local/lib/grads/libgxdummy.so
gxprint     Cairo    /usr/local/lib/grads/libgxpCairo.so
gxprint     GD       /usr/local/lib/grads/libgxpGD.so
gxprint     gxdummy  /usr/local/lib/grads/libgxdummy.so
function    dothis   /home/username/grads/udp/dothis.so
```
