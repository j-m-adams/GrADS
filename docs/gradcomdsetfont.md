---
title: set font
---

GrADS command: set font

# **set font**

This command allows the user to select the font for subsequent text operations. This command was enhanced for version 2.1, when new font controls were introduced with the addition of graphics rendering using the Cairo library.

## Syntax

`set font `*`number`*`                     `(GrADS version 2.0\* and earlier)\
`set font `*`number`*` <file `*`filename`*`>     `(GrADS version 2.1\* and later)

### Usage Notes

Use font *`number`* 0 through 5 to specify which Hershey font to use.\
Use font *`number`* 6 through 9 for customized font files. See <a href="font.html">Hershey Font Files</a> for more information on how to create these files.\
For all GrADS versions 2.0 and earlier, 0-9 are the only font numbers that are available.

Starting with GrADS version 2.1, additional font numbers 10 through 99 are available to define or set a Cairo font based on a local font file. When defining a new font, use the `'set font `*`number`*` file `*`filename`*`'` syntax. Make sure you include the `file` keyword followed by the filename of the local font file, including its full path. Spaces are allowed in *`filename`* (see examples below), but be careful that no extra whitespace is added on at the end of the *`filename`* argument (this can happen when using the tab key to fill out a filename instead of typing it out entirely). When setting a previously-defined font, use the `'set font `*`number`*`'` syntax to draw text using that defined font -- it is not necessary to re-define the font every time you want to use it. Defining a new font number will also set the font to be that number.

If` 'set font `*`number`*`' `is invoked with a *`number`* between 10 and 99, but that font *`number`* has not yet been defined, a default generic sans-serif font will be used. Similarly, if a user-specified font file cannot be opened or is in an unsupported format, then the display will default to a generic sans-serif font.

Please read the documentation page on <a href="fontcontrol.html">Font Control in GrADS</a> for more information about using fonts in GrADS version 2.1.

It is possible to <a href="fontcontrol.html">temporarily override the font</a> in a string of text by using the back quote character (\`) followed by a single-digit font number. To override fonts with 2-digit font numbers, use back quote with an f (\`f) followed by the font number. Examples are below.

If your build of GrADS version 2.1+ is not enabled with the Cairo graphics library, you will get an error message if you use <a href="gradcomdsetfont.html">`set font`</a> to try to set a font number greater than 9.

 

### Examples

```text

  set font 10 file /Library/Fonts/AmericanTypewriter.ttc
  set font 11 file /Library/Fonts/Kannada MN.ttc
  set font 12 file /System/Library/Fonts/AppleGothic.ttf
  set font 13 file /usr/share/fonts/default/Type1/a010013l.pfb
  set font 14 file /usr/share/msttcorefonts/tahoma.ttf
  set font 15 file /usr/share/X11/fonts/TTF/luximbi.ttf

  set font 20 file /usr/share/fonts/default/ghsostcript/bchr.pfa
  set font 21 file /usr/share/fonts/default/ghsostcript/bchri.pfa
  set font 22 file /usr/share/fonts/default/ghsostcript/bchb.pfa
  set font 23 file /usr/share/fonts/default/ghsostcript/bchbi.pfa

  set font 20
  draw string 1 1 For emphasizing text, use `f22bold, `f21italic, `f20or `f23both

  set font 0
  draw string 1 1 `1Use Hershey font 2 to `2emphasize `1a particular word 
  
```
