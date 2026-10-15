---
title: q dialog
---

# **q dialog**

`q dialog `*`<x y w h> dialog_text`*

Launches a dialog box widget that prompts for text or numeric data entry and also provides editing of an input text or value. Returns the text entered by the user.

- *`x`*`    `X-coordinate of diaglog box center\
  *`y`*`    `Y-coordinate of diaglog box center\
  *`w`*`    `width of diaglog box\
  *`h`*`    `width of diaglog box\

There are two options for the formatting of *`dialog_text`*:

- *`prompt`*
  - The prompt string provides an instruction to the user (e.g., "`Enter value for PI:`"). If *`dialog_text`* contains only a prompt, then the dialog box appears with the prompt and an empty entry point.

  *`prompt`*` | `*`initial_string`*\
  *`prompt`*` / `*`initial_string`*
  - It is possible to give the user a head start or a suggestion as to what to type in the dialog box. For this option, *`dialog_text`* contains a prompt and an initial string separated by a vertical bar or a backslash (e.g., "`Enter value for PI: | 3.14159`" or "`Enter value for PI: / 3.14159`"). The dialog box appears with the prompt and the editable initial string at the entry point.

## Usage Notes

1.  The string input by the user is returned when the Enter/Return key is pressed. Null text entries are accepted.
2.  The x,y,w, and h arguments are optional. GrADS will use the default settings if they are omitted entirely or set to -1. If w and h are set to 0, a minimum sized window is drawn to contain just the *`dialog_text`*.
3.  The cursor must be inside the dialog box to allow text entry or editing to occur.
4.  All text is centered in the dialog box. If the new text exceeds the window size, the text will be right justified in the window; the left and right arrow keys can be used to scroll the text back and forth.
5.  See the <a href="gradcomdsetdialog.html">`set dialog`</a> refrence page for details on controlling the color properties of the dialog box widget.

### Examples

```text

set dialog 1 0 5 1 6
q dialog Hello World
say result

```
