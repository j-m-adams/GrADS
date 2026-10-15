---
title: Font Control in GrADS
---

# Font Control in GrADS

GrADS supports traditional Hershey vector fonts and additional fonts rendered
through Cairo. This page explains how to select fonts, control their size and
thickness, and use inline font and text-position overrides.

## Contents

- [Hershey fonts](#hershey)
- [Cairo fonts](#cairo)
- [Cairo emulation of Hershey fonts](#emulation)
- [Controlling font size](#size)
- [Controlling font thickness](#thick)
- [Temporary overrides](#overrides)
- [Examples](#examples)

(hershey)=
## Hershey fonts

Hershey fonts are vector fonts drawn as collections of line segments. They can
be rotated and independently scaled in the horizontal and vertical
directions. GrADS provides fonts 0–5; fonts 6–9 are reserved for additional
user-supplied Hershey font files.

| Number | Name |
| ---: | --- |
| 0 | Simplex Roman |
| 1 | Complex Roman |
| 2 | Complex Italic |
| 3 | Complex Greek |
| 4 | Duplex Roman |
| 5 | Triplex Roman |

The font data files are included with GrADS and are normally found through
the `GADDIR` setting described on the [GrADS command-line page](gradcomdgrads.md).
Use [`set font`](gradcomdsetfont.md) to
select a font. The downloadable [`font.gs`](_downloads/grads-scripts/font.gs)
script displays the characters available in a font set.

When GrADS is built with Cairo (available beginning with version 2.1),
anti-aliasing makes Hershey text smoother in the X display and image output.

(cairo)=
## Cairo fonts

Cairo provides polygon-rendered fonts that can look smoother than Hershey
fonts, especially at high output resolution. Like Hershey fonts, Cairo text
is rendered graphically rather than as raster text in a browser or terminal.
At small sizes or low resolution, Hershey text can sometimes be easier to
read.

Cairo uses the FreeType library, which supports scalable formats such as
TrueType, Type 1, OpenType, X11 PCF, and Windows FNT. See the
[FreeType documentation](https://freetype.org/) for details. Font availability
depends on the operating system and installed packages. Common font
directories include:

- macOS: `/Library/Fonts` and `/System/Library/Fonts`
- Linux and other Unix systems: `/usr/share/fonts` and `/usr/share/X11/fonts`

Hershey fonts use single-digit font numbers. Cairo fonts use double-digit
numbers; 10–99 are reserved for user-defined Cairo fonts. Select one with
[`set font`](gradcomdsetfont.md) and provide the full path to the font file:

```text
set font 10 /path/to/font.ttf
```

If the file cannot be opened, GrADS uses a generic sans-serif font. A GrADS
build without Cairo reports an error when a font number greater than 9 is
selected.

On some systems Cairo may print:

```text
Fontconfig error: Cannot load default config file
```

This message usually means that Cairo cannot find an optional fontconfig file
from the system where the library was built. It is harmless when the intended
font still renders. Font locations are not portable, which is why user-defined
Cairo fonts should use an explicit path.

(emulation)=
## Cairo emulation of Hershey fonts

[`set hershey`](gradcomdsethershey.md) provides a shortcut for switching an
existing script to generic Cairo fonts without editing each font selection:

```text
set hershey off
```

This maps most commonly used Hershey fonts to generic Cairo families. Font 3
is the Greek/symbol font and has no exact generic Cairo equivalent.

| Hershey font | Cairo approximation |
| ---: | --- |
| 0 | Sans-serif regular |
| 1 | Serif regular |
| 2 | Monospace italic |
| 4 | Sans-serif bold |
| 5 | Serif bold |

Use `set hershey on` to return to traditional Hershey fonts.

(size)=
## Controlling font size

[`set strsiz`](gradcomdsetstrsiz.md) controls the character size for both
Hershey and Cairo fonts:

```text
set strsiz hsiz [vsiz]
```

The values are virtual-page inches. `hsiz` controls the horizontal size and
`vsiz` controls the vertical size. If `vsiz` is omitted, it defaults to
`hsiz`. Hershey fonts can be stretched independently in the two directions;
Cairo fonts preserve their aspect ratio and use the vertical size as their
effective scale.

(size2)=
(thick)=
## Controlling font thickness

[`set string`](gradcomdsetstring.md) controls the line thickness of Hershey
fonts. Cairo fonts do not have a separate stroke-thickness setting; use a
bold version of the font instead.

When Cairo mode is enabled with `set hershey off`, the thickness setting from
`set string` is ignored.

(overrides)=
## Temporary overrides

Text strings can temporarily select another font or change the text baseline
with GrADS backquote escape sequences. These overrides work in `draw string`
commands, contour labels, titles, and axis labels.

### Font override

For Hershey fonts, a backquote followed by one digit selects that font for the
rest of the string, or until another backquote appears. For Cairo fonts, use a
backquote, `f`, and a two-digit font number:

```text
`1        Select Hershey font 1
`f10      Select Cairo font 10
```

### Text-position override

Use these escapes to position subsequent text relative to the baseline:

| Escape | Effect |
| --- | --- |
| `` `a `` | Superscript/above the baseline |
| `` `b `` | Subscript/below the baseline |
| `` `n `` | Return to normal baseline position |
| `` `3. `` | Draw a degree symbol using Hershey font 3 |

The font override remains active until another font escape is encountered.
The position overrides affect subsequent characters until `a`, `b`, or `n` is
used to change the position again.

(examples)=
## Examples

```text
set font 0
draw string 1.5 1.25 `1use font 2 to `2emphasize `1a particular word
draw string 1.5 2.25 use font 3 for `3greek `0letters
draw string 1.5 3.25 `3p`0r`a2`n = area of a circle
draw string 1.5 4.25 label temperatures in `3.`0C or `3.`0F
```
