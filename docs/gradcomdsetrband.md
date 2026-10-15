---
title: set rband
---

# **set rband**

`set rband `*`num mode x1 y1 x2 y2`*

Sets the mode and defines the active region for the 'rubber band' widget.

- *`num`*`   ` - widget number\
  *`mode`*`  ` - may be either `box` or `line`\
  *`x1`*`    ` - lowest X point where the widget will be active (in virtual page units)\
  *`y1`*`    ` - lowest Y point where the widget will be active (in virtual page units)\
  *`x2`*`    ` - highest X point where the widget will be active (in virtual page units)\
  *`y2`*`    ` - highest Y point where the widget will be active (in virtual page units)\

After executing `set rband`, rubber bands are defined by executing <a href="gradcomdqpos.html">`q pos`</a> which freezes the system until the user clicks, drags, and then releases the mouse somewhere within the active rband area. See the <a href="gradcomdqpos.html">`q pos`</a> reference page for details on the returned information.

## Usage Notes

1.  In `box` mode, as the user clicks and drags the mouse in the active rband area a box is drawn with one corner located at the initial click and the opposite corner located at the release point. In `line` mode, a line is drawn between these two points.

### Examples

See <a href="script.html#widgets">widgets</a> for more information and examples.
