---
title: Reinitialization of GrADS
---

# Reinitialization of GrADS

GrADS provides two commands for resetting or reinitializing its state:
[`reinit`](gradcomdreinit) and [`reset`](gradcomdreset).

## `reinit`

[`reinit`](gradcomdreinit) returns GrADS completely to its initial state. It:

- closes all open files;
- releases all defined variables; and
- resets all graphics settings to their defaults.

```text
ga-> reinit
```

## `reset`

[`reset`](gradcomdreset) returns most of GrADS to its initial state, with these
exceptions:

- open files are retained;
- defined variables are retained; and
- [`set display`](gradcomdsetdisplay) settings are not modified.

If files are open, file 1 becomes the default file. The dimension environment
is set to X and Y varying, with Z and T fixed at 1, as though file 1 had just
been opened.

```text
ga-> reset
```

### Reset qualifiers

Qualify `reset` when only one part of the GrADS state should be reset:

| Command | Effect |
| --- | --- |
| `reset events` | Reset the events buffer, such as mouse clicks. |
| `reset graphics` | Reset graphics without resetting widgets. |
| `reset hbuff` | Reset the display buffer when double buffering is enabled. |
| `reset norset` | Reset X events only. |

For the complete syntax and behavior, see the [`reset` command reference](gradcomdreset).
