---
title: draw button
---

# **draw button**

`draw button `*`number xpos ypos width height string`*

Draws a button centered on *`xpos,ypos`* with the following attributes:

- `number   `button number, 1 - 512\
  `xpos     `x center of the button in page coordinates (inches)\
  `ypos     `y center of the button in page coordinates (inches)\
  `width    `width (x) of the button (inches)\
  `height   `height (y) of the button (inches)\
  `string   `text to display centered in the button\

## Usage Notes

1.  Button colors are specified with the <a href="gradcomdsetbutton.html">`set button`</a> command.
2.  A button's initial state is `ON`. If a user clicks on a button following a <a href="gradcomdqpos.html">`q pos`</a> command, then the button state will switch from `ON (1)` to `OFF (0)`. A second <a href="gradcomdqpos.html">`q pos`</a> followed by a mouse click on the button will return it to the `ON` state.
3.  The button "state" (i.e. `ON/OFF`) may also be set with the <a href="gradcomdredrawbutton.html">`redraw button`</a> command.
4.  See the section of the User's Guide on <a href="script.html#widgets">widgets</a> for more information on using buttons.

### Example

```text

set rgb 90 100 100 100
set rgb 91  50  50  50
set rgb 92 200 200 200
set button 2 90 91 92 3 90 91 92 10
draw button  1 2.5 8.0 2.5 0.5  Click Here

```
