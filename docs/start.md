---
title: Getting Started with GrADS
---

# Getting Started with GrADS

This page explains how to start GrADS, choose the initial graphics mode, get
help, and exit the program.

## Starting GrADS

Start GrADS by entering the name of the executable at a command-line prompt:

```console
$ grads
```

Before initializing the graphics output environment, GrADS asks whether to use
landscape or portrait mode. Landscape mode uses an 11 × 8.5 page, while
portrait mode uses an 8.5 × 11 page. The graphics window is scaled to fit the
workstation display; these dimensions describe the page aspect ratio rather
than the physical size of the window.

For the complete list of command-line options, see the [`grads` command
reference](gradcomdgrads).

## Startup options

The following options can be supplied when starting GrADS:

| Option | Description |
| --- | --- |
| `-help` | Print a list of command-line options. |
| `-l` | Start in landscape mode without asking which orientation to use. |
| `-p` | Start in portrait mode without asking which orientation to use. |
| `-b` | Start in batch mode; do not open a graphics output window. |
| `-g geom` | Set the graphics-window dimensions explicitly at startup. |
| `-c cmd` | Execute `cmd` as the first GrADS command after startup. |

GrADS treats the display window as an 11 × 8.5 or 8.5 × 11 page regardless of
its pixel dimensions. When using `-g`, choose dimensions with the corresponding
aspect ratio.

For example, to start in landscape mode:

```console
$ grads -l
```

## Resizing the graphics window

After GrADS starts, resize the graphics window with the [`set xsize` command
reference](gradcomdsetxsize):

```text
ga-> set xsize x y
```

Here, `x` and `y` are the window width and height in pixels.

## Working with GrADS

Enter GrADS commands in the text window from which you started the program.
Graphics output appears in the graphics window in response to those commands.
Keep the text window active when entering commands so that it receives
keyboard input.

## Getting help

The `help` command prints a summary of the operations needed for basic work:

```text
ga-> help
```

This summary is intended as a quick reminder rather than an exhaustive manual.
For details about a command, enter the command name by itself or consult the
appropriate command page in the documentation.

## Quitting GrADS

Exit GrADS with the `quit` command:

```text
ga-> quit
```
