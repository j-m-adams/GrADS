---
title: set dialog
---

# **set dialog**

`set dialog `*`pc fc bc oc th`*` <numeric|n>`

Sets color properties for dialog box widgets.

- *`pc`*`    `prompt color\
  *`fc`*`    `foreground text color\
  *`bc`*`    `background color\
  *`oc`*`    `outline color\
  *`th`*`    `outline thickness\
  `n     `numeric input only (optional)\

## Usage Notes

1.  If *`th`* is 1-5, a one-pixel border is drawn; if it is 6 or more, a two-pixel border is drawn.
2.  Default colors are accessed by setting the colors to -1.
3.  If the numeric option is invoked, keyboard inputs are restricted to 0-9, ., -, +, e, and E. Entered value is checked to assure it is a rational number of the form +/-nnnn, +/-nnnn.dddd, or +/-nnnn.ddddE+/-xxxx. This assures the returned numeric value is valid for use by a GrADS script.
4.  See the <a href="gradcomdqdialog.html">`q dialog`</a> refrence page for details on execution of the dialog box widgets.

### Examples

```text

set dialog 1 0 5 1 6
q dialog Hello World
say result

```
