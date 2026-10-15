---
title: set rbcols
---

# **set rbcols**

`set rbcols `*`color1 color2 ... colorN`*\

This command specifies a new rainbow color sequence. The *`color#`* arguments may be taken from the <a href="colorcontrol.html">16 GrADS default colors</a> or they may be new color numbers defined via the <a href="gradcomdsetrgb.html">`set rgb`</a> command. This sequence of colors replaces the <a href="colorcontrol.html">default rainbow color sequence</a> whenever the rainbow colors are used.

`set rbcols ` given without any arguments will return to the default GrADS rainbow sequence.

## Usage Notes

1.  Changes to the rainbow color sequence 'stick' until reset by a new execution of `set rbcols`.

### Examples
