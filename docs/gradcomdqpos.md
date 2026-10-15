---
title: q pos
---

# **q pos**

`q pos `

After this command is issued, GrADS waits for user's mouse click, then returns the coordinates of the mouse click plus additional information. If the `nowait` argument is used, then GrADS will query the mouse without waiting for a mouse click. The returned information makes '`q pos`' such a powerful command especialy when used in conjunction with the different 'classes' of widgets: buttons, rubber bands, and dropmenus. Here is a template of the information that '`q pos`' returns after a mouse click (note the difference in output between the different widget classes):

`Position = `*`xpos ypos mbtn class`*`                                   ` (somewhere in the graphics window)\
`Position = `*`xpos ypos mbtn class widget# btnstate`*`                  ` (for buttons)\
`Position = `*`xpos ypos mbtn class widget# xpos2 ypos2`*`               ` (for rbands)\
`Position = `*`xpos ypos mbtn class widget# menuitem <casc# cascitem>`*` ` (for dropmenus)\

where:

- *`xpos, ypos`*`    `- coordinates of the mouse click in real page units\
  *`mbtn`*`          `- either 1, 2, or 3 for the left, center, or right mouse button\
  *`class`*`         `- either 1, 2, 3, or 0 for button, rband, dropmenu, or 'not a widget'\
  *`widget#`*`       `- the number assigned to the widget when it was originally set up\
  *`btnstate`*`      `- either 0 (meaning "off") or 1 (meaning "on")\
  *`xpos2, ypos2`*`  `- coordinates of the mouse release point in virtual page units\
  *`menuitem`*`      `- the item number selected from the menu list\
  *`casc#`*`         `- the cascade menu number selected from the dropmenu list\
  *`cascitem`*`      `- the item number selected from the cascade menu\

## Usage Notes

1.  If the user did not click on a widget, then*`class`* will be 0 and there will be no furtheroutput.
2.  If the user clicks on a dropmenu but no menu item is selected,then *`widget#`* and *`menuitem`*will both be -1.
3.  There can be up to three levels of nested cascading dropmenus launchedfrom the main dropmenu. In other words, *`casc#`* and *`cascitem`* willrepeat up to three times in the output from '`q pos`'.
4.  The following reference pages contain information on configuring and drawing the widgets:\
    <a href="gradcomdsetbutton.html">`set button`</a>\
    <a href="gradcomddrawbutton.html">`draw button`</a>\
    <a href="gradcomdredrawbutton.html">`redraw button`</a>\
    <a href="gradcomdsetrband.html">`set rband`</a>\
    <a href="gradcomdsetdropmenu.html">`set dropmenu`</a>\
    <a href="gradcomddrawdropmenu.html">`draw dropmenu`</a>

### Examples

See the section of the User's Guide on <a href="script.html#widgets">widgets</a>for plenty of script examples showing how to use 'q pos'.
