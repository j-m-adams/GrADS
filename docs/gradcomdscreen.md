---
title: screen
---

# **screen**

`screen save|show|free `*`num`*

This command allows the user to save the contents of the display window into a buffer in memory, and also to restore a saved buffer to the current display at any time. This command will work in double buffer mode and was written to facilitate animations when working interactively with GrADS. The options are as follows:

- `save`

  - Saves the contents of the display screen into memory as buffer \# *`num`*.

  `show`

  - Draws the contents of buffer \# *`num`* to the screen.

  `free`

  - Releases the contents of buffer \# *`num`* from memory.

## Usage Notes

This command does not work in batch mode, or in version 2.1+ when Cairo is enabled.
