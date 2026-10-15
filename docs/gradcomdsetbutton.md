---
title: set button
---

# **set button**

Sets up the color characteristics of a button widget. The syntax is:

`set button `*`<button_ON_colors> <button_OFF_colors> thickness`*

where *`<button_ON_colors>`* and *`<button_OFF_colors>`* each contain four color numbers:

- `text   `color of the button text\
  `face   `color of the button face\
  `bcol   `bright color of the shadow outline for 3-D look\
  `dcol   `dark color of the shadow outline for 3-D look\

and *`thickness`* is the thickness of the shadow outline\

## Usage Notes

1.  `set button` generally precedes the <a href="gradcomddrawbutton.html">`draw button`</a> command.
2.  See the section of the User's Guide on <a href="script.html#widgets">widgets</a> for more information on using buttons.

### Example

```text

'set rgb 90 100 100 100'
'set rgb 91  50  50  50'
'set rgb 92 200 200 200'
'set button 1 90 91 92 2 90 92 91 6'
'draw button 1 1.5 8.0 2 0.5 Button #1'
'draw button 2 4.0 8.0 2 0.5 Button #2'
'draw button 3 6.5 8.0 2 0.5 Button #3'

```
