---
title: set dropmenu
---

# **set dropmenu**

Sets up the color characteristics of a dropmenu widget. The syntax is:

`set dropmenu `*`fc bc oc1 oc2 tfc tbc toc1 toc2 bfc bbc boc1 boc2 soc1 soc2 thick`*

where:

- *`fc     `* menu base text color\
  *`bc     `* menu base face color\
  *`oc1    `* dark color of shadow outline for menu base\
  *`oc2    `* bright color of shadow outline for menu base\
  *`tfc    `* menu base text color when selected with a mouse click\
  *`tbc    `* menu base face color when selected with a mouse click\
  *`toc1   `* dark color of shadow outline for menu base when selected\
  *`toc2   `* bright color of shadow outline for menu base when selected\
  *`bfc    `* menu list text color\
  *`bbc    `* menu list face color\
  *`boc1   `* dark color of shadow outline for menu list\
  *`boc2   `* bright color of shadow outline for menu list\
  *`soc1   `* dark color of shadow outline for highlighted menu item\
  *`soc2   `* bright color of shadow outline for highlighted menu item\
  *`thick  `* thickness of the shadow outline\

## Usage Notes

1.  `set dropmenu` generally precedes the <a href="gradcomddrawdropmenu.html">`draw dropmenu`</a> command.
2.  See the section of the User's Guide on <a href="script.html#widgets">widgets</a> for more information on using dropmenus.

### Example

```text

set rgb 90 100 100 100
set rgb 91  50  50  50
set rgb 92 200 200 200
set dropmenu 1 90 91 92 0 90 92 91 1 90 91 92 92 91 6
draw dropmenu 1 1 8 1.5 0.5 Menu Base | Menu Item #1 | Menu Item #2 

```
