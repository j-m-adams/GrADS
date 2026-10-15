---
title: reset
---

# **reset**

`reset `*`<qualifiers>`*

This command returns GrADS to its initial state with the following exceptions:

1.  No files are closed
2.  No defined variables are released
3.  The <a href="gradcomdsetdisplay.html">`set display`</a> settings are not modified

If files are open, the default file is set to 1, and the dimension environment is set to X,Y varying and Z and T set to 1 (as though file 1 were just opened).

The `reset` command may be qualified so that only certain aspects of GrADS are returned to their initial state.

The optional *`qualifiers`* are as follows:

- `reset events    ` resets the events buffer (e.g., mouse clicks)\
  `reset graphics  ` resets the graphics, but not the widgets\
  `reset hbuff     ` resets the display buffer when in double buffer mode\
  `reset norset    ` resets the X events only

## Usage Notes

See the section of the <a href="users.html">User's Guide</a> on <a href="reinitialization.html">reinitialization of GrADS</a>.
