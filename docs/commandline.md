---
title: Command-line Editing and History
---

# Command-line Editing and History

When the `readline` library is available, GrADS displays the prompt `ga->`
instead of `ga>`. The arrow indicates that command-line editing is active.
Readline uses Emacs-style editing by default, but it can also be configured
for Vi-style editing.

## Editing keys

These keys are commonly useful at the GrADS prompt:

| Key | Action |
| --- | --- |
| `Ctrl-A` | Move to the beginning of the line. |
| `Ctrl-E` | Move to the end of the line. |
| `Ctrl-F` | Move forward one character. |
| `Ctrl-B` | Move backward one character. |
| `Ctrl-D` | Delete the character under the cursor. |
| `Ctrl-P` | Recall the previous command line. |
| `Ctrl-N` | Recall the next command line. |
| `Ctrl-R` | Search backward through command history. |

## Filename completion

Press `Tab` to complete a filename or directory name. If more than one
completion is possible, press `Tab` twice to list the available choices.

For example, entering:

```text
ga-> open /h
```

and pressing `Tab` twice may show:

```text
h      home   home1  home2
```

Continue typing the desired path and use `Tab` twice whenever you need to see
the possible completions:

```text
ga-> open /home1/
GCC         bogus603    gnu         iqpops      nmcobs      roesserd
GRIB        cstrey      grads       lost+found  pacek       tsai
Mosaic      dh          hamilton    mendhall    picardr     witt
NEWDBS      dolan       hout        nicholso    qcops
```

The same technique works for files. For example:

```text
ga-> open /home1/GRIB/dat/nogaps.25.
nogaps.25.95021600.grb       nogaps.25.95021912.grb
nogaps.25.95021600.gribmap   nogaps.25.95021912.gribmap
nogaps.25.95021612.anal.grb  nogaps.25.anal.ctl
nogaps.25.95021612.ctl       nogaps.25.anal.gribmap
nogaps.25.95021612.grb       nogaps.25.ls.mask.ctl
nogaps.25.95021612.gribmap   nogaps.25.ls.mask.dat
nogaps.25.95021700.anal.grb  nogaps.25.95021700.ctl
```

After typing `950217` and pressing `Tab` twice, the shorter list is:

```text
ga-> open /home1/GRIB/dat/nogaps.25.950217
nogaps.25.95021700.anal.grb  nogaps.25.95021712.ctl
nogaps.25.95021700.ctl       nogaps.25.95021712.grb
nogaps.25.95021700.grb       nogaps.25.95021712.gribmap
nogaps.25.95021700.gribmap   nogaps.25.95021712.anal.grb
```

Complete the filename, for example `nogaps.25.95021712.ctl`, and press
`Enter` to open it.

## Disabling readline

Readline support is not guaranteed to work on every system. Use the `-h`
startup option to disable the readline routines when necessary. See the
[`grads` command reference](gradcomdgrads) for other startup options.
